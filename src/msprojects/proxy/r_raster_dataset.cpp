#include "stdafx.h"
#include "r_raster_dataset.h"
#include "misc.h"
#include "r_raster.h"
using namespace rd;

const char* raster_dataset::class_name = "++raster_dataset++";
//static
SEXP raster_dataset::create_from(SEXP rHost, SEXP from)
{
  if (R_ExternalPtrTag(rtl::getExt(from)) == rr::raster::s_tag())
  {
    tools::protect pt;
    SEXP ptr = pt.add(rtl::createT<raster_dataset>(rHost));
    if (TYPEOF(ptr) == EXTPTRSXP)
    {
      auto r0 = rtl::getCObject<rr::raster>(from);
      auto pThis = reinterpret_cast<raster_dataset*>(R_ExternalPtrAddr(ptr));
      pThis->m_dataset = r0->m_dataset;
    }
    return ptr;
  }
  return dataset::create_from<raster_dataset>(rHost, from);
}

const char* raster_mosaic_dataset::class_name = "++raster_mosaic_dataset++";
//static
SEXP raster_mosaic_dataset::create_from(SEXP rHost, SEXP from)
{
  if (R_ExternalPtrTag(rtl::getExt(from)) == rr::raster::s_tag())
  {
    tools::protect pt;
    SEXP ptr = pt.add(rtl::createT<raster_mosaic_dataset>(rHost));
    if (TYPEOF(ptr) == EXTPTRSXP)
    {
      auto r0 = rtl::getCObject<rr::raster>(from);
      auto pThis = reinterpret_cast<raster_mosaic_dataset*>(R_ExternalPtrAddr(ptr));
      pThis->m_dataset = r0->m_dataset;
    }
    return ptr;
  }
  return dataset::create_from<raster_mosaic_dataset>(rHost, from);
}

/*SEXP raster_mosaic_dataset::get_fields()
{
  return forward_from_keyvalue_variant(m_dataset->get_fields());
}*/
