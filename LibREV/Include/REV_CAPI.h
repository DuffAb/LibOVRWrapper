/********************************************************************************//**
\file      REV_CAPI.h
\brief     C Interface to the Oculus PC SDK tracking and rendering library.
\copyright Copyright 2014-2016 Oculus VR, LLC All Rights reserved.
************************************************************************************/

#ifndef REV_CAPI__h  //   We don't use version numbers within this name, as all versioned variations of this file are currently mutually exclusive.
#define REV_CAPI__h  ///< Header include guard


#include "REV_CAPI_Keys.h"
#include "REV_Version.h"
#include "REV_ErrorCode.h"


#include <stdint.h>

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable: 4324) // structure was padded due to __declspec(align())
    #pragma warning(disable: 4359) // The alignment specified for a type is less than the alignment of the type of one of its data members
#endif



//-----------------------------------------------------------------------------------
// ***** REV_OS
//
#if !defined(REV_OS_WIN32) && defined(_WIN32)
    #define REV_OS_WIN32
#endif

#if !defined(REV_OS_MAC) && defined(__APPLE__)
    #define REV_OS_MAC
#endif

#if !defined(REV_OS_LINUX) && defined(__linux__)
    #define REV_OS_LINUX
#endif



//-----------------------------------------------------------------------------------
// ***** REV_CPP
//
#if !defined(REV_CPP)
    #if defined(__cplusplus)
        #define REV_CPP(x) x
    #else
        #define REV_CPP(x) /* Not C++ */
    #endif
#endif



//-----------------------------------------------------------------------------------
// ***** REV_CDECL
//
/// LibOVR calling convention for 32-bit Windows builds.
//
#if !defined(REV_CDECL)
    #if defined(_WIN32)
        #define REV_CDECL __cdecl
    #else
        #define REV_CDECL
    #endif
#endif



//-----------------------------------------------------------------------------------
// ***** REV_EXTERN_C
//
/// Defined as extern "C" when built from C++ code.
//
#if !defined(REV_EXTERN_C)
    #ifdef __cplusplus
        #define REV_EXTERN_C extern "C"
    #else
        #define REV_EXTERN_C
    #endif
#endif



//-----------------------------------------------------------------------------------
// ***** REV_PUBLIC_FUNCTION / REV_PRIVATE_FUNCTION
//
// REV_PUBLIC_FUNCTION  - Functions that externally visible from a shared library. Corresponds to Microsoft __dllexport.
// REV_PUBLIC_CLASS     - C++ structs and classes that are externally visible from a shared library. Corresponds to Microsoft __dllexport.
// REV_PRIVATE_FUNCTION - Functions that are not visible outside of a shared library. They are private to the shared library.
// REV_PRIVATE_CLASS    - C++ structs and classes that are not visible outside of a shared library. They are private to the shared library.
//
// REV_DLL_BUILD        - Used to indicate that the current compilation unit is of a shared library.
// REV_DLL_IMPORT       - Used to indicate that the current compilation unit is a user of the corresponding shared library.
// REV_STATIC_BUILD     - used to indicate that the current compilation unit is not a shared library but rather statically linked code.
//
#if !defined(REV_PUBLIC_FUNCTION)
    #if defined(REV_DLL_BUILD)
        #if defined(_WIN32)
            #define REV_PUBLIC_FUNCTION(rval) REV_EXTERN_C __declspec(dllexport) rval REV_CDECL
            #define REV_PUBLIC_CLASS          __declspec(dllexport)
            #define REV_PRIVATE_FUNCTION(rval) rval REV_CDECL
            #define REV_PRIVATE_CLASS
        #else
            #define REV_PUBLIC_FUNCTION(rval) REV_EXTERN_C __attribute__((visibility("default"))) rval REV_CDECL /* Requires GCC 4.0+ */
            #define REV_PUBLIC_CLASS          __attribute__((visibility("default"))) /* Requires GCC 4.0+ */
            #define REV_PRIVATE_FUNCTION(rval) __attribute__((visibility("hidden"))) rval REV_CDECL
            #define REV_PRIVATE_CLASS         __attribute__((visibility("hidden")))
        #endif
    #elif defined(REV_DLL_IMPORT)
        #if defined(_WIN32)
            #define REV_PUBLIC_FUNCTION(rval) REV_EXTERN_C __declspec(dllimport) rval REV_CDECL
            #define REV_PUBLIC_CLASS          __declspec(dllimport)
        #else
            #define REV_PUBLIC_FUNCTION(rval) REV_EXTERN_C rval REV_CDECL
            #define REV_PUBLIC_CLASS
        #endif
        #define REV_PRIVATE_FUNCTION(rval) rval REV_CDECL
        #define REV_PRIVATE_CLASS
    #else // REV_STATIC_BUILD
        #define REV_PUBLIC_FUNCTION(rval)     REV_EXTERN_C rval REV_CDECL
        #define REV_PUBLIC_CLASS
        #define REV_PRIVATE_FUNCTION(rval) rval REV_CDECL
        #define REV_PRIVATE_CLASS
    #endif
#endif


//-----------------------------------------------------------------------------------
// ***** REV_EXPORT
//
/// Provided for backward compatibility with older versions of this library.
//
#if !defined(REV_EXPORT)
    #ifdef REV_OS_WIN32
        #define REV_EXPORT __declspec(dllexport)
    #else
        #define REV_EXPORT
    #endif
#endif



//-----------------------------------------------------------------------------------
// ***** REV_ALIGNAS
//
#if !defined(REV_ALIGNAS)
    #if defined(__GNUC__) || defined(__clang__)
        #define REV_ALIGNAS(n) __attribute__((aligned(n)))
    #elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
        #define REV_ALIGNAS(n) __declspec(align(n))
    #elif defined(__CC_ARM)
        #define REV_ALIGNAS(n) __align(n)
    #else
        #error Need to define REV_ALIGNAS
    #endif
#endif


//-----------------------------------------------------------------------------------
// ***** REV_CC_HAS_FEATURE
//
// This is a portable way to use compile-time feature identification available
// with some compilers in a clean way. Direct usage of __has_feature in preprocessing
// statements of non-supporting compilers results in a preprocessing error.
//
// Example usage:
//     #if REV_CC_HAS_FEATURE(is_pod)
//         if(__is_pod(T)) // If the type is plain data then we can safely memcpy it.
//             memcpy(&destObject, &srcObject, sizeof(object));
//     #endif
//
#if !defined(REV_CC_HAS_FEATURE)
    #if defined(__clang__) // http://clang.llvm.org/docs/LanguageExtensions.html#id2
        #define REV_CC_HAS_FEATURE(x) __has_feature(x)
    #else
        #define REV_CC_HAS_FEATURE(x) 0
    #endif
#endif


// ------------------------------------------------------------------------
// ***** REV_STATIC_ASSERT
//
// Portable support for C++11 static_assert().
// Acts as if the following were declared:
//     void REV_STATIC_ASSERT(bool const_expression, const char* msg);
//
// Example usage:
//     REV_STATIC_ASSERT(sizeof(int32_t) == 4, "int32_t expected to be 4 bytes.");

#if !defined(REV_STATIC_ASSERT)
    #if !(defined(__cplusplus) && (__cplusplus >= 201103L)) /* Other */ && \
        !(defined(__GXX_EXPERIMENTAL_CXX0X__)) /* GCC */ && \
        !(defined(__clang__) && defined(__cplusplus) && REV_CC_HAS_FEATURE(cxx_static_assert)) /* clang */ && \
        !(defined(_MSC_VER) && (_MSC_VER >= 1600) && defined(__cplusplus)) /* VS2010+  */

        #if !defined(REV_SA_UNUSED)
        #if defined(REV_CC_GNU) || defined(REV_CC_CLANG)
            #define REV_SA_UNUSED __attribute__((unused))
        #else
            #define REV_SA_UNUSED
        #endif
        #define REV_SA_PASTE(a,b) a##b
        #define REV_SA_HELP(a,b)  REV_SA_PASTE(a,b)
        #endif

        #if defined(__COUNTER__)
            #define REV_STATIC_ASSERT(expression, msg) typedef char REV_SA_HELP(compileTimeAssert, __COUNTER__) [((expression) != 0) ? 1 : -1] REV_SA_UNUSED
        #else
            #define REV_STATIC_ASSERT(expression, msg) typedef char REV_SA_HELP(compileTimeAssert, __LINE__) [((expression) != 0) ? 1 : -1] REV_SA_UNUSED
        #endif

    #else
        #define REV_STATIC_ASSERT(expression, msg) static_assert(expression, msg)
    #endif
#endif


//-----------------------------------------------------------------------------------
// ***** Padding
//
/// Defines explicitly unused space for a struct.
/// When used correcly, usage of this macro should not change the size of the struct.
/// Compile-time and runtime behavior with and without this defined should be identical.
///
#if !defined(REV_UNUSED_STRUCT_PAD)
    #define REV_UNUSED_STRUCT_PAD(padName, size) char padName[size];
#endif


//-----------------------------------------------------------------------------------
// ***** Word Size
//
/// Specifies the size of a pointer on the given platform.
///
#if !defined(REV_PTR_SIZE)
    #if defined(__WORDSIZE)
        #define REV_PTR_SIZE ((__WORDSIZE) / 8)
    #elif defined(_WIN64) || defined(__LP64__) || defined(_LP64) || defined(_M_IA64) || defined(__ia64__) || defined(__arch64__) || defined(__64BIT__) || defined(__Ptr_Is_64)
        #define REV_PTR_SIZE 8
    #elif defined(__CC_ARM) && (__sizeof_ptr == 8)
        #define REV_PTR_SIZE 8
    #else
        #define REV_PTR_SIZE 4
    #endif
#endif


//-----------------------------------------------------------------------------------
// ***** REV_ON32 / REV_ON64
//
#if REV_PTR_SIZE == 8
    #define REV_ON32(x)
    #define REV_ON64(x) x
#else
    #define REV_ON32(x) x
    #define REV_ON64(x)
#endif

#ifndef REV_BASICS
#define REV_BASICS

//-----------------------------------------------------------------------------------
// ***** revBool

typedef char revBool;   ///< Boolean type
#define revFalse 0      ///< revBool value of false.
#define revTrue  1      ///< revBool value of true.


//-----------------------------------------------------------------------------------
// ***** Simple Math Structures

/// A 2D vector with integer components.
typedef struct REV_ALIGNAS(4) revVector2i_
{
    int x, y;
} revVector2i;

/// A 2D size with integer components.
typedef struct REV_ALIGNAS(4) revSizei_
{
    int w, h;
} revSizei;

/// A 2D rectangle with a position and size.
/// All components are integers.
typedef struct REV_ALIGNAS(4) revRecti_
{
    revVector2i Pos;
    revSizei    Size;
} revRecti;

/// A quaternion rotation.
typedef struct REV_ALIGNAS(4) revQuatf_
{
    float x, y, z, w;
} revQuatf;

