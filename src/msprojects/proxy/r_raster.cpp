#include "stdafx.h"
#include "r_dataset.h"
#include "r_raster.h"
#include "misc.h"

using namespace rr;


raster::raster()
{
}

//arc.raster()
bool raster::initialize(SEXP source, SEXP sargs)
{
  using namespace arcobject;

  auto args = tools::pairlist2args_map(sargs);

  if (Rf_isNull(source)) //arc.raster("NULL", ...
  {
    std::wstring path;
    double origin_x = 0.0;
    double origin_y = 0.0;
    double cellsize_x = 0.0;
    double cellsize_y = 0.0;
    std::vector<int> dim;
    int pixel_type = 0;
    arcobject::sr_type sr;
    try
    {
      tools::unpack_args<true, true>(args,
        std::tie("path", path),
        std::tie("origin_x", origin_x),
        std::tie("origin_y", origin_y),
        std::tie("cellsize_x", cellsize_x),
        std::tie("cellsize_y", cellsize_y),
        std::tie("dim", dim),
        std::tie("pixel_type", pixel_type),
        std::tie("sr", sr));

      path = normalize_path(path);
      if (path.empty())
        throw std::out_of_range("path");
      if (dim.size() < 2)
        throw std::out_of_range("dim");
    }
    catch(const std::exception& e)
    {
      return error_Ret(e.what()), false;
    }
    //create empty raster

    int compression_type = -1;
    //get_arg(args, "compression_type", compression_type);
    bool overwrite = false;
    //get_arg(args, "overwrite", overwrite);

    tools::unpack_args<false, true>(args,
      std::tie("compression_type", compression_type),
      std::tie("overwrite", overwrite));

    if (_api->is_dataset_exists(path))
    {
      if (!overwrite)
        return showError<false>("dataset already exists"), false;
      if (!_api->delete_dataset(path))
        return showError<false>("failed to overwrite existing dataset. use arc.delete(path) or add overwrite=TRUE argument"), false;
    }

    auto r = _api->create_empty_raster(path,
      origin_x, origin_y,
      cellsize_x, cellsize_y,
      dim[0], dim[1], dim.size() == 3 ? dim[2] : 1,
      pixel_type, compression_type,
      sr);

    if (r == nullptr) return false;
    m_raster.reset(r);

    update_by(args);
    return true;
  }
  else
  {
    //from dataset or raster
    if (R_ExternalPtrTag(rtl::getExt(source)) == s_tag())
    {
      //this is r$copy()
      auto r0 = rtl::getCObject<raster>(source);
      m_dataset = r0->m_dataset;
      m_raster.reset(r0->m_raster->clone());
    }
    else
    {
      auto rd = rtl::getCObject<rd::dataset>(source);
      m_dataset = rd->m_dataset;

      std::vector<int> bands;
      auto is_bands = tools::unpack_args<false, true>(args, std::tie("bands", bands));
      if (std::get<0>(is_bands))
        return error_Ret(std::get<0>(is_bands) == 1 ? "missing argument : bands" : "incorrect type, argument : bands"), false;

      auto r = _api->create_raster(m_dataset.get(), bands);
      m_raster.reset(r);
    }
    if (m_raster.get() == nullptr) return false;
    update_by(args);
    return true;
  }
}

SEXP raster::update(SEXP sargs)
{
  auto args = tools::pairlist2args_map(sargs);
  return tools::newVal(update_by(args));
}

static inline void show_warning(const std::string &arg)
{
  std::string msg("incorrect parameter: ");
  msg.append(arg);
  Rf_warning(msg.c_str());
}

