#include "stdafx.h"
#include "tchannel.h"
#include "tools.h"
#include "misc.h"
#include "rconnect_interface.h"

extern const rconnect_interface* current_connect;
extern DWORD g_main_TID;

SEXP arc_progress_label(SEXP arg)
{
  /* not impl
  if (current_connect)
  {
    std::wstring msg;
    if (!tools::copy_to(arg, msg))
      return R_NilValue;
    current_connect->progress_title(msg.c_str());
  }*/
  return R_NilValue;
}

SEXP arc_progress_pos(SEXP arg)
{
  /* not impl
  if (current_connect)
  {
    int pos = -1;
    if (!tools::copy_to(arg, pos))
      return R_NilValue;
    current_connect->progress(pos);
  }*/
  return R_NilValue;
}

tchannel& tchannel::singleton()
{
  static tchannel _channel;
  return _channel;
}

static void woe_forward(SEXP e, bool is_error)
{
  std::vector<std::wstring> s;
  tools::copy_to(e, s);
  try 
  {
    tchannel::value p(tchannel::R_TXT_OUT, new std::wstring(1, is_error? L'1' : L'2'));
    for (size_t i = 0, n = s.size(); i < n; i++)
      p.data_str->append(s[i]);
    tchannel& channel = tchannel::singleton();
    channel.from_thread.push(p);
  }catch(...){}
}
SEXP arc_warning(SEXP e)
{
  if (current_connect)
    woe_forward(e, false);
  return R_NilValue;
}

SEXP arc_error(SEXP e)
{
  ATLTRACE("arc_error() %s", R_curErrorBuf());
  if (current_connect)
    woe_forward(e, true);
  return R_NilValue;
}

void arc_browsehelp(char ** url)
{
  std::wstring u = tools::fromUtf8(url[0]);
  if (current_connect)
    current_connect->show_help(u.c_str());
}

extern bool g_InProc;

SEXP error_Ret(const char* str_or_UTF8, SEXP retVal)
{
  if (!g_InProc)
  {
    Rf_error(str_or_UTF8);
    return retVal;
  }
  tchannel& channel = tchannel::singleton();
  if (!channel.all_errors_text.empty())
    channel.all_errors_text.append("\n");
  channel.all_errors_text.append(str_or_UTF8);
  return retVal;
}

SEXP R_fnN(fn_struct &it)
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
    tchannel::value p(tchannel::FN_CALL, &it);
    channel.from_thread.push(p);
    channel.to_thread.pop(p, INFINITE);
    if (!channel.all_errors_text.empty()) //show error now -> arc._Call
    {
      Rf_defineVar(Rf_install(".arc_err"), tools::newVal(channel.all_errors_text), R_GlobalEnv);
      channel.all_errors_text.clear();
    }
    ATLASSERT(p.type == tchannel::FN_CALL_RET);
    return p.data_sexp;
  }
  else
    return it.call();
}