/// A 2D vector with float components.
typedef struct REV_ALIGNAS(4) revVector2f_
{
    float x, y;
} revVector2f;

/// A 3D vector with float components.
typedef struct REV_ALIGNAS(4) revVector3f_
{
    float x, y, z;
} revVector3f;

/// A 4x4 matrix with float elements.
typedef struct REV_ALIGNAS(4) revMatrix4f_
{
    float M[4][4];
} revMatrix4f;

#endif

/// Position and orientation together.
typedef struct REV_ALIGNAS(4) revPosef_
{
    revQuatf     Orientation;
    revVector3f  Position;
} revPosef;

/// A full pose (rigid body) configuration with first and second derivatives.
///
/// Body refers to any object for which revPoseStatef is providing data.
/// It can be the HMD, Touch controller, sensor or something else. The context 
/// depends on the usage of the struct.
typedef struct REV_ALIGNAS(8) revPoseStatef_
{
    revPosef     ThePose;               ///< Position and orientation.
    revVector3f  AngularVelocity;       ///< Angular velocity in radians per second.
    revVector3f  LinearVelocity;        ///< Velocity in meters per second.
    revVector3f  AngularAcceleration;   ///< Angular acceleration in radians per second per second.
    revVector3f  LinearAcceleration;    ///< Acceleration in meters per second per second.
    REV_UNUSED_STRUCT_PAD(pad0, 4)      ///< \internal struct pad.
    double       TimeInSeconds;         ///< Absolute time that this pose refers to. \see rev_GetTimeInSeconds
} revPoseStatef;

/// Describes the up, down, left, and right angles of the field of view.
///
/// Field Of View (FOV) tangent of the angle units.
/// \note For a standard 90 degree vertical FOV, we would
/// have: { UpTan = tan(90 degrees / 2), DownTan = tan(90 degrees / 2) }.
typedef struct REV_ALIGNAS(4) revFovPort_
{
    float UpTan;    ///< The tangent of the angle between the viewing vector and the top edge of the field of view.
    float DownTan;  ///< The tangent of the angle between the viewing vector and the bottom edge of the field of view.
    float LeftTan;  ///< The tangent of the angle between the viewing vector and the left edge of the field of view.
    float RightTan; ///< The tangent of the angle between the viewing vector and the right edge of the field of view.
} revFovPort;


//-----------------------------------------------------------------------------------
// ***** HMD Types

/// Enumerates all HMD types that we support.
///
/// The currently released developer kits are revHmd_DK1 and revHmd_DK2. The other enumerations are for internal use only.
typedef enum revHmdType_
{
	revHmd_None      = 0,
    revHmd_DK1       = 3,
    revHmd_DKHD      = 4,
    revHmd_DK2       = 6,
    revHmd_CB        = 8,
    revHmd_Other     = 9,
    revHmd_E3_2015   = 10,
    revHmd_ES06      = 11,
    revHmd_ES09      = 12,
    revHmd_ES11      = 13,
    revHmd_CV1       = 14,

    revHmd_EnumSize  = 0x7fffffff ///< \internal Force type int32_t.
} revHmdType;


/// HMD capability bits reported by device.
///
typedef enum revHmdCaps_
{
    // Read-only flags
    revHmdCap_DebugDevice             = 0x0010,   ///< <B>(read only)</B> Specifies that the HMD is a virtual debug device.


    revHmdCap_EnumSize            = 0x7fffffff ///< \internal Force type int32_t.
} revHmdCaps;


/// Tracking capability bits reported by the device.
/// Used with rev_GetTrackingCaps.
typedef enum revTrackingCaps_
{
    revTrackingCap_Orientation      = 0x0010,    ///< Supports orientation tracking (IMU).
    revTrackingCap_MagYawCorrection = 0x0020,    ///< Supports yaw drift correction via a magnetometer or other means.
    revTrackingCap_Position         = 0x0040,    ///< Supports positional tracking.
    revTrackingCap_EnumSize         = 0x7fffffff ///< \internal Force type int32_t.
} revTrackingCaps;


/// Specifies which eye is being used for rendering.
/// This type explicitly does not include a third "NoStereo" monoscopic option, as such is
/// not required for an HMD-centered API.
typedef enum revEyeType_
{
    revEye_Left     = 0,         ///< The left eye, from the viewer's perspective.
    revEye_Right    = 1,         ///< The right eye, from the viewer's perspective.
    revEye_Count    = 2,         ///< \internal Count of enumerated elements.
    revEye_EnumSize = 0x7fffffff ///< \internal Force type int32_t.
} revEyeType;

/// Specifies the coordinate system revTrackingState returns tracking poses in.
/// Used with rev_SetTrackingOriginType()
typedef enum revTrackingOrigin_
{
    /// \brief Tracking system origin reported at eye (HMD) height
    /// \details Prefer using this origin when your application requires
    /// matching user's current physical head pose to a virtual head pose
    /// without any regards to a the height of the floor. Cockpit-based,
    /// or 3rd-person experiences are ideal candidates.
    /// When used, all poses in revTrackingState are reported as an offset
    /// transform from the profile calibrated or recentered HMD pose.
    /// It is recommended that apps using this origin type call rev_RecenterTrackingOrigin
    /// prior to starting the VR experience, but notify the user before doing so
    /// to make sure the user is in a comfortable pose, facing a comfortable
    /// direction.
    revTrackingOrigin_EyeLevel = 0,
    /// \brief Tracking system origin reported at floor height
    /// \details Prefer using this origin when your application requires the
    /// physical floor height to match the virtual floor height, such as
    /// standing experiences.
    /// When used, all poses in revTrackingState are reported as an offset
    /// transform from the profile calibrated floor pose. Calling rev_RecenterTrackingOrigin
    /// will recenter the X & Z axes as well as yaw, but the Y-axis (i.e. height) will continue
    /// to be reported using the floor height as the origin for all poses.
    revTrackingOrigin_FloorLevel = 1,
    revTrackingOrigin_Count = 2,            ///< \internal Count of enumerated elements.
    revTrackingOrigin_EnumSize = 0x7fffffff ///< \internal Force type int32_t.
} revTrackingOrigin;

/// Identifies a graphics device in a platform-specific way.
/// For Windows this is a LUID type.
typedef struct REV_ALIGNAS(REV_PTR_SIZE) revGraphicsLuid_
{
    // Public definition reserves space for graphics API-specific implementation
    char        Reserved[8];
} revGraphicsLuid;


/// This is a complete descriptor of the HMD.
typedef struct REV_ALIGNAS(REV_PTR_SIZE) revHmdDesc_
{
    revHmdType   Type;                         ///< The type of HMD.
    REV_ON64(REV_UNUSED_STRUCT_PAD(pad0, 4))   ///< \internal struct paddding.
    char         ProductName[64];              ///< UTF8-encoded product identification string (e.g. "Oculus Rift DK1").
    char         Manufacturer[64];             ///< UTF8-encoded HMD manufacturer identification string.
    short        VendorId;                     ///< HID (USB) vendor identifier of the device.
    short        ProductId;                    ///< HID (USB) product identifier of the device.
    char         SerialNumber[24];             ///< HMD serial number.
    short        FirmwareMajor;                ///< HMD firmware major version.
    short        FirmwareMinor;                ///< HMD firmware minor version.
    unsigned int AvailableHmdCaps;             ///< Capability bits described by revHmdCaps which the HMD currently supports.
    unsigned int DefaultHmdCaps;               ///< Capability bits described by revHmdCaps which are default for the current Hmd.
    unsigned int AvailableTrackingCaps;        ///< Capability bits described by revTrackingCaps which the system currently supports.
    unsigned int DefaultTrackingCaps;          ///< Capability bits described by revTrackingCaps which are default for the current system.
    revFovPort   DefaultEyeFov[revEye_Count];  ///< Defines the recommended FOVs for the HMD.
    revFovPort   MaxEyeFov[revEye_Count];      ///< Defines the maximum FOVs for the HMD.
    revSizei     Resolution;                   ///< Resolution of the full HMD screen (both eyes) in pixels.
    float        DisplayRefreshRate;           ///< Nominal refresh rate of the display in cycles per second at the time of HMD creation.
    REV_ON64(REV_UNUSED_STRUCT_PAD(pad1, 4))   ///< \internal struct paddding.
} revHmdDesc;


/// Used as an opaque pointer to an OVR session.
typedef struct revHmdStruct* revSession;



/// Bit flags describing the current status of sensor tracking.
///  The values must be the same as in enum StatusBits
///
/// \see revTrackingState
///
typedef enum revStatusBits_
{
    revStatus_OrientationTracked    = 0x0001,    ///< Orientation is currently tracked (connected and in use).
    revStatus_PositionTracked       = 0x0002,    ///< Position is currently tracked (false if out of range).
    revStatus_EnumSize              = 0x7fffffff ///< \internal Force type int32_t.
} revStatusBits;


///  Specifies the description of a single sensor.
///
/// \see revGetTrackerDesc
///
typedef struct REV_ALIGNAS(REV_PTR_SIZE) revTrackerDesc_
{
    float FrustumHFovInRadians;      ///< Sensor frustum horizontal field-of-view (if present).
    float FrustumVFovInRadians;      ///< Sensor frustum vertical field-of-view (if present).
    float FrustumNearZInMeters;      ///< Sensor frustum near Z (if present).
    float FrustumFarZInMeters;       ///< Sensor frustum far Z (if present).
} revTrackerDesc;


///  Specifies sensor flags.
///
///  /see revTrackerPose
///
typedef enum revTrackerFlags_
{
    revTracker_Connected   = 0x0020,      ///< The sensor is present, else the sensor is absent or offline.
    revTracker_PoseTracked = 0x0004       ///< The sensor has a valid pose, else the pose is unavailable. This will only be set if revTracker_Connected is set.
} revTrackerFlags;


///  Specifies the pose for a single sensor.
///
typedef struct REV_ALIGNAS(8) _revTrackerPose
{
    unsigned int TrackerFlags;      ///< revTrackerFlags.
    revPosef     Pose;              ///< The sensor's pose. This pose includes sensor tilt (roll and pitch). For a leveled coordinate system use LeveledPose.
    revPosef     LeveledPose;       ///< The sensor's leveled pose, aligned with gravity. This value includes position and yaw of the sensor, but not roll and pitch. It can be used as a reference point to render real-world objects in the correct location.
    REV_UNUSED_STRUCT_PAD(pad0, 4)  ///< \internal struct pad.
} revTrackerPose;