bool raster::update_by(tools::args_map_type &args)
{
  if (args.empty())
    return true;

  bool ret = false;
  arcobject::sr_type sp_ref;
  std::vector<double> extent;
  std::vector<byte> colormap;
  VARIANT v = {VT_EMPTY};

  auto is_args = tools::unpack_args<false, true>(args,
    std::tie("sr", sp_ref),
    std::tie("extent", extent),
    std::tie("colormap", colormap));

  if (std::get<0>(is_args) == 0)
  {
    ret = m_raster->set_sr(sp_ref);
    if (!ret) show_warning("sr");
  }

  if (std::get<1>(is_args) == 0 && extent.size() == 4)
  {
    v.vt = VT_R8 | VT_BYREF;
    v.pdblVal = &extent[0];
    ret = m_raster->set_prop("extent", v);
    if (!ret) show_warning("extent");
    v.vt = VT_EMPTY;
  }

  if (std::get<2>(is_args) == 0 && colormap.size() >= 3)
  {
    auto n = colormap.size() / 3;
    auto sa = ::SafeArrayCreateVector(VT_I4, 0, (long)n);
    DWORD *ptr = nullptr;
    ::SafeArrayAccessData(sa, (void**)&ptr);
    for (size_t i = 0, j = 0; i < n; i++)
    {
      auto r = colormap[j++];
      auto g = colormap[j++];
      auto b = colormap[j++];
      ptr[i] = RGB(r, g, b);
    }
    ::SafeArrayUnaccessData(sa);
    v.vt = VT_I4|VT_ARRAY;
    v.parray = sa;
    ret = m_raster->set_prop("colormap", v);
    if (!ret) show_warning("colormap");
    ::VariantClear(&v);
  }

  v.vt = VT_EMPTY;

  for (const auto it: args)
  {
    if (tools::r2variant(it.second, v, true))
    {
      ret = m_raster->set_prop(it.first, v);
      if (!ret) show_warning(it.first);
      ::VariantClear(&v);
    }
  }
  return ret;
}

SEXP raster::get_sr()
{
  ATLASSERT(m_raster.get() != nullptr);
  if (m_raster.get() != nullptr)
  {
    auto v = m_raster->get_sr();
    return forward_from_keyvalue_variant(v);
  }
  return R_NilValue;
}

SEXP raster::get_rasterinfo()
{
  ATLASSERT(m_raster.get() != nullptr);
  if (m_raster.get() != nullptr)
  {
    auto v = m_raster->get_info();
    return forward_from_keyvalue_variant(v);
  }
  return R_NilValue;
}

SEXP raster::fill_pixelblock(SEXP px, SEXP offset_dim, SEXP sbands)
{
  if (!Rf_isVector(px))
    return error_Ret("px is not a matrix");
  auto px_len = Rf_xlength(px);

  //SEXP sdim = Rf_getAttrib(px, R_DimSymbol);//GET_DIM(px);
  std::vector<int> dim;
  if (!(tools::copy_to(offset_dim, dim) && dim.size() == 4 && dim[2] > 0 && dim[3] > 0))
    return error_Ret("nrow > 0 && ncol > 0");
  std::vector<int> bands;
  if (!tools::copy_to(sbands, bands))
    return error_Ret("incorrect bands");

  auto st = TYPEOF(px);
  unsigned char bpp = 0;
  void *data = nullptr;
  switch (st)
  {
  case RAWSXP:
    bpp = sizeof(char);
    data = (void*)RAW(px);
    break;
  case INTSXP:
    bpp = sizeof(int);
    data = (void*)INTEGER(px);
    break;
  case REALSXP:
    bpp = sizeof(double);
    data = (void*)REAL(px);
    break;
  default:
    ATLASSERT(0);
    return error_Ret("array type is not 'raw' or 'int' or 'double'");
  }

  const int offset[2] = {dim[0], dim[1]};
  const unsigned int nrow = (unsigned int)dim[2];
  const unsigned int ncol = (unsigned int)dim[3];
  //const size_t row_ncell = ncol * bpp;
  const size_t band_ncell = nrow * ncol;
  const size_t nbands = bands.size();
  unsigned int block_nrow = std::min(nrow, 256U);

  if ((band_ncell * nbands) != px_len)
    return error_Ret("incorrect array size");

  const BYTE* end = (BYTE*)data + (px_len * bpp);


  //reading by stripes (256 rows)
  for (unsigned int offset_row = 0; offset_row < nrow; offset_row += block_nrow)
  {
    if ((offset_row + block_nrow) > nrow)
      block_nrow = nrow - offset_row;

    m_raster->begin(offset[1], offset[0] + offset_row, block_nrow, ncol);
    for (size_t band = 0; band < nbands; band++)
    {
      BYTE* ptr = (BYTE*)data + (band_ncell * band + offset_row * ncol)*bpp;
      ATLASSERT(ptr + (block_nrow * ncol * bpp) <= end);
      m_raster->fill_data(ptr, bands[band] - 1, bpp, block_nrow, ncol);
    }
  }

  if ((end - (BYTE*)data) > 4*1024*1024)
    m_raster->end();

  return px;
}

