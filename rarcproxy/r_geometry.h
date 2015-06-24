#pragma once
#include "table_read.h"
#include "tools.h"

class r_geom
{
public:
  virtual bool add(IGeometry* g) = 0;
  virtual SEXP getSEXP(bool release) const = 0;
  CComPtr<ISpatialReference> m_ipSROverride;
};

static const char SHAPE_INFO[] = "shape_info";
class r_points : public r_geom
{
  mutable std::vector<double> m_x;
  mutable std::vector<double> m_y;
  mutable std::vector<double> m_m;
  mutable std::vector<double> m_z;
  bool is_z, is_m;
public:
  r_points(bool z, bool m) : is_z(z),is_m(m){}
  virtual bool add(IGeometry* g) override;
  virtual SEXP getSEXP(bool release) const override;
};

class r_parts: public r_geom
{
protected:
  std::string m_type;
  //mutable tools::listGeneric m_list;
  mutable tools::listGeneric m_list_raw;
  r_parts(const char *type) : m_type(type), m_list_raw(64)//, m_list(64)
  {}
public:
  virtual bool add(IGeometry* g) override;
  virtual SEXP getSEXP(bool release) const override;
};

class r_polyline: public r_parts
{
public:
  r_polyline() : r_parts("Polyline"){}
};
class r_polygon: public r_parts
{
public:
  r_polygon() : r_parts("Polygon"){}
};

class r_geometry : public fl_base
{
  CComPtr<IGeometryDef> ipGDef;
  esriGeometryType gt;
  std::auto_ptr<r_geom> m_sub;
public:
  r_geometry(esriFieldType t, long id, const wchar_t* str, IGeometryDef* pGDef, ISpatialReference* pSROverride);
  virtual void push(CComVariant &v) override
  {
    if (v.vt == VT_EMPTY)
      m_sub->add(NULL);
    else
      m_sub->add(CComQIPtr<IGeometry>(v.punkVal));
  }
  virtual void move_to(SEXP df) override;
  virtual SEXP getSEXP() const override { return m_sub->getSEXP(true); }
};

class shape_extractor
{
  SEXP m_shape;
  SEXP m_parts[4];
  CComPtr<ISpatialReference> m_ipSR;
  esriGeometryType m_gt;
  size_t m_len;
  bool m_hasZ;
  bool m_hasM;
  bool m_as_matrix;
public:
  shape_extractor() :
        m_shape(NULL),
        m_len(0), m_hasZ(false), m_hasM(false),
        m_gt(esriGeometryNull), 
        m_as_matrix(false) {}
  long init(SEXP shape, SEXP shape_info);

  size_t size() const { return m_len;}
  esriGeometryType type() const { return m_gt; };
  ISpatialReference* sr() const { return m_ipSR; }
  long at(size_t i, IGeometry **ppNewGeom);
};

#define FIX_DEFAULT_SR(sr) { sr->ConstructFromHorizon(); sr->SetDefaultXYResolution(); ipSRR->SetDefaultZResolution(); ipSRR->SetDefaultMResolution(); }

SEXP spatial_reference2wkt(ISpatialReference* pSR);
SEXP extent2r(IEnvelope* pEnv);
std::wstring sr2wkt(ISpatialReference* pSR);
bool create_sr(int wkid, std::wstring wkt, ISpatialReference **ppSR);
