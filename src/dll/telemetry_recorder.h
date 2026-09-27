#pragma once

#include "aim_pose_trace.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#ifdef HALOMCCVR_TELEMETRY_TESTING
#include <string>
#include <vector>
#endif

inline constexpr uint32_t kTelemetrySchemaVersion = 2;
inline constexpr uint32_t kTelemetryQueueSlots = 4096;
inline constexpr uint32_t kTelemetryQueueUsableCapacity =
    kTelemetryQueueSlots - 1;

enum class TelemetryRecorderState : uint8_t
{
    Unavailable = 0,
    Idle,
    Starting,
    Recording,
    Finalizing,
    Error,
};

enum class TelemetryErrorCode : uint8_t
{
    None = 0,
    InvalidLogDirectory,
    CreateControlEventFailed,
    CreateWorkerFailed,
    OpenFileFailed,
    WriteFailed,
    FlushFailed,
    CloseFailed,
    AllocationFailed,
    InternalFailure,
    CreateDirectoryFailed,
};

struct TelemetryVec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct TelemetryQuat
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

struct TelemetryPose
{
    TelemetryQuat orientation{};
    TelemetryVec3 position{};
};

struct TelemetryEyeView
{
    TelemetryPose pose{};
    float fovLeft = 0.0f;
    float fovRight = 0.0f;
    float fovUp = 0.0f;
    float fovDown = 0.0f;
};

struct TelemetryPadState
{
    bool valid = false;
    float moveX = 0.0f;
    float moveY = 0.0f;
    float turnX = 0.0f;
    float turnY = 0.0f;
    float trigL = 0.0f;
    float trigR = 0.0f;
    float gripL = 0.0f;
    float gripR = 0.0f;
    bool a = false;
    bool b = false;
    bool x = false;
    bool y = false;
    bool clickL = false;
    bool clickR = false;
    bool menu = false;
    bool thumbrestDpad = false;
    float dpadX = 0.0f;
    float dpadY = 0.0f;
    bool exclusiveInput = false;
};

struct TelemetryEffectiveSettings
{
    bool twoHandEnabled = false;
    bool twoHandLatched = false;
    bool twoHandToggle = true;
    bool virtualStockEnabled = false;
    int32_t virtualStockRearReference = 0;
    float virtualStockStrength = 0.0f;
    uint8_t hybridDiagnosticOverride = 0;
    float hybridOffhandInfluence = 0.0f;
    int32_t hybridAdsReference = 0;
    float hybridSeatFullM = 0.0f;
    float hybridSeatReleaseM = 0.0f;
    bool horizontalReleaseEnabled = false;
    float horizontalReleaseFullM = 0.0f;
    float horizontalReleaseReleaseM = 0.0f;
    bool inverseNeckEnabled = false;
    float inverseNeckStrength = 0.0f;
    float inverseNeckForwardM = 0.0f;
    float inverseNeckUpM = 0.0f;
    float inverseNeckLateralM = 0.0f;
    float rearHeightM = 0.0f;
    float shoulderBackM = 0.0f;
    float shoulderSideM = 0.0f;
    float chestHeightM = 0.0f;
    float chestBackM = 0.0f;
    float chestSideM = 0.0f;
    bool proximityRelease = false;
    float proximityFullM = 0.0f;
    float proximityReleaseM = 0.0f;
    float gunYawDeg = 0.0f;
    float gunPitchDeg = 0.0f;
    float gunRollDeg = 0.0f;
    bool supportGripPoseEnabled = false;
    bool supportEndpointUsedGrip = false;
    bool leftHanded = false;
};

struct TelemetryAimResult
{
    bool valid = false;
    TelemetryPose pose{};
    TelemetryVec3 forward{};
    bool twoHandActive = false;
    bool rejectedExtreme = false;
    float rejectedAgreement = 0.0f;
};

