#include "stdafx.h"
#define ESRI_WINDOWS
#define ESRI_QUICK
#undef STRICT
#include <CppAPI\ArcSDK.h> //require - ArcObjectsSDKCPP10.3.1
#include "tools.h"
#include <sstream>
#pragma comment(lib, "Version.lib")

const wchar_t* set_gdal_data_path();
extern DWORD g_main_TID;
wchar_t arcgis_path[MAX_PATH] = {0};

SEXP R_AoInitialize()
{
  static std::string sver;
  if (g_main_TID == 0)
  {
    ::CoInitialize(NULL);
    VARIANT_BOOL vb = 0;
    CComPtr<IArcGISVersion> ipVersion; ipVersion.CoCreateInstance(_CLSID_VERSION_MANAGER);
    if (ipVersion == NULL)
      return showError<true>();
#ifdef _WIN64
    HRESULT hr = ipVersion->LoadVersion(esriArcGISEngine, CComBSTR(L"10.3"), &vb);
#else
    HRESULT hr = ipVersion->LoadVersion(esriArcGISDesktop, CComBSTR(L"10.3"), &vb);
#endif
    if (hr != S_OK)
      return showError<true>();

    DWORD  verHandle = NULL;
    LPCWSTR dll_name = L"DADFLib.dll";
    HMODULE hm = GetModuleHandle(dll_name);
    wchar_t bin_path[MAX_PATH] = {0};

    if (hm != NULL && ::GetModuleFileName(hm, bin_path, MAX_PATH))
    {
      ::PathRemoveFileSpec(bin_path);
      ::AddDllDirectory(bin_path);
      ::PathRemoveFileSpec(bin_path);
      wcscpy_s(arcgis_path, bin_path);
#ifndef _WIN64
      GetShortPathName(arcgis_path, arcgis_path, MAX_PATH);
#endif
      set_gdal_data_path();
    }

    DWORD  verSize = GetFileVersionInfoSize(dll_name, &verHandle);
    if (verSize > 0)
    {
      UINT size = 0;
      std::vector<BYTE> buff(verSize);
      LPBYTE lpBuffer = NULL;
      if (GetFileVersionInfo(dll_name, verHandle, verSize, &buff[0]) &&  VerQueryValue(&buff[0],L"\\",(VOID FAR* FAR*)&lpBuffer, &size))
      {
        VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
        std::ostringstream ver;
        ver << ((verInfo->dwProductVersionMS>>16)&0xffff) << '.' << ((verInfo->dwProductVersionMS>>0)&0xffff) << '.';
        ver << ((verInfo->dwProductVersionLS>>16)&0xffff) << '.' << ((verInfo->dwProductVersionLS>>0)&0xffff);
        sver = ver.str();
      }
    }
  }

  static struct {esriLicenseProductCode e; const char* name;} try_code[] = 
  {
    { esriLicenseProductCodeAdvanced,    "Advanced"},
    { esriLicenseProductCodeStandard,    "Standard"},
    { esriLicenseProductCodeBasic,       "Basic"   },
    { esriLicenseProductCodeArcServer,   "Server"},
    { esriLicenseProductCodeEngineGeoDB, "EngineGeoDB"},
    { esriLicenseProductCodeEngine,      "Engine"   },
  };
  IAoInitializePtr ipAO;
  HRESULT hr = ipAO.CreateInstance(CLSID_AoInitialize);

  static const char *product_name = NULL;
  for(int i = 0; product_name == NULL && i < _countof(try_code); i++)
  {
    esriLicenseStatus sa = esriLicenseFailure;
    hr = ipAO->Initialize(try_code[i].e, &sa);
    if (sa == esriLicenseFailure || sa == esriLicenseNotInitialized || sa == esriLicenseNotLicensed)
      continue;
    product_name = try_code[i].name;
    break;
  }
  if (product_name == NULL)
    return showError<true>(L"Could not bind to a valid ArcGIS installation");

  tools::listGeneric info(3);
  info.push_back(product_name, "license");
  info.push_back(sver, "version");

  info.push_back(std::wstring(arcgis_path), "path");

  g_main_TID = GetCurrentThreadId();

  return info.get();
}
