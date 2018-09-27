// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// reference additional headers your program requires here
#include "../LibREV/Include/REV_CAPI.h"
#include "../LibREV/Include/REV_Version.h"
#include "../LibREV/Include/REV_ErrorCode.h"

#include "../LibREV/Include/REV_CAPI_D3D.h"   
#include "../LibREV/Include/REV_CAPI_GL.h"  