// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define VC_EXTRALEAN
#define NOMCX
// Windows Header Files:
#include <windows.h>

#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS // some CString constructors will be explicit

#define SearchCommand MSSearchCommand
#define IProgressDialog IMSProgressDialog
#define ISegment IMSSegment

#include <crtdbg.h>

#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include <comutil.h>

#undef SearchCommand
#undef IProgressDialog
#undef ISegment

#undef min
#undef max

#pragma warning (push)
#pragma warning (disable: 4192)
#import <esriSystem.olb> named_guids no_namespace no_implementation raw_interfaces_only   exclude("UINT_PTR", "OLE_HANDLE", "OLE_COLOR", "XMLSerializer") \
  rename("GetObject", "_GetObject") 
#import <esriSystemUI.olb> named_guids no_namespace no_implementation raw_interfaces_only exclude("UINT_PTR", "OLE_HANDLE", "OLE_COLOR")
#import <esriGeometry.olb> named_guids no_namespace no_implementation raw_interfaces_only exclude("UINT_PTR", "OLE_HANDLE", "OLE_COLOR")
#import <esriGeodatabase.olb> named_guids no_namespace no_implementation raw_interfaces_only exclude("UINT_PTR", "OLE_HANDLE", "OLE_COLOR") \
  rename("GetMessage", "_GetMessage")
#import <esriDataSourcesGDB.olb> named_guids no_namespace no_implementation raw_interfaces_only exclude("UINT_PTR", "OLE_HANDLE", "OLE_COLOR")
#import <esriDataSourcesFile.olb> named_guids no_namespace no_implementation raw_interfaces_only exclude("UINT_PTR", "OLE_HANDLE", "OLE_COLOR")
#import <esriDataSourcesRaster.olb> named_guids no_namespace no_implementation raw_interfaces_only exclude("UINT_PTR", "OLE_HANDLE", "OLE_COLOR")\
  rename("StartService", "_StartService")
#import <esriDisplay.olb> named_guids no_namespace no_implementation raw_interfaces_only exclude("UINT_PTR", "OLE_HANDLE", "OLE_COLOR") \
  rename("RGB", "_RGB") rename("CMYK", "_CMYK") rename("ResetDC", "_ResetDC") rename ("DrawText", "_DrawText")
#import <esriCarto.olb> named_guids no_namespace no_implementation raw_interfaces_only exclude("UINT_PTR", "OLE_HANDLE", "OLE_COLOR") \
  rename("PostMessage", "_PostMessage") 
#import <esriGeoprocessing.olb> named_guids no_namespace no_implementation raw_interfaces_only exclude("UINT_PTR", "OLE_HANDLE", "OLE_COLOR") \
  rename("GetMessage", "_GetMessage") rename("GetObject", "_GetObject")
#pragma warning (pop)

#undef OUT
#undef length
#undef ERROR
#undef min
#undef max

#define R_NO_REMAP
#include <R.h>
#include <Rversion.h>
#include <Rdefines.h>
//#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#define RVERSION_DLL_BUILD R_MAJOR "." R_MINOR

using namespace ATL;
bool isCancel();
