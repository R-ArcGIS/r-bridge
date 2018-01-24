#include "stdafx.h"

#include "exporter.h"
#include "r_dataset.h"
#include "r_feature_class.h"
#include "r_container.h"
#include "r_raster_dataset.h"
#include "r_raster.h"
#include "misc.h"
#include "rconnect_interface.h"
#include <unordered_set>


#define REGISTER_CALLS(m, C){ const R_CallMethodDef* pm = C; while (pm && pm->name) m.push_back(*pm), pm++; }
#define REGISTER_CALLMETHODS(m, C) REGISTER_CALLS(m, C::get_CallMethods()) 

#define REGISTER_EXTERNAL(m, C){ const R_ExternalMethodDef* pm = C; while (pm && pm->name) m.push_back(*pm), pm++; }
#define REGISTER_EXTERNAL_METHODS(m, C) REGISTER_EXTERNAL(m, C::get_ExtMethods())


static bool register_R_API(DllInfo* dllInfo)
{
  if (_api == nullptr)
  {
    extern bool g_InProc;
    extern fn_api get_api;
    ATLASSERT(g_InProc == false && get_api != nullptr);
    if (get_api == nullptr)
      return false;
    //_api = arcobject::api(g_InProc);
    _api = get_api(g_InProc);
    const auto err_msg = _api->getLastComError();
  }

#if DEBUG && 0
  auto s = tools::newVal({L"1",L"2"});
  s = tools::newVal({"1", "2"});
  s = tools::newVal({1, 2});
  s = tools::newVal({1.1, 2.2});
  std::vector<std::wstring> vs = {L"1", L"2"};
  s = tools::newVal(vs);
  s = tools::newVal(vs[0]);
  std::vector<std::string> vs0 = {"1", "2"};
  s = tools::newVal(vs0);
  tools::nameIt(s, {"1","2"});
  tools::nameIt(s, vs);
  s = tools::newVal(vs0[0]);
#endif

//install interop
  std::vector<R_CallMethodDef> all_methods = 
  {
    {"arc_export2dataset",    (DL_FUNC) R_fn2<R_export2dataset>, 2},
    {"arc_fromWkt2P4",        (DL_FUNC) R_fn1<R_fromWkt2P4>,     1},
    {"arc_fromP42Wkt",        (DL_FUNC) R_fn1<R_fromP42Wkt>,     1},
    {"arc_error",             (DL_FUNC) arc_error,               1},
    {"arc_warning",           (DL_FUNC) arc_warning,             1},
    {"arc_getEnv",            (DL_FUNC) R_fn0<R_getEnv>,         0},
    {"arc_AoInitialize",      (DL_FUNC) R_AoInitialize,          0},
    {"arc_progress_label",    (DL_FUNC) &arc_progress_label,     1},
    {"arc_progress_pos",      (DL_FUNC) &arc_progress_pos,       1},
    {"object.release_internals", (DL_FUNC)R_fn1<rtl::release_internalsT<rtl::object> >, 1},
    {"arc_delete",            (DL_FUNC) R_fn1<R_delete>,         1},
  };
  REGISTER_CALLMETHODS(all_methods, rd::dataset);
  REGISTER_CALLMETHODS(all_methods, rd::table);
  REGISTER_CALLMETHODS(all_methods, rd::feature_class);
  REGISTER_CALLMETHODS(all_methods, rd::container);
  REGISTER_CALLMETHODS(all_methods, rd::raster_dataset);
  REGISTER_CALLMETHODS(all_methods, rd::raster_mosaic_dataset);
  REGISTER_CALLMETHODS(all_methods, rr::raster);

#if _DEBUG
  //all export method must be unique
  std::unordered_set<std::string> uniques;
  for (const auto& it : all_methods)
  {
    const auto elm = uniques.find(it.name);
    ATLASSERT(elm == uniques.end());
    uniques.insert(it.name);
  }
#endif

  R_NativePrimitiveArgType types[1] = {STRSXP};
  R_CMethodDef methodsC[] =
  {
    {NULL}
  };

  static const R_ExternalMethodDef endExt = {NULL, NULL, 0};
  std::vector<R_ExternalMethodDef> all_external_methods;
  REGISTER_EXTERNAL_METHODS(all_external_methods, rd::table);

  static const R_CallMethodDef endCall = {NULL, NULL, 0};
  all_methods.push_back(endCall);
  all_external_methods.push_back(endExt);

  int ret = R_registerRoutines(dllInfo, methodsC, &all_methods[0], NULL, &all_external_methods[0]);
  //ATLASSERT(ret);
  return true;
}

#define xANDy(x,y) x ## y
#define R_INIT_NAME(x) xANDy(R_init_,x)
extern "C" _declspec(dllexport) void R_INIT_NAME(DLL_NAME)(DllInfo *info)
{
  register_R_API(info);
}
