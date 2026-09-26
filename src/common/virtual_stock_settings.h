#pragma once

#include "virtual_stock_diagnostic.h"

// Production-owned Virtual Stock defaults and validation bounds. Config,
// runtime aim construction, geometry helpers, and diagnostic profiles all
// consume this neutral settings seam.
inline constexpr bool kVirtualStockEnabledDefault = false;
inline constexpr float kVirtualStockStrengthDefault = 0.95f;
inline constexpr float kVirtualStockProductStrengthDefault = 0.80f;
inline constexpr float kVirtualStockStandardStrengthDefault =
    kVirtualStockStrengthDefault;
inline constexpr float kVirtualStockPlusStrengthDefault =
    kVirtualStockProductStrengthDefault;
inline constexpr float kVirtualStockStrengthMinimum = 0.0f;
inline constexpr float kVirtualStockStrengthMaximum = 1.0f;
inline constexpr float kVirtualStockRearHeightDefaultM = -0.220f;
inline constexpr float kVirtualStockRearHeightMinimumM = -0.30f;
inline constexpr float kVirtualStockRearHeightMaximumM = 0.10f;
inline constexpr int kVirtualStockRearReferenceDefault = 0;
inline constexpr int kVirtualStockProductRearReferenceDefault = 3;
inline constexpr int kVirtualStockRearReferenceMinimum = 0;
inline constexpr int kVirtualStockRearReferenceMaximum = 3;
inline constexpr float kVirtualStockShoulderBackDefaultM = 0.005f;
inline constexpr float kVirtualStockShoulderBackMinimumM = 0.0f;
inline constexpr float kVirtualStockShoulderBackMaximumM = 0.25f;
inline constexpr float kVirtualStockShoulderSideDefaultM = 0.015f;
inline constexpr float kVirtualStockShoulderSideMinimumM = 0.0f;
inline constexpr float kVirtualStockShoulderSideMaximumM = 0.20f;
inline constexpr float kVirtualStockChestHeightDefaultM = -0.320f;
inline constexpr float kVirtualStockChestHeightMinimumM = -0.50f;
inline constexpr float kVirtualStockChestHeightMaximumM = -0.220f;
inline constexpr float kVirtualStockChestBackDefaultM = 0.000f;
inline constexpr float kVirtualStockChestBackMinimumM = 0.0f;
inline constexpr float kVirtualStockChestBackMaximumM = 0.25f;
inline constexpr float kVirtualStockChestSideDefaultM = 0.015f;
inline constexpr float kVirtualStockChestSideMinimumM = 0.0f;
inline constexpr float kVirtualStockChestSideMaximumM = 0.20f;
inline constexpr float kVirtualStockAdaptiveTopHeightDefaultM = -0.180f;
inline constexpr float kVirtualStockAdaptiveTopHeightMinimumM = -0.35f;
inline constexpr float kVirtualStockAdaptiveTopHeightMaximumM = 0.0f;
inline constexpr float kVirtualStockAdaptiveBottomHeightDefaultM = -0.450f;
inline constexpr float kVirtualStockAdaptiveBottomHeightMinimumM = -0.65f;
inline constexpr float kVirtualStockAdaptiveBottomHeightMaximumM = -0.20f;
inline constexpr float kVirtualStockAdaptiveTopHalfWidthDefaultM = 0.080f;
inline constexpr float kVirtualStockAdaptiveTopHalfWidthMinimumM = 0.02f;
inline constexpr float kVirtualStockAdaptiveTopHalfWidthMaximumM = 0.25f;
inline constexpr float kVirtualStockAdaptiveBottomHalfWidthDefaultM = 0.140f;
inline constexpr float kVirtualStockAdaptiveBottomHalfWidthMinimumM = 0.02f;
inline constexpr float kVirtualStockAdaptiveBottomHalfWidthMaximumM = 0.30f;
inline constexpr float kVirtualStockHybridOffhandInfluenceDefault = 0.50f;
inline constexpr float kVirtualStockHybridOffhandInfluenceMinimum = 0.0f;
inline constexpr float kVirtualStockHybridOffhandInfluenceMaximum = 1.0f;
inline constexpr int kVirtualStockHybridAdsReferenceDefault = 0;
inline constexpr int kVirtualStockHybridAdsReferenceMinimum = 0;
inline constexpr int kVirtualStockHybridAdsReferenceMaximum = 1;
inline constexpr float kVirtualStockHybridSeatFullDefaultM = 0.050f;
inline constexpr float kVirtualStockProductHybridSeatFullDefaultM = 0.464f;
inline constexpr float kVirtualStockHybridSeatFullMinimumM = 0.010f;
inline constexpr float kVirtualStockHybridSeatFullMaximumM = 0.600f;
inline constexpr float kVirtualStockHybridSeatReleaseDefaultM = 0.150f;
inline constexpr float kVirtualStockProductHybridSeatReleaseDefaultM = 0.474f;
inline constexpr float kVirtualStockHybridSeatReleaseMinimumM = 0.020f;
inline constexpr float kVirtualStockHybridSeatReleaseMaximumM = 0.800f;
inline constexpr float kVirtualStockHybridSeatMinimumSeparationM = 0.010f;
inline constexpr float kVirtualStockHybridSeatComparisonEpsilon = 1.0e-6f;
inline constexpr bool kVirtualStockProximityReleaseDefault = true;
inline constexpr float kVirtualStockProximityFullDefaultM = 0.270f;
inline constexpr float kVirtualStockProximityFullMinimumM = 0.10f;
inline constexpr float kVirtualStockProximityFullMaximumM = 0.60f;
inline constexpr float kVirtualStockProximityReleaseDefaultM = 0.425f;
inline constexpr float kVirtualStockProximityReleaseMinimumM = 0.15f;
inline constexpr float kVirtualStockProximityReleaseMaximumM = 0.80f;
inline constexpr float kVirtualStockProximityMinimumSeparationM = 0.010f;
inline constexpr bool kVirtualStockHybridHorizontalRearReleaseEnabledDefault = false;
inline constexpr float kVirtualStockHybridHorizontalRearReleaseFullDefaultM =
    kVirtualStockProximityFullDefaultM;
