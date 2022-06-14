// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <filesystem>
namespace fs = std::filesystem;

#pragma comment(lib, "Delayimp.lib")

HMODULE hDllHandle = NULL;
HMODULE hapi_dll = 0;
fn_api get_api = nullptr;
const arcobject::API* _api = nullptr;

#if !defined(DESKTOP10)
#pragma comment(lib, "Version.lib")
static const std::array<unsigned short, 4> query_product_ver(const fs::path& core_dll_path)
{
  //static const auto code_dll_name = L"AfCore.dll";
  DWORD verSize = ::GetFileVersionInfoSize(core_dll_path.c_str(), nullptr);
  if (verSize > 0)
  {
    UINT size = 0;
    std::vector<BYTE> buff(verSize);
    LPBYTE lpBuffer = NULL;
    if (::GetFileVersionInfo(core_dll_path.c_str(), 0, verSize, &buff[0]) && ::VerQueryValue(&buff[0], L"\\", (VOID FAR * FAR*) & lpBuffer, &size))
    {
      VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
      return {
        (unsigned short)((verInfo->dwProductVersionMS >> 16) & 0xffff),
        (unsigned short)((verInfo->dwProductVersionMS >> 0) & 0xffff),
        (unsigned short)((verInfo->dwProductVersionLS >> 16) & 0xffff),
        (unsigned short)((verInfo->dwProductVersionLS >> 0) & 0xffff)
      };
    }
  }
  return { 0,0,0,0 };
}

static fs::path pro_install_bin_path()
{
  static const wchar_t code_dll_name[] = L"AfCore.dll";
  for (auto h : { HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER })
  {
    for (const auto& str : { L"SOFTWARE\\Esri\\ArcGISPro", L"SOFTWARE\\Esri\\ArcGISPro1.0" })
    {
      wchar_t szFile[2048];
      ULONG len = _countof(szFile);
      LSTATUS res = ::RegGetValue(h, str, L"InstallDir", RRF_RT_REG_SZ, nullptr, (LPBYTE)szFile, &len);
      if (res != ERROR_SUCCESS)
        continue;
      try
      {
        fs::path path;
        path = szFile;
        path /= L"bin";
        if (fs::exists(path / code_dll_name))
          return path;
      }
      catch (...) {}
    }
  }
  return {};
}
#endif

bool load_arcobjectlib(HMODULE hModule)
{
  if (_api != nullptr)
    return true;

  wchar_t dllName[MAX_PATH] = { 0 };
  ::GetModuleFileName(hModule, dllName, _countof(dllName));

  //prepare search directories to load libobjecs.dll
  auto tmp = fs::path(dllName).parent_path();
  //force load api dll

  fs::path lib_api_dll_name(LIBRARY_API_DLL_NAME ".dll");

#if !defined(DESKTOP10)
  //in Pro 3.0+ COM interfaces are not binary compatible with previos version
  //so, find libobject_pro{runtime PRO_VER}.dll
  {
    std::array<unsigned short, 4> ver{0,0,0,0};
    if (auto h = GetModuleHandle(L"AfCore.dll"); h != 0)
    {
      wchar_t path[MAX_PATH] = { 0 };
      ::GetModuleFileName(h, path, _countof(path));
      ver = query_product_ver(path);
    }
    else
    {
      auto bin_path = pro_install_bin_path();
      ver = query_product_ver(bin_path / L"AfCore.dll");
    }

    if (ver[0] >= 13)
    {
      std::wstring ver_major(std::to_wstring(ver[0] - 10));
      std::wstring ver_full = ver_major + std::to_wstring(ver[1]);
      for (const auto& v : { ver_full, ver_major })
      {
        fs::path dll_ver(LIBRARY_API_DLL_NAME);
        dll_ver += v + L".dll";
        fs::path full_path(tmp);
        full_path /= dll_ver;
        if (std::error_code e; fs::exists(full_path, e))
        {
          lib_api_dll_name = dll_ver;
          break;
        }
      }
    }
  }
#else
  //prior Pro 3.0 we have to pin dll from unloading
  HMODULE hm;
  ::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, dllName, &hm);
#endif
  tmp /= lib_api_dll_name;

  hapi_dll = ::LoadLibraryEx(tmp.wstring().c_str(), NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
  get_api = (fn_api)::GetProcAddress(hapi_dll, "api");
  ATLASSERT(get_api != nullptr);
  if (get_api == nullptr)
    return false;

  try {
    extern bool g_InProc;
    ATLASSERT(g_InProc == false && get_api != nullptr);
    auto api = get_api(g_InProc);
    if (api == nullptr)
      return false;
    //validate
    api->getLastComError();
    _api = api;
    return true;
  }
  catch (...) { }
  return false;
}

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved)
{
  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
    {
      ::DisableThreadLibraryCalls(hModule);
      hDllHandle = hModule;
      _api = nullptr;
      if (auto h = GetModuleHandle(L"AfCore.dll"); h != 0)
      {
#if !defined(DESKTOP10)
        wchar_t path[MAX_PATH] = { 0 };
        ::GetModuleFileName(h, path, _countof(path));
        auto ver = query_product_ver(path);
        //Desktop and Pro < 3.0 keep from unloading
        if (ver[0] < 13)
#endif        
        {
          HMODULE hm = nullptr;
          ::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)hModule, &hm);
        }
      }
    }
    break;
    case DLL_PROCESS_DETACH:
    {
      if (hapi_dll != 0)
        FreeLibrary(hapi_dll);
      hapi_dll = 0;
      ::OutputDebugStringA(DLL_NAME_STR " Terminated!\n");
    }
    break;
  }
  return TRUE;
}
