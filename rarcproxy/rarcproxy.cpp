// rarcproxy.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "rarcproxy_exports.h"

#define ADD_FRIEND
#include "rconnect_interface.h"

const rconnect_interface* current_connect = NULL;
rconnect_interface::fn_do1line initializeR();
int execute_tool2(const wchar_t* script_path, IArray* pParameters);

rconnect_interface* Rconnect(rconnect_interface *connect)
{
  ATLASSERT(sizeof(rconnect_interface) == 16);

  static rconnect_interface::fn_do1line eval = NULL;
  rconnect_interface* old = const_cast<rconnect_interface*>(current_connect);

  if (eval == NULL)
  { 
    eval = (rconnect_interface::fn_do1line)1;
    current_connect = connect;
    rconnect_interface::fn_do1line f = initializeR();
    current_connect = old;
    if (f) eval = f;
  }
  if (eval == (rconnect_interface::fn_do1line)1)
    return NULL;

  if (old)
  {
    old->do1line = NULL;
    old->gp_tool_exec = NULL;
  }

  if (connect == (rconnect_interface*)1)
  {
    current_connect = NULL;
    return (rconnect_interface*)1;
  }

  if (connect != old)
  {
    current_connect = connect;
    if (connect)
    {
      connect->do1line = eval;
      connect->gp_tool_exec = (rconnect_interface::fn_gptool_exec)execute_tool2;
    }
  }
  return old ? old : (rconnect_interface*)1;
}

namespace GNU_GPL
{
  bool initInProcInterpreter(bool bInteractive);
  int __cdecl do1line(const wchar_t* code);
};

rconnect_interface::fn_do1line initializeR()
{ 
  if (GNU_GPL::initInProcInterpreter(true))
    return (rconnect_interface::fn_do1line)GNU_GPL::do1line;
  return NULL;
}

