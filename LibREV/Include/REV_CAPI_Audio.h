/********************************************************************************//**
\file      REV_CAPI_Audio.h
\brief     CAPI audio functions.
\copyright Copyright 2015 Oculus VR, LLC. All Rights reserved.
************************************************************************************/


#ifndef REV_CAPI_Audio_h
#define REV_CAPI_Audio_h

#ifdef _WIN32
#include <windows.h>
#include "REV_CAPI.h"
#define REV_AUDIO_MAX_DEVICE_STR_SIZE 128

/// Gets the ID of the preferred VR audio output device.
///
/// \param[out] deviceOutId The ID of the user's preferred VR audio device to use, which will be valid upon a successful return value, else it will be WAVE_MAPPER.
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use
///         rev_GetLastErrorInfo to get more information.
///
REV_PUBLIC_FUNCTION(revResult) rev_GetAudioDeviceOutWaveId(UINT* deviceOutId);

/// Gets the ID of the preferred VR audio input device.
///
/// \param[out] deviceInId The ID of the user's preferred VR audio device to use, which will be valid upon a successful return value, else it will be WAVE_MAPPER.
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use
///         rev_GetLastErrorInfo to get more information.
///
REV_PUBLIC_FUNCTION(revResult) rev_GetAudioDeviceInWaveId(UINT* deviceInId);


/// Gets the GUID of the preferred VR audio device as a string.
///
/// \param[out] deviceOutStrBuffer A buffer where the GUID string for the device will copied to.
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use
///         rev_GetLastErrorInfo to get more information.
///
REV_PUBLIC_FUNCTION(revResult) rev_GetAudioDeviceOutGuidStr(WCHAR deviceOutStrBuffer[REV_AUDIO_MAX_DEVICE_STR_SIZE]);


/// Gets the GUID of the preferred VR audio device.
///
/// \param[out] deviceOutGuid The GUID of the user's preferred VR audio device to use, which will be valid upon a successful return value, else it will be NULL.
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use
///         rev_GetLastErrorInfo to get more information.
///
REV_PUBLIC_FUNCTION(revResult) rev_GetAudioDeviceOutGuid(GUID* deviceOutGuid);


/// Gets the GUID of the preferred VR microphone device as a string.
///
/// \param[out] deviceInStrBuffer A buffer where the GUID string for the device will copied to.
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use
///         rev_GetLastErrorInfo to get more information.
///
REV_PUBLIC_FUNCTION(revResult) rev_GetAudioDeviceInGuidStr(WCHAR deviceInStrBuffer[REV_AUDIO_MAX_DEVICE_STR_SIZE]);


/// Gets the GUID of the preferred VR microphone device.
///
/// \param[out] deviceInGuid The GUID of the user's preferred VR audio device to use, which will be valid upon a successful return value, else it will be NULL.
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use
///         rev_GetLastErrorInfo to get more information.
///
REV_PUBLIC_FUNCTION(revResult) rev_GetAudioDeviceInGuid(GUID* deviceInGuid);

#endif //REV_OS_MS

#endif    // REV_CAPI_Audio_h