/// Tracking state at a given absolute time (describes predicted HMD pose, etc.).
/// Returned by rev_GetTrackingState.
///
/// \see rev_GetTrackingState
///
typedef struct REV_ALIGNAS(8) revTrackingState_
{
    /// Predicted head pose (and derivatives) at the requested absolute time.
    revPoseStatef  HeadPose;

    /// HeadPose tracking status described by revStatusBits.
    unsigned int   StatusFlags;

    /// The most recent calculated pose for each hand when hand controller tracking is present.
    /// HandPoses[revHand_Left] refers to the left hand and HandPoses[revHand_Right] to the right hand.
    /// These values can be combined with revInputState for complete hand controller information.
    revPoseStatef  HandPoses[2];

    /// HandPoses status flags described by revStatusBits.
    /// Only revStatus_OrientationTracked and revStatus_PositionTracked are reported.
    unsigned int   HandStatusFlags[2];

    /// The pose of the origin captured during calibration.
    /// Like all other poses here, this is expressed in the space set by rev_RecenterTrackingOrigin,
    /// and so will change every time that is called. This pose can be used to calculate
    /// where the calibrated origin lands in the new recentered space.
    /// If an application never calls rev_RecenterTrackingOrigin, expect this value to be the identity
    /// pose and as such will point respective origin based on revTrackingOrigin requested when
    /// calling rev_GetTrackingState.
    revPosef      CalibratedOrigin;

} revTrackingState;


/// Rendering information for each eye. Computed by rev_GetRenderDesc() based on the
/// specified FOV. Note that the rendering viewport is not included
/// here as it can be specified separately and modified per frame by
/// passing different Viewport values in the layer structure.
///
/// \see rev_GetRenderDesc
///
typedef struct REV_ALIGNAS(4) revEyeRenderDesc_
{
    revEyeType  Eye;                        ///< The eye index to which this instance corresponds.
    revFovPort  Fov;                        ///< The field of view.
    revRecti    DistortedViewport;          ///< Distortion viewport.
    revVector2f PixelsPerTanAngleAtCenter;  ///< How many display pixels will fit in tan(angle) = 1.
    revVector3f HmdToEyeOffset;             ///< Translation of each eye, in meters.
} revEyeRenderDesc;


/// Projection information for revLayerEyeFovDepth.
///
/// Use the utility function revTimewarpProjectionDesc_FromProjection to
/// generate this structure from the application's projection matrix.
///
/// \see revLayerEyeFovDepth, revTimewarpProjectionDesc_FromProjection
///
typedef struct REV_ALIGNAS(4) revTimewarpProjectionDesc_
{
    float Projection22;     ///< Projection matrix element [2][2].
    float Projection23;     ///< Projection matrix element [2][3].
    float Projection32;     ///< Projection matrix element [3][2].
} revTimewarpProjectionDesc;


/// Contains the data necessary to properly calculate position info for various layer types.
/// - HmdToEyeOffset is the same value pair provided in revEyeRenderDesc.
/// - HmdSpaceToWorldScaleInMeters is used to scale player motion into in-application units.
///   In other words, it is how big an in-application unit is in the player's physical meters.
///   For example, if the application uses inches as its units then HmdSpaceToWorldScaleInMeters would be 0.0254.
///   Note that if you are scaling the player in size, this must also scale. So if your application
///   units are inches, but you're shrinking the player to half their normal size, then
///   HmdSpaceToWorldScaleInMeters would be 0.0254*2.0.
///
/// \see revEyeRenderDesc, rev_SubmitFrame
///
typedef struct REV_ALIGNAS(4) revViewScaleDesc_
{
    revVector3f HmdToEyeOffset[revEye_Count];   ///< Translation of each eye.
    float       HmdSpaceToWorldScaleInMeters;   ///< Ratio of viewer units to meter units.
} revViewScaleDesc;


//-----------------------------------------------------------------------------------
// ***** Platform-independent Rendering Configuration

/// The type of texture resource.
///
/// \see revTextureSwapChainDesc
///
typedef enum revTextureType_
{
    revTexture_2D,              ///< 2D textures.
    revTexture_2D_External,     ///< External 2D texture. Not used on PC
    revTexture_Cube,            ///< Cube maps. Not currently supported on PC.
    revTexture_Count,
    revTexture_EnumSize = 0x7fffffff  ///< \internal Force type int32_t.
} revTextureType;

/// The bindings required for texture swap chain.
///
/// All texture swap chains are automatically bindable as shader
/// input resources since the Oculus runtime needs this to read them.
///
/// \see revTextureSwapChainDesc
///
typedef enum revTextureBindFlags_
{
    revTextureBind_None,
    revTextureBind_DX_RenderTarget = 0x0001,    ///< The application can write into the chain with pixel shader
    revTextureBind_DX_UnorderedAccess = 0x0002, ///< The application can write to the chain with compute shader
    revTextureBind_DX_DepthStencil = 0x0004,    ///< The chain buffers can be bound as depth and/or stencil buffers

    revTextureBind_EnumSize = 0x7fffffff  ///< \internal Force type int32_t.
} revTextureBindFlags;

/// The format of a texture.
///
/// \see revTextureSwapChainDesc
///
typedef enum revTextureFormat_
{
    REV_FORMAT_UNKNOWN,
    REV_FORMAT_B5G6R5_UNORM,    ///< Not currently supported on PC. Would require a DirectX 11.1 device.
    REV_FORMAT_B5G5R5A1_UNORM,  ///< Not currently supported on PC. Would require a DirectX 11.1 device.
    REV_FORMAT_B4G4R4A4_UNORM,  ///< Not currently supported on PC. Would require a DirectX 11.1 device.
    REV_FORMAT_R8G8B8A8_UNORM,
    REV_FORMAT_R8G8B8A8_UNORM_SRGB,
    REV_FORMAT_B8G8R8A8_UNORM,
    REV_FORMAT_B8G8R8A8_UNORM_SRGB, ///< Not supported for OpenGL applications
    REV_FORMAT_B8G8R8X8_UNORM,      ///< Not supported for OpenGL applications
    REV_FORMAT_B8G8R8X8_UNORM_SRGB, ///< Not supported for OpenGL applications
    REV_FORMAT_R16G16B16A16_FLOAT,
    REV_FORMAT_D16_UNORM,
    REV_FORMAT_D24_UNORM_S8_UINT,
    REV_FORMAT_D32_FLOAT,
    REV_FORMAT_D32_FLOAT_S8X24_UINT,

    REV_FORMAT_ENUMSIZE = 0x7fffffff  ///< \internal Force type int32_t.
} revTextureFormat;

/// Misc flags overriding particular
///   behaviors of a texture swap chain
///
/// \see revTextureSwapChainDesc
///
typedef enum revTextureMiscFlags_
{
    revTextureMisc_None, 

    /// DX only: The underlying texture is created with a TYPELESS equivalent of the
    /// format specified in the texture desc. The SDK will still access the
    /// texture using the format specified in the texture desc, but the app can
    /// create views with different formats if this is specified.
    revTextureMisc_DX_Typeless = 0x0001,

    /// DX only: Allow generation of the mip chain on the GPU via the GenerateMips
    /// call. This flag requires that RenderTarget binding also be specified.
    revTextureMisc_AllowGenerateMips = 0x0002,

    revTextureMisc_EnumSize = 0x7fffffff  ///< \internal Force type int32_t.
} revTextureFlags;

/// Description used to create a texture swap chain.
///
/// \see rev_CreateTextureSwapChainDX
/// \see rev_CreateTextureSwapChainGL
///
typedef struct
{
    revTextureType      Type;
    revTextureFormat    Format;
    int                 ArraySize;      ///< Only supported with revTexture_2D. Not supported on PC at this time.
    int                 Width;
    int                 Height;
    int                 MipLevels;
    int                 SampleCount;    ///< Current only supported on depth textures
    revBool             StaticImage;    ///< Not buffered in a chain. For images that don't change
    unsigned int        MiscFlags;      ///< revTextureMiscFlags
    unsigned int        BindFlags;      ///< revTextureBindFlags. Not used for GL.
} revTextureSwapChainDesc;

/// Description used to create a mirror texture.
///
/// \see rev_CreateMirrorTextureDX
/// \see rev_CreateMirrorTextureGL
///
typedef struct
{
    revTextureFormat    Format;
    int                 Width;
    int                 Height;
    unsigned int        MiscFlags;      ///< revTextureMiscFlags
} revMirrorTextureDesc;

typedef struct revTextureSwapChainData* revTextureSwapChain;
typedef struct revMirrorTextureData* revMirrorTexture;

//-----------------------------------------------------------------------------------

/// Describes button input types.
/// Button inputs are combined; that is they will be reported as pressed if they are 
/// pressed on either one of the two devices.
/// The revButton_Up/Down/Left/Right map to both XBox D-Pad and directional buttons.
/// The revButton_Enter and revButton_Return map to Start and Back controller buttons, respectively.
typedef enum revButton_
{    
    revButton_A         = 0x00000001,
    revButton_B         = 0x00000002,
    revButton_RThumb    = 0x00000004,
    revButton_RShoulder = 0x00000008,

    // Bit mask of all buttons on the right Touch controller
    revButton_RMask     = revButton_A | revButton_B | revButton_RThumb | revButton_RShoulder,

    revButton_X         = 0x00000100,
    revButton_Y         = 0x00000200,
    revButton_LThumb    = 0x00000400,
    revButton_LShoulder = 0x00000800,

    // Bit mask of all buttons on the left Touch controller
    revButton_LMask     = revButton_X | revButton_Y | revButton_LThumb | revButton_LShoulder,

    // Navigation through DPad.
    revButton_Up        = 0x00010000,
    revButton_Down      = 0x00020000,
    revButton_Left      = 0x00040000,
    revButton_Right     = 0x00080000,
    revButton_Enter     = 0x00100000, // Start on XBox controller.
    revButton_Back      = 0x00200000, // Back on Xbox controller.
    revButton_VolUp     = 0x00400000,  // only supported by Remote.
    revButton_VolDown   = 0x00800000,  // only supported by Remote.
    revButton_Home      = 0x01000000,
    revButton_Private   = revButton_VolUp | revButton_VolDown | revButton_Home,


    revButton_EnumSize  = 0x7fffffff ///< \internal Force type int32_t.
} revButton;

/// Describes touch input types.
/// These values map to capacitive touch values reported revInputState::Touch.
/// Some of these values are mapped to button bits for consistency.
typedef enum revTouch_
{
    revTouch_A              = revButton_A,
    revTouch_B              = revButton_B,
    revTouch_RThumb         = revButton_RThumb,
    revTouch_RIndexTrigger  = 0x00000010,

    // Bit mask of all the button touches on the right controller
    revTouch_RButtonMask    = revTouch_A | revTouch_B | revTouch_RThumb | revTouch_RIndexTrigger,

    revTouch_X              = revButton_X,
    revTouch_Y              = revButton_Y,
    revTouch_LThumb         = revButton_LThumb,
    revTouch_LIndexTrigger  = 0x00001000,

    // Bit mask of all the button touches on the left controller
    revTouch_LButtonMask    = revTouch_X | revTouch_Y | revTouch_LThumb | revTouch_LIndexTrigger,

    // Finger pose state 
    // Derived internally based on distance, proximity to sensors and filtering.
    revTouch_RIndexPointing = 0x00000020,
    revTouch_RThumbUp       = 0x00000040,

    // Bit mask of all right controller poses
    revTouch_RPoseMask      = revTouch_RIndexPointing | revTouch_RThumbUp,

    revTouch_LIndexPointing = 0x00002000,
    revTouch_LThumbUp       = 0x00004000,

    // Bit mask of all left controller poses
    revTouch_LPoseMask      = revTouch_LIndexPointing | revTouch_LThumbUp,

    revTouch_EnumSize       = 0x7fffffff ///< \internal Force type int32_t.
} revTouch;

