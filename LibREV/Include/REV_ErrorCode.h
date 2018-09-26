/********************************************************************************//**
\file  REV_ErrorCode.h
\brief     This header provides LibOVR error code declarations.
\copyright Copyright 2015-2016 Oculus VR, LLC All Rights reserved.
*************************************************************************************/

#ifndef REV_ErrorCode__h
#define REV_ErrorCode__h


#include "REV_Version.h"
#include <stdint.h>







#ifndef REV_RESULT_DEFINED
#define REV_RESULT_DEFINED ///< Allows revResult to be independently defined.
/// API call results are represented at the highest level by a single revResult.
typedef int32_t revResult;
#endif


/// \brief Indicates if an revResult indicates success.
///
/// Some functions return additional successful values other than revSucces and
/// require usage of this macro to indicate successs.
///
#if !defined(REV_SUCCESS)
    #define REV_SUCCESS(result) (result >= 0)
#endif


/// \brief Indicates if an revResult indicates an unqualified success.
///
/// This is useful for indicating that the code intentionally wants to
/// check for result == revSuccess as opposed to REV_SUCCESS(), which
/// checks for result >= revSuccess.
///
#if !defined(REV_UNQUALIFIED_SUCCESS)
    #define REV_UNQUALIFIED_SUCCESS(result) (result == revSuccess)
#endif


/// \brief Indicates if an revResult indicates failure.
///
#if !defined(REV_FAILURE)
    #define REV_FAILURE(result) (!REV_SUCCESS(result))
#endif


// Success is a value greater or equal to 0, while all error types are negative values.
#ifndef REV_SUCCESS_DEFINED
#define REV_SUCCESS_DEFINED ///< Allows revResult to be independently defined.
typedef enum revSuccessType_
{
    /// This is a general success result. Use REV_SUCCESS to test for success.
    revSuccess = 0,

    /// Returned from a call to SubmitFrame. The call succeeded, but what the app
    /// rendered will not be visible on the HMD. Ideally the app should continue
    /// calling SubmitFrame, but not do any rendering. When the result becomes
    /// revSuccess, rendering should continue as usual.
    revSuccess_NotVisible                 = 1000,

    revSuccess_HMDFirmwareMismatch        = 4100,   ///< The HMD Firmware is out of date but is acceptable.
    revSuccess_TrackerFirmwareMismatch    = 4101,   ///< The Tracker Firmware is out of date but is acceptable.
    revSuccess_ControllerFirmwareMismatch = 4104,   ///< The controller firmware is out of date but is acceptable.
    revSuccess_TrackerDriverNotFound      = 4105,   ///< The tracker driver interface was not found. Can be a temporary error

} revSuccessType;
#endif


