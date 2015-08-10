#include "stdafx.h"
#include "r_dataset.h"
#include "tools.h"
#include "table_read.h"
#include "r_geometry.h"

//#import "\ArcGIS\com\esriGeoprocessing.olb" named_guids no_namespace no_implementation raw_interfaces_only exclude("UINT_PTR", "OLE_HANDLE", "OLE_COLOR")

static const std::pair<std::wstring, std::string> read_field_info(IField* pField)
{
  CComBSTR name;
  pField->get_Name(&name);
  //return Rf_cons(tools::newVal(std::wstring(name), pt), tools::newVal((int)2, pt));

  std::pair<std::wstring, std::string> ret;
  ret.first = name;
  //pField->get_AliasName(&name);
  //arr[1] = name;
  esriFieldType ft;
  pField->get_Type(&ft);

  const char* str_ft[] =
  {
    "SmallInteger",
    "Integer",
    "Single",
    "Double",
    "String",
    "Date",
    "OID",
    "Geometry",
    "Blob",
    "Raster",
    "GUID",
    "GlobalID",
    "XML"
  };

  ret.second = str_ft[ft];
  /*std::vector<std::string> names(2);
  names[0] = "name";
  names[2] = "type";
  SEXP ret = tools::newVal(arr, pt);
  return tools::nameIt(ret, names);
  //return tools::newVal(arr, pt);
  */
  return ret;
}



const char* dataset::class_name = "++dataset++";
//bool dataset::create(SEXP rHost)

template<class T>
static bool is_Valid(CComPtr<T> &ipT)
{
  VARIANT_BOOL b = VARIANT_FALSE;
  ipT->get_Valid(&b);
  return (b != VARIANT_FALSE);
}

SEXP dataset::open(SEXP rPath)
{
  std::wstring path;
  if (!tools::copy_to(rPath, path))
    return error_Ret("wrong path");

  CComBSTR dataset_name(path.c_str());
  CComPtr<IGPUtilities> ipGPUtil;
  HRESULT hr = ipGPUtil.CoCreateInstance(CLSID_GPUtilities);
  if (hr != S_OK)
    return error_Ret("COM error. CoCreateInstance(CLSID_GPUtilities)");

  CComQIPtr<ITable> ipTable;
  CComQIPtr<ILayer> ipLayer;
  CComQIPtr<IStandaloneTable> ipSTable;

  hr = ipGPUtil->FindMapLayer(dataset_name, &ipLayer);

  if (!ipLayer)
  {
    hr = ipGPUtil->FindMapTable(dataset_name, &ipTable);
    if (ipTable)
    {
      m_ipDataset = ipTable;
      ipSTable = ipTable;
      if (ipSTable && !is_Valid(ipSTable))
        error_Ret("input table is invalid");
      
      ipTable.Release();
    }
    else
    {
      //open from disk
      CComPtr<IDataset> ipDataset;
      hr = ipGPUtil->OpenDatasetFromLocation(dataset_name, &ipDataset);
      if (ipDataset)
      {
        ipTable = ipDataset;
        m_ipDataset = ipDataset;
      }
    }
  }
  else
  {
    if (!is_Valid(ipLayer))
      return error_Ret("input layer is invalid");

    //CComPtr<IDataLayer>m_ipDataset 
    m_ipDataset = ipLayer;
  }
  /*
  CComQIPtr<IDisplayTable> ipDTable;
  if (ipLayer)
  {
    ipDTable = ipLayer;
  }
  else
    ipDTable = ipSTable;

  if (ipDTable)
  {
    ipDTable->get_DisplayTable(&ipTable);
  }

  if(!ipTable)
    return false;
  
  m_ipDataset = ipTable;
  */

  if (!m_ipDataset)
    return showError<true>(L"cannot open dataset");

  return tools::newVal(true);
}

SEXP dataset::get_type()
{
  if (!m_ipDataset)
    return error_Ret("not a table");

  esriDatasetType dt;
  m_ipDataset->get_Type(&dt);
  static const char* ds_name[] = {
    "<ERROR>",
    "Any",
    "Container",
    "Geo",
    "FeatureDataset",
    "FeatureClass",
    "PlanarGraph",
    "GeometricNetwork",
    "Topology",
    "Text",
    "Table",
    "RelationshipClass",
    "RasterDataset",
    "RasterBand",
    "Tin",
    "CadDrawing",
    "RasterCatalog",
    "Toolbox",
    "Tool",
    "NetworkDataset",
    "Terrain",
    "RepresentationClass",
    "CadastralFabric",
    "SchematicDataset",
    "Locator",
    "Map",
    "Layer",
    "Style",
    "MosaicDataset",
    "LasDataset"
  };
  return tools::newVal(ds_name[dt]);
}