/// Specifies which controller is connected; multiple can be connected at once.
typedef enum revControllerType_
{
    revControllerType_None      = 0x00,
    revControllerType_LTouch    = 0x01,
    revControllerType_RTouch    = 0x02,
    revControllerType_Touch     = 0x03,
    revControllerType_Remote    = 0x04,
    revControllerType_XBox      = 0x10,

    revControllerType_Active    = 0xff,      ///< Operate on or query whichever controller is active.

    revControllerType_EnumSize  = 0x7fffffff ///< \internal Force type int32_t.
} revControllerType;


/// Provides names for the left and right hand array indexes.
///
/// \see revInputState, revTrackingState
/// 
typedef enum revHandType_
{
    revHand_Left  = 0,
    revHand_Right = 1,
    revHand_Count = 2,
    revHand_EnumSize = 0x7fffffff ///< \internal Force type int32_t.
} revHandType;



/// revInputState describes the complete controller input state, including Oculus Touch,
/// and XBox gamepad. If multiple inputs are connected and used at the same time,
/// their inputs are combined.
typedef struct revInputState_
{
    // System type when the controller state was last updated.
    double              TimeInSeconds;

    // Values for buttons described by revButton.
    unsigned int        Buttons;

    // Touch values for buttons and sensors as described by revTouch.
    unsigned int        Touches;
    
    // Left and right finger trigger values (revHand_Left and revHand_Right), in the range 0.0 to 1.0f.
    float               IndexTrigger[revHand_Count];
    
    // Left and right hand trigger values (revHand_Left and revHand_Right), in the range 0.0 to 1.0f.
    float               HandTrigger[revHand_Count];

    // Horizontal and vertical thumbstick axis values (revHand_Left and revHand_Right), in the range -1.0f to 1.0f.
    revVector2f         Thumbstick[revHand_Count];

    // The type of the controller this state is for.
    revControllerType   ControllerType;
    
} revInputState;



//-----------------------------------------------------------------------------------
// ***** Initialize structures

/// Initialization flags.
///
/// \see revInitParams, rev_Initialize
///
typedef enum revInitFlags_
{
    /// When a debug library is requested, a slower debugging version of the library will
    /// run which can be used to help solve problems in the library and debug application code.
    revInit_Debug          = 0x00000001,

    /// When a version is requested, the LibOVR runtime respects the RequestedMinorVersion
    /// field and verifies that the RequestedMinorVersion is supported.
    revInit_RequestVersion = 0x00000004,

    // These bits are writable by user code.
    revInit_WritableBits   = 0x00ffffff,

    revInit_EnumSize       = 0x7fffffff ///< \internal Force type int32_t.
} revInitFlags;


/// Logging levels
///
/// \see revInitParams, revLogCallback
///
typedef enum revLogLevel_
{
    revLogLevel_Debug    = 0, ///< Debug-level log event.
    revLogLevel_Info     = 1, ///< Info-level log event.
    revLogLevel_Error    = 2, ///< Error-level log event.

    revLogLevel_EnumSize = 0x7fffffff ///< \internal Force type int32_t.
} revLogLevel;


/// Signature of the logging callback function pointer type.
///
/// \param[in] userData is an arbitrary value specified by the user of revInitParams.
/// \param[in] level is one of the revLogLevel constants.
/// \param[in] message is a UTF8-encoded null-terminated string.
/// \see revInitParams, revLogLevel, rev_Initialize
///
typedef void (REV_CDECL* revLogCallback)(uintptr_t userData, int level, const char* message);


/// Parameters for rev_Initialize.
///
/// \see rev_Initialize
///
typedef struct REV_ALIGNAS(8) revInitParams_
{
    /// Flags from revInitFlags to override default behavior.
    /// Use 0 for the defaults.
    uint32_t       Flags;

    /// Requests a specific minimum minor version of the LibOVR runtime.
    /// Flags must include revInit_RequestVersion or this will be ignored
    /// and REV_MINOR_VERSION will be used.
    uint32_t       RequestedMinorVersion;

    /// User-supplied log callback function, which may be called at any time
    /// asynchronously from multiple threads until rev_Shutdown completes.
    /// Use NULL to specify no log callback.
    revLogCallback LogCallback;

    /// User-supplied data which is passed as-is to LogCallback. Typically this 
    /// is used to store an application-specific pointer which is read in the 
    /// callback function.
    uintptr_t      UserData;

    /// Relative number of milliseconds to wait for a connection to the server
    /// before failing. Use 0 for the default timeout.
    uint32_t       ConnectionTimeoutMS;

    REV_ON64(REV_UNUSED_STRUCT_PAD(pad0, 4)) ///< \internal

} revInitParams;


#ifdef __cplusplus
extern "C" {
#endif


// -----------------------------------------------------------------------------------
// ***** API Interfaces

// Overview of the API
//
// Setup:
//  - rev_Initialize().
//  - rev_Create(&hmd, &graphicsId).
//  - Use hmd members and rev_GetFovTextureSize() to determine graphics configuration
//    and rev_GetRenderDesc() to get per-eye rendering parameters.
//  - Allocate texture swap chains with rev_CreateTextureSwapChainDX() or
//    rev_CreateTextureSwapChainGL(). Create any associated render target views or
//    frame buffer objects.
//
// Application Loop:
//  - Call rev_GetPredictedDisplayTime() to get the current frame timing information.
//  - Call rev_GetTrackingState() and rev_CalcEyePoses() to obtain the predicted
//    rendering pose for each eye based on timing.
//  - Render the scene content into the current buffer of the texture swapchains
//    for each eye and layer you plan to update this frame. If you render into a
//    texture swap chain, you must call rev_CommitTextureSwapChain() on it to commit
//    the changes before you reference the chain this frame (otherwise, your latest
//    changes won't be picked up).
//  - Call rev_SubmitFrame() to render the distorted layers to and present them on the HMD.
//    If rev_SubmitFrame returns revSuccess_NotVisible, there is no need to render the scene
//    for the next loop iteration. Instead, just call rev_SubmitFrame again until it returns
//    revSuccess. 
//
// Shutdown:
//  - rev_Destroy().
//  - rev_Shutdown().


/// Initializes LibOVR
///
/// Initialize LibOVR for application usage. This includes finding and loading the LibOVRRT
/// shared library. No LibOVR API functions, other than rev_GetLastErrorInfo, can be called
/// unless rev_Initialize succeeds. A successful call to rev_Initialize must be eventually
/// followed by a call to rev_Shutdown. rev_Initialize calls are idempotent.
/// Calling rev_Initialize twice does not require two matching calls to rev_Shutdown.
/// If already initialized, the return value is rev_Success.
/// 
/// LibOVRRT shared library search order:
///      -# Current working directory (often the same as the application directory).
///      -# Module directory (usually the same as the application directory,
///         but not if the module is a separate shared library).
///      -# Application directory
///      -# Development directory (only if REV_ENABLE_DEVELOPER_SEARCH is enabled,
///         which is off by default).
///      -# Standard OS shared library search location(s) (OS-specific).
///
/// \param params Specifies custom initialization options. May be NULL to indicate default options.
/// \return Returns an revResult indicating success or failure. In the case of failure, use
///         rev_GetLastErrorInfo to get more information. Example failed results include:
///     - revError_Initialize: Generic initialization error.
///     - revError_LibLoad: Couldn't load LibOVRRT.
///     - revError_LibVersion: LibOVRRT version incompatibility.
///     - revError_ServiceConnection: Couldn't connect to the OVR Service.
///     - revError_ServiceVersion: OVR Service version incompatibility.
///     - revError_IncompatibleOS: The operating system version is incompatible.
///     - revError_DisplayInit: Unable to initialize the HMD display.
///     - revError_ServerStart:  Unable to start the server. Is it already running?
///     - revError_Reinitialization: Attempted to re-initialize with a different version.
///
/// <b>Example code</b>
///     \code{.cpp}
///         revResult result = rev_Initialize(NULL);
///         if(REV_FAILURE(result)) {
///             revErrorInfo errorInfo;
///             rev_GetLastErrorInfo(&errorInfo);
///             DebugLog("rev_Initialize failed: %s", errorInfo.ErrorString);
///             return false;
///         }
///         [...]
///     \endcode
///
/// \see rev_Shutdown
///
REV_PUBLIC_FUNCTION(revResult) rev_Initialize(const revInitParams* params);


/// Shuts down LibOVR
///
/// A successful call to rev_Initialize must be eventually matched by a call to rev_Shutdown.
/// After calling rev_Shutdown, no LibOVR functions can be called except rev_GetLastErrorInfo
/// or another rev_Initialize. rev_Shutdown invalidates all pointers, references, and created objects
/// previously returned by LibOVR functions. The LibOVRRT shared library can be unloaded by
/// rev_Shutdown.
///
/// \see rev_Initialize
///
REV_PUBLIC_FUNCTION(void) rev_Shutdown();

/// Returns information about the most recent failed return value by the
/// current thread for this library.
///
/// This function itself can never generate an error.
/// The last error is never cleared by LibOVR, but will be overwritten by new errors.
/// Do not use this call to determine if there was an error in the last API
/// call as successful API calls don't clear the last revErrorInfo.
/// To avoid any inconsistency, rev_GetLastErrorInfo should be called immediately
/// after an API function that returned a failed revResult, with no other API
/// functions called in the interim.
///
/// \param[out] errorInfo The last revErrorInfo for the current thread.
///
/// \see revErrorInfo
///
REV_PUBLIC_FUNCTION(void) rev_GetLastErrorInfo(revErrorInfo* errorInfo);


/// Returns the version string representing the LibOVRRT version.
///
/// The returned string pointer is valid until the next call to rev_Shutdown.
///
/// Note that the returned version string doesn't necessarily match the current
/// REV_MAJOR_VERSION, etc., as the returned string refers to the LibOVRRT shared
/// library version and not the locally compiled interface version.
///
/// The format of this string is subject to change in future versions and its contents
/// should not be interpreted.
///
/// \return Returns a UTF8-encoded null-terminated version string.
///
REV_PUBLIC_FUNCTION(const char*) rev_GetVersionString();


/// Writes a message string to the LibOVR tracing mechanism (if enabled).
///
/// This message will be passed back to the application via the revLogCallback if
/// it was registered.
///
/// \param[in] level One of the revLogLevel constants.
/// \param[in] message A UTF8-encoded null-terminated string.
/// \return returns the strlen of the message or a negative value if the message is too large.
///
/// \see revLogLevel, revLogCallback
///
REV_PUBLIC_FUNCTION(int) rev_TraceMessage(int level, const char* message);


//-------------------------------------------------------------------------------------
/// @name HMD Management
///
/// Handles the enumeration, creation, destruction, and properties of an HMD (head-mounted display).
///@{


/// Returns information about the current HMD.
///
/// rev_Initialize must have first been called in order for this to succeed, otherwise revHmdDesc::Type
/// will be reported as revHmd_None.
/// 
/// \param[in] session Specifies an revSession previously returned by rev_Create, else NULL in which
///                case this function detects whether an HMD is present and returns its info if so.
///
/// \return Returns an revHmdDesc. If the hmd is NULL and revHmdDesc::Type is revHmd_None then 
///         no HMD is present.
///
REV_PUBLIC_FUNCTION(revHmdDesc) rev_GetHmdDesc(revSession session);


/// Returns the number of sensors. 
///
/// The number of sensors may change at any time, so this function should be called before use 
/// as opposed to once on startup.
/// 
/// \param[in] session Specifies an revSession previously returned by rev_Create.
///
/// \return Returns unsigned int count.
///
REV_PUBLIC_FUNCTION(unsigned int) rev_GetTrackerCount(revSession session);


/// Returns a given sensor description.
///
/// It's possible that sensor desc [0] may indicate a unconnnected or non-pose tracked sensor, but 
/// sensor desc [1] may be connected.
///
/// rev_Initialize must have first been called in order for this to succeed, otherwise the returned
/// trackerDescArray will be zero-initialized. The data returned by this function can change at runtime.
/// 
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// 
/// \param[in] trackerDescIndex Specifies a sensor index. The valid indexes are in the range of 0 to 
///            the sensor count returned by rev_GetTrackerCount.
///
/// \return Returns revTrackerDesc. An empty revTrackerDesc will be returned if trackerDescIndex is out of range.
///
/// \see revTrackerDesc, rev_GetTrackerCount
///
REV_PUBLIC_FUNCTION(revTrackerDesc) rev_GetTrackerDesc(revSession session, unsigned int trackerDescIndex);


/// Creates a handle to a VR session.
///
/// Upon success the returned revSession must be eventually freed with rev_Destroy when it is no longer needed.
/// A second call to rev_Create will result in an error return value if the previous Hmd has not been destroyed.
///
/// \param[out] pSession Provides a pointer to an revSession which will be written to upon success.
/// \param[out] luid Provides a system specific graphics adapter identifier that locates which
/// graphics adapter has the HMD attached. This must match the adapter used by the application
/// or no rendering output will be possible. This is important for stability on multi-adapter systems. An
/// application that simply chooses the default adapter will not run reliably on multi-adapter systems.
/// \return Returns an revResult indicating success or failure. Upon failure
///         the returned pHmd will be NULL.
///
/// <b>Example code</b>
///     \code{.cpp}
///         revSession session;
///         revGraphicsLuid luid;
///         revResult result = rev_Create(&session, &luid);
///         if(REV_FAILURE(result))
///            ...
///     \endcode
///
/// \see rev_Destroy
///
REV_PUBLIC_FUNCTION(revResult) rev_Create(revSession* pSession, revGraphicsLuid* pLuid);


/// Destroys the HMD.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \see rev_Create
///
REV_PUBLIC_FUNCTION(void) rev_Destroy(revSession session);


/// Specifies status information for the current session.
///
/// \see rev_GetSessionStatus
///
typedef struct revSessionStatus_
{
    revBool IsVisible;    ///< True if the process has VR focus and thus is visible in the HMD.
    revBool HmdPresent;   ///< True if an HMD is present.
    revBool HmdMounted;   ///< True if the HMD is on the user's head.
    revBool DisplayLost;  ///< True if the session is in a display-lost state. See rev_SubmitFrame.
    revBool ShouldQuit;   ///< True if the application should initiate shutdown.    
    revBool ShouldRecenter;  ///< True if UX has requested re-centering. Must call rev_ClearShouldRecenterFlag or rev_RecenterTrackingOrigin.
}revSessionStatus;


/// Returns status information for the application.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[out] sessionStatus Provides an revSessionStatus that is filled in.
///
/// \return Returns an revResult indicating success or failure. In the case of
///         failure, use rev_GetLastErrorInfo to get more information.
//          Return values include but aren't limited to:
///     - revSuccess: Completed successfully.
///     - revError_ServiceConnection: The service connection was lost and the application
//        must destroy the session.
///
REV_PUBLIC_FUNCTION(revResult) rev_GetSessionStatus(revSession session, revSessionStatus* sessionStatus);


//@}



//-------------------------------------------------------------------------------------
/// @name Tracking
///
/// Tracking functions handle the position, orientation, and movement of the HMD in space.
///
/// All tracking interface functions are thread-safe, allowing tracking state to be sampled
/// from different threads.
///
///@{



/// Sets the tracking origin type
///
/// When the tracking origin is changed, all of the calls that either provide
/// or accept revPosef will use the new tracking origin provided.
/// 
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] origin Specifies an revTrackingOrigin to be used for all revPosef
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use
///         rev_GetLastErrorInfo to get more information.
///
/// \see revTrackingOrigin, rev_GetTrackingOriginType
REV_PUBLIC_FUNCTION(revResult) rev_SetTrackingOriginType(revSession session, revTrackingOrigin origin);


