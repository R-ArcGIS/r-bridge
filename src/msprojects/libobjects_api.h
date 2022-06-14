#pragma once
#include <string>
#include <vector>
#include <array>
#include <functional>
//#include <type_traits>

#include <codecvt>
#include <memory>

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
    std::wstring license;
    std::array<unsigned short, 4> ver;
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
  typedef std::pair<std::wstring, int> sr_type;

  struct column_t2
  {
    enum class ct
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

    // {VT_ |VT_VECTOR, VT_BSTR|VT_VECTOR}
    std::pair<PROPVARIANT, PROPVARIANT> data = {{VT_EMPTY}, {VT_EMPTY}};

    // {VT_VARIANT|VT_VECTOR, VT_BSTR|VT_VECTOR}
    std::pair<PROPVARIANT, PROPVARIANT> attr = {{VT_EMPTY}, {VT_EMPTY}};
    ct t = ct::eNone;
    std::wstring name;
    ~column_t2()
    {
      auto hr = ::PropVariantClear(&data.first);
      ATLASSERT(hr == S_OK);
      hr = ::PropVariantClear(&data.second);
      ATLASSERT(hr == S_OK);
      hr = ::PropVariantClear(&attr.first);
      ATLASSERT(hr == S_OK);
      hr = ::PropVariantClear(&attr.second);
      ATLASSERT(hr == S_OK);
    }
  };
  class dataset;

  class ATL_NO_VTABLE raster
  {
  protected:
    raster() = default;
  public:
    virtual ~raster(){}
    virtual VARIANT get_sr() const = 0;
    virtual VARIANT get_info() const = 0;
    virtual std::vector<std::unique_ptr<column_t2>> get_attribute_table() const = 0;
    virtual bool set_prop(const std::string &name, const VARIANT &v) = 0;
    virtual bool set_sr(const sr_type& sr) = 0;

    virtual bool begin(int tlc_x, int tlc_y, unsigned int nrow, unsigned int ncol) = 0;
    virtual bool fill_data(void* data, unsigned int band, unsigned char bpp, unsigned int data_nrow, unsigned int data_ncol) const = 0;
    virtual bool write_data(const void* data, unsigned char bpp, unsigned int data_nrow, unsigned int data_ncol, int tlc_x, int tlc_y) = 0;
    virtual void end() = 0;
    virtual std::unique_ptr<dataset> commit(const std::string &opt) = 0;
    virtual bool save_as(const std::wstring &path) const = 0;
    virtual std::unique_ptr<raster> clone() const = 0;
  };

  class ATL_NO_VTABLE dataset
  {
  protected:
    dataset() = default;
  public:
    enum class geometry_kind
    {
      //ersi shape binary buffer
      eShapeBuffer = 0,
      //WKB binary buffer
      eWkb,
      //json geometry without SR
      eJson,
      //json geometry with SR
      eJsonSR
    };

    virtual ~dataset(){};
    virtual const char* get_type() const = 0;
    virtual bool is_table() const = 0;
    virtual bool is_fc() const = 0;
    virtual VARIANT get_properties_info() const = 0;
    virtual VARIANT get_metadata_info() const = 0;
    virtual VARIANT get_shape_info() const = 0;
    virtual VARIANT get_sr() const = 0;
    virtual VARIANT get_extent() const = 0;
    virtual VARIANT get_fields() const = 0;
    virtual std::pair<std::vector<std::wstring>, std::vector<std::string>> get_children() const = 0;
    //virtual raster* create_raster(const std::vector<int> &bands) const = 0;
    virtual std::vector<std::unique_ptr<column_t2>> select(const std::vector<std::wstring> &fld,
      const std::wstring &where_clause,
      bool use_selection,
      const sr_type &sr,
      bool densify,
      geometry_kind kind,
      const std::wstring &transformation) const = 0;
  };
  typedef std::pair<std::string, std::pair<std::wstring, int>> geometry_info;

  class ATL_NO_VTABLE cursor
  {
  public:
    virtual ~cursor(){}
    virtual long add_field(const std::wstring &name, const std::string &type, int max_len = 0) = 0;
    virtual bool begin() = 0;
    virtual bool setValue(int, const VARIANT &v) = 0;
    virtual bool set_point(const std::vector<double> &pts) = 0;
    virtual bool set_shape(const std::vector<byte> &shp) = 0;
    virtual bool next() = 0;
    virtual std::unique_ptr<dataset> commit() = 0;
    virtual const char* warnings() const = 0;
  };

  struct ATL_NO_VTABLE API
  {
    virtual const product_info& AoInitialize() const = 0;
    virtual std::unique_ptr<gp_execute> gp_begin_execute(const rconnect_interface *connect, IArray* pParams) const = 0;
    virtual bool gp_env_generate(std::function<bool(const std::wstring&, const VARIANT&)> const& fn) const = 0;

    virtual std::string fromWkt2P4(const std::string &wkt) const = 0;
    virtual std::string fromWkID2P4(int wktid) const = 0;
    virtual std::string fromP42Wkt(const std::string &p4) const = 0;
    virtual std::wstring getLastComError() const = 0;

    virtual bool is_dataset_exists(const std::wstring &path) const = 0;
    virtual bool delete_dataset(const std::wstring &path) const = 0;
    virtual std::unique_ptr<dataset> open_dataset(const std::wstring &path) const = 0;
    virtual std::unique_ptr<cursor> create_insert_cursor(const std::wstring &path, const geometry_info &geometry, bool simplify) const = 0;
    virtual std::unique_ptr<raster> create_raster(const dataset *pdataset, const std::vector<int> &bands) const = 0;
    virtual std::unique_ptr<raster> create_empty_raster(const std::wstring &path,
      double origin_x, double origin_y,
      double cellsize_x, double cellsize_y,
      int nrow, int ncol, int nband,
      int pixel_type, int compression_type,
      const sr_type &sr) const = 0;
    virtual bool portal_signon(const std::wstring &url, const std::wstring &username, const std::wstring &pwd, std::wstring &token) const = 0;
    virtual VARIANT portal_info() const = 0;
  };
  extern "C" { LIBRARY_API const API* api(bool InProc); }
}//arcobject namespace

#pragma warning(push)
#pragma warning(disable: 4996) // codecvt_utf8 was declared deprecated

static inline std::string toUtf8(const wchar_t* value)
{
  if (value == 0)
    return std::string();
  try
  {
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(value);
  }catch(...){ return std::string(); }
}

static inline std::wstring fromUtf8(const char* value)
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

#pragma warning(pop)
