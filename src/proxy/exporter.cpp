#include "stdafx.h"
#include "exporter.h"
#include "tools.h"
#include "misc.h"
#include <limits>
#include <memory>

class shape_extractor
{
  SEXP m_shape;
  SEXP m_parts[4];
  std::pair<std::string, std::pair<std::wstring, int>> m_geometry_info;
  size_t m_len;
  bool m_hasZ;
  bool m_hasM;
  bool m_as_matrix;
  bool m_points;
public:
  shape_extractor() :
        m_shape(NULL),
        m_len(0), m_hasZ(false), m_hasM(false),m_points(false),
        m_as_matrix(false){}

  long init(SEXP shape, SEXP shape_info);
  size_t size() const { return m_len;}
  const std::pair<std::string, std::pair<std::wstring, int>>& geometry_info() const { return m_geometry_info; }
  bool isPoints() const {return m_points; }
  std::vector<double> shape_extractor::getPoint(size_t i);
  std::vector<byte> shape_extractor::getShape(size_t i);
};

long shape_extractor::init(SEXP sh, SEXP sinfo)
{
  if (sh == NULL || Rf_isNull(sh))
    return S_FALSE;
  if (Rf_isNull(sinfo))
    return S_FALSE;

  m_shape = sh;
  tools::vectorGeneric shape_info(sinfo);
  tools::copy_to(shape_info.at("type"), m_geometry_info.first);
  m_geometry_info.second.second = 0;

  if (m_geometry_info.first.empty())
    return S_FALSE;

  tools::copy_to(shape_info.at("hasZ"), m_hasZ);
  tools::copy_to(shape_info.at("hasM"), m_hasM);

  if (m_geometry_info.first == "Point")//esriGeometryPoint)
  {
    m_points = true;
    if (Rf_isMatrix(m_shape))
    {
      if (TYPEOF(m_shape) != REALSXP)
        return showError<false>("expected type of double for shape list"), E_FAIL;E_FAIL;

      m_as_matrix = true;
      //return showError<false>("for Point geometry 'shape' shoud be matrix"), E_FAIL;E_FAIL;
      SEXP dims = Rf_getAttrib(sh, R_DimSymbol);
      size_t ndim = tools::size(dims);
      if (ndim != 2)
        return showError<false>("dimention != 2"), E_FAIL;

      m_len = (size_t)INTEGER(dims)[0];
      int c = INTEGER(dims)[1];
      if (m_hasZ && m_hasM && c != 4)
        return showError<false>("incorrect structue. matrix with 4 columns expected"), E_FAIL;
      if ((m_hasZ ^ m_hasM) && c != 3)
        return showError<false>("incorrect structue. matrix with 3 columns expected"), E_FAIL;
      if (!m_hasZ && !m_hasM && c != 2)
        return showError<false>("incorrect structue. matrix with 2 columns expected"), E_FAIL;
    }
    else if (Rf_isVectorList(m_shape))
    {
      m_as_matrix = false;
      size_t nn = 2;
      nn += m_hasZ ? 1 : 0;
      nn += m_hasM ? 1 : 0;
      size_t n = (R_len_t)tools::size(m_shape);
      if (n < nn)
        return showError<false>("incorrect list size for Point geometry"), E_FAIL;
      n = std::min(n, nn);
      //check all arrays must be same len
      for (size_t i = 0; i < n; i++)
      {
        m_parts[i] = VECTOR_ELT(sh, (R_len_t)i);
        if (TYPEOF(m_parts[i]) != REALSXP)
          return showError<false>("expected type of double for shape list"), E_FAIL;

        size_t n = tools::size(m_parts[i]);
        if (i == 0)
          m_len = n;
        else
        {
          if (m_len != n)
            return showError<false>("lists are not same sizes"), E_FAIL;
        }
      }
    }
    else
      return showError<false>("for Point geometry 'shape' shoud be matrix or list"), E_FAIL;
    if (m_hasZ)
      m_geometry_info.first += ":Z";
    if (m_hasM)
      m_geometry_info.first += ":M";
  }
  else
    m_len = tools::size(sh);

  tools::copy_to(shape_info.at("WKID"), m_geometry_info.second.second);
  if (m_geometry_info.second.second <= 0)
  {
    std::wstring wkt;
    if (tools::copy_to(shape_info.at("WKT"), wkt))
    {
      struct eq
      {
        static bool op(const wchar_t &c) { return c == L'\''; }
      };
      std::replace_if(wkt.begin(), wkt.end(), eq::op, L'\"');
      m_geometry_info.second.first = wkt;
    }
  }
  return S_OK;
}

