#include "stdafx.h"
#include "exporter.h"
#include "tools.h"
#include "misc.h"
#include "table_read.h"
#include "r_geometry.h"

template <typename T1, typename T2> struct IsSameType { enum { e = false }; };
template <typename T> struct IsSameType<T,T> { enum { e = true }; };

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
protected:
  cols_base(){ vNULL.vt = VT_NULL; }
public:
  long pos;
  CComVariant vNULL;
  std::wstring* name_ref;
  virtual bool get(size_t i, VARIANT &v) const = 0;
};

template <class T, int vt>
class cols_wrap : public cols_base
{
  SEXP vect;
  size_t len;
public:
  cols_wrap(SEXP sexp):vect(sexp), len(tools::size(sexp)) {}
  bool get(size_t i, VARIANT &v) const override;
};

template <>
bool cols_wrap<bool, VT_BOOL>::get(size_t i, VARIANT &v) const
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
bool cols_wrap<int, VT_I4>::get(size_t i, VARIANT &v) const
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
bool cols_wrap<double, VT_R8>::get(size_t i, VARIANT &v) const
{
  if (i >= len)
    v.vt = VT_NULL;
  else
  {
    v.vt = VT_R8;
    v.dblVal = REAL(vect)[i];
    if (ISNA(v.dblVal))
      v.vt = VT_NULL;
  }
  return true;
}

template <>
bool cols_wrap<double, VT_DATE>::get(size_t i, VARIANT &v) const
{
  if (i >= len)
    v.vt = VT_NULL;
  else
  {
    v.vt = VT_DATE;
    v.dblVal = REAL(vect)[i];
    if (ISNA(v.dblVal))
      v.vt = VT_NULL;
    else
      v.date = epoch2ole(v.date);
  }
  return true;
}

template <>
bool cols_wrap<std::string, VT_BSTR>::get(size_t i, VARIANT &v) const
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
       item = new cols_wrap<int, VT_I4>(it);
       item->pos = createField(str, esriFieldTypeInteger, pFields);
       break;
     case REALSXP:
     {
       bool isDate = Rf_inherits(it, "POSIXct");
       if (isDate)
         item = new cols_wrap<double, VT_DATE>(it);
       else
         item = new cols_wrap<double, VT_R8>(it);
       item->pos = createField(str, isDate ? esriFieldTypeDate : esriFieldTypeDouble, pFields);
     }break;
     case STRSXP:
     case CHARSXP:
       item = new cols_wrap<std::string, VT_BSTR>(it);
       item->pos = createField(str, esriFieldTypeString, pFields);
       break;
     case LGLSXP: 
       item = new cols_wrap<bool, VT_BOOL>(it);
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
  CComPtr<IFields> ipRealFields;
  ipCursor->get_Fields(&ipRealFields);
  for (size_t c = 0; c < cols.c.size(); c++)
  {
    ipRealFields->FindField(CComBSTR(cols.c[c]->name_ref->c_str()), &(cols.c[c]->pos));
    CComPtr<IField> ipField;
    ipRealFields->get_Field(cols.c[c]->pos, &ipField);
    VARIANT_BOOL b = VARIANT_FALSE;
    ipField->get_IsNullable(&b);
    if (b == VARIANT_FALSE)
    {
      esriFieldType ft = esriFieldTypeInteger;
      ipField->get_Type(&ft);
      switch(ft)
      {
        case esriFieldTypeInteger:
          cols.c[c]->vNULL = 0;//std::numeric_limits<int>::min();
          break;
        case esriFieldTypeDate:
        case esriFieldTypeDouble:
          cols.c[c]->vNULL = 0.0;//-std::numeric_limits<double>::max();
          break;
        case esriFieldTypeString:
          cols.c[c]->vNULL = L"";
      }
    }
  }

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
      if (val.vt == VT_NULL)
        hr = ipRowBuffer->put_Value(cols.c[c]->pos, cols.c[c]->vNULL);
      else
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

