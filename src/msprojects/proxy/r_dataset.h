#pragma once
#include "Rtl.h"
#include <memory>

namespace rd {
class dataset : public rtl::object//<dataset>
{
protected:

public:
  static const char* class_name;
  ~dataset()
  {
  }

  std::shared_ptr<arcobject::dataset> m_dataset;

  /*static const std::vector<R_CallMethodDef>& _exports()
  {
    std::vector<R_CallMethodDef> m = {
      {"dataset.create", (DL_FUNC) fn::R1<rtl::createT<dataset> >, 1},
      {"dataset.open",   (DL_FUNC) fn::R2<rtl::call_1<dataset, &dataset::open> >, 2},
    };
    return m;
  }*/

  BEGIN_CALL_MAP(dataset)
   {"dataset.create", (DL_FUNC)(FN1)fn::R<decltype(&rtl::createT<dataset>), &rtl::createT<dataset>, SEXP>, 1},
   {"dataset.open",   (DL_FUNC)(FN2)fn::R<dataset, decltype(&dataset::open), &dataset::open, SEXP>, 2},
   {"dataset.type",   (DL_FUNC)(FN1)fn::R<dataset, decltype(&dataset::get_type), &dataset::get_type>, 1},
   {"dataset.extent", (DL_FUNC)(FN1)fn::R<dataset, decltype(&dataset::get_extent), &dataset::get_extent>, 1},
   {"dataset.sr",     (DL_FUNC)(FN1)fn::R<dataset, decltype(&dataset::get_sr), &dataset::get_sr>, 1},
   {"dataset.props",  (DL_FUNC)(FN1)fn::R<dataset, decltype(&dataset::get_props), &dataset::get_props>, 1},
   {"dataset.metadata", (DL_FUNC)(FN1)fn::R<dataset, decltype(&dataset::get_metadata), &dataset::get_metadata>, 1},
   {"dataset.is_table", (DL_FUNC)(FN1)fn::R<dataset, decltype(&dataset::is_table), &dataset::is_table>, 1},
   {"dataset.is_feature_class", (DL_FUNC)(FN1)fn::R<decltype(&dataset::is_feature_class), &dataset::is_feature_class, SEXP>, 1},
  END_CALL_MAP()

protected:
  SEXP is_table()
  {
    return tools::newVal(m_dataset->is_table());
  }
  static SEXP is_feature_class(SEXP s4)
  {
    dataset *ptr = rtl::getCObject<dataset>(s4);
    return tools::newVal(ptr->m_dataset->is_fc());
  }
  template<class T>
  static SEXP create_from(SEXP rHost, SEXP from)
  {
    const auto ds = rtl::getCObject<dataset>(from);
    ATLASSERT(ds != nullptr);
    if (ds == nullptr)
      return R_NilValue;
    tools::protect pt;
    SEXP ptr = pt.add(rtl::createT<T>(rHost));
    if (TYPEOF(ptr) == EXTPTRSXP)
    {
      auto pThis = reinterpret_cast<T*>(R_ExternalPtrAddr(ptr));//rtl::getCObject<T>(rHost);
      pThis->m_dataset = ds->m_dataset;
    }
    return ptr;
  }

  SEXP get_type();
  SEXP open(SEXP rPath);
  SEXP get_extent();
  SEXP get_sr();

  SEXP get_props();
  SEXP get_metadata();
};
}
