#pragma once

#include "stdafx.h"

#if !defined(OVR_DLL_BUILD)
#define OVR_DLL_BUILD
#endif

#include "../LibOVR0.5/Include/OVR_CAPI_0_5_0.h"

void copyPose(ovrPosef* dest, const revPosef* source);
void copyPoseState(ovrPoseStatef* dest, const revPoseStatef* source);