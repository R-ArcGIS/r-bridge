#include "stdafx.h"
#include "gp_env.h"
#include "tools.h"
#include "r_geometry.h"
#include <map>

typedef SEXP(*fn_com2r)(IGPValue* pVal);
typedef std::map<std::wstring, fn_com2r> static_map;
const static_map& get_smap();


template<class T> void fill_vector(IGPMultiValue* pVals, std::vector<T> &a)
{
  long n = 0;
  pVals->get_Count(&n);
  a.resize((size_t)n);

  for (long i = 0; i < n; i++)
  {
    CComPtr<IGPValue> ipV;
    pVals->get_Value(i, &ipV);
    VARIANT_BOOL b = VARIANT_FALSE;
    ipV->IsEmpty(&b);
    CComVariant v;
    if (b != VARIANT_TRUE)
    {
      CComQIPtr<IGPVariant> ipGPVar(ipV);
      ipGPVar->get_Variant(&v);
    }
    a[i] = tools::castVar<T>(v);
  }
}

template<class T>
static SEXP com2r(IGPValue* pVal)
{
  if (!pVal)
    return R_NilValue;
  VARIANT_BOOL b = VARIANT_TRUE;
  pVal->IsEmpty(&b);
  if (b == VARIANT_TRUE)
    return R_NilValue;

  CComBSTR dt_name;
  CComPtr<IGPDataType> ipDataType;
  pVal->get_DataType(&ipDataType);
  ipDataType->get_Name(&dt_name);

  static_map::const_iterator it = get_smap().find(std::wstring(dt_name));
  if (it != get_smap().end())
    return (*(it->second))(pVal);
  ATLTRACE(L"unsupported type:%s\n", (LPCWSTR)dt_name);
  return com2str(pVal);
}

SEXP gpvalue2any(IGPValue* pVal)
{
  return com2r<void*>(pVal);
}


static SEXP com2str(IGPValue* pVal)
{
  CComBSTR bstr;
  pVal->GetAsText(&bstr);
  return tools::newVal(tools::toUtf8(bstr));
}

#define IMPL_SCALAR_2_SEXP(T) \
template<> static SEXP com2r<T>(IGPValue* pVal) \
{                                               \
  CComVariant v;                                \
  CComQIPtr<IGPVariant> ipGPVariant(pVal);      \
  ATLASSERT(ipGPVariant);                       \
  ipGPVariant->get_Variant(&v);                 \
  return tools::newVal(tools::castVar<T>(v));  \
}

IMPL_SCALAR_2_SEXP(bool)
IMPL_SCALAR_2_SEXP(int)
IMPL_SCALAR_2_SEXP(double)
IMPL_SCALAR_2_SEXP(std::wstring)

#define IMPL_SCALAR_2_VECTOR(T){ std::vector<T> a;             \
                                 fill_vector<T>(ipMValue, a);  \
                                 return tools::newVal(a);      \
                                }

template<>
static SEXP com2r<IGPMultiValue>(IGPValue* pVal)
{
  CComQIPtr<IGPMultiValue> ipMValue(pVal);
  CComPtr<IGPDataType> ipDataType;
  ipMValue->get_MemberDataType(&ipDataType);
  CComBSTR dt_name;
  ipDataType->get_Name(&dt_name);
  ATLTRACE(dt_name);
  long n = 0;
  ipMValue->get_Count(&n);
  //tools::protect pt;

  static_map::const_iterator it = get_smap().find(std::wstring(dt_name));
  if (it != get_smap().end())
  {
    if (it->second == &com2r<double>)
      IMPL_SCALAR_2_VECTOR(double)
    if (it->second == &com2r<int>)
      IMPL_SCALAR_2_VECTOR(int)
    if (it->second == &com2r<bool>)
      IMPL_SCALAR_2_VECTOR(bool)
    if (it->second == &com2r<std::wstring>)
      IMPL_SCALAR_2_VECTOR(std::wstring)

    tools::listGeneric a((size_t)n);
    for (long i = 0; i < n; i++)
    {
      CComPtr<IGPValue> ipV;
      ipMValue->get_Value(i, &ipV);
      a.push_back((*(it->second))(ipV));
    }
    return a.get();
  }
  std::vector<std::wstring> a(n);
  for (long i = 0; i < n; i++)
  {
    CComPtr<IGPValue> ipV;
    ipMValue->get_Value(i, &ipV);
    CComBSTR bstr;
    ipV->GetAsText(&bstr);
    a[i] = bstr;
  }
  return tools::newVal(a);
}

