#pragma once
#include "r_dataset.h"
namespace rd {
class table: public dataset
{
protected:
public:
  static const char* class_name;

  /*virtual */SEXP get_fields();
  /*virtual */SEXP select2(SEXP fields, SEXP args);

  BEGIN_CALL_MAP(table)
  //{"table.create",      (DL_FUNC)R_fn1<rtl::createT<table> >, 1},
  {"table.create_from", (DL_FUNC)R_fn2<dataset::create_from<table> >, 2},
  {"table.fields",      (DL_FUNC)R_fn1<rtl::call_0<table, &get_fields> >, 1},
  {"table.select",      (DL_FUNC)R_fn3<rtl::call_2<table, &select2> >, 3},
  END_CALL_MAP()
  BEGIN_EXTERNAL_MAP(table)
    //{"table.select2",     (DL_FUNC)R_fnExt<rtl::ext_N<table, &table::select2> >, 5},
  END_EXTERNAL_MAP()
private:

};
}
