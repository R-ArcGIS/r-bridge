#pragma once

/***
* to run R code
* implement 'rconnect_interface' class
* LoadLibrary() and find 'Rconnect' exported function
* register new interface
* execute R code
* disconnect and restore prev interface

HMODULE h = LoadLibrary("rarcproxy.dll")
fnRconnect fRconnect = (fnRconnect)::GetProcAddress(h, "Rconnect");
rconnect_interface* old = (fRconnect)(&instace);
instace.eval_one(L"print('hello R world')");
(fRconnect)(old);
FreeLibrary(h);
***/

struct IArray;
class rconnect_interface;
typedef rconnect_interface* (*fnRconnect)(rconnect_interface*);

class rconnect_interface
{
protected:
  rconnect_interface() : gp_tool_exec(NULL)
    //,do1line(NULL), doNOP(NULL)
    {}
public:
  //from proxy
  virtual bool isCancel() const = 0;
  // virtual void busy() const {} 
  virtual void print_out(const wchar_t *str, int e) const = 0;

  virtual void progress(int pos) const{}
  virtual void progress_title(const wchar_t *title) const{}
#if 0 //depricated
  virtual void show_help(const wchar_t *url) const = 0;
  virtual void new_prompt(const wchar_t *prompt){}
  typedef const void* (*fnNOP)(void*, int);
  fn_do1line do1line;
  fnNOP doNOP;
  const void* nop(void*, int) const { return 0; }

  inline int eval_one(const wchar_t* cmd) const
  {
    if (do1line)
      return (do1line)(cmd);
    return 0;
  }
#endif
  inline int exec_gptool(const wchar_t* script_path, IArray* pParameters) const
  {
    if (gp_tool_exec)
      return (gp_tool_exec)(script_path, pParameters);
    return 0;
  }

  typedef int (*fn_gptool_exec)(const wchar_t* script_path, IArray* pParameters);
  typedef int (*fn_do1line)(const wchar_t*);
private:
  //initialized by Rconnect()
#ifdef ADD_FRIEND
  friend rconnect_interface* Rconnect(rconnect_interface*);
#endif
  fn_gptool_exec gp_tool_exec;
};
