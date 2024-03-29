#include "stdafx.h"
#include "tchannel.h"
#include "tools.h"
#include "rconnect_interface.h"
#include <process.h>
//#include <thread>
//#include <future>

#include <filesystem>
#if _HAS_CXX17
namespace fs = std::filesystem;
#else
namespace fs = std::tr2::sys;
#endif

DWORD g_main_TID = 0;

//#undef ERROR

#define Win32
#include <Rembedded.h>
#include <R_ext/RStartup.h>
#include <R_ext/Parse.h>

#include <string>
#include <sstream>

extern "C"
{
  __declspec(dllimport) extern int UserBreak;
  __declspec(dllimport) extern int R_SignalHandlers;
  void run_Rmainloop();
}

//extern char dllName[MAX_PATH];

const rconnect_interface* current_connect();

bool g_InProc = false;
extern fn_api get_api;

namespace GNU_GPL
{
  //private:
  struct local
  {
    static void run_loop(void*)
    {
      if (!initialize(/**(bool*)b)*/true))
      {
        tchannel::singleton().from_thread.push_back(tchannel::value{ tchannel::value_type::R_PROMPT, (std::wstring*)nullptr });
        return;
      }
      try {
        ::run_Rmainloop();
      }
      catch (...)
      {
        ATLASSERT(0);//Not good
      }
    }
    //public:
    static void showMessage(const char* msg)
    {
      ::MessageBox(nullptr, fromUtf8(msg).c_str(), nullptr, MB_OK);
    }
    static void busy(int which)
    {
      //if (HIWORD(GetAsyncKeyState(VK_ESCAPE)))
      //    UserBreak = 1;
      //::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT)));
    }

    static void writeOutEx(const char* bufUtf8, int len, int otype)
    {
      if (len == 0) return;
      auto connect = current_connect();
      if (connect != nullptr)
      {
        if (otype == 1) otype = 0;//R message()
        ATLASSERT(otype == 0);//warning = 2 and we handle then in arc_warning()
        if (g_main_TID != GetCurrentThreadId())
        {
          //while(g_channel.from_thread.size() > 1000)
          //  Sleep(10);
          try {
            std::wstring data_str(1, otype ? L'2' : L'0');
            data_str.append(fromUtf8(bufUtf8));
            tchannel::singleton().from_thread.push_back(tchannel::value{ tchannel::value_type::R_TXT_OUT,data_str });
          }
          catch (...)
          {
          }
        }
        else
          connect->print_out(fromUtf8(bufUtf8).c_str(), otype);
      }
      if (otype)
      {
        ::OutputDebugStringA(bufUtf8);
      }
    }

