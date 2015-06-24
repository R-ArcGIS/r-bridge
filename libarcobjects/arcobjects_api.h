#pragma once
#include <string>
#include <vector>
#include <functional>
//#include <type_traits>
#include <codecvt>

struct IArray;
class rconnect_interface;
namespace arcobject
{
  std::string getLastError();
  struct product_info
  {
    std::wstring license;
    std::wstring version;
    std::wstring install_path;
  };
  const product_info& AoInitialize();
  class ATL_NO_VTABLE gp_execute
  {
  protected:
    gp_execute(){};
  public:
    typedef std::vector<std::pair<VARIANT, std::wstring>>param_val;
    virtual ~gp_execute(){};
    static gp_execute* begin_execute(const rconnect_interface* connect, IArray* pParams);
    virtual const param_val& get_inputs() = 0;
    virtual const param_val& get_outputs() = 0;
    virtual bool update_output(int i, const VARIANT& val) = 0;
  };

  const wchar_t* arcgis_bin_path(const wchar_t* set_path = nullptr);
  bool gp_env_generate(std::function<bool(const std::wstring&, const VARIANT &v)> const& fn);

  class ATL_NO_VTABLE dataset_handle
  {
  protected:
    dataset_handle(){};
  public:
    typedef std::vector<std::pair<std::string, VARIANT>>shape_info;
    typedef std::pair<std::wstring, int> sr_type;
    virtual ~dataset_handle(){};
    virtual const char* get_type() const = 0;
    virtual bool is_table() const = 0;
    virtual bool is_fc() const = 0;
    static shape_info get_shape_info(const dataset_handle *ptr);
    static std::pair<std::vector<std::wstring>, std::vector<std::string>> get_fields(const dataset_handle *ptr);
    static std::vector<double> get_extent(const dataset_handle *ptr);

    struct column_t
    {
      enum ct
      {
        eNone = 0,
        eInt,
        eDouble,
        eString,
        eGeometry_point,
        eGeometry_multipoint,
        eGeometry_polyline,
        eGeometry_polygon,
      };
      const ct t;
      const std::wstring name;
      sr_type sr;
      column_t(ct e, const wchar_t* n, void* ptr):t(e),name(n),vals(ptr),sr(L"", 0){}
      virtual ~column_t(){ }
      static std::vector<int>& get_ints(column_t &c) { ATLASSERT(c.t == eInt); return *static_cast<std::vector<int>*>(c.vals); }
      static std::vector<double>& get_doubles(column_t &c) { ATLASSERT(c.t == eDouble); return *static_cast<std::vector<double>*>(c.vals); }
      static std::vector<std::string>& get_strings(column_t &c) { ATLASSERT(c.t == eString); return *static_cast<std::vector<std::string>*>(c.vals); }
      static std::vector<std::vector<double>>& get_points(column_t &c) { ATLASSERT(c.t == eGeometry_point); return *static_cast<std::vector<std::vector<double>>*>(c.vals); }
      static std::vector<std::vector<byte>>& get_shapes(column_t &c)
      {
        ATLASSERT(c.t == eGeometry_polyline || c.t == eGeometry_polygon || c.t == eGeometry_multipoint);
        return *static_cast<std::vector<std::vector<byte>>*>(c.vals);
      }
    protected:
      void* vals;
    };
    static std::vector<column_t*> select(const dataset_handle *ptr, const std::vector<std::wstring> &fld, const std::wstring& where_clause, bool use_selection, const sr_type& sr);
  };
  dataset_handle *open_dataset(const std::wstring &path);
  std::string fromWkt2P4(const std::string &wkt);
  std::string fromWkID2P4(int wktid);
  std::string fromP42Wkt(const std::string &p4);

  class ATL_NO_VTABLE cursor
  {
  public:
    virtual ~cursor(){}
    virtual long add_field(const std::wstring &name, const std::string &type) = 0;
    virtual bool begin() = 0;
    virtual bool setValue(int, const VARIANT &v) = 0;
    virtual bool set_point(const std::vector<double>& pts) = 0;
    virtual bool set_shape(const std::vector<byte>& shp) = 0;
    virtual bool next() = 0;
    virtual void commit() = 0;
  };
  typedef std::pair<std::string, std::pair<std::wstring, int>> geometry_info;
  cursor* create_insert_cursor(const std::wstring &path, const geometry_info& geometry);
}

inline static std::string toUtf8(const wchar_t* value)
{
  if (value == 0)
    return std::string();
  try
  {
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(value);
  }catch(...){ return std::string(); }
}

inline static std::wstring fromUtf8(const char* value)
{
  if (value == 0)
    return std::wstring();

  //some optimization
  if (value[0] == L'\n' && value[1] == 0)
    return std::wstring(1, L'\n');
  try
  {
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(value);
  }catch(...){ return std::wstring(); }
}