/// Gets the tracking origin state
/// 
/// \param[in] session Specifies an revSession previously returned by rev_Create.
///
/// \return Returns the revTrackingOrigin that was either set by default, or previous set by the application.
///
/// \see revTrackingOrigin, rev_SetTrackingOriginType
REV_PUBLIC_FUNCTION(revTrackingOrigin) rev_GetTrackingOriginType(revSession session);


/// Re-centers the sensor position and orientation.
///
/// This resets the (x,y,z) positional components and the yaw orientation component.
/// The Roll and pitch orientation components are always determined by gravity and cannot
/// be redefined. All future tracking will report values relative to this new reference position.
/// If you are using revTrackerPoses then you will need to call rev_GetTrackerPose after 
/// this, because the sensor position(s) will change as a result of this.
/// 
/// The headset cannot be facing vertically upward or downward but rather must be roughly
/// level otherwise this function will fail with revError_InvalidHeadsetOrientation.
///
/// For more info, see the notes on each revTrackingOrigin enumeration to understand how
/// recenter will vary slightly in its behavior based on the current revTrackingOrigin setting.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
///
/// \return Returns an revResult indicating success or failure. In the case of failure, use
///         rev_GetLastErrorInfo to get more information. Return values include but aren't limited to:
///     - revSuccess: Completed successfully.
///     - revError_InvalidHeadsetOrientation: The headset was facing an invalid direction when
///       attempting recentering, such as facing vertically.
///
/// \see revTrackingOrigin, rev_GetTrackerPose
///
REV_PUBLIC_FUNCTION(revResult) rev_RecenterTrackingOrigin(revSession session);


/// Clears the ShouldRecenter status bit in revSessionStatus.
///
/// Clears the ShouldRecenter status bit in revSessionStatus, allowing further recenter 
/// requests to be detected. Since this is automatically done by rev_RecenterTrackingOrigin,
/// this is only needs to be called when application is doing its own re-centering.
REV_PUBLIC_FUNCTION(void) rev_ClearShouldRecenterFlag(revSession session);


/// Returns tracking state reading based on the specified absolute system time.
///
/// Pass an absTime value of 0.0 to request the most recent sensor reading. In this case
/// both PredictedPose and SamplePose will have the same value.
///
/// This may also be used for more refined timing of front buffer rendering logic, and so on.
/// This may be called by multiple threads.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] absTime Specifies the absolute future time to predict the return
///            revTrackingState value. Use 0 to request the most recent tracking state.
/// \param[in] latencyMarker Specifies that this call is the point in time where
///            the "App-to-Mid-Photon" latency timer starts from. If a given revLayer
///            provides "SensorSampleTimestamp", that will override the value stored here.
/// \return Returns the revTrackingState that is predicted for the given absTime.
///
/// \see revTrackingState, rev_GetEyePoses, rev_GetTimeInSeconds
///
REV_PUBLIC_FUNCTION(revTrackingState) rev_GetTrackingState(revSession session, double absTime, revBool latencyMarker);



/// Returns the revTrackerPose for the given sensor.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] trackerPoseIndex Index of the sensor being requested.
///
/// \return Returns the requested revTrackerPose. An empty revTrackerPose will be returned if trackerPoseIndex is out of range.
///
/// \see rev_GetTrackerCount
///
REV_PUBLIC_FUNCTION(revTrackerPose) rev_GetTrackerPose(revSession session, unsigned int trackerPoseIndex);



/// Returns the most recent input state for controllers, without positional tracking info.
///
/// \param[out] inputState Input state that will be filled in.
/// \param[in] revControllerType Specifies which controller the input will be returned for.
/// \return Returns revSuccess if the new state was successfully obtained.
///
/// \see revControllerType
///
REV_PUBLIC_FUNCTION(revResult) rev_GetInputState(revSession session, revControllerType controllerType, revInputState* inputState);


/// Returns controller types connected to the system OR'ed together.
///
/// \return A bitmask of revControllerTypes connected to the system.
///
/// \see revControllerType
///
REV_PUBLIC_FUNCTION(unsigned int) rev_GetConnectedControllerTypes(revSession session);


/// Turns on vibration of the given controller.
///
/// To disable vibration, call rev_SetControllerVibration with an amplitude of 0.
/// Vibration automatically stops after a nominal amount of time, so if you want vibration 
/// to be continuous over multiple seconds then you need to call this function periodically.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] controllerType Specifies the controller to apply the vibration to.
/// \param[in] frequency Specifies a vibration frequency in the range of 0.0 to 1.0. 
///            Currently the only valid values are 0.0, 0.5, and 1.0 and other values will
///            be clamped to one of these.
/// \param[in] amplitude Specifies a vibration amplitude in the range of 0.0 to 1.0.
///
/// \return Returns revSuccess upon success.
///
/// \see revControllerType
/// 
REV_PUBLIC_FUNCTION(revResult) rev_SetControllerVibration(revSession session, revControllerType controllerType,
                                                            float frequency, float amplitude);

///@}


//-------------------------------------------------------------------------------------
// @name Layers
//
///@{


///  Specifies the maximum number of layers supported by rev_SubmitFrame.
///
///  /see rev_SubmitFrame
///
enum {
    revMaxLayerCount = 16
};