const char* table::class_name = "++table++";

SEXP table::create_from(SEXP rHost, SEXP from)
{
  dataset* ds = rtl::getCObject<dataset>(from);
  if (ds)
  {
    if (rtl::createT<table>(rHost))
    {
      table* ts = rtl::getCObject<table>(rHost);
      ts->m_ipDataset = ds->m_ipDataset;
    }
  }
  return rHost;
}

SEXP table::get_fields()
{
  if (fix_table() == NULL)
    return error_Ret("arc.table");

  //std::vector<SEXP> fields;
  CComPtr<IFields> ipFields;
  m_ipTable->get_Fields(&ipFields);
  ATLASSERT(ipFields);
  if (!ipFields)
    return R_NilValue;

  long n = 0;
  ipFields->get_FieldCount(&n);
  std::vector< std::wstring > all_name(n);
  //std::vector< std::wstring > all_alias;
  std::vector<std::string> all_type(n);
  for (long i = 0; i < n; i++)
  {
    CComPtr<IField> ipField;
    ipFields->get_Field(i, &ipField);
    //ipField->get_AliasName()
    std::pair<std::wstring, std::string> info = read_field_info(ipField);
    all_name[i] = info.first;
    //all_alias.push_back(info[1]);
    all_type[i] = info.second;
  }
  if (all_name.empty())
    return R_NilValue;

  return tools::nameIt(tools::newVal(all_type), all_name);
}

SEXP construct_named_list(const std::vector<fl_base*> &retColumns)
{
  tools::listGeneric result_list(retColumns.size());// + pSRNew == nullptr ? 0 : 1);
  std::vector<std::wstring> column_names(retColumns.size());
  for (size_t i = 0, n = retColumns.size(); i < n; i++)
  {
    column_names[i] = retColumns[i]->name;
    //result_list[i] = pt.add(retColumns[i]->getSEXP());
    result_list.push_back(retColumns[i]->getSEXP());
  }
  return tools::nameIt(result_list.get(), column_names);
}

void extractSS(IDisplayTable* pDT, ISelectionSet **ppSS, BSTR *where_);