struct TelemetryControlResult
{
    uint8_t profileId = 0;
    TelemetryEffectiveSettings effectiveSettings{};
    TelemetryAimResult aim{};
    uint8_t path = 0;
    uint8_t requestedTarget = 0;
    uint8_t actualTarget = 0;
    bool shoulderToHeadFallback = false;
    bool fixedTargetValid = false;
    TelemetryVec3 fixedTarget{};
    bool fixedRearDistanceValid = false;
    float fixedRearToTargetDistanceM = 0.0f;
    bool fixedProximityEnabled = false;
    bool fixedProximityCalculated = false;
    float fixedProximityInfluence = 0.0f;
    float fixedConfiguredStrength = 0.0f;
    float fixedEffectiveStrength = 0.0f;
    bool fixedDirectionValid = false;
    bool exactAEndpointSelected = false;
    bool orientationRebuildAttempted = false;
    bool orientationRebuildSucceeded = false;
};

struct TelemetryFrame
{
    uint32_t schemaVersion = kTelemetrySchemaVersion;
    uint64_t preparedSerial = 0;
    int64_t predictedDisplayTime = 0;
    int64_t predictedDisplayPeriod = 0;
    int64_t captureBeginQpc = 0;
    int64_t captureEndQpc = 0;
    uint64_t contactSpaceEpoch = 0;
    int64_t contactSpaceChangeAtNs = 0;
    uint8_t activeTitle = 0;
    int32_t sessionState = 0;
    uint32_t locatedViewCount = 0;
    bool shouldRender = false;
    bool upcomingViewsValid = false;
    bool focused = false;
    bool stereoEnabled = false;
    bool menuOpen = false;

    bool semanticPrimaryValid = false;
    TelemetryPose semanticPrimaryAim{};
    TelemetryVec3 semanticPrimaryForward{};
    bool semanticSupportValid = false;
    TelemetryPose semanticSupportAim{};
    bool supportEndpointValid = false;
    TelemetryVec3 supportEndpoint{};
    bool supportEndpointUsedGrip = false;

    bool physicalLeftAimValid = false;
    TelemetryPose physicalLeftAim{};
    bool physicalRightAimValid = false;
    TelemetryPose physicalRightAim{};

    bool supportGripValid = false;
    TelemetryVec3 supportGripPosition{};
    bool semanticPrimaryGripPositionValid = false;
    TelemetryVec3 semanticPrimaryGripPosition{};

    bool headSampleValid = false;
    bool stockHeadValid = false;
    TelemetryPose semanticHmd{};
    float headsetSmoothing = 0.0f;

    TelemetryEyeView views[2]{};
    bool semanticPrimaryVelocityValid = false;
    TelemetryVec3 semanticPrimaryVelocity{};
    uint64_t semanticPrimaryVelocityAtMs = 0;
    bool semanticSupportVelocityValid = false;
    TelemetryVec3 semanticSupportVelocity{};
    uint64_t semanticSupportVelocityAtMs = 0;
    TelemetryPadState pad{};

    uint8_t testProfileId = 0;
    bool testProfileCustom = true;
    TelemetryEffectiveSettings effectiveSettings{};
    AimPoseTrace aimTrace{};
    TelemetryAimResult canonicalAim{};

    // Grab/release aim continuity (Virtual Stock, Standard and Plus). This
    // family describes the presentation layer only; aim_trace.final_direction
    // and canonical_aim above stay the raw live solve. transition_stock_mode is
    // the product stock mode this frame (0 = Standard, 1 = Plus, i.e. rear
    // reference 3) and is only meaningful while transition_stock_mode_valid is
    // true (Virtual Stock enabled); when invalid it reads 0, never a stale
    // mode.
    uint8_t transitionStockMode = 0;
    bool transitionStockModeValid = false;
    bool transitionActive = false;
    uint8_t transitionPhase = 0;
    uint8_t transitionEdgeKind = 0;
    uint8_t transitionAnchorSource = 0;
    bool transitionLiveCalibratedForwardValid = false;
    TelemetryVec3 transitionLiveCalibratedForward{};
    bool transitionPresentedForwardValid = false;
    TelemetryVec3 transitionPresentedForward{};
    float transitionInitialCorrectionDeg = 0.0f;
    float transitionRemainingCorrectionDeg = 0.0f;
    float transitionElapsedMs = 0.0f;
    bool transitionOneHandAnchorValid = false;
    TelemetryVec3 transitionOneHandAnchorForward{};
    uint64_t transitionAdvanceCount = 0;
    uint64_t transitionLastPreparedSerial = 0;
    // The prepared serial the layer actually ran for; 0 when it did not run
    // (feature off/inactive), so absence is unambiguous.
    uint64_t transitionAppliedSerial = 0;