/// Describes layer types that can be passed to rev_SubmitFrame.
/// Each layer type has an associated struct, such as revLayerEyeFov.
///
/// \see revLayerHeader
///
typedef enum revLayerType_
{
    revLayerType_Disabled    = 0,         ///< Layer is disabled.
    revLayerType_EyeFov      = 1,         ///< Described by revLayerEyeFov.
    revLayerType_Quad        = 3,         ///< Described by revLayerQuad. Previously called revLayerType_QuadInWorld.
    /// enum 4 used to be revLayerType_QuadHeadLocked. Instead, use revLayerType_Quad with revLayerFlag_HeadLocked.
    revLayerType_EyeMatrix   = 5,         ///< Described by revLayerEyeMatrix.
    revLayerType_EnumSize    = 0x7fffffff ///< Force type int32_t.
} revLayerType;


/// Identifies flags used by revLayerHeader and which are passed to rev_SubmitFrame.
///
/// \see revLayerHeader
///
typedef enum revLayerFlags_
{
    /// revLayerFlag_HighQuality enables 4x anisotropic sampling during the composition of the layer.
    /// The benefits are mostly visible at the periphery for high-frequency & high-contrast visuals.
    /// For best results consider combining this flag with an revTextureSwapChain that has mipmaps and
    /// instead of using arbitrary sized textures, prefer texture sizes that are powers-of-two.
    /// Actual rendered viewport and doesn't necessarily have to fill the whole texture.
    revLayerFlag_HighQuality               = 0x01,

    /// revLayerFlag_TextureOriginAtBottomLeft: the opposite is TopLeft.
    /// Generally this is false for D3D, true for OpenGL.
    revLayerFlag_TextureOriginAtBottomLeft = 0x02,

    /// Mark this surface as "headlocked", which means it is specified
    /// relative to the HMD and moves with it, rather than being specified
    /// relative to sensor/torso space and remaining still while the head moves.
    /// What used to be revLayerType_QuadHeadLocked is now revLayerType_Quad plus this flag.
    /// However the flag can be applied to any layer type to achieve a similar effect.
    revLayerFlag_HeadLocked                = 0x04

} revLayerFlags;


/// Defines properties shared by all revLayer structs, such as revLayerEyeFov.
///
/// revLayerHeader is used as a base member in these larger structs.
/// This struct cannot be used by itself except for the case that Type is revLayerType_Disabled.
///
/// \see revLayerType, revLayerFlags
///
typedef struct REV_ALIGNAS(REV_PTR_SIZE) revLayerHeader_
{
    revLayerType    Type;   ///< Described by revLayerType.
    unsigned        Flags;  ///< Described by revLayerFlags.
} revLayerHeader;


/// Describes a layer that specifies a monoscopic or stereoscopic view.
/// This is the kind of layer that's typically used as layer 0 to rev_SubmitFrame,
/// as it is the kind of layer used to render a 3D stereoscopic view.
///
/// Three options exist with respect to mono/stereo texture usage:
///    - ColorTexture[0] and ColorTexture[1] contain the left and right stereo renderings, respectively.
///      Viewport[0] and Viewport[1] refer to ColorTexture[0] and ColorTexture[1], respectively.
///    - ColorTexture[0] contains both the left and right renderings, ColorTexture[1] is NULL,
///      and Viewport[0] and Viewport[1] refer to sub-rects with ColorTexture[0].
///    - ColorTexture[0] contains a single monoscopic rendering, and Viewport[0] and
///      Viewport[1] both refer to that rendering.
///
/// \see revTextureSwapChain, rev_SubmitFrame
///
typedef struct REV_ALIGNAS(REV_PTR_SIZE) revLayerEyeFov_
{
    /// Header.Type must be revLayerType_EyeFov.
    revLayerHeader      Header;

    /// revTextureSwapChains for the left and right eye respectively.
    /// The second one of which can be NULL for cases described above.
    revTextureSwapChain  ColorTexture[revEye_Count];

    /// Specifies the ColorTexture sub-rect UV coordinates.
    /// Both Viewport[0] and Viewport[1] must be valid.
    revRecti            Viewport[revEye_Count];

    /// The viewport field of view.
    revFovPort          Fov[revEye_Count];

    /// Specifies the position and orientation of each eye view, with the position specified in meters.
    /// RenderPose will typically be the value returned from rev_CalcEyePoses,
    /// but can be different in special cases if a different head pose is used for rendering.
    revPosef            RenderPose[revEye_Count];

    /// Specifies the timestamp when the source revPosef (used in calculating RenderPose)
    /// was sampled from the SDK. Typically retrieved by calling rev_GetTimeInSeconds
    /// around the instant the application calls rev_GetTrackingState
    /// The main purpose for this is to accurately track app tracking latency.
    double              SensorSampleTime;

} revLayerEyeFov;




/// Describes a layer that specifies a monoscopic or stereoscopic view.
/// This uses a direct 3x4 matrix to map from view space to the UV coordinates.
/// It is essentially the same thing as revLayerEyeFov but using a much
/// lower level. This is mainly to provide compatibility with specific apps.
/// Unless the application really requires this flexibility, it is usually better
/// to use revLayerEyeFov.
///
/// Three options exist with respect to mono/stereo texture usage:
///    - ColorTexture[0] and ColorTexture[1] contain the left and right stereo renderings, respectively.
///      Viewport[0] and Viewport[1] refer to ColorTexture[0] and ColorTexture[1], respectively.
///    - ColorTexture[0] contains both the left and right renderings, ColorTexture[1] is NULL,
///      and Viewport[0] and Viewport[1] refer to sub-rects with ColorTexture[0].
///    - ColorTexture[0] contains a single monoscopic rendering, and Viewport[0] and
///      Viewport[1] both refer to that rendering.
///
/// \see revTextureSwapChain, rev_SubmitFrame
///
typedef struct REV_ALIGNAS(REV_PTR_SIZE) revLayerEyeMatrix_
{
    /// Header.Type must be revLayerType_EyeMatrix.
    revLayerHeader      Header;

    /// revTextureSwapChains for the left and right eye respectively.
    /// The second one of which can be NULL for cases described above.
    revTextureSwapChain  ColorTexture[revEye_Count];

    /// Specifies the ColorTexture sub-rect UV coordinates.
    /// Both Viewport[0] and Viewport[1] must be valid.
    revRecti            Viewport[revEye_Count];

    /// Specifies the position and orientation of each eye view, with the position specified in meters.
    /// RenderPose will typically be the value returned from rev_CalcEyePoses,
    /// but can be different in special cases if a different head pose is used for rendering.
    revPosef            RenderPose[revEye_Count];

    /// Specifies the mapping from a view-space vector
    /// to a UV coordinate on the textures given above.
    /// P = (x,y,z,1)*Matrix
    /// TexU  = P.x/P.z
    /// TexV  = P.y/P.z
    revMatrix4f         Matrix[revEye_Count];

    /// Specifies the timestamp when the source revPosef (used in calculating RenderPose)
    /// was sampled from the SDK. Typically retrieved by calling rev_GetTimeInSeconds
    /// around the instant the application calls rev_GetTrackingState
    /// The main purpose for this is to accurately track app tracking latency.
    double              SensorSampleTime;

} revLayerEyeMatrix;





/// Describes a layer of Quad type, which is a single quad in world or viewer space.
/// It is used for revLayerType_Quad. This type of layer represents a single
/// object placed in the world and not a stereo view of the world itself.
///
/// A typical use of revLayerType_Quad is to draw a television screen in a room
/// that for some reason is more convenient to draw as a layer than as part of the main
/// view in layer 0. For example, it could implement a 3D popup GUI that is drawn at a
/// higher resolution than layer 0 to improve fidelity of the GUI.
///
/// Quad layers are visible from both sides; they are not back-face culled.
///
/// \see revTextureSwapChain, rev_SubmitFrame
///
typedef struct REV_ALIGNAS(REV_PTR_SIZE) revLayerQuad_
{
    /// Header.Type must be revLayerType_Quad.
    revLayerHeader      Header;

    /// Contains a single image, never with any stereo view.
    revTextureSwapChain  ColorTexture;

    /// Specifies the ColorTexture sub-rect UV coordinates.
    revRecti            Viewport;

    /// Specifies the orientation and position of the center point of a Quad layer type.
    /// The supplied direction is the vector perpendicular to the quad.
    /// The position is in real-world meters (not the application's virtual world,
    /// the physical world the user is in) and is relative to the "zero" position
    /// set by rev_RecenterTrackingOrigin unless the revLayerFlag_HeadLocked flag is used.
    revPosef            QuadPoseCenter;

    /// Width and height (respectively) of the quad in meters.
    revVector2f         QuadSize;

} revLayerQuad;




/// Union that combines revLayer types in a way that allows them
/// to be used in a polymorphic way.
typedef union revLayer_Union_
{
    revLayerHeader      Header;
    revLayerEyeFov      EyeFov;
    revLayerQuad        Quad;
} revLayer_Union;


//@}



/// @name SDK Distortion Rendering
///
/// All of rendering functions including the configure and frame functions
/// are not thread safe. It is OK to use ConfigureRendering on one thread and handle
/// frames on another thread, but explicit synchronization must be done since
/// functions that depend on configured state are not reentrant.
///
/// These functions support rendering of distortion by the SDK.
///
//@{

/// TextureSwapChain creation is rendering API-specific.
/// rev_CreateTextureSwapChainDX and rev_CreateTextureSwapChainGL can be found in the
/// rendering API-specific headers, such as REV_CAPI_D3D.h and REV_CAPI_GL.h

/// Gets the number of buffers in an revTextureSwapChain.
///
/// \param[in]  session Specifies an revSession previously returned by rev_Create.
/// \param[in]  chain Specifies the revTextureSwapChain for which the length should be retrieved.
/// \param[out] out_Length Returns the number of buffers in the specified chain.
///
/// \return Returns an revResult for which REV_SUCCESS(result) is false upon error. 
///
/// \see rev_CreateTextureSwapChainDX, rev_CreateTextureSwapChainGL
///
REV_PUBLIC_FUNCTION(revResult) rev_GetTextureSwapChainLength(revSession session, revTextureSwapChain chain, int* out_Length);

/// Gets the current index in an revTextureSwapChain.
///
/// \param[in]  session Specifies an revSession previously returned by rev_Create.
/// \param[in]  chain Specifies the revTextureSwapChain for which the index should be retrieved.
/// \param[out] out_Index Returns the current (free) index in specified chain.
///
/// \return Returns an revResult for which REV_SUCCESS(result) is false upon error. 
///
/// \see rev_CreateTextureSwapChainDX, rev_CreateTextureSwapChainGL
///
REV_PUBLIC_FUNCTION(revResult) rev_GetTextureSwapChainCurrentIndex(revSession session, revTextureSwapChain chain, int* out_Index);

