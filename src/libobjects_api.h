#pragma once
#include <string>
#include <vector>
#include <functional>
//#include <type_traits>
#include <codecvt>

#ifndef LIBRARY_API
  #define LIBRARY_API _declspec(dllimport)
#endif

#ifndef ATL_NO_VTABLE
  #define ATL_NO_VTABLE __declspec(novtable)
#endif

struct IArray;
class rconnect_interface;
namespace arcobject
{
  struct product_info
  {
    std::string license;
    std::wstring version;
    std::wstring install_path;
  };
  class ATL_NO_VTABLE gp_execute
  {
  protected:
    gp_execute() = default;
  public:
    typedef std::vector<std::pair<VARIANT, std::wstring>>param_val;
    virtual ~gp_execute() = default;
    virtual const param_val& get_inputs() = 0;
    virtual const param_val& get_outputs() = 0;
    virtual bool update_output(int i, const VARIANT& val) = 0;
  };

  class ATL_NO_VTABLE dataset_handle
  {
  protected:
    dataset_handle() = default;
  public:
    typedef std::vector<std::pair<std::string, VARIANT>>shape_info;
    typedef std::pair<std::wstring, int> sr_type;
    virtual ~dataset_handle(){};
    virtual const char* get_type() const = 0;
    virtual bool is_table() const = 0;
    virtual bool is_fc() const = 0;
    LIBRARY_API static shape_info get_shape_info(const dataset_handle *ptr);
    LIBRARY_API static shape_info get_sr(const dataset_handle *ptr);
    LIBRARY_API static std::pair<std::vector<std::wstring>, std::vector<std::string>> get_fields(const dataset_handle *ptr);
    LIBRARY_API static std::pair<std::vector<std::wstring>, std::vector<std::string>> get_children(const dataset_handle *ptr);
    LIBRARY_API static std::vector<double> get_extent(const dataset_handle *ptr);

    struct column_t
    {
      enum ct
      {
        eNone = 0,
        eInt,
        eDouble,
        eDate,
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
      static std::vector<double>& get_doubles(column_t &c) { ATLASSERT(c.t == eDouble || c.t == eDate); return *static_cast<std::vector<double>*>(c.vals); }
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
    LIBRARY_API static std::vector<column_t*> select(const dataset_handle *ptr, const std::vector<std::wstring> &fld, const std::wstring& where_clause, bool use_selection, const sr_type& sr);
  };

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

  //exports
  LIBRARY_API const product_info& AoInitialize();
  LIBRARY_API gp_execute* gp_begin_execute(const rconnect_interface* connect, IArray* pParams);
  LIBRARY_API bool gp_env_generate(std::function<bool(const std::wstring&, const VARIANT &v)> const& fn);

  LIBRARY_API std::string fromWkt2P4(const std::string &wkt);
  LIBRARY_API std::string fromWkID2P4(int wktid);
  LIBRARY_API std::string fromP42Wkt(const std::string &p4);
  LIBRARY_API dataset_handle *open_dataset(const std::wstring &path);
  LIBRARY_API cursor* create_insert_cursor(const std::wstring &path, const geometry_info& geometry);

  LIBRARY_API std::string getLastComError();
};//arcobject


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

