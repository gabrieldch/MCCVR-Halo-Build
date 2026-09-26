#pragma once

#include <cstdint>
#include <type_traits>

enum class AimSolverPath : uint8_t
{
    None = 0,
    OneHand,
    LegacyTwoHand,
    FixedStock,
    Hybrid,
};

enum class AimStockTarget : uint8_t
{
    None = 0,
    Head,
    Shoulder,
    Chest,
};

struct AimTraceVec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct AimTraceQuat
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

struct AimPoseTrace
{
    AimSolverPath path = AimSolverPath::None;

    bool aValid = false;
    AimTraceQuat primaryQuaternion{};
    AimTraceVec3 primaryDirection{};

    bool bAttempted = false;
    bool bAccepted = false;
    AimTraceVec3 bDirection{};
    float bAgreement = 0.0f;
    bool bExtremeRejected = false;
    float bRejectedAgreement = 0.0f;

    bool hipBlendAttempted = false;
    bool hipBlendSucceeded = false;
    bool hipAimValid = false;
    AimTraceVec3 hipAimDirection{};
    float offhandInfluenceUsed = 0.0f;
    bool exactAEndpointSelected = false;

    AimStockTarget requestedTarget = AimStockTarget::None;
    AimStockTarget actualTarget = AimStockTarget::None;
    bool shoulderToHeadFallback = false;
    bool targetValid = false;
    AimTraceVec3 target{};
    bool inverseNeckAttempted = false;
    bool inverseNeckValid = false;
    bool inverseNeckNeutralValid = false;
    float inverseNeckStrength = 0.0f;
    AimTraceQuat inverseNeckNeutralOrientation{};
    uint64_t inverseNeckNeutralCaptureSerial = 0;
    uint64_t inverseNeckNeutralCaptureContactSpaceEpoch = 0;
    AimTraceVec3 inverseNeckCurrentOffset{};
    AimTraceVec3 inverseNeckNeutralOffset{};
    AimTraceVec3 inverseNeckPredictedOrbit{};
    bool inverseNeckCorrectionClamped = false;
    AimTraceVec3 inverseNeckAppliedCorrection{};
    AimTraceVec3 inverseNeckCorrectedHeadPosition{};
    bool rawHeadTargetValid = false;
    AimTraceVec3 rawHeadTarget{};
    bool correctedHeadTargetValid = false;
    AimTraceVec3 correctedHeadTarget{};
    bool cAttempted = false;
    bool cValid = false;
    bool virtualRearValid = false;
    AimTraceVec3 virtualRear{};
    AimTraceVec3 cDirection{};
    float stockStrengthUsed = 0.0f;

    bool seatAttempted = false;
    bool seatValid = false;
    float seatSegmentLength = 0.0f;
    float seatRawProjection = 0.0f;
    float seatClampedProjection = 0.0f;
    bool seatClosestValid = false;
    AimTraceVec3 seatClosest{};
    float seatErrorM = 0.0f;
    bool wNaturalValid = false;
    float wNatural = 0.0f;
    bool wAfterDiagnosticValid = false;
    float wAfterDiagnostic = 0.0f;

    bool horizontalReleaseAttempted = false;
    bool horizontalReleaseValid = false;
    float rearHorizontalReachM = 0.0f;
    float horizontalReleaseInfluence = 0.0f;

    bool releaseGeometryValid = false;
    float rearToStockTargetDistanceM = 0.0f;
    float primaryToSupportDistanceM = 0.0f;
    float stockToSupportDistanceM = 0.0f;

    uint8_t effectiveDiagnosticOverride = 0;
    bool wEffectiveValid = false;
    float wEffective = 0.0f;
    bool overrideApplied = false;
    bool forceStockEligible = false;
    bool stockBlendAttempted = false;
    bool stockBlendSucceeded = false;
    bool stockContributed = false;
    bool finalDirectionValid = false;
    AimTraceVec3 finalDirection{};
    bool orientationRebuildAttempted = false;
    bool orientationRebuildSucceeded = false;
    bool finalCalibrationValid = false;

    bool fixedTargetValid = false;
    AimTraceVec3 fixedTarget{};
    bool fixedRearDistanceValid = false;
    float fixedRearToTargetDistanceM = 0.0f;
    bool fixedProximityEnabled = false;
    bool fixedProximityCalculated = false;
    float fixedProximityInfluence = 0.0f;
    float fixedConfiguredStrength = 0.0f;
    float fixedEffectiveStrength = 0.0f;
    bool fixedDirectionValid = false;
};

static_assert(std::is_trivially_copyable_v<AimPoseTrace>);
