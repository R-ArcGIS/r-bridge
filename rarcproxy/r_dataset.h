#pragma once
#include "Rtl.h"

class dataset : public rtl::object<dataset>
{
protected:
public:
  static const char* class_name;

  CComPtr<IDataset> m_ipDataset;
  SEXP get_type();
  SEXP open(SEXP rPath);
  BEGIN_CALLMETHOD(dataset)
   {"dataset.create", (DL_FUNC) R_fn1<rtl::createT<dataset> >, 1},
   {"dataset.open",   (DL_FUNC) R_fn2<rtl::call_1<dataset, &dataset::open> >, 2},
   {"dataset.type",   (DL_FUNC) R_fn1<rtl::call_0<dataset, &dataset::get_type> >, 1},
   {"dataset.is_table", (DL_FUNC)R_fn1<rtl::call_0<dataset, &dataset::is_table> >, 1},
   {"dataset.is_feature_class", (DL_FUNC) R_fn1<dataset::is_feature_class>, 1},
  END_CALLMETHOD()

  SEXP is_table()
  {
    return tools::newVal(as_table(m_ipDataset, NULL));
  }
  static SEXP is_feature_class(SEXP s4)
  {
    dataset *ptr = rtl::getCObject<dataset>(s4);
    return tools::newVal(as_feature_class(ptr->m_ipDataset, NULL));
  }

  static bool as_table(IDataset* pDataset, ITable** ppTable)
  {
    CComQIPtr<IDisplayTable> ipDT(pDataset);
    CComQIPtr<ITable> ipTable;
    if (ipDT)
      ipDT->get_DisplayTable(&ipTable);
    else
      ipTable = pDataset;

    if (ppTable)
      ipTable.CopyTo(ppTable);
    return ipTable != NULL;
  }
  static bool as_feature_class(IDataset* pDataset, IFeatureClass** ppFC)
  {
    CComQIPtr<IFeatureClass> ipFC(pDataset);
    if (!ipFC)
    {
      CComQIPtr<IGeoFeatureLayer> ipFL(pDataset);
      if (ipFL)
        ipFL->get_DisplayFeatureClass(&ipFC);
    }
    if (ppFC)
      ipFC.CopyTo(ppFC);
    return ipFC != NULL;
  }
};

class table: public dataset
{
  CComPtr<ITable> m_ipTable;
protected:
  ITable* fix_table()
  {
    if (!m_ipTable)
      as_table(m_ipDataset, &m_ipTable);
    return m_ipTable;
  }
public:
  static const char* class_name;

  static SEXP create_from(SEXP rHost, SEXP from);

  SEXP get_fields();
  SEXP select(SEXP add_geometry, SEXP args);

  BEGIN_CALLMETHOD(table)
   {"table.create", (DL_FUNC)     R_fn1<rtl::createT<table> >, 1},
   {"table.create_from", (DL_FUNC)R_fn2<table::create_from>, 2},
   {"table.fields", (DL_FUNC)     R_fn1<rtl::call_0<table, &table::get_fields> >, 1},
   {"table.select", (DL_FUNC)     R_fn3<rtl::call_2<table, &table::select> >, 3},
  END_CALLMETHOD()
};

class feature_class: public table
{
  CComPtr<IFeatureClass> m_ipFC;
  IFeatureClass* fix_fc()
  {
    if (!m_ipFC)
      as_feature_class(m_ipDataset, &m_ipFC);
    return m_ipFC;
  }
public:
  static const char* class_name;

  //bool create(SEXP rHost);
  static SEXP create_from(SEXP rHost, SEXP from);
  SEXP get_shape_info();
  SEXP get_extent();
  BEGIN_CALLMETHOD(table)
   {"feature_class.create",      (DL_FUNC)R_fn1<rtl::createT<feature_class> >, 1},
   {"feature_class.create_from", (DL_FUNC)R_fn2<feature_class::create_from>, 2},
   {"feature_class.shape_info",  (DL_FUNC)R_fn1<rtl::call_0<feature_class, &feature_class::get_shape_info> >, 1},
   {"feature_class.extent",      (DL_FUNC)R_fn1<rtl::call_0<feature_class, &feature_class::get_extent> >, 1},
  END_CALLMETHOD()
};
