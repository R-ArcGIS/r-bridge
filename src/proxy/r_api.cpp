#include <stdafx.h>

#include "exporter.h"
#include "r_dataset.h"
#include "misc.h"
#include "rconnect_interface.h"

#define REGISTER_CALLS(m, C){ R_CallMethodDef* pm = C; while (pm && pm->name) m.push_back(*pm), pm++; }
#define REGISTER_CALLMETHODS(m, C) REGISTER_CALLS(m, C::get_CallMethods()) 

#define REGISTER_EXTERNAL(m, C){ R_ExternalMethodDef* pm = C; while (pm && pm->name) m.push_back(*pm), pm++; }
#define REGISTER_EXTERNAL_METHODS(m, C) REGISTER_EXTERNAL(m, C::get_ExtMethods())


static bool register_R_API(DllInfo* dllInfo)
{
//install interop
  static R_CallMethodDef methods[] = 
  {
    {"arc_export2dataset",    (DL_FUNC) R_fn4<R_export2dataset>, 4},
    {"arc_fromWkt2P4",        (DL_FUNC) R_fn1<R_fromWkt2P4>,     1},
    {"arc_fromP42Wkt",        (DL_FUNC) R_fn1<R_fromP42Wkt>,     1},
    {"arc_error",             (DL_FUNC) arc_error,               1},
    {"arc_warning",           (DL_FUNC) arc_warning,             1},
    {"arc_getEnv",            (DL_FUNC) R_fn0<R_getEnv>,         0},
    {"arc_AoInitialize",      (DL_FUNC) R_AoInitialize,          0},
    {"arc_progress_label",    (DL_FUNC) &arc_progress_label,     1},
    {"arc_progress_pos",      (DL_FUNC) &arc_progress_pos,       1},
    {NULL, NULL, 0}
  };

  std::vector<R_CallMethodDef> all_methods;
  REGISTER_CALLS(all_methods, methods);
  REGISTER_CALLMETHODS(all_methods, dataset);
  REGISTER_CALLMETHODS(all_methods, table);
  REGISTER_CALLMETHODS(all_methods, feature_class);
  REGISTER_CALLMETHODS(all_methods, container);

  static R_CallMethodDef endCall = {NULL, NULL, 0};
  all_methods.push_back(endCall);

  R_NativePrimitiveArgType types[1] = {STRSXP};
  R_CMethodDef methodsC[] =
  {
    {NULL, NULL, 0, NULL,NULL}
  };

  static R_ExternalMethodDef endExt = {NULL, NULL, 0};
  std::vector<R_ExternalMethodDef> all_external_methods;
  REGISTER_EXTERNAL_METHODS(all_external_methods, table);
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