typedef enum revErrorType_
{
    /* General errors */
    revError_MemoryAllocationFailure    = -1000,   ///< Failure to allocate memory.
    revError_SocketCreationFailure      = -1001,   ///< Failure to create a socket.
    revError_InvalidSession             = -1002,   ///< Invalid revSession parameter provided.
    revError_Timeout                    = -1003,   ///< The operation timed out.
    revError_NotInitialized             = -1004,   ///< The system or component has not been initialized.
    revError_InvalidParameter           = -1005,   ///< Invalid parameter provided. See error info or log for details.
    revError_ServiceError               = -1006,   ///< Generic service error. See error info or log for details.
    revError_NoHmd                      = -1007,   ///< The given HMD doesn't exist.
    revError_Unsupported                = -1009,   ///< Function call is not supported on this hardware/software
    revError_DeviceUnavailable          = -1010,   ///< Specified device type isn't available.
    revError_InvalidHeadsetOrientation  = -1011,   ///< The headset was in an invalid orientation for the requested operation (e.g. vertically oriented during rev_RecenterPose).
    revError_ClientSkippedDestroy       = -1012,   ///< The client failed to call rev_Destroy on an active session before calling rev_Shutdown. Or the client crashed.
    revError_ClientSkippedShutdown      = -1013,   ///< The client failed to call rev_Shutdown or the client crashed.

    /* Audio error range, reserved for Audio errors. */
    revError_AudioReservedBegin         = -2000,   ///< First Audio error.
    revError_AudioDeviceNotFound        = -2001,   ///< Failure to find the specified audio device.
    revError_AudioComError              = -2002,   ///< Generic COM error.
    revError_AudioReservedEnd           = -2999,   ///< Last Audio error.

    /* Initialization errors. */
    revError_Initialize                 = -3000,   ///< Generic initialization error.
    revError_LibLoad                    = -3001,   ///< Couldn't load LibOVRRT.
    revError_LibVersion                 = -3002,   ///< LibOVRRT version incompatibility.
    revError_ServiceConnection          = -3003,   ///< Couldn't connect to the OVR Service.
    revError_ServiceVersion             = -3004,   ///< OVR Service version incompatibility.
    revError_IncompatibleOS             = -3005,   ///< The operating system version is incompatible.
    revError_DisplayInit                = -3006,   ///< Unable to initialize the HMD display.
    revError_ServerStart                = -3007,   ///< Unable to start the server. Is it already running?
    revError_Reinitialization           = -3008,   ///< Attempting to re-initialize with a different version.
    revError_MismatchedAdapters         = -3009,   ///< Chosen rendering adapters between client and service do not match
    revError_LeakingResources           = -3010,   ///< Calling application has leaked resources
    revError_ClientVersion              = -3011,   ///< Client version too old to connect to service
    revError_OutOfDateOS                = -3012,   ///< The operating system is out of date.
    revError_OutOfDateGfxDriver         = -3013,   ///< The graphics driver is out of date.
    revError_IncompatibleGPU            = -3014,   ///< The graphics hardware is not supported
    revError_NoValidVRDisplaySystem     = -3015,   ///< No valid VR display system found.
    revError_Obsolete                   = -3016,   ///< Feature or API is obsolete and no longer supported.
    revError_DisabledOrDefaultAdapter   = -3017,   ///< No supported VR display system found, but disabled or driverless adapter found.
    revError_HybridGraphicsNotSupported = -3018,   ///< The system is using hybrid graphics (Optimus, etc...), which is not support.
    revError_DisplayManagerInit         = -3019,   ///< Initialization of the DisplayManager failed.
    revError_TrackerDriverInit          = -3020,   ///< Failed to get the interface for an attached tracker

    /* Hardware errors */
    revError_InvalidBundleAdjustment    = -4000,   ///< Headset has no bundle adjustment data.
    revError_USBBandwidth               = -4001,   ///< The USB hub cannot handle the camera frame bandwidth.
    revError_USBEnumeratedSpeed         = -4002,   ///< The USB camera is not enumerating at the correct device speed.
    revError_ImageSensorCommError       = -4003,   ///< Unable to communicate with the image sensor.
    revError_GeneralTrackerFailure      = -4004,   ///< We use this to report various sensor issues that don't fit in an easily classifiable bucket.
    revError_ExcessiveFrameTruncation   = -4005,   ///< A more than acceptable number of frames are coming back truncated.
    revError_ExcessiveFrameSkipping     = -4006,   ///< A more than acceptable number of frames have been skipped.
    revError_SyncDisconnected           = -4007,   ///< The sensor is not receiving the sync signal (cable disconnected?).
    revError_TrackerMemoryReadFailure   = -4008,   ///< Failed to read memory from the sensor.
    revError_TrackerMemoryWriteFailure  = -4009,   ///< Failed to write memory from the sensor.
    revError_TrackerFrameTimeout        = -4010,   ///< Timed out waiting for a camera frame.
    revError_TrackerTruncatedFrame      = -4011,   ///< Truncated frame returned from sensor.
    revError_TrackerDriverFailure       = -4012,   ///< The sensor driver has encountered a problem.
    revError_TrackerNRFFailure          = -4013,   ///< The sensor wireless subsystem has encountered a problem.
    revError_HardwareGone               = -4014,   ///< The hardware has been unplugged
    revError_NordicEnabledNoSync        = -4015,   ///< The nordic indicates that sync is enabled but it is not sending sync pulses
    revError_NordicSyncNoFrames         = -4016,   ///< It looks like we're getting a sync signal, but no camera frames have been received
    revError_CatastrophicFailure        = -4017,   ///< A catastrophic failure has occurred.  We will attempt to recover by resetting the device

    revError_HMDFirmwareMismatch        = -4100,   ///< The HMD Firmware is out of date and is unacceptable.
    revError_TrackerFirmwareMismatch    = -4101,   ///< The sensor Firmware is out of date and is unacceptable.
    revError_BootloaderDeviceDetected   = -4102,   ///< A bootloader HMD is detected by the service.
    revError_TrackerCalibrationError    = -4103,   ///< The sensor calibration is missing or incorrect.
    revError_ControllerFirmwareMismatch = -4104,   ///< The controller firmware is out of date and is unacceptable.

    revError_IMUTooManyLostSamples      = -4200,   ///< Too many lost IMU samples.
    revError_IMURateError               = -4201,   ///< IMU rate is outside of the expected range.
    revError_FeatureReportFailure       = -4202,   ///< A feature report has failed.

    /* Synchronization errors */
    revError_Incomplete                 = -5000,   ///<Requested async work not yet complete.
    revError_Abandoned                  = -5001,   ///<Requested async work was abandoned and result is incomplete.

    /* Rendering errors */
    revError_DisplayLost                = -6000,   ///<In the event of a system-wide graphics reset or cable unplug this is returned to the app.
    revError_TextureSwapChainFull       = -6001,   ///<rev_CommitTextureSwapChain was called too many times on a texture swapchain without calling submit to use the chain.
    revError_TextureSwapChainInvalid    = -6002,   ///<The revTextureSwapChain is in an incomplete or inconsistent state. Ensure rev_CommitTextureSwapChain was called at least once first.

    /* Fatal errors */
    revError_RuntimeException           = -7000,   ///< A runtime exception occurred. The application is required to shutdown LibOVR and re-initialize it before this error state will be cleared.


    revError_MetricsUnknownApp            = -90000,
    revError_MetricsDuplicateApp          = -90001,
    revError_MetricsNoEvents              = -90002,
    revError_MetricsRuntime               = -90003,
    revError_MetricsFile                  = -90004,
    revError_MetricsNoClientInfo          = -90005,
    revError_MetricsNoAppMetaData         = -90006,
    revError_MetricsNoApp                 = -90007,
    revError_MetricsOafFailure            = -90008,
    revError_MetricsSessionAlreadyActive  = -90009,
    revError_MetricsSessionNotActive      = -90010,

} revErrorType;



/// Provides information about the last error.
/// \see rev_GetLastErrorInfo
typedef struct revErrorInfo_
{
    revResult Result;               ///< The result from the last API call that generated an error revResult.
    char      ErrorString[512];     ///< A UTF8-encoded null-terminated English string describing the problem. The format of this string is subject to change in future versions.
} revErrorInfo;

#endif /* REV_ErrorCode_h */
