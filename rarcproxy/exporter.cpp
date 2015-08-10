#include "stdafx.h"
#include "exporter.h"
#include "tools.h"
//#include "rconnect.h"
#include "table_read.h"
#include "r_geometry.h"

template <typename T1, typename T2> struct IsSameType { enum { e = false }; };
template <typename T> struct IsSameType<T,T> { enum { e = true }; };

/*
CComPtr<ISpatialReference> g_lastUsedSR;

SEXP construct_named_list(const std::vector<fl_base*> &retColumns);

SEXP construct_dataframe(const std::vector<fl_base*> &retColumns)
{
  tools::protect pt;
  SEXP pairs = tools::newPair(R_NilValue, R_NilValue, pt);
  for (size_t i = 0, n = retColumns.size(); i < n; i++)
  {
    retColumns[i]->move_to(pairs);
    delete retColumns[i];
  }

  SEXP dfSym = ::Rf_install("data.frame");
  SEXP data_frame = ::Rf_eval(Rf_lcons(dfSym, CDR(pairs)), R_GlobalEnv);
  return data_frame;
}*/

long createField(const wchar_t *name, esriFieldType ft, IFieldsEdit* pFields)
{
  CComPtr<IFieldEdit> ipFieldEdit;
  HRESULT hr = ipFieldEdit.CoCreateInstance(CLSID_Field);
  if (hr != S_OK) return showError<true>(), -1;
  
  ipFieldEdit->put_Type(ft);
  if (ft == esriFieldTypeOID)
  {
    ipFieldEdit->put_Name(CComBSTR(L"OBJECTID"));
    ipFieldEdit->put_IsNullable(VARIANT_FALSE);
  }
  else if (ft == esriFieldTypeGeometry)
  {
    CComPtr<IGeometryDefEdit> ipGeoDef; hr = ipGeoDef.CoCreateInstance(CLSID_GeometryDef);
    if (hr != S_OK) return showError<true>(), -1;
    /*
    ipGeoDef->put_GeometryType(gt);
    CComQIPtr<ISpatialReference> ipSR(pSR);
    if (!ipSR)
    {
      ipSR.CoCreateInstance(CLSID_UnknownCoordinateSystem);
      CComQIPtr<ISpatialReferenceResolution> ipSRR(ipSR);
      if (ipSRR) FIX_DEFAULT_SR(ipSRR);
    }
    ipGeoDef->putref_SpatialReference(ipSR);
    */
    ipFieldEdit->put_Name(CComBSTR(L"Shape"));
    ipFieldEdit->put_IsNullable(VARIANT_TRUE);
    ipFieldEdit->putref_GeometryDef(ipGeoDef);
  }
  else
  {
    ipFieldEdit->put_Name(CComBSTR(name));
    ipFieldEdit->put_IsNullable(VARIANT_TRUE);
  }
  pFields->AddField(ipFieldEdit);
  long n = 0;
  pFields->get_FieldCount(&n);
  return n - 1;
}


class cols_base
{
public:
  long pos;
  std::wstring* name_ref;
  virtual bool get(size_t i, VARIANT &v) const = 0;
};

template <class T>
class cols_wrap : public cols_base
{
  SEXP vect;
  size_t len;
public:
  cols_wrap(SEXP sexp):vect(sexp), len(tools::size(sexp)) {}
  bool get(size_t i, VARIANT &v) const;
};

template <>
bool cols_wrap<bool>::get(size_t i, VARIANT &v) const
{
  if (i >= len)
    v.vt = VT_NULL;
  else
  {
    v.vt = VT_BOOL;
    v.boolVal = LOGICAL(vect)[i] ? VARIANT_TRUE : VARIANT_FALSE;
    if (v.boolVal == NA_INTEGER)
      v.vt = VT_NULL;
  }

  return true;
}

template <>
bool cols_wrap<int>::get(size_t i, VARIANT &v) const
{
  if (i >= len)
    v.vt = VT_NULL;
  else
  {
    v.vt = VT_I4;
    v.intVal = INTEGER(vect)[i];
    if (v.intVal == NA_INTEGER)
      v.vt=VT_NULL;
  }
  return true;
}
template <>
bool cols_wrap<double>::get(size_t i, VARIANT &v) const
{
  if (i >= len)
    v.vt = VT_NULL;
  else
  {
    v.vt = VT_R8;
    v.dblVal = REAL(vect)[i];
    if (ISNAN(v.dblVal))
      v.vt = VT_NULL;
  }
  return true;
}

