#include "stdafx.h"
#include "tools.h"
#include "rconnect_interface.h"
#include "misc.h"
#include <memory>
#include <string>
#include <sstream>

namespace GNU_GPL
{
  int do1line(const char* codeU8);
};

#include <vector>

extern const rconnect_interface* current_connect;
int execute_tool2(const wchar_t* script_path, IArray* pParameters)
{
  if (current_connect == 0 || pParameters == 0 || script_path == 0)
    return 0;

  const rconnect_interface* connect = current_connect;
  std::wstring file_path(script_path);

  bool ok = true;
  int errorOccurred = 0;

  if (file_path.length() == 0)
    return 0;

  std::unique_ptr<arcobject::gp_execute> gp(arcobject::gp_begin_execute(connect, pParameters));

  std::string env_name(".gptool");

  static int just_count = 0;
  env_name += std::to_string(just_count++); //unique name
  tools::protect pt;
  SEXP tool_env = pt.add(Rf_allocSExp(ENVSXP));
  {
    Rf_defineVar(Rf_install(env_name.c_str()), tool_env, R_GlobalEnv);

    auto& inputs = gp->get_inputs();
    tools::listGeneric in_params(inputs.size());

    for(const auto &it : inputs)
      in_params.push_back(it.first, it.second);

    auto& outputs = gp->get_outputs();
    tools::listGeneric out_params(outputs.size());
    for(auto &it : outputs)
      out_params.push_back(it.first, it.second);

    Rf_defineVar(Rf_install("scriptfile"), tools::newVal(file_path), tool_env);
    Rf_defineVar(Rf_install("params_in"),  in_params.get(), tool_env);
    Rf_defineVar(Rf_install("params_out"), out_params.get(), tool_env);
  }

/*  const static wchar_t eval_str[] = L".arc$result<-local({"
    L"en<-new.env(hash=TRUE);"
    L"eval(parse(file=scriptfile), envir=en);"
    L"tool_exec<-get('tool_exec',en);"
    L"tool_exec(params_in, params_out)"
    L"},envir=list(scriptfile=.arc$scriptfile,params_in=.arc$params_in,params_out=.arc$params_out))";
  const static wchar_t eval_str2[] = 
    L".arc$gptool$result<-local({"
    L"eval(parse(file=.arc$scriptfile));"
    L"do.call('tool_exec', args=list(params_in, params_out))"
    L"},envir=list(params_in=.arc$params_in,params_out=.arc$params_out))";*/

  std::ostringstream eval_cmd;
  eval_cmd << env_name << "$result<-local({"
    << "eval(parse(file=" << env_name << "$scriptfile));"
    << "do.call('tool_exec', args=list(params_in, params_out))"
    << "},envir=list(params_in=" << env_name << "$params_in,params_out=" << env_name << "$params_out))";


  ok = GNU_GPL::do1line(eval_cmd.str().c_str()) == 1;

  connect->print_out(NULL, -1);

  //handle ok
  if (ok)
  {
    SEXP ret = Rf_findVarInFrame3(tool_env, Rf_install("result"), TRUE);
    //SEXP ret = Rf_findVar(Rf_install("result"), tool_env);
    tools::vectorGeneric ret_out(ret == R_UnboundValue ? R_NilValue : ret);
    for (size_t i = 0, n = ret_out.size(); i < n; i++)
    {
      //if (!r2param(ret_out.at(idx), return_params[i].m_T))
      //CComVariant v;
      VARIANT v;
      ::VariantInit(&v);
      if (!r2variant(ret_out.at(i), v))
      {
        std::wstring msg(L"failed to set output parameter[");
        msg += std::to_wstring(i);
        msg += L']';
        connect->print_out(msg.c_str(), 2);
      }
      else
        gp->update_output((int)i, v);
      ::VariantClear(&v);
    }
  }
  { //remove .gptoolNN env
    SEXP expr = pt.add(Rf_lang2(Rf_install("rm"), tools::newVal(env_name)));
    R_tryEvalSilent(expr, R_GlobalEnv, NULL);
  }
  pt.release();
  R_gc();

  return ok ? 0 : 1;
}
