/************************************************************************************

Filename    :   REV_CAPIShim.c
Content     :   CAPI DLL user library
Created     :   November 20, 2014
Copyright   :   Copyright 2014-2016 Oculus VR, LLC All Rights reserved.

Licensed under the Oculus VR Rift SDK License Version 3.3 (the "License");
you may not use the Oculus VR Rift SDK except in compliance with the License,
which is provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

You may obtain a copy of the License at

http://www.oculusvr.com/licenses/LICENSE-3.3

Unless required by applicable law or agreed to in writing, the Oculus VR SDK
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************************/


#include "REV_CAPI.h"
#include "REV_Version.h"
#include "REV_ErrorCode.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#if defined(_WIN32)
    #if defined(_MSC_VER)
        #pragma warning(push, 0)
    #endif
    #include <Windows.h>
    #if defined(_MSC_VER)
        #pragma warning(pop)
    #endif

    #include "../Include/REV_CAPI_D3D.h"
#else
    #if defined(__APPLE__)
        #include <mach-o/dyld.h>
        #include <sys/syslimits.h>
        #include <libgen.h>
        #include <pwd.h>
        #include <unistd.h>
    #endif
    #include <dlfcn.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif
#include "../Include/REV_CAPI_GL.h"


#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable: 4996) // 'getenv': This function or variable may be unsafe.
#endif

static const uint8_t OculusSDKUniqueIdentifier[] = 
{
    0x9E, 0xB2, 0x0B, 0x1A, 0xB7, 0x97, 0x09, 0x20, 0xE0, 0xFB, 0x83, 0xED, 0xF8, 0x33, 0x5A, 0xEB, 
    0x80, 0x4D, 0x8E, 0x92, 0x20, 0x69, 0x13, 0x56, 0xB4, 0xBB, 0xC4, 0x85, 0xA7, 0x9E, 0xA4, 0xFE,
    REV_MAJOR_VERSION_, REV_MINOR_VERSION_, REV_PATCH_VERSION_
};

static const uint8_t OculusSDKUniqueIdentifierXORResult = 0xcb;


// -----------------------------------------------------------------------------------
// ***** REV_ENABLE_DEVELOPER_SEARCH
//
// If defined then our shared library loading code searches for developer build
// directories.
//
#if !defined(REV_ENABLE_DEVELOPER_SEARCH)
#endif


// -----------------------------------------------------------------------------------
// ***** REV_BUILD_DEBUG
//
// Defines REV_BUILD_DEBUG when the compiler default debug preprocessor is set.
//
// If you want to control the behavior of these flags, then explicitly define
// either -DREV_BUILD_RELEASE or -DREV_BUILD_DEBUG in the compiler arguments.

#if !defined(REV_BUILD_DEBUG) && !defined(REV_BUILD_RELEASE)
    #if defined(_MSC_VER)
        #if defined(_DEBUG)
            #define REV_BUILD_DEBUG
        #endif
    #else
        #if defined(DEBUG)
            #define REV_BUILD_DEBUG
        #endif
    #endif
#endif


//-----------------------------------------------------------------------------------
// ***** FilePathCharType, ModuleHandleType, ModuleFunctionType
//
#if defined(_WIN32)                                       // We need to use wchar_t on Microsoft platforms, as that's the native file system character type.
    #define FilePathCharType       wchar_t                // #define instead of typedef because debuggers (VC++, XCode) don't recognize typedef'd types as a string type.
    typedef HMODULE                ModuleHandleType;
    typedef FARPROC                ModuleFunctionType;
#else
    #define FilePathCharType       char
    typedef void*                  ModuleHandleType;
    typedef void*                  ModuleFunctionType;
#endif

#define ModuleHandleTypeNull   ((ModuleHandleType)NULL)
#define ModuleFunctionTypeNull ((ModuleFunctionType)NULL)


//-----------------------------------------------------------------------------------
// ***** REV_MAX_PATH
//
#if !defined(REV_MAX_PATH)
    #if defined(_WIN32)
        #define REV_MAX_PATH  _MAX_PATH
    #elif defined(__APPLE__)
        #define REV_MAX_PATH  PATH_MAX
    #else
        #define REV_MAX_PATH  1024
    #endif
#endif



//-----------------------------------------------------------------------------------
// ***** REV_DECLARE_IMPORT
//
// Creates typedef and pointer declaration for a function of a given signature.
// The typedef is <FunctionName>Type, and the pointer is <FunctionName>Ptr.
//
// Example usage:
//     int MultiplyValues(float x, float y);  // Assume this function exists in an external shared library. We don't actually need to redeclare it.
//     REV_DECLARE_IMPORT(int, MultiplyValues, (float x, float y)) // This creates a local typedef and pointer for it.

#define REV_DECLARE_IMPORT(ReturnValue, FunctionName, Arguments)  \
    typedef ReturnValue (REV_CDECL *FunctionName##Type)Arguments; \
    FunctionName##Type FunctionName##Ptr = NULL;


	static size_t REV_strlcpy(char* dest, const char* src, size_t destsize)
	{
		const char* s = src;
		size_t      n = destsize;

		if (n && --n)
		{
			do {
				if ((*dest++ = *s++) == 0)
					break;
			} while (--n);
		}

		if (!n)
		{
			if (destsize)
				*dest = 0;
			while (*s++)
			{
			}
		}

		return (size_t)((s - src) - 1);
	}


	static size_t REV_strlcat(char* dest, const char* src, size_t destsize)
	{
		const size_t d = destsize ? strlen(dest) : 0;
		const size_t s = strlen(src);
		const size_t t = s + d;

		if (t < destsize)
			memcpy(dest + d, src, (s + 1) * sizeof(*src));
		else
		{
			if (destsize)
			{
				memcpy(dest + d, src, ((destsize - d) - 1) * sizeof(*src));
				dest[destsize - 1] = 0;
			}
		}

		return t;
	}

//-----------------------------------------------------------------------------------
// ***** REV_GETFUNCTION
//
// Loads <FunctionName>Ptr from hLibOVR if not already loaded.
// Assumes a variable named <FunctionName>Ptr of type <FunctionName>Type exists which is called <FunctionName> in LibOVR.
//
// Example usage:
//     REV_GETFUNCTION(MultiplyValues)    // Normally this would be done on library init and not before every usage.
//     int result = ovrtiplyValuesPtr(3.f, 4.f);



//ADDED
static LPCSTR fixname(LPCSTR name) {
	size_t s = strlen(name) + (1 - 3) * sizeof(char); //remove 
	char *r = (char*)malloc(s * sizeof(char));
	strncpy(r, name, strlen(name) - 3);
	r[strlen(name) - 3] = '\0';
	 
	return r;
}

#if !defined(REV_DLSYM)
    #if defined(_WIN32)
        #define REV_DLSYM(dlImage, name) GetProcAddress(dlImage, name)
    #else
        #define REV_DLSYM(dlImage, name) dlsym(dlImage, name)
    #endif
#endif

#define REV_GETFUNCTION(f)             \
    if(!f##Ptr)                        \
    {                                  \
        union                          \
        {                              \
            f##Type p1;                \
            ModuleFunctionType p2;     \
        } u;                           \
        u.p2 = REV_DLSYM(hLibOVR, #f); \
        f##Ptr = u.p1;                 \
    }




#if defined(__APPLE__)
    static revBool REV_strend(const char* pStr, const char* pFind, size_t strLength, size_t findLength)
    {
        if(strLength == (size_t)-1)
            strLength = strlen(pStr);
        if(findLength == (size_t)-1)
            findLength = strlen(pFind);
        if(strLength >= findLength)
            return (strcmp(pStr + strLength - findLength, pFind) == 0);
        return revFalse;
    }

    static revBool REV_isBundleFolder(const char* filePath)
    {
        static const char* extensionArray[] = { ".app", ".bundle", ".framework", ".plugin", ".kext" };
        size_t i;

        for(i = 0; i < sizeof(extensionArray)/sizeof(extensionArray[0]); i++)
        {
            if(REV_strend(filePath, extensionArray[i], (size_t)-1, (size_t)-1))
                return revTrue;
        }

        return revFalse;
    }
#endif


#if defined(REV_ENABLE_DEVELOPER_SEARCH)

// Returns true if the path begins with the given prefix.
// Doesn't support non-ASCII paths, else the return value may be incorrect.
static int REV_PathStartsWith(const FilePathCharType* path, const char* prefix)
{
    while(*prefix)
    {
        if(tolower((unsigned char)*path++) != tolower((unsigned char)*prefix++))
            return revFalse;
    }

    return revTrue;
}

#endif


static revBool REV_GetCurrentWorkingDirectory(FilePathCharType* directoryPath, size_t directoryPathCapacity)
{
    #if defined(_WIN32)
        DWORD dwSize = GetCurrentDirectoryW((DWORD)directoryPathCapacity, directoryPath);

        if((dwSize > 0) && (directoryPathCapacity > 1)) // Test > 1 so we have room to possibly append a \ char.
        {
            size_t length = wcslen(directoryPath);

            if((length == 0) || ((directoryPath[length - 1] != L'\\') && (directoryPath[length - 1] != L'/')))
            {
                directoryPath[length++] = L'\\';
                directoryPath[length]   = L'\0';
            }

            return revTrue;
        }

    #else
        char* cwd = getcwd(directoryPath, directoryPathCapacity);

        if(cwd && directoryPath[0] && (directoryPathCapacity > 1)) // Test > 1 so we have room to possibly append a / char.
        {
            size_t length = strlen(directoryPath);

            if((length == 0) || (directoryPath[length - 1] != '/'))
            {
                directoryPath[length++] = '/';
                directoryPath[length]   = '\0';
            }

            return revTrue;
        }
    #endif

    if(directoryPathCapacity > 0)
        directoryPath[0] = '\0';

    return revFalse;
}


// The appContainer argument is specific currently to only Macintosh. If true and the application is a .app bundle then it returns the
// location of the bundle and not the path to the executable within the bundle. Else return the path to the executable binary itself.
// The moduleHandle refers to the relevant dynamic (a.k.a. shared) library. The main executable is the main module, and each of the shared
// libraries is a module. This way you can specify that you want to know the directory of the given shared library, which may be different
// from the main executable. If the moduleHandle is NULL then the current application module is used.
static revBool REV_GetCurrentApplicationDirectory(FilePathCharType* directoryPath, size_t directoryPathCapacity, revBool appContainer, ModuleHandleType moduleHandle)
{
    #if defined(_WIN32)
        DWORD length = GetModuleFileNameW(moduleHandle, directoryPath, (DWORD)directoryPathCapacity);
        DWORD pos;

        if((length != 0) && (length < (DWORD)directoryPathCapacity)) // If there wasn't an error and there was enough capacity...
        {
            for(pos = length; (pos > 0) && (directoryPath[pos] != '\\') && (directoryPath[pos] != '/'); --pos)
            {
                if((directoryPath[pos - 1] != '\\') && (directoryPath[pos - 1] != '/'))
                   directoryPath[pos - 1] = 0;
            }

            return revTrue;
        }

        (void)appContainer; // Not used on this platform.

    #elif defined(__APPLE__)
        uint32_t directoryPathCapacity32 = (uint32_t)directoryPathCapacity;
        int result = _NSGetExecutablePath(directoryPath, &directoryPathCapacity32);

        if(result == 0) // If success...
        {
            char realPath[REV_MAX_PATH];

            if(realpath(directoryPath, realPath)) // realpath returns the canonicalized absolute file path.
            {
                size_t length = 0;

                if(appContainer) // If the caller wants the path to the containing bundle...
                {
                    char    containerPath[REV_MAX_PATH];
                    revBool pathIsContainer;

                    REV_strlcpy(containerPath, realPath, sizeof(containerPath));
                    pathIsContainer = REV_isBundleFolder(containerPath);

                    while(!pathIsContainer && strncmp(containerPath, ".", REV_MAX_PATH) && strncmp(containerPath, "/", REV_MAX_PATH)) // While the container we're looking for is not found and while the path doesn't start with a . or /
                    {
                        REV_strlcpy(containerPath, dirname(containerPath), sizeof(containerPath));
                        pathIsContainer = REV_isBundleFolder(containerPath);
                    }

                    if(pathIsContainer)
                        length = REV_strlcpy(directoryPath, containerPath, directoryPathCapacity);
                }

                if(length == 0) // If not set above in the appContainer block...
                    length = REV_strlcpy(directoryPath, realPath, directoryPathCapacity);

                while(length-- && (directoryPath[length] != '/'))
                    directoryPath[length] = '\0'; // Strip the file name from the file path, leaving a trailing / char.

                return revTrue;
            }
        }

        (void)moduleHandle;  // Not used on this platform.

    #else
        ssize_t length = readlink("/proc/self/exe", directoryPath, directoryPathCapacity);
        ssize_t pos;

        if(length > 0)
        {
            for(pos = length; (pos > 0) && (directoryPath[pos] != '/'); --pos)
            {
                if(directoryPath[pos - 1] != '/')
                   directoryPath[pos - 1]  = '\0';
            }

            return revTrue;
        }

        (void)appContainer; // Not used on this platform.
        (void)moduleHandle;
    #endif

    if(directoryPathCapacity > 0)
        directoryPath[0] = '\0';

    return revFalse;
}


#if defined(_WIN32) || defined(REV_ENABLE_DEVELOPER_SEARCH) // Used only in these cases

// Get the file path to the current module's (DLL or EXE) directory within the current process.
// Will be different from the process module handle if the current module is a DLL and is in a different directory than the EXE module.
// If successful then directoryPath will be valid and revTrue is returned, else directoryPath will be empty and revFalse is returned.
static revBool REV_GetCurrentModuleDirectory(FilePathCharType* directoryPath, size_t directoryPathCapacity, revBool appContainer)
{
    #if defined(_WIN32)
        HMODULE hModule;
        BOOL result = GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)(uintptr_t)REV_GetCurrentModuleDirectory, &hModule);
        if(result)
            REV_GetCurrentApplicationDirectory(directoryPath, directoryPathCapacity, revTrue, hModule);
        else
            directoryPath[0] = 0;

        (void)appContainer;

        return directoryPath[0] ? revTrue : revFalse;
    #else
        return REV_GetCurrentApplicationDirectory(directoryPath, directoryPathCapacity, appContainer, NULL);
    #endif
}

#endif

#if defined(_WIN32)

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4201)
#endif

