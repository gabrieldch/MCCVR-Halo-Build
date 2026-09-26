"""Extract the shipping OpenXR support-grip locator for offline tests."""

import argparse
from pathlib import Path
from generate_haptics_runtime_fixture import extract_function


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    source_path = args.source_root.resolve() / "src" / "dll" / "vr.cpp"
    source = source_path.read_text(encoding="utf-8-sig")
    body, line = extract_function(
        source,
        "bool TryLocateSupportGripPosition(bool enabled, XrPath handPath,\n"
        "                                      XrSpace gripSpace, XrTime time,\n"
        "                                      XrVector3f& outPosition)",
    )
    output = [
        "// Generated from current shipping source; do not edit.",
        f'#line {line} "{source_path.as_posix()}"',
        body,
    ]
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n\n".join(output) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
