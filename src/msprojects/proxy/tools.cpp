#include "stdafx.h"
#include "tools.h"
#include "tchannel.h"

extern DWORD g_main_TID;

SEXP fn::wrap(const fn& it)
{
  if (g_main_TID == 0)
  {
    Rf_error("Could not bind to a valid ArcGIS installation");
    return R_NilValue;
  }

  if (g_main_TID != GetCurrentThreadId())
  {
    tchannel& channel = tchannel::singleton();
    //this is R thread
    //tchannel::value p(tchannel::FN_CALL, &it);
    channel.from_thread.push_back(tchannel::value{tchannel::value_type::FN_CALL, &it});
    tchannel::value p;
    channel.to_thread.pop(p, INFINITE);
    if (!channel.all_errors_text.empty()) //show error now -> arc._Call
    {
      Rf_defineVar(Rf_install(".arc_err"), tools::newVal(channel.all_errors_text), R_GlobalEnv);
      channel.all_errors_text.clear();
    }
    ATLASSERT(p.type == tchannel::value_type::FN_CALL_RET);
    return p.data_sexp();
  }
  else
    return it.call();
}
/*
template<class... Args, class T, class R>
auto resolve(R (T::*m)(Args...)) -> decltype(m)
{ return m; }

template<class T, class R>
auto resolve(R (T::*m)(void)) -> decltype(m)
{ return m; }
*/