#include <Softpub.h>
#include <Wincrypt.h>

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

// Expected certificates:
#define ExpectedNumCertificates 3
typedef struct CertificateEntry_t {
    const wchar_t* Issuer;
    const wchar_t* Subject;
} CertificateEntry;

CertificateEntry NewCertificateChain[ExpectedNumCertificates] = {
    { L"DigiCert SHA2 Assured ID Code Signing CA", L"Oculus VR, LLC" },
    { L"DigiCert Assured ID Root CA", L"DigiCert SHA2 Assured ID Code Signing CA" },
    { L"DigiCert Assured ID Root CA", L"DigiCert Assured ID Root CA" },
};

#define CertificateChainCount 1
CertificateEntry* AllowedCertificateChains[CertificateChainCount] = {
    NewCertificateChain
};

typedef WINCRYPT32API
DWORD
(WINAPI *PtrCertGetNameStringW)(
    PCCERT_CONTEXT pCertContext,
    DWORD dwType,
    DWORD dwFlags,
    void *pvTypePara,
    LPWSTR pszNameString,
    DWORD cchNameString
    );
typedef LONG (WINAPI *PtrWinVerifyTrust)(HWND hwnd, GUID *pgActionID,
                                  LPVOID pWVTData);
typedef CRYPT_PROVIDER_DATA * (WINAPI *PtrWTHelperProvDataFromStateData)(HANDLE hStateData);
typedef CRYPT_PROVIDER_SGNR * (WINAPI *PtrWTHelperGetProvSignerFromChain)(
    CRYPT_PROVIDER_DATA *pProvData, DWORD idxSigner, BOOL fCounterSigner, DWORD idxCounterSigner);

PtrCertGetNameStringW m_PtrCertGetNameStringW = 0;
PtrWinVerifyTrust m_PtrWinVerifyTrust = 0;
PtrWTHelperProvDataFromStateData m_PtrWTHelperProvDataFromStateData = 0;
PtrWTHelperGetProvSignerFromChain m_PtrWTHelperGetProvSignerFromChain = 0;

typedef enum ValidateCertificateContentsResult_
{
    VCCRSuccess          =  0,
    VCCRErrorCertCount   = -1,
    VCCRErrorTrust       = -2,
    VCCRErrorValidation  = -3
} ValidateCertificateContentsResult;

static ValidateCertificateContentsResult ValidateCertificateContents(CertificateEntry* chain, CRYPT_PROVIDER_SGNR* cps)
{
    int certIndex;

    if (!cps ||
        !cps->pasCertChain ||
        cps->csCertChain != ExpectedNumCertificates)
    {
        return VCCRErrorCertCount;
    }

    for (certIndex = 0; certIndex < ExpectedNumCertificates; ++certIndex)
    {
        CRYPT_PROVIDER_CERT* pCertData = &cps->pasCertChain[certIndex];
        wchar_t subjectStr[400] = { 0 };
        wchar_t issuerStr[400] = { 0 };

        if ((pCertData->fSelfSigned && !pCertData->fTrustedRoot) ||
            pCertData->fTestCert)
        {
            return VCCRErrorTrust;
        }

        m_PtrCertGetNameStringW(
            pCertData->pCert,
            CERT_NAME_ATTR_TYPE,
            0,
            szOID_COMMON_NAME,
            subjectStr,
            ARRAYSIZE(subjectStr));

        m_PtrCertGetNameStringW(
            pCertData->pCert,
            CERT_NAME_ATTR_TYPE,
            CERT_NAME_ISSUER_FLAG,
            0,
            issuerStr,
            ARRAYSIZE(issuerStr));

        if (wcscmp(subjectStr, chain[certIndex].Subject) != 0 ||
            wcscmp(issuerStr, chain[certIndex].Issuer) != 0)
        {
            return VCCRErrorValidation;
        }
    }

    return VCCRSuccess;
}

#define REV_SIGNING_CONVERT_PTR(ftype, fptr, procaddr) { \
        union { ftype p1; ModuleFunctionType p2; } u; \
        u.p2 = procaddr; \
        fptr = u.p1; }

