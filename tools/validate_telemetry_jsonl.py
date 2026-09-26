"""Validate the telemetry recorder's recoverable JSON Lines output."""

import argparse
import copy
import json
import math
from pathlib import Path


SCHEMA_VERSION = 2
PROFILE_ROLES = frozenset(
    (
        "none",
        "vs_off_control",
        "fixed_head_control",
        "fixed_shoulder_control",
        "tuning",
        "diagnostic",
    )
)
CONTROL_ROLE_BY_FIELD = {
    "cf_vs_off": "vs_off_control",
    "cf_fixed_head": "fixed_head_control",
    "cf_fixed_shoulder": "fixed_shoulder_control",
}


def reject_constant(value: str) -> None:
    raise ValueError(f"invalid JSON numeric constant: {value}")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def is_finite_number(value) -> bool:
    return type(value) in (int, float) and math.isfinite(value)


def approximately_equal(left, right, tolerance=1e-6) -> bool:
    return math.isclose(left, right, rel_tol=0.0, abs_tol=tolerance)


def compute_proximity_influence(distance, full_distance, release_distance) -> float:
    if distance <= full_distance:
        return 1.0
    if distance >= release_distance:
        return 0.0
    transition = (distance - full_distance) / (release_distance - full_distance)
    smooth = transition * transition * (3.0 - 2.0 * transition)
    return 1.0 - smooth


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("jsonl", type=Path)
    parser.add_argument("--expect-fixture", action="store_true")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()

    raw = args.jsonl.read_text(encoding="utf-8")
    clean = validate_text(raw, args.expect_fixture)
    if args.self_test:
        run_self_test(raw)
    print(f"valid telemetry JSONL ({'clean' if clean else 'recoverable unclean'} session)")


def parse_recoverable(raw: str):
    lines = raw.splitlines()
    records = []
    trailing_partial = False
    for index, line in enumerate(lines):
        if not line:
            continue
        try:
            records.append(json.loads(line, parse_constant=reject_constant))
        except (json.JSONDecodeError, ValueError):
            if index == len(lines) - 1 and not raw.endswith(("\n", "\r")):
                trailing_partial = True
                break
            raise
    unterminated_final = bool(raw) and not raw.endswith(("\n", "\r"))
    return records, trailing_partial or unterminated_final


def build_profile_mapping(start):
    mapping = start.get("test_profile_enum_mapping")
    require(isinstance(mapping, list) and mapping, "missing test-profile mapping")

    profiles_by_id = {}
    profiles_by_name = {}
    profiles_by_role = {role: [] for role in PROFILE_ROLES}
    for entry in mapping:
        require(isinstance(entry, dict), "test-profile mapping entry is not an object")
        profile_id = entry.get("id")
        name = entry.get("name")
        role = entry.get("role")
        uses_custom_settings = entry.get("uses_custom_settings")
        horizontal_release_enabled = entry.get("horizontal_release_enabled")
        require(
            type(profile_id) is int and 0 <= profile_id <= 255,
            "test-profile mapping ID is not an unsigned byte",
        )
        require(isinstance(name, str) and name, "test-profile mapping name is empty")
        require(
            isinstance(role, str) and role in PROFILE_ROLES,
            "test-profile mapping role is unknown",
        )
        require(
            type(uses_custom_settings) is bool,
            "test-profile mapping Custom policy is not boolean",
        )
        require(
            type(horizontal_release_enabled) is bool,
            "test-profile mapping horizontal-release policy is not boolean",
        )
        require(profile_id not in profiles_by_id, "duplicate test-profile mapping ID")
        require(name not in profiles_by_name, "duplicate test-profile mapping name")
        profiles_by_id[profile_id] = entry
        profiles_by_name[name] = entry
        profiles_by_role[role].append(entry)

    for role in CONTROL_ROLE_BY_FIELD.values():
        require(
            len(profiles_by_role[role]) == 1,
            f"test-profile mapping requires exactly one {role}",
        )
        require(
            not profiles_by_role[role][0]["uses_custom_settings"],
            f"control role {role} cannot use caller Custom settings",
        )
    require(
        sum(entry["uses_custom_settings"] for entry in mapping) == 1,
        "test-profile mapping requires exactly one Custom-settings profile",
    )
    return profiles_by_id, profiles_by_name, profiles_by_role


