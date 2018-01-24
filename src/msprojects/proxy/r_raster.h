#pragma once

#include "Rtl.h"
#include <memory>
#include <unordered_map>
namespace rr{
class raster : public rtl::object
{
protected:
  std::unique_ptr<arcobject::raster> m_raster;
public:
  static SEXP s_tag() { static SEXP tag = Rf_install("raster"); return tag;}
  std::shared_ptr<arcobject::dataset> m_dataset;

  //static const char* class_name;
  raster();

  BEGIN_CALL_MAP(raster)
  {"raster.create",   (DL_FUNC)R_fn3<raster::create>, 3},
  {"raster.sr",     (DL_FUNC)R_fn1<rtl::call_0<raster, &raster::get_sr>>, 1},
  {"raster.rasterinfo",  (DL_FUNC)R_fn1<rtl::call_0<raster, &raster::get_rasterinfo>>, 1},
  {"raster.attribute_table",  (DL_FUNC)R_fn1<rtl::call_0<raster, &raster::attribute_table>>, 1},
  {"raster.update",  (DL_FUNC)R_fn2<rtl::call_1<raster, &raster::update>>, 2},
  {"raster.fill_pixelblock", (DL_FUNC)R_fn4<rtl::call_3<raster, &raster::fill_pixelblock>>, 4},
  {"raster.begin", (DL_FUNC)R_fn2<rtl::call_1<raster, &raster::write_begin>>, 2},
  {"raster.commit", (DL_FUNC)R_fn2<rtl::call_1<raster, &raster::commit>>, 2},
  {"raster.write_pixelblock", (DL_FUNC)R_fn3<rtl::call_2<raster, &raster::write_pixelblock>>, 3},
  {"raster.save_as", (DL_FUNC)R_fn3<rtl::call_2<raster, &raster::save_as>>, 3},
  END_CALL_MAP()
private:
  static SEXP create(SEXP s4, SEXP source, SEXP args)
  {
    tools::protect pt;
    SEXP ptr = pt.add(rtl::createT<raster>(s4));
    if (TYPEOF(ptr) == EXTPTRSXP)
    {
      R_SetExternalPtrTag(ptr, s_tag());

      auto pThis = reinterpret_cast<raster*>(R_ExternalPtrAddr(ptr));//rtl::getCObject<T>(rHost);
      if (!pThis->initialize(source, args))
        return error_Ret(toUtf8(_api->getLastComError().c_str()).c_str());

    }
    return ptr;
  }
  bool initialize(SEXP dataset, SEXP args);
  SEXP update(SEXP args);
  SEXP get_sr();
  SEXP get_rasterinfo();
  SEXP fill_pixelblock(SEXP px, SEXP offset_dim, SEXP sbands);
  SEXP write_pixelblock(SEXP px, SEXP offset_dim);
  SEXP write_begin(SEXP opt);
  SEXP commit(SEXP opt);
  SEXP save_as(SEXP path, SEXP overwrite);
  SEXP attribute_table();

  bool update_by(std::unordered_map<std::string, SEXP> &args);
};
}