static HANDLE REV_Win32_SignCheck(FilePathCharType* fullPath)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    WINTRUST_FILE_INFO fileData;
    WINTRUST_DATA wintrustData;
    GUID actionGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    LONG resultStatus;
    int verified = 0;
    HMODULE libWinTrust = LoadLibraryW(L"wintrust");
    HMODULE libCrypt32 = LoadLibraryW(L"crypt32");
    if (libWinTrust == NULL || libCrypt32 == NULL)
    {
        return INVALID_HANDLE_VALUE;
    }

    REV_SIGNING_CONVERT_PTR(PtrCertGetNameStringW, m_PtrCertGetNameStringW, GetProcAddress(libCrypt32, "CertGetNameStringW"));
    REV_SIGNING_CONVERT_PTR(PtrWinVerifyTrust, m_PtrWinVerifyTrust, GetProcAddress(libWinTrust, "WinVerifyTrust"));
    REV_SIGNING_CONVERT_PTR(PtrWTHelperProvDataFromStateData, m_PtrWTHelperProvDataFromStateData, GetProcAddress(libWinTrust, "WTHelperProvDataFromStateData"));
    REV_SIGNING_CONVERT_PTR(PtrWTHelperGetProvSignerFromChain, m_PtrWTHelperGetProvSignerFromChain, GetProcAddress(libWinTrust, "WTHelperGetProvSignerFromChain"));

    if (m_PtrCertGetNameStringW == NULL || m_PtrWinVerifyTrust == NULL ||
        m_PtrWTHelperProvDataFromStateData == NULL || m_PtrWTHelperGetProvSignerFromChain == NULL)
    {
        return INVALID_HANDLE_VALUE;
    }

    if (!fullPath)
    {
        return INVALID_HANDLE_VALUE;
    }

    hFile = CreateFileW(fullPath, GENERIC_READ, FILE_SHARE_READ,
                        0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return INVALID_HANDLE_VALUE;
    }

    ZeroMemory(&fileData, sizeof(fileData));
    fileData.cbStruct      = sizeof(fileData);
    fileData.pcwszFilePath = fullPath;
    fileData.hFile         = hFile;

    ZeroMemory(&wintrustData, sizeof(wintrustData));
    wintrustData.cbStruct      = sizeof(wintrustData);
    wintrustData.pFile         = &fileData;
    wintrustData.dwUnionChoice = WTD_CHOICE_FILE; // Specify WINTRUST_FILE_INFO.
    wintrustData.dwUIChoice    = WTD_UI_NONE; // Do not display any UI.
    wintrustData.dwUIContext   = WTD_UICONTEXT_EXECUTE; // Hint that this is about app execution.
    wintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    wintrustData.dwProvFlags = WTD_REVOCATION_CHECK_NONE;
    wintrustData.dwStateAction = WTD_STATEACTION_VERIFY;
    wintrustData.hWVTStateData = 0;

    resultStatus = m_PtrWinVerifyTrust(
        (HWND)INVALID_HANDLE_VALUE, // Do not display any UI.
        &actionGUID, // V2 verification
        &wintrustData);

    if (resultStatus == ERROR_SUCCESS &&
        wintrustData.hWVTStateData != 0 &&
        wintrustData.hWVTStateData != INVALID_HANDLE_VALUE)
    {
        CRYPT_PROVIDER_DATA* cpd = m_PtrWTHelperProvDataFromStateData(wintrustData.hWVTStateData);
        if (cpd && cpd->csSigners == 1)
        {
            CRYPT_PROVIDER_SGNR* cps = m_PtrWTHelperGetProvSignerFromChain(cpd, 0, FALSE, 0);
            int chainIndex;
            for (chainIndex = 0; chainIndex < CertificateChainCount; ++chainIndex)
            {
                CertificateEntry* chain = AllowedCertificateChains[chainIndex];
                if (0 == ValidateCertificateContents(chain, cps))
                {
                    verified = 1;
                    break;
                }
            }
        }
    }

    wintrustData.dwStateAction = WTD_STATEACTION_CLOSE;

    m_PtrWinVerifyTrust(
        (HWND)INVALID_HANDLE_VALUE, // Do not display any UI.
        &actionGUID, // V2 verification
        &wintrustData);

    if (verified != 1)
    {
        CloseHandle(hFile);
        return INVALID_HANDLE_VALUE;
    }

    return hFile;
}

#endif // #if defined(_WIN32)

static ModuleHandleType REV_OpenLibrary(const FilePathCharType* libraryPath)
{
    #if defined(_WIN32)
		return LoadLibraryW(libraryPath);
#if 0
        DWORD fullPathNameLen = 0;
        FilePathCharType fullPath[MAX_PATH] = { 0 };
        HANDLE hFilePinned = INVALID_HANDLE_VALUE;
        ModuleHandleType hModule = 0;
		swprintf(fullPath, MAX_PATH, L"%ls", libraryPath);

        fullPathNameLen = GetFullPathNameW(libraryPath, MAX_PATH, fullPath, 0);
        if (fullPathNameLen <= 0 || fullPathNameLen >= MAX_PATH)
        {
            return 0;
        }
        fullPath[MAX_PATH - 1] = 0;

        hFilePinned = REV_Win32_SignCheck(fullPath);
        if (hFilePinned == INVALID_HANDLE_VALUE)
        {
            return 0;
        }

        hModule = LoadLibraryW(fullPath);

        if (hFilePinned != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hFilePinned);
        }

        return hModule;
#endif
    #else
        // Don't bother trying to dlopen() a file that is not even there.
        if (access(libraryPath, X_OK | R_OK ) != 0)
        {
            return NULL;
        }

        dlerror(); // Clear any previous dlopen() errors

        // Use RTLD_NOW because we don't want unexpected stalls at runtime, and the library isn't very large.
        // Use RTLD_LOCAL to avoid unilaterally exporting resolved symbols to the rest of this process.
        void *lib = dlopen(libraryPath, RTLD_NOW | RTLD_LOCAL);

        if (!lib)
        {
            #if defined(__APPLE__)
            // TODO: Output the error in whatever logging system OSX uses (jhughes)
            #else  // __APPLE__
            fprintf(stderr, "ERROR: Can't load '%s':\n%s\n", libraryPath, dlerror());
            #endif // __APPLE__
        }

        return lib;
    #endif
}


static void REV_CloseLibrary(ModuleHandleType hLibrary)
{
    if (hLibrary)
    {
        #if defined(_WIN32)
            // We may need to consider what to do in the case that the library is in an exception state.
            // In a Windows C++ DLL, all global objects (including static members of classes) will be constructed just 
            // before the calling of the DllMain with DLL_PROCESS_ATTACH and they will be destroyed just after 
            // the call of the DllMain with DLL_PROCESS_DETACH. We may need to intercept DLL_PROCESS_DETACH and 
            // have special handling for the case that the DLL is broken.
            FreeLibrary(hLibrary);
        #else
            dlclose(hLibrary);
        #endif
    }
}