std::vector<double> shape_extractor::getPoint(size_t i)
{
  std::vector<double> xyzm;
  ATLASSERT(m_points);
  if (!m_points)
    return xyzm;
  if (i >= m_len)
    return xyzm;

  if (i < m_len)
  {
    xyzm.resize(4, std::numeric_limits<double>::quiet_NaN());
    double z = std::numeric_limits<double>::quiet_NaN(), m = std::numeric_limits<double>::quiet_NaN();
    if (m_as_matrix)
    {
      ATLASSERT(Rf_isMatrix(m_shape));
      size_t r = m_len;//INTEGER(dims)[0];
      xyzm[0] = REAL(m_shape)[i];
      xyzm[1] = REAL(m_shape)[i + r];
      if (m_hasZ || m_hasM)
      {
        m = z = REAL(m_shape)[i + (r*2)];
        if (m_hasZ && m_hasM)
          m = REAL(m_shape)[i + (r*3)];
      }
    }
    else
    {
      xyzm[0] = REAL(m_parts[0])[i];
      xyzm[1] = REAL(m_parts[1])[i];
      if (m_hasZ || m_hasM)
      {
        double z, m;
        m = z = REAL(m_parts[2])[i];
        if (m_hasZ && m_hasM)
          m = REAL(m_parts[3])[i];
      }
    }
    if (m_hasZ) xyzm[2] = z;
    if (m_hasM) xyzm[3] = m;
  }
  return xyzm;
}

std::vector<byte> shape_extractor::getShape(size_t i)
{
  std::vector<byte> ret;
  SEXP it = 0;
  tools::vectorGeneric geometry(m_shape);
  it = geometry.at(i);
  if (Rf_isNull(it))
  {
    return ret;
  }
  else
  {
    if (TYPEOF(it) == NILSXP || Rf_isNumeric(it))
      return ret;
    else
    {
      if (!tools::copy_to(it, ret))
        return showError<false>("unknown structure"), ret;
      return ret;
    }
  }
}

class cols_base
{
public:
  long pos;
  virtual bool get(size_t i, VARIANT &v) const = 0;
};

template <class T, int sub = 0>
class cols_wrap : public cols_base
{
  SEXP vect;
  size_t len;
public:
  cols_wrap(SEXP sexp):vect(sexp), len(tools::size(sexp)) {}
  bool get(size_t i, VARIANT &v) const override;
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
bool cols_wrap<double, VT_R8>::get(size_t i, VARIANT &v) const
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
bool cols_wrap<double, VT_DATE>::get(size_t i, VARIANT &v) const
{
  if (i >= len)
    v.vt = VT_NULL;
  else
  {
    v.vt = VT_DATE;
    v.date = REAL(vect)[i];
    if (ISNAN(v.date))
      v.vt = VT_NULL;
    else
      v.date = epoch2ole(v.date);
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

  SEXP s = STRING_ELT(vect, i);
  if (s == NA_STRING)
    v.vt = VT_NULL;
  else 
  {
    v.vt = VT_BSTR;
    v.bstrVal = ::SysAllocString(fromUtf8(Rf_translateCharUTF8(s)).c_str());
  }
  return true;
}

static cols_base* setup_field(arcobject::cursor* cur, SEXP it, const std::wstring &str)
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
       item->pos = cur->add_field(str, "Integer");
       break;
     case REALSXP:
       if (Rf_inherits(it, "POSIXct"))
       {
         item = new cols_wrap<double, VT_DATE>(it);
         item->pos = cur->add_field(str, "Date");
       }
       else
       {
         item = new cols_wrap<double, VT_R8>(it);
         item->pos = cur->add_field(str, "Double");
       }
       break;
     case STRSXP:
     case CHARSXP:
       item = new cols_wrap<std::string>(it);
       item->pos = cur->add_field(str, "String");
       break;
     case LGLSXP: 
       item = new cols_wrap<bool>(it);
       item->pos = cur->add_field(str, "Integer");
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
    ~_cleanup()
    {
      for (size_t i = 0, n = c.size(); i < n; i++)
        delete c[i];
    }
  }cols;

