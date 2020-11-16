#include "stdafx.h"
#include "r_table.h"
#include "tools.h"
#include "misc.h"

using namespace rd;
const char* table::class_name = "++table++";

SEXP table::get_fields()
{
  //if (!m_dataset->is_table())
  //  return error_Ret("arc.table");
  auto v = m_dataset->get_fields();
  return forward_from_keyvalue_variant(v);
}
#include <unordered_set>

#if 0
void test_realloc()
{
  SEXP sx = nullptr;
  __try
  {
  for (size_t i = 0, n = 0; i < 90000000; i++)
  {
    if (i == n)
    {
      auto new_n = n == 0 ? 8 : n + (n/2);
      Rprintf("allocating : %u\n" , (unsigned int)n);
      auto s = Rf_protect(Rf_allocVector(INTSXP, new_n));
      if (sx != nullptr)
      {
        std::memcpy(INTEGER(s), INTEGER(sx), sizeof(n)*sizeof(int));
        ::R_ReleaseObject(sx);
        //R_gc();
      }
      sx = s;
      ::R_PreserveObject(sx);
      Rf_unprotect(1);
      n = new_n;
    }
    INTEGER(sx)[i] = (int)i;
  }
  }
  __finally
  {
    ::R_ReleaseObject(sx);
    R_gc();
    Rf_error("out of memory");
  }
}
#endif
#if 0
SEXP table::select(SEXP fields, SEXP args)
{
  if (!m_dataset->is_table())
    return error_Ret("arc.table");

  SEXP selected = CAR(args);args = CDR(args);
  SEXP where_str = CAR(args);args = CDR(args);
  SEXP sr = CAR(args);args = CDR(args);

  std::vector<std::wstring> fld;
  if (!tools::copy_to(fields, fld))
    return error_Ret("'fields' should be string(s)");
  bool use_selection = true;
  tools::copy_to(selected, use_selection);

  if (fld.empty())
    return error_Ret("nothing to select");

  std::wstring where_clause;
  tools::copy_to(where_str, where_clause);

  const auto sp_ref = from_sr(sr);

  std::vector<std::wstring> unique;
#if 0
  for (const auto& it : fld)
    unique.push_back(tools::tolower(it));

  auto end = std::unique(unique.begin(), unique.end());
  auto new_size = std::distance(unique.begin(), end);
  unique.resize(new_size);
#else
  std::unordered_set<std::wstring> set;
  for (const auto& it : fld)
  {
    const auto str = tools::tolower(it);
    if (set.find(str) == set.end())
    {
      set.insert(str);
      unique.push_back(it);
    }
  }

#endif
  auto cols = m_dataset->select(unique, where_clause, use_selection, sp_ref);
  if (cols.empty())
    return showError<true>("COM error");

  tools::protect pt;
  std::vector<std::pair<SEXP, std::wstring>> unique_cols;
  for (auto &it : cols)
  {
    switch (it->t)
    {
    case arcobject::column_t::eInt:
    {
      auto &data = arcobject::column_t::get_ints(*it);
      unique_cols.emplace_back(pt.add(tools::newVal(data)), it->name);
    }break;
    case arcobject::column_t::eDate:
    {
      auto &data = arcobject::column_t::get_doubles(*it);
      for (auto &d : data)
        d = ole2epoch(d);

      auto column = pt.add(tools::newVal(data));
      const static std::vector<std::string> names{ "POSIXt", "POSIXct" };
      Rf_setAttrib(column, R_ClassSymbol, pt.add(tools::newVal(names)));
      //use local time zone
      //Rf_setAttrib(column, Rf_install("tzone"), tools::newVal("UTC"));
      unique_cols.emplace_back(column, it->name);
    }
    break;
    case arcobject::column_t::eDouble:
    {
      auto &data = arcobject::column_t::get_doubles(*it);
      unique_cols.emplace_back(pt.add(tools::newVal(data)), it->name);
    }break;
    case arcobject::column_t::eString:
    {
      auto &data = arcobject::column_t::get_strings(*it);
      unique_cols.emplace_back(pt.add(tools::newVal(data)), it->name);
    }break;
    case arcobject::column_t::eGeometry_point:
    {
      bool add_wkt = (!it->sr.first.empty() || it->sr.second != 0);
      auto& pts = arcobject::column_t::get_points(*it);
      tools::listGeneric xyzm((add_wkt ? 2 : 0) + 2 + (pts[2].empty() ? 0 : 1) + (pts[3].empty() ? 0 : 1));
      xyzm.push_back(pts[0], L"x"); pts[0].clear();
      xyzm.push_back(pts[1], L"y"); pts[1].clear();
      if (!pts[2].empty())
      {
        xyzm.push_back(pts[2], L"z");
        pts[2].clear();
      }
      if (!pts[3].empty())
      {
        xyzm.push_back(pts[3], L"m");
        pts[3].clear();
      }
      if (add_wkt)
      {
        xyzm.push_back(it->sr.first, L"WKT");
        xyzm.push_back(it->sr.second, L"WKID");
      }
      unique_cols.emplace_back(pt.add(xyzm.get()), it->name);
    } break;
    case arcobject::column_t::eGeometry_polyline:
    case arcobject::column_t::eGeometry_polygon:
    case arcobject::column_t::eGeometry_multipoint:
    {
      auto& geoms = arcobject::column_t::get_shapes(*it);
      bool add_wkt = (!it->sr.first.empty() || it->sr.second != 0);
      tools::listGeneric shapes(geoms.size() + add_wkt ? 2 : 0);
      std::wstring name;
      if (it->t == arcobject::column_t::eGeometry_polyline)
        name = L"Polyline";
      else if (it->t == arcobject::column_t::eGeometry_polygon)
        name = L"Polygon";
      else if (it->t == arcobject::column_t::eGeometry_multipoint)
        name = L"Multipoint";

      for (auto &g : geoms)
      {
        if (g.empty())
          shapes.push_back(R_NilValue, name);
        else
          shapes.push_back(g, name);
        g.clear();
      }
      if (add_wkt)
      {
        shapes.push_back(it->sr.first, L"WKT");
        shapes.push_back(it->sr.second, L"WKID");
      }
      unique_cols.emplace_back(pt.add(shapes.get()), it->name);
    }break;
    default:
      ATLASSERT(0);
      break;
    }
    it.reset(nullptr);
  }
  //use same reference for duplicated columns
  tools::listGeneric column_list(fld.size());
  ATLASSERT(unique_cols.size() == unique.size());
  if (unique_cols.size() == unique.size())
  {
    for(auto it = fld.begin(), end = fld.end(); it != end; ++it)
    {
      //auto idx = std::distance(unique.begin(), std::find(unique.begin(), unique.end(), tools::tolower(*it)));
      auto idx = std::distance(unique.begin(), std::find(unique.begin(), unique.end(), *it));
      column_list.push_back(unique_cols[idx].first, it->c_str());
    }
  }
  else
  {
    for(const auto &it : unique_cols)
      column_list.push_back(it.first, it.second.c_str());
  }
  return column_list.get();
}
#endif

