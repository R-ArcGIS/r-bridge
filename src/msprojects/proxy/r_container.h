#pragma once
#include "r_dataset.h"
namespace rd {
class container : public dataset
{
public:
  static const char* class_name;

  SEXP get_children();

  BEGIN_CALL_MAP(container)
  //{"container.create", (DL_FUNC)R_fn1<rtl::createT<container> >, 1},
  {"container.create_from", (DL_FUNC)R_fn2<dataset::create_from<container> >, 2 },
  {"container.children", (DL_FUNC)R_fn1<rtl::call_0<container, &container::get_children> >, 1 },
  END_CALL_MAP()
};
}
