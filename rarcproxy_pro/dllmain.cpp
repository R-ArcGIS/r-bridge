// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

HMODULE hDllHandle = NULL;
char dllName[MAX_PATH] = {0};

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
  switch (ul_reason_for_call)
  {
    case DLL_PROCESS_ATTACH:
    {
      HMODULE hm;
      GetModuleFileNameA(hModule, dllName, _countof(dllName));
      GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_PIN, dllName, &hm);
      DisableThreadLibraryCalls(hModule);
      hDllHandle = hModule;
    }
    break;
    case DLL_PROCESS_DETACH:
      ::OutputDebugString(L"rarcproxy_pro.dll Terminating!\n");
    break;
  }
  return TRUE;
}

