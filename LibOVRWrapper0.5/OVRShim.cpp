#include "stdafx.h"

#undef OVR_PUBLIC_FUNCTION
#undef OVR_PUBLIC_CLASS
#undef OVR_PRIVATE_FUNCTION
#undef OVR_PRIVATE_CLASS

#if !defined(OVR_DLL_BUILD)
	#define OVR_DLL_BUILD
#endif

struct ovrHmdStruct {
	int dummy;
};

#include "../LibOVR0.5/Include/OVR_CAPI_0_5_0.h"
#include "../LibOVR0.5/Include/OVR_CAPI_D3D.h"

#include "shimhelper.h"
#include "OVRShim.h"

ovrLogCallback oldLogCallback;
void logcallback(uintptr_t userData, int level, const char* message) {
	oldLogCallback(level, message);
}

double initTime;
OVR_PUBLIC_FUNCTION(ovrBool) ovr_Initialize(const ovrInitParams* params) {
	BOOST_LOG_TRIVIAL(trace) << "ovr_Initialize";

	const revInitParams* pp = nullptr;
	if (params) {
		revInitParams p;
		ZeroMemory(&p, sizeof(revInitParams));

		p.Flags = params->Flags;
		p.RequestedMinorVersion = params->RequestedMinorVersion;
		if (params->LogCallback != nullptr) {
			p.LogCallback = logcallback;
			oldLogCallback = params->LogCallback;
		}
		pp = &p;
	}
	initChains();

	//TODO: handle ovrInit_ServerOptional ?
	bool r = REV_SUCCESS(rev_Initialize(pp));

	initTime = rev_GetTimeInSeconds();

	return r;
}

OVR_PUBLIC_FUNCTION(void) ovr_Shutdown() {
	BOOST_LOG_TRIVIAL(trace) << "ovr_Shutdown";

	ShutdownD3D11();

	rev_Shutdown();
}

OVR_PUBLIC_FUNCTION(const char*) ovr_GetVersionString() {
	BOOST_LOG_TRIVIAL(trace) << "ovr_GetVersionString";

	return "0.5.0";
}

OVR_PUBLIC_FUNCTION(int) ovr_TraceMessage(int level, const char* message) {
	BOOST_LOG_TRIVIAL(trace) << "ovr_TraceMessage";

	return rev_TraceMessage(level, message);
}

OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_StartPerfLog(ovrHmd hmd, const char* fileName, const char* userData1)
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_StartPerfLog";

	return ovrFalse;
}

OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_StopPerfLog(ovrHmd hmd)
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_StopPerfLog";

	return ovrFalse;
}

float globalRefreshRate = 90.0f;

OVR_PUBLIC_FUNCTION(ovrHmd) ovrHmd_CreateDebug(ovrHmdType type) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_CreateDebug";

	return ovrHmd_Create(0);
}

OVR_PUBLIC_FUNCTION(const char*) ovrHmd_GetLastError(ovrHmd hmd) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetLastError";

	static revErrorInfo LastError;
	rev_GetLastErrorInfo(&LastError);
	return LastError.ErrorString;
}

OVR_PUBLIC_FUNCTION(int) ovrHmd_Detect() {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_Detect";

	revHmdDesc desc = rev_GetHmdDesc(NULL);

	if (desc.Type == ovrHmd_None) {
		return 0;
	} else {
		return 1;
	}
}

revGraphicsLuid globalGraphicsLuid;
HWND globalMirrorWindowHandle;
bool globalDisableMirror = false;
ovrRenderAPIConfig globalAPIConfig;
ovrFovPort globalEyeFOV[2];
ovrEyeRenderDesc globalEyeRenderDesc[2];

