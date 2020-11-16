#pragma once
#include "r_dataset.h"
namespace rd {
class table: public dataset
{
protected:
public:
  static const char* class_name;

  SEXP get_fields();
  SEXP select2(SEXP fields, SEXP args);

  BEGIN_CALL_MAP(table)
  //{"table.create",      (DL_FUNC)fn::R1<rtl::createT<table> >, 1},
  {"table.create_from", (DL_FUNC)(FN2)fn::R<decltype(&dataset::create_from<table>), &dataset::create_from<table>, SEXP, SEXP>, 2},
  {"table.fields",      (DL_FUNC)(FN1)fn::R<table, decltype(&table::get_fields), &table::get_fields>, 1},
  {"table.select",      (DL_FUNC)(FN3)fn::R<table, decltype(&table::select2), &table::select2, SEXP, SEXP>, 3},
  END_CALL_MAP()
  BEGIN_EXTERNAL_MAP(table)
    //{"table.select2",     (DL_FUNC)R_fnExt<rtl::ext_N<table, &table::select2> >, 5},
  END_EXTERNAL_MAP()
private:

};
}
