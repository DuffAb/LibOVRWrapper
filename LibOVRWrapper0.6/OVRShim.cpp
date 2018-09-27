#include "stdafx.h"

#undef OVR_PUBLIC_FUNCTION
#undef OVR_PUBLIC_CLASS
#undef OVR_PRIVATE_FUNCTION
#undef OVR_PRIVATE_CLASS

#if !defined(OVR_DLL_BUILD)
	#define OVR_DLL_BUILD
#endif

#include "../LibOVR0.6/Include/OVR_CAPI_0_6_0.h"
#include "../LibOVR0.6/Include/OVR_CAPI_D3D.h"

#include "shimhelper.h"
#include "OVRShim.h"

ovrLogCallback oldLogCallback;
void logcallback(uintptr_t userData, int level, const char* message) {
	oldLogCallback(level, message);
}

OVR_PUBLIC_FUNCTION(ovrResult) ovr_Initialize(const ovrInitParams* params) {
	BOOST_LOG_TRIVIAL(trace) << "ovr_Initialize";

	revInitParams p;
	ZeroMemory(&p, sizeof(revInitParams));

	p.Flags = params->Flags;
	p.RequestedMinorVersion = params->RequestedMinorVersion;
	if (params->LogCallback != nullptr) {
		p.LogCallback = logcallback;
		oldLogCallback = params->LogCallback;
	}

	initChains();

	//TODO: handle ovrInit_ServerOptional ?

	return rev_Initialize(&p);
}

OVR_PUBLIC_FUNCTION(void) ovr_Shutdown() {
	BOOST_LOG_TRIVIAL(trace) << "ovr_Shutdown";

	rev_Shutdown();
}

OVR_PUBLIC_FUNCTION(void) ovr_GetLastErrorInfo(ovrErrorInfo* errorInfo) {
	rev_GetLastErrorInfo((revErrorInfo*)errorInfo);
}

OVR_PUBLIC_FUNCTION(const char*) ovr_GetVersionString() {
	BOOST_LOG_TRIVIAL(trace) << "ovr_GetVersionString";

	return "0.6.0.1";
}

OVR_PUBLIC_FUNCTION(int) ovr_TraceMessage(int level, const char* message) {
	return rev_TraceMessage(level, message);
}

float globalRefreshRate = 90.0f;

OVR_PUBLIC_FUNCTION(ovrResult) ovrHmd_CreateDebug(ovrHmdType type, ovrHmd* pHmd) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_CreateDebug";

	return ovrHmd_Create(0, pHmd);
}

OVR_PUBLIC_FUNCTION(ovrResult) ovrHmd_Detect() {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_Detect";

	revHmdDesc desc = rev_GetHmdDesc(NULL);

	if (desc.Type == ovrHmd_None) {
		return 0;
	} else {
		return 1;
	}
}

revGraphicsLuid globalGraphicsLuid;

