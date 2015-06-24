#pragma once
#include <vector>
#include <string>
#include <limits>

class fl_base
{
protected:
  esriFieldType ft;
public:
  _bstr_t name;
  long idx;
  virtual ~fl_base(){}
  virtual void push(CComVariant &v) = 0;
  virtual void move_to(SEXP df) = 0;
  virtual SEXP getSEXP() const = 0;
};

template<class  T>
class fl : public fl_base
{
public:
  std::vector<T> vect1;
  fl(esriFieldType t, long id, const wchar_t* str)
  {
    ft = t;
    name = str;
    idx = id;
  }
  virtual void push(CComVariant &v) override;
  virtual void move_to(SEXP df) override;
  virtual SEXP getSEXP() const override;
};


HRESULT load_from_cursor(ICursor* pCursor, const std::vector<std::wstring> &fields, std::vector<fl_base*> &retColumns, ISpatialReference* pSRNew);

inline LPCSTR geometryType2str(esriGeometryType gt)
{
  static const char* type_names[]=
  {
    "Null",
    "Point",
    "Multipoint",
    "Polyline",
    "Polygon",
    "Envelope",
    "Path",
    "Any",
    "8",
    "MultiPatch",
    "10",
    "Ring",
    "12",
    "Line",
    "CircularArc",
    "Bezier3Curve",
    "EllipticArc",
    "Bag",
    "TriangleStrip",
    "TriangleFan",
    "Ray",
    "Sphere",
    "Triangles"
  };
  return type_names[gt];
}

inline esriGeometryType str2geometryType(LPCSTR str)
{
  if (str == NULL)
    return esriGeometryNull;
  size_t n = strlen(str);
  for (int i = (int)esriGeometryNull; i <= (int)esriGeometryTriangles; i++)
  {
    if (strncmp(str, geometryType2str((esriGeometryType)i), n) == 0)
      return (esriGeometryType)i;
  }
  return esriGeometryNull;
}