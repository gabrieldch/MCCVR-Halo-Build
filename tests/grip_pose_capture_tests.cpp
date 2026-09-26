#include <Windows.h>
#include <openxr/openxr.h>
#include <cmath>
#include <cstdio>

namespace
{
unsigned checks{}, failures{};
unsigned stateCalls{}, locateCalls{};
XrResult stateResult=XR_SUCCESS, locateResult=XR_SUCCESS;
XrBool32 stateActive=XR_TRUE;
XrSpaceLocationFlags locateFlags=XR_SPACE_LOCATION_POSITION_VALID_BIT;
XrVector3f locatePosition{1.0f, 2.0f, 3.0f};
XrAction lastStateAction=XR_NULL_HANDLE;
XrPath lastStateSubaction=XR_NULL_PATH;
XrSpace lastLocateSpace=XR_NULL_HANDLE, lastLocateBase=XR_NULL_HANDLE;
XrTime lastLocateTime=0;
XrAction g_supportGripPoseAction=reinterpret_cast<XrAction>(uintptr_t{11});
XrSession g_session=reinterpret_cast<XrSession>(uintptr_t{12});
XrSpace g_localSpace=reinterpret_cast<XrSpace>(uintptr_t{13});

void Check(bool value,const char* message)
{
    ++checks;
    if (!value)
    {
        ++failures;
        std::fprintf(stderr,"FAIL: %s\n",message);
    }
}

bool SamePosition(const XrVector3f& value, const XrVector3f& expected)
{
    return value.x==expected.x && value.y==expected.y && value.z==expected.z;
}

void Reset()
{
    stateCalls=locateCalls=0;
    stateResult=locateResult=XR_SUCCESS;
    stateActive=XR_TRUE;
    locateFlags=XR_SPACE_LOCATION_POSITION_VALID_BIT;
    locatePosition={1.0f,2.0f,3.0f};
    lastStateAction=XR_NULL_HANDLE;
    lastStateSubaction=XR_NULL_PATH;
    lastLocateSpace=lastLocateBase=XR_NULL_HANDLE;
    lastLocateTime=0;
}

void CheckFailurePreservesOutput(const char* label, bool result,
    const XrVector3f& output, const XrVector3f& sentinel)
{
    Check(!result,label);
    Check(SamePosition(output,sentinel),"Failed grip capture preserves the output sentinel");
}
}

extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrGetActionStatePose(
    XrSession, const XrActionStateGetInfo* info, XrActionStatePose* state)
{
    ++stateCalls;
    lastStateAction=info->action;
    lastStateSubaction=info->subactionPath;
    state->isActive=stateActive;
    return stateResult;
}

extern "C" XRAPI_ATTR XrResult XRAPI_CALL xrLocateSpace(
    XrSpace space, XrSpace base, XrTime time, XrSpaceLocation* location)
{
    ++locateCalls;
    lastLocateSpace=space;
    lastLocateBase=base;
    lastLocateTime=time;
    location->locationFlags=locateFlags;
    location->pose.position=locatePosition;
    return locateResult;
}

