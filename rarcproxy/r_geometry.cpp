#include "stdafx.h"
#include "r_geometry.h"

r_geometry::r_geometry(esriFieldType t, long id, const wchar_t* str, IGeometryDef* pGDef, ISpatialReference* pSROverride)
{
  ipGDef = pGDef;
  pGDef->get_GeometryType(&gt);
  ft = t;
  name = str;
  idx = id;

  VARIANT_BOOL bZ = VARIANT_FALSE;
  pGDef->get_HasZ(&bZ);
  VARIANT_BOOL bM = VARIANT_FALSE;
  pGDef->get_HasM(&bM);
  if (gt == esriGeometryPoint)
  {
    m_sub.reset(new r_points(bZ != VARIANT_FALSE, bM != VARIANT_FALSE));
  }
  else if (gt == esriGeometryPolyline)
  {
    m_sub.reset(new r_polyline);
  }
  else if (gt == esriGeometryPolygon)
  {
    m_sub.reset(new r_polygon);
  }
  else
  {
    ATLASSERT(0);
  }
  m_sub->m_ipSROverride = pSROverride;
}
/*
inline void get_xy(CComVariant &v, double &x, double &y)
{
  x = std::numeric_limits<double>::quiet_NaN();
  y = std::numeric_limits<double>::quiet_NaN();
  if (v.vt == VT_UNKNOWN)
  {
    CComQIPtr<IGeometry> ipGeom(v.punkVal);
    VARIANT_BOOL b = VARIANT_FALSE;
    ipGeom->get_IsEmpty(&b);
    if (b != VARIANT_FALSE)
      return;

    CComQIPtr<IPoint> ipPoint(ipGeom);
    if (ipPoint)
      ipPoint->QueryCoords(&x, &y);
    else
    {
      CComQIPtr<IGeometry5> ipG5(ipGeom);
      if (ipG5)
        ipG5->get_CentroidEx(&ipPoint);
      else
      {
        CComQIPtr<IArea> ipArea(ipGeom);
        if (ipArea)
          ipArea->get_Centroid(&ipPoint);
      }
      v = ipPoint;
      get_xy(v, x, y);
    }
  }
}*/

bool r_points::add(IGeometry* pGeom)
{
  double x = std::numeric_limits<double>::quiet_NaN();
  double y = std::numeric_limits<double>::quiet_NaN();
  double z = std::numeric_limits<double>::quiet_NaN();
  double m = std::numeric_limits<double>::quiet_NaN();
  
  if (pGeom != NULL)
  {
    VARIANT_BOOL b = VARIANT_FALSE;
    pGeom->get_IsEmpty(&b);
    if (b == VARIANT_FALSE)
    {
      CComQIPtr<IPoint> ipPoint(pGeom);
      ipPoint->QueryCoords(&x, &y);
      if (is_z)
        ipPoint->get_Z(&z);
      if (is_m)
        ipPoint->get_M(&m);
    }
  }
  m_x.push_back(x);
  m_y.push_back(y);
  if (is_z)
    m_z.push_back(z);
  if (is_m)
    m_m.push_back(m);

  return true;
}

SEXP r_points::getSEXP(bool release) const
{
  bool add_wkt = m_ipSROverride != nullptr;
  size_t len = 2 + (is_z ? 1 :0) + (is_m ? 1: 0) + (add_wkt ? 2 : 0);
  tools::listGeneric ret(len);

  ret.push_back(m_x, "x");
  ret.push_back(m_y, "y");
  if (is_z)
    ret.push_back(m_z, "z");
  if (is_m)
    ret.push_back(m_m, "m");

  if (add_wkt)
  {
    ret.push_back(spatial_reference2wkt(m_ipSROverride), "WKT");
    long srid = 0;
    m_ipSROverride ->get_FactoryCode(&srid);
    ret.push_back(srid, "WKID");
  }
  return ret.get();
}