OVR_PUBLIC_FUNCTION(ovrResult) ovrHmd_Create(int index, ovrHmd* pHmd) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_Create";

	revSession pSession;
	revGraphicsLuid pLuid;

	ovrResult r = rev_Create(&pSession, &pLuid);

	if (!OVR_SUCCESS(r)) {
		return r;
	}

	globalGraphicsLuid = pLuid;

	revHmdDesc desc = rev_GetHmdDesc(pSession);

	ovrHmdDesc* d = (ovrHmdDesc*)malloc(sizeof(ovrHmdDesc));

	d->Handle = (ovrHmdStruct*)pSession;
	d->HmdCaps = ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction;
	d->TrackingCaps = desc.AvailableTrackingCaps;

	revTrackerDesc tracker = rev_GetTrackerDesc(pSession, 0);

	d->CameraFrustumFarZInMeters = tracker.FrustumFarZInMeters;
	d->CameraFrustumHFovInRadians = tracker.FrustumHFovInRadians;
	d->CameraFrustumNearZInMeters = tracker.FrustumNearZInMeters;
	d->CameraFrustumVFovInRadians = tracker.FrustumVFovInRadians;

	memcpy(d->DefaultEyeFov, desc.DefaultEyeFov, sizeof(d->DefaultEyeFov));
	globalRefreshRate = desc.DisplayRefreshRate;
	d->FirmwareMajor = desc.FirmwareMajor;
	d->FirmwareMinor = desc.FirmwareMinor;

	d->Manufacturer = (char*)malloc(sizeof(char) * 64);
	d->ProductName = (char*)malloc(sizeof(char) * 64);

	strncpy_s((char*)d->Manufacturer, sizeof(char) * 64, desc.Manufacturer, sizeof(desc.Manufacturer) / sizeof(desc.Manufacturer[0]));
	memcpy(d->MaxEyeFov, desc.MaxEyeFov, sizeof(d->MaxEyeFov));

	d->ProductId = desc.ProductId;
	strncpy_s((char*)d->ProductName, sizeof(char) * 64, desc.ProductName, sizeof(desc.ProductName) / sizeof(desc.ProductName[0]));
	d->Resolution = *(ovrSizei *)&desc.Resolution;

	strncpy_s(d->SerialNumber, sizeof(d->SerialNumber), desc.SerialNumber, sizeof(d->SerialNumber) / sizeof(d->SerialNumber[0]));
	d->VendorId = desc.VendorId;

	if (desc.Type > 8) {
		d->Type = (ovrHmdType)8;
	}
	else {
		d->Type = (ovrHmdType)desc.Type;
	}

	d->EyeRenderOrder[0] = ovrEye_Left;
	d->EyeRenderOrder[1] = ovrEye_Right;

	rev_SetTrackingOriginType(pSession, revTrackingOrigin_EyeLevel);

	*pHmd = d;

	return r;
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_Destroy(ovrHmd hmd) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_Destroy";

	rev_Destroy((revSession)hmd->Handle);
}

OVR_PUBLIC_FUNCTION(unsigned int) ovrHmd_GetEnabledCaps(ovrHmd hmd) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetEnabledCaps";

	revHmdDesc desc = rev_GetHmdDesc((revSession)hmd->Handle);

	//not possible anymore
	return desc.DefaultHmdCaps;
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_SetEnabledCaps(ovrHmd hmd, unsigned int hmdCaps) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_SetEnabledCaps";
	//not possible anymore
}

OVR_PUBLIC_FUNCTION(ovrResult) ovrHmd_ConfigureTracking(ovrHmd hmd, unsigned int requestedTrackingCaps,
	unsigned int requiredTrackingCaps) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_ConfigureTracking";
	//not used anymore
	return revSuccess;
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_RecenterPose(ovrHmd hmd) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_RecenterPose";

	rev_RecenterTrackingOrigin((revSession)hmd->Handle);
}

void copyPose(ovrPosef* dest, const revPosef* source) {
	dest->Orientation = *(ovrQuatf*)&source->Orientation;
	dest->Position = *(ovrVector3f*)&source->Position;
}

void copyPoseR(revPosef* dest, const ovrPosef* source) {
	dest->Orientation = *(revQuatf*)&source->Orientation;
	dest->Position = *(revVector3f*)&source->Position;
}

void copyPoseState(ovrPoseStatef* dest, const revPoseStatef* source) {
	dest->AngularAcceleration = *(ovrVector3f*)&source->AngularAcceleration;
	dest->AngularVelocity = *(ovrVector3f*)&source->AngularVelocity;
	dest->LinearAcceleration = *(ovrVector3f*)&source->LinearAcceleration;
	dest->LinearVelocity = *(ovrVector3f*)&source->LinearVelocity;
	copyPose(&(dest->ThePose), &(source->ThePose));
	dest->TimeInSeconds = source->TimeInSeconds;
}

double globalTrackingStateTime = 0.0;
uint32_t globalLastCameraFrameCounter = 0;

