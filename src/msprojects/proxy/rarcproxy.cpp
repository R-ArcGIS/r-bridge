// rarcproxy.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "rarcproxy_exports.h"

#define ADD_FRIEND
#include "rconnect_interface.h"

volatile rconnect_interface* _current_connect = nullptr;
const rconnect_interface* current_connect()
{
  return const_cast<const rconnect_interface*>(_current_connect);
}

namespace GNU_GPL
{
  bool initInProcInterpreter();
};
int execute_tool2(const wchar_t* script_path, IArray* pParameters);

rconnect_interface* Rconnect(rconnect_interface *connect)
{
  static int once = 0;

  if (once == 0)
  {
    once = 1;
    _current_connect = connect;
    if (GNU_GPL::initInProcInterpreter())
      once = 2;
    _current_connect = nullptr;
  }
  if (once == 1)
    return NULL;

  auto old = const_cast<rconnect_interface*>(_current_connect);

  if (old != nullptr)
    old->gp_tool_exec = nullptr;

  if (connect == (rconnect_interface*)1)
  {
    _current_connect = NULL;
    return (rconnect_interface*)1;
  }

  if (connect != old)
  {
    _current_connect = connect;
    if (connect)
    {
#ifdef DESKTOP10
      connect->padding[0] = connect->padding[1] = nullptr;
#endif
      connect->gp_tool_exec = (rconnect_interface::fn_gptool_exec)execute_tool2;
    }
  }
  return old != nullptr ? old : (rconnect_interface*)1;
}
