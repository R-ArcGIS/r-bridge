// rarcproxy.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "rarcproxy_exports.h"

#define ADD_FRIEND
#include "rconnect_interface.h"

//__declspec(thread) const rconnect_interface*  current_connect = NULL;
const rconnect_interface*  current_connect = NULL;
namespace GNU_GPL
{
  bool initInProcInterpreter(bool bInteractive);
};
int execute_tool2(const wchar_t* script_path, IArray* pParameters);

rconnect_interface* Rconnect(rconnect_interface *connect)
{
  //ATLASSERT(sizeof(rconnect_interface) == 32);

  static int once = 0;
  rconnect_interface* old = const_cast<rconnect_interface*>(current_connect);

  if (once == 0)
  {
    once = 1;
    current_connect = connect;
    once = GNU_GPL::initInProcInterpreter(true) ? 2 : 1;
    current_connect = old;
  }
  if (once == 1)
    return NULL;

  if (old)
    old->gp_tool_exec = NULL;

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
#ifdef DESKTOP10
      connect->padding[0] = connect->padding[1] = nullptr;
#endif
      connect->gp_tool_exec = (rconnect_interface::fn_gptool_exec)execute_tool2;
    }
  }
  return old ? old : (rconnect_interface*)1;
}