def validate_profile_reference(record, profiles_by_id, context):
    require(isinstance(record, dict), f"{context} is not an object")
    profile_id = record.get("profile_id")
    profile_name = record.get("profile_name")
    require(type(profile_id) is int, f"{context} profile ID is not an integer")
    require(profile_id in profiles_by_id, f"{context} profile ID is unknown")
    require(
        profile_name == profiles_by_id[profile_id]["name"],
        f"{context} profile name/id mismatch",
    )
    return profiles_by_id[profile_id]


def validate_text(raw: str, expect_fixture: bool = False) -> bool:
    records, trailing_partial = parse_recoverable(raw)
    if not records:
        raise ValueError("missing session_start")

    start = records[0]
    require(isinstance(start, dict), "session_start is not an object")
    require(isinstance(records[-1], dict), "final record is not an object")
    end = records[-1] if records[-1].get("type") == "session_end" else None
    frames = records[1:-1] if end else records[1:]

    require(start.get("type") == "session_start", "first record is not session_start")
    require(start.get("schema_version") == SCHEMA_VERSION, "unexpected session schema")
    require(start.get("ring_slots") == 4096, "unexpected ring slot count")
    require(start.get("ring_usable_capacity") == 4095, "unexpected ring capacity")
    require(start.get("drop_policy") == "drop_new", "unexpected drop policy")
    require(start.get("tracking_space") == "OpenXR LOCAL", "unexpected tracking space")
    require(start.get("quaternion_order") == "x,y,z,w", "unexpected quaternion order")
    require(
        start.get("coordinate_handedness") == "OpenXR right-handed",
        "unexpected coordinate handedness",
    )
    profiles_by_id, _, profiles_by_role = build_profile_mapping(start)

    for frame in frames:
        require(isinstance(frame, dict), "frame record is not an object")
        require(frame.get("type") == "frame", "non-frame record inside frame range")
        require(frame.get("schema_version") == SCHEMA_VERSION, "unexpected frame schema")
        require(
            type(frame.get("capture_begin_qpc")) is int
            and type(frame.get("capture_end_qpc")) is int,
            "capture QPC fields are not integers",
        )
        require(
            frame["capture_end_qpc"] >= frame["capture_begin_qpc"],
            "capture QPC interval is negative",
        )
        selected = validate_profile_reference(
            {
                "profile_id": frame.get("test_profile_id"),
                "profile_name": frame.get("test_profile_name"),
            },
            profiles_by_id,
            "selected",
        )
        require(
            type(frame.get("test_profile_custom")) is bool
            and frame["test_profile_custom"]
            == selected["uses_custom_settings"],
            "selected profile Custom policy mismatch",
        )
        for field, role in CONTROL_ROLE_BY_FIELD.items():
            control = frame.get(field)
            control_profile = validate_profile_reference(
                control, profiles_by_id, field
            )
            require(
                control_profile["id"] == profiles_by_role[role][0]["id"],
                f"{field} does not use the profile assigned to {role}",
            )
        settings = frame.get("effective_settings")
        require(isinstance(settings, dict), "effective settings are not an object")
        require(
            all(
                key in settings
                for key in (
                    "chest_height_m",
                    "chest_back_m",
                    "chest_side_m",
                    "two_hand_toggle",
                    "horizontal_release_enabled",
                    "horizontal_release_full_m",
                    "horizontal_release_release_m",
                )
            ),
            "effective settings are incomplete",
        )
        require(
            type(settings["horizontal_release_enabled"]) is bool,
            "horizontal release setting is not boolean",
        )
        for key in (
            "horizontal_release_full_m",
            "horizontal_release_release_m",
        ):
            require(
                is_finite_number(settings[key]),
                f"effective setting {key} is not a finite number",
            )
        require(
            settings["horizontal_release_enabled"]
            == selected["horizontal_release_enabled"],
            "selected profile horizontal-release policy mismatch",
        )
        if settings["horizontal_release_enabled"]:
            require(
                0.0 <= settings["horizontal_release_full_m"]
                < settings["horizontal_release_release_m"],
                "enabled horizontal release thresholds are invalid",
            )
        trace = frame.get("aim_trace")
        require(isinstance(trace, dict), "aim trace is not an object")
        require(
            all(
                key in trace
                for key in (
                    "path",
                    "w_natural_valid",
                    "w_natural",
                    "w_after_diagnostic_valid",
                    "w_after_diagnostic",
                    "w_effective_valid",
                    "w_effective",
                    "horizontal_release_attempted",
                    "horizontal_release_valid",
                    "rear_horizontal_reach_m",
                    "horizontal_release_influence",
                    "effective_diagnostic_override",
                    "override_applied",
                    "force_stock_eligible",
                    "c_valid",
                )
            ),
            "aim trace is missing Hybrid authority stages",
        )
        require(
            type(trace["path"]) is int and 0 <= trace["path"] <= 4,
            "aim trace solver path is unknown",
        )
        for key in (
            "w_natural_valid",
            "w_after_diagnostic_valid",
            "w_effective_valid",
            "horizontal_release_attempted",
            "horizontal_release_valid",
            "override_applied",
            "force_stock_eligible",
            "c_valid",
        ):
            require(type(trace[key]) is bool, f"aim trace {key} is not boolean")
        for key in (
            "w_natural",
            "w_after_diagnostic",
            "w_effective",
            "horizontal_release_influence",
        ):
            require(
                is_finite_number(trace[key]) and 0.0 <= trace[key] <= 1.0,
                f"aim trace {key} is not a finite unit influence",
            )
        require(
            is_finite_number(trace["rear_horizontal_reach_m"])
            and trace["rear_horizontal_reach_m"] >= 0.0,
            "rear horizontal reach is not a non-negative finite number",
        )
        diagnostic = trace["effective_diagnostic_override"]
        require(
            type(diagnostic) is int and diagnostic in (0, 1, 2),
            "effective diagnostic override is unknown",
        )
        settings_diagnostic = settings.get("hybrid_diagnostic_override")
        require(
            type(settings_diagnostic) is int and settings_diagnostic in (0, 1, 2),
            "effective settings diagnostic override is unknown",
        )
        for validity_key, value_key in (
            ("w_natural_valid", "w_natural"),
            ("w_after_diagnostic_valid", "w_after_diagnostic"),
            ("w_effective_valid", "w_effective"),
        ):
            require(
                trace[validity_key] or trace[value_key] == 0,
                f"invalid Hybrid authority stage {value_key} is nonzero",
            )
        if not settings["horizontal_release_enabled"]:
            require(
                not trace["horizontal_release_attempted"]
                and not trace["horizontal_release_valid"],
                "disabled horizontal release is not explicitly non-applicable",
            )
        require(
            not trace["horizontal_release_valid"]
            or trace["horizontal_release_attempted"],
            "horizontal release is valid without being attempted",
        )

        if trace.get("path") == 4:
            require(
                diagnostic == settings_diagnostic,
                "effective settings and Hybrid trace diagnostic overrides disagree",
            )
            require(
                trace["horizontal_release_attempted"]
                == settings["horizontal_release_enabled"],
                "Hybrid horizontal release attempt does not match effective settings",
            )
            require(
                trace["force_stock_eligible"] == trace["c_valid"],
                "Hybrid Force Stock eligibility disagrees with C validity",
            )
            require(
                trace["override_applied"]
                == (trace["force_stock_eligible"] and diagnostic != 0),
                "Hybrid diagnostic override application is inconsistent",
            )
            if trace["override_applied"]:
                forced_influence = 0.0 if diagnostic == 1 else 1.0
                require(
                    trace["w_after_diagnostic_valid"]
                    and approximately_equal(
                        trace["w_after_diagnostic"], forced_influence
                    ),
                    "Hybrid diagnostic override did not produce its exact authority",
                )
            else:
                require(
                    trace["w_after_diagnostic_valid"]
                    == trace["w_natural_valid"]
                    and approximately_equal(
                        trace["w_after_diagnostic"], trace["w_natural"]
                    ),
                    "Hybrid post-diagnostic authority does not preserve natural authority",
                )

            if settings["horizontal_release_enabled"]:
                expected_valid = (
                    trace["w_after_diagnostic_valid"]
                    and trace["horizontal_release_valid"]
                )
                expected_influence = (
                    trace["w_after_diagnostic"]
                    * trace["horizontal_release_influence"]
                    if trace["horizontal_release_valid"]
                    else 0.0
                )
            else:
                expected_valid = trace["w_after_diagnostic_valid"]
                expected_influence = trace["w_after_diagnostic"]
            require(
                trace["w_effective_valid"] == expected_valid
                and approximately_equal(trace["w_effective"], expected_influence),
                "Hybrid effective authority does not match its recorded stages",
            )
        else:
            require(
                not trace["horizontal_release_attempted"]
                and not trace["horizontal_release_valid"],
                "horizontal release ran outside the Hybrid solver path",
            )

        if not trace["horizontal_release_attempted"]:
            require(
                trace["rear_horizontal_reach_m"] == 0
                and trace["horizontal_release_influence"] == 0,
                "non-applicable horizontal release contains computed geometry",
            )
        elif not trace["horizontal_release_valid"]:
            require(
                trace["rear_horizontal_reach_m"] == 0
                and trace["horizontal_release_influence"] == 0,
                "invalid horizontal release retained geometry or authority",
            )
        else:
            expected_release_influence = compute_proximity_influence(
                trace["rear_horizontal_reach_m"],
                settings["horizontal_release_full_m"],
                settings["horizontal_release_release_m"],
            )
            require(
                approximately_equal(
                    trace["horizontal_release_influence"],
                    expected_release_influence,
                ),
                "horizontal release influence disagrees with reach and thresholds",
            )
        require(
            all(
                not isinstance(value, float) or math.isfinite(value)
                for value in walk_values(frame)
            ),
            "serialized frame contains a non-finite JSON number",
        )
        if expect_fixture:
            validate_fixture_frame(frame, selected)

    if end:
        require(isinstance(end, dict), "session_end is not an object")
        require(end.get("schema_version") == SCHEMA_VERSION, "unexpected end schema")
        require(end.get("clean_stop") is True, "session_end is not clean")
        require(
            all(
                type(end.get(key)) is int
                for key in (
                    "producer_calls",
                    "duplicate_serial_suppressed",
                    "enqueued",
                    "written",
                    "dropped_queue_full",
                )
            ),
            "session_end accounting fields are not integers",
        )
        require(
            end["producer_calls"]
            == end["duplicate_serial_suppressed"]
            + end["enqueued"]
            + end["dropped_queue_full"],
            "producer accounting identity failed",
        )
        require(
            end["written"] == end["enqueued"] == len(frames),
            "clean session did not write every enqueued frame",
        )

    if expect_fixture:
        require(
            not trailing_partial and end is not None,
            "fixture session is not cleanly terminated",
        )
        require(len(frames) == 2, "fixture frame count mismatch")
        require(
            [frame["prepared_serial"] for frame in frames] == [0, 42],
            "fixture serial sequence mismatch",
        )
        require(end["producer_calls"] == 4, "fixture producer count mismatch")
        require(
            end["duplicate_serial_suppressed"] == 2,
            "fixture duplicate count mismatch",
        )
        require(end["enqueued"] == 2, "fixture enqueue count mismatch")
        require(end["written"] == 2, "fixture written count mismatch")
        require(end["dropped_queue_full"] == 0, "fixture drop count mismatch")

    return end is not None and not trailing_partial


