#pragma once
#include "r_dataset.h"
namespace rd {
class container : public dataset
{
public:
  static const char* class_name;

  SEXP get_children();

  BEGIN_CALL_MAP(container)
  //{"container.create", (DL_FUNC)fn::R1<rtl::createT<container> >, 1},
  {"container.create_from", (DL_FUNC)(FN2)fn::R<decltype(&dataset::create_from<container>), &dataset::create_from<container>, SEXP, SEXP>, 2},
  {"container.children", (DL_FUNC)(FN1)fn::R<container, decltype(&container::get_children), &container::get_children>, 1},
  END_CALL_MAP()
};
}