    static ParseStatus pre_eval2arc_cmd(const char* statement, int id)
    {
      tools::protect pt;
      SEXP cmd = pt.add(tools::newVal(statement));
      //pre-parse multiline
      ParseStatus status = PARSE_NULL;
      SEXP expr_cmd = pt.add(::R_ParseVector(cmd, -1, &status, R_NilValue));
      if (status == PARSE_INCOMPLETE)
        return status;

      std::string cmd_name(".arc_cmd");
      cmd_name += std::to_string(id);
      //set unique .arc_cmdXX string
      //then readConsole() will pick it in the main thread
      //Rf_defineVar(Rf_install(cmd_name.c_str()), cmd, R_GlobalEnv);
      ::Rf_defineVar(::Rf_install(cmd_name.c_str()), expr_cmd, R_GlobalEnv);
      return status;
    }

#if R_VERSION >= 262656
    static int readConsole(const char* prompt, unsigned char* buf, int len, int addtohistory)
#else
    static int readConsole(const char* prompt, char* buf, int len, int addtohistory)
#endif
    {
      tchannel& channel = tchannel::singleton();
      channel.from_thread.push_back(tchannel::value(tchannel::value_type::R_PROMPT, fromUtf8(prompt)));

      if (xwait(channel.to_thread.handle(), 0) == -1)
        return 0;

      if (tchannel::value p; channel.to_thread.pop(p, 0))
      {
        ATLASSERT(p.type == tchannel::value_type::R_CMD);
        if (int id = p.cmd_id(); id > 0)
        {
          /*static const char err_func[] = "function(e, calls, t){"
                   "gc();dcall<-deparse(calls)[1L]\n"
                   "if(dcall=='parse(text = .arc_cmd)'||dcall=='eval(expr, envir, enclos)')\n"
                   "  msg<-paste('Error: ', geterrmessage(), '\\n', sep='')\n"
                   "else msg<-paste('Error in ', dcall,': ', geterrmessage(), '\\n', sep='')\n"
                   "invisible(.Call('arc_error', msg, PACKAGE='rarcproxy_pro'))}";*/
          static const char err_func2[] = "function(e, calls){"
            "trace.back <- lapply(calls, deparse)\n"
            "if (trace.back[1] == 'eval') msg <- conditionMessage(e) "
            "else if (trace.back[1] == '') msg<-conditionMessage(e) "
            "else msg <-paste0('Error in ', trace.back[1], ' : ', conditionMessage(e))\n"
            "invisible(.Call('arc_error', msg, PACKAGE='" DLL_NAME_STR "'))}";
          std::ostringstream eval_cmd;
          eval_cmd <<
            "tryCatch("
            "withRestarts("
            "withCallingHandlers("
            //"eval(parse(text=.arc_cmd" << id << ")),"
            "eval(.arc_cmd" << id << "),"
            "error=function(e)invokeRestart('big.bada.boom', e, sys.call(sys.parent()-2L))," //sys.function(sys.parent()-4L)
            "warning=function(w){.Call('arc_warning', conditionMessage(w), PACKAGE='" DLL_NAME_STR "'); invokeRestart('muffleWarning')}"
            "),"
            "big.bada.boom=" << err_func2 <<
            "), finally=rm(.arc_cmd" << id << ", envir=.GlobalEnv)"
            ")\n";
          auto eval = eval_cmd.str();
          strncpy_s((char*)buf, len, eval.c_str(), eval.length());
        }
        else //empty command
        {
          buf[0] = '\n';
          buf[1] = 0;
        }
        return 1;
      }
      return 0;
    }

    static int yes_no_cancel(const char* msg)
    {
      return ::MessageBox(nullptr, fromUtf8(msg).c_str(), L"R scripting", MB_YESNOCANCEL);
    }

    static void myCallBack()
    {
      /// called during i/o, eval, graphics in ProcessEvents
      //OutputDebugStringA(".");
      if (UserBreak == 0 && isCancel())
      {
        UserBreak = 1;
      }
    }

    //--------------
    static ParseStatus evaluate_string(std::string exec_code)
    {
      //std::string exec_code(statement);

      //remove \r
      size_t pos = 0;
      while ((pos = exec_code.find('\r', pos)) != std::string::npos)
        exec_code.at(pos++) = ' ';

      ParseStatus status;
      SEXP cmdSexp = nullptr;
      tools::protect pt;
      cmdSexp = pt.add(tools::newVal(exec_code));
      SEXP env = R_GlobalEnv;

      SEXP cmdexpr = pt.add(R_ParseVector(cmdSexp, -1, &status, R_NilValue));
      if (status != PARSE_OK)
        return PARSE_NULL;

      int eError = 0;
      SEXP res = nullptr;
      if (TYPEOF(cmdexpr) == EXPRSXP)
      {
        for (int i = 0, n = Rf_length(cmdexpr); i < n; i++)
        {
          res = R_tryEval(VECTOR_ELT(cmdexpr, i), env, &eError);
          if (eError)
          {
            break;
          }
          //Rf_PrintValue(res);
        }
      }
      else
        res = R_tryEval(cmdexpr, env, &eError);

      if (eError)
      {
        std::string error;
        tools::copy_to(res, error);
        return PARSE_NULL;
      }

      return status;
    }

    static bool process_message(tchannel::value& p)
    {
      if (tchannel::try_process(p))
        return false;

      switch (p.type)
      {
      case tchannel::value_type::FN_CALL:
      {
        if (const auto* it = (fn*)p.data(); it != nullptr)
        {
          tchannel::singleton().to_thread.push_back(tchannel::value(tchannel::value_type::FN_CALL_RET, it->call()));
        }
      }break;
      case tchannel::value_type::R_PROMPT:
      {
        //ATLASSERT(current_connect);
        //const_cast<rconnect_interface*>(current_connect)->new_prompt(p.data_str->c_str());
      }return true;//PARSE_OK;

      case tchannel::value_type::R_TXT_OUT:
      {
        auto str = p.data_str();
        ATLASSERT(current_connect());
        if (const wchar_t* ptr = str.empty() ? nullptr : str.data(); ptr != nullptr)
          current_connect()->print_out(ptr + 1, ptr[0] - L'0');
      }break;
      default:
        ATLASSERT(0); //unknown command???
        return true;//PARSE_ERROR;
      }
      return false;
    }

