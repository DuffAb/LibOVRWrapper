/********************************************************************************//**
\file      REV_Version.h
\brief     This header provides LibOVR version identification.
\copyright Copyright 2014-2016 Oculus VR, LLC All Rights reserved.
*************************************************************************************/

#ifndef REV_Version__h
#define REV_Version__h



/// Conventional string-ification macro.
#if !defined(REV_STRINGIZE)
    #define REV_STRINGIZEIMPL(x) #x
    #define REV_STRINGIZE(x)     REV_STRINGIZEIMPL(x)
#endif


// Master version numbers
#define REV_PRODUCT_VERSION_ 1  // Product version doesn't participate in semantic versioning.
#define REV_MAJOR_VERSION_   1  // If you change these values then you need to also make sure to change LibOVR/Projects/Windows/LibOVR.props in parallel.
#define REV_MINOR_VERSION_   3  // 
#define REV_PATCH_VERSION_   0
#define REV_BUILD_NUMBER_    0

// This is the ((product * 100) + major) version of the service that the DLL is compatible with.
// When we backport changes to old versions of the DLL we update the old DLLs
// to move this version number up to the latest version.
// The DLL is responsible for checking that the service is the version it supports
// and returning an appropriate error message if it has not been made compatible.
#define REV_DLL_COMPATIBLE_VERSION 101

#define REV_FEATURE_VERSION 0


/// "Major.Minor.Patch"
#if !defined(REV_VERSION_STRING)
    #define REV_VERSION_STRING  REV_STRINGIZE(REV_MAJOR_VERSION.REV_MINOR_VERSION.REV_PATCH_VERSION)
#endif


/// "Major.Minor.Patch.Build"
#if !defined(REV_DETAILED_VERSION_STRING)
    #define REV_DETAILED_VERSION_STRING REV_STRINGIZE(REV_MAJOR_VERSION.REV_MINOR_VERSION.REV_PATCH_VERSION.REV_BUILD_NUMBER)
#endif


/// \brief file description for version info
/// This appears in the user-visible file properties. It is intended to convey publicly
/// available additional information such as feature builds.
#if !defined(REV_FILE_DESCRIPTION_STRING)
    #if defined(_DEBUG)
        #define REV_FILE_DESCRIPTION_STRING "dev build debug"
    #else
        #define REV_FILE_DESCRIPTION_STRING "dev build"
    #endif
#endif


#endif // REV_Version_h