std::pair<std::vector<SEXP>, std::vector<std::wstring>> process_cols(tools::protect &pt, std::vector<std::unique_ptr<arcobject::column_t2>> &cols)
{
  //std::vector<std::pair<SEXP, std::wstring>> unique_cols;
  std::pair<std::vector<SEXP>, std::vector<std::wstring>> unique_cols;
  for (auto &it : cols)
  {
    auto s = pt.add(tools::newVal(it->data.first));
    if (it->data.second.vt != VT_EMPTY)
      tools::nameIt(s, tools::newVal(it->data.second));
    if (it->attr.first.vt == (VT_VARIANT|VT_VECTOR) && it->attr.second.vt == (VT_LPSTR|VT_VECTOR))
    {
      ATLASSERT(it->attr.first.capropvar.cElems == it->attr.second.calpstr.cElems);
      for (ULONG i = 0; i < it->attr.first.capropvar.cElems; i++)
      {
        Rf_setAttrib(s, Rf_install(it->attr.second.calpstr.pElems[i]), tools::newVal(it->attr.first.capropvar.pElems[i]));
      }
    }
    if (it->t == arcobject::column_t2::eDate)
    {
      Rf_setAttrib(s, R_ClassSymbol, tools::newVal({ "POSIXt", "POSIXct" }));
    }
    //unique_cols.emplace_back(s, it->name);
    unique_cols.first.emplace_back(s);
    unique_cols.second.emplace_back(std::move(it->name));
    it.reset(nullptr);
  }
  return unique_cols;
}