  shape_extractor extractor;
  bool isShape = extractor.init(shape, shape_info) == S_OK;
  tools::getNames(dataframe, cols.name);

  R_xlen_t nlen = 0;
  ATLTRACE("dataframe type:%s", Rf_type2char(TYPEOF(dataframe)));

  if (Rf_isVectorList(dataframe))
  {
    size_t k = tools::size(dataframe);
    ATLASSERT(cols.name.size() == k);
    cols.name.resize(k);
    for (size_t i = 0; i < k; i++)
    {
      nlen = std::max(nlen, tools::size(VECTOR_ELT(dataframe, i)));
      if (cols.name[i].empty())
        cols.name[i] = L"data";
    }
  }
  else
  {
    if (Rf_isNull(dataframe))
      nlen = extractor.size();
    else
      return showError<false>("unsupported datat type"), R_NilValue;
  }

  if (nlen == 0)
    return showError<false>("nothing to save: 0 length"), R_NilValue;

  if (isShape && nlen != extractor.size() )
    Rf_warning("length of shape != data.frame length");
    //return showError<false>("length of shape != data.frame"), R_NilValue;

  std::unique_ptr<arcobject::cursor> acur(arcobject::create_insert_cursor(dataset_name, extractor.geometry_info()));
  if (acur.get() == NULL)
    return showError<true>(), R_NilValue;
  arcobject::cursor* cur = acur.get();

  for (size_t i = 0; i < cols.name.size(); i++)
  {
    ATLASSERT(!cols.name[i].empty());
    SEXP it = VECTOR_ELT(dataframe, i);
    bool skip = false;
    if (isShape)//if(gt == esriGeometryPolygon || gt == esriGeometryLine)
    {
      skip = cols.name[i] == L"Shape_Area";
      skip = !skip ? cols.name[i] == L"Shape_Length" : true;
    }
    if (!skip)
    {
      cols_base* item = setup_field(cur, it, cols.name[i]);
      if (!item)
        Rf_warning("unsupported data.field column type");//return showError<false>(L"unsupported data.field column type"), R_NilValue;
      else
        cols.c.push_back(item);
    }
  }

  if (!cur->begin())
    return showError<true>(), R_NilValue;
  for (R_xlen_t i = 0; i < nlen; i++)
  {
    //ATLTRACE("\n");
    for (const auto &c : cols.c)
    {
      if (c->pos < 0)
        continue;
      VARIANT val;
      ::VariantInit(&val);
      c->get(i, val);
      if (!cur->setValue(c->pos, val))
      {
        ::VariantClear(&val);
        return showError<true>("insert row value failed"), R_NilValue;
      }
      ::VariantClear(&val);
    }
    if (isShape)
    {
      if (extractor.isPoints())
        cur->set_point(extractor.getPoint(i));
      else
        cur->set_shape(extractor.getShape(i));
    }

    if (!cur->next())
      return showError<true>("insert row failed"), R_NilValue;
  }
  cur->commit();
  return R_NilValue;
}