    TelemetryControlResult cfVsOff{};
    TelemetryControlResult cfFixedHead{};
    TelemetryControlResult cfFixedShoulder{};
};

static_assert(std::atomic<uint32_t>::is_always_lock_free);
static_assert(std::atomic<uint64_t>::is_always_lock_free);
static_assert(std::is_trivially_copyable_v<TelemetryFrame>);
static_assert(std::is_standard_layout_v<TelemetryFrame>);
static_assert(sizeof(TelemetryFrame) <= 2048,
              "TelemetryFrame grew beyond the T0 hot-path budget");

struct TelemetryStatusSnapshot
{
    TelemetryRecorderState state = TelemetryRecorderState::Unavailable;
    TelemetryErrorCode error = TelemetryErrorCode::None;
    uint32_t systemError = 0;
    bool desiredRecording = false;
    bool accepting = false;
    int64_t sessionStartQpc = 0;
    int64_t qpcNow = 0;
    int64_t qpcFrequency = 0;
    uint64_t producerCalls = 0;
    uint64_t duplicateSerialSuppressed = 0;
    uint64_t enqueued = 0;
    uint64_t written = 0;
    uint64_t droppedQueueFull = 0;
    uint64_t writerFailures = 0;
    uint64_t weaponEventsEnqueued = 0;
    uint64_t weaponEventsWritten = 0;
    uint64_t weaponEventsDroppedQueueFull = 0;
};

// ---- Weapon-order diagnostic events (read-only evidence tranche) ----
// Fixed-size, trivially copyable event records for the persistent-support-grip
// first-B-frame ordering question. Transported by the telemetry recorder's own
// worker/file pipeline (same JSONL file, same analyser); no second recorder.
inline constexpr uint32_t kWeaponOrderEventQueueSlots = 1024;

enum class WeaponOrderEventKind : uint8_t
{
    PresentBegin = 0,
    AfterPresentBeforePrepare,
    CapturePreLatch,
    CaptureProbeBegin,
    CaptureProbeResult,
    FpEntry,
    FpWeaponCommit,
};

enum class WeaponOrderEventStatus : uint8_t
{
    NoObservation = 0,
    Success,
    ReaderReturnedFalse,
    GuardRejected,
    ExceptionOrFault,
    NotAttemptedNoSafeThread,
    NotAttemptedThreadMismatch,
    DefinitivelyAbsent,
};

struct TelemetryWeaponEvent
{
    uint64_t sequence = 0; // process-wide diagnostic order, 0 = slot free
    uint64_t session = 0; // recorder admission generation at publish
    int64_t timestampQpc = 0;
    uint32_t threadId = 0;
    uint8_t kind = 0;
    uint8_t status = 0;
    uint8_t title = 0;
    uint32_t titleGeneration = 0;
    uint64_t preparedSerial = 0;
    uint32_t controlledUnit = 0;
    uint32_t primaryWeapon = 0;
    // Per-kind meaning: CaptureProbeResult carries aux0 = matching
    // CaptureProbeBegin sequence (0 when no Begin was emitted) and aux0/aux1
    // otherwise carry title-defined validity/provenance bits.
    uint64_t aux0 = 0;
    uint64_t aux1 = 0;
    uint64_t tearGuard = 0; // must equal sequence once the slot is committed
};

static_assert(std::is_trivially_copyable_v<TelemetryWeaponEvent>);
static_assert(std::is_standard_layout_v<TelemetryWeaponEvent>);

struct TelemetryWeaponEventCounters
{
    uint64_t enqueued = 0;
    uint64_t written = 0;
    uint64_t droppedQueueFull = 0;
    uint64_t sessionSkipped = 0;
};

