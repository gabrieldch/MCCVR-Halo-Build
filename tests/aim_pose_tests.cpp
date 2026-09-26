#include <Windows.h>
#include <openxr/openxr.h>
#include "../src/common/virtual_stock_logic.h"
#include "../src/common/virtual_stock_test_profiles.h"
#include "../src/dll/aim_pose_trace.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iterator>

namespace
{
unsigned checks{}, failures{};

void Check(bool value, const char* message)
{
    ++checks;
    if (!value)
    {
        ++failures;
        std::fprintf(stderr, "FAIL: %s\n", message);
    }
}

XrVector3f Rotate(const XrQuaternionf& q, const XrVector3f& v)
{
    const XrVector3f u{q.x, q.y, q.z};
    const auto cross = [](const XrVector3f& a, const XrVector3f& b) {
        return XrVector3f{
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
    };
    XrVector3f c1 = cross(u, v);
    c1.x += q.w * v.x;
    c1.y += q.w * v.y;
    c1.z += q.w * v.z;
    const XrVector3f c2 = cross(u, c1);
    return {v.x + 2.0f * c2.x, v.y + 2.0f * c2.y, v.z + 2.0f * c2.z};
}

#include "aim_pose_functions.inl"

bool Near(float actual, float expected, float epsilon = 1.0e-5f)
{
    return std::fabs(actual - expected) <= epsilon;
}

bool SamePosition(const XrVector3f& actual, const XrVector3f& expected)
{
    return Near(actual.x, expected.x) && Near(actual.y, expected.y) &&
        Near(actual.z, expected.z);
}

bool SameQuaternion(const XrQuaternionf& actual, const XrQuaternionf& expected,
    float epsilon = 1.0e-5f)
{
    return Near(actual.x, expected.x, epsilon) &&
        Near(actual.y, expected.y, epsilon) &&
        Near(actual.z, expected.z, epsilon) &&
        Near(actual.w, expected.w, epsilon);
}

XrVector3f AimForward(const AimPoseResult& result)
{
    return Rotate(result.pose.orientation, {0.0f, 0.0f, -1.0f});
}

bool SameAimResult(const AimPoseResult& actual, const AimPoseResult& expected,
    float epsilon = 1.0e-5f)
{
    return actual.valid == expected.valid &&
        actual.updateTwoHandActivity == expected.updateTwoHandActivity &&
        actual.twoHandActive == expected.twoHandActive &&
        actual.rejectedExtreme == expected.rejectedExtreme &&
        Near(actual.rejectedAgreement, expected.rejectedAgreement, epsilon) &&
        SamePosition(actual.pose.position, expected.pose.position) &&
        SameQuaternion(actual.pose.orientation, expected.pose.orientation, epsilon) &&
        SamePosition(AimForward(actual), AimForward(expected));
}

bool ExactAimResult(const AimPoseResult& actual, const AimPoseResult& expected)
{
    return actual.valid == expected.valid &&
        actual.updateTwoHandActivity == expected.updateTwoHandActivity &&
        actual.twoHandActive == expected.twoHandActive &&
        actual.rejectedExtreme == expected.rejectedExtreme &&
        actual.rejectedAgreement == expected.rejectedAgreement &&
        actual.pose.position.x == expected.pose.position.x &&
        actual.pose.position.y == expected.pose.position.y &&
        actual.pose.position.z == expected.pose.position.z &&
        actual.pose.orientation.x == expected.pose.orientation.x &&
        actual.pose.orientation.y == expected.pose.orientation.y &&
        actual.pose.orientation.z == expected.pose.orientation.z &&
        actual.pose.orientation.w == expected.pose.orientation.w;
}

XrQuaternionf Multiply(const XrQuaternionf& a, const XrQuaternionf& b)
{
    return {
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z};
}

XrQuaternionf ApplyCalibration(XrQuaternionf pose, float yawDeg,
    float pitchDeg, float rollDeg)
{
    constexpr float kDegToRad = 0.01745329252f;
    const float yaw = yawDeg * kDegToRad;
    const float pitch = pitchDeg * kDegToRad;
    const float roll = rollDeg * kDegToRad;
    const XrQuaternionf qYaw{
        0.0f, std::sin(yaw * 0.5f), 0.0f, std::cos(yaw * 0.5f)};
    const XrQuaternionf qPitch{
        std::sin(pitch * 0.5f), 0.0f, 0.0f, std::cos(pitch * 0.5f)};
    const XrQuaternionf qRoll{
        0.0f, 0.0f, std::sin(-roll * 0.5f), std::cos(roll * 0.5f)};
    const XrQuaternionf corrected = Multiply(
        pose, Multiply(Multiply(qYaw, qPitch), qRoll));
    const float length = std::sqrt(
        corrected.x * corrected.x + corrected.y * corrected.y +
        corrected.z * corrected.z + corrected.w * corrected.w);
    return {
        corrected.x / length, corrected.y / length,
        corrected.z / length, corrected.w / length};
}

AimPoseInputs HybridInputs()
{
    AimPoseInputs inputs{};
    inputs.rightValid = true;
    inputs.right.orientation = {0.0f, 0.0f, 0.0f, 1.0f};
    inputs.right.position = {0.0f, 0.0f, 0.0f};
    inputs.leftValid = true;
    inputs.left.orientation = {0.0f, 0.0f, 0.0f, 1.0f};
    inputs.left.position = {0.0f, 0.0f, 0.0f};
    inputs.supportPosition = {0.6f, 0.0f, -1.0f};
    inputs.twoHandEnabled = true;
    inputs.twoHandLatched = true;
    inputs.virtualStockEnabled = true;
    inputs.virtualStockStrength = 1.0f;
    inputs.virtualStockRearHeightM = 0.0f;
    inputs.virtualStockRearReference = 3;
    inputs.virtualStockShoulderBackM = 0.20f;
    inputs.virtualStockShoulderSideM = 0.10f;
    inputs.virtualStockHybridOffhandInfluence = 0.50f;
    inputs.virtualStockHybridAdsReference = 0;
    inputs.virtualStockHybridSeatFullM = 0.050f;
    inputs.virtualStockHybridSeatReleaseM = 0.150f;
    inputs.headValid = true;
    inputs.headPosition = {0.0f, 0.5f, 0.0f};
    inputs.headOrientation = {0.0f, 0.0f, 0.0f, 1.0f};
    return inputs;
}

AimPoseInputs EquivalenceInputs()
{
    AimPoseInputs inputs = HybridInputs();
    inputs.right.position = {0.10f, 0.08f, 0.02f};
    inputs.supportPosition = {0.42f, 0.08f, -0.78f};
    inputs.headPosition = {-0.18f, 0.62f, 0.11f};
    inputs.headOrientation = {0.0f, 0.0f, 0.0f, 1.0f};
    inputs.virtualStockStrength = 0.73f;
    inputs.virtualStockRearHeightM = -0.12f;
    inputs.virtualStockShoulderBackM = 0.11f;
    inputs.virtualStockShoulderSideM = 0.07f;
    inputs.virtualStockProximityRelease = false;
    inputs.virtualStockProximityFullM = 0.270f;
    inputs.virtualStockProximityReleaseM = 0.425f;
    inputs.hybridDiagnosticOverride = HybridDiagnosticOverride::Normal;
    return inputs;
}

void TestHybridMode3()
{
    AimPoseInputs inputs = HybridInputs();
    inputs.headValid = false;
    inputs.virtualStockHybridOffhandInfluence = 0.50f;
    inputs.virtualStockAdaptiveTopHeightM = NAN;
    inputs.virtualStockAdaptiveBottomHeightM = NAN;
    inputs.virtualStockAdaptiveTopHalfWidthM = NAN;
    inputs.virtualStockAdaptiveBottomHalfWidthM = NAN;
    const AimPoseResult result = ComputeAimPose(inputs);
    AimPoseTrace trace{};
    const AimPoseResult traced = ComputeAimPose(inputs, &trace);

    const virtual_stock::Point3 a{0.0f, 0.0f, -1.0f};
    const virtual_stock::Point3 b = virtual_stock::Point3{
        0.6f, 0.0f, -1.0f};
    const float bLength = std::sqrt(1.36f);
    const virtual_stock::Point3 expected = {
        (a.x * 0.5f + b.x / bLength * 0.5f),
        0.0f,
        (a.z * 0.5f + b.z / bLength * 0.5f)};
    const float expectedLength = std::sqrt(
        expected.x * expected.x + expected.y * expected.y +
        expected.z * expected.z);
    const XrVector3f expectedDirection{
        expected.x / expectedLength, expected.y / expectedLength,
        expected.z / expectedLength};
    Check(result.valid && result.twoHandActive,
        "Production mode 3 selects Hybrid and keeps accepted B presentation active");
    Check(ExactAimResult(traced, result) &&
            trace.path == AimSolverPath::Hybrid &&
            !trace.fixedTargetValid && !trace.fixedRearDistanceValid &&
            !trace.fixedProximityCalculated &&
            trace.requestedTarget == AimStockTarget::Head,
        "Traced mode 3 stays on Hybrid and never instruments the dormant Adaptive tail");
    Check(SamePosition(result.pose.position, inputs.right.position),
        "Hybrid production output position remains the exact primary P");
    Check(SamePosition(Rotate(result.pose.orientation, {0.0f, 0.0f, -1.0f}),
            expectedDirection),
        "Production mode 3 uses A/B direction authority rather than Adaptive geometry");
}

void TestHybridEndpointsAndFallbacks()
{
    AimPoseInputs inputs = HybridInputs();
    inputs.headValid = false;
    inputs.virtualStockHybridOffhandInfluence = 0.0f;
    const XrQuaternionf exactA{
        0.0f, std::sin(0.2f), 0.0f, std::cos(0.2f)};
    inputs.right.orientation = exactA;
    AimPoseResult result = ComputeAimPose(inputs);
    Check(result.valid && result.twoHandActive &&
            SameQuaternion(result.pose.orientation, exactA),
        "Hybrid influence zero with no C preserves the exact A quaternion and active presentation");
    Check(SamePosition(result.pose.position, inputs.right.position),
        "Pure-A Hybrid endpoint preserves P");

    inputs = HybridInputs();
    inputs.virtualStockHybridOffhandInfluence = 0.0f;
    result = ComputeAimPose(inputs);
    Check(result.valid && result.twoHandActive &&
            SameQuaternion(result.pose.orientation, inputs.right.orientation),
        "Accepted B plus influence zero plus W zero preserves exact A and remains active");

    inputs = HybridInputs();
    inputs.headValid = false;
    inputs.left.orientation = {0.4f, 0.0f, 0.0f, 0.916515f};
    result = ComputeAimPose(inputs);
    AimPoseInputs unchangedSupport = inputs;
    unchangedSupport.left.orientation = {0.0f, 0.7f, 0.0f, 0.714143f};
    const AimPoseResult rotatedSupport = ComputeAimPose(unchangedSupport);
    Check(result.valid && result.twoHandActive &&
            SameQuaternion(result.pose.orientation, rotatedSupport.pose.orientation),
        "Support-controller rotation has no direct Hybrid forward or roll authority");

    inputs = HybridInputs();
    inputs.headValid = false;
    inputs.supportPosition = {1.0f, 0.0f, 0.0f};
    result = ComputeAimPose(inputs);
    Check(result.valid && !result.twoHandActive &&
            SameQuaternion(result.pose.orientation, inputs.right.orientation),
        "Rejected B with no valid positive C falls back to exact A and inactive presentation");

    inputs = HybridInputs();
    inputs.supportPosition = {0.2f, 0.0f, 0.0f};
    inputs.right.position = {0.2f, 0.0f, 0.0f};
    inputs.headPosition = {0.0f, 0.0f, 0.0f};
    inputs.virtualStockHybridOffhandInfluence = 0.0f;
    result = ComputeAimPose(inputs);
    const XrVector3f cOnlyForward = Rotate(
        result.pose.orientation, {0.0f, 0.0f, -1.0f});
    Check(result.valid && result.twoHandActive &&
            Near(cOnlyForward.x, 1.0f, 1.0e-4f) &&
            Near(cOnlyForward.y, 0.0f, 1.0e-4f) &&
            Near(cOnlyForward.z, 0.0f, 1.0e-4f),
        "Rejected B can be rescued by valid C and W greater than zero");

    inputs = HybridInputs();
    inputs.supportPosition = {1.0f, 0.0f, 0.0f};
    inputs.virtualStockStrength = 0.0f;
    inputs.headValid = true;
    result = ComputeAimPose(inputs);
    Check(result.valid && !result.twoHandActive &&
            SameQuaternion(result.pose.orientation, inputs.right.orientation),
        "Rejected B cannot be resurrected through zero-strength C");
}

void TestHybridTargetsAndCalibration()
{
    AimPoseInputs inputs = HybridInputs();
    inputs.right.position = {0.0f, 0.0f, 0.0f};
    inputs.supportPosition = {0.3f, 0.0f, -1.0f};
    inputs.headPosition = {0.0f, 0.0f, 0.0f};
    inputs.virtualStockHybridOffhandInfluence = 0.0f;
    inputs.virtualStockHybridSeatFullM = 0.50f;
    inputs.virtualStockHybridSeatReleaseM = 0.60f;
    inputs.virtualStockHybridAdsReference = 0;
    AimPoseResult head = ComputeAimPose(inputs);
    const XrVector3f headForward = Rotate(
        head.pose.orientation, {0.0f, 0.0f, -1.0f});

    inputs.virtualStockHybridAdsReference = 1;
    const AimPoseResult shoulder = ComputeAimPose(inputs);
    const XrVector3f shoulderForward = Rotate(
        shoulder.pose.orientation, {0.0f, 0.0f, -1.0f});
    Check(head.valid && shoulder.valid && head.twoHandActive &&
            shoulder.twoHandActive &&
            !SamePosition(headForward, shoulderForward),
        "Hybrid Head and Shoulder ADS references produce independently observable C targets");

    inputs.headOrientation.x = NAN;
    const AimPoseResult shoulderFallback = ComputeAimPose(inputs);
    Check(SamePosition(Rotate(shoulderFallback.pose.orientation,
                {0.0f, 0.0f, -1.0f}), headForward),
        "Invalid Hybrid Shoulder target falls back to the valid Head target");

    inputs = HybridInputs();
    inputs.headValid = false;
    inputs.virtualStockHybridOffhandInfluence = 0.0f;
    inputs.gunYawDeg = 7.0f;
    inputs.gunPitchDeg = -3.0f;
    inputs.gunRollDeg = 2.0f;
    const AimPoseResult calibrated = ComputeAimPose(inputs);
    const XrQuaternionf expected = ApplyCalibration(
        inputs.right.orientation, inputs.gunYawDeg,
        inputs.gunPitchDeg, inputs.gunRollDeg);
    Check(calibrated.valid && SameQuaternion(
            calibrated.pose.orientation, expected, 2.0e-5f),
        "Shared finishAimPose applies gun calibration exactly once after Hybrid exact-A selection");
}

void TestHybridIgnoresLegacyProximity()
{
    AimPoseInputs baseline = HybridInputs();
    baseline.headPosition = {0.0f, 0.0f, 0.0f};
    baseline.virtualStockProximityRelease = false;
    baseline.virtualStockProximityFullM = 0.250f;
    baseline.virtualStockProximityReleaseM = 0.450f;
    const AimPoseResult withoutLegacyProximity = ComputeAimPose(baseline);

    AimPoseInputs changed = baseline;
    changed.virtualStockProximityRelease = true;
    changed.virtualStockProximityFullM = 0.010f;
    changed.virtualStockProximityReleaseM = 0.020f;
    const AimPoseResult withLegacyProximity = ComputeAimPose(changed);

    Check(withoutLegacyProximity.valid == withLegacyProximity.valid &&
            withoutLegacyProximity.twoHandActive == withLegacyProximity.twoHandActive &&
            withoutLegacyProximity.rejectedExtreme == withLegacyProximity.rejectedExtreme &&
            Near(withoutLegacyProximity.rejectedAgreement,
                withLegacyProximity.rejectedAgreement) &&
            SamePosition(withoutLegacyProximity.pose.position,
                withLegacyProximity.pose.position) &&
            SameQuaternion(withoutLegacyProximity.pose.orientation,
                withLegacyProximity.pose.orientation),
        "Hybrid output is independent of legacy proximity release settings");
}

void TestHybridHorizontalReleaseComposition()
{
    const auto makeBase = [](float horizontalReach) {
        AimPoseInputs inputs = HybridInputs();
        inputs.right.position = {horizontalReach, 0.0f, 0.0f};
        inputs.supportPosition = {
            horizontalReach + 0.15f, 0.0f, -1.0f};
        inputs.headPosition = {0.0f, 0.0f, 0.0f};
        inputs.headOrientation = {0.0f, 0.0f, 0.0f, 1.0f};
        return inputs;
    };
    const auto forProfile = [](const AimPoseInputs& base,
                               VirtualStockTestProfile profile) {
        return AimPoseInputsForProfile(base, profile);
    };
    constexpr VirtualStockTestProfile kHorizontalExperimentProfile =
        static_cast<VirtualStockTestProfile>(
            kVirtualStockExperimentProfileFirstId);
    constexpr VirtualStockAimSettings kHorizontalSettings =
        MakeHybridHorizontalReleaseProfileSettings(0.270f, 0.425f);
    const auto forHorizontal = [&](const AimPoseInputs& base) {
        AimPoseInputs resolved = base;
        ApplyVirtualStockAimSettings(
            resolved, kHorizontalSettings, kHorizontalExperimentProfile);
        return resolved;
    };

    const AimPoseInputs closeBase = makeBase(0.20f);
    const AimPoseInputs closeHorizontal = forHorizontal(closeBase);
    const AimPoseInputs closeForceStock = forProfile(
        closeBase, VirtualStockTestProfile::F_HybridForceStockHead);
    AimPoseTrace closeTrace{};
    const AimPoseResult closeResult = ComputeAimPose(
        closeHorizontal, &closeTrace);
    const AimPoseResult forceStockResult = ComputeAimPose(closeForceStock);
    Check(SameAimResult(closeResult, forceStockResult, 2.0e-5f) &&
            closeTrace.horizontalReleaseAttempted &&
            closeTrace.horizontalReleaseValid &&
            closeTrace.horizontalReleaseInfluence == 1.0f &&
            closeTrace.wAfterDiagnosticValid &&
            closeTrace.wAfterDiagnostic == 1.0f &&
            closeTrace.wEffectiveValid &&
            closeTrace.wEffective == 1.0f &&
            SamePosition(closeResult.pose.position,
                closeHorizontal.right.position),
        "Horizontal-release tuning inside full radius exactly matches Force Stock C and preserves P");

    const AimPoseInputs releasedBase = makeBase(0.60f);
    const AimPoseInputs releasedHorizontal = forHorizontal(releasedBase);
    const AimPoseInputs releasedFixed = forProfile(
        releasedBase, VirtualStockTestProfile::B_FixedHeadControl);
    AimPoseTrace releasedTrace{};
    const AimPoseResult releasedResult = ComputeAimPose(
        releasedHorizontal, &releasedTrace);
    const AimPoseResult fixedReleasedResult = ComputeAimPose(releasedFixed);
    Check(SameAimResult(releasedResult, fixedReleasedResult, 2.0e-5f) &&
            releasedTrace.bAccepted &&
            releasedTrace.horizontalReleaseValid &&
            releasedTrace.horizontalReleaseInfluence == 0.0f &&
            releasedTrace.wAfterDiagnostic == 1.0f &&
            releasedTrace.wEffective == 0.0f &&
            !releasedTrace.stockContributed &&
            SamePosition(releasedResult.pose.position,
                releasedHorizontal.right.position),
        "Horizontal-release tuning outside release exactly matches the fixed-stock guarded B endpoint");

    const AimPoseInputs transitionBase = makeBase(0.3475f);
    const AimPoseInputs transition = forHorizontal(transitionBase);
    AimPoseTrace transitionTrace{};
    const AimPoseResult transitionResult = ComputeAimPose(
        transition, &transitionTrace);
    virtual_stock::Point3 expectedTransition{};
    const bool expectedTransitionValid =
        virtual_stock::TryBlendDirectionAuthority(
            {transitionTrace.bDirection.x, transitionTrace.bDirection.y,
                transitionTrace.bDirection.z},
            {transitionTrace.cDirection.x, transitionTrace.cDirection.y,
                transitionTrace.cDirection.z},
            transitionTrace.horizontalReleaseInfluence,
            expectedTransition);
    Check(transitionResult.valid && transitionResult.twoHandActive &&
            transitionTrace.bAccepted && transitionTrace.cValid &&
            transitionTrace.horizontalReleaseValid &&
            transitionTrace.horizontalReleaseInfluence > 0.0f &&
            transitionTrace.horizontalReleaseInfluence < 1.0f &&
            Near(transitionTrace.wEffective,
                transitionTrace.horizontalReleaseInfluence) &&
            transitionTrace.stockBlendSucceeded &&
            expectedTransitionValid &&
            Near(transitionTrace.finalDirection.x, expectedTransition.x) &&
            Near(transitionTrace.finalDirection.y, expectedTransition.y) &&
            Near(transitionTrace.finalDirection.z, expectedTransition.z) &&
            SamePosition(AimForward(transitionResult),
                {expectedTransition.x, expectedTransition.y,
                    expectedTransition.z}) &&
            SamePosition(transitionResult.pose.position,
                transition.right.position),
        "Horizontal-release tuning blends guarded B directly toward C with finite partial authority");

    XrVector3f previousForward{};
    bool havePrevious = false;
    bool continuous = true;
    for (int step = 0; step <= 40; ++step)
    {
        const float reach = 0.25f + static_cast<float>(step) * 0.005f;
        const AimPoseInputs sample = forHorizontal(makeBase(reach));
        const AimPoseResult result = ComputeAimPose(sample);
        const XrVector3f forward = AimForward(result);
        continuous = continuous && result.valid &&
            std::isfinite(forward.x) && std::isfinite(forward.y) &&
            std::isfinite(forward.z) &&
            SamePosition(result.pose.position, sample.right.position);
        if (havePrevious)
        {
            const float dx = forward.x - previousForward.x;
            const float dy = forward.y - previousForward.y;
            const float dz = forward.z - previousForward.z;
            continuous = continuous &&
                std::sqrt(dx * dx + dy * dy + dz * dz) < 0.05f;
        }
        previousForward = forward;
        havePrevious = true;
    }
    Check(continuous,
        "Horizontal-release tuning stays finite and continuous across both exact endpoints");

    AimPoseInputs yawChanged = transition;
    yawChanged.headOrientation = {
        0.0f, std::sin(0.7f), 0.0f, std::cos(0.7f)};
    const AimPoseResult yawChangedResult = ComputeAimPose(yawChanged);
    Check(SameAimResult(yawChangedResult, transitionResult, 2.0e-5f),
        "Horizontal release ignores HMD yaw when head and hand positions are unchanged");

    AimPoseInputs supportRotated = transition;
    supportRotated.left.orientation = {
        0.4f, 0.3f, 0.1f, 0.8602325f};
    Check(SameAimResult(
            ComputeAimPose(supportRotated), transitionResult, 2.0e-5f),
        "Horizontal release grants no authority to support-controller orientation");

    AimPoseInputs verticallyMoved = transition;
    verticallyMoved.right.position.y += 2.0f;
    AimPoseTrace verticalTrace{};
    ComputeAimPose(verticallyMoved, &verticalTrace);
    Check(verticalTrace.horizontalReleaseValid &&
            verticalTrace.rearHorizontalReachM ==
                transitionTrace.rearHorizontalReachM &&
            verticalTrace.horizontalReleaseInfluence ==
                transitionTrace.horizontalReleaseInfluence,
        "Horizontal release influence is invariant to vertical rear-hand movement");

    AimPoseInputs zeroStrength = closeHorizontal;
    zeroStrength.virtualStockStrength = 0.0f;
    AimPoseInputs zeroStrengthFree = zeroStrength;
    zeroStrengthFree.virtualStockEnabled = false;
    const AimPoseResult zeroStrengthResult = ComputeAimPose(zeroStrength);
    Check(SameAimResult(
            zeroStrengthResult, ComputeAimPose(zeroStrengthFree), 2.0e-5f),
        "Horizontal release cannot fabricate C when stock strength is zero");

    AimPoseInputs noHead = closeHorizontal;
    noHead.headValid = false;
    AimPoseInputs noHeadFree = noHead;
    noHeadFree.virtualStockEnabled = false;
    AimPoseTrace noHeadTrace{};
    const AimPoseResult noHeadResult = ComputeAimPose(noHead, &noHeadTrace);
    Check(SameAimResult(
            noHeadResult, ComputeAimPose(noHeadFree), 2.0e-5f) &&
            noHeadTrace.horizontalReleaseAttempted &&
            !noHeadTrace.horizontalReleaseValid &&
            !noHeadTrace.cValid && !noHeadTrace.wEffectiveValid,
        "Invalid HMD geometry fails safely to guarded free aim without C authority");

    AimPoseInputs malformed = transition;
    malformed.hybridHorizontalRearReleaseFullM = NAN;
    AimPoseInputs malformedFree = malformed;
    malformedFree.virtualStockEnabled = false;
    AimPoseTrace malformedTrace{};
    const AimPoseResult malformedResult = ComputeAimPose(
        malformed, &malformedTrace);
    Check(SameAimResult(
            malformedResult, ComputeAimPose(malformedFree), 2.0e-5f) &&
            malformedTrace.horizontalReleaseAttempted &&
            !malformedTrace.horizontalReleaseValid &&
            malformedTrace.wEffective == 0.0f,
        "Malformed enabled release thresholds fail to zero C authority");

    AimPoseInputs rejectedBase = makeBase(0.20f);
    rejectedBase.supportPosition = rejectedBase.right.position;
    const AimPoseInputs rejectedClose = forHorizontal(rejectedBase);
    const AimPoseInputs rejectedForceStock = forProfile(
        rejectedBase, VirtualStockTestProfile::F_HybridForceStockHead);
    Check(SameAimResult(
            ComputeAimPose(rejectedClose),
            ComputeAimPose(rejectedForceStock), 2.0e-5f),
        "Valid close C retains the existing ability to rescue rejected B");

    const AimPoseInputs isolationBase = EquivalenceInputs();
    bool existingProfilesUnchanged = true;
    for (size_t index = 0; index < 15; ++index)
    {
        const VirtualStockTestProfile profile =
            VirtualStockTestProfileDefinitionAt(index).id;
        AimPoseInputs baseline = forProfile(isolationBase, profile);
        const AimPoseResult expected = ComputeAimPose(baseline);
        existingProfilesUnchanged = existingProfilesUnchanged &&
            !baseline.hybridHorizontalRearReleaseEnabled;
        baseline.hybridHorizontalRearReleaseFullM = NAN;
        baseline.hybridHorizontalRearReleaseReleaseM = INFINITY;
        existingProfilesUnchanged = existingProfilesUnchanged &&
            ExactAimResult(ComputeAimPose(baseline), expected);
    }
    Check(existingProfilesUnchanged,
        "Custom and A-N ignore dormant horizontal release fields exactly");
}

void TestHybridDiagnosticState()
{
    HybridDiagnosticOverrideState state;
    Check(state.Load() == HybridDiagnosticOverride::Normal,
        "Hybrid diagnostic state starts in Normal");
    state.Store(HybridDiagnosticOverride::ForceHip);
    Check(state.Load() == HybridDiagnosticOverride::ForceHip,
        "Hybrid diagnostic state stores Force Hip at runtime");
    state.Store(HybridDiagnosticOverride::ForceStock);
    Check(state.Load() == HybridDiagnosticOverride::ForceStock,
        "Hybrid diagnostic state stores Force Stock at runtime");
    state.Store(static_cast<HybridDiagnosticOverride>(99));
    Check(state.Load() == HybridDiagnosticOverride::Normal,
        "Invalid Hybrid diagnostic state input normalizes to Normal");
    HybridDiagnosticOverrideState freshState;
    Check(freshState.Load() == HybridDiagnosticOverride::Normal,
        "A fresh Hybrid diagnostic state resets to Normal");
}

void CheckFixedStockEquivalence(
    int fixedReference, int hybridAdsReference, bool leftHanded,
    const char* message)
{
    AimPoseInputs fixed = EquivalenceInputs();
    fixed.virtualStockRearReference = fixedReference;
    fixed.virtualStockLeftHanded = leftHanded;
    const AimPoseResult fixedResult = ComputeAimPose(fixed);

    AimPoseInputs hybrid = fixed;
    hybrid.virtualStockRearReference = 3;
    hybrid.virtualStockHybridAdsReference = hybridAdsReference;
    hybrid.hybridDiagnosticOverride = HybridDiagnosticOverride::ForceStock;
    const AimPoseResult hybridResult = ComputeAimPose(hybrid);

    Check(SameAimResult(hybridResult, fixedResult, 2.0e-5f), message);
}

void TestHybridStockEquivalence()
{
    CheckFixedStockEquivalence(
        0, 0, false,
        "Hybrid Force Stock Head matches fixed Head with proximity disabled");
    CheckFixedStockEquivalence(
        1, 1, false,
        "Hybrid Force Stock Shoulder matches fixed Shoulder with proximity disabled");
    CheckFixedStockEquivalence(
        1, 1, true,
        "Hybrid Force Stock Shoulder preserves left-handed mirroring");
}

void TestHybridDiagnosticOverrides()
{
    AimPoseInputs inputs = HybridInputs();
    inputs.headPosition = {0.0f, 0.0f, 0.0f};
    inputs.hybridDiagnosticOverride = HybridDiagnosticOverride::ForceHip;
    const AimPoseResult forceHip = ComputeAimPose(inputs);
    virtual_stock::Point3 support{0.6f, 0.0f, -1.0f};
    virtual_stock::Point3 offhand{};
    bool rejectedExtreme = false;
    float rejectedAgreement = 0.0f;
    Check(virtual_stock::TryBuildAcceptedSupportDirection(
            {0.0f, 0.0f, 0.0f}, support, {0.0f, 0.0f, -1.0f},
            offhand, rejectedExtreme, rejectedAgreement),
        "Force Hip fixture accepts B");
    virtual_stock::Point3 expectedHip{};
    Check(virtual_stock::TryBlendDirectionAuthority(
            {0.0f, 0.0f, -1.0f}, offhand, 0.50f, expectedHip),
        "Force Hip fixture builds normal A/B HipAim");
    Check(forceHip.valid && forceHip.twoHandActive &&
            SamePosition(AimForward(forceHip),
                {expectedHip.x, expectedHip.y, expectedHip.z}),
        "Force Hip ignores natural W and leaves normal HipAim active");

    inputs = HybridInputs();
    inputs.headPosition = {0.0f, 0.0f, 0.0f};
    inputs.virtualStockHybridOffhandInfluence = 0.0f;
    inputs.right.orientation = {0.0f, std::sin(0.2f), 0.0f, std::cos(0.2f)};
    inputs.hybridDiagnosticOverride = HybridDiagnosticOverride::ForceHip;
    const AimPoseResult pureHip = ComputeAimPose(inputs);
    Check(pureHip.valid && pureHip.twoHandActive &&
            SameQuaternion(pureHip.pose.orientation, inputs.right.orientation),
        "Force Hip with zero influence preserves exact A and active presentation");

    inputs = HybridInputs();
    inputs.supportPosition = {1.0f, 0.0f, 0.0f};
    inputs.hybridDiagnosticOverride = HybridDiagnosticOverride::ForceHip;
    const AimPoseResult rejectedHip = ComputeAimPose(inputs);
    Check(rejectedHip.valid && !rejectedHip.twoHandActive &&
            SameQuaternion(rejectedHip.pose.orientation, inputs.right.orientation),
        "Force Hip with rejected B falls back to exact A and inactive presentation");

    inputs = HybridInputs();
    inputs.headPosition = {0.0f, 0.0f, 0.0f};
    inputs.hybridDiagnosticOverride = HybridDiagnosticOverride::ForceStock;
    const AimPoseResult forceStock = ComputeAimPose(inputs);
    const float supportLength = std::sqrt(1.36f);
    Check(forceStock.valid && forceStock.twoHandActive &&
            SamePosition(AimForward(forceStock),
                {0.6f / supportLength, 0.0f, -1.0f / supportLength}),
        "Force Stock gives valid C full final authority");

    inputs.virtualStockStrength = 0.0f;
    const AimPoseResult zeroStrengthStock = ComputeAimPose(inputs);
    Check(zeroStrengthStock.valid && zeroStrengthStock.twoHandActive &&
            SamePosition(AimForward(zeroStrengthStock),
                {expectedHip.x, expectedHip.y, expectedHip.z}),
        "Force Stock cannot manufacture C when Hybrid strength is zero");

    inputs = HybridInputs();
    inputs.headValid = false;
    inputs.hybridDiagnosticOverride = HybridDiagnosticOverride::ForceStock;
    const AimPoseResult noHeadStock = ComputeAimPose(inputs);
    Check(noHeadStock.valid && noHeadStock.twoHandActive &&
            SamePosition(AimForward(noHeadStock),
                {expectedHip.x, expectedHip.y, expectedHip.z}),
        "Force Stock falls back to HipAim when HMD is unavailable");

    inputs = HybridInputs();
    inputs.headPosition.x = NAN;
    inputs.hybridDiagnosticOverride = HybridDiagnosticOverride::ForceStock;
    const AimPoseResult invalidTargetStock = ComputeAimPose(inputs);
    Check(invalidTargetStock.valid && invalidTargetStock.twoHandActive &&
            SamePosition(AimForward(invalidTargetStock),
                {expectedHip.x, expectedHip.y, expectedHip.z}),
        "Force Stock falls back to HipAim for a non-finite target");

    inputs = HybridInputs();
    inputs.right.position = {0.2f, 0.0f, 0.0f};
    inputs.supportPosition = {0.2f, 0.0f, 0.0f};
    inputs.headPosition = {0.0f, 0.0f, 0.0f};
    inputs.virtualStockHybridOffhandInfluence = 0.0f;
    inputs.hybridDiagnosticOverride = HybridDiagnosticOverride::ForceStock;
    const AimPoseResult rejectedBStock = ComputeAimPose(inputs);
    Check(rejectedBStock.valid && rejectedBStock.twoHandActive &&
            SamePosition(AimForward(rejectedBStock), {1.0f, 0.0f, 0.0f}),
        "Force Stock lets valid C rescue a rejected B");

    inputs = HybridInputs();
    inputs.supportPosition = {1.0f, 0.0f, 0.0f};
    inputs.headPosition = {1.0f, 0.0f, 0.0f};
    inputs.hybridDiagnosticOverride = HybridDiagnosticOverride::ForceStock;
    const AimPoseResult invalidCStock = ComputeAimPose(inputs);
    Check(invalidCStock.valid && !invalidCStock.twoHandActive &&
            SameQuaternion(invalidCStock.pose.orientation, inputs.right.orientation),
        "Force Stock keeps rejected-B/invalid-C inactive at exact A");
}

void TestHybridDiagnosticModeIsolation()
{
    const HybridDiagnosticOverride overrides[] = {
        HybridDiagnosticOverride::ForceHip,
        HybridDiagnosticOverride::ForceStock};
    const int fixedModes[] = {0, 1, 2};
    for (const int mode : fixedModes)
    {
        AimPoseInputs baseline = EquivalenceInputs();
        baseline.virtualStockRearReference = mode;
        baseline.hybridDiagnosticOverride = HybridDiagnosticOverride::Normal;
        const AimPoseResult expected = ComputeAimPose(baseline);
        for (const HybridDiagnosticOverride diagnostic : overrides)
        {
            AimPoseInputs isolated = baseline;
            isolated.hybridDiagnosticOverride = diagnostic;
            const AimPoseResult actual = ComputeAimPose(isolated);
            Check(SameAimResult(actual, expected, 2.0e-5f),
                "Hybrid diagnostic override is ignored by fixed modes 0-2");
        }
    }

    AimPoseInputs stockOff = EquivalenceInputs();
    stockOff.virtualStockEnabled = false;
    stockOff.virtualStockRearReference = 0;
    const AimPoseResult expectedOff = ComputeAimPose(stockOff);
    for (const HybridDiagnosticOverride diagnostic : overrides)
    {
        stockOff.hybridDiagnosticOverride = diagnostic;
        Check(SameAimResult(ComputeAimPose(stockOff), expectedOff, 2.0e-5f),
            "Hybrid diagnostic override is ignored when Virtual Stock is off");
    }
}

bool SameVirtualStockSettings(
    const VirtualStockAimSettings& actual,
    const VirtualStockAimSettings& expected,
    float epsilon);

void CheckTraceEquivalence(const AimPoseInputs& inputs, const char* message)
{
    const AimPoseResult withoutTrace = ComputeAimPose(inputs, nullptr);
    AimPoseTrace trace{};
    const AimPoseResult withTrace = ComputeAimPose(inputs, &trace);
    Check(ExactAimResult(withTrace, withoutTrace), message);
}

void TestAimTraceObservationalEquivalence()
{
    AimPoseInputs inputs = EquivalenceInputs();
    inputs.rightValid = false;
    CheckTraceEquivalence(inputs,
        "Null and non-null traces are exactly equivalent for invalid primary input");

    inputs = EquivalenceInputs();
    inputs.twoHandEnabled = false;
    CheckTraceEquivalence(inputs,
        "Null and non-null traces are exactly equivalent for one-hand aim");

    inputs = EquivalenceInputs();
    inputs.virtualStockEnabled = false;
    CheckTraceEquivalence(inputs,
        "Null and non-null traces are exactly equivalent for legacy two-hand aim");

    inputs = EquivalenceInputs();
    inputs.virtualStockRearReference = 0;
    CheckTraceEquivalence(inputs,
        "Null and non-null traces are exactly equivalent for fixed Head aim");

    inputs.virtualStockRearReference = 1;
    CheckTraceEquivalence(inputs,
        "Null and non-null traces are exactly equivalent for fixed Shoulder aim");

    inputs.virtualStockRearReference = 2;
    CheckTraceEquivalence(inputs,
        "Null and non-null traces are exactly equivalent for fixed Chest aim");

    inputs = EquivalenceInputs();
    inputs.virtualStockRearReference = 3;
    CheckTraceEquivalence(inputs,
        "Null and non-null traces are exactly equivalent for normal Hybrid aim");

    inputs.hybridDiagnosticOverride = HybridDiagnosticOverride::ForceHip;
    CheckTraceEquivalence(inputs,
        "Null and non-null traces are exactly equivalent for Force Hip aim");
    inputs.hybridDiagnosticOverride = HybridDiagnosticOverride::ForceStock;
    CheckTraceEquivalence(inputs,
        "Null and non-null traces are exactly equivalent for Force Stock aim");

    inputs = EquivalenceInputs();
    ApplyVirtualStockAimSettings(
        inputs,
        MakeHybridHorizontalReleaseProfileSettings(0.270f, 0.425f),
        static_cast<VirtualStockTestProfile>(
            kVirtualStockExperimentProfileFirstId));
    CheckTraceEquivalence(inputs,
        "Null and non-null traces are exactly equivalent for horizontal-release aim");
}

void TestAimTraceSemantics()
{
    AimPoseInputs inputs = HybridInputs();
    inputs.right.position = {0.10f, 0.0f, -0.50f};
    inputs.supportPosition = {0.0f, 0.0f, -1.0f};
    inputs.headPosition = {0.0f, 0.0f, 0.0f};
    AimPoseTrace normal{};
    const AimPoseResult normalResult = ComputeAimPose(inputs, &normal);
    Check(normalResult.valid && normal.path == AimSolverPath::Hybrid &&
            normal.aValid && normal.bAttempted && normal.bAccepted &&
            normal.hipAimValid && normal.targetValid && normal.cAttempted &&
            normal.cValid && normal.virtualRearValid && normal.seatAttempted &&
            normal.seatValid && normal.seatClosestValid &&
            normal.wNaturalValid && normal.wAfterDiagnosticValid &&
            normal.wEffectiveValid &&
            normal.releaseGeometryValid && normal.finalDirectionValid &&
            normal.finalCalibrationValid,
        "Hybrid rich trace marks all valid geometry stages explicitly");
    Check(Near(normal.wNatural, 0.5f) &&
            Near(normal.wAfterDiagnostic, normal.wNatural) &&
            Near(normal.wEffective, normal.wNatural) &&
            !normal.overrideApplied &&
            !normal.horizontalReleaseAttempted &&
            !normal.horizontalReleaseValid,
        "Normal Hybrid trace keeps natural and effective seating authority equal");
    Check(Near(normal.seatSegmentLength, 1.0f) &&
            Near(normal.seatRawProjection, 0.5f) &&
            Near(normal.seatClampedProjection, 0.5f) &&
            Near(normal.seatErrorM, 0.1f) &&
            Near(normal.rearToStockTargetDistanceM,
                std::sqrt(0.26f)),
        "Hybrid rich trace reports independently checkable seat and release geometry");

    inputs.hybridDiagnosticOverride = HybridDiagnosticOverride::ForceHip;
    AimPoseTrace forceHip{};
    ComputeAimPose(inputs, &forceHip);
    Check(Near(forceHip.wNatural, 0.5f) &&
            forceHip.wAfterDiagnostic == 0.0f &&
            forceHip.wEffective == 0.0f &&
            forceHip.wNaturalValid && forceHip.wAfterDiagnosticValid &&
            forceHip.wEffectiveValid &&
            forceHip.overrideApplied && forceHip.forceStockEligible &&
            !forceHip.stockContributed,
        "Force Hip preserves natural W while exposing overridden effective W");

    inputs.hybridDiagnosticOverride = HybridDiagnosticOverride::ForceStock;
    AimPoseTrace forceStock{};
    ComputeAimPose(inputs, &forceStock);
    Check(Near(forceStock.wNatural, 0.5f) &&
            forceStock.wAfterDiagnostic == 1.0f &&
            forceStock.wEffective == 1.0f &&
            forceStock.wNaturalValid && forceStock.wAfterDiagnosticValid &&
            forceStock.wEffectiveValid &&
            forceStock.overrideApplied && forceStock.forceStockEligible &&
            forceStock.stockContributed,
        "Force Stock preserves natural W while exposing full effective W");

    inputs.headValid = false;
    AimPoseTrace unavailableStock{};
    ComputeAimPose(inputs, &unavailableStock);
    Check(!unavailableStock.targetValid && !unavailableStock.cAttempted &&
            !unavailableStock.cValid && !unavailableStock.seatAttempted &&
            !unavailableStock.wNaturalValid &&
            unavailableStock.wNatural == 0.0f &&
            !unavailableStock.wAfterDiagnosticValid &&
            unavailableStock.wAfterDiagnostic == 0.0f &&
            !unavailableStock.wEffectiveValid &&
            unavailableStock.wEffective == 0.0f &&
            !unavailableStock.overrideApplied &&
            !unavailableStock.forceStockEligible,
        "Invalid Hybrid stock geometry remains explicit zero/default trace data");

    inputs.hybridDiagnosticOverride = HybridDiagnosticOverride::ForceHip;
    AimPoseTrace unavailableForceHip{};
    ComputeAimPose(inputs, &unavailableForceHip);
    Check(!unavailableForceHip.wNaturalValid &&
            !unavailableForceHip.wAfterDiagnosticValid &&
            !unavailableForceHip.wEffectiveValid &&
            unavailableForceHip.wNatural == 0.0f &&
            unavailableForceHip.wAfterDiagnostic == 0.0f &&
            unavailableForceHip.wEffective == 0.0f &&
            !unavailableForceHip.overrideApplied,
        "Force Hip cannot claim an applied valid-zero weight when C is unavailable");

    inputs = HybridInputs();
    inputs.headValid = false;
    inputs.virtualStockHybridOffhandInfluence = 0.0f;
    AimPoseTrace exactA{};
    const AimPoseResult exactAResult = ComputeAimPose(inputs, &exactA);
    Check(exactAResult.valid && exactAResult.twoHandActive &&
            exactA.exactAEndpointSelected && !exactA.orientationRebuildAttempted &&
            SameQuaternion(exactAResult.pose.orientation, inputs.right.orientation),
        "Trace distinguishes the exact-A endpoint from an orientation rebuild");

    inputs = EquivalenceInputs();
    inputs.virtualStockRearReference = 1;
    inputs.virtualStockProximityRelease = true;
    inputs.virtualStockProximityFullM = 0.60f;
    inputs.virtualStockProximityReleaseM = 0.80f;
    AimPoseTrace fixed{};
    ComputeAimPose(inputs, &fixed);
    Check(fixed.path == AimSolverPath::FixedStock &&
            fixed.requestedTarget == AimStockTarget::Shoulder &&
            fixed.actualTarget == AimStockTarget::Shoulder &&
            fixed.fixedTargetValid && fixed.fixedRearDistanceValid &&
            fixed.fixedProximityEnabled && fixed.fixedProximityCalculated &&
            Near(fixed.fixedConfiguredStrength, inputs.virtualStockStrength) &&
            Near(fixed.fixedProximityInfluence, 1.0f) &&
            Near(fixed.fixedEffectiveStrength, inputs.virtualStockStrength) &&
            fixed.fixedDirectionValid &&
            fixed.orientationRebuildAttempted &&
            fixed.orientationRebuildSucceeded &&
            !fixed.exactAEndpointSelected,
        "Fixed Shoulder trace reports target, distance, proximity and direction stages");
}

void TestCounterfactualProfileIdentity()
{
    AimPoseInputs base = EquivalenceInputs();
    base.testProfileUsed = VirtualStockTestProfile::E_HybridMaxSeat;
    base.supportEndpointUsedGrip = true;
    base.supportGripPoseEnabled = true;
    base.twoHandToggle = false;
    const VirtualStockAimSettings originalSettings =
        VirtualStockAimSettingsFromAimPoseInputs(base);
    const VirtualStockTestProfile controls[] = {
        kVsOffControlProfile,
        kFixedHeadControlProfile,
        kFixedShoulderControlProfile,
    };
    for (const VirtualStockTestProfile profile : controls)
    {
        const AimPoseInputs selectedActual =
            AimPoseInputsForSelectedProfile(
                base, originalSettings, profile);
        const AimPoseResult actual = ComputeAimPose(selectedActual);

        const AimPoseInputs loggedCounterfactual =
            AimPoseInputsForProfile(base, profile);
        AimPoseTrace counterfactualTrace{};
        const AimPoseResult counterfactual =
            ComputeAimPose(loggedCounterfactual, &counterfactualTrace);
        Check(ExactAimResult(actual, counterfactual),
            "Selected A/B/C profile output exactly equals its same-frame counterfactual");
        Check(SameVirtualStockSettings(
                    VirtualStockAimSettingsFromAimPoseInputs(selectedActual),
                    VirtualStockAimSettingsFromAimPoseInputs(loggedCounterfactual),
                    0.0f) &&
                selectedActual.testProfileUsed == profile &&
                loggedCounterfactual.testProfileUsed == profile &&
                loggedCounterfactual.supportEndpointUsedGrip &&
                loggedCounterfactual.supportGripPoseEnabled &&
                !selectedActual.twoHandToggle &&
                !loggedCounterfactual.twoHandToggle,
            "Counterfactual input records explicit profile, acquisition mode and support-endpoint provenance");
    }

    Check(base.testProfileUsed == VirtualStockTestProfile::E_HybridMaxSeat &&
            base.supportEndpointUsedGrip && base.supportGripPoseEnabled &&
            SameVirtualStockSettings(
                VirtualStockAimSettingsFromAimPoseInputs(base), originalSettings,
                1.0e-6f),
        "Counterfactual solves leave the immutable canonical input packet unchanged");

    const AimPoseInputs a = AimPoseInputsForProfile(
        base, kVsOffControlProfile);
    const AimPoseInputs b = AimPoseInputsForProfile(
        base, kFixedHeadControlProfile);
    const AimPoseInputs c = AimPoseInputsForProfile(
        base, kFixedShoulderControlProfile);
    Check(!a.virtualStockEnabled && a.virtualStockRearReference == 0 &&
            b.virtualStockEnabled && b.virtualStockRearReference == 0 &&
            c.virtualStockEnabled && c.virtualStockRearReference == 1,
        "A/B/C controls resolve to deterministic VS OFF, fixed Head and fixed Shoulder settings");
}

void TestHybridInverseNeckIntegration()
{
    AimPoseInputs base = HybridInputs();
    base.right.position = {0.10f, 1.35f, 0.02f};
    base.supportPosition = {0.25f, 1.36f, -0.42f};
    base.headPosition = {0.0f, 1.60f, 0.0f};
    base.headOrientation = {0.0f, std::sin(0.30f), 0.0f, std::cos(0.30f)};
    base.inverseNeckNeutralValid = true;
    base.inverseNeckNeutralOrientation = {0.0f, 0.0f, 0.0f, 1.0f};
    base.inverseNeckNeutralCaptureSerial = 77;
    base.inverseNeckNeutralCaptureContactSpaceEpoch = 9;

    VirtualStockAimSettings acceptedY3 =
        MakeHybridHorizontalReleaseProfileSettings(0.330f, 0.475f);
    acceptedY3.virtualStockStrength = 0.80f;
    AimPoseInputs accepted = base;
    ApplyVirtualStockAimSettings(
        accepted, acceptedY3, VirtualStockTestProfile::Custom);
    AimPoseTrace acceptedTrace{};
    const AimPoseResult acceptedResult = ComputeAimPose(accepted, &acceptedTrace);

    AimPoseInputs nk0 = base;
    ApplyVirtualStockAimSettings(nk0,
        MakeHybridInverseNeckY3ProfileSettings(
            0.0f, 0.100f, 0.040f, 0.000f),
        static_cast<VirtualStockTestProfile>(
            kVirtualStockExperimentProfileFirstId));
    AimPoseTrace nk0Trace{};
    const AimPoseResult nk0Result = ComputeAimPose(nk0, &nk0Trace);
    Check(ExactAimResult(nk0Result, acceptedResult) &&
            nk0Trace.inverseNeckAttempted && nk0Trace.inverseNeckValid &&
            nk0Trace.rawHeadTargetValid &&
            nk0Trace.correctedHeadTargetValid &&
            nk0Trace.rawHeadTarget.x == nk0Trace.correctedHeadTarget.x &&
            nk0Trace.rawHeadTarget.y == nk0Trace.correctedHeadTarget.y &&
            nk0Trace.rawHeadTarget.z == nk0Trace.correctedHeadTarget.z,
        "NK0 is exact accepted Y3 output parity while auditing a zero correction");
    Check(nk0Trace.horizontalReleaseValid && acceptedTrace.horizontalReleaseValid &&
            nk0Trace.rearHorizontalReachM ==
                acceptedTrace.rearHorizontalReachM &&
            nk0Trace.horizontalReleaseInfluence ==
                acceptedTrace.horizontalReleaseInfluence,
        "inverse-neck leaves horizontal release on raw head-position XZ semantics");

    AimPoseInputs nk100 = base;
    ApplyVirtualStockAimSettings(nk100,
        MakeHybridInverseNeckY3ProfileSettings(
            1.0f, 0.100f, 0.040f, 0.000f),
        static_cast<VirtualStockTestProfile>(
            kVirtualStockExperimentProfileFirstId));
    AimPoseTrace nk100Trace{};
    const AimPoseResult nk100Result = ComputeAimPose(nk100, &nk100Trace);
    Check(nk100Result.valid && nk100Trace.inverseNeckValid &&
            nk100Trace.inverseNeckNeutralValid &&
            nk100Trace.inverseNeckNeutralCaptureSerial == 77 &&
            nk100Trace.inverseNeckNeutralCaptureContactSpaceEpoch == 9 &&
            !SamePosition(
                {nk100Trace.rawHeadTarget.x, nk100Trace.rawHeadTarget.y,
                 nk100Trace.rawHeadTarget.z},
                {nk100Trace.correctedHeadTarget.x,
                 nk100Trace.correctedHeadTarget.y,
                 nk100Trace.correctedHeadTarget.z}),
        "Hybrid C consumes the corrected Head target only with enabled valid neutral state");
    Check(nk100Trace.rearHorizontalReachM ==
            acceptedTrace.rearHorizontalReachM,
        "NK100 correction does not feed horizontal release geometry");

    AimPoseInputs invalidNeutral = nk100;
    invalidNeutral.inverseNeckNeutralValid = false;
    AimPoseTrace invalidTrace{};
    const AimPoseResult invalidResult = ComputeAimPose(
        invalidNeutral, &invalidTrace);
    Check(ExactAimResult(invalidResult, acceptedResult) &&
            invalidTrace.inverseNeckAttempted && !invalidTrace.inverseNeckValid &&
            !invalidTrace.inverseNeckNeutralValid,
        "missing neutral state fails cleanly to raw-HMD accepted Y3 behavior");

    AimPoseInputs disabled = accepted;
    disabled.inverseNeckNeutralValid = true;
    disabled.inverseNeckNeutralOrientation =
        base.inverseNeckNeutralOrientation;
    disabled.inverseNeckNeutralCaptureSerial = 77;
    Check(ExactAimResult(ComputeAimPose(disabled), acceptedResult),
        "current Head Y3 with inverse-neck disabled remains behavior-identical");

    AimPoseInputs shoulder = nk100;
    shoulder.virtualStockHybridAdsReference = 1;
    AimPoseInputs shoulderDisabled = shoulder;
    shoulderDisabled.hybridInverseNeckEnabled = false;
    AimPoseTrace shoulderTrace{};
    const AimPoseResult shoulderResult =
        ComputeAimPose(shoulder, &shoulderTrace);
    AimPoseTrace shoulderDisabledTrace{};
    const AimPoseResult shoulderDisabledResult =
        ComputeAimPose(shoulderDisabled, &shoulderDisabledTrace);
    Check(!ExactAimResult(shoulderResult, shoulderDisabledResult) &&
            shoulderTrace.inverseNeckAttempted &&
            shoulderTrace.inverseNeckValid,
        "Plus Shoulder consumes the corrected positional base when sway is ON");
    Check(shoulderTrace.horizontalReleaseValid &&
            shoulderDisabledTrace.horizontalReleaseValid &&
            shoulderTrace.rearHorizontalReachM ==
                shoulderDisabledTrace.rearHorizontalReachM &&
            shoulderTrace.horizontalReleaseInfluence ==
                shoulderDisabledTrace.horizontalReleaseInfluence,
        "Plus Shoulder correction preserves raw-HMD horizontal release timing");
    AimPoseInputs shoulderNoQ0 = shoulder;
    shoulderNoQ0.inverseNeckNeutralValid = false;
    Check(ExactAimResult(
            ComputeAimPose(shoulderNoQ0), shoulderDisabledResult),
        "Plus Shoulder without valid Q0 falls back to raw-HMD behaviour");

    AimPoseInputs oneHand = nk100;
    oneHand.twoHandEnabled = false;
    AimPoseInputs oneHandDisabled = oneHand;
    oneHandDisabled.hybridInverseNeckEnabled = false;
    AimPoseTrace oneHandTrace{};
    const AimPoseResult oneHandResult = ComputeAimPose(oneHand, &oneHandTrace);
    Check(ExactAimResult(
            oneHandResult, ComputeAimPose(oneHandDisabled)) &&
            !oneHandTrace.inverseNeckAttempted &&
            oneHandTrace.inverseNeckNeutralValid &&
            oneHandTrace.inverseNeckNeutralCaptureSerial == 77,
        "one-hand output remains unchanged while telemetry retains valid runtime Q0 provenance");
}

void TestPlantedStandardSwayMatrix()
{
    AimPoseInputs base = EquivalenceInputs();
    base.right.position = {0.10f, 1.35f, 0.02f};
    base.supportPosition = {0.25f, 1.36f, -0.42f};
    base.headPosition = {0.0f, 1.60f, 0.0f};
    base.headOrientation = {0.0f, std::sin(0.30f), 0.0f, std::cos(0.30f)};
    base.headValid = true;
    base.virtualStockEnabled = true;
    base.virtualStockStrength = 0.95f;
    base.virtualStockRearHeightM = -0.220f;
    base.virtualStockShoulderBackM = 0.005f;
    base.virtualStockShoulderSideM = 0.015f;
    base.virtualStockProximityRelease = false;
    base.virtualStockProximityFullM = 0.270f;
    base.virtualStockProximityReleaseM = 0.425f;
    base.hybridDiagnosticOverride = HybridDiagnosticOverride::Normal;
    base.inverseNeckNeutralValid = true;
    base.inverseNeckNeutralOrientation = {0.0f, 0.0f, 0.0f, 1.0f};
    base.inverseNeckNeutralCaptureSerial = 77;
    base.inverseNeckNeutralCaptureContactSpaceEpoch = 9;

    // Standard Centre OFF reproduces raw fixed behaviour.
    AimPoseInputs centreOff = base;
    centreOff.virtualStockRearReference = 0;
    centreOff.hybridInverseNeckEnabled = false;
    const AimPoseResult centreOffResult = ComputeAimPose(centreOff);
    // Standard Centre ON applies correction to the rear positional base.
    AimPoseInputs centreOn = centreOff;
    centreOn.hybridInverseNeckEnabled = true;
    centreOn.hybridInverseNeckStrength = 1.00f;
    centreOn.hybridInverseNeckForwardM = 0.100f;
    centreOn.hybridInverseNeckUpM = 0.040f;
    centreOn.hybridInverseNeckLateralM = 0.000f;
    AimPoseTrace centreOnTrace{};
    const AimPoseResult centreOnResult =
        ComputeAimPose(centreOn, &centreOnTrace);
    Check(centreOffResult.valid && centreOnResult.valid &&
            !ExactAimResult(centreOnResult, centreOffResult) &&
            centreOnTrace.inverseNeckAttempted &&
            centreOnTrace.inverseNeckValid,
        "Standard Centre sway ON corrects the rear base while staying valid");
    // Dormant thresholds do not alter planted Standard when disabled.
    AimPoseInputs centreVaried = centreOn;
    centreVaried.virtualStockProximityFullM = 0.31f;
    centreVaried.virtualStockProximityReleaseM = 0.57f;
    Check(ExactAimResult(ComputeAimPose(centreVaried), centreOnResult),
        "planted Standard Centre ignores dormant proximity thresholds");
    // Standard Shoulder OFF/ON with HMD yaw basis preserved.
    AimPoseInputs shoulderOff = centreOff;
    shoulderOff.virtualStockRearReference = 1;
    const AimPoseResult shoulderOffResult = ComputeAimPose(shoulderOff);
    AimPoseInputs shoulderOn = centreOn;
    shoulderOn.virtualStockRearReference = 1;
    AimPoseTrace shoulderOnTrace{};
    const AimPoseResult shoulderOnResult =
        ComputeAimPose(shoulderOn, &shoulderOnTrace);
    Check(shoulderOffResult.valid && shoulderOnResult.valid &&
            !ExactAimResult(shoulderOnResult, shoulderOffResult) &&
            shoulderOnTrace.inverseNeckAttempted &&
            shoulderOnTrace.inverseNeckValid,
        "Standard Shoulder sway ON corrects the positional base while staying valid");
    AimPoseInputs shoulderVaried = shoulderOn;
    shoulderVaried.virtualStockProximityFullM = 0.31f;
    shoulderVaried.virtualStockProximityReleaseM = 0.57f;
    Check(ExactAimResult(ComputeAimPose(shoulderVaried), shoulderOnResult),
        "planted Standard Shoulder ignores dormant proximity thresholds");
    // Invalid Q0 falls back to raw without dropout.
    AimPoseInputs centreNoQ0 = centreOn;
    centreNoQ0.inverseNeckNeutralValid = false;
    Check(ExactAimResult(ComputeAimPose(centreNoQ0), centreOffResult),
        "Standard Centre without valid Q0 falls back to raw-HMD behaviour");
    AimPoseInputs shoulderNoQ0 = shoulderOn;
    shoulderNoQ0.inverseNeckNeutralValid = false;
    Check(ExactAimResult(ComputeAimPose(shoulderNoQ0), shoulderOffResult),
        "Standard Shoulder without valid Q0 falls back to raw-HMD behaviour");
    // Planted Standard matches the Force-Stock oracle at .95 with sway OFF.
    CheckFixedStockEquivalence(0, 0, false,
        "planted Standard Centre matches the Force-Stock oracle");
    CheckFixedStockEquivalence(1, 1, false,
        "planted Standard Shoulder matches the Force-Stock oracle");
    CheckFixedStockEquivalence(1, 1, true,
        "planted Standard Shoulder preserves left-handed mirroring");
    // Exact zero preserves the legacy/no-stock path.
    AimPoseInputs zeroCentre = centreOff;
    zeroCentre.virtualStockStrength = 0.0f;
    AimPoseTrace zeroTrace{};
    const AimPoseResult zeroResult = ComputeAimPose(zeroCentre, &zeroTrace);
    Check(zeroResult.valid &&
            zeroTrace.path == AimSolverPath::LegacyTwoHand &&
            !zeroTrace.fixedDirectionValid,
        "Standard strength zero preserves the legacy no-stock path");
}

void TestDormantChestUnaffectedBySway()
{
    AimPoseInputs base = EquivalenceInputs();
    base.right.position = {0.10f, 1.35f, 0.02f};
    base.supportPosition = {0.25f, 1.36f, -0.42f};
    base.headPosition = {0.0f, 1.60f, 0.0f};
    base.headOrientation = {0.0f, std::sin(0.30f), 0.0f, std::cos(0.30f)};
    base.headValid = true;
    base.virtualStockEnabled = true;
    base.virtualStockStrength = 0.95f;
    base.virtualStockRearHeightM = -0.220f;
    base.virtualStockRearReference = 2;
    base.virtualStockChestHeightM = -0.320f;
    base.virtualStockChestBackM = 0.000f;
    base.virtualStockChestSideM = 0.015f;
    base.virtualStockProximityRelease = false;
    base.virtualStockProximityFullM = 0.270f;
    base.virtualStockProximityReleaseM = 0.425f;
    base.hybridDiagnosticOverride = HybridDiagnosticOverride::Normal;
    base.inverseNeckNeutralValid = true;
    base.inverseNeckNeutralOrientation = {0.0f, 0.0f, 0.0f, 1.0f};
    base.inverseNeckNeutralCaptureSerial = 77;
    base.inverseNeckNeutralCaptureContactSpaceEpoch = 9;

    // Dormant Chest with a valid Chest target: sway must not alter output.
    AimPoseInputs chestOff = base;
    chestOff.hybridInverseNeckEnabled = false;
    AimPoseTrace chestOffTrace{};
    const AimPoseResult chestOffResult =
        ComputeAimPose(chestOff, &chestOffTrace);
    AimPoseInputs chestOn = base;
    chestOn.hybridInverseNeckEnabled = true;
    chestOn.hybridInverseNeckStrength = 1.00f;
    chestOn.hybridInverseNeckForwardM = 0.100f;
    chestOn.hybridInverseNeckUpM = 0.040f;
    chestOn.hybridInverseNeckLateralM = 0.000f;
    AimPoseTrace chestOnTrace{};
    const AimPoseResult chestOnResult =
        ComputeAimPose(chestOn, &chestOnTrace);
    Check(chestOffResult.valid && chestOnResult.valid &&
            ExactAimResult(chestOnResult, chestOffResult) &&
            !chestOnTrace.inverseNeckAttempted,
        "dormant Chest output is identical with sway OFF vs ON");
    // Fallback edge: invalid Chest construction falls back to Head/Centre.
    // The fallback must remain raw-HMD with sway OFF vs ON.
    AimPoseInputs fallbackOff = chestOff;
    fallbackOff.virtualStockChestBackM =
        std::numeric_limits<float>::quiet_NaN();
    AimPoseTrace fallbackOffTrace{};
    const AimPoseResult fallbackOffResult =
        ComputeAimPose(fallbackOff, &fallbackOffTrace);
    AimPoseInputs fallbackOn = chestOn;
    fallbackOn.virtualStockChestBackM =
        std::numeric_limits<float>::quiet_NaN();
    AimPoseTrace fallbackOnTrace{};
    const AimPoseResult fallbackOnResult =
        ComputeAimPose(fallbackOn, &fallbackOnTrace);
    Check(fallbackOffResult.valid && fallbackOnResult.valid &&
            fallbackOffTrace.shoulderToHeadFallback ==
                fallbackOnTrace.shoulderToHeadFallback &&
            fallbackOnTrace.actualTarget == AimStockTarget::Head &&
            fallbackOffTrace.actualTarget == AimStockTarget::Head &&
            ExactAimResult(fallbackOnResult, fallbackOffResult) &&
            !fallbackOnTrace.inverseNeckAttempted,
        "invalid-Chest fallback remains raw-HMD with sway OFF vs ON");
}

bool SameVirtualStockSettings(
    const VirtualStockAimSettings& actual,
    const VirtualStockAimSettings& expected,
    float epsilon = 1.0e-6f)
{
    return actual.virtualStockEnabled == expected.virtualStockEnabled &&
        Near(actual.virtualStockStrength, expected.virtualStockStrength, epsilon) &&
        Near(actual.virtualStockRearHeightM, expected.virtualStockRearHeightM, epsilon) &&
        actual.virtualStockRearReference == expected.virtualStockRearReference &&
        Near(actual.virtualStockShoulderBackM, expected.virtualStockShoulderBackM, epsilon) &&
        Near(actual.virtualStockShoulderSideM, expected.virtualStockShoulderSideM, epsilon) &&
        Near(actual.virtualStockChestHeightM, expected.virtualStockChestHeightM, epsilon) &&
        Near(actual.virtualStockChestBackM, expected.virtualStockChestBackM, epsilon) &&
        Near(actual.virtualStockChestSideM, expected.virtualStockChestSideM, epsilon) &&
        Near(actual.virtualStockAdaptiveTopHeightM,
            expected.virtualStockAdaptiveTopHeightM, epsilon) &&
        Near(actual.virtualStockAdaptiveBottomHeightM,
            expected.virtualStockAdaptiveBottomHeightM, epsilon) &&
        Near(actual.virtualStockAdaptiveTopHalfWidthM,
            expected.virtualStockAdaptiveTopHalfWidthM, epsilon) &&
        Near(actual.virtualStockAdaptiveBottomHalfWidthM,
            expected.virtualStockAdaptiveBottomHalfWidthM, epsilon) &&
        Near(actual.virtualStockHybridOffhandInfluence,
            expected.virtualStockHybridOffhandInfluence, epsilon) &&
        actual.virtualStockHybridAdsReference ==
            expected.virtualStockHybridAdsReference &&
        Near(actual.virtualStockHybridSeatFullM,
            expected.virtualStockHybridSeatFullM, epsilon) &&
        Near(actual.virtualStockHybridSeatReleaseM,
            expected.virtualStockHybridSeatReleaseM, epsilon) &&
        actual.hybridHorizontalRearReleaseEnabled ==
            expected.hybridHorizontalRearReleaseEnabled &&
        Near(actual.hybridHorizontalRearReleaseFullM,
            expected.hybridHorizontalRearReleaseFullM, epsilon) &&
        Near(actual.hybridHorizontalRearReleaseReleaseM,
            expected.hybridHorizontalRearReleaseReleaseM, epsilon) &&
        actual.hybridInverseNeckEnabled ==
            expected.hybridInverseNeckEnabled &&
        Near(actual.hybridInverseNeckStrength,
            expected.hybridInverseNeckStrength, epsilon) &&
        Near(actual.hybridInverseNeckForwardM,
            expected.hybridInverseNeckForwardM, epsilon) &&
        Near(actual.hybridInverseNeckUpM,
            expected.hybridInverseNeckUpM, epsilon) &&
        Near(actual.hybridInverseNeckLateralM,
            expected.hybridInverseNeckLateralM, epsilon) &&
        actual.hybridDiagnosticOverride == expected.hybridDiagnosticOverride &&
        actual.virtualStockProximityRelease == expected.virtualStockProximityRelease &&
        Near(actual.virtualStockProximityFullM,
            expected.virtualStockProximityFullM, epsilon) &&
        Near(actual.virtualStockProximityReleaseM,
            expected.virtualStockProximityReleaseM, epsilon);
}

void CheckProfileSettings(VirtualStockTestProfile profile, bool enabled,
    int rearReference, HybridDiagnosticOverride diagnostic, int adsReference,
    float offhandInfluence, float seatFull, float seatRelease,
    bool proximityRelease, const char* message)
{
    const VirtualStockAimSettings actual = ResolveVirtualStockTestProfile(
        profile, CanonicalVirtualStockAimSettings());
    VirtualStockAimSettings expected = CanonicalVirtualStockAimSettings();
    expected.virtualStockEnabled = enabled;
    expected.virtualStockRearReference = rearReference;
    expected.hybridDiagnosticOverride = diagnostic;
    expected.virtualStockHybridAdsReference = adsReference;
    expected.virtualStockHybridOffhandInfluence = offhandInfluence;
    expected.virtualStockHybridSeatFullM = seatFull;
    expected.virtualStockHybridSeatReleaseM = seatRelease;
    expected.virtualStockProximityRelease = proximityRelease;
    Check(SameVirtualStockSettings(actual, expected),
        message);
}

void TestVirtualStockTestProfiles()
{
    struct HistoricalProfile
    {
        VirtualStockTestProfile profile;
        uint8_t id;
        const char* stableName;
        const char* uiLabel;
        VirtualStockTestProfileRole role;
    };
    constexpr HistoricalProfile historical[] = {
        {VirtualStockTestProfile::Custom, 0, "Custom", "Custom",
            VirtualStockTestProfileRole::None},
        {VirtualStockTestProfile::A_VsOffControl, 1, "A_VsOffControl",
            "A - VS OFF Control", VirtualStockTestProfileRole::VsOffControl},
        {VirtualStockTestProfile::B_FixedHeadControl, 2,
            "B_FixedHeadControl", "B - Fixed Head Control",
            VirtualStockTestProfileRole::FixedHeadControl},
        {VirtualStockTestProfile::C_FixedShoulderControl, 3,
            "C_FixedShoulderControl", "C - Fixed Shoulder Control",
            VirtualStockTestProfileRole::FixedShoulderControl},
        {VirtualStockTestProfile::D_HybridBaseline, 4, "D_HybridBaseline",
            "D - Hybrid Baseline", VirtualStockTestProfileRole::Tuning},
        {VirtualStockTestProfile::E_HybridMaxSeat, 5, "E_HybridMaxSeat",
            "E - Hybrid Max Seat", VirtualStockTestProfileRole::Tuning},
        {VirtualStockTestProfile::F_HybridForceStockHead, 6,
            "F_HybridForceStockHead", "F - Hybrid Force Stock Head",
            VirtualStockTestProfileRole::Diagnostic},
        {VirtualStockTestProfile::G_HybridForceStockShoulder, 7,
            "G_HybridForceStockShoulder", "G - Hybrid Force Stock Shoulder",
            VirtualStockTestProfileRole::Diagnostic},
        {VirtualStockTestProfile::H_HybridForceHip50, 8,
            "H_HybridForceHip50", "H - Hybrid Force Hip 50%",
            VirtualStockTestProfileRole::Diagnostic},
        {VirtualStockTestProfile::I_HybridForceHip100, 9,
            "I_HybridForceHip100", "I - Hybrid Force Hip 100%",
            VirtualStockTestProfileRole::Diagnostic},
    };
    constexpr std::array<VirtualStockExperimentProfileSpec, 0>
        emptyExperiments{};
    constexpr auto emptyRegistry =
        MergeVirtualStockTestProfiles(emptyExperiments);
    bool emptyRegistryMatchesCore =
        emptyRegistry.size() == kVirtualStockCoreTestProfileCount;
    for (size_t index = 0; index < emptyRegistry.size(); ++index)
    {
        emptyRegistryMatchesCore = emptyRegistryMatchesCore &&
            emptyRegistry[index].id ==
                kVirtualStockCoreTestProfileDefinitions[index].id &&
            VirtualStockTestProfileStringEqual(
                emptyRegistry[index].stableName,
                kVirtualStockCoreTestProfileDefinitions[index].stableName) &&
            VirtualStockTestProfileStringEqual(
                emptyRegistry[index].uiLabel,
                kVirtualStockCoreTestProfileDefinitions[index].uiLabel) &&
            emptyRegistry[index].role ==
                kVirtualStockCoreTestProfileDefinitions[index].role &&
            emptyRegistry[index].useCustomSettings ==
                kVirtualStockCoreTestProfileDefinitions[index].useCustomSettings &&
            SameVirtualStockSettings(
                emptyRegistry[index].settings,
                kVirtualStockCoreTestProfileDefinitions[index].settings);
    }
    Check(kVirtualStockCoreTestProfileCount == std::size(historical) + 5 &&
            kVirtualStockTestProfileCount ==
                kVirtualStockCoreTestProfileCount +
                    kVirtualStockExperimentProfileCount &&
            emptyRegistryMatchesCore,
        "Empty experiment merge is exactly the immutable Custom and A-N core registry");
    Check(kVirtualStockExperimentProfileCount == 2,
        "Final-composition experiment pack contains exactly AB0 and AB1");
    const auto ab0Profile = static_cast<VirtualStockTestProfile>(
        kVirtualStockExperimentProfileFirstId);
    const auto ab1Profile = static_cast<VirtualStockTestProfile>(
        kVirtualStockExperimentProfileFirstId + 1);
    const auto* ab0Definition =
        FindVirtualStockTestProfileDefinition(ab0Profile);
    const auto* ab1Definition =
        FindVirtualStockTestProfileDefinition(ab1Profile);
    const VirtualStockAimSettings ab0 = ResolveVirtualStockTestProfile(
        ab0Profile, CanonicalVirtualStockAimSettings());
    const VirtualStockAimSettings ab1 = ResolveVirtualStockTestProfile(
        ab1Profile, CanonicalVirtualStockAimSettings());
    Check(ab0Definition != nullptr && ab1Definition != nullptr &&
            std::strcmp(ab0Definition->stableName,
                "AB0_NK100_ForceStock_Control") == 0 &&
            std::strcmp(ab1Definition->stableName,
                "AB1_NK100_Production_Seat464_474") == 0 &&
            SameVirtualStockSettings(ab0,
                MakeHybridInverseNeckY3ProfileSettings(
                    1.00f, 0.100f, 0.040f, 0.000f)) &&
            SameVirtualStockSettings(ab1,
                MakeHybridInverseNeckY3ProductionCandidateSettings(
                    1.00f, 0.100f, 0.040f, 0.000f)),
        "AB0 remains exact NK100 ForceStock and AB1 resolves through the narrow production builder");
    VirtualStockAimSettings ab1AsControl = ab1;
    ab1AsControl.hybridDiagnosticOverride =
        HybridDiagnosticOverride::ForceStock;
    ab1AsControl.virtualStockHybridSeatFullM =
        ab0.virtualStockHybridSeatFullM;
    ab1AsControl.virtualStockHybridSeatReleaseM =
        ab0.virtualStockHybridSeatReleaseM;
    Check(ab0.hybridInverseNeckEnabled && ab1.hybridInverseNeckEnabled &&
            ab0.hybridDiagnosticOverride ==
                HybridDiagnosticOverride::ForceStock &&
            ab1.hybridDiagnosticOverride == HybridDiagnosticOverride::Normal &&
            Near(ab1.virtualStockHybridSeatFullM, 0.464f) &&
            Near(ab1.virtualStockHybridSeatReleaseM, 0.474f) &&
            SameVirtualStockSettings(ab0, ab1AsControl),
        "AB1 differs from AB0 only by diagnostic override and the 0.464/0.474 seat envelope");
    for (size_t index = 0; index < std::size(historical); ++index)
    {
        const HistoricalProfile& expected = historical[index];
        const auto* definition =
            FindVirtualStockTestProfileDefinition(expected.profile);
        Check(definition != nullptr &&
                static_cast<uint8_t>(definition->id) == expected.id &&
                std::strcmp(definition->stableName, expected.stableName) == 0 &&
                std::strcmp(definition->uiLabel, expected.uiLabel) == 0 &&
                definition->role == expected.role &&
                VirtualStockTestProfileDefinitionAt(index).id ==
                    expected.profile &&
                VirtualStockTestProfileIndex(expected.profile) == index,
            "Historical A-I profile ID, name, label, role and order are immutable");
    }
    Check(kVsOffControlProfile == VirtualStockTestProfile::A_VsOffControl &&
            kFixedHeadControlProfile ==
                VirtualStockTestProfile::B_FixedHeadControl &&
            kFixedShoulderControlProfile ==
                VirtualStockTestProfile::C_FixedShoulderControl &&
            VirtualStockTestProfileRoleCount(
                VirtualStockTestProfileRole::VsOffControl) == 1 &&
            VirtualStockTestProfileRoleCount(
                VirtualStockTestProfileRole::FixedHeadControl) == 1 &&
            VirtualStockTestProfileRoleCount(
                VirtualStockTestProfileRole::FixedShoulderControl) == 1,
        "Control roles resolve once to the immutable A/B/C profiles");
    Check(std::strcmp(VirtualStockTestProfileRoleName(
                VirtualStockTestProfileRole::VsOffControl),
                "vs_off_control") == 0 &&
            std::strcmp(VirtualStockTestProfileRoleName(
                VirtualStockTestProfileRole::FixedHeadControl),
                "fixed_head_control") == 0 &&
            std::strcmp(VirtualStockTestProfileRoleName(
                VirtualStockTestProfileRole::FixedShoulderControl),
                "fixed_shoulder_control") == 0,
        "Control roles expose stable telemetry names");
    Check(std::strcmp(
                VirtualStockTestProfileName(
                    VirtualStockTestProfile::E_HybridMaxSeat),
                "E_HybridMaxSeat") == 0 &&
            std::strcmp(
                VirtualStockTestProfileLabel(
                    VirtualStockTestProfile::E_HybridMaxSeat),
                "E - Hybrid Max Seat") == 0,
        "Profile E has a stable max-seat name rather than a historical reproduction name");
    CheckProfileSettings(VirtualStockTestProfile::A_VsOffControl,
        false, 0, HybridDiagnosticOverride::Normal, 0, 0.50f,
        0.050f, 0.150f, false,
        "Profile A resolves to Virtual Stock OFF");
    CheckProfileSettings(VirtualStockTestProfile::B_FixedHeadControl,
        true, 0, HybridDiagnosticOverride::Normal, 0, 0.50f,
        0.050f, 0.150f, true,
        "Profile B resolves to fixed Head with legacy proximity enabled");
    CheckProfileSettings(VirtualStockTestProfile::C_FixedShoulderControl,
        true, 1, HybridDiagnosticOverride::Normal, 0, 0.50f,
        0.050f, 0.150f, true,
        "Profile C resolves to fixed Shoulder with legacy proximity enabled");
    CheckProfileSettings(VirtualStockTestProfile::D_HybridBaseline,
        true, 3, HybridDiagnosticOverride::Normal, 0, 0.50f,
        0.050f, 0.150f, false,
        "Profile D resolves to the canonical Hybrid baseline");
    CheckProfileSettings(VirtualStockTestProfile::E_HybridMaxSeat,
        true, 3, HybridDiagnosticOverride::Normal, 0, 0.50f,
        0.600f, 0.800f, false,
        "Profile E resolves to the maximum-valid 0.600/0.800 metre max-seat condition");
    CheckProfileSettings(VirtualStockTestProfile::F_HybridForceStockHead,
        true, 3, HybridDiagnosticOverride::ForceStock, 0, 0.50f,
        0.050f, 0.150f, false,
        "Profile F resolves to Hybrid Force Stock Head");
    CheckProfileSettings(VirtualStockTestProfile::G_HybridForceStockShoulder,
        true, 3, HybridDiagnosticOverride::ForceStock, 1, 0.50f,
        0.050f, 0.150f, false,
        "Profile G resolves to Hybrid Force Stock Shoulder");
    CheckProfileSettings(VirtualStockTestProfile::H_HybridForceHip50,
        true, 3, HybridDiagnosticOverride::ForceHip, 0, 0.50f,
        0.050f, 0.150f, false,
        "Profile H resolves to Hybrid Force Hip at 50 percent influence");
    CheckProfileSettings(VirtualStockTestProfile::I_HybridForceHip100,
        true, 3, HybridDiagnosticOverride::ForceHip, 0, 1.00f,
        0.050f, 0.150f, false,
        "Profile I resolves to Hybrid Force Hip at 100 percent influence");

    struct TuningProfile
    {
        VirtualStockTestProfile profile;
        uint8_t id;
        const char* stableName;
        const char* uiLabel;
        float seatFullM;
    };
    constexpr TuningProfile tuning[] = {
        {VirtualStockTestProfile::J_HybridSeat_464_474, 10,
            "J_HybridSeat_464_474", "J - Hybrid Seat 0.464 / 0.474", 0.464f},
        {VirtualStockTestProfile::K_HybridSeat_440_474, 11,
            "K_HybridSeat_440_474", "K - Hybrid Seat 0.440 / 0.474", 0.440f},
        {VirtualStockTestProfile::L_HybridSeat_420_474, 12,
            "L_HybridSeat_420_474", "L - Hybrid Seat 0.420 / 0.474", 0.420f},
        {VirtualStockTestProfile::M_HybridSeat_400_474, 13,
            "M_HybridSeat_400_474", "M - Hybrid Seat 0.400 / 0.474", 0.400f},
        {VirtualStockTestProfile::N_HybridSeat_380_474, 14,
            "N_HybridSeat_380_474", "N - Hybrid Seat 0.380 / 0.474", 0.380f},
    };
    const VirtualStockAimSettings tuningReference =
        ResolveVirtualStockTestProfile(
            VirtualStockTestProfile::J_HybridSeat_464_474,
            CanonicalVirtualStockAimSettings());
    for (size_t index = 0; index < std::size(tuning); ++index)
    {
        const TuningProfile& expectedProfile = tuning[index];
        const auto* definition = FindVirtualStockTestProfileDefinition(
            expectedProfile.profile);
        Check(definition != nullptr &&
                static_cast<uint8_t>(definition->id) == expectedProfile.id &&
                std::strcmp(definition->stableName,
                    expectedProfile.stableName) == 0 &&
                std::strcmp(definition->uiLabel,
                    expectedProfile.uiLabel) == 0 &&
                definition->role == VirtualStockTestProfileRole::Tuning &&
                !definition->useCustomSettings &&
                VirtualStockTestProfileIndex(expectedProfile.profile) ==
                    std::size(historical) + index,
            "J-N have append-only IDs, stable metadata and tuning roles");
        CheckProfileSettings(expectedProfile.profile,
            true, 3, HybridDiagnosticOverride::Normal, 0, 0.50f,
            expectedProfile.seatFullM, 0.474f, false,
            "J-N inherit canonical Hybrid settings and declare only seat thresholds");

        VirtualStockAimSettings expectedSettings = tuningReference;
        expectedSettings.virtualStockHybridSeatFullM =
            expectedProfile.seatFullM;
        const VirtualStockAimSettings actualSettings =
            ResolveVirtualStockTestProfile(
                expectedProfile.profile, CanonicalVirtualStockAimSettings());
        Check(SameVirtualStockSettings(actualSettings, expectedSettings) &&
                actualSettings.virtualStockHybridSeatFullM >=
                    kVirtualStockHybridSeatFullMinimumM &&
                actualSettings.virtualStockHybridSeatFullM <=
                    kVirtualStockHybridSeatFullMaximumM &&
                actualSettings.virtualStockHybridSeatReleaseM >=
                    kVirtualStockHybridSeatReleaseMinimumM &&
                actualSettings.virtualStockHybridSeatReleaseM <=
                    kVirtualStockHybridSeatReleaseMaximumM &&
                actualSettings.virtualStockHybridSeatReleaseM -
                        actualSettings.virtualStockHybridSeatFullM >=
                    kVirtualStockHybridSeatMinimumSeparationM,
            "J-N vary only seat-full while retaining one valid 0.474 release threshold");
    }

    constexpr std::array syntheticExperiments = {
        VirtualStockExperimentProfileSpec{
            "Temporary_Seat_360_474",
            "Temporary - Seat 0.360 / 0.474",
            MakeHybridTuningProfileSettings(0.360f, 0.474f)},
        VirtualStockExperimentProfileSpec{
            "Temporary_Horizontal_300_450",
            "Temporary - Horizontal 0.300 / 0.450",
            MakeHybridHorizontalReleaseProfileSettings(0.300f, 0.450f)},
    };
    constexpr auto syntheticRegistry =
        MergeVirtualStockTestProfiles(syntheticExperiments);
    constexpr auto firstTemporary = static_cast<VirtualStockTestProfile>(
        kVirtualStockExperimentProfileFirstId);
    constexpr auto secondTemporary = static_cast<VirtualStockTestProfile>(
        kVirtualStockExperimentProfileFirstId + 1);
    const auto* firstTemporaryDefinition =
        FindVirtualStockTestProfileDefinitionInRegistry(
            syntheticRegistry, firstTemporary);
    const auto* secondTemporaryDefinition =
        FindVirtualStockTestProfileDefinitionInRegistry(
            syntheticRegistry, secondTemporary);
    Check(VirtualStockTestProfileRegistryIsValid(
              syntheticRegistry, kVirtualStockCoreTestProfileCount) &&
            syntheticRegistry.size() == kVirtualStockCoreTestProfileCount + 2 &&
            firstTemporaryDefinition != nullptr &&
            secondTemporaryDefinition != nullptr &&
            std::strcmp(firstTemporaryDefinition->stableName,
                "Temporary_Seat_360_474") == 0 &&
            std::strcmp(secondTemporaryDefinition->stableName,
                "Temporary_Horizontal_300_450") == 0 &&
            firstTemporaryDefinition->role ==
                VirtualStockTestProfileRole::Tuning &&
            secondTemporaryDefinition->role ==
                VirtualStockTestProfileRole::Tuning &&
            !firstTemporaryDefinition->useCustomSettings &&
            !secondTemporaryDefinition->useCustomSettings,
        "Synthetic experiment rows receive contiguous IDs 128-129 and forced tuning metadata");
    Check(SameVirtualStockSettings(
              firstTemporaryDefinition->settings,
              MakeHybridTuningProfileSettings(0.360f, 0.474f)) &&
            SameVirtualStockSettings(
                secondTemporaryDefinition->settings,
                MakeHybridHorizontalReleaseProfileSettings(0.300f, 0.450f)),
        "Synthetic experiment merge preserves production-built settings exactly");

    constexpr std::array duplicateNames = {
        VirtualStockExperimentProfileSpec{
            "Duplicate", "First", MakeHybridProfileSettings()},
        VirtualStockExperimentProfileSpec{
            "Duplicate", "Second", MakeHybridProfileSettings()},
    };
    constexpr std::array emptyName = {
        VirtualStockExperimentProfileSpec{
            "", "Empty name", MakeHybridProfileSettings()},
    };
    constexpr std::array emptyLabel = {
        VirtualStockExperimentProfileSpec{
            "Empty_Label", "", MakeHybridProfileSettings()},
    };
    constexpr std::array duplicateLabels = {
        VirtualStockExperimentProfileSpec{
            "First_Label", "Duplicate label", MakeHybridProfileSettings()},
        VirtualStockExperimentProfileSpec{
            "Second_Label", "Duplicate label", MakeHybridProfileSettings()},
    };
    constexpr std::array coreCollision = {
        VirtualStockExperimentProfileSpec{
            "D_HybridBaseline", "Core collision", MakeHybridProfileSettings()},
    };
    constexpr std::array coreLabelCollision = {
        VirtualStockExperimentProfileSpec{
            "Core_Label_Collision", "D - Hybrid Baseline",
            MakeHybridProfileSettings()},
    };
    constexpr std::array badScalar = {
        VirtualStockExperimentProfileSpec{
            "Bad_Scalar", "Bad scalar", [] {
                auto settings = MakeHybridProfileSettings();
                settings.virtualStockStrength = 2.0f;
                return settings;
            }()},
    };
    constexpr std::array badSeat = {
        VirtualStockExperimentProfileSpec{
            "Bad_Seat", "Bad seat", MakeHybridTuningProfileSettings(0.40f, 0.40f)},
    };
    constexpr std::array badHorizontal = {
        VirtualStockExperimentProfileSpec{
            "Bad_Horizontal", "Bad horizontal",
            MakeHybridHorizontalReleaseProfileSettings(0.45f, 0.30f)},
    };
    constexpr std::array badProximity = {
        VirtualStockExperimentProfileSpec{
            "Bad_Proximity", "Bad proximity", [] {
                auto settings = MakeHybridProfileSettings();
                settings.virtualStockProximityFullM = 0.50f;
                settings.virtualStockProximityReleaseM = 0.40f;
                return settings;
            }()},
    };
    Check(!VirtualStockExperimentProfileNamesAreUnique(duplicateNames) &&
            !VirtualStockExperimentProfileNamesAreNonEmpty(emptyName) &&
            !VirtualStockExperimentProfileLabelsAreNonEmpty(emptyLabel) &&
            !VirtualStockExperimentProfileLabelsAreUnique(duplicateLabels) &&
            !VirtualStockExperimentProfileNamesDoNotCollideWithCore(
                coreCollision) &&
            !VirtualStockExperimentProfileLabelsDoNotCollideWithCore(
                coreLabelCollision) &&
            !VirtualStockExperimentScalarSettingsAreValid(badScalar) &&
            !VirtualStockExperimentSettingIsInRange(
                badScalar, &VirtualStockAimSettings::virtualStockStrength,
                kVirtualStockStrengthMinimum, kVirtualStockStrengthMaximum) &&
            !VirtualStockExperimentSeatSettingsAreValid(badSeat) &&
            !VirtualStockExperimentSettingSeparationIsValid(
                badSeat,
                &VirtualStockAimSettings::virtualStockHybridSeatFullM,
                &VirtualStockAimSettings::virtualStockHybridSeatReleaseM,
                kVirtualStockHybridSeatMinimumSeparationM) &&
            !VirtualStockExperimentHorizontalReleaseSettingsAreValid(
                badHorizontal) &&
            !VirtualStockExperimentSettingSeparationIsValid(
                badHorizontal,
                &VirtualStockAimSettings::hybridHorizontalRearReleaseFullM,
                &VirtualStockAimSettings::hybridHorizontalRearReleaseReleaseM,
                kVirtualStockProximityMinimumSeparationM) &&
            !VirtualStockExperimentProximitySettingsAreValid(badProximity) &&
            !VirtualStockExperimentSettingSeparationIsValid(
                badProximity,
                &VirtualStockAimSettings::virtualStockProximityFullM,
                &VirtualStockAimSettings::virtualStockProximityReleaseM,
                kVirtualStockProximityMinimumSeparationM) &&
            !VirtualStockExperimentProfileCountIsValid(
                kVirtualStockExperimentProfileCapacity + 1),
        "Hostile experiment packs are rejected by focused compile-time predicates");
    Check(NormalizeVirtualStockTestProfile(15) ==
              VirtualStockTestProfile::Custom &&
            NormalizeVirtualStockTestProfile(127) ==
              VirtualStockTestProfile::Custom &&
            NormalizeVirtualStockTestProfile(255) ==
              VirtualStockTestProfile::Custom,
        "Reserved and invalid profile IDs normalize to Custom");
    const auto firstConfiguredTemporary = static_cast<VirtualStockTestProfile>(
        kVirtualStockExperimentProfileFirstId);
    Check(kVirtualStockExperimentProfileCount != 0
            ? FindVirtualStockTestProfileDefinition(firstConfiguredTemporary) !=
                  nullptr &&
                NormalizeVirtualStockTestProfile(
                    kVirtualStockExperimentProfileFirstId) ==
                    firstConfiguredTemporary
            : NormalizeVirtualStockTestProfile(
                  kVirtualStockExperimentProfileFirstId) ==
                  VirtualStockTestProfile::Custom,
        "Configured temporary ID 128 resolves, while an empty pack leaves it invalid");

    const VirtualStockAimSettings maxSeat = ResolveVirtualStockTestProfile(
        VirtualStockTestProfile::E_HybridMaxSeat,
        CanonicalVirtualStockAimSettings());
    Check(maxSeat.virtualStockHybridSeatFullM ==
                kVirtualStockHybridSeatFullMaximumM &&
            maxSeat.virtualStockHybridSeatReleaseM ==
                kVirtualStockHybridSeatReleaseMaximumM &&
            maxSeat.virtualStockHybridSeatReleaseM -
                    maxSeat.virtualStockHybridSeatFullM >=
                kVirtualStockHybridSeatMinimumSeparationM,
        "Profile E is exactly the maximum-seat 0.600/0.800 stress condition");

    VirtualStockAimSettings custom = CanonicalVirtualStockAimSettings();
    custom.virtualStockEnabled = true;
    custom.virtualStockStrength = 0.37f;
    custom.virtualStockRearHeightM = -0.11f;
    custom.virtualStockRearReference = 2;
    custom.virtualStockShoulderBackM = 0.17f;
    custom.virtualStockShoulderSideM = 0.18f;
    custom.virtualStockChestHeightM = -0.41f;
    custom.virtualStockChestBackM = 0.13f;
    custom.virtualStockChestSideM = 0.19f;
    custom.virtualStockAdaptiveTopHeightM = -0.21f;
    custom.virtualStockAdaptiveBottomHeightM = -0.51f;
    custom.virtualStockAdaptiveTopHalfWidthM = 0.09f;
    custom.virtualStockAdaptiveBottomHalfWidthM = 0.18f;
    custom.virtualStockHybridOffhandInfluence = 0.73f;
    custom.virtualStockHybridAdsReference = 1;
    custom.virtualStockHybridSeatFullM = 0.08f;
    custom.virtualStockHybridSeatReleaseM = 0.22f;
    custom.hybridHorizontalRearReleaseFullM = 0.29f;
    custom.hybridHorizontalRearReleaseReleaseM = 0.51f;
    custom.hybridInverseNeckEnabled = true;
    custom.hybridInverseNeckStrength = 0.62f;
    custom.hybridInverseNeckForwardM = 0.12f;
    custom.hybridInverseNeckUpM = 0.03f;
    custom.hybridInverseNeckLateralM = -0.02f;
    custom.hybridDiagnosticOverride = HybridDiagnosticOverride::ForceStock;
    custom.virtualStockProximityRelease = true;
    custom.virtualStockProximityFullM = 0.31f;
    custom.virtualStockProximityReleaseM = 0.57f;
    Check(SameVirtualStockSettings(
            ResolveVirtualStockTestProfile(
                VirtualStockTestProfile::Custom, custom), custom),
        "Custom resolves immediately back to every ordinary user setting");
    Check(SameVirtualStockSettings(
            ResolveVirtualStockTestProfile(
                static_cast<VirtualStockTestProfile>(99), custom), custom),
        "Invalid profile values normalize to Custom settings");

    VirtualStockTestProfileState state;
    Check(state.Load() == VirtualStockTestProfile::Custom,
        "A fresh test-profile state starts in Custom");
    state.Store(VirtualStockTestProfile::A_VsOffControl);
    Check(state.Load() == VirtualStockTestProfile::A_VsOffControl,
        "Test-profile state stores a runtime profile");
    state.Store(static_cast<VirtualStockTestProfile>(99));
    Check(state.Load() == VirtualStockTestProfile::Custom,
        "Invalid test-profile state normalizes to Custom");
    VirtualStockTestProfileState freshState;
    Check(freshState.Load() == VirtualStockTestProfile::Custom,
        "A fresh process-equivalent test-profile state resets to Custom");

    state.Store(VirtualStockTestProfile::E_HybridMaxSeat);
    const VirtualStockAimSettings activeMaxSeat =
        ResolveVirtualStockTestProfile(state.Load(), custom);
    Check(activeMaxSeat.virtualStockRearReference == 3 &&
            Near(activeMaxSeat.virtualStockHybridSeatFullM, 0.600f) &&
            Near(activeMaxSeat.virtualStockHybridSeatReleaseM, 0.800f),
        "Selecting a profile resolves effective settings without changing Custom values");
    state.Store(VirtualStockTestProfile::Custom);
    Check(SameVirtualStockSettings(
            ResolveVirtualStockTestProfile(
                state.Load(), custom), custom),
        "Switching the runtime profile back to Custom restores user settings");
}
}

int main()
{
    TestHybridMode3();
    TestHybridEndpointsAndFallbacks();
    TestHybridTargetsAndCalibration();
    TestHybridIgnoresLegacyProximity();
    TestHybridHorizontalReleaseComposition();
    TestHybridDiagnosticState();
    TestHybridStockEquivalence();
    TestHybridDiagnosticOverrides();
    TestHybridDiagnosticModeIsolation();
    TestAimTraceObservationalEquivalence();
    TestAimTraceSemantics();
    TestCounterfactualProfileIdentity();
    TestHybridInverseNeckIntegration();
    TestPlantedStandardSwayMatrix();
    TestDormantChestUnaffectedBySway();
    TestVirtualStockTestProfiles();
    std::printf("Production Hybrid aim-pose fixture: %u checks, %u failures\n",
        checks, failures);
    return failures ? 1 : 0;
}