def validate_fixture_frame(frame, selected_profile) -> None:
    require(
        frame["semantic_primary_aim"]["position"] == [1, 2, 3],
        "fixture primary position mismatch",
    )
    require(
        approximately_equal_sequence(
            frame["semantic_primary_aim"]["orientation"],
            [0.1, 0.2, 0.3, 0.9],
        ),
        "fixture primary orientation mismatch",
    )
    require(
        frame["semantic_primary_linear_velocity"] == [None, 2, 3],
        "fixture velocity null mismatch",
    )
    require(math.isclose(frame["pad"]["moveX"], 0.5, abs_tol=1e-6), "fixture pad mismatch")
    require(
        selected_profile["role"] == "tuning" and not frame["test_profile_custom"],
        "fixture profile is not a non-Custom tuning profile",
    )
    require(
        frame["effective_settings"]["two_hand_toggle"] is False,
        "fixture acquisition mode mismatch",
    )
    require(
        math.isclose(
            frame["effective_settings"]["hybrid_seat_full_m"],
            0.6,
            abs_tol=1e-6,
        ),
        "fixture full-seat threshold mismatch",
    )
    require(
        math.isclose(
            frame["effective_settings"]["hybrid_seat_release_m"],
            0.8,
            abs_tol=1e-6,
        ),
        "fixture release-seat threshold mismatch",
    )
    require(frame["aim_trace"]["w_natural_valid"], "fixture natural W is invalid")
    require(
        frame["aim_trace"]["w_after_diagnostic_valid"],
        "fixture post-diagnostic W is invalid",
    )
    require(frame["aim_trace"]["w_effective_valid"], "fixture effective W is invalid")
    require(
        math.isclose(frame["aim_trace"]["w_natural"], 0.375, abs_tol=1e-6),
        "fixture natural W mismatch",
    )
    require(
        math.isclose(
            frame["aim_trace"]["w_after_diagnostic"], 0.375, abs_tol=1e-6
        ),
        "fixture post-diagnostic W mismatch",
    )
    require(
        math.isclose(frame["aim_trace"]["w_effective"], 0.375, abs_tol=1e-6),
        "fixture effective W mismatch",
    )
    require(
        math.isclose(
            frame["aim_trace"]["rear_to_stock_target_distance_m"],
            0.25,
            abs_tol=1e-6,
        ),
        "fixture release distance mismatch",
    )
    require(
        frame["effective_settings"]["horizontal_release_enabled"] is False
        and frame["aim_trace"]["horizontal_release_attempted"] is False
        and frame["aim_trace"]["horizontal_release_valid"] is False,
        "fixture disabled horizontal release is not explicitly non-applicable",
    )
    require(
        frame["cf_fixed_head"]["path"] == 3
        and frame["cf_fixed_head"]["fixed_direction_valid"]
        and frame["cf_fixed_head"]["orientation_rebuild_succeeded"],
        "fixture fixed-control cause fields mismatch",
    )