template <>
bool cols_wrap<std::string>::get(size_t i, VARIANT &v) const
{
  if (i >= len)
  {
    v.vt = VT_NULL;
    return true;
  }

  SEXP s = STRING_ELT(vect, (R_len_t)i);
  if (s == NA_STRING)
    v.vt = VT_NULL;
  else 
  {  v.vt = VT_BSTR;
     const char* ptr = Rf_translateChar(s);
    _bstr_t str(ptr);
    v.bstrVal = str.Detach();
  }
  return true;
}
#if 0
SEXP R_dataframe2dataset(SEXP dtaframe, SEXP path, SEXP shape_columns)
{
  if (!Rf_isFrame(dtaframe))
    return showError<false>(L"argument 0 is not a data.frame"), R_NilValue;
//same as narray_tools.cpp
  std::wstring dataset_name;
  tools::copy_to(path, dataset_name);

  struct _cleanup
  {
    typedef std::vector<cols_base*> c_type;
    std::vector<std::string> name;
    c_type c;
    //std::vector<c_type::const_iterator> shape;
    c_type shape;
    ~_cleanup()
    {
      for (size_t i = 0; i < c.size(); i++)
        delete c[i];
      for (size_t i = 0; i < shape.size(); i++)
        delete shape[i];
    }
  }cols;

  //cols.name = df.attr("names");
  tools::getNames(dtaframe, cols.name);

  if (cols.name.empty())
    return showError<false>(L"data.frame has 0 column"), R_NilValue;
  if (tools::size(dtaframe) != cols.name.size())
    return showError<false>(L"unknown"), R_NilValue;

  CComPtr<IGPUtilities> ipDEUtil;
  if (ipDEUtil.CoCreateInstance(CLSID_GPUtilities) != S_OK)
    return showError<true>(L"IDEUtilitiesImpl - CoCreateInstance has failed"), R_NilValue;
  HRESULT hr;

  //cols.c.resize(cols.name.size(), NULL);

  bool isShape = false;
  if (shape_columns != R_NilValue)
  {
    std::vector<std::string> shapes;
    tools::copy_to(shape_columns, shapes);
    if (shapes.size() < 2 || shapes.size() > 4)
      return showError<false>(L"shape expecting 2 strings"), NULL;
    isShape = true;
    for (size_t i = 0; i < shapes.size(); i++)
    {
      std::vector<std::string>::iterator it = std::find(cols.name.begin(), cols.name.end(), shapes[i]);
      if (it == cols.name.end())
        return showError<false>(L"cannot find shape in data.frame"), NULL;

      size_t pos = std::distance(cols.name.begin(), it);
      cols.shape.push_back(new cols_wrap<double>(VECTOR_ELT(dtaframe, pos)));
      //cols.shape.[i] = cols.c.begin() + pos;
      it->clear();
    }
  }


  CComPtr<IName> ipName;
  if (isShape)
    hr = ipDEUtil->CreateFeatureClassName(CComBSTR(dataset_name.c_str()), &ipName);
  else
    hr = ipDEUtil->CreateTableName(CComBSTR(dataset_name.c_str()), &ipName);

  CComQIPtr<IDatasetName> ipDatasetName(ipName);
  CComPtr<IWorkspaceName> ipWksName;
  CComQIPtr<IWorkspace> ipWks;
  if (hr == S_OK)
    hr = ipDatasetName->get_WorkspaceName(&ipWksName);
  if (hr == S_OK)
  {
    CComPtr<IUnknown> ipUnk;
    hr = CComQIPtr<IName>(ipWksName)->Open(&ipUnk);
    ipWks = ipUnk;
  }

  if (hr != S_OK)
    return showError<true>(L"invalid table name"), R_NilValue;

  CComQIPtr<IFeatureWorkspace> ipFWKS(ipWks);
  ATLASSERT(ipFWKS);
  if (!ipFWKS)
    return showError<true>(L"not a FeatureWorkspace"), R_NilValue;

  CComBSTR bstrTableName;
  ipDatasetName->get_Name(&bstrTableName);
  /*
  CComQIPtr<IWorkspaceSchemaImpl> ipWSchema(ipWks);
  if (ipWSchema)
  {
    VARIANT_BOOL b = VARIANT_FALSE;
    ipWSchema->TableExists(bstrTableName, &b);
    if (b != VARIANT_FALSE)
      return ::Rf_error("table Exists"), NULL;
  }*/

  CComPtr<IFieldsEdit> ipFields;
  hr = ipFields.CoCreateInstance(CLSID_Fields);
  if (hr != S_OK) return showError<true>(L"CoCreateInstance"), R_NilValue;
  
  //if (!createField(NULL, esriFieldTypeOID, ipFields))
  //  return NULL;
  if (isShape)
  {
    long pos = createField(NULL, esriFieldTypeGeometry, ipFields);
    CComPtr<IGeometryDef> ipGeoDef;
    CComPtr<IField> ipField;
    ipFields->get_Field(pos, &ipField);
    ipField->get_GeometryDef(&ipGeoDef);
    CComQIPtr<IGeometryDefEdit> ipGeoDefEd(ipGeoDef);
    ipGeoDefEd->put_GeometryType(esriGeometryPoint);
    CComQIPtr<ISpatialReference> ipSR(g_lastUsedSR);
    if (!ipSR)
    {
      ipSR.CoCreateInstance(CLSID_UnknownCoordinateSystem);
      CComQIPtr<ISpatialReferenceResolution> ipSRR(ipSR);
      if (ipSRR) FIX_DEFAULT_SR(ipSRR);
    }
    ipGeoDefEd->putref_SpatialReference(ipSR);
  }


  for (size_t i = 0; i < cols.name.size(); i++)
  {
    if (cols.name[i].empty())
      continue;
    const char* str = cols.name[i].c_str();
    cols_base* item = NULL;
    SEXP it = VECTOR_ELT(dtaframe, i);
    switch (TYPEOF(it))
    {
       case NILSXP: case SYMSXP: case RAWSXP: case LISTSXP:
       case CLOSXP: case ENVSXP: case PROMSXP: case LANGSXP:
       case SPECIALSXP: case BUILTINSXP:
       case CPLXSXP: case DOTSXP: case ANYSXP: case VECSXP:
       case EXPRSXP: case BCODESXP: case EXTPTRSXP: case WEAKREFSXP:
       case S4SXP:
       default:
          return showError<false>(L"unsupported datat.field column type"), NULL;
       case INTSXP:
         item = new cols_wrap<int>(it);
         item->pos = createField(str, esriFieldTypeInteger, ipFields); 
         break;
       case REALSXP: 
         item = new cols_wrap<double>(it);
         item->pos = createField(str, esriFieldTypeDouble, ipFields); 
         break;
       case STRSXP:
       case CHARSXP:
         item = new cols_wrap<std::string>(it);
         item->pos = createField(str, esriFieldTypeString, ipFields);
         break;
       case LGLSXP: 
         item = new cols_wrap<bool>(it);
         item->pos = createField(str, esriFieldTypeInteger, ipFields);
         break;
    }
    ATLASSERT(item);
    cols.c.push_back(item);
    item->name_ref = &cols.name[i];
  }

  CComPtr<IFieldChecker> ipFieldChecker; ipFieldChecker.CoCreateInstance(CLSID_FieldChecker);  
  if (ipFieldChecker)
  {
    ipFieldChecker->putref_ValidateWorkspace(ipWks);
    long error = 0;

    //fix fields names
    CComPtr<IFields> ipFixedFields;
    
    CComPtr<IEnumFieldError> ipEError;
    hr = ipFieldChecker->Validate(ipFields, &ipEError, &ipFixedFields);
    if (hr != S_OK)
      return showError<true>(L"validate fields failed"), NULL;
  
    if (ipFixedFields)
    {
      ipFields = ipFixedFields;
      for (size_t c = 0; c < cols.c.size(); c++)
      {
        CComPtr<IField> ipFixedField;
        ipFixedFields->get_Field(cols.c[c]->pos, &ipFixedField);
        _bstr_t name;
        ipFixedField->get_Name(name.GetAddress());
        cols.c[c]->name_ref->assign(name);
      }
    }
  }

  CComPtr<IUID> ipUID; ipUID.CoCreateInstance(CLSID_UID);
  CComQIPtr<ITable> ipTableNew;
  CComBSTR keyword(L"");
  hr = E_FAIL;
  if (isShape)
  {
    if (ipUID)
    {
      OLECHAR buf[256];
      ::StringFromGUID2(CLSID_Feature, buf, 256);
      ipUID->put_Value(CComVariant(buf));
    }

    CComPtr<IFeatureClass> ipFClass;
    hr = ipFWKS->CreateFeatureClass(bstrTableName, ipFields, ipUID, 0, esriFTSimple, CComBSTR(L"Shape"), keyword, &ipFClass);
    ipTableNew = ipFClass;
  }
  else
  {
    if (ipUID)
    {
      OLECHAR buf[256];
      ::StringFromGUID2(CLSID_Row, buf, 256);
      ipUID->put_Value(CComVariant(buf));
    }
    hr = ipFWKS->CreateTable(bstrTableName, ipFields, ipUID, 0, keyword, &ipTableNew);
  }

  if (hr != S_OK)
    return showError<true>(L"validate fields failed"), R_NilValue;

  CComVariant oid;
 
  CComPtr<ICursor> ipCursor;
  CComPtr<IRowBuffer> ipRowBuffer;
  hr = ipTableNew->Insert(VARIANT_TRUE, &ipCursor);
  if (hr != S_OK)
    return showError<true>(L"Insert cursor failed"), R_NilValue;
  hr = ipTableNew->CreateRowBuffer(&ipRowBuffer);
  if (hr != S_OK)
    return showError<true>(L"Insert cursor failed"), R_NilValue;
  
  //re-map fields
  for (size_t c = 0; c < cols.c.size(); c++)
    ipCursor->FindField(CComBSTR(cols.c[c]->name_ref->c_str()), &(cols.c[c]->pos));

  R_len_t n = tools::size(VECTOR_ELT(dtaframe, 0));
  for (R_len_t i = 0; i < n; i++)
  {
    //ATLTRACE("\n");
    for (size_t c = 0; c < cols.c.size(); c++)
    {
      if (cols.c[c]->pos < 0)
        continue;
      CComVariant val;
      cols.c[c]->get(i, val);
      hr = ipRowBuffer->put_Value(cols.c[c]->pos, val);
      if (hr != S_OK)
        return showError<true>(L"insert row value failed"), R_NilValue;
      //ATLTRACE(" [%i]=%f",cols[c]->pos, (float)val.dblVal);
    }
    VARIANT oid;
    
    if (isShape)
    {
      CComQIPtr<IPoint> ipPoint; ipPoint.CoCreateInstance(CLSID_Point);
      CComVariant valX, valY;
      cols.shape[0]->get(i, valX);
      cols.shape[1]->get(i, valY);
      ipPoint->PutCoords(valX.dblVal, valY.dblVal);
      CComQIPtr<IFeatureBuffer> ipFBuffer(ipRowBuffer);
      ATLASSERT(ipFBuffer);
      hr = ipFBuffer->putref_Shape(ipPoint);
      if (hr != S_OK)
        return showError<true>(L"insert shape failed"), R_NilValue;
    }

    hr = ipCursor->InsertRow(ipRowBuffer, &oid);
    if (hr != S_OK)
      return showError<true>(L"insert row failed"), R_NilValue;
  }
  return R_NilValue;
}
#endif