    static inline int xwait(HANDLE h1, HWND hw = (HWND)-1)
    {
      DWORD n = 1;
      HANDLE h[] = { h1 };

      DWORD wakemask = QS_POSTMESSAGE | QS_TIMER | QS_PAINT |
        //QS_KEY|   
        QS_MOUSE | QS_POSTMESSAGE | QS_SENDMESSAGE;

      while (true)
      {
        DWORD dw = ::MsgWaitForMultipleObjects(n, h, FALSE, INFINITE, wakemask);
        if ((dw - WAIT_OBJECT_0) < n)
          return dw - WAIT_OBJECT_0;
        dw = ::WaitForMultipleObjects(n, h, FALSE, 0);
        if ((dw - WAIT_OBJECT_0) < n)
          return dw - WAIT_OBJECT_0;

        MSG msg;
        while (::PeekMessage(&msg, hw, 0, 0, PM_REMOVE))
        {
          if (msg.message == WM_QUIT)
            return -1;
          ::DispatchMessage(&msg);
        }
      }
    }

    // get max phisical memory
    static DWORD maxRamMemory()
    {
      static const DWORDLONG BYTES_IN_MB = 1024UL * 1024UL;

      MEMORYSTATUSEX memStatus;
      memStatus.dwLength = sizeof(memStatus);
      ::GlobalMemoryStatusEx(&memStatus);
      //DWORDLONG virtualMem = memStatus.ullTotalVirtual;
      DWORDLONG physicalMem = memStatus.ullTotalPhys;
#ifndef _WIN64
      //3G max for 32bit app
      physicalMem = std::min(3UL * 1024UL * BYTES_IN_MB, physicalMem);
#endif
      return (DWORD)(physicalMem / BYTES_IN_MB);
    }