namespace
{
#include "grip_pose_capture_functions.inl"

void TestNoQueryShortCircuits()
{
    const XrVector3f sentinel{9.0f,8.0f,7.0f};
    XrVector3f output=sentinel;
    const XrPath hand=static_cast<XrPath>(21);
    const XrSpace space=reinterpret_cast<XrSpace>(uintptr_t{22});
    const XrTime time=123456;

    Reset();
    CheckFailurePreservesOutput("Disabled grip capture is rejected",
        TryLocateSupportGripPosition(false,hand,space,time,output),output,sentinel);
    Check(stateCalls==0&&locateCalls==0,"Disabled grip capture makes no OpenXR calls");

    Reset();
    g_supportGripPoseAction=XR_NULL_HANDLE;
    output=sentinel;
    CheckFailurePreservesOutput("Missing grip action is rejected",
        TryLocateSupportGripPosition(true,hand,space,time,output),output,sentinel);
    Check(stateCalls==0&&locateCalls==0,"Missing grip action makes no OpenXR calls");
    g_supportGripPoseAction=reinterpret_cast<XrAction>(uintptr_t{11});

    Reset();
    output=sentinel;
    CheckFailurePreservesOutput("Missing grip space is rejected",
        TryLocateSupportGripPosition(true,hand,XR_NULL_HANDLE,time,output),output,sentinel);
    Check(stateCalls==0&&locateCalls==0,"Missing grip space makes no OpenXR calls");
}

void TestStateShortCircuitsLocate()
{
    const XrPath hand=static_cast<XrPath>(21);
    const XrSpace space=reinterpret_cast<XrSpace>(uintptr_t{22});
    const XrTime time=123456;
    const XrVector3f sentinel{9.0f,8.0f,7.0f};
    XrVector3f output=sentinel;

    Reset();
    stateResult=XR_ERROR_RUNTIME_FAILURE;
    CheckFailurePreservesOutput("Grip action-state failure is rejected",
        TryLocateSupportGripPosition(true,hand,space,time,output),output,sentinel);
    Check(stateCalls==1&&locateCalls==0,"Action-state failure prevents locate");

    Reset();
    stateActive=XR_FALSE;
    output=sentinel;
    CheckFailurePreservesOutput("Inactive grip action is rejected",
        TryLocateSupportGripPosition(true,hand,space,time,output),output,sentinel);
    Check(stateCalls==1&&locateCalls==0,"Inactive action prevents locate");
}

void TestLocateAdmission()
{
    const XrPath hand=static_cast<XrPath>(21);
    const XrSpace space=reinterpret_cast<XrSpace>(uintptr_t{22});
    const XrTime time=123456;
    const XrVector3f sentinel{9.0f,8.0f,7.0f};
    XrVector3f output=sentinel;

    Reset();
    locateResult=XR_ERROR_RUNTIME_FAILURE;
    CheckFailurePreservesOutput("Locate failure is rejected",
        TryLocateSupportGripPosition(true,hand,space,time,output),output,sentinel);
    Check(stateCalls==1&&locateCalls==1,"Locate failure reaches locate exactly once");

    Reset();
    locateFlags=0;
    output=sentinel;
    CheckFailurePreservesOutput("Missing position validity is rejected",
        TryLocateSupportGripPosition(true,hand,space,time,output),output,sentinel);

    Reset();
    locateFlags=XR_SPACE_LOCATION_POSITION_VALID_BIT;
    output=sentinel;
    const XrVector3f expected{4.5f,-2.25f,8.75f};
    locatePosition=expected;
    Check(TryLocateSupportGripPosition(true,hand,space,time,output),
        "Position-valid grip capture succeeds without orientation validity");
    Check(SamePosition(output,expected),"Successful grip capture copies the exact position");
    Check(lastStateAction==g_supportGripPoseAction&&lastStateSubaction==hand,
        "Grip query uses the support action and requested physical hand");
    Check(lastLocateSpace==space&&lastLocateBase==g_localSpace&&lastLocateTime==time,
        "Grip locate uses the requested space, local base, and predicted display time");
}

void TestFiniteValues()
{
    const XrPath hand=static_cast<XrPath>(21);
    const XrSpace space=reinterpret_cast<XrSpace>(uintptr_t{22});
    const XrTime time=123456;
    const XrVector3f sentinel{9.0f,8.0f,7.0f};
    const float invalid[]{NAN,INFINITY,NAN};
    for (int axis=0;axis<3;++axis)
    {
        Reset();
        locatePosition={1.0f,2.0f,3.0f};
        if (axis==0) locatePosition.x=invalid[0];
        if (axis==1) locatePosition.y=invalid[1];
        if (axis==2) locatePosition.z=invalid[2];
        XrVector3f output=sentinel;
        CheckFailurePreservesOutput("Non-finite grip position is rejected",
            TryLocateSupportGripPosition(true,hand,space,time,output),output,sentinel);
        Check(stateCalls==1&&locateCalls==1,"Non-finite position is rejected after one query and locate");
    }
}
}

int main()
{
    TestNoQueryShortCircuits();
    TestStateShortCircuitsLocate();
    TestLocateAdmission();
    TestFiniteValues();
    std::printf("Support grip capture fixture: %u checks, %u failures\n",checks,failures);
    return failures ? 1 : 0;
}
