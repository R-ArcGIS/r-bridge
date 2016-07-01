// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#if !defined(DESKTOP10) && !defined(_WIN64)
#error "There is no Win32 (32bit) version of rarcproxy_pro.dll. Build x64 version"
#endif

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define VC_EXTRALEAN
#define NOMCX
// Windows Header Files:
#include <windows.h>

#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS // some CString constructors will be explicit

#include <crtdbg.h>

#include <atlbase.h>
//#include <atlcom.h>
//#include <atlctl.h>
#include <comutil.h>


#undef min
#undef max
#undef OUT
#undef length
#undef ERROR

#define R_NO_REMAP
#include <R.h>
#include <Rversion.h>
#include <Rdefines.h>
//#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#define RVERSION_DLL_BUILD R_MAJOR "." R_MINOR

#include "..\arcobjects_api.h"

using namespace ATL;
bool isCancel();

#define asSTR(s) #s
#define toSTR(s) asSTR(s)
#define DLL_NAME_STR toSTR(DLL_NAME)
#define LIBRARY_API_DLL_NAME toSTR(LIBRARY_API_NAME)