    static bool initialize(bool bInteractive = true)
    {
      const char* dllVer = getDLLVersion();
      if (!dllVer)
        return false;
      //check major relese
      if (dllVer[0] != R_MAJOR[0])
        return false;
      // strict check
      // if (strncmp(dllVer, RVERSION_DLL_BUILD, 3) != 0)
      //   return false;

      //char tmp[MAX_PATH] = {0};
      //GetModuleFileNameA(NULL, tmp, _countof(tmp));

      R_SignalHandlers = 0;

      structRstart Rst;
      memset(&Rst, 0, sizeof(Rst));

      R_setStartTime();
      R_DefParams(&Rst);

      wchar_t buf[MAX_PATH] = { 0 };

      //R_HOME is already set by ArcGIS, so use it
      static std::string rhome;
      if (::GetEnvironmentVariable(L"R_HOME", buf, _countof(buf)) > 0)
        rhome = toUtf8(buf);
      else
        rhome = get_R_HOME();

      //propagate R_MAX_MEM_SIZE to R
      std::string mem_str;//using "--max-mem-size=" in R_set_command_line_arguments() is not working
      char bufA[MAX_PATH] = { 0 };
      auto n = (size_t)::GetEnvironmentVariableA("R_MAX_MEM_SIZE", bufA, _countof(bufA));
      if (n > 0)
      {
        const auto sufix = bufA[n - 1];
        if (sufix == 'G')
        {
          bufA[n - 1] = 0;
          mem_str = bufA;
          mem_str += "*1048576L";//"*1073741824L";
        }
        else if (sufix == 'M')
        {
          bufA[n - 1] = 0;
          mem_str = bufA;
          //mem_str += "*1048576L";
        }
        else if (sufix == 'K' || sufix == 'k')
        {
          bufA[n - 1] = 0;
          mem_str = bufA;
          mem_str += "/1024L";
        }
        else
        {
          mem_str = bufA;
          mem_str += "/1048576L";
        }
      }
      else
      {
        //use all phisical memory. virtal memory is too slow
        mem_str = std::to_string(maxRamMemory());
      }

      //WIN32
      Rst.rhome = (char*)rhome.c_str();
      Rst.home = getRUser();
      Rst.CharacterMode = LinkDLL; //RGui
      Rst.ReadConsole = &local::readConsole;//NULL;
      Rst.WriteConsole = nullptr;//writeOut;
      Rst.WriteConsoleEx = &local::writeOutEx;
      Rst.CallBack = local::myCallBack;
      Rst.ShowMessage = local::showMessage;
      Rst.YesNoCancel = yes_no_cancel;
      Rst.Busy = busy;

      Rst.R_Quiet = (Rboolean)bInteractive ? TRUE : FALSE;
      //Rst.RestoreAction = SA_RESTORE;
      Rst.SaveAction = SA_NOSAVE;
      //WIN32
      Rst.R_Interactive = (Rboolean)TRUE;// sets interactive() to eval to false 
      R_SetParams(&Rst);

      std::wstring rhome_bin(fromUtf8(Rst.rhome));
#if !defined(_WIN64)
      rhome_bin += L"\\bin\\i386";
#else
      rhome_bin += L"\\bin\\x64";
#endif
      wchar_t cur_dir[MAX_PATH + 1] = { 0 };
      ::GetCurrentDirectory(MAX_PATH, cur_dir);
      ::SetCurrentDirectory(rhome_bin.c_str());
      //fix loading stats.dll, Rlapack.dll require in dlls search
      n = (size_t)::GetEnvironmentVariable(L"PATH", nullptr, 0);
      //if (_wgetenv_s(&n, nullptr, 0, L"PATH") == 0 && n > 0)
      if (n > 0)
      {
        std::unique_ptr<wchar_t[]> buffer(new wchar_t[n + 1]);
        //_wgetenv_s(&n, buffer.get(), n, L"PATH");
        ::GetEnvironmentVariable(L"PATH", buffer.get(), (DWORD)n);
        std::wstring path(buffer.get());
        buffer.reset();
        if (path.find(rhome_bin) == std::wstring::npos)
        {
          path += L';';
          path += rhome_bin;
          //::AddDllDirectory(fromUtf8(rhome_bin.c_str()).c_str());
          //_wputenv_s(L"PATH", path.c_str());
          ::SetEnvironmentVariable(L"PATH", path.c_str());
        }
      }

      //char* av[] = {"arcgis.rproxy", "--interactive"};//, "--no-readline", "--vanilla"};
      const char* av[] = { "arcgis:" DLL_NAME_STR, "--no-readline", "--vanilla" };
      //char* av[] = {"arcgis.rproxy", "--no-readline", "--no-save", "--no-restore"};
      //R_set_command_line_arguments(_countof(av), av);
      //bool ret = Rf_initEmbeddedR(_countof(av), av) == 1;
      R_set_command_line_arguments(_countof(av), (char**)av);
      //once = 1;
      HMODULE hGA = ::LoadLibrary(L"Rgraphapp.dll");
      if (hGA)
      {
        typedef int (*fn_GA_initapp)(int, char**);
        fn_GA_initapp pGA = (fn_GA_initapp) ::GetProcAddress(hGA, "GA_initapp");
        (pGA)(0, 0);
        //GA_initapp(0,0);
      }

      FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
      readconsolecfg();

      setup_Rmainloop();
      if (cur_dir[0])
        ::SetCurrentDirectory(cur_dir);

      //if (ret)
      {
        wchar_t dllName[MAX_PATH] = { 0 };
        extern HMODULE hDllHandle;
        ::GetModuleFileName(hDllHandle, dllName, _countof(dllName));
        fs::path path(dllName);
        auto pkg_path = path.parent_path().parent_path().parent_path().parent_path().generic_u8string();
        //std::replace(pkg_path.begin(), pkg_path.end(), '\\', '/');

        //verify it
        Rf_defineVar(Rf_install(".arcgisbinding_inproc"), tools::newVal(g_InProc), R_GlobalEnv);
        std::string load_pkg("library(arcgisbinding,lib.loc='");
        load_pkg += (const char*)pkg_path.c_str();
        load_pkg += "')";
        auto x = evaluate_string(load_pkg.c_str());
        ATLASSERT(x);
        if (x == 0)
          return false;
        load_pkg = "memory.limit(" + mem_str + ")";
        x = evaluate_string(load_pkg.c_str());
        //in_memsize(tools::newVal(32*1024*1024));

        //evaluate_string("options(browser = function(url) {.C('arc_browsehelp', url)})");
        set_current_thread_name("R interpreter");
      }
      return true;
    }

