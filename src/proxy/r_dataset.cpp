#include "stdafx.h"
#include "r_dataset.h"
#include "tools.h"
#include "misc.h"
#include <Rdefines.h>
#include <unordered_map>


const char* dataset::class_name = "++dataset++";

SEXP dataset::open(SEXP rPath)
{
  std::wstring path;
  if (!tools::copy_to(rPath, path))
    return error_Ret("wrong path");

  //m_ipDataset.Attach(arcobject::open_dataset(path));
  m_dataset.reset(arcobject::open_dataset(path));
  if (m_dataset.get() == nullptr)
    return showError<true>("cannot open dataset");

  return tools::newVal(true);
}

SEXP dataset::get_type()
{
  const char* type_name = m_dataset->get_type();
  if (type_name == nullptr)
    return error_Ret("not a dataset");
  return tools::newVal(type_name);
}

const char* table::class_name = "++table++";

SEXP table::get_fields()
{
  //if (fix_table() == NULL)
  if (!m_dataset->is_table())
    return error_Ret("arc.table");
  auto ret =  arcobject::dataset_handle::get_fields(m_dataset.get());

  tools::protect pt;
  SEXP vect_type = pt.add(tools::newVal(ret.second));
  return tools::nameIt(vect_type, ret.first);
}

//SEXP table::select2(SEXP args)
SEXP table::select(SEXP fields, SEXP args)
{
  using namespace arcobject;
  //if (fix_table() == NULL)
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

  std::pair<std::wstring, int> sp_ref;
  sp_ref.second = 0;
  if (!tools::copy_to(sr, sp_ref.second))
  {
    std::wstring wkt;
    if (tools::copy_to(sr, wkt))
    {
      struct eq
      {
        static bool op(const wchar_t &c) { return c == L'\''; }
      };
      std::replace_if(wkt.begin(), wkt.end(), eq::op, L'\"');
      sp_ref.first = wkt;
    }
  }

  std::vector<std::wstring> unique;
  for (const auto& it : fld)
    unique.push_back(tools::tolower(it));

  auto end = std::unique(unique.begin(), unique.end());
  auto new_size = std::distance(unique.begin(), end);
  unique.resize(new_size);

  auto cols = arcobject::dataset_handle::select(m_dataset.get(), unique, where_clause, use_selection, sp_ref);
  if (cols.empty())
    return showError<true>("COM error");

  tools::protect pt;
  std::vector<std::pair<SEXP, std::wstring>> unique_cols;
  for (dataset_handle::column_t* it : cols)
  {
    switch (it->t)
    {
      case dataset_handle::column_t::eInt:
        unique_cols.emplace_back(pt.add(tools::newVal(dataset_handle::column_t::get_ints(*it))), it->name);
      break;
      case dataset_handle::column_t::eDate:
        {
          auto &data = dataset_handle::column_t::get_doubles(*it);
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
      case dataset_handle::column_t::eDouble:
        unique_cols.emplace_back(pt.add(tools::newVal(dataset_handle::column_t::get_doubles(*it))), it->name);
      break;
      case dataset_handle::column_t::eString:
        unique_cols.emplace_back(pt.add(tools::newVal(dataset_handle::column_t::get_strings(*it))), it->name);
      break;
      case dataset_handle::column_t::eGeometry_point:
      {
        bool add_wkt = (!it->sr.first.empty() || it->sr.second != 0);
        auto& pts = dataset_handle::column_t::get_points(*it);
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
      case dataset_handle::column_t::eGeometry_polyline:
      case dataset_handle::column_t::eGeometry_polygon:
      case dataset_handle::column_t::eGeometry_multipoint:
      {
        auto& geoms = dataset_handle::column_t::get_shapes(*it);
        bool add_wkt = (!it->sr.first.empty() || it->sr.second != 0);
        tools::listGeneric shapes(geoms.size() + add_wkt ? 2 : 0);
        std::wstring name;
        if (it->t == dataset_handle::column_t::eGeometry_polyline)
          name = L"Polyline";
        else if (it->t == dataset_handle::column_t::eGeometry_polygon)
          name = L"Polygon";
        else if (it->t == dataset_handle::column_t::eGeometry_multipoint)
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
    delete it;
  }
  //use same reference for duplicated columns
  tools::listGeneric column_list(fld.size());
  ATLASSERT(unique_cols.size() == unique.size());
  if (unique_cols.size() == unique.size())
  {
    for(auto it = fld.begin(), end = fld.end(); it != end; ++it)
    {
      auto idx = std::distance(unique.begin(), std::find(unique.begin(), unique.end(), tools::tolower(*it)));
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


const char* feature_class::class_name = "++feature_class++";

static SEXP get_shape_info(const arcobject::dataset_handle *h)
{
  auto ret = arcobject::dataset_handle::get_shape_info(h);
  tools::listGeneric shape_attr(ret.size());
  for (auto &it : ret)
  {
    shape_attr.push_back(it.second, it.first);
    ::VariantClear(&it.second);
  }
  return shape_attr.get();
}

SEXP feature_class::get_shape_info()
{
  if (!m_dataset->is_fc())
    return error_Ret("arc.feature");

  return ::get_shape_info(m_dataset.get());
}

SEXP feature_class::get_extent()
{
  if (!m_dataset->is_fc())
    return error_Ret("arc.feature");
  return extent2r(arcobject::dataset_handle::get_extent(m_dataset.get()));
}

const char* container::class_name = "++container++";
SEXP container::get_children()
{
  std::unordered_map<std::string, std::vector<std::wstring>> map;
  {
    const auto ret = arcobject::dataset_handle::get_children(m_dataset.get());
    for (size_t i = 0, n = ret.first.size(); i < n; i++)
      map[ret.second[i]].push_back(ret.first[i]);
  }
  tools::listGeneric list(map.size());
  for (const auto &it : map)
    list.push_back(tools::newVal(it.second), it.first);
  return list.get();
}

SEXP container::get_extent()
{
  return extent2r(arcobject::dataset_handle::get_extent(m_dataset.get()));
}

SEXP container::get_sr()
{
  auto ret = arcobject::dataset_handle::get_sr(m_dataset.get());
  tools::listGeneric sr_attr(ret.size());
  for (auto &it : ret)
  {
    sr_attr.push_back(it.second, it.first);
    ::VariantClear(&it.second);
  }
  return sr_attr.get();
}

