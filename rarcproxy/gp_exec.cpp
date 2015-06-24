#include "stdafx.h"
#include "tools.h"
#include "rconnect_interface.h"
#include "gp_env.h"

#include <vector>
/////////////
/////////////
static const std::pair<SEXP, std::string> param2r(IGPUtilities* pUtils, IGPParameter* pParam)
{
  CComBSTR name;
  pParam->get_Name(&name);
  CComPtr<IGPValue> ipVal;
  if (pUtils)
    pUtils->UnpackGPValue(pParam, &ipVal);
  else
    pParam->get_Value(&ipVal);
  return std::make_pair(gpvalue2any(ipVal), tools::toUtf8(name));
}

static bool r2param(SEXP r, IGPParameter* pParam)
{
  CComPtr<IGPValue> ipVal;
  pParam->get_Value(&ipVal);
  return any2gpvalue(r, ipVal);
}

extern const rconnect_interface* current_connect;
int execute_tool2(const wchar_t* script_path, IArray* pParameters)
{
  if (current_connect == 0 || pParameters == 0 || script_path == 0)
    return 0;

  _bstr_t file_path(script_path);

  long nParams = 0;
  pParameters->get_Count(&nParams);

  bool ok = true;
  int errorOccurred = 0;

  if (file_path.length())
  {
    CComPtr<IGPUtilities> ipGPUtil; ipGPUtil.CoCreateInstance(CLSID_GPUtilities);

    std::vector< CAdapt<CComPtr<IGPParameter> > > return_params;
    //ipParameters->get_Count(&n);
    tools::protect pt;
    const char env_name[] = ".gptool";
    SEXP tool_env = pt.add(Rf_allocSExp(ENVSXP));
    Rf_defineVar(Rf_install(env_name), tool_env, R_GlobalEnv);
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
        std::pair<SEXP, std::string> p = param2r(ipGPUtil, ipParam);
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

      SEXP p1 = tools::nameIt(tools::newVal(in_params), in_params_names);
      SEXP p2 = tools::nameIt(tools::newVal(out_params), out_params_names);

      Rf_defineVar(Rf_install("scriptfile"), pt.add(tools::newVal(file_path)), tool_env);
      Rf_defineVar(Rf_install("params_in"),  p1, tool_env);
      Rf_defineVar(Rf_install("params_out"), p2, tool_env);
    }

/*    const static wchar_t eval_str[] = L".arc$result<-local({"
      L"en<-new.env(hash=TRUE);"
      L"eval(parse(file=scriptfile), envir=en);"
      L"tool_exec<-get('tool_exec',en);"
      L"tool_exec(params_in, params_out)"
      L"},envir=list('scriptfile'=.arc$scriptfile,'params_in'=.arc$params_in,'params_out'=.arc$params_out))";
      */
    const static wchar_t eval_str2[] = 
      L".gptool$result<-local({"
      L"eval(parse(file=.gptool$scriptfile));"
      L"do.call('tool_exec', args=list(params_in, params_out))"
      L"},envir=list(params_in=.gptool$params_in,params_out=.gptool$params_out))";
    ok = current_connect->eval_one(eval_str2) == 1;
    current_connect->print_out(NULL, -1);
    if (ok && !return_params.empty())
    {
      SEXP ret = Rf_findVarInFrame3(tool_env, Rf_install("result"), TRUE);
      tools::vectorGeneric ret_out(ret == R_UnboundValue ? R_NilValue : ret);
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
    }

    SEXP expr = pt.add(Rf_lang2(Rf_install("rm"), tools::newVal(env_name)));
    R_tryEvalSilent(expr, R_GlobalEnv, NULL);
    pt.release();
    R_gc();
  }
  return ok ? 0 : 1;
}
