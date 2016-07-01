// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <filesystem>
#pragma comment(lib, "Delayimp.lib")

HMODULE hDllHandle = NULL;

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

      HMODULE hm;
      wchar_t dllName[MAX_PATH] = { 0 };
      ::GetModuleFileName(hModule, dllName, _countof(dllName));
      ::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_PIN, dllName, &hm);
      //prepare search directories to load libtieup.dll
      auto tmp = std::tr2::sys::path(dllName).remove_filename();
      ::AddDllDirectory(tmp.c_str());

      //force load api dll
      tmp /= LIBRARY_API_DLL_NAME ".dll";
      ::LoadLibraryEx(tmp.c_str(), NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
      //verify it
      arcobject::getLastComError();
    }
    break;
    case DLL_PROCESS_DETACH:
      ::OutputDebugStringA(DLL_NAME_STR " Terminating!\n");
    break;
  }
  return TRUE;
}