static cols_base* setup_field(IFieldsEdit* pFields, SEXP it, const wchar_t* str)
{
  cols_base* item = NULL;
  switch (TYPEOF(it))
  {
     case NILSXP: case SYMSXP: case RAWSXP: case LISTSXP:
     case CLOSXP: case ENVSXP: case PROMSXP: case LANGSXP:
     case SPECIALSXP: case BUILTINSXP:
     case CPLXSXP: case DOTSXP: case ANYSXP: case VECSXP:
     case EXPRSXP: case BCODESXP: case EXTPTRSXP: case WEAKREFSXP:
     case S4SXP:
     default:
        return NULL;
     case INTSXP:
       item = new cols_wrap<int>(it);
       item->pos = createField(str, esriFieldTypeInteger, pFields); 
       break;
     case REALSXP: 
       item = new cols_wrap<double>(it);
       item->pos = createField(str, esriFieldTypeDouble, pFields); 
       break;
     case STRSXP:
     case CHARSXP:
       item = new cols_wrap<std::string>(it);
       item->pos = createField(str, esriFieldTypeString, pFields);
       break;
     case LGLSXP: 
       item = new cols_wrap<bool>(it);
       item->pos = createField(str, esriFieldTypeInteger, pFields);
       break;
  }
  return item;
}
SEXP R_export2dataset(SEXP path, SEXP dataframe, SEXP shape, SEXP shape_info)
{
  std::wstring dataset_name;
  tools::copy_to(path, dataset_name);

  struct _cleanup
  {
    typedef std::vector<cols_base*> c_type;
    std::vector<std::wstring> name;
    c_type c;
    //std::vector<c_type::const_iterator> shape;
    c_type shape;
    ~_cleanup()
    {
      for (size_t i = 0; i < c.size(); i++)
        delete c[i];
      for (size_t i = 0; i < shape.size(); i++)
        delete shape[i];
    }
  }cols;

  shape_extractor extractor;
  bool isShape = extractor.init(shape, shape_info) == S_OK;
  //SEXP sinfo = Rf_getAttrib(shape, Rf_mkChar("shape_info"));
  //cols.name = df.attr("names");
  tools::getNames(dataframe, cols.name);
  
  //tools::vectorGeneric shape_info(sinfo);
  //std::string gt_type;
  //tools::copy_to(shape_info.at("type"), gt_type);
  esriGeometryType gt = extractor.type();//str2geometryType(gt_type.c_str());
  R_xlen_t n = 0;

  ATLTRACE("dataframe type:%s", Rf_type2char(TYPEOF(dataframe)));

  if (Rf_isVectorList(dataframe))
  {
    size_t k = tools::size(dataframe);
    cols.name.resize(k);
    for (size_t i = 0; i < k; i++)
    {
      n = std::max(n, tools::size(VECTOR_ELT(dataframe, (R_xlen_t)i)));
      if (cols.name[i].empty())
        cols.name[i] = L"data";
    }
  }
  else
  {
    n = tools::size(dataframe);
    ATLASSERT(cols.name.empty());
  }

  if (isShape == false && n == 0)
    return showError<false>(L"nothing to save"), R_NilValue;

  if (isShape && n != extractor.size() )
    return showError<false>(L"length of shape != data.frame"), R_NilValue;

  CComPtr<IGPUtilities> ipDEUtil;
  if (ipDEUtil.CoCreateInstance(CLSID_GPUtilities) != S_OK)
    return showError<true>(L"IDEUtilitiesImpl - CoCreateInstance has failed"), R_NilValue;

  HRESULT hr = 0;

  CComPtr<IName> ipName;
  if (isShape)
    hr = ipDEUtil->CreateFeatureClassName(CComBSTR(dataset_name.c_str()), &ipName);
  else
    hr = ipDEUtil->CreateTableName(CComBSTR(dataset_name.c_str()), &ipName);

  CComQIPtr<IDatasetName> ipDatasetName(ipName);
  CComPtr<IWorkspaceName> ipWksName;
  CComQIPtr<IWorkspace> ipWks;
  if (hr == S_OK)
    hr = ipDatasetName->get_WorkspaceName(&ipWksName);
  if (hr == S_OK)
  {
    CComPtr<IUnknown> ipUnk;
    hr = CComQIPtr<IName>(ipWksName)->Open(&ipUnk);
    ipWks = ipUnk;
  }

  if (hr != S_OK)
    return showError<true>(L"invalid table name"), R_NilValue;
  
  CComQIPtr<IFeatureWorkspace> ipFWKS(ipWks);
  ATLASSERT(ipFWKS);
  if (!ipFWKS)
    return showError<true>(L"not a FeatureWorkspace"), R_NilValue;
  
  CComBSTR bstrTableName;
  ipDatasetName->get_Name(&bstrTableName);

  CComPtr<IFieldsEdit> ipFields;
  hr = ipFields.CoCreateInstance(CLSID_Fields);
  if (hr != S_OK) return showError<true>(L"CoCreateInstance"), R_NilValue;

  createField(NULL, esriFieldTypeOID, ipFields);

  CComPtr<ISpatialReference> ipSR;

  if (isShape)
  {
    long pos = createField(NULL, esriFieldTypeGeometry, ipFields);
    CComPtr<IGeometryDef> ipGeoDef;
    CComPtr<IField> ipField;
    ipFields->get_Field(pos, &ipField);
    ipField->get_GeometryDef(&ipGeoDef);
    
    CComQIPtr<IGeometryDefEdit> ipGeoDefEd(ipGeoDef);
    ipGeoDefEd->put_GeometryType(gt);
    ipGeoDefEd->putref_SpatialReference(extractor.sr());
  }

  if (cols.name.empty())
  {
    cols.name.push_back(L"data");
    cols_base* item = setup_field(ipFields, dataframe, cols.name[0].c_str());
    if (!item)
      return showError<false>(L"unsupported datat.field column type"), NULL;
    cols.c.push_back(item);
    item->name_ref = &cols.name[0];
  }
  else
    for (size_t i = 0; i < cols.name.size(); i++)
    {
      if (cols.name[i].empty())
        continue;
      const wchar_t* str = cols.name[i].c_str();
      SEXP it = VECTOR_ELT(dataframe, (R_len_t)i);
      cols_base* item = setup_field(ipFields, it, str);
      if (!item)
        return showError<false>(L"unsupported datat.field column type"), NULL;
      cols.c.push_back(item);
      item->name_ref = &cols.name[i];
    }

  CComPtr<IFieldChecker> ipFieldChecker; ipFieldChecker.CoCreateInstance(CLSID_FieldChecker);  
  if (ipFieldChecker)
  {
    ipFieldChecker->putref_ValidateWorkspace(ipWks);
    long error = 0;

    //fix fields names
    CComPtr<IFields> ipFixedFields;
    
    CComPtr<IEnumFieldError> ipEError;
    hr = ipFieldChecker->Validate(ipFields, &ipEError, &ipFixedFields);
    if (hr != S_OK)
      return showError<true>(L"validate fields failed"), NULL;
  
    if (ipFixedFields)
    {
      ipFields = ipFixedFields;
      for (size_t c = 0; c < cols.c.size(); c++)
      {
        CComPtr<IField> ipFixedField;
        ipFixedFields->get_Field(cols.c[c]->pos, &ipFixedField);
        _bstr_t name;
        ipFixedField->get_Name(name.GetAddress());
        cols.c[c]->name_ref->assign(name);
      }
    }
  }

  CComPtr<IUID> ipUID; ipUID.CoCreateInstance(CLSID_UID);
  if (ipUID)
  {
    OLECHAR buf[256];
    ::StringFromGUID2(isShape ? CLSID_Feature : CLSID_Row, buf, 256);
    ipUID->put_Value(CComVariant(buf));
  }

  CComQIPtr<ITable> ipTableNew;
  CComBSTR keyword(L"");
  hr = E_FAIL;
  if (isShape)
  {
    CComPtr<IFeatureClass> ipFClass;
    hr = ipFWKS->CreateFeatureClass(bstrTableName, ipFields, ipUID, 0, esriFTSimple, CComBSTR(L"Shape"), keyword, &ipFClass);
    ipTableNew = ipFClass;
  }
  else
  {
    hr = ipFWKS->CreateTable(bstrTableName, ipFields, ipUID, 0, keyword, &ipTableNew);
  }
  if (hr != S_OK)
  {
    std::wstring err_txt(isShape ? L"Create FeatureClass :" : L"Create Table :");
    err_txt += bstrTableName;
    err_txt += L" has failed";
    return showError<true>(err_txt.c_str()), R_NilValue;
  }

  CComVariant oid;

  CComPtr<ICursor> ipCursor;
  CComPtr<IRowBuffer> ipRowBuffer;
  hr = ipTableNew->Insert(VARIANT_TRUE, &ipCursor);
  if (hr != S_OK)
    return showError<true>(L"Insert cursor failed"), R_NilValue;
  hr = ipTableNew->CreateRowBuffer(&ipRowBuffer);
  if (hr != S_OK)
    return showError<true>(L"Insert cursor failed"), R_NilValue;
  
  //re-map fields
  for (size_t c = 0; c < cols.c.size(); c++)
    ipCursor->FindField(CComBSTR(cols.c[c]->name_ref->c_str()), &(cols.c[c]->pos));

  CComQIPtr<IFeatureBuffer> ipFBuffer(ipRowBuffer);
  for (R_len_t i = 0; i < n; i++)
  {
    //ATLTRACE("\n");
    for (size_t c = 0; c < cols.c.size(); c++)
    {
      if (cols.c[c]->pos < 0)
        continue;
      CComVariant val;
      cols.c[c]->get(i, val);
      hr = ipRowBuffer->put_Value(cols.c[c]->pos, val);
      if (FAILED(hr))
        return showError<true>(L"insert row value failed"), R_NilValue;
    }
    VARIANT oid;

    if (isShape)
    {
      ATLASSERT(ipFBuffer);
      CComQIPtr<IGeometry> ipNewShape;
      hr = extractor.at(i, &ipNewShape);
      if (hr != S_OK)
        return R_NilValue;

      hr = ipFBuffer->putref_Shape(ipNewShape);
      if (FAILED(hr))
        return showError<true>(L"insert shape failed"), R_NilValue;
    }

    hr = ipCursor->InsertRow(ipRowBuffer, &oid);
    if (hr != S_OK)
      return showError<true>(L"insert row failed"), R_NilValue;
  }
  return R_NilValue;
}