void r_geometry::move_to(SEXP df)
{
  tools::protect pt;
  SEXP pair = tools::newPair(getSEXP(), R_NilValue, pt);
  tools::nameIt(pair, (LPCSTR)name);
  tools::push_back(df, pair);
}
/*
template<bool with_names>
SEXP one_line(std::vector<WKSPoint> xy)
{
  tools::protect pt;
  SEXP line = pt.add(Rf_allocMatrix(REALSXP, xy.size(), 2));
  for (std::size_t i = 0, n = xy.size(); i < n; i++)
  {
#ifdef __WKS_FWD_DEFINED__
    REAL(line)[i]     = xy[i].x;
    REAL(line)[i + n] = xy[i].y;
#else
    REAL(line)[i]     = xy[i].X;
    REAL(line)[i + n] = xy[i].Y;
#endif
  }
  if (with_names)
  {
    std::vector<SEXP> dimnames(2);
    dimnames[0] = R_NilValue;
    std::vector<std::string>cols(2);
    cols[0] = "x";
    cols[1] = "y";
    dimnames[1] = tools::newVal(cols, pt);
    return Rf_dimnamesgets(line, tools::newVal(dimnames, pt));
  }
  else
    return line;
}*/

bool r_parts::add(IGeometry* pGeom)
{
  tools::protect pt;
  if (pGeom == NULL)
  {
    m_list_raw.push_back(std::numeric_limits<double>::quiet_NaN());
    return true;
  }

  VARIANT_BOOL b = VARIANT_FALSE;
  pGeom->get_IsEmpty(&b);
  if (b != VARIANT_FALSE)
  {
    m_list_raw.push_back(std::numeric_limits<double>::quiet_NaN(), m_type);
    return true;
  }
  CComQIPtr<IESRIShape2> ipShape(pGeom);
  static const long exportFlags =
      esriShapeExportNoSwap |
      esriShapeExportDistanceDensify |
      esriShapeExportStripIDs |
      esriShapeExportStripZs |
      esriShapeExportStripMs;

  long buffSize = 0;
  ipShape->get_ESRIShapeSizeEx2(exportFlags, &buffSize);
  if (buffSize)
  {
    try
    {
      std::vector<BYTE> buff(buffSize);
      BYTE *ptr = &buff[0];
      if (FAILED(ipShape->ExportToESRIShapeEx2(exportFlags, &buffSize, ptr)))
        throw;
      m_list_raw.push_back(buff, m_type);
    }catch(...)
    {
      m_list_raw.push_back(std::numeric_limits<double>::quiet_NaN(), m_type);
    }
  }
  else
    m_list_raw.push_back(std::numeric_limits<double>::quiet_NaN(), m_type);

  return true;
}

SEXP r_parts::getSEXP(bool release) const
{
  if (m_ipSROverride != nullptr)
  {
    m_list_raw.push_back(spatial_reference2wkt(m_ipSROverride), "WKT");
    long srid = 0;
    m_ipSROverride ->get_FactoryCode(&srid);
    m_list_raw.push_back(srid, "WKID");
  }
  return m_list_raw.get();
}

bool create_sr(int wkid, std::wstring wkt, ISpatialReference **ppSR)
{
  CComPtr<ISpatialReferenceFactory3> ipSpatialRefFactory; ipSpatialRefFactory.CoCreateInstance(CLSID_SpatialReferenceEnvironment);
  CComPtr<ISpatialReference> ipSR;

  if (wkid > 0)
  {
    ipSpatialRefFactory->CreateSpatialReference(wkid, &ipSR);
  }
  else
  {
    if (!wkt.empty())
    {
      long dw;
      struct eq
      {
        static bool op(const wchar_t &c) { return c == L'\''; }
      };
      std::replace_if(wkt.begin(), wkt.end(), eq::op, L'\"');
      CComBSTR bstr(wkt.c_str());
      HRESULT hr = ipSpatialRefFactory->CreateESRISpatialReference(bstr, &ipSR, &dw);
      if (FAILED(hr))
      {
        CComPtr<ISpatialReferenceInfo> ipSRInfo;
        if (FAILED(ipSpatialRefFactory->CreateESRISpatialReferenceInfoFromPRJ(bstr, &ipSRInfo)))
          return false;
        long fcode = 0;
        ipSRInfo->get_FactoryCode(&fcode);
        if (FAILED(ipSpatialRefFactory->CreateSpatialReference(fcode, &ipSR)))
          return false;
      }
    }
  }
  if (ipSR == NULL)
    ipSR.CoCreateInstance(CLSID_UnknownCoordinateSystem);
  CComQIPtr<ISpatialReferenceResolution> ipSRR(ipSR);
  if (ipSRR)
    FIX_DEFAULT_SR(ipSRR);
  *ppSR = ipSR.Detach();
  return *ppSR != nullptr;
}