OVR_PUBLIC_FUNCTION(ovrTrackingState) ovrHmd_GetTrackingState(ovrHmd hmd, double absTime) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetTrackingState";

	revTrackingState state = rev_GetTrackingState((revSession)hmd->Handle, absTime, ovrTrue);
	revTrackerPose tpose = rev_GetTrackerPose((revSession)hmd->Handle, 0);	
	
	ovrTrackingState r;	
	copyPose(&(r.CameraPose), &(tpose.Pose));
	r.CameraPose.Orientation = *(ovrQuatf*)&tpose.Pose.Orientation;
	r.CameraPose.Position = *(ovrVector3f*)&tpose.Pose.Position;
	
	copyPoseState(&(r.HeadPose), &(state.HeadPose));

	//r.LastCameraFrameCounter not filled
	r.LastCameraFrameCounter = ++globalLastCameraFrameCounter;

	copyPose(&(r.LeveledCameraPose), &(tpose.LeveledPose));

	//r.RawSensorData not filled
	r.RawSensorData.Accelerometer.x = 0;
	r.RawSensorData.Accelerometer.y = 0;
	r.RawSensorData.Accelerometer.z = 0;

	r.RawSensorData.Gyro.x = 0;
	r.RawSensorData.Gyro.y = 0;
	r.RawSensorData.Gyro.z = 0;

	r.RawSensorData.Magnetometer.x = 0;
	r.RawSensorData.Magnetometer.y = 0;
	r.RawSensorData.Magnetometer.z = 0;

	r.RawSensorData.Temperature = 20.0f;

	r.RawSensorData.TimeInSeconds = (float)rev_GetTimeInSeconds();

	r.StatusFlags = state.StatusFlags | ovrStatus_CameraPoseTracked | ovrStatus_PositionConnected | ovrStatus_HmdConnected;
	
	globalTrackingStateTime = rev_GetTimeInSeconds();

	return r;
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_DestroySwapTextureSet(ovrHmd hmd, ovrSwapTextureSet* textureSet) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_DestroySwapTextureSet";

	rev_DestroyTextureSwapChain((revSession)hmd->Handle, getChain((revSession)hmd->Handle, textureSet)->swapChain);

	removeChain((revSession)hmd->Handle, textureSet);
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_DestroyMirrorTexture(ovrHmd hmd, ovrTexture* mirrorTexture) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_DestroyMirrorTexture";

	revMirrorTexture* mirror = getMirror();

	rev_DestroyMirrorTexture((revSession)hmd->Handle, *mirror);

	setMirror(NULL);

	if (mirrorTexture->Header.API == ovrRenderAPI_D3D11) {
		union ovrD3D11Texture* ovrtext = (union ovrD3D11Texture*)mirrorTexture;

		ovrtext->D3D11.pTexture->Release();
	}
}

OVR_PUBLIC_FUNCTION(ovrSizei) ovrHmd_GetFovTextureSize(ovrHmd hmd, ovrEyeType eye, ovrFovPort fov,
	float pixelsPerDisplayPixel) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetFovTextureSize";
	
	revFovPort fport;
	fport.DownTan = fov.DownTan;
	fport.LeftTan = fov.LeftTan;
	fport.RightTan = fov.RightTan;
	fport.UpTan = fov.UpTan;

	return *(ovrSizei *)&rev_GetFovTextureSize((revSession)hmd->Handle, (revEyeType)eye, fport, pixelsPerDisplayPixel);
}

