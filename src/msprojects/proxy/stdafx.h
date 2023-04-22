// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#define COM_NO_WINDOWS_H

#if !defined(DESKTOP10) && !defined(_WIN64)
#error ArcGISPro 64bit only"
#endif


#define asSTR(s) #s
#define toSTR(s) asSTR(s)
#define DLL_NAME_STR toSTR(DLL_NAME)
#define LIBRARY_API_DLL_NAME toSTR(LIBRARY_API_NAME)
#define R_PATH_SRT toSTR(R_PATH)

#if defined(DESKTOP10)
#pragma message("##### " toSTR(ProxyName) " for ArcGIS Desktop")
#else
#pragma message("##### " toSTR(ProxyName) " for ArcGIS Pro")
#endif

#pragma message("  R-HOME =" R_PATH_SRT)
#pragma message("  LIBRARY_API_DLL_NAME =" LIBRARY_API_DLL_NAME)
#pragma message("  DLL_NAME_STR =" DLL_NAME_STR)
#pragma message("  VCToolsVersion = " toSTR(VCToolsVersion))

#pragma message("##### ")

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define VC_EXTRALEAN
#define NOMCX
// Windows Header Files:
//#include <windows.h>
//#include <windef.h>


#include <combaseapi.h>
#include <propapi.h>

//#include <crtdbg.h>
//#include <ole2.h>
#include <oleauto.h>
//#include <oaidl.h>

//#define _ATL_NO_AUTOMATIC_NAMESPACE
//#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS // some CString constructors will be explicit
//#include <atlbase.h>
//#include <atlcom.h>
//#include <atlctl.h>
//#include <comutil.h>


#ifndef ATLASSERT
#include <assert.h>
#define ATLASSERT(x) assert(x)
#endif
#ifndef ATLTRACE
#define ATLTRACE(a, b) ((void)0)
#endif

#undef min
#undef max
#undef OUT
#undef length
#undef ERROR

#define R_NO_REMAP
#define STRICT_R_HEADERS
#include <R.h>
#include <Rversion.h>
#include <Rdefines.h>
//#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#define RVERSION_DLL_BUILD R_MAJOR "." R_MINOR

#include "../libobjects_api.h"
extern const arcobject::API* _api;
//typedef const arcobject::API* (*fn_api)(bool);
using fn_api = const arcobject::API* (*)(bool);

//using namespace ATL;
bool isCancel();
