#include "stdafx.h"
#include "tools.h"
#include "rconnect_interface.h"
#include "rarcproxy_exports.h"
#include "gp_env.h"

#include <memory>
/*
class gp_connect_impl : public rconnect_interface
{
  rconnect_interface* save;
public:
  CComPtr<IGeoProcessor> m_ipGeoProcessor;
  ITrackCancel* m_pTrackCancel;
  gp_connect_impl() : m_pTrackCancel(NULL), save(NULL){}
  ~gp_connect_impl()
  {
    if (m_pTrackCancel)
      m_pTrackCancel->Release();
    Rconnect(save);
  }

  bool init()
  {
    ATLASSERT(m_ipGeoProcessor == NULL);
    ATLASSERT(m_pTrackCancel == NULL);
    m_ipGeoProcessor.CoCreateInstance(CLSID_GeoProcessor);
    if (m_ipGeoProcessor != NULL)
    {
      //m_ipGeoProcessor->GetCurrentTrackCancel(&m_pTrackCancel);
      save = Rconnect(static_cast<rconnect_interface*>(this));
      if (!save)
      {
        if (m_ipGeoProcessor != NULL)
          m_ipGeoProcessor->AddError(CComBSTR(L"failed to initialize R interpreter"));
        m_ipGeoProcessor.Release();
        return false;
      }
    }
    return m_ipGeoProcessor != NULL;
  }
  bool isCancel() const
  {
    static DWORD lastCall = ::GetTickCount();
    if (!m_pTrackCancel)
      return false;

    DWORD cur = ::GetTickCount() - 300;
    if (cur > lastCall)
    {
      VARIANT_BOOL bContinue = VARIANT_FALSE;
      m_pTrackCancel->Continue(&bContinue);
      lastCall = ::GetTickCount() - 300;
      if (bContinue == VARIANT_FALSE)
        return true;//UserBreak = 1;
    }
    return false;
  }

  void show_help(const wchar_t *url) const {} //ignore
  void print_out(const wchar_t *str, int e) const
  {
    struct _e{std::wstring s;
              esriGPMessageType e;
              void move(_e& x){ s = x.s; x.s.clear(); e=x.e; x.e = (esriGPMessageType)-1; }
    };
    static _e hold = {L"", (esriGPMessageType)-1};
    _e last, msg;
    if (e == -1)
      last.move(hold);
    else
    {
      esriGPMessageType e0;
      switch (e)
      {
        case  0: e0 = esriGPMessageTypeInformative; break;
        case  1: e0 = esriGPMessageTypeError;break;
        default: e0 = esriGPMessageTypeWarning; break;
      }
      if (hold.e != e0)
        last.move(hold);
      hold.s.append(str);
      hold.e = e0;

      if (hold.s.length() > 1 && *(hold.s.rbegin()) == L'\n')
        msg.move(hold);
      if (last.s.length() == 1 && *(last.s.rbegin()) == L'\n')
        last.s.clear();
    }
    if (m_ipGeoProcessor)//direct to GP progress dialog
    {
      if (!last.s.empty())
        newMessage(last.s, m_ipGeoProcessor, last.e);
      if (!msg.s.empty())
        newMessage(msg.s, m_ipGeoProcessor, msg.e);
    }
  }
  static bool newMessage(std::wstring &str, IGeoProcessor *pGP, esriGPMessageType type=esriGPMessageTypeInformative)
  {
    if (str.empty())
      return false;
    if (!pGP)
      return false;

    while (!str.empty() && *str.rbegin() == L'\n')
      str.erase(--str.end());

    while (!str.empty() && *str.begin() == L'\n')
      str.erase(str.begin());

    if (str.empty())
      return false;

    CComBSTR bstr(str.c_str());
    switch (type)
    {
      case esriGPMessageTypeInformative:
          pGP->AddMessage(bstr);
      break;
      case esriGPMessageTypeWarning:
        pGP->AddWarning(bstr);
      break;
      default:
        pGP->AddError(bstr);
      break;
    }
    return true;
  }
};
*/
/////////////
/////////////
/////////////
const std::pair<SEXP, std::string> param2r(IGPParameter* pParam)
{
  CComBSTR name;
  pParam->get_Name(&name);
  CComPtr<IGPValue> ipVal;
  pParam->get_Value(&ipVal);
  return std::make_pair(gpvalue2any(ipVal), tools::toUtf8(name));
}

bool r2param(SEXP r, IGPParameter* pParam)
{
  CComPtr<IGPValue> ipVal;
  pParam->get_Value(&ipVal);
  return any2gpvalue(r, ipVal);
}