inline constexpr float kVirtualStockHybridHorizontalRearReleaseReleaseDefaultM =
    kVirtualStockProximityReleaseDefaultM;
inline constexpr bool kVirtualStockProductHybridHorizontalReleaseDefault = true;
inline constexpr float kVirtualStockProductHybridHorizontalFullDefaultM = 0.330f;
inline constexpr float kVirtualStockProductHybridHorizontalReleaseDefaultM = 0.475f;
inline constexpr float kVirtualStockHybridHorizontalRearReleaseMinimumSeparationM =
    0.010f;
inline constexpr bool kVirtualStockHybridInverseNeckEnabledDefault = false;
inline constexpr float kVirtualStockHybridInverseNeckStrengthDefault = 0.0f;
inline constexpr float kVirtualStockHybridInverseNeckStrengthMinimum = 0.0f;
inline constexpr float kVirtualStockHybridInverseNeckStrengthMaximum = 1.0f;
inline constexpr float kVirtualStockHybridInverseNeckForwardDefaultM = 0.0f;
inline constexpr float kVirtualStockHybridInverseNeckForwardMinimumM = 0.0f;
inline constexpr float kVirtualStockHybridInverseNeckForwardMaximumM = 0.30f;
inline constexpr float kVirtualStockHybridInverseNeckUpDefaultM = 0.0f;
inline constexpr float kVirtualStockHybridInverseNeckUpMinimumM = -0.30f;
inline constexpr float kVirtualStockHybridInverseNeckUpMaximumM = 0.30f;
inline constexpr float kVirtualStockHybridInverseNeckLateralDefaultM = 0.0f;
inline constexpr float kVirtualStockHybridInverseNeckLateralMinimumM = -0.30f;
inline constexpr float kVirtualStockHybridInverseNeckLateralMaximumM = 0.30f;
inline constexpr float kVirtualStockHybridInverseNeckCorrectionCapM = 0.15f;
inline constexpr bool kVirtualStockHeadTurnSwayCorrectionDefault = true;
inline constexpr float kVirtualStockProductInverseNeckStrength = 1.00f;
inline constexpr float kVirtualStockProductNeckForwardM = 0.100f;
inline constexpr float kVirtualStockProductNeckUpM = 0.040f;
inline constexpr float kVirtualStockProductNeckLateralM = 0.000f;
inline constexpr bool kTwoHandSupportGripPoseDefault = true;