// Returns a valid ModuleHandleType (e.g. Windows HMODULE) or returns ModuleHandleTypeNull (e.g. NULL).
// The caller is required to eventually call REV_CloseLibrary on a valid return handle.
//
static ModuleHandleType REV_FindLibraryPath(int requestedProductVersion, int requestedMajorVersion,
                               FilePathCharType* libraryPath, size_t libraryPathCapacity)
{
    ModuleHandleType moduleHandle;
    int printfResult;
    FilePathCharType developerDir[REV_MAX_PATH] = { '\0' };

    #if defined(_MSC_VER)
        #if defined(_WIN64)
            const char* pBitDepth = "64";
        #else
            const char* pBitDepth = "32";
        #endif
    #elif defined(__APPLE__)
        // For Apple platforms we are using a Universal Binary LibOVRRT dylib which has both 32 and 64 in it.
    #else // Other Unix.
        #if defined(__x86_64__)
            const char* pBitDepth = "64";
        #else
            const char* pBitDepth = "32";
        #endif
    #endif

    (void)requestedProductVersion;

    moduleHandle = ModuleHandleTypeNull;
    if(libraryPathCapacity)
        libraryPath[0] = '\0';

    // Note: REV_ENABLE_DEVELOPER_SEARCH is deprecated in favor of the simpler LIBREV_DLL_DIR, as the edge
    // case uses of the former created some complications that may be best solved by simply using a LIBREV_DLL_DIR
    // environment variable which the user can set in their debugger or system environment variables.
    #if (defined(_MSC_VER) || defined(_WIN32)) && !defined(REV_FILE_PATH_SEPARATOR)
        #define REV_FILE_PATH_SEPARATOR "\\"
    #else
        #define REV_FILE_PATH_SEPARATOR "/"
    #endif

    {
        const char* pLibOvrDllDir = getenv("LIBREV_DLL_DIR"); // Example value: /dev/OculusSDK/Main/LibOVR/Mac/Debug/

        if(pLibOvrDllDir)
        {
            char developerDir8[REV_MAX_PATH];
            size_t length = REV_strlcpy(developerDir8, pLibOvrDllDir, sizeof(developerDir8)); // If missing a trailing path separator then append one.

            if((length > 0) && (length < sizeof(developerDir8)) && (developerDir8[length - 1] != REV_FILE_PATH_SEPARATOR[0]))
            {
                length = REV_strlcat(developerDir8, REV_FILE_PATH_SEPARATOR, sizeof(developerDir8));

                if(length < sizeof(developerDir8))
                {
                    #if defined(_WIN32)
                        size_t i;
                        for(i = 0; i <= length; ++i) // ASCII conversion of 8 to 16 bit text.
                            developerDir[i] = (FilePathCharType)(uint8_t)developerDir8[i];
                    #else
                        REV_strlcpy(developerDir, developerDir8, sizeof(developerDir));
                    #endif
                }
            }
        }
    }

    // Support checking for a developer library location override via the REV_SDK_ROOT environment variable.
    // This pathway is deprecated in favor of using LIBREV_DLL_DIR instead.
    #if defined(REV_ENABLE_DEVELOPER_SEARCH)
    if (!developerDir[0]) // If not already set by LIBREV_DLL_PATH...
    {
        // __FILE__ maps to <sdkRoot>/LibOVR/Src/REV_CAPIShim.c
        char sdkRoot[REV_MAX_PATH];
        char* pLibOVR;
        size_t i;

        // We assume that __FILE__ returns a full path, which isn't the case for some compilers.
        // Need to compile with /FC under VC++ for __FILE__ to expand to the full file path.
        // clang expands __FILE__ to a full path by default.
        REV_strlcpy(sdkRoot, __FILE__, sizeof(sdkRoot));
        for(i = 0; sdkRoot[i]; ++i)
            sdkRoot[i] = (char)tolower(sdkRoot[i]); // Microsoft doesn't maintain case.
        pLibOVR = strstr(sdkRoot, "libovr");
        if(pLibOVR && (pLibOVR > sdkRoot))
            pLibOVR[-1] = '\0';
        else
            sdkRoot[0] = '\0';

        if(sdkRoot[0])
        {
            // We want to use a developer version of the library only if the application is also being executed from
            // a developer location. Ideally we would do this by checking that the relative path from the executable to
            // the shared library is the same at runtime as it was when the executable was first built, but we don't have
            // an easy way to do that from here and it would require some runtime help from the application code.
            // Instead we verify that the application is simply in the same developer tree that was was when built.
            // We could put in some additional logic to make it very likely to know if the EXE is in its original location.
            FilePathCharType modulePath[REV_MAX_PATH];
            const revBool pathMatch = REV_GetCurrentModuleDirectory(modulePath, REV_MAX_PATH, revTrue) &&
                                        (REV_PathStartsWith(modulePath, sdkRoot) == revTrue);
            if(pathMatch == revFalse)
            {
                sdkRoot[0] = '\0'; // The application module is not in the developer tree, so don't try to use the developer shared library.
            }
        }

        if(sdkRoot[0])
        {
            #if defined(REV_BUILD_DEBUG)
                const char* pConfigDirName = "Debug";
            #else
                const char* pConfigDirName = "Release";
            #endif

            #if defined(_MSC_VER)
                #if defined(_WIN64)
                    const char* pArchDirName = "x64";
                #else
                    const char* pArchDirName = "Win32";
                #endif
            #else
                #if defined(__x86_64__)
                    const char* pArchDirName = "x86_64";
                #else
                    const char* pArchDirName = "i386";
                #endif
            #endif

            #if defined(_MSC_VER) && (_MSC_VER == 1600)
                const char* pCompilerVersion = "VS2010";
            #elif defined(_MSC_VER) && (_MSC_VER == 1700)
                const char* pCompilerVersion = "VS2012";
            #elif defined(_MSC_VER) && (_MSC_VER == 1800)
                const char* pCompilerVersion = "VS2013";
            #elif defined(_MSC_VER) && (_MSC_VER == 1900)
                const char* pCompilerVersion = "VS2014";
            #endif

            #if defined(_WIN32)
                int count = swprintf_s(developerDir, REV_MAX_PATH, L"%hs\\LibOVR\\Lib\\Windows\\%hs\\%hs\\%hs\\",
                                        sdkRoot, pArchDirName, pConfigDirName, pCompilerVersion);
            #elif defined(__APPLE__)
                // Apple/XCode doesn't let you specify an arch in build paths, which is OK if we build a universal binary.
                (void)pArchDirName;
                int count = snprintf(developerDir, REV_MAX_PATH, "%s/LibOVR/Lib/Mac/%s/",
                                        sdkRoot, pConfigDirName);
            #else
                int count = snprintf(developerDir, REV_MAX_PATH, "%s/LibOVR/Lib/Linux/%s/%s/", 
                                        sdkRoot, pArchDirName, pConfigDirName);
            #endif

            if((count < 0) || (count >= (int)REV_MAX_PATH)) // If there was an error or capacity overflow... clear the string.
            {
                developerDir[0] = '\0';
            }
        }
    }
    #endif // REV_ENABLE_DEVELOPER_SEARCH

    {
        #if !defined(_WIN32)
            FilePathCharType cwDir[REV_MAX_PATH]; // Will be filled in below.
            FilePathCharType appDir[REV_MAX_PATH];
        #endif
        size_t i;

        #if defined(_WIN32)
            // On Windows, only search the developer directory and the usual path
			FilePathCharType  moduleDir[REV_MAX_PATH];
            const FilePathCharType* directoryArray[3];
            directoryArray[0] = developerDir; // Developer directory.
            directoryArray[1] = moduleDir; // No directory, which causes Windows to use the standard search strategy to find the DLL.
			REV_GetCurrentModuleDirectory(moduleDir, sizeof(moduleDir) / sizeof(moduleDir[0]), revTrue);
			directoryArray[2] = L"";
        #elif defined(__APPLE__)
            // https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man1/dyld.1.html

            FilePathCharType  homeDir[REV_MAX_PATH];
            FilePathCharType  homeFrameworkDir[REV_MAX_PATH];
            const FilePathCharType* directoryArray[5];
            size_t            homeDirLength = 0;

            const char* pHome = getenv("HOME"); // Try getting the HOME environment variable.

            if (pHome)
            {
                homeDirLength = REV_strlcpy(homeDir, pHome, sizeof(homeDir));
            }
            else
            {
                // https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man3/getpwuid_r.3.html
                const long pwBufferSize = sysconf(_SC_GETPW_R_SIZE_MAX);

                if (pwBufferSize != -1)
                {
                    char pwBuffer[pwBufferSize];
                    struct passwd  pw;
                    struct passwd* pwResult = NULL;

                    if ((getpwuid_r(getuid(), &pw, pwBuffer, pwBufferSize, &pwResult) == 0) && pwResult)
                        homeDirLength = REV_strlcpy(homeDir, pw.pw_dir, sizeof(homeDir));
                }
            }

            if (homeDirLength)
            {
                if (homeDir[homeDirLength - 1] == '/')
                    homeDir[homeDirLength - 1] = '\0';
                REV_strlcpy(homeFrameworkDir, homeDir, sizeof(homeFrameworkDir));
                REV_strlcat(homeFrameworkDir, "/Library/Frameworks/", sizeof(homeFrameworkDir));
            }
            else
            {
                homeFrameworkDir[0] = '\0';
            }

            directoryArray[0] = cwDir;
            directoryArray[1] = appDir;
            directoryArray[2] = homeFrameworkDir;           // ~/Library/Frameworks/
            directoryArray[3] = "/Library/Frameworks/";     // DYLD_FALLBACK_FRAMEWORK_PATH
            directoryArray[4] = developerDir;               // Developer directory.

        #else
            #define STR1(x) #x
            #define STR(x)  STR1(x)
            #ifdef LIBDIR
                #define TEST_LIB_DIR STR(LIBDIR) "/"
            #else
                #define TEST_LIB_DIR appDir
            #endif

            const FilePathCharType* directoryArray[5];
            directoryArray[0] = cwDir;
            directoryArray[1] = TEST_LIB_DIR;           // Directory specified by LIBDIR if defined.
            directoryArray[2] = developerDir;           // Developer directory.
            directoryArray[3] = "/usr/local/lib/";
            directoryArray[4] = "/usr/lib/";
        #endif

        #if !defined(_WIN32)
            REV_GetCurrentWorkingDirectory(cwDir, sizeof(cwDir) / sizeof(cwDir[0]));
            REV_GetCurrentApplicationDirectory(appDir, sizeof(appDir) / sizeof(appDir[0]), revTrue, NULL);
        #endif

        // Versioned file expectations.
        //     Windows: LibOVRRT<BIT_DEPTH>_<PRODUCT_VERSION>_<MAJOR_VERSION>.dll                                  // Example: LibOVRRT64_1_1.dll -- LibOVRRT 64 bit, product 1, major version 1, minor/patch/build numbers unspecified in the name.
        //     Mac:     LibOVRRT_<PRODUCT_VERSION>.framework/Versions/<MAJOR_VERSION>/LibOVRRT_<PRODUCT_VERSION>   // We are not presently using the .framework bundle's Current directory to hold the version number. This may change.
        //     Linux:   libOVRRT<BIT_DEPTH>_<PRODUCT_VERSION>.so.<MAJOR_VERSION>                                   // The file on disk may contain a minor version number, but a symlink is used to map this major-only version to it.

        // Since we are manually loading the LibOVR dynamic library, we need to look in various locations for a file
        // that matches our requirements. The functionality required is somewhat similar to the operating system's
        // dynamic loader functionality. Each OS has some differences in how this is handled.
        // Future versions of this may iterate over all libOVRRT.so.* files in the directory and use the one that matches our requirements.
        //
        // We need to look for a library that matches the product version and major version of the caller's request,
        // and that library needs to support a minor version that is >= the requested minor version. Currently we
        // don't test the minor version here, as the library is named based only on the product and major version.
        // Currently the minor version test is handled via the initialization of the library and the initialization
        // fails if minor version cannot be supported by the library. The reason this is done during initialization
        // is that the library can at runtime support multiple minor versions based on the user's request. To the
        // external user, all that matters it that they call rev_Initialize with a requested version and it succeeds
        // or fails.
        //
        // The product version is something that is at a higher level than the major version, and is not something that's
        // always seen in libraries (an example is the well-known LibXml2 library, in which the 2 is essentially the product version).

        for(i = 0; i < sizeof(directoryArray)/sizeof(directoryArray[0]); ++i)
        {
            #if defined(_WIN32)
                //printfResult = swprintf(libraryPath, libraryPathCapacity, L"%lsLibOVRRevive_%d.dll", directoryArray[i], requestedMajorVersion);
				printfResult = swprintf(libraryPath, libraryPathCapacity, L"%lsLibOVRRevive_1.dll", directoryArray[i]);

                /*if (*directoryArray[i] == 0)
                {
                    int k;
                    FilePathCharType foundPath[MAX_PATH] = { 0 };
                    DWORD searchResult = SearchPathW(NULL, libraryPath, NULL, MAX_PATH, foundPath, NULL);
                    if (searchResult <= 0 || searchResult >= libraryPathCapacity)
                    {
                        continue;
                    }
                    foundPath[MAX_PATH - 1] = 0;
                    for (k = 0; k < MAX_PATH; ++k)
                    {
                        libraryPath[k] = foundPath[k];
                    }
                }*/

            #elif defined(__APPLE__)
                // https://developer.apple.com/library/mac/documentation/MacOSX/Conceptual/BPFrameworks/Concepts/VersionInformation.html
                // Macintosh application bundles have the option of embedding dependent frameworks within the application
                // bundle itself. A problem with that is that it doesn't support vendor-supplied updates to the framework.
                printfResult = snprintf(libraryPath, libraryPathCapacity, "%sLibOVRRT.framework/Versions/%d/LibOVRRT", directoryArray[i], requestedMajorVersion);

            #else // Unix
                // Applications that depend on the OS (e.g. ld-linux / ldd) can rely on the library being in a common location
                // such as /usr/lib or can rely on the -rpath linker option to embed a path for the OS to check for the library,
                // or can rely on the LD_LIBRARY_PATH environment variable being set. It's generally not recommended that applications
                // depend on LD_LIBRARY_PATH be globally modified, partly due to potentialy security issues.
                // Currently we check the current application directory, current working directory, and then /usr/lib and possibly others.
                printfResult = snprintf(libraryPath, libraryPathCapacity, "%slibOVRRT%s.so.%d", directoryArray[i], pBitDepth, requestedMajorVersion);
            #endif

            if((printfResult >= 0) && (printfResult < (int)libraryPathCapacity))
            {
                moduleHandle = REV_OpenLibrary(libraryPath);
                if(moduleHandle != ModuleHandleTypeNull)
                    return moduleHandle;
            }
        }
    }

    return moduleHandle;
}