OVR_PUBLIC_FUNCTION(ovrHmd) ovrHmd_Create(int index) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_Create";

	revSession pSession;
	revGraphicsLuid pLuid;

	globalDisableMirror = false;

	revResult r = rev_Create(&pSession, &pLuid);

	if (!REV_SUCCESS(r)) {
		return NULL;
	}

	globalGraphicsLuid = pLuid;

	revHmdDesc desc = rev_GetHmdDesc(pSession);

	ovrHmdDesc* d = (ovrHmdDesc*)malloc(sizeof(ovrHmdDesc));
	ZeroMemory(d, sizeof(ovrHmdDesc));

	d->Handle = (ovrHmdStruct *)pSession;
	d->HmdCaps = ovrHmdCap_Present | ovrHmdCap_Available | ovrHmdCap_Captured | ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction;
	d->DistortionCaps = ovrDistortionCap_Vignette | ovrDistortionCap_Overdrive; // ovrDistortionCap_TimeWarp | 
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

	d->DisplayDeviceName = "\\\\.\\DISPLAY0";

	d->EyeRenderOrder[0] = ovrEye_Left;
	d->EyeRenderOrder[1] = ovrEye_Right;

	d->DisplayId = 0;
	d->WindowsPos.x = 0;
	d->WindowsPos.y = 0;

	rev_SetTrackingOriginType(pSession, revTrackingOrigin_EyeLevel);

	return d;
}
#if 0 //ldf
OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_GetMirrorTexture(ovrHmd hmd, ovrTexture** outMirrorTexture)
{
	GetMirrorTexture(outMirrorTexture);

	return 1;
}
#endif
OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_AttachToWindow(ovrHmd hmd, void* window,
	const ovrRecti* destMirrorRect, const ovrRecti* sourceRenderTargetRect)
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_AttachToWindow";

	globalMirrorWindowHandle = (HWND)window;
	//CreateMirrorTextureD3D11((revSession)hmd->Handle, destMirrorRect, sourceRenderTargetRect);
	//todo: possibly save sourceRenderTargetRect which is the area of the mirror window we should draw to (or NULL for whole mirror window)
	return ovrTrue;
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_Destroy(ovrHmd hmd) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_Destroy";

	rev_Destroy((revSession)hmd->Handle);
}

OVR_PUBLIC_FUNCTION(unsigned int) ovrHmd_GetEnabledCaps(ovrHmd hmd) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetEnabledCaps";

	revHmdDesc desc = rev_GetHmdDesc((revSession)hmd->Handle);

	//not possible anymore
	return desc.DefaultHmdCaps | ovrHmdCap_Present | ovrHmdCap_Available | ovrHmdCap_Captured | ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction;
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_SetEnabledCaps(ovrHmd hmd, unsigned int hmdCaps) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_SetEnabledCaps";
	globalDisableMirror = hmdCaps & ovrHmdCap_NoMirrorToWindow;
}

OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_ConfigureTracking(ovrHmd hmd, unsigned int requestedTrackingCaps,
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
	ZeroMemory(&r, sizeof(ovrTrackingState));

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

OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_ConfigureRendering(ovrHmd hmd, const ovrRenderAPIConfig* apiConfig, unsigned int distortionCaps,
	const ovrFovPort eyeFovIn[2], ovrEyeRenderDesc eyeRenderDescOut[2])
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_ConfigureRendering";

	ovrEyeRenderDesc r[2];
	for (int eye = 0; eye < 2; eye++) {
		r[eye] = ovrHmd_GetRenderDesc(hmd, (ovrEyeType)eye, eyeFovIn[eye]);
		if (eyeRenderDescOut)
			eyeRenderDescOut[eye] = r[eye];
	}
	if (apiConfig) {
		globalAPIConfig = *apiConfig;
		if (apiConfig->Header.API == ovrRenderAPI_D3D11) {
			ConfigureD3D11((revSession)(hmd->Handle), apiConfig, distortionCaps, eyeFovIn, r);
		} else {
			BOOST_LOG_TRIVIAL(trace) << "ovrHmd_ConfigureRendering error. unsupported rendering API " << apiConfig->Header.API;
			return ovrFalse;
		}
	}
	return ovrTrue;
}

unsigned int globalFrameIndex = 0;

