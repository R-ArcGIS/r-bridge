#pragma once

/***
* to run R code
* implement 'rconnect_interface' class
* LoadLibrary() and find 'Rconnect' exported function
* register new interface
* execute R code
* disconnect and restore prev interface

*** example
HMODULE h = LoadLibrary("rarcproxy.dll")
fnRconnect fRconnect = (fnRconnect)::GetProcAddress(h, "Rconnect");
rconnect_interface* old = (fRconnect)(&instace);
instace.exec_gptool(L"c:\\my_script.R, ipParameters);
(fRconnect)(old);
FreeLibrary(h);
***/

struct IArray;
class rconnect_interface;
typedef rconnect_interface* (*fnRconnect)(rconnect_interface*);

class rconnect_interface
{
protected:
  rconnect_interface() : gp_tool_exec(NULL) {}
public:
  //from proxy
  virtual bool isCancel() const = 0;
  // virtual void busy() const {} 
  virtual void print_out(const wchar_t *str, int e) const = 0;

#ifdef DESKTOP10 //depricated
  virtual void foo0(const wchar_t*) const {}
  virtual void foo1(const wchar_t*) {}
#endif

  virtual void progress(int pos) const {}
  virtual void progress_title(const wchar_t *title) const {}

#ifdef DESKTOP10 //depricated
  const wchar_t* foo2(void) const { return 0; }
  inline int     foo3(const wchar_t*) const { return 0; }
#endif

  inline int exec_gptool(const wchar_t* script_path, IArray* pParameters) const
  {
    if (gp_tool_exec)
      return (gp_tool_exec)(script_path, pParameters);
    return 0;
  }

  typedef int (*fn_gptool_exec)(const wchar_t* script_path, IArray* pParameters);
private:
  //initialized by Rconnect()
#ifdef ADD_FRIEND
  friend rconnect_interface* Rconnect(rconnect_interface*);
#endif

#ifdef DESKTOP10
  void *padding[2];
#endif
  fn_gptool_exec gp_tool_exec;
};
