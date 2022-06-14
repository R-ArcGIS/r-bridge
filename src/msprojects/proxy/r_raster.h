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
  {"raster.create",         (DL_FUNC)(FN3)fn::R<decltype(raster::create), &raster::create, SEXP, SEXP, SEXP>, 3},
  {"raster.sr",             (DL_FUNC)(FN1)fn::R<raster, decltype(&raster::get_sr), &raster::get_sr>, 1},
  {"raster.rasterinfo",     (DL_FUNC)(FN1)fn::R<raster, decltype(&raster::get_rasterinfo), &raster::get_rasterinfo>, 1},
  {"raster.attribute_table",(DL_FUNC)(FN1)fn::R<raster, decltype(&raster::attribute_table), &raster::attribute_table>, 1},
  {"raster.update",         (DL_FUNC)(FN2)fn::R<raster, decltype(&raster::update), &raster::update, SEXP>, 2},
  {"raster.fill_pixelblock",(DL_FUNC)(FN4)fn::R<raster, decltype(&raster::fill_pixelblock), &raster::fill_pixelblock, SEXP, SEXP, SEXP>, 4},
  {"raster.begin",          (DL_FUNC)(FN2)fn::R<raster, decltype(&raster::write_begin), &raster::write_begin, SEXP>, 2},
  {"raster.commit",         (DL_FUNC)(FN2)fn::R<raster, decltype(&raster::commit), &raster::commit, SEXP>, 2},
  {"raster.write_pixelblock",(DL_FUNC)(FN3)fn::R<raster, decltype(&raster::write_pixelblock), &raster::write_pixelblock, SEXP, SEXP>, 3},
  {"raster.save_as",        (DL_FUNC)(FN3)fn::R<raster, decltype(&raster::save_as), &raster::save_as, SEXP, SEXP>, 3},
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