OVR_PUBLIC_FUNCTION(ovrFrameTiming) ovrHmd_BeginFrame(ovrHmd hmd, unsigned int frameIndex)
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_BeginFrame index " << frameIndex;

	globalFrameIndex = frameIndex;
	return ovrHmd_GetFrameTiming(hmd, frameIndex);
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_EndFrame(ovrHmd hmd, const ovrPosef renderPose[2], const ovrTexture eyeTexture[2])
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_EndFrame";

	// This is where we do the actual rendering, using the eye textures that they passed in.
	if (!eyeTexture || eyeTexture[0].Header.API != ovrRenderAPI_D3D11)
		return;
	PresentD3D11((revSession)(hmd->Handle), renderPose, eyeTexture);
}

OVR_PUBLIC_FUNCTION(ovrFrameTiming) ovrHmd_BeginFrameTiming(ovrHmd hmd, unsigned int frameIndex)
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_BeginFrameTiming";

	return ovrHmd_BeginFrame(hmd, frameIndex);
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_EndFrameTiming(ovrHmd hmd)
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_EndFrameTiming";

	// This is where we do the actual non-rendering. They are trying to do the distortion themselves.
}



OVR_PUBLIC_FUNCTION(ovrPosef) ovrHmd_GetHmdPosePerEye(ovrHmd hmd, ovrEyeType eye)
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetHmdPosePerEye";

	// todo
	{
		static ovrPosef nullPose;
		memset(&nullPose, 0, sizeof(nullPose));
		nullPose.Orientation.w = 1.0f; // Return a proper quaternion.
		return nullPose;
	}
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

OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_CreateDistortionMesh(ovrHmd hmd, ovrEyeType eyeType, ovrFovPort fov,
	unsigned int distortionCaps, ovrDistortionMesh *meshData)
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_CreateDistortionMesh";

	//todo
	return ovrFalse;
}

OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_CreateDistortionMeshDebug(ovrHmd hmd, ovrEyeType eyeType, ovrFovPort fov, unsigned int distortionCaps,
	ovrDistortionMesh *meshData, float debugEyeReliefOverrideInMetres)
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_CreateDistortionMeshDebug";

	//todo
	return ovrFalse;
}

OVR_PUBLIC_FUNCTION(int) ovrHmd_AddDistortionTimeMeasurement(ovrHmd hmd, double distortionTimeSeconds) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_AddDistortionTimeMeasurement";
	// not sure what to do or return here
	return ovrFalse;
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_DestroyDistortionMesh(ovrDistortionMesh* meshData)
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_DestroyDistortionMesh";

	//todo - since we can't create a distortion mesh, there's never going to be anything to destroy here
}

OVR_PUBLIC_FUNCTION(int) ovrHmd_RegisterPostDistortionCallback(ovrHmd hmd, void *callback) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_RegisterPostDistortionCallback";
	return 0;
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_GetRenderScaleAndOffset(ovrFovPort fov, ovrSizei textureSize, ovrRecti renderViewport,
	ovrVector2f uvScaleOffsetOut[2])
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetRenderScaleAndOffset";

	// todo: set values to something sensible?
	return;
}