def records_to_text(records) -> str:
    return "".join(json.dumps(record) + "\n" for record in records)


def require_rejected(records, message: str) -> None:
    try:
        validate_text(records_to_text(records))
    except ValueError:
        return
    raise AssertionError(message)


def profile_for_role(start, role):
    return next(
        entry
        for entry in start["test_profile_enum_mapping"]
        if entry["role"] == role
    )


def run_self_test(fixture_raw: str) -> None:
    records, partial = parse_recoverable(fixture_raw)
    require(not partial, "clean fixture parsed as partial")
    start = records[0]
    end = dict(records[-1])
    end.update(
        producer_calls=0,
        duplicate_serial_suppressed=0,
        enqueued=0,
        written=0,
        dropped_queue_full=0,
    )
    empty_clean = json.dumps(start) + "\n" + json.dumps(end) + "\n"
    require(validate_text(empty_clean), "empty clean session was rejected")

    reordered = copy.deepcopy(records)
    reordered[0]["test_profile_enum_mapping"].reverse()
    require(
        validate_text(records_to_text(reordered)),
        "self-describing mapping became order-dependent",
    )

    renumbered = copy.deepcopy(records)
    id_remap = {}
    for index, entry in enumerate(renumbered[0]["test_profile_enum_mapping"]):
        old_id = entry["id"]
        entry["id"] = 200 + index
        id_remap[old_id] = entry["id"]
    for frame in renumbered[1:-1]:
        frame["test_profile_id"] = id_remap[frame["test_profile_id"]]
        for field in CONTROL_ROLE_BY_FIELD:
            frame[field]["profile_id"] = id_remap[frame[field]["profile_id"]]
    require(
        validate_text(records_to_text(renumbered)),
        "self-consistent file-local profile renumbering was rejected",
    )

    extended = copy.deepcopy(records)
    used_ids = {
        entry["id"] for entry in extended[0]["test_profile_enum_mapping"]
    }
    extra_id = next(value for value in range(255, -1, -1) if value not in used_ids)
    extended[0]["test_profile_enum_mapping"].append(
        {
            "id": extra_id,
            "name": "FutureTuningProfile",
            "role": "tuning",
            "uses_custom_settings": False,
            "horizontal_release_enabled": False,
        }
    )
    require(
        validate_text(records_to_text(extended)),
        "additional non-control profile was rejected",
    )

    enabled_release = copy.deepcopy(records)
    selected_id = enabled_release[1]["test_profile_id"]
    selected_mapping = next(
        entry
        for entry in enabled_release[0]["test_profile_enum_mapping"]
        if entry["id"] == selected_id
    )
    selected_mapping["horizontal_release_enabled"] = True
    for enabled_frame in enabled_release[1:-1]:
        enabled_frame["effective_settings"]["horizontal_release_enabled"] = True
        enabled_frame["effective_settings"]["hybrid_diagnostic_override"] = 2
        enabled_trace = enabled_frame["aim_trace"]
        enabled_trace["effective_diagnostic_override"] = 2
        enabled_trace["override_applied"] = True
        enabled_trace["w_after_diagnostic"] = 1.0
        enabled_trace["horizontal_release_attempted"] = True
        enabled_trace["horizontal_release_valid"] = True
        enabled_trace["rear_horizontal_reach_m"] = 0.3475
        enabled_trace["horizontal_release_influence"] = 0.5
        enabled_trace["w_effective"] = 0.5
    require(
        validate_text(records_to_text(enabled_release)),
        "internally consistent enabled horizontal release was rejected",
    )

    enabled_non_hybrid = copy.deepcopy(enabled_release)
    for non_hybrid_frame in enabled_non_hybrid[1:-1]:
        non_hybrid_trace = non_hybrid_frame["aim_trace"]
        non_hybrid_trace["path"] = 1
        non_hybrid_trace["c_valid"] = False
        non_hybrid_trace["force_stock_eligible"] = False
        non_hybrid_trace["effective_diagnostic_override"] = 0
        non_hybrid_trace["override_applied"] = False
        non_hybrid_trace["w_natural_valid"] = False
        non_hybrid_trace["w_natural"] = 0.0
        non_hybrid_trace["w_after_diagnostic_valid"] = False
        non_hybrid_trace["w_after_diagnostic"] = 0.0
        non_hybrid_trace["w_effective_valid"] = False
        non_hybrid_trace["w_effective"] = 0.0
        non_hybrid_trace["horizontal_release_attempted"] = False
        non_hybrid_trace["horizontal_release_valid"] = False
        non_hybrid_trace["rear_horizontal_reach_m"] = 0.0
        non_hybrid_trace["horizontal_release_influence"] = 0.0
    require(
        validate_text(records_to_text(enabled_non_hybrid)),
        "enabled profile taking a non-Hybrid path was rejected",
    )

    wrong_release_curve = copy.deepcopy(enabled_release)
    wrong_release_curve[1]["aim_trace"]["rear_horizontal_reach_m"] = 0.20
    wrong_release_curve[1]["aim_trace"]["horizontal_release_influence"] = 0.5
    wrong_release_curve[1]["aim_trace"]["w_effective"] = 0.5
    require_rejected(
        wrong_release_curve,
        "horizontal release influence inconsistent with reach was accepted",
    )

    invalid_nonzero_authority = copy.deepcopy(records)
    invalid_trace = invalid_nonzero_authority[1]["aim_trace"]
    invalid_trace["c_valid"] = False
    invalid_trace["force_stock_eligible"] = False
    invalid_trace["w_natural_valid"] = False
    invalid_trace["w_natural"] = 0.5
    invalid_trace["w_after_diagnostic_valid"] = False
    invalid_trace["w_after_diagnostic"] = 0.5
    invalid_trace["w_effective_valid"] = False
    invalid_trace["w_effective"] = 0.5
    require_rejected(
        invalid_nonzero_authority,
        "invalid Hybrid authority stages retained nonzero values",
    )

    duplicate_id = copy.deepcopy(records)
    duplicate_id[0]["test_profile_enum_mapping"].append(
        {
            "id": duplicate_id[0]["test_profile_enum_mapping"][0]["id"],
            "name": "DuplicateIdProfile",
            "role": "tuning",
            "uses_custom_settings": False,
            "horizontal_release_enabled": False,
        }
    )
    require_rejected(duplicate_id, "duplicate profile ID was accepted")

    duplicate_name = copy.deepcopy(records)
    duplicate_name[0]["test_profile_enum_mapping"].append(
        {
            "id": extra_id,
            "name": duplicate_name[0]["test_profile_enum_mapping"][0]["name"],
            "role": "tuning",
            "uses_custom_settings": False,
            "horizontal_release_enabled": False,
        }
    )
    require_rejected(duplicate_name, "duplicate profile name was accepted")

    missing_role = copy.deepcopy(records)
    profile_for_role(missing_role[0], "vs_off_control")["role"] = "tuning"
    require_rejected(missing_role, "missing required control role was accepted")

    duplicate_role = copy.deepcopy(records)
    profile_for_role(duplicate_role[0], "tuning")["role"] = "vs_off_control"
    require_rejected(duplicate_role, "duplicate required control role was accepted")

    unknown_role = copy.deepcopy(records)
    unknown_role[0]["test_profile_enum_mapping"][0]["role"] = "unknown_role"
    require_rejected(unknown_role, "unknown profile role was accepted")

    unknown_selected = copy.deepcopy(records)
    unknown_selected[1]["test_profile_id"] = extra_id
    require_rejected(unknown_selected, "unknown selected profile ID was accepted")

    mismatched_name = copy.deepcopy(records)
    mismatched_name[1]["test_profile_name"] = "MismatchedProfileName"
    require_rejected(mismatched_name, "selected profile name mismatch was accepted")

    mismatched_custom = copy.deepcopy(records)
    mismatched_custom[1]["test_profile_custom"] = not mismatched_custom[1][
        "test_profile_custom"
    ]
    require_rejected(mismatched_custom, "selected profile Custom policy mismatch was accepted")

    unknown_control = copy.deepcopy(records)
    unknown_control[1]["cf_vs_off"]["profile_id"] = extra_id
    require_rejected(unknown_control, "unknown control profile ID was accepted")

    wrong_control = copy.deepcopy(records)
    fixed_head = profile_for_role(wrong_control[0], "fixed_head_control")
    wrong_control[1]["cf_vs_off"]["profile_id"] = fixed_head["id"]
    wrong_control[1]["cf_vs_off"]["profile_name"] = fixed_head["name"]
    require_rejected(wrong_control, "control profile assigned to the wrong role was accepted")

    malformed_role = copy.deepcopy(records)
    malformed_role[0]["test_profile_enum_mapping"][0]["role"] = []
    require_rejected(malformed_role, "non-string profile role was accepted")

    malformed_qpc = copy.deepcopy(records)
    malformed_qpc[1]["capture_begin_qpc"] = "100"
    malformed_qpc[1]["capture_end_qpc"] = "140"
    require_rejected(malformed_qpc, "string capture QPC fields were accepted")

    malformed_final = copy.deepcopy(records)
    malformed_final.append([])
    require_rejected(malformed_final, "non-object final record was accepted")

    missing_authority_stage = copy.deepcopy(records)
    del missing_authority_stage[1]["aim_trace"]["w_after_diagnostic"]
    require_rejected(
        missing_authority_stage,
        "missing post-diagnostic authority stage was accepted",
    )

    disabled_but_attempted = copy.deepcopy(records)
    disabled_but_attempted[1]["aim_trace"]["horizontal_release_attempted"] = True
    require_rejected(
        disabled_but_attempted,
        "disabled horizontal release was accepted as attempted",
    )

    malformed_authority = copy.deepcopy(records)
    malformed_authority[1]["aim_trace"]["w_natural"] = "0.375"
    require_rejected(
        malformed_authority,
        "non-numeric Hybrid authority was accepted",
    )

    out_of_range_influence = copy.deepcopy(records)
    out_of_range_influence[1]["aim_trace"]["horizontal_release_influence"] = 7.0
    require_rejected(
        out_of_range_influence,
        "out-of-range horizontal release influence was accepted",
    )

    inconsistent_authority = copy.deepcopy(records)
    inconsistent_authority[1]["aim_trace"]["w_effective"] = 0.9
    require_rejected(
        inconsistent_authority,
        "inconsistent Hybrid authority stages were accepted",
    )

    policy_mismatch = copy.deepcopy(records)
    policy_mismatch[1]["effective_settings"]["horizontal_release_enabled"] = True
    require_rejected(
        policy_mismatch,
        "profile horizontal-release policy mismatch was accepted",
    )

    inconsistent_diagnostic = copy.deepcopy(records)
    inconsistent_diagnostic[1]["effective_settings"][
        "hybrid_diagnostic_override"
    ] = 2
    inconsistent_diagnostic[1]["aim_trace"]["effective_diagnostic_override"] = 2
    inconsistent_diagnostic[1]["aim_trace"]["override_applied"] = True
    require_rejected(
        inconsistent_diagnostic,
        "inconsistent Force Stock diagnostic authority was accepted",
    )

    truncated = fixture_raw.rstrip("\r\n")[:-8]
    require(not validate_text(truncated), "truncated final record was accepted as clean")

    unterminated = fixture_raw.rstrip("\r\n")
    require(
        not validate_text(unterminated),
        "valid JSON without a final newline was accepted as clean",
    )

    corrupted_middle = json.dumps(start) + "\n{broken}\n" + json.dumps(end) + "\n"
    try:
        validate_text(corrupted_middle)
    except json.JSONDecodeError:
        pass
    else:
        raise AssertionError("malformed non-final records must be rejected")


def walk_values(value):
    if isinstance(value, dict):
        for child in value.values():
            yield from walk_values(child)
    elif isinstance(value, list):
        for child in value:
            yield from walk_values(child)
    else:
        yield value


def approximately_equal_sequence(actual, expected, tolerance=1e-6):
    return len(actual) == len(expected) and all(
        math.isclose(left, right, abs_tol=tolerance)
        for left, right in zip(actual, expected)
    )


if __name__ == "__main__":
    main()