SEXP raster::write_begin(SEXP opt)
{
  return R_NilValue;
}

SEXP raster::commit(SEXP sopt)
{
  std::string opt;
  if (tools::copy_to(sopt, opt))
  {
    std::unique_ptr<arcobject::dataset> d(m_raster->commit(opt));
    if (d.get())
      m_dataset.reset(d.release());
  }
  return R_NilValue;
}

SEXP raster::write_pixelblock(SEXP px, SEXP offset_dim)
{
  if (!Rf_isVector(px))
    return error_Ret("px is not a matrix");
  auto px_len = Rf_xlength(px);

  std::vector<int> dim;
  if (!(tools::copy_to(offset_dim, dim) && dim.size() == 4 && dim[2] > 0 && dim[3] > 0))
    return error_Ret("nrow > 0 && ncol > 0");
  //std::vector<int> bands;
  //if (!tools::copy_to(sbands, bands))
  //  return error_Ret("incorrect bands");

  auto st = TYPEOF(px);
  unsigned char bpp = 0;
  void *data = nullptr;
  switch (st)
  {
  case RAWSXP:
    bpp = sizeof(char);
    data = (void*)RAW(px);
    break;
  case INTSXP:
    bpp = sizeof(int);
    data = (void*)INTEGER(px);
    break;
  case REALSXP:
    bpp = sizeof(double);
    data = (void*)REAL(px);
    break;
  default:
    ATLASSERT(0);
    return error_Ret("array type is not 'int' or 'double'");
  }
  const int offset[2] = {dim[0], dim[1]};
  const unsigned int nrow = (unsigned int)dim[2];
  const unsigned int ncol = (unsigned int)dim[3];

  bool ret = false;
  if (m_raster->begin(offset[1], offset[0], nrow, ncol))
  {
    ret = m_raster->write_data(data, bpp, nrow, ncol, offset[1], offset[0]);
  }
  if (!ret)
    return error_Ret("failed to write pixel block");
  return tools::newVal(ret);
}

SEXP raster::save_as(SEXP spath, SEXP overwrite)
{
  std::wstring path;
  if (!tools::copy_to(spath, path))
    return error_Ret("missing path");

  path = normalize_path(path);
  if (path.empty())
    return error_Ret("missing path");

  if (_api->is_dataset_exists(path))
  {
    bool b = false;
    tools::copy_to(overwrite, b);
    if (!b)
      return showError<false>("dataset already exists"), R_NilValue;
    if (!_api->delete_dataset(path))
      return showError<false>("failed to overwrite existing dataset. use arc.delete(path) or add overwrite=TRUE argument"), R_NilValue;
  }

  if (!m_raster->save_as(path))
    return error_Ret("save_as() failed");
  return tools::newVal(true);
}

SEXP raster::attribute_table()
{
  auto cols = m_raster->get_attribute_table();
  if (cols.empty())
    return showError<true>("COM error");
  tools::protect pt;
  std::pair<std::vector<SEXP>, std::vector<std::wstring>> process_cols(tools::protect &pt, std::vector<std::unique_ptr<arcobject::column_t2>> &cols);
  auto scols = process_cols(pt, cols);
  auto ret = pt.add(tools::newVal(scols.first));
  return tools::nameIt(ret, scols.second);
}