/// It is generally expected that the following holds:
/// ThisFrameSeconds < TimewarpPointSeconds < NextFrameSeconds < 
/// EyeScanoutSeconds[EyeOrder[0]] <= ScanoutMidpointSeconds <= EyeScanoutSeconds[EyeOrder[1]].
OVR_PUBLIC_FUNCTION(ovrFrameTiming) ovrHmd_GetFrameTiming(ovrHmd hmd, unsigned int frameIndex) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetFrameTiming index " << frameIndex;

	ovrFrameTiming timing;
	timing.ScanoutMidpointSeconds = rev_GetPredictedDisplayTime((revSession)hmd->Handle, frameIndex);
	timing.ThisFrameSeconds = rev_GetTimeInSeconds();
	timing.DeltaSeconds = 1.0f / globalRefreshRate; //todo: calculate this somehow?
	timing.NextFrameSeconds = timing.ThisFrameSeconds + timing.DeltaSeconds;
	timing.EyeScanoutSeconds[0] = timing.ScanoutMidpointSeconds;
	timing.EyeScanoutSeconds[1] = timing.ScanoutMidpointSeconds;
	timing.TimewarpPointSeconds = timing.ScanoutMidpointSeconds;
	
	return timing;
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_ResetFrameTiming(ovrHmd hmd, unsigned int frameIndex) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_ResetFrameTiming";

	globalFrameIndex = frameIndex;
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_GetEyeTimewarpMatrices(ovrHmd hmd, ovrEyeType eye, ovrPosef renderPose, ovrMatrix4f twmOut[2])
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetEyeTimewarpMatrices";
	// todo
	
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_GetEyeTimewarpMatricesDebug(ovrHmd hmd, ovrEyeType eye, ovrPosef renderPose,
	ovrQuatf playerTorsoMotion, ovrMatrix4f twmOut[2], double debugTimingOffsetInSeconds)
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetEyeTimewarpMatricesDebug";

	// todo
}

OVR_PUBLIC_FUNCTION(double) ovr_GetTimeInSeconds() {
	BOOST_LOG_TRIVIAL(trace) << "ovr_GetTimeInSeconds";

	return rev_GetTimeInSeconds();
}

OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_ProcessLatencyTest(ovrHmd hmd, unsigned char rgbColorOut[3])
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_ProcessLatencyTest";

	return ovrFalse;
}

OVR_PUBLIC_FUNCTION(const char*) ovrHmd_GetLatencyTestResult(ovrHmd hmd)
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetLatencyTestResult";
	return "(Unable to load LibOVR)";
}

OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_GetLatencyTest2DrawColor(ovrHmd hmd, unsigned char rgbColorOut[3])
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetLatencyTest2DrawColor";

	return ovrFalse;
}

/* hidden functions */
OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_GetLatencyTestDrawColor(ovrHmd hmd, unsigned char rgbColorOut[3])
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetLatencyTestDrawColor";

	//todo: right parameters

	return ovrFalse;
}

/* hidden functions */
OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_GetMeasuredLatencyTest2(ovrHmd hmd)
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetMeasuredLatencyTest2";

	//todo: right parameters

	return ovrFalse;
}

OVR_PUBLIC_FUNCTION(void) ovrHmd_GetHSWDisplayState(ovrHmd hmd, ovrHSWDisplayState *hasWarningState)
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_GetHSWDisplayState";

	if (hasWarningState) {
		hasWarningState->Displayed = false;
		hasWarningState->DismissibleTime = initTime;
		hasWarningState->StartTime = initTime;		
	}
}

OVR_PUBLIC_FUNCTION(ovrBool) ovrHmd_DismissHSWDisplay(ovrHmd hmd)
{
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_DismissHSWDisplay";

	return ovrTrue;
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
		rev_GetFloatArray((revSession)hmd->Handle,	REV_KEY_NECK_TO_EYE_DISTANCE_, values, 2);

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
	float values[], unsigned int arraySize) {
	BOOST_LOG_TRIVIAL(trace) << "ovrHmd_SetFloatArray " << propertyName;

	return rev_SetFloatArray((revSession)hmd->Handle, propertyName, values, arraySize);
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

OVR_PUBLIC_FUNCTION(ovrBool) ovr_InitializeRenderingShim()
{
	BOOST_LOG_TRIVIAL(trace) << "ovr_InitializeRenderingShim";

	return ovrTrue;
}

OVR_PUBLIC_FUNCTION(ovrBool) ovr_InitializeRenderingShimVersion(int requestedMinorVersion)
{
	BOOST_LOG_TRIVIAL(trace) << "ovr_InitializeRenderingShimVersion";

	return ovrTrue;
}

OVR_PUBLIC_FUNCTION(IUnknown *) ovr_GetDX11SwapChain()
{
	BOOST_LOG_TRIVIAL(trace) << "ovr_GetDX11SwapChain";
	return NULL;
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



