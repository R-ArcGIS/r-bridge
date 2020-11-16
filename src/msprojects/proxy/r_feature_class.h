#pragma once
#include "r_table.h"

namespace rd {
class feature_class: public table
{
public:
  static const char* class_name;

  SEXP get_shape_info();
  BEGIN_CALL_MAP(table)
  //{"feature_class.create",      (DL_FUNC)fn::R1<rtl::createT<feature_class> >, 1},
  {"feature_class.create_from", (DL_FUNC)(FN2)fn::R<decltype(&dataset::create_from<feature_class>), &dataset::create_from<feature_class>, SEXP, SEXP>, 2},
  {"feature_class.shape_info",  (DL_FUNC)(FN1)fn::R<feature_class, decltype(&feature_class::get_shape_info), &feature_class::get_shape_info>, 1},
  END_CALL_MAP()
};
}