OVR_PUBLIC_FUNCTION(ovrEyeRenderDesc) ovrHmd_GetRenderDesc(ovrHmd hmd,
	ovrEyeType eyeType, ovrFovPort fov) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetRenderDesc";

	revFovPort fport;
	fport.DownTan = fov.DownTan;
	fport.LeftTan = fov.LeftTan;
	fport.RightTan = fov.RightTan;
	fport.UpTan = fov.UpTan;

	revEyeRenderDesc desc = rev_GetRenderDesc((revSession)hmd->Handle, (revEyeType)eyeType, fport);

	ovrEyeRenderDesc r;

	r.DistortedViewport = *(ovrRecti*)&desc.DistortedViewport;
	r.Eye = (ovrEyeType)desc.Eye;
	r.Fov.DownTan = desc.Fov.DownTan;
	r.Fov.LeftTan = desc.Fov.LeftTan;
	r.Fov.RightTan = desc.Fov.RightTan;
	r.Fov.UpTan = desc.Fov.UpTan;
	r.HmdToEyeViewOffset = *(ovrVector3f*)&desc.HmdToEyeOffset;
	r.PixelsPerTanAngleAtCenter = *(ovrVector2f *)&desc.PixelsPerTanAngleAtCenter;

	return r;
}

revTextureSwapChain renderChain(revSession session, ovrSwapTextureSet* ts)
{
	ovrTextureSwapChainWrapper* chainwrapper = getChain(session, ts);

	int currentIndex = 0;
	rev_GetTextureSwapChainCurrentIndex(session, chainwrapper->swapChain, &currentIndex);

	CopyTexture(chainwrapper->pContext, chainwrapper->textures[currentIndex], &ts->Textures[ts->CurrentIndex]);	
	
	rev_CommitTextureSwapChain(session, chainwrapper->swapChain);

	return chainwrapper->swapChain;
}