/// Gets the description of the buffers in an revTextureSwapChain
///
/// \param[in]  session Specifies an revSession previously returned by rev_Create.
/// \param[in]  chain Specifies the revTextureSwapChain for which the description should be retrieved.
/// \param[out] out_Desc Returns the description of the specified chain.
///
/// \return Returns an revResult for which REV_SUCCESS(result) is false upon error. 
///
/// \see rev_CreateTextureSwapChainDX, rev_CreateTextureSwapChainGL
///
REV_PUBLIC_FUNCTION(revResult) rev_GetTextureSwapChainDesc(revSession session, revTextureSwapChain chain, revTextureSwapChainDesc* out_Desc);

/// Commits any pending changes to an revTextureSwapChain, and advances its current index
///
/// \param[in]  session Specifies an revSession previously returned by rev_Create.
/// \param[in]  chain Specifies the revTextureSwapChain to commit.
///
/// \note When Commit is called, the texture at the current index is considered ready for use by the
/// runtime, and further writes to it should be avoided. The swap chain's current index is advanced,
/// providing there's room in the chain. The next time the SDK dereferences this texture swap chain,
/// it will synchronize with the app's graphics context and pick up the submitted index, opening up
/// room in the swap chain for further commits.
///
/// \return Returns an revResult for which REV_SUCCESS(result) is false upon error. 
///         Failures include but aren't limited to:
///     - revError_TextureSwapChainFull: rev_CommitTextureSwapChain was called too many times on a texture swapchain without calling submit to use the chain.
///
/// \see rev_CreateTextureSwapChainDX, rev_CreateTextureSwapChainGL
///
REV_PUBLIC_FUNCTION(revResult) rev_CommitTextureSwapChain(revSession session, revTextureSwapChain chain);

/// Destroys an revTextureSwapChain and frees all the resources associated with it.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] chain Specifies the revTextureSwapChain to destroy. If it is NULL then this function has no effect.
///
/// \see rev_CreateTextureSwapChainDX, rev_CreateTextureSwapChainGL
///
REV_PUBLIC_FUNCTION(void) rev_DestroyTextureSwapChain(revSession session, revTextureSwapChain chain);


/// MirrorTexture creation is rendering API-specific.
/// rev_CreateMirrorTextureDX and rev_CreateMirrorTextureGL can be found in the
/// rendering API-specific headers, such as REV_CAPI_D3D.h and REV_CAPI_GL.h

/// Destroys a mirror texture previously created by one of the mirror texture creation functions.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] mirrorTexture Specifies the revTexture to destroy. If it is NULL then this function has no effect.
///
/// \see rev_CreateMirrorTextureDX, rev_CreateMirrorTextureGL
///
REV_PUBLIC_FUNCTION(void) rev_DestroyMirrorTexture(revSession session, revMirrorTexture mirrorTexture);


/// Calculates the recommended viewport size for rendering a given eye within the HMD
/// with a given FOV cone.
///
/// Higher FOV will generally require larger textures to maintain quality.
/// Apps packing multiple eye views together on the same texture should ensure there are
/// at least 8 pixels of padding between them to prevent texture filtering and chromatic
/// aberration causing images to leak between the two eye views.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] eye Specifies which eye (left or right) to calculate for.
/// \param[in] fov Specifies the revFovPort to use.
/// \param[in] pixelsPerDisplayPixel Specifies the ratio of the number of render target pixels
///            to display pixels at the center of distortion. 1.0 is the default value. Lower
///            values can improve performance, higher values give improved quality.
/// \return Returns the texture width and height size.
///
REV_PUBLIC_FUNCTION(revSizei) rev_GetFovTextureSize(revSession session, revEyeType eye, revFovPort fov,
                                                       float pixelsPerDisplayPixel);

/// Computes the distortion viewport, view adjust, and other rendering parameters for
/// the specified eye.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] eyeType Specifies which eye (left or right) for which to perform calculations.
/// \param[in] fov Specifies the revFovPort to use.
///
/// \return Returns the computed revEyeRenderDesc for the given eyeType and field of view.
///
/// \see revEyeRenderDesc
///
REV_PUBLIC_FUNCTION(revEyeRenderDesc) rev_GetRenderDesc(revSession session,
                                                           revEyeType eyeType, revFovPort fov);

/// Submits layers for distortion and display.
///
/// rev_SubmitFrame triggers distortion and processing which might happen asynchronously.
/// The function will return when there is room in the submission queue and surfaces
/// are available. Distortion might or might not have completed.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
///
/// \param[in] frameIndex Specifies the targeted application frame index, or 0 to refer to one frame
///        after the last time rev_SubmitFrame was called.
///
/// \param[in] viewScaleDesc Provides additional information needed only if layerPtrList contains
///        an revLayerType_Quad. If NULL, a default version is used based on the current configuration and a 1.0 world scale.
///
/// \param[in] layerPtrList Specifies a list of revLayer pointers, which can include NULL entries to
///        indicate that any previously shown layer at that index is to not be displayed.
///        Each layer header must be a part of a layer structure such as revLayerEyeFov or revLayerQuad,
///        with Header.Type identifying its type. A NULL layerPtrList entry in the array indicates the
//         absence of the given layer.
///
/// \param[in] layerCount Indicates the number of valid elements in layerPtrList. The maximum
///        supported layerCount is not currently specified, but may be specified in a future version.
///
/// - Layers are drawn in the order they are specified in the array, regardless of the layer type.
///
/// - Layers are not remembered between successive calls to rev_SubmitFrame. A layer must be
///   specified in every call to rev_SubmitFrame or it won't be displayed.
///
/// - If a layerPtrList entry that was specified in a previous call to rev_SubmitFrame is
///   passed as NULL or is of type revLayerType_Disabled, that layer is no longer displayed.
///
/// - A layerPtrList entry can be of any layer type and multiple entries of the same layer type
///   are allowed. No layerPtrList entry may be duplicated (i.e. the same pointer as an earlier entry).
///
/// <b>Example code</b>
///     \code{.cpp}
///         revLayerEyeFov  layer0;
///         revLayerQuad    layer1;
///           ...
///         revLayerHeader* layers[2] = { &layer0.Header, &layer1.Header };
///         revResult result = rev_SubmitFrame(hmd, frameIndex, nullptr, layers, 2);
///     \endcode
///
/// \return Returns an revResult for which REV_SUCCESS(result) is false upon error and true
///         upon success. Return values include but aren't limited to:
///     - revSuccess: rendering completed successfully.
///     - revSuccess_NotVisible: rendering completed successfully but was not displayed on the HMD,
///       usually because another application currently has ownership of the HMD. Applications receiving
///       this result should stop rendering new content, but continue to call rev_SubmitFrame periodically
///       until it returns a value other than revSuccess_NotVisible.
///     - revError_DisplayLost: The session has become invalid (such as due to a device removal)
///       and the shared resources need to be released (rev_DestroyTextureSwapChain), the session needs to
///       destroyed (rev_Destroy) and recreated (rev_Create), and new resources need to be created
///       (rev_CreateTextureSwapChainXXX). The application's existing private graphics resources do not
///       need to be recreated unless the new rev_Create call returns a different GraphicsLuid.
///     - revError_TextureSwapChainInvalid: The revTextureSwapChain is in an incomplete or inconsistent state. 
///       Ensure rev_CommitTextureSwapChain was called at least once first.
///
/// \see rev_GetPredictedDisplayTime, revViewScaleDesc, revLayerHeader
///
REV_PUBLIC_FUNCTION(revResult) rev_SubmitFrame(revSession session, long long frameIndex,
                                                  const revViewScaleDesc* viewScaleDesc,
                                                  revLayerHeader const * const * layerPtrList, unsigned int layerCount);
///@}



//-------------------------------------------------------------------------------------
/// @name Frame Timing
///
//@{


/// Gets the time of the specified frame midpoint.
///
/// Predicts the time at which the given frame will be displayed. The predicted time 
/// is the middle of the time period during which the corresponding eye images will 
/// be displayed. 
///
/// The application should increment frameIndex for each successively targeted frame,
/// and pass that index to any relevent OVR functions that need to apply to the frame
/// identified by that index.
///
/// This function is thread-safe and allows for multiple application threads to target
/// their processing to the same displayed frame.
/// 
/// In the even that prediction fails due to various reasons (e.g. the display being off
/// or app has yet to present any frames), the return value will be current CPU time.
/// 
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] frameIndex Identifies the frame the caller wishes to target.
///            A value of zero returns the next frame index.
/// \return Returns the absolute frame midpoint time for the given frameIndex. 
/// \see rev_GetTimeInSeconds
///
REV_PUBLIC_FUNCTION(double) rev_GetPredictedDisplayTime(revSession session, long long frameIndex);


/// Returns global, absolute high-resolution time in seconds.
///
/// The time frame of reference for this function is not specified and should not be
/// depended upon.
///
/// \return Returns seconds as a floating point value.
/// \see revPoseStatef, revFrameTiming
///
REV_PUBLIC_FUNCTION(double) rev_GetTimeInSeconds();


/// Performance HUD enables the HMD user to see information critical to
/// the real-time operation of the VR application such as latency timing,
/// and CPU & GPU performance metrics
///
///     App can toggle performance HUD modes as such:
///     \code{.cpp}
///         revPerfHudMode PerfHudMode = revPerfHud_LatencyTiming;
///         rev_SetInt(Hmd, REV_PERF_HUD_MODE, (int)PerfHudMode);
///     \endcode
///
typedef enum revPerfHudMode_
{
	revPerfHud_Off                = 0,  ///< Turns off the performance HUD
    revPerfHud_PerfSummary        = 1,  ///< Shows performance summary and headroom
    revPerfHud_LatencyTiming      = 2,  ///< Shows latency related timing info
    revPerfHud_AppRenderTiming    = 3,  ///< Shows render timing info for application
    revPerfHud_CompRenderTiming   = 4,  ///< Shows render timing info for OVR compositor
    revPerfHud_VersionInfo        = 5,  ///< Shows SDK & HMD version Info
    revPerfHud_Count              = 6,  ///< \internal Count of enumerated elements.
    revPerfHud_EnumSize = 0x7fffffff    ///< \internal Force type int32_t.
} revPerfHudMode;

/// Layer HUD enables the HMD user to see information about a layer
///
///     App can toggle layer HUD modes as such:
///     \code{.cpp}
///         revLayerHudMode LayerHudMode = revLayerHud_Info;
///         rev_SetInt(Hmd, REV_LAYER_HUD_MODE, (int)LayerHudMode);
///     \endcode
///
typedef enum revLayerHudMode_
{
    revLayerHud_Off = 0, ///< Turns off the layer HUD
    revLayerHud_Info = 1, ///< Shows info about a specific layer
    revLayerHud_EnumSize = 0x7fffffff
} revLayerHudMode;

///@}