struct VirtualStockAimSettings
{
    bool virtualStockEnabled = kVirtualStockEnabledDefault;
    float virtualStockStrength = kVirtualStockStrengthDefault;
    float virtualStockRearHeightM = kVirtualStockRearHeightDefaultM;
    int virtualStockRearReference = kVirtualStockRearReferenceDefault;
    float virtualStockShoulderBackM = kVirtualStockShoulderBackDefaultM;
    float virtualStockShoulderSideM = kVirtualStockShoulderSideDefaultM;
    float virtualStockChestHeightM = kVirtualStockChestHeightDefaultM;
    float virtualStockChestBackM = kVirtualStockChestBackDefaultM;
    float virtualStockChestSideM = kVirtualStockChestSideDefaultM;
    float virtualStockAdaptiveTopHeightM =
        kVirtualStockAdaptiveTopHeightDefaultM;
    float virtualStockAdaptiveBottomHeightM =
        kVirtualStockAdaptiveBottomHeightDefaultM;
    float virtualStockAdaptiveTopHalfWidthM =
        kVirtualStockAdaptiveTopHalfWidthDefaultM;
    float virtualStockAdaptiveBottomHalfWidthM =
        kVirtualStockAdaptiveBottomHalfWidthDefaultM;
    float virtualStockHybridOffhandInfluence =
        kVirtualStockHybridOffhandInfluenceDefault;
    int virtualStockHybridAdsReference = kVirtualStockHybridAdsReferenceDefault;
    float virtualStockHybridSeatFullM = kVirtualStockHybridSeatFullDefaultM;
    float virtualStockHybridSeatReleaseM =
        kVirtualStockHybridSeatReleaseDefaultM;
    bool hybridHorizontalRearReleaseEnabled =
        kVirtualStockHybridHorizontalRearReleaseEnabledDefault;
    float hybridHorizontalRearReleaseFullM =
        kVirtualStockHybridHorizontalRearReleaseFullDefaultM;
    float hybridHorizontalRearReleaseReleaseM =
        kVirtualStockHybridHorizontalRearReleaseReleaseDefaultM;
    bool hybridInverseNeckEnabled =
        kVirtualStockHybridInverseNeckEnabledDefault;
    float hybridInverseNeckStrength =
        kVirtualStockHybridInverseNeckStrengthDefault;
    float hybridInverseNeckForwardM =
        kVirtualStockHybridInverseNeckForwardDefaultM;
    float hybridInverseNeckUpM = kVirtualStockHybridInverseNeckUpDefaultM;
    float hybridInverseNeckLateralM =
        kVirtualStockHybridInverseNeckLateralDefaultM;
    HybridDiagnosticOverride hybridDiagnosticOverride =
        HybridDiagnosticOverride::Normal;
    bool virtualStockProximityRelease = kVirtualStockProximityReleaseDefault;
    float virtualStockProximityFullM = kVirtualStockProximityFullDefaultM;
    float virtualStockProximityReleaseM =
        kVirtualStockProximityReleaseDefaultM;
};

inline constexpr VirtualStockAimSettings CanonicalVirtualStockAimSettings() noexcept
{
    return {};
}