SEXP table::select2(SEXP fields, SEXP sargs)
{
  //if (!m_dataset->is_table())
  //  return error_Ret("arc.table");

  std::vector<std::wstring> fld;
  if (!tools::copy_to(fields, fld))
    return error_Ret("'fields' should be string(s)");

  if (fld.empty())
    return error_Ret("nothing to select");

  bool use_selection = true;
  std::wstring where_clause;
  arcobject::sr_type sp_ref(std::wstring(), 0);
  bool asWKB = false;
  bool asJson = false;
  bool asJsonSR = false;
  bool dencify = true;

  const auto args = tools::pairlist2args_map(sargs);
  const auto is_args = tools::unpack_args<false>(args,
    std::tie("selected", use_selection),
    std::tie("where_clause", where_clause),
    std::tie("sr", sp_ref),
    std::tie("asWKB", asWKB),
    std::tie("asJson", asJson),
    std::tie("asJsonSR", asJsonSR),
    std::tie("dencify", dencify));

  if (std::get<0>(is_args) == 2)
    return error_Ret("incorrect type, argument: selected");
  if (std::get<1>(is_args) == 2)
    return error_Ret("incorrect type, argument: where_clause");
  if (std::get<2>(is_args) == 2)
    return error_Ret("'asWKB' argument is not logical");
  if (std::get<3>(is_args) == 2)
    return error_Ret("'dencify' argument is not logical");

  /*
  auto it = args.find("selected");
  if (it != args.end())
    tools::copy_to(it->second, use_selection);
  it = args.find("where_clause");
  if (it != args.end())
    tools::copy_to(it->second, where_clause);

  it = args.find("sr");
  if (it != args.end())
    sp_ref = from_sr(it->second);
  else
    sp_ref.second = 0;
  it = args.find("asWKB");
  if (it != args.end() && tools::copy_to(it->second, asWKB) == false)
    return error_Ret("'asWKB' argument is not logical");

  it = args.find("dencify");
  if (it != args.end() && tools::copy_to(it->second, dencify) == false)
    return error_Ret("'dencify' argument is not logical");
  */
  std::vector<std::wstring> unique;
  std::unordered_set<std::wstring> set;
  for (const auto& it : fld)
  {
    const auto str = tools::tolower(it);
    if (set.find(str) == set.end())
    {
      set.insert(str);
      unique.push_back(it);
    }
  }

  arcobject::dataset::geometry_kind kind = arcobject::dataset::geometry_kind::eShapeBuffer;
  if (asWKB) kind = arcobject::dataset::geometry_kind::eWkb;
  else if (asJson) kind = arcobject::dataset::geometry_kind::eJson;
  else if (asJsonSR) kind = arcobject::dataset::geometry_kind::eJsonSR;

  auto cols = m_dataset->select(unique, where_clause, use_selection, sp_ref, dencify, kind);

  if (cols.empty())
    return showError<true>("COM error");

  tools::protect pt;
  auto unique_cols = process_cols(pt, cols);
  //use same reference for duplicated columns
  //tools::listGeneric column_list(fld.size());
  ATLASSERT(unique_cols.first.size() == unique.size());
  /*if (unique_cols.size() == unique.size())
  {
    for(auto it = fld.begin(), end = fld.end(); it != end; ++it)
    {
      //auto idx = std::distance(unique.begin(), std::find(unique.begin(), unique.end(), tools::tolower(*it)));
      auto idx = std::distance(unique.begin(), std::find(unique.begin(), unique.end(), *it));
      column_list.push_back(unique_cols[idx].first, it->c_str());
    }
  }
  else
  {
    for(const auto &it : unique_cols)
      column_list.push_back(it.first, it.second.c_str());
  }
  return column_list.get();*/
  auto ret = pt.add(tools::newVal(unique_cols.first));
  return tools::nameIt(ret, unique_cols.second);
}