extern const rconnect_interface* current_connect;
int execute_tool2(const wchar_t* script_path, IArray* pParameters)
{
  if (pParameters == 0)
    return 0;
  //gp_connect_impl connect;
  //if (!connect.init())
  //  return 1;

  if (pParameters == 0)
    return 0;

  _bstr_t file_path(script_path);
  long nParams = 0;
  pParameters->get_Count(&nParams);
  //CComQIPtr<IGPScriptTool>(pGPTool)->get_FileName(file_path.GetAddress());


  bool ok = true;
  int errorOccurred = 0;

  if (file_path.length() && nParams)
  {
    std::vector< CAdapt<CComPtr<IGPParameter> > > return_params;
    //ipParameters->get_Count(&n);
    SEXP arc_env = Rf_findVar(Rf_install("arc"), R_GlobalEnv);

    {
      std::vector<SEXP> in_params;
      std::vector<std::string> in_params_names;
      std::vector<SEXP> out_params;
      std::vector<std::string> out_params_names;
      tools::protect pt;

      for (int i = 0; i < nParams; i++)
      {
        CComPtr<IUnknown> ipUnk;
        pParameters->get_Element(i, &ipUnk);
        CComQIPtr<IGPParameter> ipParam(ipUnk);
        esriGPParameterDirection eD;
        ipParam->get_Direction(&eD);
        std::pair<SEXP, std::string> p = param2r(ipParam);
        if (eD == esriGPParameterDirectionInput)
        {
          in_params.push_back(pt.add(p.first));
          in_params_names.push_back(p.second);
        }
        else
        {
          out_params.push_back(pt.add(p.first));
          out_params_names.push_back(p.second);
          return_params.push_back(ipParam);
        }
      }

      SEXP p1 = tools::newVal(in_params, pt);
      tools::nameIt(p1, in_params_names);
      SEXP p2 = tools::newVal(out_params, pt);
      tools::nameIt(p2, out_params_names);

      Rf_defineVar(Rf_install(".file"), tools::newVal(file_path, pt), arc_env);
      Rf_defineVar(Rf_install(".in"),  p1, arc_env);
      Rf_defineVar(Rf_install(".out"), p2, arc_env);
    }

    const static wchar_t eval_str[] = L"arc$.ret<-local({"
      L"en<-new.env(hash=TRUE);"
      L"eval(parse(file=arc$.file), envir=en);"
      L"tool_exec<-get('tool_exec',en);"
      L"tool_exec(in_param, out_param)"
      L"},envir=list('in_param'=arc$.in,'out_param'=arc$.out))";

    ok = current_connect->eval_one(eval_str) == 1;
    current_connect->print_out(NULL, -1);

    Rf_defineVar(Rf_install(".file"), R_NilValue, arc_env);
    Rf_defineVar(Rf_install(".in"), R_NilValue, arc_env);
    Rf_defineVar(Rf_install(".out"), R_NilValue, arc_env);
    R_gc();

    //TODO: handle ok
    if (ok)
    {
      /*CComPtr<IGPMessages> ipMsgs;
      if (connect.m_ipGeoProcessor)
        connect.m_ipGeoProcessor->GetReturnMessages(&ipMsgs);
      if (ipMsgs)
      {
        VARIANT_BOOL bErr = VARIANT_FALSE;
        CComQIPtr<IGPMessage>(ipMsgs)->IsError(&bErr);
        if (bErr != VARIANT_FALSE)
          ok = false;
      }*/
      if (!return_params.empty())
      {
        //connect.m_ipGeoProcessor->Is
        SEXP ret = Rf_findVar(Rf_install(".ret"), arc_env);
        tools::vectorGeneric ret_out(ret);
        //tools::vectorGeneric ret_out(ret.get());
        for (size_t i = 0, n = return_params.size(); i < n; i++)
        {
          _bstr_t name;
          return_params[i].m_T->get_Name(name.GetAddress());
          size_t idx = ret_out.idx(std::string(name));
          if (idx != (size_t)-1)
          {
            if (!r2param(ret_out.at(idx), return_params[i].m_T))
            {
              std::wstring msg(L"failed to set output parameter - ");
              msg += name;
              current_connect->print_out(msg.c_str(), 2);
            }
          }
        }
        //TODO list
      }
      Rf_defineVar(Rf_install(".ret"), R_NilValue, arc_env);
    }
  }
  return ok ? 0 : 1;
}
