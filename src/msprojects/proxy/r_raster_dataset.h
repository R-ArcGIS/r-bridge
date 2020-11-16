#pragma once
//#include "r_dataset.h"
#include "r_feature_class.h"

namespace rd {
/*template <class T, SEXP(T::*fn)(SEXP)>
inline SEXP call_S0(SEXP self)
{
  T* obj = rtl::getCObject<T>(self);
  ATLASSERT(obj);
  if (obj) return (obj->*fn)(self);
  return R_NilValue;
}*/

class raster_dataset : public dataset
{
public:
  static const char* class_name;

  BEGIN_CALL_MAP(raster_dataset)
    {"raster_dataset.create_from", (DL_FUNC)(FN2)fn::R<decltype(&raster_dataset::create_from), &raster_dataset::create_from, SEXP, SEXP>, 2 },
  END_CALL_MAP()
private:
  static SEXP create_from(SEXP rHost, SEXP from);
};

class raster_mosaic_dataset : public feature_class
{
public:
  static const char* class_name;

  BEGIN_CALL_MAP(raster_mosaic_dataset)
    {"raster_mosaic_dataset.create_from", (DL_FUNC)(FN2)fn::R<decltype(&raster_mosaic_dataset::create_from), &raster_mosaic_dataset::create_from, SEXP, SEXP>, 2},
  END_CALL_MAP()
private:
  static SEXP create_from(SEXP rHost, SEXP from);
  //SEXP get_fields() override;
  //SEXP select2(SEXP fields, SEXP args) override;
};
}