template<>
static SEXP com2r<IGPCoordinateSystem>(IGPValue* pVal)
{
  CComPtr<ISpatialReference> ipSR;
  CComQIPtr<IGPCoordinateSystem>(pVal)->get_SpatialReference(&ipSR);
  return spatial_reference2wkt(ipSR);
}
template<>
static SEXP com2r<IGPExtent>(IGPValue* pVal)
{
  tools::protect pt;
  CComPtr<IEnvelope> ipExt;
  esriGPExtentEnum extType;
  CComQIPtr<IGPExtent>(pVal)->GetExtent(&extType, &ipExt);
  if (extType == esriGPExtentValue)
    return extent2r(ipExt);
  return com2str(pVal);
}
template<>
static SEXP com2r<IGPSpatialReference>(IGPValue* pVal)
{
  tools::protect pt;
  CComPtr<ISpatialReference> ipSR;
  CComQIPtr<IGPSpatialReference>(pVal)->get_SpatialReference(&ipSR);
  return spatial_reference2wkt(ipSR);
}

static const static_map& get_smap()
{
  static static_map smap;
  if (smap.empty())
  {
    smap[L"GPDouble"]  = &com2r<double>;
    smap[L"GPLong"]    = &com2r<int>;
    smap[L"GPBoolean"] = &com2r<bool>;
    smap[L"GPString"]  = &com2r<std::wstring>;
    smap[L"GPSpatialReference"] = &com2r<IGPSpatialReference>;
    smap[L"GPCoordinateSystem"] = &com2r<IGPCoordinateSystem>;
    smap[L"GPExtent"]  = &com2r<IGPExtent>;
    smap[L"GPMultiValue"] = &com2r<IGPMultiValue>;
  }
  return smap;
}

VARIANT r2variant(SEXP r, VARTYPE vt)
{
  VARIANT v;
  ::VariantInit(&v);
  v.vt = vt;
  switch (vt)
  {
    case VT_I1: case VT_UI1:
      v.bVal = (BYTE)Rf_asInteger(r);
      return v;
    case VT_BOOL:
      v.boolVal = Rf_asLogical(r) ? VARIANT_TRUE : VARIANT_FALSE;
      return v;
    case VT_I4: case VT_UI4:
      v.lVal = Rf_asInteger(r);
      return v;
    case VT_R4:
      v.fltVal = (float)Rf_asReal(r);
      return v;
    case VT_R8:
      v.dblVal = Rf_asReal(r);
      return v;
    default:
      ATLASSERT(0);
    case VT_BSTR:
    {
      SEXP rs = Rf_asChar(r);
      std::wstring str;
      if (tools::copy_to(rs, str))
      {
        v.vt = VT_BSTR;
        v.bstrVal = CComBSTR(str.c_str()).Detach();
        return v;
      }
      v.vt = VT_NULL;
    }
  }
  return v;
}

static inline void add_as_variant(IGPMultiValue* pVals, const VARIANT &v)
{
  CComQIPtr<IGPVariant> ipVar; ipVar.CoCreateInstance(CLSID_GPVariant);
  ipVar->put_Variant(v);
  pVals->AddValue(CComQIPtr<IGPValue>(ipVar));
}

template<class T> void fill_vars(const std::vector<T> &a, IGPMultiValue* pVals)
{
  for (size_t i = 0, n = a.size(); i< n; i++)
  {
    CComVariant v(a[i]);
    add_as_variant(pVals, v);
  }
}
template<> void fill_vars<std::wstring>(const std::vector<std::wstring> &a, IGPMultiValue* pVals)
{
  for (size_t i = 0, n = a.size(); i< n; i++)
  {
    CComVariant v(a[i].c_str());
    add_as_variant(pVals, v);
  }
}

