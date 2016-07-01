#include "stdafx.h"
#include "tchannel.h"
#include "tools.h"
#include "misc.h"
#include "rconnect_interface.h"

extern const rconnect_interface* current_connect;
extern DWORD g_main_TID;

#if 0
extern "C" _declspec(dllexport) SEXP showArgs(SEXP args)
{
    args = CDR(args); /* skip 'name' */
    const char *me = CHAR(PRINTNAME(TAG(args)));
    for(int i = 0; args != R_NilValue; i++, args = CDR(args)) 
    {
        const char *name = Rf_isNull(TAG(args)) ? "" : CHAR(PRINTNAME(TAG(args)));
        SEXP el = CAR(args);
        if (Rf_length(el) == 0) 
        {
            Rprintf("[%d] '%s' R type, length 0\n", i+1, name);
           continue;
        }
        int t = TYPEOF(el);
        switch(t)
        {
        case REALSXP:
            Rprintf("[%d] '%s' %f\n", i+1, name, REAL(el)[0]);
            break;
        case LGLSXP:
        case INTSXP:
            Rprintf("[%d] '%s' %d\n", i+1, name, INTEGER(el)[0]);
            break;
        case CPLXSXP:
        {
            Rcomplex cpl = COMPLEX(el)[0];
            Rprintf("[%d] '%s' %f + %fi\n", i+1, name, cpl.r, cpl.i);
        }
            break;
        case STRSXP:
            Rprintf("[%d] '%s' %s\n", i+1, name,
                   CHAR(STRING_ELT(el, 0)));
           break;
        default:
            Rprintf("[%d] '%s' R type (%d)\n", i+1, name, t);
       }
    }
    return(R_NilValue);
}
#endif

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

SEXP extent2r(const std::vector<double> &ext)
{
  if (ext.empty())
     return R_NilValue;

  static std::vector<std::string> names(4);
  if (names.front().empty())
  {
    names[0] = "xmin";
    names[1] = "ymin";
    names[2] = "xmax";
    names[3] = "ymax";
  }
  return tools::nameIt(tools::newVal(ext), names);
}

//#define USE_MS_TIME_API // accurate and slower

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

VARIANT r2variant(SEXP r, VARTYPE vt)
{
  VARIANT v;
  ::VariantInit(&v);
  v.vt = vt;
  switch (vt)
  {
    case VT_I1: case VT_UI1:
      v.bVal = (BYTE)Rf_asInteger(r);
      return v;
    case VT_BOOL:
      v.boolVal = Rf_asLogical(r) ? VARIANT_TRUE : VARIANT_FALSE;
      return v;
    case VT_I4: case VT_UI4:
      v.lVal = Rf_asInteger(r);
      return v;
    case VT_R4:
      v.fltVal = (float)Rf_asReal(r);
      return v;
    case VT_R8:
      v.dblVal = Rf_asReal(r);
      return v;
    case VT_DATE:
      v.dblVal = epoch2ole(Rf_asReal(r));
      return v;
    default:
      ATLASSERT(0);
    case VT_BSTR:
    {
      SEXP rs = Rf_asChar(r);
      std::wstring str;
      if (tools::copy_to(rs, str))
      {
        v.vt = VT_BSTR;
        v.bstrVal = CComBSTR(str.c_str()).Detach();
        return v;
      }
      v.vt = VT_NULL;
    }
  }
  return v;
}

bool r2variant(SEXP r, VARIANT &v)
{
  v.vt = VT_NULL;
  if (r == NULL || Rf_isNull(r))
    return true;
  auto n = tools::size(r);
  if (n == 0)
    v.vt = VT_NULL;
  else if (n == 1 || TYPEOF(r) == STRSXP)
  {
    tools::protect pt;
    SEXP x = pt.add(Rf_asChar(r));
    std::wstring str;
    if (tools::copy_to(x, str))
    {
      v.vt = VT_BSTR;
      v.bstrVal = ::SysAllocString(str.c_str());
    }
  }
  else //multi values
  {
    ATLASSERT(0);
  }
  return true;
}

inline static SEXP make_safe_return(const std::string& str)
{
  if (str.empty())
    return Rf_ScalarString(R_NaString);

  SEXP ret = tools::newVal(str);
  return ret == R_NaString ? Rf_ScalarString(R_NaString): ret;
}

SEXP R_fromWkt2P4(SEXP e)
{
  int wktid = 0;
  std::string p4;
  if (tools::copy_to(e, wktid))
  {
    p4 = arcobject::fromWkID2P4(wktid);
  }
  else
  {
    std::string wkt;
    tools::copy_to(e, wkt);
    wkt.erase(wkt.find_last_not_of(' ') + 1);
    if (wkt.empty())
      return Rf_ScalarString(R_NaString);
    p4 = arcobject::fromWkt2P4(wkt);
  }
  return make_safe_return(p4);
}

SEXP R_fromP42Wkt(SEXP str)
{
  std::string p4;
  tools::copy_to(str, p4);
  p4.erase(p4.find_last_not_of(' ') + 1);
  if (p4.empty())
    return Rf_ScalarString(R_NaString);

  auto wkt = arcobject::fromP42Wkt(p4);
  return make_safe_return(wkt);
}

SEXP R_getEnv()
{
  tools::listGeneric vals(44);
  auto f([&vals](const std::wstring& name, VARIANT const &v)
  {
    vals.push_back(tools::newVal(v), name.c_str());
    return true;
  });
  //std::function<bool(void*,const std::wstring& name, VARIANT &v)> f1(std::cref(f));
  arcobject::gp_env_generate(std::cref(f));

  return vals.get();
}

extern DWORD g_main_TID;
SEXP R_AoInitialize()
{
  const arcobject::product_info &info= arcobject::AoInitialize();

  if (info.license.front() == '-')
    showError<true>(info.license.c_str() + 1);

  SEXP ns = R_FindNamespace(Rf_mkString("arcgisbinding"));
  ATLTRACE("SEXP type:%s", Rf_type2char(TYPEOF(ns)));

  g_main_TID = GetCurrentThreadId();

  tools::listGeneric vals(3);
  vals.push_back(info.license, L"license");
  vals.push_back(info.version, L"version");
  vals.push_back(info.install_path, L"path");
  return vals.get();
}