// Cheap recorder-active gate for diagnostic probes. When false, probes must
// perform no diagnostic reads, allocate nothing and publish nothing.
bool Telemetry_WeaponEventsAccepting() noexcept;
// Admission generation of the live recording session (0 when not accepting).
// Diagnostic metadata is valid only within the session that produced it.
uint64_t Telemetry_CurrentSessionToken() noexcept;
// Publishes one fixed event record. Returns the assigned global sequence, or
// 0 when recording is off or the record was dropped (queue full). Never
// allocates, logs, takes locks, performs I/O/COM/scans, or mutates gameplay.
uint64_t Telemetry_PublishWeaponEvent(uint8_t kind, uint8_t status,
    uint8_t title, uint32_t titleGeneration, uint64_t preparedSerial,
    uint32_t controlledUnit, uint32_t primaryWeapon, uint64_t aux0,
    uint64_t aux1) noexcept;
TelemetryWeaponEventCounters Telemetry_GetWeaponEventCounters() noexcept;

bool Telemetry_Init() noexcept;
void Telemetry_RequestStart() noexcept;
void Telemetry_RequestStop() noexcept;
TelemetryStatusSnapshot Telemetry_GetStatus() noexcept;
const char* Telemetry_StateName(TelemetryRecorderState state) noexcept;
const char* Telemetry_ErrorName(TelemetryErrorCode error) noexcept;

// Returns false on the disabled path, a Start/Stop race, a duplicate serial, or
// a full queue. Queue-full rejection happens before the caller captures a frame.
// A true return owns one producer-in-flight receipt that must be completed by
// exactly one Telemetry_PublishFrame call.
bool Telemetry_BeginFrame(uint64_t preparedSerial) noexcept;
void Telemetry_PublishFrame(const TelemetryFrame& frame) noexcept;

#ifdef HALOMCCVR_TELEMETRY_TESTING
void Telemetry_TestForceWeaponEventsAccepting(bool enabled) noexcept;
void Telemetry_TestResetWeaponEvents() noexcept;
bool Telemetry_TestPopWeaponEvent(TelemetryWeaponEvent& out) noexcept;
uint64_t Telemetry_TestWeaponEventDrops() noexcept;
bool Telemetry_TestSerializeWeaponEvent(const TelemetryWeaponEvent& event,
    char* output, size_t capacity, size_t& written) noexcept;
// Runs the production worker drain core through an in-memory sink with the
// supplied session token. Exposes gap markers and cross-session skips exactly
// as the real worker file path emits them.
bool Telemetry_TestDrainWeaponEvents(uint64_t sessionToken,
    std::vector<std::string>& lines) noexcept;
// Same, but the sink fails after `maxLines` successful lines. Used to prove a
// failed write does not advance the drain pointer and the event is retried.
bool Telemetry_TestDrainWeaponEventsLimited(uint64_t sessionToken,
    size_t maxLines, std::vector<std::string>& lines) noexcept;
// Exercises the session final-drain semantics (producer quiescence assumed):
// every claimed sequence up to the frontier must be serialized or explicitly
// gapped before the session may close cleanly.
bool Telemetry_TestFinalDrainWeaponEvents(uint64_t sessionToken,
    std::vector<std::string>& lines) noexcept;