inline long newShape(esriGeometryType gt, ISpatialReference* pSR, bool hasZ, bool hasM, IGeometry **ppOut)
{
  CComPtr<IGeometry> ipGeom;
  HRESULT hr = E_NOTIMPL;
  const CLSID *guid = NULL;
  switch (gt)
  {
    default:
      ATLASSERT(0);//TODO
      return E_NOTIMPL;
    case esriGeometryPolyline:
      guid = &CLSID_Polyline;
      break;
    case esriGeometryPolygon:
      guid = &CLSID_Polygon;
      break;
    case esriGeometryMultipoint:
      guid = &CLSID_Multipoint;
      break;
    case esriGeometryPoint:
      guid = &CLSID_Point;
      break;
  }

  //make empty
  hr = ipGeom.CoCreateInstance(*guid);
  if (hr != S_OK)
    return hr;

  ipGeom->SetEmpty();

  if (hasZ)
  {
    CComQIPtr<IZAware> ipZ(ipGeom);
    if (ipZ)
      ipZ->put_ZAware(VARIANT_TRUE);
  }
  if (hasM)
  {
    CComQIPtr<IMAware> ipM(ipGeom);
    if (ipM)
      ipM->put_MAware(VARIANT_TRUE);
  }
  if (pSR)
    ipGeom->putref_SpatialReference(pSR);

  return ipGeom.CopyTo(ppOut);
}

//////////////
long shape_extractor::init(SEXP sh, SEXP sinfo)
{
  if (sh == NULL || Rf_isNull(sh))
    return S_FALSE;
  //SEXP sinfo = Rf_getAttrib(sh, Rf_install(SHAPE_INFO));
  if (Rf_isNull(sinfo))
    return S_FALSE;

  m_shape = sh;
  tools::vectorGeneric shape_info(sinfo);
  std::string gt_type;
  tools::copy_to(shape_info.at("type"), gt_type);
  m_gt = str2geometryType(gt_type.c_str());
  if (m_gt == esriGeometryNull)
    return S_FALSE;

  tools::copy_to(shape_info.at("hasZ"), m_hasZ);
  tools::copy_to(shape_info.at("hasM"), m_hasM);

  //m_len = tools::size(m_shape);
  if (m_gt == esriGeometryPoint)
  {
    if (Rf_isMatrix(m_shape))
    {
      if (TYPEOF(m_shape) != REALSXP)
        return showError<false>(L"expected type of double for shape list"), E_FAIL;E_FAIL;

      m_as_matrix = true;
      //return showError<false>(L"for Point geometry 'shape' shoud be matrix"), E_FAIL;E_FAIL;
      SEXP dims = Rf_getAttrib(sh, R_DimSymbol);
      size_t ndim = tools::size(dims);
      if (ndim != 2)
        return showError<false>(L"dimention != 2"), E_FAIL;

      m_len = (size_t)INTEGER(dims)[0];
      int c = INTEGER(dims)[1];
      if (m_hasZ && m_hasM && c != 4)
        return showError<false>(L"incorrect structue. matrix with 4 columns expected"), E_FAIL;
      if ((m_hasZ ^ m_hasM) && c != 3)
        return showError<false>(L"incorrect structue. matrix with 3 columns expected"), E_FAIL;
      if (!m_hasZ && !m_hasM && c != 2)
        return showError<false>(L"incorrect structue. matrix with 2 columns expected"), E_FAIL;
    }
    else if (Rf_isVectorList(m_shape))
    {
      m_as_matrix = false;
      size_t nn = 2;
      nn += m_hasZ ? 1 : 0;
      nn += m_hasM ? 1 : 0;
      size_t n = (R_len_t)tools::size(m_shape);
      if (n < nn)
        return showError<false>(L"incorrect list size for Point geometry"), E_FAIL;
      n = std::min(n, nn);
      //check all arrays must be same len
      for (size_t i = 0; i < n; i++)
      {
        m_parts[i] = VECTOR_ELT(sh, (R_len_t)i);
        if (TYPEOF(m_parts[i]) != REALSXP)
          return showError<false>(L"expected type of double for shape list"), E_FAIL;E_FAIL;

        size_t n = tools::size(m_parts[i]);
        if (i == 0)
          m_len = n;
        else
        {
          if (m_len != n)
            return showError<false>(L"lists are not same sizes"), E_FAIL;
        }
      }
    }
    else return showError<false>(L"for Point geometry 'shape' shoud be matrix or list"), E_FAIL;E_FAIL;

  }
  else
    m_len = tools::size(sh);

  //CComPtr<ISpatialReferenceFactory3> ipSpatialRefFactory; ipSpatialRefFactory.CoCreateInstance(CLSID_SpatialReferenceEnvironment);
  int srid = 0;
  std::wstring wkt;
  m_ipSR.Release();
  if (!tools::copy_to(shape_info.at("WKID"), srid))
    tools::copy_to(shape_info.at("WKT"), wkt);

  create_sr(srid, wkt, &m_ipSR);
  return S_OK;
}