//-----------------------------------------------------------------------------------
// ***** hLibOVR
//
// global handle to the LivOVR shared library.
//
static ModuleHandleType hLibOVR = NULL;

// This function is currently unsupported.
ModuleHandleType rev_GetLibOVRRTHandle()
{
    return hLibOVR;
}



//-----------------------------------------------------------------------------------
// ***** Function declarations
//
// To consider: Move REV_DECLARE_IMPORT and the declarations below to REV_CAPI.h
//
REV_DECLARE_IMPORT(revBool,          ovr_InitializeRenderingShimVersion, (int requestedMinorVersion))
REV_DECLARE_IMPORT(revResult,        ovr_Initialize, (const revInitParams* params))
REV_DECLARE_IMPORT(revBool,          ovr_Shutdown, ())
REV_DECLARE_IMPORT(const char*,      ovr_GetVersionString, ())
REV_DECLARE_IMPORT(void,             ovr_GetLastErrorInfo, (revErrorInfo* errorInfo))
REV_DECLARE_IMPORT(revHmdDesc,       ovr_GetHmdDesc, (revSession session))
REV_DECLARE_IMPORT(unsigned int,     ovr_GetTrackerCount, (revSession session))
REV_DECLARE_IMPORT(revTrackerDesc,   ovr_GetTrackerDesc, (revSession session, unsigned int trackerDescIndex))
REV_DECLARE_IMPORT(revResult,        ovr_Create, (revSession* pSession, revGraphicsLuid* pLuid))
REV_DECLARE_IMPORT(void,             ovr_Destroy, (revSession session))
REV_DECLARE_IMPORT(revResult,        ovr_GetSessionStatus, (revSession session, revSessionStatus* sessionStatus))
REV_DECLARE_IMPORT(revResult,         ovr_SetTrackingOriginType, (revSession session, revTrackingOrigin origin))
REV_DECLARE_IMPORT(revTrackingOrigin, ovr_GetTrackingOriginType, (revSession session))

REV_DECLARE_IMPORT(revResult,        ovr_RecenterTrackingOrigin, (revSession session))
REV_DECLARE_IMPORT(void,             ovr_ClearShouldRecenterFlag, (revSession session))
REV_DECLARE_IMPORT(revTrackingState, ovr_GetTrackingState, (revSession session, double absTime, revBool latencyMarker))
REV_DECLARE_IMPORT(revTrackerPose,   ovr_GetTrackerPose, (revSession session, unsigned int index))
REV_DECLARE_IMPORT(revResult,        ovr_GetInputState, (revSession session, revControllerType controllerType, revInputState*))
REV_DECLARE_IMPORT(unsigned int,     ovr_GetConnectedControllerTypes, (revSession session))
REV_DECLARE_IMPORT(revResult,        ovr_SetControllerVibration, (revSession session, revControllerType controllerType, float frequency, float amplitude))
REV_DECLARE_IMPORT(revSizei,         ovr_GetFovTextureSize, (revSession session, revEyeType eye, revFovPort fov, float pixelsPerDisplayPixel))
REV_DECLARE_IMPORT(revResult,        ovr_SubmitFrame, (revSession session, long long frameIndex, const revViewScaleDesc* viewScaleDesc, revLayerHeader const * const * layerPtrList, unsigned int layerCount))
REV_DECLARE_IMPORT(revEyeRenderDesc, ovr_GetRenderDesc, (revSession session, revEyeType eyeType, revFovPort fov))
REV_DECLARE_IMPORT(double,           ovr_GetPredictedDisplayTime, (revSession session, long long frameIndex))
REV_DECLARE_IMPORT(double,           ovr_GetTimeInSeconds, ())
REV_DECLARE_IMPORT(revBool,          ovr_GetBool, (revSession session, const char* propertyName, revBool defaultVal))
REV_DECLARE_IMPORT(revBool,          ovr_SetBool, (revSession session, const char* propertyName, revBool value))
REV_DECLARE_IMPORT(int,              ovr_GetInt, (revSession session, const char* propertyName, int defaultVal))
REV_DECLARE_IMPORT(revBool,          ovr_SetInt, (revSession session, const char* propertyName, int value))
REV_DECLARE_IMPORT(float,            ovr_GetFloat, (revSession session, const char* propertyName, float defaultVal))
REV_DECLARE_IMPORT(revBool,          ovr_SetFloat, (revSession session, const char* propertyName, float value))
REV_DECLARE_IMPORT(unsigned int,     ovr_GetFloatArray, (revSession session, const char* propertyName, float values[], unsigned int arraySize))
REV_DECLARE_IMPORT(revBool,          ovr_SetFloatArray, (revSession session, const char* propertyName, const float values[], unsigned int arraySize))
REV_DECLARE_IMPORT(const char*,      ovr_GetString, (revSession session, const char* propertyName, const char* defaultVal))
REV_DECLARE_IMPORT(revBool,          ovr_SetString, (revSession session, const char* propertyName, const char* value))
REV_DECLARE_IMPORT(int,              ovr_TraceMessage, (int level, const char* message))

#if defined (_WIN32)
REV_DECLARE_IMPORT(revResult, ovr_CreateTextureSwapChainDX, (revSession session, IUnknown* d3dPtr, const revTextureSwapChainDesc* desc, revTextureSwapChain* outTextureChain))
REV_DECLARE_IMPORT(revResult, ovr_CreateMirrorTextureDX, (revSession session, IUnknown* d3dPtr, const revMirrorTextureDesc* desc, revMirrorTexture* outMirrorTexture))
REV_DECLARE_IMPORT(revResult, ovr_GetTextureSwapChainBufferDX, (revSession session, revTextureSwapChain chain, int index, IID iid, void** ppObject))
REV_DECLARE_IMPORT(revResult, ovr_GetMirrorTextureBufferDX, (revSession session, revMirrorTexture mirror, IID iid, void** ppObject))
REV_DECLARE_IMPORT(revResult, ovr_GetAudioDeviceOutWaveId, (UINT* deviceOutId))
REV_DECLARE_IMPORT(revResult, ovr_GetAudioDeviceInWaveId, (UINT* deviceInId))
REV_DECLARE_IMPORT(revResult, ovr_GetAudioDeviceOutGuidStr, (WCHAR* deviceOutStrBuffer))
REV_DECLARE_IMPORT(revResult, ovr_GetAudioDeviceOutGuid, (GUID* deviceOutGuid))
REV_DECLARE_IMPORT(revResult, ovr_GetAudioDeviceInGuidStr, (WCHAR* deviceInStrBuffer))
REV_DECLARE_IMPORT(revResult, ovr_GetAudioDeviceInGuid, (GUID* deviceInGuid))
#endif

REV_DECLARE_IMPORT(revResult, ovr_CreateTextureSwapChainGL, (revSession session, const revTextureSwapChainDesc* desc, revTextureSwapChain* outTextureChain))
REV_DECLARE_IMPORT(revResult, ovr_CreateMirrorTextureGL, (revSession session, const revMirrorTextureDesc* desc, revMirrorTexture* outMirrorTexture))
REV_DECLARE_IMPORT(revResult, ovr_GetTextureSwapChainBufferGL, (revSession session, revTextureSwapChain chain, int index, unsigned int* texId))
REV_DECLARE_IMPORT(revResult, ovr_GetMirrorTextureBufferGL, (revSession session, revMirrorTexture mirror, unsigned int* texId))

REV_DECLARE_IMPORT(revResult, ovr_GetTextureSwapChainLength, (revSession session, revTextureSwapChain chain, int* length))
REV_DECLARE_IMPORT(revResult, ovr_GetTextureSwapChainCurrentIndex, (revSession session, revTextureSwapChain chain, int* currentIndex))
REV_DECLARE_IMPORT(revResult, ovr_GetTextureSwapChainDesc, (revSession session, revTextureSwapChain chain, revTextureSwapChainDesc* desc))
REV_DECLARE_IMPORT(revResult, ovr_CommitTextureSwapChain, (revSession session, revTextureSwapChain chain))
REV_DECLARE_IMPORT(void, ovr_DestroyTextureSwapChain, (revSession session, revTextureSwapChain chain))
REV_DECLARE_IMPORT(void, ovr_DestroyMirrorTexture, (revSession session, revMirrorTexture texture))
REV_DECLARE_IMPORT(revResult, ovr_SetQueueAheadFraction, (revSession session, float queueAheadFraction))

REV_DECLARE_IMPORT(revResult, ovr_Lookup, (const char* name, void** data));