// Weapon-event producer-in-flight handshake hooks (transport tests only).
uint32_t Telemetry_TestWeaponEventProducersInFlight() noexcept;
void Telemetry_TestPauseWeaponEventProducer(bool enabled) noexcept;
bool Telemetry_TestWeaponEventProducerReachedPause() noexcept;
// Test-only pause after an accepted sequence claim and before its commit
// marker: creates a genuinely outstanding claim for drain-safety tests.
void Telemetry_TestPauseWeaponEventCommit(bool enabled) noexcept;
bool Telemetry_TestWeaponEventCommitReachedPause() noexcept;
void Telemetry_TestBumpAdmissionGeneration() noexcept;
void Telemetry_TestResetAdmissionToken() noexcept;
void Telemetry_TestResetRing() noexcept;
bool Telemetry_TestPushRing(const TelemetryFrame& frame) noexcept;
bool Telemetry_TestPopRing(TelemetryFrame& frame) noexcept;
uint64_t Telemetry_TestRingDrops() noexcept;
void Telemetry_TestSetForceOpenFailure(bool enabled) noexcept;
void Telemetry_TestFailWriteAfter(int32_t successfulWrites) noexcept;
void Telemetry_TestFailFlushAfter(int32_t successfulFlushes) noexcept;
void Telemetry_TestSetForceCloseFailure(bool enabled) noexcept;
void Telemetry_TestPauseStarting(bool enabled) noexcept;
void Telemetry_TestPauseRecording(bool enabled) noexcept;
bool Telemetry_TestWorkerReachedRecording() noexcept;
void Telemetry_TestPauseRecordingDrain(bool enabled) noexcept;
bool Telemetry_TestWorkerReachedRecordingDrain() noexcept;
void Telemetry_TestPauseFinalizing(bool enabled) noexcept;
void Telemetry_TestPauseProducerAdmission(bool enabled) noexcept;
bool Telemetry_TestProducerReachedAdmission() noexcept;
void Telemetry_TestPauseProducerSecondCheck(bool enabled) noexcept;
bool Telemetry_TestProducerReachedSecondCheck() noexcept;
bool Telemetry_TestSerializeFrame(
    const TelemetryFrame& frame, char* output, size_t capacity,
    size_t& written) noexcept;
bool Telemetry_TestEscapeJson(
    const char* value, char* output, size_t capacity,
    size_t& written) noexcept;
struct TelemetryAnalyserLaunchPlanTest
{
    std::wstring scriptPath;
    std::wstring recordingPath;
    std::wstring commandLine;
};
struct TelemetryAnalyserLauncherTestSnapshot
{
    uint32_t launchRequests = 0;
    uint32_t launchCompletions = 0;
    uint32_t pythonResolutionAttempts = 0;
    uint32_t pyResolutionAttempts = 0;
    uint32_t pythonProcessAttempts = 0;
    uint32_t pyProcessAttempts = 0;
    TelemetryRecorderState stateAtQueue =
        TelemetryRecorderState::Unavailable;
    TelemetryRecorderState stateAtLastProcessAttempt =
        TelemetryRecorderState::Unavailable;
    bool sessionClosedAtLastProcessAttempt = false;
    bool standardHandlesValid = false;
    bool inheritHandles = false;
    bool restrictedHandleList = false;
    uint32_t creationFlags = 0;
    std::wstring applicationName;
    std::wstring commandLine;
    std::wstring recordingPath;
};
std::wstring Telemetry_TestQuoteWindowsArgument(const wchar_t* value);
std::vector<wchar_t> Telemetry_TestBuildUtf8Environment(
    const wchar_t* environmentBlock);
bool Telemetry_TestBuildAnalyserLaunchPlan(
    const wchar_t* moduleDirectory, const wchar_t* recordingPath,
    const wchar_t* interpreterPath, bool pyLauncher,
    TelemetryAnalyserLaunchPlanTest& plan);
void Telemetry_TestResetAnalyserLauncher();
void Telemetry_TestConfigureAnalyserLauncher(
    bool pythonAvailable, bool pyAvailable,
    bool pythonLaunchSucceeds, bool pyLaunchSucceeds);
void Telemetry_TestSetAnalyserModuleDirectory(const wchar_t* directory);
// Reports the recording directory the recorder itself constructed, so tests
// assert on the implementation's path rather than a locally assembled string.
const wchar_t* Telemetry_TestRecordingDirectoryPath() noexcept;
void Telemetry_TestUseRealAnalyserLauncher(bool enabled) noexcept;
void Telemetry_TestPauseAnalyserLaunch(bool enabled) noexcept;
bool Telemetry_TestAnalyserLaunchReached() noexcept;
void Telemetry_TestLaunchAnalyser(
    const wchar_t* moduleDirectory, const wchar_t* recordingPath) noexcept;
uint32_t Telemetry_TestAnalyserLaunchCompletions() noexcept;
TelemetryAnalyserLauncherTestSnapshot
Telemetry_TestGetAnalyserLauncherSnapshot();
#endif
