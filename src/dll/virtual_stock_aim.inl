// Virtual-stock productised semantics re-hosted from the donor final state
// (Standard/Plus product modes, donor defaults, Hybrid mode 3 with neck/seat
// neutral-capture state). Included inside vr.cpp's anonymous namespace after
// CurrentAimPoseInputs. The default-off path keeps the legacy controller solver
// shape; this file never moves result.pose.position.

    struct ResolvedVirtualStockAimSettings
    {
        VirtualStockTestProfile profile = VirtualStockTestProfile::Custom;
        VirtualStockAimSettings settings{};
    };

    ResolvedVirtualStockAimSettings
    CurrentResolvedVirtualStockAimSettings() noexcept
    {
        const VirtualStockAimSettings userSettings =
            VirtualStockAimSettingsFromConfig(
                g_config, g_hybridDiagnosticOverride.Load());
        ResolvedVirtualStockAimSettings resolved{};
        resolved.profile = g_virtualStockTestProfile.Load();
        resolved.settings = ResolveVirtualStockTestProfile(
            resolved.profile, userSettings);
        return resolved;
    }

    VirtualStockAimSettings CurrentEffectiveVirtualStockAimSettings() noexcept
    {
        return CurrentResolvedVirtualStockAimSettings().settings;
    }

    void ApplyVirtualStockAimSettings(
        AimPoseInputs& inputs, const VirtualStockAimSettings& settings,
        VirtualStockTestProfile profile) noexcept
    {
        inputs.testProfileUsed = profile;
        inputs.virtualStockEnabled = settings.virtualStockEnabled;
        inputs.virtualStockStrength = settings.virtualStockStrength;
        inputs.virtualStockRearHeightM = settings.virtualStockRearHeightM;
        inputs.virtualStockRearReference = settings.virtualStockRearReference;
        inputs.virtualStockShoulderBackM = settings.virtualStockShoulderBackM;
        inputs.virtualStockShoulderSideM = settings.virtualStockShoulderSideM;
        inputs.virtualStockChestHeightM = settings.virtualStockChestHeightM;
        inputs.virtualStockChestBackM = settings.virtualStockChestBackM;
        inputs.virtualStockChestSideM = settings.virtualStockChestSideM;
        inputs.virtualStockAdaptiveTopHeightM =
            settings.virtualStockAdaptiveTopHeightM;
        inputs.virtualStockAdaptiveBottomHeightM =
            settings.virtualStockAdaptiveBottomHeightM;
        inputs.virtualStockAdaptiveTopHalfWidthM =
            settings.virtualStockAdaptiveTopHalfWidthM;
        inputs.virtualStockAdaptiveBottomHalfWidthM =
            settings.virtualStockAdaptiveBottomHalfWidthM;
        inputs.virtualStockHybridOffhandInfluence =
            settings.virtualStockHybridOffhandInfluence;
        inputs.virtualStockHybridAdsReference =
            settings.virtualStockHybridAdsReference;
        inputs.virtualStockHybridSeatFullM =
            settings.virtualStockHybridSeatFullM;
        inputs.virtualStockHybridSeatReleaseM =
            settings.virtualStockHybridSeatReleaseM;
        inputs.hybridHorizontalRearReleaseEnabled =
            settings.hybridHorizontalRearReleaseEnabled;
        inputs.hybridHorizontalRearReleaseFullM =
            settings.hybridHorizontalRearReleaseFullM;
        inputs.hybridHorizontalRearReleaseReleaseM =
            settings.hybridHorizontalRearReleaseReleaseM;
        inputs.hybridInverseNeckEnabled = settings.hybridInverseNeckEnabled;
        inputs.hybridInverseNeckStrength = settings.hybridInverseNeckStrength;
        inputs.hybridInverseNeckForwardM = settings.hybridInverseNeckForwardM;
        inputs.hybridInverseNeckUpM = settings.hybridInverseNeckUpM;
        inputs.hybridInverseNeckLateralM = settings.hybridInverseNeckLateralM;
        inputs.hybridDiagnosticOverride = settings.hybridDiagnosticOverride;
        inputs.virtualStockProximityRelease =
            settings.virtualStockProximityRelease;
        inputs.virtualStockProximityFullM = settings.virtualStockProximityFullM;
        inputs.virtualStockProximityReleaseM =
            settings.virtualStockProximityReleaseM;
    }

    VirtualStockAimSettings VirtualStockAimSettingsFromAimPoseInputs(
        const AimPoseInputs& inputs) noexcept
    {
        VirtualStockAimSettings settings{};
        settings.virtualStockEnabled = inputs.virtualStockEnabled;
        settings.virtualStockStrength = inputs.virtualStockStrength;
        settings.virtualStockRearHeightM = inputs.virtualStockRearHeightM;
        settings.virtualStockRearReference = inputs.virtualStockRearReference;
        settings.virtualStockShoulderBackM = inputs.virtualStockShoulderBackM;
        settings.virtualStockShoulderSideM = inputs.virtualStockShoulderSideM;
        settings.virtualStockChestHeightM = inputs.virtualStockChestHeightM;
        settings.virtualStockChestBackM = inputs.virtualStockChestBackM;
        settings.virtualStockChestSideM = inputs.virtualStockChestSideM;
        settings.virtualStockAdaptiveTopHeightM =
            inputs.virtualStockAdaptiveTopHeightM;
        settings.virtualStockAdaptiveBottomHeightM =
            inputs.virtualStockAdaptiveBottomHeightM;
        settings.virtualStockAdaptiveTopHalfWidthM =
            inputs.virtualStockAdaptiveTopHalfWidthM;
        settings.virtualStockAdaptiveBottomHalfWidthM =
            inputs.virtualStockAdaptiveBottomHalfWidthM;
        settings.virtualStockHybridOffhandInfluence =
            inputs.virtualStockHybridOffhandInfluence;
        settings.virtualStockHybridAdsReference =
            inputs.virtualStockHybridAdsReference;
        settings.virtualStockHybridSeatFullM =
            inputs.virtualStockHybridSeatFullM;
        settings.virtualStockHybridSeatReleaseM =
            inputs.virtualStockHybridSeatReleaseM;
        settings.hybridHorizontalRearReleaseEnabled =
            inputs.hybridHorizontalRearReleaseEnabled;
        settings.hybridHorizontalRearReleaseFullM =
            inputs.hybridHorizontalRearReleaseFullM;
        settings.hybridHorizontalRearReleaseReleaseM =
            inputs.hybridHorizontalRearReleaseReleaseM;
        settings.hybridInverseNeckEnabled = inputs.hybridInverseNeckEnabled;
        settings.hybridInverseNeckStrength = inputs.hybridInverseNeckStrength;
        settings.hybridInverseNeckForwardM = inputs.hybridInverseNeckForwardM;
        settings.hybridInverseNeckUpM = inputs.hybridInverseNeckUpM;
        settings.hybridInverseNeckLateralM = inputs.hybridInverseNeckLateralM;
        settings.hybridDiagnosticOverride = inputs.hybridDiagnosticOverride;
        settings.virtualStockProximityRelease =
            inputs.virtualStockProximityRelease;
        settings.virtualStockProximityFullM =
            inputs.virtualStockProximityFullM;
        settings.virtualStockProximityReleaseM =
            inputs.virtualStockProximityReleaseM;
        return settings;
    }

    AimPoseInputs AimPoseInputsForProfile(
        const AimPoseInputs& base,
        VirtualStockTestProfile profile) noexcept
    {
        AimPoseInputs resolved = base;
        const VirtualStockAimSettings settings =
            ResolveVirtualStockTestProfile(
                profile, VirtualStockAimSettingsFromAimPoseInputs(base));
        ApplyVirtualStockAimSettings(resolved, settings, profile);
        return resolved;
    }

    AimPoseInputs AimPoseInputsForSelectedProfile(
        const AimPoseInputs& base,
        const VirtualStockAimSettings& userSettings,
        VirtualStockTestProfile selectedProfile) noexcept
    {
        AimPoseInputs resolved = base;
        ApplyVirtualStockAimSettings(
            resolved,
            ResolveVirtualStockTestProfile(selectedProfile, userSettings),
            selectedProfile);
        return resolved;
    }

    // Stock-aware primary-aim constructor. Reads the live F1/config toggle on
    // every call, so switching virtual stock while engaged takes effect
    // immediately without unlatching or re-grabbing. Callers pass a head pose
    // that is coherent with this exact controller sample (same prepared frame
    // under g_headCs, or the frame thread right after both captures).
    AimPoseInputs CurrentStockAimPoseInputs(
        bool rightValid, const XrPosef& right,
        bool leftValid, const XrPosef& left,
        bool coherentHeadValid, const XrVector3f& coherentHeadPosition,
        const XrQuaternionf& coherentHeadOrientation,
        bool supportGripPositionValid,
        const XrVector3f& supportGripPosition) noexcept
    {
        AimPoseInputs inputs = CurrentAimPoseInputs(rightValid, right, leftValid, left);
        const VirtualStockAimSettings userSettings =
            VirtualStockAimSettingsFromConfig(
                g_config, g_hybridDiagnosticOverride.Load());
        inputs = AimPoseInputsForSelectedProfile(
            inputs, userSettings, g_virtualStockTestProfile.Load());
        inputs.virtualStockLeftHanded =
            g_capturedLeftHanded.load(std::memory_order_acquire);
        inputs.headValid = coherentHeadValid;
        inputs.headPosition = coherentHeadPosition;
        inputs.headOrientation = coherentHeadOrientation;
        inputs.inverseNeckNeutralValid =
            g_inverseNeckNeutralCapture.neutralValid;
        inputs.inverseNeckNeutralOrientation = {
            g_inverseNeckNeutralCapture.neutralOrientation.x,
            g_inverseNeckNeutralCapture.neutralOrientation.y,
            g_inverseNeckNeutralCapture.neutralOrientation.z,
            g_inverseNeckNeutralCapture.neutralOrientation.w};
        inputs.inverseNeckNeutralCaptureSerial =
            g_inverseNeckNeutralCapture.captureSerial;
        inputs.inverseNeckNeutralCaptureContactSpaceEpoch =
            g_inverseNeckNeutralCapture.captureContactSpaceEpoch;
        const auto toPoint = [](const XrVector3f& v) {
            return virtual_stock::Point3{v.x, v.y, v.z};
        };
        const virtual_stock::SupportEndpointSelection support =
            virtual_stock::SelectTwoHandSupportEndpoint(
                g_config.two_hand_support_grip_pose, toPoint(left.position),
                supportGripPositionValid, toPoint(supportGripPosition));
        inputs.supportPosition = XrVector3f{
            support.position.x, support.position.y, support.position.z};
        inputs.supportEndpointUsedGrip = support.usedGrip;
        inputs.supportGripPoseEnabled = g_config.two_hand_support_grip_pose;
        return inputs;
    }

    template <bool CaptureTrace>
    AimPoseResult ComputeAimPoseImpl(
        const AimPoseInputs& inputs, AimPoseTrace* trace) noexcept
    {
        if constexpr (CaptureTrace)
            *trace = {};
        if constexpr (CaptureTrace)
        {
            virtual_stock::Quat4 neutralOrientation{};
            trace->inverseNeckNeutralValid =
                inputs.hybridInverseNeckEnabled &&
                inputs.inverseNeckNeutralValid &&
                virtual_stock::TryNormalizeQuaternion(
                    {inputs.inverseNeckNeutralOrientation.x,
                     inputs.inverseNeckNeutralOrientation.y,
                     inputs.inverseNeckNeutralOrientation.z,
                     inputs.inverseNeckNeutralOrientation.w},
                    neutralOrientation);
            trace->inverseNeckStrength = std::isfinite(
                inputs.hybridInverseNeckStrength)
                ? std::clamp(inputs.hybridInverseNeckStrength, 0.0f, 1.0f)
                : 0.0f;
            if (trace->inverseNeckNeutralValid)
            {
                trace->inverseNeckNeutralOrientation = {
                    neutralOrientation.x, neutralOrientation.y,
                    neutralOrientation.z, neutralOrientation.w};
                trace->inverseNeckNeutralCaptureSerial =
                    inputs.inverseNeckNeutralCaptureSerial;
                trace->inverseNeckNeutralCaptureContactSpaceEpoch =
                    inputs.inverseNeckNeutralCaptureContactSpaceEpoch;
            }
        }
        AimPoseResult result{};
        if (!inputs.rightValid)
            return result;

        const auto traceVec = [](const virtual_stock::Point3& value) {
            return AimTraceVec3{value.x, value.y, value.z};
        };
        const auto traceXrVec = [](const XrVector3f& value) {
            return AimTraceVec3{value.x, value.y, value.z};
        };
        if constexpr (CaptureTrace)
        {
            trace->aValid = true;
            trace->primaryQuaternion = {
                inputs.right.orientation.x, inputs.right.orientation.y,
                inputs.right.orientation.z, inputs.right.orientation.w};
            trace->primaryDirection = traceXrVec(
                Rotate(inputs.right.orientation, {0.0f, 0.0f, -1.0f}));
            trace->finalDirectionValid = true;
            trace->finalDirection = trace->primaryDirection;
        }

        result.updateTwoHandActivity = true;
        result.pose = inputs.right;

        auto finishAimPose = [&]() {
            auto multiply = [](const XrQuaternionf& a,
                               const XrQuaternionf& b) {
                return XrQuaternionf{
                    a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
                    a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
                    a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w,
                    a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z};
            };
            constexpr float kDegToRad = 0.01745329252f;
            const float yaw = inputs.gunYawDeg * kDegToRad;
            const float pitch = inputs.gunPitchDeg * kDegToRad;
            const float roll = inputs.gunRollDeg * kDegToRad;
            const XrQuaternionf qYaw{
                0.0f, sinf(yaw*0.5f), 0.0f, cosf(yaw*0.5f)};
            const XrQuaternionf qPitch{
                sinf(pitch*0.5f), 0.0f, 0.0f, cosf(pitch*0.5f)};
            const XrQuaternionf qRoll{
                0.0f, 0.0f, sinf(-roll*0.5f), cosf(roll*0.5f)};
            const XrQuaternionf corrected = multiply(
                result.pose.orientation,
                multiply(multiply(qYaw, qPitch), qRoll));
            const float length = sqrtf(
                corrected.x*corrected.x + corrected.y*corrected.y +
                corrected.z*corrected.z + corrected.w*corrected.w);
            if (!std::isfinite(length) || length < 1e-5f)
                return;
            result.pose.orientation = {
                corrected.x/length, corrected.y/length,
                corrected.z/length, corrected.w/length};
            result.valid = true;
            if constexpr (CaptureTrace)
                trace->finalCalibrationValid = true;
        };

        if (!inputs.twoHandEnabled || !inputs.leftValid ||
            !inputs.twoHandLatched)
        {
            if constexpr (CaptureTrace)
            {
                trace->path = AimSolverPath::OneHand;
                trace->exactAEndpointSelected = true;
            }
            finishAimPose();
            return result;
        }

        // Two-hand direction selection and orientation construction live in
        // the virtual_stock helper so unit tests exercise the production
        // maths. Legacy controller->controller behavior and thresholds are
        // unchanged. A valid positive-strength stock ray bypasses the
        // primary-forward agreement test; a degenerate stock ray falls back
        // to the legacy line. Base position stays the primary controller
        // throughout: only result.pose.orientation is reassigned below,
        // never result.pose.position.
        const XrVector3f supportEndpoint = inputs.supportPosition;
        const XrQuaternionf rq = inputs.right.orientation;
        const XrVector3f rp = inputs.right.position;
        const XrVector3f rup = Rotate(rq, {0,1,0});
        const XrVector3f rawForward = Rotate(rq, {0,0,-1});
        const auto toPoint = [](const XrVector3f& v) {
            return virtual_stock::Point3{v.x, v.y, v.z};
        };
        if (inputs.virtualStockEnabled &&
            inputs.virtualStockRearReference == 3)
        {
            if constexpr (CaptureTrace)
                trace->path = AimSolverPath::Hybrid;
            const virtual_stock::Point3 primary = toPoint(rp);
            const virtual_stock::Point3 support = toPoint(supportEndpoint);
            const virtual_stock::Point3 primaryForward = toPoint(rawForward);
            virtual_stock::Point3 offhandDirection{};
            bool rejectedExtreme = false;
            float rejectedAgreement = 0.0f;
            const bool offhandAccepted =
                virtual_stock::TryBuildAcceptedSupportDirection(
                    primary, support, primaryForward, offhandDirection,
                    rejectedExtreme, rejectedAgreement);
            if constexpr (CaptureTrace)
            {
                trace->bAttempted = true;
                trace->bAccepted = offhandAccepted;
                trace->bDirection = traceVec(offhandDirection);
                const float agreement = virtual_stock::Dot(
                    offhandDirection, primaryForward);
                trace->bAgreement = std::isfinite(agreement)
                    ? agreement : 0.0f;
                trace->bExtremeRejected = rejectedExtreme;
                trace->bRejectedAgreement = rejectedAgreement;
            }

            const float offhandInfluence =
                std::isfinite(inputs.virtualStockHybridOffhandInfluence)
                    ? std::clamp(inputs.virtualStockHybridOffhandInfluence,
                                 0.0f, 1.0f)
                    : 0.0f;
            if constexpr (CaptureTrace)
                trace->offhandInfluenceUsed = offhandInfluence;
            virtual_stock::Point3 hipDirection = primaryForward;
            bool hipUsable = virtual_stock::Finite(hipDirection);
            bool hipNeedsOrientation = false;
            if (offhandAccepted)
            {
                virtual_stock::Point3 blendedHip{};
                const bool blended =
                    virtual_stock::TryBlendDirectionAuthority(
                        primaryForward, offhandDirection, offhandInfluence,
                        blendedHip);
                if constexpr (CaptureTrace)
                {
                    trace->hipBlendAttempted = true;
                    trace->hipBlendSucceeded = blended;
                }
                if (blended)
                {
                    hipDirection = blendedHip;
                    hipNeedsOrientation = offhandInfluence > 0.0f;
                }
                // A failed intermediate blend falls back to the exact primary
                // direction; it never manufactures another authority vector.
                else
                {
                    hipDirection = primaryForward;
                    hipNeedsOrientation = false;
                }
                hipUsable = virtual_stock::Finite(hipDirection);
            }
            if constexpr (CaptureTrace)
            {
                trace->hipAimValid = hipUsable;
                trace->hipAimDirection = traceVec(hipDirection);
            }

            virtual_stock::Point3 rawHeadTarget{};
            const bool rawHeadTargetValid = inputs.headValid &&
                virtual_stock::BuildVirtualStockRearTarget(
                    toPoint(inputs.headPosition),
                    inputs.virtualStockRearHeightM, rawHeadTarget);
            virtual_stock::HybridInverseNeckEvaluation inverseNeck{};
            // Product sway correction applies to both Plus Centre and Plus
            // Shoulder; only the rear-reference geometry differs. Horizontal
            // release below stays on raw HMD XZ regardless.
            const bool inverseNeckAttempted =
                inputs.hybridInverseNeckEnabled;
            if (inverseNeckAttempted && inputs.headValid &&
                inputs.inverseNeckNeutralValid)
            {
                inverseNeck = virtual_stock::EvaluateHybridInverseNeck(
                    toPoint(inputs.headPosition),
                    {inputs.headOrientation.x, inputs.headOrientation.y,
                     inputs.headOrientation.z, inputs.headOrientation.w},
                    {inputs.inverseNeckNeutralOrientation.x,
                     inputs.inverseNeckNeutralOrientation.y,
                     inputs.inverseNeckNeutralOrientation.z,
                     inputs.inverseNeckNeutralOrientation.w},
                    inputs.hybridInverseNeckStrength,
                    inputs.hybridInverseNeckForwardM,
                    inputs.hybridInverseNeckUpM,
                    inputs.hybridInverseNeckLateralM);
            }
            virtual_stock::Point3 headTarget = rawHeadTarget;
            bool headTargetValid = rawHeadTargetValid;
            if (inverseNeck.valid)
            {
                headTargetValid = virtual_stock::BuildVirtualStockRearTarget(
                    inverseNeck.correctedHead,
                    inputs.virtualStockRearHeightM, headTarget);
            }
            virtual_stock::Point3 adsTarget = headTarget;
            bool adsTargetValid = headTargetValid;
            bool shoulderTargetUsed = false;
            if constexpr (CaptureTrace)
            {
                trace->inverseNeckAttempted = inverseNeckAttempted;
                trace->inverseNeckValid = inverseNeck.valid;
                if (inverseNeck.valid)
                {
                    trace->inverseNeckCurrentOffset =
                        traceVec(inverseNeck.currentOffset);
                    trace->inverseNeckNeutralOffset =
                        traceVec(inverseNeck.neutralOffset);
                    trace->inverseNeckPredictedOrbit =
                        traceVec(inverseNeck.predictedOrbit);
                    trace->inverseNeckCorrectionClamped =
                        inverseNeck.correctionClamped;
                    trace->inverseNeckAppliedCorrection =
                        traceVec(inverseNeck.appliedCorrection);
                    trace->inverseNeckCorrectedHeadPosition =
                        traceVec(inverseNeck.correctedHead);
                }
                trace->rawHeadTargetValid = rawHeadTargetValid;
                if (rawHeadTargetValid)
                    trace->rawHeadTarget = traceVec(rawHeadTarget);
                trace->correctedHeadTargetValid = headTargetValid;
                if (headTargetValid)
                    trace->correctedHeadTarget = traceVec(headTarget);
                trace->requestedTarget =
                    inputs.virtualStockHybridAdsReference == 1
                        ? AimStockTarget::Shoulder : AimStockTarget::Head;
            }
            if (adsTargetValid && inputs.virtualStockHybridAdsReference == 1)
            {
                // Shoulder corrects the position origin only; the horizontal
                // back/side basis stays on the current HMD yaw orientation.
                // Q0 orientation is never used for forward/right here.
                const virtual_stock::Point3 shoulderBase =
                    inverseNeck.valid ? inverseNeck.correctedHead
                                      : toPoint(inputs.headPosition);
                virtual_stock::Point3 shoulderTarget{};
                if (virtual_stock::TryBuildHmdRelativeShoulderRearTarget(
                        shoulderBase,
                        {inputs.headOrientation.x, inputs.headOrientation.y,
                         inputs.headOrientation.z, inputs.headOrientation.w},
                        inputs.virtualStockRearHeightM,
                        inputs.virtualStockShoulderBackM,
                        inputs.virtualStockShoulderSideM,
                        inputs.virtualStockLeftHanded, shoulderTarget))
                {
                    adsTarget = shoulderTarget;
                    shoulderTargetUsed = true;
                }
            }
            if constexpr (CaptureTrace)
            {
                trace->actualTarget = adsTargetValid
                    ? (shoulderTargetUsed
                        ? AimStockTarget::Shoulder : AimStockTarget::Head)
                    : AimStockTarget::None;
                trace->shoulderToHeadFallback = adsTargetValid &&
                    inputs.virtualStockHybridAdsReference == 1 &&
                    !shoulderTargetUsed;
                trace->targetValid = adsTargetValid;
                if (adsTargetValid)
                    trace->target = traceVec(adsTarget);
            }

            virtual_stock::HybridStockEvaluation stockEvaluation{};
            virtual_stock::Point3 stockDirection{};
            const bool stockAvailable = adsTargetValid &&
                virtual_stock::TryBuildHybridStockDirection(
                    primary, adsTarget, support,
                    inputs.virtualStockStrength, stockDirection,
                    CaptureTrace ? &stockEvaluation : nullptr);
            virtual_stock::HybridSeatEvaluation seatEvaluation{};
            const float naturalSeatInfluence = stockAvailable
                ? virtual_stock::ComputeHybridSeatInfluence(
                    primary, adsTarget, support,
                    inputs.virtualStockHybridSeatFullM,
                    inputs.virtualStockHybridSeatReleaseM,
                    CaptureTrace ? &seatEvaluation : nullptr)
                : 0.0f;
            if constexpr (CaptureTrace)
            {
                trace->cAttempted = adsTargetValid;
                trace->cValid = stockAvailable;
                trace->virtualRearValid = stockEvaluation.virtualRearValid;
                if (stockEvaluation.virtualRearValid)
                    trace->virtualRear = traceVec(stockEvaluation.virtualRear);
                if (stockAvailable)
                    trace->cDirection = traceVec(stockDirection);
                trace->stockStrengthUsed = inputs.virtualStockStrength;
                trace->seatAttempted = stockAvailable;
                trace->seatValid = seatEvaluation.valid;
                trace->seatSegmentLength = seatEvaluation.segmentLength;
                trace->seatRawProjection = seatEvaluation.rawProjection;
                trace->seatClampedProjection =
                    seatEvaluation.clampedProjection;
                trace->seatClosestValid = seatEvaluation.valid;
                if (seatEvaluation.valid)
                    trace->seatClosest = traceVec(seatEvaluation.closest);
                trace->seatErrorM = seatEvaluation.seatError;
                trace->wNaturalValid = seatEvaluation.valid;
                trace->wNatural = naturalSeatInfluence;

                float rearDistanceSquared = 0.0f;
                float primarySupportDistanceSquared = 0.0f;
                float targetSupportDistanceSquared = 0.0f;
                if (adsTargetValid &&
                    virtual_stock::TryDistanceSquared(
                        primary, adsTarget, rearDistanceSquared) &&
                    virtual_stock::TryDistanceSquared(
                        primary, support, primarySupportDistanceSquared) &&
                    virtual_stock::TryDistanceSquared(
                        adsTarget, support, targetSupportDistanceSquared))
                {
                    const float rearDistance = std::sqrt(rearDistanceSquared);
                    const float primarySupportDistance =
                        std::sqrt(primarySupportDistanceSquared);
                    const float targetSupportDistance =
                        std::sqrt(targetSupportDistanceSquared);
                    if (std::isfinite(rearDistance) &&
                        std::isfinite(primarySupportDistance) &&
                        std::isfinite(targetSupportDistance))
                    {
                        trace->releaseGeometryValid = true;
                        trace->rearToStockTargetDistanceM = rearDistance;
                        trace->primaryToSupportDistanceM =
                            primarySupportDistance;
                        trace->stockToSupportDistanceM =
                            targetSupportDistance;
                    }
                }
            }
            float afterDiagnosticInfluence = naturalSeatInfluence;
            const HybridDiagnosticOverride diagnosticOverride =
                NormalizeHybridDiagnosticOverride(
                    static_cast<uint8_t>(inputs.hybridDiagnosticOverride));
            bool afterDiagnosticInfluenceValid =
                seatEvaluation.valid || !CaptureTrace;
            if (diagnosticOverride == HybridDiagnosticOverride::ForceHip &&
                stockAvailable)
            {
                afterDiagnosticInfluence = 0.0f;
                afterDiagnosticInfluenceValid = true;
            }
            else if (diagnosticOverride == HybridDiagnosticOverride::ForceStock &&
                stockAvailable)
            {
                afterDiagnosticInfluence = 1.0f;
                afterDiagnosticInfluenceValid = true;
            }

            virtual_stock::HybridHorizontalReleaseEvaluation
                horizontalReleaseEvaluation{};
            float effectiveSeatInfluence = afterDiagnosticInfluence;
            bool effectiveSeatInfluenceValid = afterDiagnosticInfluenceValid;
            if (inputs.hybridHorizontalRearReleaseEnabled)
            {
                const float horizontalInfluence =
                    virtual_stock::ComputeHybridHorizontalRearReleaseInfluence(
                        primary, inputs.headValid,
                        toPoint(inputs.headPosition),
                        inputs.hybridHorizontalRearReleaseFullM,
                        inputs.hybridHorizontalRearReleaseReleaseM,
                        &horizontalReleaseEvaluation);
                effectiveSeatInfluence = horizontalReleaseEvaluation.valid
                    ? afterDiagnosticInfluence * horizontalInfluence : 0.0f;
                effectiveSeatInfluenceValid = afterDiagnosticInfluenceValid &&
                    horizontalReleaseEvaluation.valid &&
                    std::isfinite(effectiveSeatInfluence);
                if (!effectiveSeatInfluenceValid)
                    effectiveSeatInfluence = 0.0f;
            }
            if constexpr (CaptureTrace)
            {
                trace->effectiveDiagnosticOverride =
                    static_cast<uint8_t>(diagnosticOverride);
                trace->wAfterDiagnosticValid = stockAvailable &&
                    afterDiagnosticInfluenceValid;
                trace->wAfterDiagnostic = afterDiagnosticInfluence;
                trace->horizontalReleaseAttempted =
                    inputs.hybridHorizontalRearReleaseEnabled;
                trace->horizontalReleaseValid =
                    horizontalReleaseEvaluation.valid;
                trace->rearHorizontalReachM =
                    horizontalReleaseEvaluation.horizontalReach;
                trace->horizontalReleaseInfluence =
                    horizontalReleaseEvaluation.influence;
                trace->wEffectiveValid = stockAvailable &&
                    effectiveSeatInfluenceValid;
                trace->wEffective = effectiveSeatInfluence;
                trace->overrideApplied = stockAvailable &&
                    diagnosticOverride != HybridDiagnosticOverride::Normal;
                trace->forceStockEligible = stockAvailable;
            }

            const bool horizontalReleaseEnabled =
                inputs.hybridHorizontalRearReleaseEnabled;
            virtual_stock::Point3 finalDirection = horizontalReleaseEnabled
                ? (offhandAccepted ? offhandDirection : primaryForward)
                : hipDirection;
            const bool baseNeedsOrientation = horizontalReleaseEnabled
                ? offhandAccepted
                : (offhandAccepted && hipNeedsOrientation);
            const bool baseUsable = horizontalReleaseEnabled
                ? virtual_stock::Finite(finalDirection) : hipUsable;
            bool stockContributed = false;
            if (stockAvailable && effectiveSeatInfluence > 0.0f)
            {
                if constexpr (CaptureTrace)
                    trace->stockBlendAttempted = true;
                if (horizontalReleaseEnabled)
                {
                    virtual_stock::Point3 composed{};
                    if (baseUsable &&
                        virtual_stock::TryBlendDirectionAuthority(
                            finalDirection, stockDirection,
                            effectiveSeatInfluence, composed))
                    {
                        finalDirection = composed;
                        stockContributed = true;
                        if constexpr (CaptureTrace)
                            trace->stockBlendSucceeded = true;
                    }
                }
                else if (diagnosticOverride ==
                    HybridDiagnosticOverride::ForceStock)
                {
                    finalDirection = stockDirection;
                    stockContributed = true;
                    if constexpr (CaptureTrace)
                        trace->stockBlendSucceeded = true;
                }
                else if (hipUsable)
                {
                    virtual_stock::Point3 composed{};
                    if (virtual_stock::TryBlendDirectionAuthority(
                            hipDirection, stockDirection, effectiveSeatInfluence,
                            composed))
                    {
                        finalDirection = composed;
                        stockContributed = true;
                        if constexpr (CaptureTrace)
                            trace->stockBlendSucceeded = true;
                    }
                    // A failed HipAim->C blend falls back to HipAim, never to C.
                }
            }

            const bool requiresOrientation = stockContributed ||
                baseNeedsOrientation;
            bool orientationBuilt = true;
            if (requiresOrientation)
            {
                if constexpr (CaptureTrace)
                    trace->orientationRebuildAttempted = true;
                virtual_stock::Quat4 aimOrientation{};
                orientationBuilt = baseUsable &&
                    virtual_stock::Finite(finalDirection) &&
                    virtual_stock::BuildTwoHandAimOrientation(
                        finalDirection, toPoint(rup), aimOrientation);
                if (orientationBuilt)
                {
                    result.pose.orientation = {
                        aimOrientation.x, aimOrientation.y,
                        aimOrientation.z, aimOrientation.w};
                    if constexpr (CaptureTrace)
                        trace->orientationRebuildSucceeded = true;
                }
            }

            if (!orientationBuilt)
            {
                result.twoHandActive = false;
            }
            else
            {
                // Accepted B owns presentation even when it contributes zero
                // numerical authority. C alone can activate presentation only
                // after its final orientation has been built successfully.
                result.twoHandActive = offhandAccepted || stockContributed;
            }
            if (!offhandAccepted && rejectedExtreme &&
                !result.twoHandActive)
            {
                result.rejectedExtreme = true;
                result.rejectedAgreement = rejectedAgreement;
            }
            if constexpr (CaptureTrace)
            {
                trace->stockContributed = stockContributed && orientationBuilt;
                trace->exactAEndpointSelected =
                    !orientationBuilt ||
                    (!stockContributed && !baseNeedsOrientation);
                const virtual_stock::Point3 appliedDirection =
                    orientationBuilt ? finalDirection : primaryForward;
                trace->finalDirectionValid =
                    virtual_stock::Finite(appliedDirection);
                if (trace->finalDirectionValid)
                    trace->finalDirection = traceVec(appliedDirection);
            }
            finishAimPose();
            return result;
        }
        float stockStrength = 0.0f;
        virtual_stock::DirectionSelection selection{};
        bool shoulderTargetConstructed = false;
        bool chestTargetConstructed = false;
        bool adaptiveTargetSelected = false;
        virtual_stock::Point3 adaptiveTarget{};
        const bool shoulderRequested = inputs.virtualStockEnabled &&
            inputs.virtualStockRearReference == 1 && inputs.headValid;
        const bool chestRequested = inputs.virtualStockEnabled &&
            inputs.virtualStockRearReference == 2 && inputs.headValid;
        const bool adaptiveRequested = inputs.virtualStockEnabled &&
            inputs.virtualStockRearReference == 3 && inputs.headValid;
        // Product sway correction for Standard Centre/Shoulder only. Corrects
        // the positional base only; Shoulder keeps the current HMD yaw basis.
        // Dormant Chest/Adaptive never evaluate or consume correction.
        // Failure falls back to raw HMD and never drops stock.
        virtual_stock::HybridInverseNeckEvaluation fixedInverseNeck{};
        const bool productFixedReference =
            inputs.virtualStockRearReference == 0 ||
            inputs.virtualStockRearReference == 1;
        const bool fixedInverseNeckAttempted =
            inputs.virtualStockEnabled && productFixedReference &&
            inputs.hybridInverseNeckEnabled &&
            inputs.headValid && inputs.inverseNeckNeutralValid;
        if (fixedInverseNeckAttempted)
        {
            fixedInverseNeck = virtual_stock::EvaluateHybridInverseNeck(
                toPoint(inputs.headPosition),
                {inputs.headOrientation.x, inputs.headOrientation.y,
                 inputs.headOrientation.z, inputs.headOrientation.w},
                {inputs.inverseNeckNeutralOrientation.x,
                 inputs.inverseNeckNeutralOrientation.y,
                 inputs.inverseNeckNeutralOrientation.z,
                 inputs.inverseNeckNeutralOrientation.w},
                inputs.hybridInverseNeckStrength,
                inputs.hybridInverseNeckForwardM, inputs.hybridInverseNeckUpM,
                inputs.hybridInverseNeckLateralM);
        }
        const virtual_stock::Point3 fixedHeadBase =
            fixedInverseNeck.valid ? fixedInverseNeck.correctedHead
                                   : toPoint(inputs.headPosition);
        if constexpr (CaptureTrace)
        {
            trace->inverseNeckAttempted =
                trace->inverseNeckAttempted || fixedInverseNeckAttempted;
            if (fixedInverseNeckAttempted && fixedInverseNeck.valid)
            {
                trace->inverseNeckValid = true;
                trace->inverseNeckCurrentOffset =
                    traceVec(fixedInverseNeck.currentOffset);
                trace->inverseNeckNeutralOffset =
                    traceVec(fixedInverseNeck.neutralOffset);
                trace->inverseNeckPredictedOrbit =
                    traceVec(fixedInverseNeck.predictedOrbit);
                trace->inverseNeckCorrectionClamped =
                    fixedInverseNeck.correctionClamped;
                trace->inverseNeckAppliedCorrection =
                    traceVec(fixedInverseNeck.appliedCorrection);
                trace->inverseNeckCorrectedHeadPosition =
                    traceVec(fixedInverseNeck.correctedHead);
            }
            trace->effectiveDiagnosticOverride = static_cast<uint8_t>(
                NormalizeHybridDiagnosticOverride(
                    static_cast<uint8_t>(inputs.hybridDiagnosticOverride)));
            trace->fixedConfiguredStrength = inputs.virtualStockStrength;
            trace->fixedProximityEnabled =
                inputs.virtualStockProximityRelease;
            if (inputs.virtualStockEnabled &&
                inputs.virtualStockRearReference >= 0 &&
                inputs.virtualStockRearReference <= 2)
            {
                trace->requestedTarget = inputs.virtualStockRearReference == 1
                    ? AimStockTarget::Shoulder
                    : inputs.virtualStockRearReference == 2
                        ? AimStockTarget::Chest
                        : AimStockTarget::Head;
            }
        }
        const auto recordFixedTarget = [&](virtual_stock::Point3 target,
                                            bool valid,
                                            float effectiveStrength,
                                            AimStockTarget actualTarget) {
            if constexpr (CaptureTrace)
            {
                trace->actualTarget = valid
                    ? actualTarget : AimStockTarget::None;
                trace->targetValid = valid;
                trace->fixedTargetValid = valid;
                trace->fixedEffectiveStrength = effectiveStrength;
                if (!valid)
                    return;
                trace->target = traceVec(target);
                trace->fixedTarget = traceVec(target);
                float distanceSquared = 0.0f;
                if (virtual_stock::TryDistanceSquared(
                        toPoint(rp), target, distanceSquared))
                {
                    const float distance = std::sqrt(distanceSquared);
                    if (std::isfinite(distance))
                    {
                        trace->fixedRearDistanceValid = true;
                        trace->fixedRearToTargetDistanceM = distance;
                        if (inputs.virtualStockProximityRelease)
                        {
                            trace->fixedProximityCalculated = true;
                            trace->fixedProximityInfluence =
                                virtual_stock::ComputeProximityInfluence(
                                    distance,
                                    inputs.virtualStockProximityFullM,
                                    inputs.virtualStockProximityReleaseM);
                        }
                    }
                }
            }
        };
        if (shoulderRequested)
        {
            virtual_stock::Point3 shoulderRear{};
            const bool shoulderTargetValid =
                virtual_stock::TryBuildHmdRelativeShoulderRearTarget(
                    fixedHeadBase,
                    {inputs.headOrientation.x, inputs.headOrientation.y,
                     inputs.headOrientation.z, inputs.headOrientation.w},
                    inputs.virtualStockRearHeightM,
                    inputs.virtualStockShoulderBackM,
                    inputs.virtualStockShoulderSideM,
                    inputs.virtualStockLeftHanded, shoulderRear);
            if (shoulderTargetValid)
            {
                shoulderTargetConstructed = true;
                stockStrength =
                    virtual_stock::ApplyVirtualStockProximityReleaseForTarget(
                        inputs.virtualStockProximityRelease,
                        inputs.virtualStockStrength, toPoint(rp), true,
                        shoulderRear, inputs.virtualStockProximityFullM,
                        inputs.virtualStockProximityReleaseM);
                selection = virtual_stock::SelectTwoHandAimDirectionForTarget(
                    inputs.virtualStockEnabled, stockStrength, true,
                    shoulderRear, toPoint(supportEndpoint), toPoint(rp),
                    toPoint(supportEndpoint), toPoint(rawForward));
                if constexpr (CaptureTrace)
                {
                    recordFixedTarget(
                        shoulderRear, true, stockStrength,
                        AimStockTarget::Shoulder);
                }
            }
        }
        else if (chestRequested)
        {
            virtual_stock::Point3 chestRear{};
            const bool chestTargetValid =
                virtual_stock::TryBuildHmdRelativeChestRearTarget(
                    toPoint(inputs.headPosition),
                    {inputs.headOrientation.x, inputs.headOrientation.y,
                     inputs.headOrientation.z, inputs.headOrientation.w},
                    inputs.virtualStockChestHeightM,
                    inputs.virtualStockChestBackM,
                    inputs.virtualStockChestSideM,
                    inputs.virtualStockLeftHanded, chestRear);
            if (chestTargetValid)
            {
                chestTargetConstructed = true;
                stockStrength =
                    virtual_stock::ApplyVirtualStockProximityReleaseForTarget(
                        inputs.virtualStockProximityRelease,
                        inputs.virtualStockStrength, toPoint(rp), true,
                        chestRear, inputs.virtualStockProximityFullM,
                        inputs.virtualStockProximityReleaseM);
                selection = virtual_stock::SelectTwoHandAimDirectionForTarget(
                    inputs.virtualStockEnabled, stockStrength, true,
                    chestRear, toPoint(supportEndpoint), toPoint(rp),
                    toPoint(supportEndpoint), toPoint(rawForward));
                if constexpr (CaptureTrace)
                {
                    recordFixedTarget(
                        chestRear, true, stockStrength,
                        AimStockTarget::Chest);
                }
            }
        }
        else if (adaptiveRequested)
        {
            virtual_stock::AdaptiveStockPatch adaptivePatch{};
            const bool patchValid =
                virtual_stock::TryBuildAdaptiveStockPatch(
                    toPoint(inputs.headPosition),
                    {inputs.headOrientation.x, inputs.headOrientation.y,
                     inputs.headOrientation.z, inputs.headOrientation.w},
                    inputs.virtualStockAdaptiveTopHeightM,
                    inputs.virtualStockAdaptiveBottomHeightM,
                    inputs.virtualStockAdaptiveTopHalfWidthM,
                    inputs.virtualStockAdaptiveBottomHalfWidthM,
                    adaptivePatch);
            adaptiveTargetSelected = patchValid &&
                virtual_stock::TrySelectAdaptiveRearTarget(
                    toPoint(rp), adaptivePatch, adaptiveTarget);
            if (adaptiveTargetSelected)
            {
                stockStrength =
                    virtual_stock::ApplyVirtualStockProximityReleaseForTarget(
                        inputs.virtualStockProximityRelease,
                        inputs.virtualStockStrength, toPoint(rp), true,
                        adaptiveTarget, inputs.virtualStockProximityFullM,
                        inputs.virtualStockProximityReleaseM);
                selection = virtual_stock::SelectTwoHandAimDirectionForTarget(
                    inputs.virtualStockEnabled, stockStrength, true,
                    adaptiveTarget, toPoint(supportEndpoint), toPoint(rp),
                    toPoint(supportEndpoint), toPoint(rawForward));
            }
        }
        const bool useHeadPath =
            (!shoulderRequested && !chestRequested && !adaptiveRequested) ||
            (shoulderRequested && virtual_stock::ShouldFallbackToHeadRearTarget(
                shoulderRequested, shoulderTargetConstructed)) ||
            (chestRequested && virtual_stock::ShouldFallbackToHeadRearTarget(
                chestRequested, chestTargetConstructed)) ||
            (adaptiveRequested && !adaptiveTargetSelected);
        if (useHeadPath)
        {
            // This is the released Head path, including its existing fallback
            // behavior when the optional Shoulder basis is unavailable.
            // Product sway correction supplies the positional base; raw-H
            // fallback is already selected in fixedHeadBase.
            stockStrength = virtual_stock::ApplyVirtualStockProximityRelease(
                inputs.virtualStockProximityRelease,
                inputs.virtualStockStrength, toPoint(rp), inputs.headValid,
                fixedHeadBase, inputs.virtualStockRearHeightM,
                inputs.virtualStockProximityFullM,
                inputs.virtualStockProximityReleaseM);
            selection = virtual_stock::SelectTwoHandAimDirection(
                inputs.virtualStockEnabled, stockStrength,
                inputs.virtualStockRearHeightM, inputs.headValid,
                fixedHeadBase, toPoint(supportEndpoint),
                toPoint(rp), toPoint(supportEndpoint), toPoint(rawForward));
            if constexpr (CaptureTrace)
            {
                virtual_stock::Point3 headTarget{};
                const bool headTargetValid = inputs.virtualStockEnabled &&
                    inputs.headValid &&
                    virtual_stock::BuildVirtualStockRearTarget(
                        fixedHeadBase,
                        inputs.virtualStockRearHeightM, headTarget);
                trace->shoulderToHeadFallback = shoulderRequested &&
                    !shoulderTargetConstructed;
                recordFixedTarget(
                    headTarget, headTargetValid, stockStrength,
                    AimStockTarget::Head);
            }
        }
        if constexpr (CaptureTrace)
        {
            trace->path = selection.usedVirtualStock
                ? AimSolverPath::FixedStock
                : AimSolverPath::LegacyTwoHand;
            trace->fixedDirectionValid =
                selection.valid && selection.usedVirtualStock;
            if (!selection.usedVirtualStock)
            {
                virtual_stock::Point3 bDirection{};
                bool rejectedExtreme = false;
                float rejectedAgreement = 0.0f;
                trace->bAttempted = true;
                trace->bAccepted =
                    virtual_stock::TryBuildAcceptedSupportDirection(
                        toPoint(rp), toPoint(supportEndpoint),
                        toPoint(rawForward), bDirection,
                        rejectedExtreme, rejectedAgreement);
                trace->bDirection = traceVec(bDirection);
                const float agreement = virtual_stock::Dot(
                    bDirection, toPoint(rawForward));
                trace->bAgreement = std::isfinite(agreement)
                    ? agreement : 0.0f;
                trace->bExtremeRejected = rejectedExtreme;
                trace->bRejectedAgreement = rejectedAgreement;
            }
        }
        if (!selection.valid)
        {
            result.rejectedExtreme = selection.rejectedExtreme;
            result.rejectedAgreement = selection.rejectedAgreement;
            if constexpr (CaptureTrace)
                trace->exactAEndpointSelected = true;
            finishAimPose();
            return result;
        }
        virtual_stock::Quat4 aimOrientation{};
        if constexpr (CaptureTrace)
            trace->orientationRebuildAttempted = true;
        if (!virtual_stock::BuildTwoHandAimOrientation(
                selection.direction, toPoint(rup), aimOrientation))
        {
            if constexpr (CaptureTrace)
                trace->exactAEndpointSelected = true;
            finishAimPose();
            return result;
        }
        result.pose.orientation = {
            aimOrientation.x, aimOrientation.y,
            aimOrientation.z, aimOrientation.w};
        result.twoHandActive = true;
        if constexpr (CaptureTrace)
        {
            trace->orientationRebuildSucceeded = true;
            trace->finalDirectionValid = true;
            trace->finalDirection = traceVec(selection.direction);
            trace->exactAEndpointSelected = false;
        }
        finishAimPose();
        return result;
    }

    // Recipient extraction-shape name for the product aim solver. The donor
    // merged the legacy and stock paths into one body, so this is the exact
    // same calculation ComputeAimPose() performs (no Adaptive-era branch).
    AimPoseResult ComputeStockAimPose(const AimPoseInputs& inputs) noexcept
    {
        return ComputeAimPoseImpl<false>(inputs, nullptr);
    }