bool any2gpvalue(SEXP r, IGPValue* pVal)
{
  tools::protect pt;

  CComBSTR dt_name;
  CComPtr<IGPDataType> ipDataType;
  pVal->get_DataType(&ipDataType);
  ipDataType->get_Name(&dt_name);

  ATLTRACE("\nSEXP type:%s", Rf_type2char(TYPEOF(r)));

  HRESULT hr = 0;
  if (dt_name == L"GPMultiValue")
  {
    CComQIPtr<IGPMultiValue> ipMValue(pVal);
    long n = 0;
    ipMValue->get_Count(&n);
    while (--n >= 0)
      ipMValue->Remove(n);

    CComPtr<IGPDataType> ipDataType;
    ipMValue->get_MemberDataType(&ipDataType);
    CComBSTR dt_name;
    ipDataType->get_Name(&dt_name);
    ATLTRACE(L" - %s", (LPCWSTR)dt_name);

    n = (long)tools::size(r);
    if (n == 1)
    {
      SEXP x = pt.add(Rf_asChar(r));
      std::wstring str;
      if (tools::copy_to(x, str))
      {
        CComPtr<IGPValue> ipNewVal;
        ipDataType->CreateValue(CComBSTR(str.c_str()), &ipNewVal);
        hr = ipMValue->AddValue(ipNewVal);
        return hr == S_OK;
      }
      return false;
    }
    SEXP arr = pt.add(Rf_coerceVector(r, VECSXP));
    tools::vectorGeneric vect(arr);
    n = (long)vect.size();
    for (long i = 0; i < n; i++)
    {
      SEXP x = pt.add(Rf_asChar(vect.at(i)));
      std::wstring str;
      if (tools::copy_to(x, str))
      {
        CComPtr<IGPValue> ipNewVal;
        ipDataType->CreateValue(CComBSTR(str.c_str()), &ipNewVal);
        hr = ipMValue->AddValue(ipNewVal);
        if (hr != S_OK)
          return false;
      }
    }
    return true;
  }
  else
  {
    std::wstring str;
    if (!Rf_isNull(r))
    {
      SEXP x = pt.add(Rf_asChar(r));
      if (!tools::copy_to(x, str))
        return false;
    }
    CComQIPtr<IGPVariant> ipGPVar(pVal);
    if (ipGPVar)
      hr = ipGPVar->put_Variant(CComVariant(str.c_str()));
    else
      hr = pVal->SetAsText(CComBSTR(str.c_str()), NULL);
    return hr == S_OK;
  }
}

SEXP R_getEnv()
{
  CComPtr<IArray> ipEnvs;
  CComPtr<IGPUtilities2> ipGPUtil; ipGPUtil.CoCreateInstance(CLSID_GPUtilities);
  if (ipGPUtil == 0)
    return error_Ret("COM error. CoCreateInstance(CLSID_GPUtilities)");

  CComPtr<IGPVariableManager> ipVM;
  ipGPUtil->get_VariableManager(&ipVM);
  CComQIPtr<IGPEnvironmentManager> ipEM(ipVM);
  if (ipEM)
    ipEM->GetEnvironments(&ipEnvs);

  if (!ipEnvs)
    return R_NilValue;

  long n = 0;
  ipEnvs->get_Count(&n);
  tools::listGeneric vals(n);
  for (int i = 0; i < n; i++)
  {
    CComPtr<IUnknown> ipUnk;
    ipEnvs->get_Element(i, &ipUnk);
    CComQIPtr<IGPEnvironment> ipE(ipUnk);
    if (!ipE)
      continue;
    CComBSTR name;
    ipE->get_Name(&name);
    CComPtr<IGPValue> ipVal;
    ipE->get_Value(&ipVal);
    if (!ipVal) continue;

    vals.push_back(gpvalue2any(ipVal), tools::toUtf8((LPCWSTR)name).c_str());
  }
  return vals.get();
}
