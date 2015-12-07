#include "stdafx.h"
#include "tchannel.h"
#include "tools.h"
#include "misc.h"
#include "rconnect_interface.h"

extern const rconnect_interface* current_connect;
extern DWORD g_main_TID;

SEXP arc_progress_label(SEXP arg)
{
  if (current_connect)
  {
    std::wstring msg;
    if (!tools::copy_to(arg, msg))
      return R_NilValue;
    current_connect->progress_title(msg.c_str());
  }
  return R_NilValue;
}

SEXP arc_progress_pos(SEXP arg)
{
  if (current_connect)
  {
    int pos = -1;
    if (!tools::copy_to(arg, pos))
      return R_NilValue;
    current_connect->progress(pos);
  }
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

//#define USE_MS_TIME_API

#ifdef USE_MS_TIME_API

double ole2epoch(double d)
{
  if (ISNAN(d)) return d;

  SYSTEMTIME st;
  VariantTimeToSystemTime(d, &st);
  FILETIME ftime;
  SystemTimeToFileTime(&st, &ftime);
  ULARGE_INTEGER ul = { ftime.dwLowDateTime, ftime.dwHighDateTime };
  return (double)(__int64)(ul.QuadPart / 10000000ULL - 11644473600ULL);
}

double epoch2ole(double d)
{
  if (ISNAN(d)) return d;

  ULARGE_INTEGER ul;
  ul.QuadPart = (11644473600ULL + (__int64)d) * 10000000ULL;
  FILETIME ftime = { ul.LowPart, ul.HighPart };
  SYSTEMTIME st;
  FileTimeToSystemTime(&ftime, &st);
  SystemTimeToVariantTime(&st, &d);
  return d;
}
#else

double ole2epoch(double value)
{
  if (ISNAN(value)) return value;

  const int daysTo1970 = 25569;
  const int secPerDay = 86400;

  int days = (int)value;
  long double frac = std::fabs(value - days);   //fractional part of the day
  int sec = (int)(frac * secPerDay + 0.5);
  __int64 epoch = (__int64)(days - daysTo1970) * secPerDay + sec;
  return (double)epoch;
}

double epoch2ole(double epoch)
{
  if (ISNAN(epoch)) return epoch;

  const double daysTo1970 = 25569.0;
  const double secPerDay = 86400.0;
  if (epoch >= -2209075199.0) //31 Dec 1899 00:00:00
    return daysTo1970 + epoch / secPerDay;

  double days = -epoch / secPerDay;
  double frac = days - (int)days;

  days = daysTo1970 - std::ceil(days);
  if (frac > 1.0e-6)
    frac -= 1.0;
  else
    frac = 0.0;

  return days + frac;
}

#endif