static revResult REV_LoadSharedLibrary(int requestedProductVersion, int requestedMajorVersion)
{
    FilePathCharType filePath[REV_MAX_PATH];

    if(hLibOVR)
        return revSuccess;

    hLibOVR = REV_FindLibraryPath(requestedProductVersion, requestedMajorVersion,
                             filePath, sizeof(filePath) / sizeof(filePath[0]));
    if(!hLibOVR)
        return revError_LibLoad;

    REV_GETFUNCTION(ovr_InitializeRenderingShimVersion)
    REV_GETFUNCTION(ovr_Initialize)
    REV_GETFUNCTION(ovr_Shutdown)
    REV_GETFUNCTION(ovr_GetVersionString)
    REV_GETFUNCTION(ovr_GetLastErrorInfo)
    REV_GETFUNCTION(ovr_GetHmdDesc)
    REV_GETFUNCTION(ovr_GetTrackerCount)
    REV_GETFUNCTION(ovr_GetTrackerDesc)
    REV_GETFUNCTION(ovr_Create)
    REV_GETFUNCTION(ovr_Destroy)
    REV_GETFUNCTION(ovr_GetSessionStatus)
    REV_GETFUNCTION(ovr_SetTrackingOriginType)
    REV_GETFUNCTION(ovr_GetTrackingOriginType)
    REV_GETFUNCTION(ovr_RecenterTrackingOrigin)
    REV_GETFUNCTION(ovr_ClearShouldRecenterFlag)
    REV_GETFUNCTION(ovr_GetTrackingState)
    REV_GETFUNCTION(ovr_GetTrackerPose)
    REV_GETFUNCTION(ovr_GetInputState)
    REV_GETFUNCTION(ovr_GetConnectedControllerTypes)
    REV_GETFUNCTION(ovr_SetControllerVibration)
    REV_GETFUNCTION(ovr_GetFovTextureSize)
    REV_GETFUNCTION(ovr_SubmitFrame)
    REV_GETFUNCTION(ovr_GetRenderDesc)
    REV_GETFUNCTION(ovr_GetPredictedDisplayTime)
    REV_GETFUNCTION(ovr_GetTimeInSeconds)
    REV_GETFUNCTION(ovr_GetBool)
    REV_GETFUNCTION(ovr_SetBool)
    REV_GETFUNCTION(ovr_GetInt)
    REV_GETFUNCTION(ovr_SetInt)
    REV_GETFUNCTION(ovr_GetFloat)
    REV_GETFUNCTION(ovr_SetFloat)
    REV_GETFUNCTION(ovr_GetFloatArray)
    REV_GETFUNCTION(ovr_SetFloatArray)
    REV_GETFUNCTION(ovr_GetString)
    REV_GETFUNCTION(ovr_SetString)
    REV_GETFUNCTION(ovr_TraceMessage)
#if defined (_WIN32)
    REV_GETFUNCTION(ovr_CreateTextureSwapChainDX)
    REV_GETFUNCTION(ovr_CreateMirrorTextureDX)
    REV_GETFUNCTION(ovr_GetTextureSwapChainBufferDX)
    REV_GETFUNCTION(ovr_GetMirrorTextureBufferDX)
    REV_GETFUNCTION(ovr_GetAudioDeviceOutWaveId)
    REV_GETFUNCTION(ovr_GetAudioDeviceInWaveId)
    REV_GETFUNCTION(ovr_GetAudioDeviceOutGuidStr)
    REV_GETFUNCTION(ovr_GetAudioDeviceOutGuid)
    REV_GETFUNCTION(ovr_GetAudioDeviceInGuidStr)
    REV_GETFUNCTION(ovr_GetAudioDeviceInGuid)
#endif
    REV_GETFUNCTION(ovr_CreateTextureSwapChainGL)
    REV_GETFUNCTION(ovr_CreateMirrorTextureGL)
    REV_GETFUNCTION(ovr_GetTextureSwapChainBufferGL)
    REV_GETFUNCTION(ovr_GetMirrorTextureBufferGL)

    REV_GETFUNCTION(ovr_GetTextureSwapChainLength)
    REV_GETFUNCTION(ovr_GetTextureSwapChainCurrentIndex)
    REV_GETFUNCTION(ovr_GetTextureSwapChainDesc)
    REV_GETFUNCTION(ovr_CommitTextureSwapChain)
    REV_GETFUNCTION(ovr_DestroyTextureSwapChain)
    REV_GETFUNCTION(ovr_DestroyMirrorTexture)
    REV_GETFUNCTION(ovr_SetQueueAheadFraction)
    REV_GETFUNCTION(ovr_Lookup)

    return revSuccess;
}

static void REV_UnloadSharedLibrary()
{
    // To consider: Make all pointers be part of a struct and memset the struct to 0 here.
    ovr_InitializeRenderingShimVersionPtr = NULL;
    ovr_InitializePtr = NULL;
    ovr_ShutdownPtr = NULL;
    ovr_GetVersionStringPtr = NULL;
    ovr_GetLastErrorInfoPtr = NULL;
    ovr_GetHmdDescPtr = NULL;
    ovr_GetTrackerCountPtr = NULL;
    ovr_GetTrackerDescPtr = NULL;
    ovr_CreatePtr = NULL;
    ovr_DestroyPtr = NULL;
    ovr_GetSessionStatusPtr = NULL;
    ovr_SetTrackingOriginTypePtr = NULL;
    ovr_GetTrackingOriginTypePtr = NULL;
    ovr_RecenterTrackingOriginPtr = NULL;
    ovr_GetTrackingStatePtr = NULL;
    ovr_GetTrackerPosePtr = NULL;
    ovr_GetInputStatePtr = NULL;
    ovr_GetConnectedControllerTypesPtr = NULL;
    ovr_SetControllerVibrationPtr = NULL;
    ovr_GetFovTextureSizePtr = NULL;
    ovr_SubmitFramePtr = NULL;
    ovr_GetRenderDescPtr = NULL;
    ovr_GetPredictedDisplayTimePtr = NULL;
    ovr_GetTimeInSecondsPtr = NULL;
    ovr_GetBoolPtr = NULL;
    ovr_SetBoolPtr = NULL;
    ovr_GetIntPtr = NULL;
    ovr_SetIntPtr = NULL;
    ovr_GetFloatPtr = NULL;
    ovr_SetFloatPtr = NULL;
    ovr_GetFloatArrayPtr = NULL;
    ovr_SetFloatArrayPtr = NULL;
    ovr_GetStringPtr = NULL;
    ovr_SetStringPtr = NULL;
    ovr_TraceMessagePtr = NULL;
#if defined (_WIN32)
    ovr_CreateTextureSwapChainDXPtr = NULL;
    ovr_CreateMirrorTextureDXPtr = NULL;
    ovr_GetTextureSwapChainBufferDXPtr = NULL;
    ovr_GetMirrorTextureBufferDXPtr = NULL;
    ovr_GetAudioDeviceInWaveIdPtr = NULL;
    ovr_GetAudioDeviceOutWaveIdPtr = NULL;
    ovr_GetAudioDeviceInGuidStrPtr = NULL;
    ovr_GetAudioDeviceOutGuidStrPtr = NULL;
    ovr_GetAudioDeviceInGuidPtr = NULL;
    ovr_GetAudioDeviceOutGuidPtr = NULL;
#endif
    ovr_CreateTextureSwapChainGLPtr = NULL;
    ovr_CreateMirrorTextureGLPtr = NULL;
    ovr_GetTextureSwapChainBufferGLPtr = NULL;
    ovr_GetMirrorTextureBufferGLPtr = NULL;

    ovr_GetTextureSwapChainLengthPtr = NULL;
    ovr_GetTextureSwapChainCurrentIndexPtr = NULL;
    ovr_GetTextureSwapChainDescPtr = NULL;
    ovr_CommitTextureSwapChainPtr = NULL;
    ovr_DestroyTextureSwapChainPtr = NULL;
    ovr_DestroyMirrorTexturePtr = NULL;
    ovr_SetQueueAheadFractionPtr = NULL;
    ovr_LookupPtr = NULL;

    REV_CloseLibrary(hLibOVR);
    hLibOVR = NULL;
}


REV_PUBLIC_FUNCTION(revBool) rev_InitializeRenderingShim()
{
#if 1
    return revTrue;
#else
    return rev_InitializeRenderingShimVersion(REV_MINOR_VERSION_);
#endif
}


REV_PUBLIC_FUNCTION(revBool) rev_InitializeRenderingShimVersion(int requestedMinorVersion)
{
    // By design we ignore the build version in the library search.
    revBool initializeResult;
    revResult result = REV_LoadSharedLibrary(REV_PRODUCT_VERSION_, REV_MAJOR_VERSION_);

    if (result != revSuccess)
        return revFalse;

    initializeResult = ovr_InitializeRenderingShimVersionPtr(requestedMinorVersion);

    if (initializeResult == revFalse)
        REV_UnloadSharedLibrary();

    return initializeResult;
}


// These defaults are also in CAPI.cpp
static const revInitParams DefaultParams = {
    revInit_RequestVersion, // Flags
    REV_MINOR_VERSION_,      // RequestedMinorVersion
    0,                      // LogCallback
    0,                      // UserData
    0,                      // ConnectionTimeoutSeconds
    REV_ON64("")            // pad0
};

REV_PUBLIC_FUNCTION(revResult) rev_Initialize(const revInitParams* inputParams)
{
    revResult result;
    revInitParams params;

    typedef void (REV_CDECL *rev_ReportClientInfoType)(
        unsigned int compilerVersion,
        int productVersion, int majorVersion, int minorVersion,
        int patchVersion, int buildNumber);
    rev_ReportClientInfoType reportClientInfo;

    // Do something with our version signature hash to prevent
    // it from being optimized out. In this case, compute
    // a cheap CRC.
    uint8_t crc = 0;
    size_t i;

    for (i = 0; i < (sizeof(OculusSDKUniqueIdentifier) - 3); ++i) // Minus 3 because we have trailing REV_MAJOR_VERSION, REV_MINOR_VERSION, REV_PATCH_VERSION which vary per version.
    {
        crc ^= OculusSDKUniqueIdentifier[i];
    }

    assert(crc == OculusSDKUniqueIdentifierXORResult);
    if (crc != OculusSDKUniqueIdentifierXORResult)
    {
        return revError_Initialize;
    }

    if (!inputParams)
    {
        params = DefaultParams;
    }
    else
    {
        params = *inputParams;

        // If not requesting a particular minor version,
        if (!(params.Flags & revInit_RequestVersion))
        {
            // Enable requesting the default minor version.
            params.Flags |= revInit_RequestVersion;
            params.RequestedMinorVersion = REV_MINOR_VERSION_;
        }
    }

    // Clear non-writable bits provided by client code.
    params.Flags &= revInit_WritableBits;



    // By design we ignore the build version in the library search.
    result = REV_LoadSharedLibrary(REV_PRODUCT_VERSION_, REV_MAJOR_VERSION_);
    if (result != revSuccess)
       return result;

    result = ovr_InitializePtr(&params);
    if (result != revSuccess)
        REV_UnloadSharedLibrary();

    reportClientInfo = (rev_ReportClientInfoType)(uintptr_t)REV_DLSYM(hLibOVR, "rev_ReportClientInfo");

    if (reportClientInfo)
    {
        unsigned int mscFullVer = 0;
#if defined (_MSC_FULL_VER)
        mscFullVer = _MSC_FULL_VER;
#endif // _MSC_FULL_VER

        reportClientInfo(mscFullVer, REV_PRODUCT_VERSION_, REV_MAJOR_VERSION_,
            REV_MINOR_VERSION_, REV_PATCH_VERSION_, REV_BUILD_NUMBER_);
    }

    return result;
}

