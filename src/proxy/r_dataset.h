#pragma once
#include "Rtl.h"
#include <memory>
class dataset : public rtl::object<dataset>
{
protected:
public:
  static const char* class_name;

  std::shared_ptr<arcobject::dataset_handle> m_dataset;
  SEXP get_type();
  SEXP open(SEXP rPath);
  BEGIN_CALL_MAP(dataset)
   {"dataset.create", (DL_FUNC) R_fn1<rtl::createT<dataset> >, 1},
   {"dataset.open",   (DL_FUNC) R_fn2<rtl::call_1<dataset, &dataset::open> >, 2},
   {"dataset.type",   (DL_FUNC) R_fn1<rtl::call_0<dataset, &dataset::get_type> >, 1},
   {"dataset.is_table", (DL_FUNC)R_fn1<rtl::call_0<dataset, &dataset::is_table> >, 1},
   {"dataset.is_feature_class", (DL_FUNC) R_fn1<dataset::is_feature_class>, 1},
  END_CALL_MAP()

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
    dataset* ds = rtl::getCObject<dataset>(from);
    if (ds)
    {
      if (rtl::createT<T>(rHost))
      {
        auto pThis = rtl::getCObject<T>(rHost);
        pThis->m_dataset = ds->m_dataset;
      }
    }
    return rHost;
  }
};

class table: public dataset
{
protected:
public:
  static const char* class_name;

  SEXP get_fields();
  //SEXP select2(SEXP args);
  SEXP select(SEXP fields, SEXP args);

  BEGIN_CALL_MAP(table)
   {"table.create",      (DL_FUNC)R_fn1<rtl::createT<table> >, 1},
   {"table.create_from", (DL_FUNC)R_fn2<dataset::create_from<table> >, 2},
   {"table.fields",      (DL_FUNC)R_fn1<rtl::call_0<table, &table::get_fields> >, 1},
   {"table.select",      (DL_FUNC)R_fn3<rtl::call_2<table, &table::select> >, 3},
  END_CALL_MAP()
  BEGIN_EXTERNAL_MAP(table)
   //{"table.select2",     (DL_FUNC)R_fnExt<rtl::ext_N<table, &table::select2> >, 5},
  END_EXTERNAL_MAP()
private:

};

class feature_class: public table
{
public:
  static const char* class_name;

  SEXP get_shape_info();
  SEXP get_extent();
  BEGIN_CALL_MAP(table)
   {"feature_class.create",      (DL_FUNC)R_fn1<rtl::createT<feature_class> >, 1},
   {"feature_class.create_from", (DL_FUNC)R_fn2<dataset::create_from<feature_class> >, 2},
   {"feature_class.shape_info",  (DL_FUNC)R_fn1<rtl::call_0<feature_class, &feature_class::get_shape_info> >, 1},
   {"feature_class.extent",      (DL_FUNC)R_fn1<rtl::call_0<feature_class, &feature_class::get_extent> >, 1},
  END_CALL_MAP()
};

class container : public dataset
{
public:
  static const char* class_name;

  SEXP get_children();
  SEXP get_extent();
  SEXP get_sr();

  BEGIN_CALL_MAP(container)
    {"container.create", (DL_FUNC)R_fn1<rtl::createT<container> >, 1},
    {"container.create_from", (DL_FUNC)R_fn2<dataset::create_from<container> >, 2 },
    {"container.children", (DL_FUNC)R_fn1<rtl::call_0<container, &container::get_children> >, 1 },
    {"featuredataset.extent", (DL_FUNC)R_fn1<rtl::call_0<container, &container::get_extent> >, 1 },
    {"featuredataset.sr", (DL_FUNC)R_fn1<rtl::call_0<container, &container::get_sr> >, 1 },
  END_CALL_MAP()
};
