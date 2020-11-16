#include "stdafx.h"
#include "r_feature_class.h"
#include "misc.h"

using namespace rd;
const char* feature_class::class_name = "++feature_class++";

SEXP feature_class::get_shape_info()
{
  //if (!m_dataset->is_fc())
  //  return error_Ret("arc.feature");
  auto v = m_dataset->get_shape_info();
  return forward_from_keyvalue_variant(v);
}