REV_PUBLIC_FUNCTION(void) rev_Shutdown()
{
    if (!ovr_ShutdownPtr)
        return;
    ovr_ShutdownPtr();
    REV_UnloadSharedLibrary();
}

REV_PUBLIC_FUNCTION(const char*) rev_GetVersionString()
{
    // We don't directly return the value of the DLL rev_GetVersionStringPtr call,
    // because that call returns a pointer to memory within the DLL. If the DLL goes 
    // away then that pointer becomes invalid while the process may still be holding
    // onto it. So we save a local copy of it which is always valid.
    static char dllVersionStringLocal[32];
    const char* dllVersionString;

    if (!ovr_GetVersionStringPtr)
        return "(Unable to load LibOVR)";

    dllVersionString = ovr_GetVersionStringPtr(); // Guaranteed to always be valid.
    assert(dllVersionString != NULL);
    REV_strlcpy(dllVersionStringLocal, dllVersionString, sizeof(dllVersionStringLocal));

    return dllVersionStringLocal;
}

REV_PUBLIC_FUNCTION(void) rev_GetLastErrorInfo(revErrorInfo* errorInfo)
{
	if (!ovr_GetLastErrorInfoPtr)
    {
        memset(errorInfo, 0, sizeof(revErrorInfo));
        errorInfo->Result = revError_LibLoad;
    }
    else
        ovr_GetLastErrorInfoPtr(errorInfo);
}

REV_PUBLIC_FUNCTION(revHmdDesc) rev_GetHmdDesc(revSession session)
{
    if (!ovr_GetHmdDescPtr)
    {
        revHmdDesc hmdDesc;
        memset(&hmdDesc, 0, sizeof(hmdDesc));
        hmdDesc.Type = revHmd_None;
        return hmdDesc;
    }

    return ovr_GetHmdDescPtr(session);
}

REV_PUBLIC_FUNCTION(unsigned int) rev_GetTrackerCount(revSession session)
{
    if (!ovr_GetTrackerCountPtr)
    {
        return 0;
    }
    
    return ovr_GetTrackerCountPtr(session);
}

REV_PUBLIC_FUNCTION(revTrackerDesc) rev_GetTrackerDesc(revSession session, unsigned int trackerDescIndex)
{
    if (!ovr_GetTrackerDescPtr)
    {
        revTrackerDesc trackerDesc;
        memset(&trackerDesc, 0, sizeof(trackerDesc));
        return trackerDesc;
    }
    
    return ovr_GetTrackerDescPtr(session, trackerDescIndex);
}

REV_PUBLIC_FUNCTION(revResult) rev_Create(revSession* pSession, revGraphicsLuid* pLuid)
{
    if (!ovr_CreatePtr)
        return revError_NotInitialized;
    return ovr_CreatePtr(pSession, pLuid);
}

REV_PUBLIC_FUNCTION(void) rev_Destroy(revSession session)
{
    if (!ovr_DestroyPtr)
        return;
    ovr_DestroyPtr(session);
}

REV_PUBLIC_FUNCTION(revResult) rev_GetSessionStatus(revSession session, revSessionStatus* sessionStatus)
{
    if (!ovr_GetSessionStatusPtr)
    {
        if (sessionStatus)
        {
            sessionStatus->IsVisible   = revFalse;
            sessionStatus->HmdPresent  = revFalse;
            sessionStatus->HmdMounted  = revFalse;
            sessionStatus->ShouldQuit  = revFalse;
            sessionStatus->DisplayLost = revFalse;
            sessionStatus->ShouldRecenter = revFalse;
        }

        return revError_NotInitialized;
    }

    return ovr_GetSessionStatusPtr(session, sessionStatus);
}


REV_PUBLIC_FUNCTION(revResult) rev_SetTrackingOriginType(revSession session, revTrackingOrigin origin)
{
    if (!ovr_SetTrackingOriginTypePtr)
        return revError_NotInitialized;
    return ovr_SetTrackingOriginTypePtr(session, origin);
}

REV_PUBLIC_FUNCTION(revTrackingOrigin) rev_GetTrackingOriginType(revSession session)
{
    if (!ovr_GetTrackingOriginTypePtr)
        return revTrackingOrigin_EyeLevel;
    return ovr_GetTrackingOriginTypePtr(session);
}

REV_PUBLIC_FUNCTION(revResult) rev_RecenterTrackingOrigin(revSession session)
{
    if (!ovr_RecenterTrackingOriginPtr)
        return revError_NotInitialized;
    return ovr_RecenterTrackingOriginPtr(session);
}

REV_PUBLIC_FUNCTION(void) rev_ClearShouldRecenterFlag(revSession session)
{
    if (!ovr_ClearShouldRecenterFlagPtr)
        return;
    ovr_ClearShouldRecenterFlagPtr(session);
}

REV_PUBLIC_FUNCTION(revTrackingState) rev_GetTrackingState(revSession session, double absTime, revBool latencyMarker)
{
    if (!ovr_GetTrackingStatePtr)
    {
        revTrackingState nullTrackingState;
        memset(&nullTrackingState, 0, sizeof(nullTrackingState));
        return nullTrackingState;
    }

    return ovr_GetTrackingStatePtr(session, absTime, latencyMarker);
}


REV_PUBLIC_FUNCTION(revTrackerPose) rev_GetTrackerPose(revSession session, unsigned int trackerPoseIndex)
{
    if (!ovr_GetTrackerPosePtr)
    {
        revTrackerPose nullTrackerPose;
        memset(&nullTrackerPose, 0, sizeof(nullTrackerPose));
        return nullTrackerPose;
    }

    return ovr_GetTrackerPosePtr(session, trackerPoseIndex);
}


REV_PUBLIC_FUNCTION(revResult) rev_GetInputState(revSession session, revControllerType controllerType, revInputState* inputState)
{
    if (!ovr_GetInputStatePtr)
    {
        if (inputState)
            memset(inputState, 0, sizeof(revInputState));
        return revError_NotInitialized;
    }
    return ovr_GetInputStatePtr(session, controllerType, inputState);
}

REV_PUBLIC_FUNCTION(unsigned int) rev_GetConnectedControllerTypes(revSession session)
{
    if (!ovr_GetConnectedControllerTypesPtr)
    {
        return 0;
    }
    return ovr_GetConnectedControllerTypesPtr(session);
}

REV_PUBLIC_FUNCTION(revResult) rev_SetControllerVibration(revSession session, revControllerType controllerType, float frequency, float amplitude)
{
    if (!ovr_SetControllerVibrationPtr)
    {
        return revError_NotInitialized;
    }
    return ovr_SetControllerVibrationPtr(session, controllerType, frequency, amplitude);
}

REV_PUBLIC_FUNCTION(revSizei) rev_GetFovTextureSize(revSession session, revEyeType eye, revFovPort fov,
                                             float pixelsPerDisplayPixel)
{
    if (!ovr_GetFovTextureSizePtr)
    {
        revSizei nullSize;
        memset(&nullSize, 0, sizeof(nullSize));
        return nullSize;
    }

    return ovr_GetFovTextureSizePtr(session, eye, fov, pixelsPerDisplayPixel);
}

#if defined (_WIN32)
REV_PUBLIC_FUNCTION(revResult) rev_CreateTextureSwapChainDX(revSession session,
                                                            IUnknown* d3dPtr,
                                                            const revTextureSwapChainDesc* desc,
                                                            revTextureSwapChain* outTextureSet)
{
    if (!ovr_CreateTextureSwapChainDXPtr)
        return revError_NotInitialized;

    return ovr_CreateTextureSwapChainDXPtr(session, d3dPtr, desc, outTextureSet);
}

REV_PUBLIC_FUNCTION(revResult) rev_CreateMirrorTextureDX(revSession session,
                                                         IUnknown* d3dPtr,
                                                         const revMirrorTextureDesc* desc,
                                                         revMirrorTexture* outMirrorTexture)
{
    if (!ovr_CreateMirrorTextureDXPtr)
        return revError_NotInitialized;

    return ovr_CreateMirrorTextureDXPtr(session, d3dPtr, desc, outMirrorTexture);
}

REV_PUBLIC_FUNCTION(revResult) rev_GetTextureSwapChainBufferDX(revSession session,
                                                               revTextureSwapChain chain,
                                                               int index,
                                                               IID iid,
                                                               void** ppObject)
{
    if (!ovr_GetTextureSwapChainBufferDXPtr)
        return revError_NotInitialized;

    return ovr_GetTextureSwapChainBufferDXPtr(session, chain, index, iid, ppObject);
}

REV_PUBLIC_FUNCTION(revResult) rev_GetMirrorTextureBufferDX(revSession session,
                                                            revMirrorTexture mirror,
                                                            IID iid,
                                                            void** ppObject)
{
    if (!ovr_GetMirrorTextureBufferDXPtr)
        return revError_NotInitialized;

    return ovr_GetMirrorTextureBufferDXPtr(session, mirror, iid, ppObject);
}

REV_PUBLIC_FUNCTION(revResult) rev_GetAudioDeviceOutWaveId(unsigned int* deviceOutId)
{
    if (!ovr_GetAudioDeviceOutWaveIdPtr)
        return revError_NotInitialized;

    return ovr_GetAudioDeviceOutWaveIdPtr(deviceOutId);
}

REV_PUBLIC_FUNCTION(revResult) rev_GetAudioDeviceInWaveId(unsigned int* deviceInId)
{
    if (!ovr_GetAudioDeviceInWaveIdPtr)
        return revError_NotInitialized;

    return ovr_GetAudioDeviceInWaveIdPtr(deviceInId);
}

