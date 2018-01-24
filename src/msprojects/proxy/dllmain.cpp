// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <filesystem>
#if _HAS_CXX17
namespace fs = std::experimental::filesystem::v1;
#else
namespace fs = std::tr2::sys;
#endif

#pragma comment(lib, "Delayimp.lib")

HMODULE hDllHandle = NULL;
HMODULE hapi_dll = 0;
fn_api get_api = nullptr;
const arcobject::API* _api = nullptr;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
    {
      ::DisableThreadLibraryCalls(hModule);
      hDllHandle = hModule;

      wchar_t dllName[MAX_PATH] = { 0 };
      ::GetModuleFileName(hModule, dllName, _countof(dllName));
      HMODULE hm;
      ::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, dllName, &hm);
      //prepare search directories to load libobjecs.dll
      auto tmp = fs::path(dllName).remove_filename();
      ::AddDllDirectory(tmp.c_str());

      //force load api dll
      tmp /= LIBRARY_API_DLL_NAME ".dll";
      hapi_dll = ::LoadLibraryEx(tmp.c_str(), NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
      get_api = (fn_api)::GetProcAddress(hapi_dll, "api");
      ATLASSERT(get_api != nullptr);
      //verify it
      if (get_api == nullptr)
        return FALSE;
    }
    break;
    case DLL_PROCESS_DETACH:
    {
      if (hapi_dll != 0)
        FreeLibrary(hapi_dll);
      hapi_dll = 0;
      get_api = nullptr;
      ::OutputDebugStringA(DLL_NAME_STR " Terminated!\n");
    }
    break;
  }
  return TRUE;
}