/// Debug HUD is provided to help developers gauge and debug the fidelity of their app's
/// stereo rendering characteristics. Using the provided quad and crosshair guides, 
/// the developer can verify various aspects such as VR tracking units (e.g. meters),
/// stereo camera-parallax properties (e.g. making sure objects at infinity are rendered
/// with the proper separation), measuring VR geometry sizes and distances and more.
///
///     App can toggle the debug HUD modes as such:
///     \code{.cpp}
///         revDebugHudStereoMode DebugHudMode = revDebugHudStereo_QuadWithCrosshair;
///         rev_SetInt(Hmd, REV_DEBUG_HUD_STEREO_MODE, (int)DebugHudMode);
///     \endcode
///
/// The app can modify the visual properties of the stereo guide (i.e. quad, crosshair)
/// using the rev_SetFloatArray function. For a list of tweakable properties,
/// see the REV_DEBUG_HUD_STEREO_GUIDE_* keys in the REV_CAPI_Keys.h header file.
typedef enum revDebugHudStereoMode_
{
    revDebugHudStereo_Off                 = 0,  ///< Turns off the Stereo Debug HUD
    revDebugHudStereo_Quad                = 1,  ///< Renders Quad in world for Stereo Debugging
    revDebugHudStereo_QuadWithCrosshair   = 2,  ///< Renders Quad+crosshair in world for Stereo Debugging
    revDebugHudStereo_CrosshairAtInfinity = 3,  ///< Renders screen-space crosshair at infinity for Stereo Debugging
    revDebugHudStereo_Count,                    ///< \internal Count of enumerated elements

    revDebugHudStereo_EnumSize = 0x7fffffff     ///< \internal Force type int32_t
} revDebugHudStereoMode;




// -----------------------------------------------------------------------------------
/// @name Property Access
///
/// These functions read and write OVR properties. Supported properties
/// are defined in REV_CAPI_Keys.h
///
//@{

/// Reads a boolean property.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] propertyName The name of the property, which needs to be valid for only the call.
/// \param[in] defaultVal specifes the value to return if the property couldn't be read.
/// \return Returns the property interpreted as a boolean value. Returns defaultVal if
///         the property doesn't exist.
REV_PUBLIC_FUNCTION(revBool) rev_GetBool(revSession session, const char* propertyName, revBool defaultVal);

/// Writes or creates a boolean property.
/// If the property wasn't previously a boolean property, it is changed to a boolean property.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] value The value to write.
/// \return Returns true if successful, otherwise false. A false result should only occur if the property
///         name is empty or if the property is read-only.
REV_PUBLIC_FUNCTION(revBool) rev_SetBool(revSession session, const char* propertyName, revBool value);


/// Reads an integer property.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] defaultVal Specifes the value to return if the property couldn't be read.
/// \return Returns the property interpreted as an integer value. Returns defaultVal if
///         the property doesn't exist.
REV_PUBLIC_FUNCTION(int) rev_GetInt(revSession session, const char* propertyName, int defaultVal);

/// Writes or creates an integer property.
///
/// If the property wasn't previously a boolean property, it is changed to an integer property.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] value The value to write.
/// \return Returns true if successful, otherwise false. A false result should only occur if the property
///         name is empty or if the property is read-only.
REV_PUBLIC_FUNCTION(revBool) rev_SetInt(revSession session, const char* propertyName, int value);


/// Reads a float property.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] defaultVal specifes the value to return if the property couldn't be read.
/// \return Returns the property interpreted as an float value. Returns defaultVal if
///         the property doesn't exist.
REV_PUBLIC_FUNCTION(float) rev_GetFloat(revSession session, const char* propertyName, float defaultVal);

/// Writes or creates a float property.
/// If the property wasn't previously a float property, it's changed to a float property.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] value The value to write.
/// \return Returns true if successful, otherwise false. A false result should only occur if the property
///         name is empty or if the property is read-only.
REV_PUBLIC_FUNCTION(revBool) rev_SetFloat(revSession session, const char* propertyName, float value);


/// Reads a float array property.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] values An array of float to write to.
/// \param[in] valuesCapacity Specifies the maximum number of elements to write to the values array.
/// \return Returns the number of elements read, or 0 if property doesn't exist or is empty.
REV_PUBLIC_FUNCTION(unsigned int) rev_GetFloatArray(revSession session, const char* propertyName,
                                                       float values[], unsigned int valuesCapacity);

/// Writes or creates a float array property.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] values An array of float to write from.
/// \param[in] valuesSize Specifies the number of elements to write.
/// \return Returns true if successful, otherwise false. A false result should only occur if the property
///         name is empty or if the property is read-only.
REV_PUBLIC_FUNCTION(revBool) rev_SetFloatArray(revSession session, const char* propertyName,
                                                  const float values[], unsigned int valuesSize);


/// Reads a string property.
/// Strings are UTF8-encoded and null-terminated.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] defaultVal Specifes the value to return if the property couldn't be read.
/// \return Returns the string property if it exists. Otherwise returns defaultVal, which can be specified as NULL.
///         The return memory is guaranteed to be valid until next call to rev_GetString or
///         until the HMD is destroyed, whichever occurs first.
REV_PUBLIC_FUNCTION(const char*) rev_GetString(revSession session, const char* propertyName,
                                                  const char* defaultVal);

/// Writes or creates a string property.
/// Strings are UTF8-encoded and null-terminated.
///
/// \param[in] session Specifies an revSession previously returned by rev_Create.
/// \param[in] propertyName The name of the property, which needs to be valid only for the call.
/// \param[in] value The string property, which only needs to be valid for the duration of the call.
/// \return Returns true if successful, otherwise false. A false result should only occur if the property
///         name is empty or if the property is read-only.
REV_PUBLIC_FUNCTION(revBool) rev_SetString(revSession session, const char* propertyName,
                                              const char* value);

///@}

REV_PUBLIC_FUNCTION(revResult) rev_SetQueueAheadFraction(revSession session, float queueAheadFraction);

REV_PUBLIC_FUNCTION(revResult) rev_Lookup(const char* name, void** data);



#ifdef __cplusplus
} // extern "C"
#endif


#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

/// @cond DoxygenIgnore
//-----------------------------------------------------------------------------
// ***** Compiler packing validation
//
// These checks ensure that the compiler settings being used will be compatible
// with with pre-built dynamic library provided with the runtime.

REV_STATIC_ASSERT(sizeof(revBool) == 1,         "revBool size mismatch");
REV_STATIC_ASSERT(sizeof(revVector2i) == 4 * 2, "revVector2i size mismatch");
REV_STATIC_ASSERT(sizeof(revSizei) == 4 * 2,    "revSizei size mismatch");
REV_STATIC_ASSERT(sizeof(revRecti) == sizeof(revVector2i) + sizeof(revSizei), "revRecti size mismatch");
REV_STATIC_ASSERT(sizeof(revQuatf) == 4 * 4,    "revQuatf size mismatch");
REV_STATIC_ASSERT(sizeof(revVector2f) == 4 * 2, "revVector2f size mismatch");
REV_STATIC_ASSERT(sizeof(revVector3f) == 4 * 3, "revVector3f size mismatch");
REV_STATIC_ASSERT(sizeof(revMatrix4f) == 4 * 16, "revMatrix4f size mismatch");

REV_STATIC_ASSERT(sizeof(revPosef) == (7 * 4),       "revPosef size mismatch");
REV_STATIC_ASSERT(sizeof(revPoseStatef) == (22 * 4), "revPoseStatef size mismatch");
REV_STATIC_ASSERT(sizeof(revFovPort) == (4 * 4),     "revFovPort size mismatch");

REV_STATIC_ASSERT(sizeof(revHmdCaps) == 4,      "revHmdCaps size mismatch");
REV_STATIC_ASSERT(sizeof(revTrackingCaps) == 4, "revTrackingCaps size mismatch");
REV_STATIC_ASSERT(sizeof(revEyeType) == 4,      "revEyeType size mismatch");
REV_STATIC_ASSERT(sizeof(revHmdType) == 4,      "revHmdType size mismatch");

REV_STATIC_ASSERT(sizeof(revTrackerDesc) == 4 + 4 + 4 + 4, "revTrackerDesc size mismatch");
REV_STATIC_ASSERT(sizeof(revTrackerPose) == 4 + 4 + sizeof(revPosef) + sizeof(revPosef), "revTrackerPose size mismatch");
REV_STATIC_ASSERT(sizeof(revTrackingState) == sizeof(revPoseStatef) + 4 + 4 + (sizeof(revPoseStatef) * 2) + (sizeof(unsigned int) * 2) + sizeof(revPosef) + 4, "revTrackingState size mismatch");


//REV_STATIC_ASSERT(sizeof(revTextureHeader) == sizeof(revRenderAPIType) + sizeof(revSizei),
//                      "revTextureHeader size mismatch");
//REV_STATIC_ASSERT(sizeof(revTexture) == sizeof(revTextureHeader) REV_ON64(+4) + sizeof(uintptr_t) * 8,
//                      "revTexture size mismatch");
//
REV_STATIC_ASSERT(sizeof(revStatusBits) == 4, "revStatusBits size mismatch");

REV_STATIC_ASSERT(sizeof(revSessionStatus) == 6, "revSessionStatus size mismatch");

REV_STATIC_ASSERT(sizeof(revEyeRenderDesc) == sizeof(revEyeType) + sizeof(revFovPort) + sizeof(revRecti) +
                                                  sizeof(revVector2f) + sizeof(revVector3f),
                      "revEyeRenderDesc size mismatch");
REV_STATIC_ASSERT(sizeof(revTimewarpProjectionDesc) == 4 * 3, "revTimewarpProjectionDesc size mismatch");

REV_STATIC_ASSERT(sizeof(revInitFlags) == 4, "revInitFlags size mismatch");
REV_STATIC_ASSERT(sizeof(revLogLevel) == 4, "revLogLevel size mismatch");

REV_STATIC_ASSERT(sizeof(revInitParams) == 4 + 4 + sizeof(revLogCallback) + sizeof(uintptr_t) + 4 + 4,
                      "revInitParams size mismatch");

REV_STATIC_ASSERT(sizeof(revHmdDesc) ==
    + sizeof(revHmdType)                // Type
    REV_ON64(+ 4)                       // pad0
    + 64                                // ProductName 
    + 64                                // Manufacturer
    + 2                                 // VendorId
    + 2                                 // ProductId
    + 24                                // SerialNumber
    + 2                                 // FirmwareMajor
    + 2                                 // FirmwareMinor
    + 4 * 4                             // AvailableHmdCaps - DefaultTrackingCaps
    + sizeof(revFovPort) * 2            // DefaultEyeFov
    + sizeof(revFovPort) * 2            // MaxEyeFov
    + sizeof(revSizei)                  // Resolution
    + 4                                 // DisplayRefreshRate
    REV_ON64(+ 4)                       // pad1
    , "revHmdDesc size mismatch");


// -----------------------------------------------------------------------------------
// ***** Backward compatibility #includes
//
// This is at the bottom of this file because the following is dependent on the
// declarations above.

#if !defined(REV_CAPI_NO_UTILS)
    #include "Extras/REV_CAPI_Util.h"
#endif

/// @endcond

#endif // REV_CAPI_h