REV_PUBLIC_FUNCTION(revResult) rev_GetAudioDeviceOutGuidStr(WCHAR* deviceOutStrBuffer)
{
    if (!ovr_GetAudioDeviceOutGuidStrPtr)
        return revError_NotInitialized;

    return ovr_GetAudioDeviceOutGuidStrPtr(deviceOutStrBuffer);
}

REV_PUBLIC_FUNCTION(revResult) rev_GetAudioDeviceOutGuid(GUID* deviceOutGuid)
{
    if (!ovr_GetAudioDeviceOutGuidPtr)
        return revError_NotInitialized;

    return ovr_GetAudioDeviceOutGuidPtr(deviceOutGuid);
}

REV_PUBLIC_FUNCTION(revResult) rev_GetAudioDeviceInGuidStr(WCHAR* deviceInStrBuffer)
{
    if (!ovr_GetAudioDeviceInGuidStrPtr)
        return revError_NotInitialized;

    return ovr_GetAudioDeviceInGuidStrPtr(deviceInStrBuffer);
}

REV_PUBLIC_FUNCTION(revResult) rev_GetAudioDeviceInGuid(GUID* deviceInGuid)
{
    if (!ovr_GetAudioDeviceInGuidPtr)
        return revError_NotInitialized;

    return ovr_GetAudioDeviceInGuidPtr(deviceInGuid);
}

#endif

REV_PUBLIC_FUNCTION(revResult) rev_CreateTextureSwapChainGL(revSession session,
                                                            const revTextureSwapChainDesc* desc,
                                                            revTextureSwapChain* outTextureSet)
{
    if (!ovr_CreateTextureSwapChainGLPtr)
        return revError_NotInitialized;

    return ovr_CreateTextureSwapChainGLPtr(session, desc, outTextureSet);
}

REV_PUBLIC_FUNCTION(revResult) rev_CreateMirrorTextureGL(revSession session,
                                                         const revMirrorTextureDesc* desc,
                                                         revMirrorTexture* outMirrorTexture)
{
    if (!ovr_CreateMirrorTextureGLPtr)
        return revError_NotInitialized;

    return ovr_CreateMirrorTextureGLPtr(session, desc, outMirrorTexture);
}

REV_PUBLIC_FUNCTION(revResult) rev_GetTextureSwapChainBufferGL(revSession session,
                                                               revTextureSwapChain chain,
                                                               int index,
                                                               unsigned int* texId)
{
    if (!ovr_GetTextureSwapChainBufferGLPtr)
        return revError_NotInitialized;

    return ovr_GetTextureSwapChainBufferGLPtr(session, chain, index, texId);
}

REV_PUBLIC_FUNCTION(revResult) rev_GetMirrorTextureBufferGL(revSession session,
                                                            revMirrorTexture mirror,
                                                            unsigned int* texId)
{
    if (!ovr_GetMirrorTextureBufferGLPtr)
        return revError_NotInitialized;

    return ovr_GetMirrorTextureBufferGLPtr(session, mirror, texId);
}

REV_PUBLIC_FUNCTION(revResult) rev_GetTextureSwapChainLength(revSession session,
                                                             revTextureSwapChain chain,
                                                             int* length)
{
    if (!ovr_GetTextureSwapChainLengthPtr)
        return revError_NotInitialized;

    return ovr_GetTextureSwapChainLengthPtr(session, chain, length);
}

REV_PUBLIC_FUNCTION(revResult) rev_GetTextureSwapChainCurrentIndex(revSession session,
                                                                   revTextureSwapChain chain,
                                                                   int* currentIndex)
{
    if (!ovr_GetTextureSwapChainCurrentIndexPtr)
        return revError_NotInitialized;

    return ovr_GetTextureSwapChainCurrentIndexPtr(session, chain, currentIndex);
}

REV_PUBLIC_FUNCTION(revResult) rev_GetTextureSwapChainDesc(revSession session,
                                                           revTextureSwapChain chain,
                                                           revTextureSwapChainDesc* desc)
{
    if (!ovr_GetTextureSwapChainDescPtr)
        return revError_NotInitialized;

    return ovr_GetTextureSwapChainDescPtr(session, chain, desc);
}

REV_PUBLIC_FUNCTION(revResult) rev_CommitTextureSwapChain(revSession session,
                                                          revTextureSwapChain chain)
{
    if (!ovr_CommitTextureSwapChainPtr)
        return revError_NotInitialized;

    return ovr_CommitTextureSwapChainPtr(session, chain);
}

REV_PUBLIC_FUNCTION(void) rev_DestroyTextureSwapChain(revSession session, revTextureSwapChain chain)
{
    if (!ovr_DestroyTextureSwapChainPtr)
        return;

    ovr_DestroyTextureSwapChainPtr(session, chain);
}

REV_PUBLIC_FUNCTION(void) rev_DestroyMirrorTexture(revSession session, revMirrorTexture mirrorTexture)
{
    if (!ovr_DestroyMirrorTexturePtr)
        return;

    ovr_DestroyMirrorTexturePtr(session, mirrorTexture);
}

REV_PUBLIC_FUNCTION(revResult) rev_SetQueueAheadFraction(revSession session, float queueAheadFraction)
{
    if (!ovr_SetQueueAheadFractionPtr)
        return revError_NotInitialized;

    return ovr_SetQueueAheadFractionPtr(session, queueAheadFraction);
}

REV_PUBLIC_FUNCTION(revResult) rev_SubmitFrame(revSession session, long long frameIndex, const revViewScaleDesc* viewScaleDesc, revLayerHeader const * const * layerPtrList, unsigned int layerCount)
{
    if (!ovr_SubmitFramePtr)
        return revError_NotInitialized;

    return ovr_SubmitFramePtr(session, frameIndex, viewScaleDesc, layerPtrList, layerCount);
}

REV_PUBLIC_FUNCTION(revEyeRenderDesc) rev_GetRenderDesc(revSession session, revEyeType eyeType, revFovPort fov)
{
    if (!ovr_GetRenderDescPtr)
    {
        revEyeRenderDesc nullEyeRenderDesc;
        memset(&nullEyeRenderDesc, 0, sizeof(nullEyeRenderDesc));
        return nullEyeRenderDesc;
    }
    return ovr_GetRenderDescPtr(session, eyeType, fov);
}

REV_PUBLIC_FUNCTION(double) rev_GetPredictedDisplayTime(revSession session, long long frameIndex)
{
    if (!ovr_GetPredictedDisplayTimePtr)
        return 0.0;

    return ovr_GetPredictedDisplayTimePtr(session, frameIndex);
}

REV_PUBLIC_FUNCTION(double) rev_GetTimeInSeconds()
{
    if (!ovr_GetTimeInSecondsPtr)
        return 0.;
    return ovr_GetTimeInSecondsPtr();
}

REV_PUBLIC_FUNCTION(revBool) rev_GetBool(revSession session, const char* propertyName, revBool defaultVal)
{
    if (!ovr_GetBoolPtr)
        return revFalse;
    return ovr_GetBoolPtr(session, propertyName, defaultVal);
}

REV_PUBLIC_FUNCTION(revBool) rev_SetBool(revSession session, const char* propertyName, revBool value)
{
    if (!ovr_SetBoolPtr)
        return revFalse;
    return ovr_SetBoolPtr(session, propertyName, value);
}

REV_PUBLIC_FUNCTION(int) rev_GetInt(revSession session, const char* propertyName, int defaultVal)
{
    if (!ovr_GetIntPtr)
        return 0;
    return ovr_GetIntPtr(session, propertyName, defaultVal);
}

REV_PUBLIC_FUNCTION(revBool) rev_SetInt(revSession session, const char* propertyName, int value)
{
    if (!ovr_SetIntPtr)
        return revFalse;
    return ovr_SetIntPtr(session, propertyName, value);
}

REV_PUBLIC_FUNCTION(float) rev_GetFloat(revSession session, const char* propertyName, float defaultVal)
{
    if (!ovr_GetFloatPtr)
        return 0.f;
    return ovr_GetFloatPtr(session, propertyName, defaultVal);
}

REV_PUBLIC_FUNCTION(revBool) rev_SetFloat(revSession session, const char* propertyName, float value)
{
    if (!ovr_SetFloatPtr)
        return revFalse;
    return ovr_SetFloatPtr(session, propertyName, value);
}

REV_PUBLIC_FUNCTION(unsigned int) rev_GetFloatArray(revSession session, const char* propertyName,
                                            float values[], unsigned int arraySize)
{
    if (!ovr_GetFloatArrayPtr)
        return 0;
    return ovr_GetFloatArrayPtr(session, propertyName, values, arraySize);
}

REV_PUBLIC_FUNCTION(revBool) rev_SetFloatArray(revSession session, const char* propertyName,
                                             const float values[], unsigned int arraySize)
{
    if (!ovr_SetFloatArrayPtr)
        return revFalse;
    return ovr_SetFloatArrayPtr(session, propertyName, values, arraySize);
}

REV_PUBLIC_FUNCTION(const char*) rev_GetString(revSession session, const char* propertyName,
                                        const char* defaultVal)
{
    if (!ovr_GetStringPtr)
        return "(Unable to load LibOVR)";
    return ovr_GetStringPtr(session, propertyName, defaultVal);
}

REV_PUBLIC_FUNCTION(revBool) rev_SetString(revSession session, const char* propertyName,
                                    const char* value)
{
    if (!ovr_SetStringPtr)
        return revFalse;
    return ovr_SetStringPtr(session, propertyName, value);
}

REV_PUBLIC_FUNCTION(int) rev_TraceMessage(int level, const char* message)
{
    if (!ovr_TraceMessagePtr)
        return -1;

    return ovr_TraceMessagePtr(level, message);
}

REV_PUBLIC_FUNCTION(revResult) rev_Lookup(const char* name, void** data)
{
    if (!ovr_LookupPtr)
        return revError_NotInitialized;
    return ovr_LookupPtr(name, data);
}

#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

