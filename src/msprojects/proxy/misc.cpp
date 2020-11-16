#include "stdafx.h"
#include "tchannel.h"
#include "tools.h"
#include "misc.h"
#include "rconnect_interface.h"
#include <sstream>
#include <filesystem>
#if _HAS_CXX17
namespace fs = std::filesystem;
#else
namespace fs = std::tr2::sys;
#endif

const rconnect_interface* current_connect();
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
  if (current_connect() != nullptr)
    woe_forward(e, false);
  return R_NilValue;
}

SEXP arc_error(SEXP e)
{
  ATLTRACE("arc_error() %s", R_curErrorBuf());
  if (current_connect() != nullptr)
    woe_forward(e, true);
  return R_NilValue;
}

SEXP arc_progress_label(SEXP arg)
{
  auto connect = current_connect();
  if (connect != nullptr)
  {
    std::wstring msg;
    if (!tools::copy_to(arg, msg))
      return R_NilValue;
    connect->progress_title(msg.c_str());
  }
  return R_NilValue;
}

SEXP arc_progress_pos(SEXP arg)
{
  auto connect = current_connect();
  if (connect != nullptr)
  {
    int pos = -1;
    if (!tools::copy_to(arg, pos))
      return R_NilValue;
    connect->progress(pos);
  }
  return R_NilValue;
}

SEXP error_Ret(const char* str_or_UTF8, SEXP retVal)
{
  extern bool g_InProc;
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

SEXP extent2r(const std::array<double, 4> &ext)
{
  if (ext[0] == 0.0 && ext[1] == 0.0 && ext[2] == 0.0 && ext[3] == 0.0)
     return R_NilValue;
  return tools::nameIt(tools::newVal(ext), {"xmin", "ymin", "xmax", "ymax"});
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
#if 0
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
        v.bstrVal = ::SysAllocString(str.c_str());
        return v;
      }
      v.vt = VT_NULL;
    }
  }
  return v;
}
#endif


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
    p4 = _api->fromWkID2P4(wktid);
  }
  else
  {
    std::string wkt;
    tools::copy_to(e, wkt);
    wkt.erase(wkt.find_last_not_of(' ') + 1);
    if (wkt.empty())
      return Rf_ScalarString(R_NaString);
    p4 = _api->fromWkt2P4(wkt);
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

  auto wkt = _api->fromP42Wkt(p4);
  return make_safe_return(wkt);
}

static SEXP get_env()
{
  tools::listGeneric vals(44);
  auto f([&vals](const std::wstring& name, VARIANT const &v)
  {
    vals.push_back(tools::newVal(v), name.c_str());
    return true;
  });
  //std::function<bool(void*,const std::wstring& name, VARIANT &v)> f1(std::cref(f));
  _api->gp_env_generate(std::cref(f));

  return vals.get();
}

SEXP R_getEnv()
{
  SEXP ret = nullptr;
  tchannel::gp_thread([&ret]() { ret = get_env(); });
  return ret;
}

arcobject::sr_type from_sr(SEXP sr)
{
  arcobject::sr_type sp_ref;
  if (TYPEOF(sr) == VECSXP && tools::size(sr) == 2)
  {
    tools::vector_iterator list(sr);
    auto v = from_sr(list.at(1));
    if (v.second != 0)
      return v;
    return from_sr(list.at(0));
  }
  sp_ref.second = 0;
  if (!tools::copy_to(sr, sp_ref.second))
  {
    std::wstring wkt;
    if (tools::copy_to(sr, wkt))
    {
      struct eq { static bool op(const wchar_t &c) { return c == L'\''; } };
      std::replace_if(wkt.begin(), wkt.end(), eq::op, L'\"');
      sp_ref.first = wkt;
    }
  }
  return sp_ref;
}

SEXP forward_from_keyvalue_variant(VARIANT &info)
{
  if (info.vt == VT_NULL) return R_NilValue;
  ATLASSERT((info.vt & VT_ARRAY) == VT_ARRAY && tools::SafeArrayHelper::GetCount(info.parray) == 2);
  SEXP ret = tools::newVal(info);
  ::VariantClear(&info);
  return ret;
}

SEXP R_AoInitialize()
{
  if (_api == nullptr)
    return error_Ret("Failed to load '" LIBRARY_API_DLL_NAME ".dll'");

  const arcobject::product_info &info = _api->AoInitialize();

  tools::listGeneric vals(3);

  if (info.license.front() == L'-')
  {
    //showError<true>(info.license.c_str() + 1);
    vals.push_back(info.license.c_str() + 1, "error");
    return vals.get();
  }
  vals.push_back(info.license, "license");
  std::wostringstream ver;
  ver << info.ver[0] << L'.' << info.ver[1] << L'.' << info.ver[2] << L'.' << info.ver[3];
  //info.version = ver.str();

  vals.push_back(ver.str(), "version");
  vals.push_back(info.install_path, "path");

  SEXP ns = R_FindNamespace(Rf_mkString("arcgisbinding"));
  ATLTRACE("SEXP type:%s", Rf_type2char(TYPEOF(ns)));

  if (g_main_TID == 0)
    g_main_TID = GetCurrentThreadId();

  return vals.get();
}

SEXP arc_Portal(SEXP surl, SEXP suser, SEXP spw, SEXP stoken)
{
  if (Rf_isNull(surl))
  {
    auto v = _api->portal_info();
    return forward_from_keyvalue_variant(v);
  }

  std::wstring token;
  std::wstring url;
  if (!tools::copy_to(surl, url))
    return error_Ret("missing 'url' param");

  if (!Rf_isNull(stoken))
  {
    if (!tools::copy_to(stoken, token))
      return error_Ret("invalid 'token' param");
    return tools::newVal(L"TODO token");
  }

  std::wstring user;
  if (!Rf_isNull(suser) && !tools::copy_to(suser, user))
    return error_Ret("invalid 'username' param");

  std::wstring pw;
  if (!Rf_isNull(spw) && !tools::copy_to(spw, pw))
    return error_Ret("invalid 'password' param");

  if (!_api->portal_signon(url, user, pw, token))
    return showError<true>();

  auto v = _api->portal_info();
  return forward_from_keyvalue_variant(v);
}

SEXP R_delete(SEXP spath)
{
  std::wstring path;
  if (!tools::copy_to(spath, path))
    return error_Ret("missing path");
  path = normalize_path(path);
  if (path.empty())
    return error_Ret("missing path");
  return tools::newVal(_api->delete_dataset(path));
}

std::wstring normalize_path(const std::wstring& path)
{
  fs::path fs_path(path);
  if (fs_path.is_relative())
  {
    wchar_t buf[MAX_PATH+1] = {0};
    if (::GetCurrentDirectoryW(MAX_PATH, buf))
    {
      fs_path = buf;
      fs_path /= path;
    }
  }
  return fs_path;
}
