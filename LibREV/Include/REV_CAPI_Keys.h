/********************************************************************************//**
\file      REV_CAPI.h
\brief     Keys for CAPI proprty function calls
\copyright Copyright 2015 Oculus VR, LLC All Rights reserved.
************************************************************************************/

#ifndef REV_CAPI_Keys__h
#define REV_CAPI_Keys__h

#include "REV_Version.h"



#define REV_KEY_USER_                        "User"                // string

#define REV_KEY_NAME_                        "Name"                // string

#define REV_KEY_GENDER_                      "Gender"              // string "Male", "Female", or "Unknown"
#define REV_DEFAULT_GENDER_                  "Unknown"

#define REV_KEY_PLAYER_HEIGHT_               "PlayerHeight"        // float meters
#define REV_DEFAULT_PLAYER_HEIGHT_           1.778f

#define REV_KEY_EYE_HEIGHT_                  "EyeHeight"           // float meters
#define REV_DEFAULT_EYE_HEIGHT_              1.675f

#define REV_KEY_NECK_TO_EYE_DISTANCE_        "NeckEyeDistance"     // float[2] meters
#define REV_DEFAULT_NECK_TO_EYE_HORIZONTAL_  0.0805f
#define REV_DEFAULT_NECK_TO_EYE_VERTICAL_    0.075f


#define REV_KEY_EYE_TO_NOSE_DISTANCE_        "EyeToNoseDist"       // float[2] meters





#define REV_PERF_HUD_MODE_                       "PerfHudMode"                       // int, allowed values are defined in enum revPerfHudMode

#define REV_LAYER_HUD_MODE_                      "LayerHudMode"                      // int, allowed values are defined in enum revLayerHudMode
#define REV_LAYER_HUD_CURRENT_LAYER_             "LayerHudCurrentLayer"              // int, The layer to show 
#define REV_LAYER_HUD_SHOW_ALL_LAYERS_           "LayerHudShowAll"                   // bool, Hide other layers when the hud is enabled

#define REV_DEBUG_HUD_STEREO_MODE_               "DebugHudStereoMode"                // int, allowed values are defined in enum revDebugHudStereoMode
#define REV_DEBUG_HUD_STEREO_GUIDE_INFO_ENABLE_  "DebugHudStereoGuideInfoEnable"     // bool
#define REV_DEBUG_HUD_STEREO_GUIDE_SIZE_         "DebugHudStereoGuideSize2f"         // float[2]
#define REV_DEBUG_HUD_STEREO_GUIDE_POSITION_     "DebugHudStereoGuidePosition3f"     // float[3]
#define REV_DEBUG_HUD_STEREO_GUIDE_YAWPITCHROLL_ "DebugHudStereoGuideYawPitchRoll3f" // float[3]
#define REV_DEBUG_HUD_STEREO_GUIDE_COLOR_        "DebugHudStereoGuideColor4f"        // float[4]



#endif // REV_CAPI_Keys_h
