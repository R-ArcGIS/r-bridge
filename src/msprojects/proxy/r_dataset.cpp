#include "stdafx.h"
#include "r_dataset.h"
#include "misc.h"
using namespace rd;

const char* dataset::class_name = "++dataset++";

SEXP dataset::open(SEXP rPath)
{
  std::wstring path;
  if (!tools::copy_to(rPath, path))
    return error_Ret("wrong path");

  //m_ipDataset.Attach(arcobject::open_dataset(path));
  m_dataset.reset(_api->open_dataset(path));
  if (m_dataset.get() == nullptr)
    return showError<true>("cannot open dataset");

  return tools::newVal(true);
}

SEXP dataset::get_type()
{
  ATLASSERT(m_dataset.get() != nullptr);
  const char* type_name = m_dataset->get_type();
  if (type_name == nullptr)
    return error_Ret("not a dataset");
  return tools::newVal(type_name);
}

SEXP dataset::get_extent()
{
  //return extent2r(m_dataset->get_extent());
  auto v = m_dataset->get_extent();
  return forward_from_keyvalue_variant(v);
}

SEXP dataset::get_sr()
{
  auto v = m_dataset->get_sr();
  return forward_from_keyvalue_variant(v);
}

SEXP dataset::get_props()
{
  auto v = m_dataset->get_properties_info();
  return forward_from_keyvalue_variant(v);
}

SEXP dataset::get_metadata()
{
  auto v = m_dataset->get_metadata_info();
  return forward_from_keyvalue_variant(v);
}