OVR_PUBLIC_FUNCTION(ovrResult) ovrHmd_SubmitFrame(ovrHmd hmd, unsigned int frameIndex,
	const ovrViewScaleDesc* viewScaleDesc,
	ovrLayerHeader const * const * layerPtrList, unsigned int layerCount) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_SubmitFrame";

	//ovrLayerType 2, 6 do not exists anymore. max layer count is 16 instead of 32

	unsigned int trueLayerCount = 0;
	for (unsigned int i = 0;i < layerCount;i++) {
		if (layerPtrList[i] != nullptr) {
			trueLayerCount++;
		}
	}

	if (trueLayerCount > 16) {
		trueLayerCount = 16; //ignore layer counts > 16
	}	

	revLayerHeader* newlayers[16];
	
	unsigned int np = 0;

	for (unsigned int i = 0;i < layerCount;i++) {
		const ovrLayerHeader* layer = layerPtrList[i];

		if (layer == nullptr) {
			continue;
		}

		if (layer->Type == ovrLayerType_EyeFov) {
			const ovrLayerEyeFov* oldelayer = (const ovrLayerEyeFov*)layer;
			revLayerEyeFov *elayer = (revLayerEyeFov*)malloc(sizeof(revLayerEyeFov));

			//if both eyes use same swaptextureset
			if (oldelayer->ColorTexture[0] == oldelayer->ColorTexture[1]) {
				elayer->ColorTexture[0] = renderChain((revSession)hmd->Handle, oldelayer->ColorTexture[0]);
				elayer->ColorTexture[1] = elayer->ColorTexture[0];
			} else {
				elayer->ColorTexture[0] = renderChain((revSession)hmd->Handle, oldelayer->ColorTexture[0]);
				elayer->ColorTexture[1] = renderChain((revSession)hmd->Handle, oldelayer->ColorTexture[1]);
			}		
			
			elayer->Fov[0].DownTan = oldelayer->Fov[0].DownTan;
			elayer->Fov[0].LeftTan = oldelayer->Fov[0].LeftTan;
			elayer->Fov[0].UpTan = oldelayer->Fov[0].UpTan;
			elayer->Fov[0].RightTan = oldelayer->Fov[0].RightTan;
			elayer->Fov[1].DownTan = oldelayer->Fov[1].DownTan;
			elayer->Fov[1].LeftTan = oldelayer->Fov[1].LeftTan;
			elayer->Fov[1].UpTan = oldelayer->Fov[1].UpTan;
			elayer->Fov[1].RightTan = oldelayer->Fov[1].RightTan;

			elayer->Header.Flags = oldelayer->Header.Flags;
			elayer->Header.Type = (revLayerType)oldelayer->Header.Type;

			copyPoseR(&elayer->RenderPose[0], &oldelayer->RenderPose[0]);
			copyPoseR(&elayer->RenderPose[1], &oldelayer->RenderPose[1]);

			elayer->SensorSampleTime = globalTrackingStateTime;
			elayer->Viewport[0] = *(revRecti*)&oldelayer->Viewport[0];
			elayer->Viewport[1] = *(revRecti*)&oldelayer->Viewport[1];

			newlayers[np] = (revLayerHeader*)elayer;
		}		
		else if (layer->Type == ovrLayerType_QuadInWorld) {
			const ovrLayerQuad* oldelayer = (const ovrLayerQuad*)layer;
			revLayerQuad *elayer = (revLayerQuad*)malloc(sizeof(revLayerQuad));

			elayer->Header.Type = revLayerType_Quad;
			elayer->Header.Flags = layer->Flags;

			elayer->ColorTexture = renderChain((revSession)hmd->Handle, oldelayer->ColorTexture);
			
			copyPoseR(&elayer->QuadPoseCenter, &oldelayer->QuadPoseCenter);

			elayer->QuadSize = *(revVector2f *)&oldelayer->QuadSize;

			newlayers[np] = (revLayerHeader*)elayer;
		}
		else if (layer->Type == ovrLayerType_QuadHeadLocked) {
			const ovrLayerQuad* oldelayer = (const ovrLayerQuad*)layer;
			revLayerQuad *elayer = (revLayerQuad*)malloc(sizeof(revLayerQuad));
			
			elayer->Header.Type = revLayerType_Quad;
			elayer->Header.Flags = layer->Flags;
			elayer->Header.Flags |= revLayerFlag_HeadLocked;

			elayer->ColorTexture = renderChain((revSession)hmd->Handle, oldelayer->ColorTexture);

			copyPoseR(&elayer->QuadPoseCenter, &oldelayer->QuadPoseCenter);

			elayer->QuadSize = *(revVector2f *)&oldelayer->QuadSize;

			newlayers[np] = (revLayerHeader*)elayer;
		}
		else if (layer->Type == ovrLayerType_Disabled) {			
			revLayerHeader *elayer = (revLayerHeader*)malloc(sizeof(revLayerHeader));

			elayer->Flags = layer->Flags;
			elayer->Type = (revLayerType)layer->Type;

			newlayers[np] = (revLayerHeader*)elayer;
		}
		else {
			continue; //ignore unsupported layers
		}

		np++;

		if (np > 15) {
			break;
		}
	}
	
	ovrResult r = rev_SubmitFrame((revSession)hmd->Handle, frameIndex, (const revViewScaleDesc*)viewScaleDesc, newlayers, trueLayerCount);

	for (unsigned int i = 0;i < trueLayerCount;i++) {
		if(newlayers[i] != nullptr)
			free(newlayers[i]);
	}

	return r;
}

unsigned int globalFrameIndex = 0;
OVR_PUBLIC_FUNCTION(ovrFrameTiming) ovrHmd_GetFrameTiming(ovrHmd hmd, unsigned int frameIndex) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetFrameTiming";

	ovrFrameTiming timing;

	timing.AppFrameIndex = frameIndex;
	timing.DisplayMidpointSeconds = rev_GetPredictedDisplayTime((revSession)hmd->Handle, frameIndex);
	timing.DisplayFrameIndex = frameIndex; //todo: calculate this somehow?
	timing.FrameIntervalSeconds = 1.0f / globalRefreshRate; //todo: calculate this somehow?

	return timing;
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_ResetFrameTiming(ovrHmd hmd, unsigned int frameIndex) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_ResetFrameTiming";

	globalFrameIndex = frameIndex;
}

OVR_PUBLIC_FUNCTION(double) ovr_GetTimeInSeconds() {
	BOOST_LOG_TRIVIAL(trace) << "ovr_GetTimeInSeconds";

	return rev_GetTimeInSeconds();
}

OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_GetBool(ovrHmd hmd, const char* propertyName, ovrBool defaultVal) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetBool " << propertyName;

	return rev_GetBool((revSession)hmd->Handle, propertyName, defaultVal);
}

OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_SetBool(ovrHmd hmd, const char* propertyName, ovrBool value) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_SetBool " << propertyName;

	return rev_SetBool((revSession)hmd->Handle, propertyName, value);
}

OVR_PUBLIC_FUNCTION(int) ovrHmd_GetInt(ovrHmd hmd, const char* propertyName, int defaultVal) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetInt " << propertyName;

	return rev_GetInt((revSession)hmd->Handle, propertyName, defaultVal);
}

OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_SetInt(ovrHmd hmd, const char* propertyName, int value) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_SetInt " << propertyName;

	return rev_SetInt((revSession)hmd->Handle, propertyName, value);
}

OVR_PUBLIC_FUNCTION(float) ovrHmd_GetFloat(ovrHmd hmd, const char* propertyName, float defaultVal) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetFloat " << propertyName;

	if (strcmp(propertyName, OVR_KEY_IPD) == 0) {
		float values[2];
		rev_GetFloatArray((revSession)hmd->Handle, REV_KEY_NECK_TO_EYE_DISTANCE_, values, 2);

		return values[0] + values[1];
	}

	return rev_GetFloat((revSession)hmd->Handle, propertyName, defaultVal);
}

OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_SetFloat(ovrHmd hmd, const char* propertyName, float value) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_SetFloat " << propertyName;

	if (strcmp(propertyName, OVR_KEY_IPD) == 0) {
		return ovrTrue;
	} else if (strcmp(propertyName, "QueueAheadSeconds") == 0) {
		return ovrTrue;
	}	

	return rev_SetFloat((revSession)hmd->Handle, propertyName, value);
}

OVR_PUBLIC_FUNCTION(unsigned int) ovrHmd_GetFloatArray(ovrHmd hmd, const char* propertyName,
	float values[], unsigned int valuesCapacity) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetFloatArray " << propertyName;

	return rev_GetFloatArray((revSession)hmd->Handle, propertyName, values, valuesCapacity);
}

OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_SetFloatArray(ovrHmd hmd, const char* propertyName,
	const float values[], unsigned int valuesSize) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_SetFloatArray " << propertyName;

	return rev_SetFloatArray((revSession)hmd->Handle, propertyName, values, valuesSize);
}

OVR_PUBLIC_FUNCTION(const char*) ovrHmd_GetString(ovrHmd hmd, const char* propertyName,
	const char* defaultVal) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetString " << propertyName;

	return rev_GetString((revSession)hmd->Handle, propertyName, defaultVal);
}

OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_SetString(ovrHmd hmd, const char* propertyName,
	const char* value) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_SetString " << propertyName;

	return rev_SetString((revSession)hmd->Handle, propertyName, value);
}

OVR_PUBLIC_FUNCTION(ovrBool) ovr_InitializeRenderingShimVersion(int requestedMinorVersion)
{
	BOOST_LOG_TRIVIAL(trace) << "ovr_InitializeRenderingShimVersion";

	return ovrTrue;
}

//these two functions below are just for debugging purposes
/*
OVR_PUBLIC_FUNCTION(ovrResult) ovrHmd_GetTextureSwapChainCurrentIndex(ovrHmd session, ovrSwapTextureSet* textureSet, int* currentIndex) {
	revTextureSwapChain chain = getChain((revSession)session, textureSet)->swapChain;

	return rev_GetTextureSwapChainCurrentIndex((revSession)session, chain, currentIndex);
}

OVR_PUBLIC_FUNCTION(ovrResult) ovrHmd_CommitTextureSwapChain(ovrHmd session, ovrSwapTextureSet* textureSet) {
	return rev_CommitTextureSwapChain((revSession)session, getChain((revSession)session, textureSet)->swapChain);
}
*/