long shape_extractor::at(size_t i, IGeometry **ppNewGeom)
{
  if (m_gt == esriGeometryNull)
    return S_FALSE;

  CComQIPtr<IGeometry> ipNewShape;
  if (m_gt == esriGeometryPoint)
  {
    HRESULT hr = newShape(m_gt, m_ipSR, m_hasZ, m_hasM, &ipNewShape);
    if (hr != S_OK)
      return showError<true>(L"create new geometry failed"), hr;
    CComQIPtr<IPoint> ipPoint(ipNewShape);

    double x, y, z, m;
    if (m_as_matrix)
    {
      ATLASSERT(Rf_isMatrix(m_shape));
      size_t r = m_len;//INTEGER(dims)[0];
      x = REAL(m_shape)[i];
      y = REAL(m_shape)[i + r];
      if (m_hasZ || m_hasM)
      {
        m = z = REAL(m_shape)[i + (r*2)];
        if (m_hasZ && m_hasM)
          m = REAL(m_shape)[i + (r*3)];
      }
    }
    else
    {
      x = REAL(m_parts[0])[i];
      y = REAL(m_parts[1])[i];
      if (m_hasZ || m_hasM)
      {
        double z, m;
        m = z = REAL(m_parts[2])[i];
        if (m_hasZ && m_hasM)
          m = REAL(m_parts[3])[i];
      }
    }
    ipPoint->PutCoords(x, y);
    if (m_hasZ) ipPoint->put_Z(z);
    if (m_hasM) ipPoint->put_M(m);

    return ipNewShape.CopyTo(ppNewGeom);
  }

  SEXP it = 0;
  tools::vectorGeneric geometry(m_shape);
  HRESULT hr = newShape(m_gt, m_ipSR, false, false, &ipNewShape);
  if (hr != S_OK)
    return showError<true>(L"create new geometry failed"), hr;

  it = geometry.at(i);
  if (Rf_isNull(it))
  {
    ipNewShape->SetEmpty();
  }
  else
  {
    HRESULT hr = S_FALSE;
    if (TYPEOF(it) == NILSXP || Rf_isNumeric(it))
      hr = ipNewShape->SetEmpty();
    else
    {
      std::vector<BYTE> buff;
      if (!tools::copy_to(it, buff))
        return showError<false>(L"unknown structure"), E_FAIL;
      CComQIPtr<IESRIShape2> ipShape(ipNewShape);
      long buffSize = (long)buff.size();
      hr = ipShape->ImportFromESRIShapeEx(esriShapeImportNoSwap | esriShapeImportNonTrusted, &buffSize, &buff[0]);
    }
    if (hr != S_OK)
      return showError<true>(L"create new geometry"), hr;
  }
  return ipNewShape.CopyTo(ppNewGeom);
}

std::wstring sr2wkt(ISpatialReference* pSR)
{
  CComQIPtr<IESRISpatialReference> ipESR(pSR);
  if (ipESR == NULL) return L"";
  long n = 0;
  ipESR->get_ESRISpatialReferenceSize(&n);
  CComBSTR wkt(n);
  ipESR->ExportToESRISpatialReference(wkt, &n);
  return (LPCWSTR)wkt;
}

SEXP spatial_reference2wkt(ISpatialReference* pSR)
{
  auto wkt = sr2wkt(pSR);
  if (wkt.empty())
    return R_NilValue;

  return tools::newVal(wkt);
}

SEXP extent2r(IEnvelope* pEnv)
{
  if (!pEnv) return R_NilValue;

  std::vector<double> minmax(4);
  std::vector<std::string> names(minmax.size());
  names[0] = "xmin";
  names[1] = "ymin";
  names[2] = "xmax";
  names[3] = "ymax";
  pEnv->QueryCoords(&minmax[0], &minmax[1], &minmax[2], &minmax[3]);
  return tools::nameIt(tools::newVal(minmax), names);
}