    static inline void set_current_thread_name(LPCSTR name)
    {
#if 1 || defined(_DEBUG)
      static const DWORD MS_VC_EXCEPTION = 0x406D1388;
      struct tagTHREADNAME_INFO
      {
        DWORD dwType;     // Must be 0x1000.
        LPCSTR szName;    // Pointer to name (in user addr space).
        DWORD dwThreadID; // Thread ID (-1=caller thread).
        DWORD dwFlags;    // must be zero.
      } info = { 0x1000, name, (DWORD)-1, 0 };
      __try
      {
        ::RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)& info);
      }
      __except (EXCEPTION_CONTINUE_EXECUTION) {}
#endif
    }

  };
#pragma optimize( "", off )
  bool initInProcInterpreter()
  {
    ATLASSERT(get_api != nullptr);
    if (get_api == nullptr)
      return false;

#if (R_VERSION >= R_Version(4, 0, 0) && R_VERSION < R_Version(4, 2, 0))
      const char* dllVer = getDLLVersion();
      if (!dllVer)
        return false;
      //R > 4.0.2 && R < 4.2.0 are not supported
      if (dllVer[2] > '0' || (dllVer[2] == '0' && dllVer[4] > '2'))
      {
        const wchar_t msg[] = L"This version of R does not support running geoprocessing tools.\nUpdate your R version to 4.2.0 or later.";
        ::MessageBox(NULL, msg, NULL, MB_OK | MB_ICONERROR);
        return false;
      }
#endif
    if (g_main_TID != 0) return false;

    g_main_TID = GetCurrentThreadId();
    g_InProc = true;
    //_api = arcobject::api(g_InProc);
    if (_api == nullptr)
      _api = get_api(g_InProc);
    const auto err_msg = _api->getLastComError();
    _api->AoInitialize();

#if 1
    uintptr_t r_thread = ::_beginthread(&local::run_loop, 0, nullptr);
    if (r_thread < 0)
      return false;
#else
    static auto task = std::async(std::launch::async, []() -> void { local::run_loop(nullptr); });
#endif
    int ret = 0;
    tchannel& channel = tchannel::singleton();
    while (true)
    {
      local::xwait(channel.from_thread.handle());
      tchannel::value p;
      channel.from_thread.pop(p, 0);
      if (p.empty())
        return false;

      if (local::process_message(p))
        break;
    }

    return true;
  }
#pragma optimize( "", on )

  // this is main thread
  int do1line(const char* codeU8)
  {
    //tchannel::value _p(tchannel::R_CMD, 0);
    //std::string statement = toUtf8(code);
    static int cmd_count = 0;
    int cmd_id = 0;
    if (codeU8 != nullptr && codeU8[0] != 0)
    {
      cmd_id = ++cmd_count;
      //before process let check if wee need complete multiline code
      if (local::pre_eval2arc_cmd(codeU8, cmd_id) == PARSE_INCOMPLETE)
        return PARSE_INCOMPLETE; //return to editor and add second propmt +>
    }
    //else
    //  _p.cmd_id = 0;

    if (current_connect())
      current_connect()->print_out(L"\n", 1);

    // 1. send command to R thread
    // 2. wait for new prompt(R_PROMPT) - evaluation is done
    // 2.1 if R script calls arc-bainding(FN_CALL) then process it here (interpreter thread)
    // 3 meanwhile process ouptut messages (R_TXT_OUT)
    tchannel& channel = tchannel::singleton();
    channel.to_thread.push_back(tchannel::value{ tchannel::value_type::R_CMD, cmd_id });
    while (true)
    {
      tchannel::value p;
      try
      {
        while (!channel.from_thread.pop(p, 0))
        {
          local::xwait(channel.from_thread.handle(), 0);
        }
      }
      catch (...) { return PARSE_ERROR; }
      if (local::process_message(p))
        return PARSE_OK;
    }
  }

}

bool isCancel()
{
  auto connect = current_connect();
  if (connect != nullptr && connect->isCancel())
  {
    //UserBreak = 1;
    return true;
  }
  return false;
}