SEXP table::select(SEXP fields, SEXP args)
{
  if (fix_table() == NULL)
    return error_Ret("arc.table");

  std::vector<std::wstring> fld;
  if (!tools::copy_to(fields, fld)) 
    return error_Ret("'fields' should be string(s)");
  if (fld.empty())
    return error_Ret("'fields' is empty");

  bool use_selection = true;
  std::wstring where_clause;

  CComPtr<ISpatialReference> ipSRNew;
  size_t i = 0;

  while (args != R_NilValue && args != NULL) 
  {
    SEXP tag = TAG(args);
    //if (tag == R_NaRmSymbol)
    //  tools::copy_to(CAR(args), na_rm);
    //else
    {
      std::string tag_name = CHAR(PRINTNAME(tag));
      if (tag_name == "selected")
        tools::copy_to(CAR(args), use_selection);
      else if (tag_name == "where_clause")
        tools::copy_to(CAR(args), where_clause);
      else if (tag_name == "sr")
      {
        SEXP spx = CAR(args);
        int spid = 0;
        std::wstring wkt;
        if (tools::copy_to(spx, spid))
          create_sr(spid, L"", &ipSRNew);
        if (ipSRNew == NULL && tools::copy_to(spx, wkt))
          create_sr(0, wkt, &ipSRNew);
      }
    }
    args = CDR(args);
  }
  //for (SEXP s = ATTRIB(dots); s != R_NilValue; s = CDR(s))
  //{
  //  if (TAG(s) == R_NaRmSymbol)
  //    tools::copy_to(CAR(s), rm_NA);
  //}

  CComBSTR bstr_where;
  CComPtr<ISelectionSet> ipSelectionSet;
  CComPtr<ITable> ipTable;
  CComQIPtr<IDisplayTable> ipDTable(m_ipDataset);
  if (ipDTable)
  {
    ipDTable->get_DisplayTable(&ipTable);
    if (use_selection)
      extractSS(ipDTable, &ipSelectionSet, &bstr_where);
  }
  else
    ipTable = m_ipTable;

  if (!ipTable)
    return error_Ret("arc.table");

  CComPtr<ICursor> ipCursor;
  CComPtr<IQueryFilter> ipFilter;
  HRESULT hr = ipFilter.CoCreateInstance(CLSID_QueryFilter);
  CComBSTR bstr(fld[0].c_str());
  for (size_t i = 1; i < fld.size(); i++)
  {
    bstr += L",";
    bstr += fld[i].c_str();
  }
  ipFilter->put_SubFields(bstr);
  if (!where_clause.empty())
  {
    if (bstr_where)
    {
      bstr_where.Append(L" AND (");
      bstr_where.Append(where_clause.c_str());
      bstr_where.Append(L")");
    }
    else
      bstr_where = where_clause.c_str();
  }

  if (bstr_where)
    ipFilter->put_WhereClause(bstr_where);

  if (ipSRNew != NULL)
  {
    ipFilter->putref_OutputSpatialReference(L"Shape", ipSRNew);
  }

  if (ipSelectionSet)
    hr = ipSelectionSet->Search(ipFilter, VARIANT_TRUE, &ipCursor);
  else
    hr = ipTable->Search(ipFilter, VARIANT_TRUE, &ipCursor);

  SEXP ret = R_NilValue;
  //tools::protect pt;
  if (hr == S_OK)
  {
    std::vector<fl_base*> retColumns;
    hr = load_from_cursor(ipCursor, fld, retColumns, ipSRNew);
    if (hr == S_OK && !retColumns.empty())
      //ret = pt.add(construct_named_list(retColumns));
      ret = construct_named_list(retColumns);
    for (size_t i = 0, n = retColumns.size(); i < n; i++)
      delete retColumns[i];
  }
  else
    showError<true>(L"COM error");

  return ret;
}

const char* feature_class::class_name = "++feature_class++";
SEXP feature_class::create_from(SEXP rHost, SEXP from)
{
  dataset* ds = rtl::getCObject<dataset>(from);
  if (ds)
  {
    if (rtl::createT<feature_class>(rHost))
    {
      feature_class* pThis = rtl::getCObject<feature_class>(rHost);
      pThis->m_ipDataset = ds->m_ipDataset;
    }
  }
  return rHost;
}

SEXP feature_class::get_shape_info()
{
  if (!fix_fc())
    return error_Ret("arc.feature_class");

  tools::protect pt;

  esriGeometryType gt;
  m_ipFC->get_ShapeType(&gt);
  //std::vector<SEXP> shape_attr(5);
  tools::listGeneric shape_attr(5);
  std::vector<std::string> shape_attr_name(5);
  shape_attr_name[0] = "type";
  shape_attr.push_back(geometryType2str(gt));
  shape_attr_name[1] = "hasZ";
  shape_attr.push_back(false);
  shape_attr_name[2] = "hasM";
  shape_attr.push_back(false);

  CComQIPtr<IGeoDataset> ipGeo(m_ipDataset);
  CComPtr<ISpatialReference> ipSR;
  ipGeo->get_SpatialReference(&ipSR);
  if (ipSR)
  {
    shape_attr_name[3] = "WKT";
    shape_attr.push_back(spatial_reference2wkt(ipSR));
    long srid = 0;
    ipSR->get_FactoryCode(&srid);
    shape_attr.push_back((int)srid);
    shape_attr_name[4] = "WKID";
  }

  //SEXP shape = tools::newVal(shape_attr, pt);
  return tools::nameIt(shape_attr.get(), shape_attr_name);
}

SEXP feature_class::get_extent()
{
  if (!fix_fc())
    return error_Ret("arc.feature_class");

  CComQIPtr<IGeoDataset> ipGeoDS(m_ipFC);
  if (!ipGeoDS)
    return R_NilValue;

  tools::protect pt;
  CComPtr<IEnvelope> ipExtent;
  ipGeoDS->get_Extent(&ipExtent);
  return extent2r(ipExtent);
}
