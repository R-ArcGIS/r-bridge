#include "stdafx.h"
#include "tools.h"
#include "gdal_helper.h"
#include "r_geometry.h"
#include <unordered_map>

//GDAL.dll dynamic load
typedef void* (__stdcall *fn_newSR)(const char*);
typedef void (__stdcall *fn_delSR)(void*);
typedef int (__stdcall *fn_exportP4)(void*, char**);

typedef int (*fn_importWKT)(void*, char**);
typedef int (*fn_importP4)(void*, char const*);
typedef void (*fn_free)(void*);
typedef int (*fn_morph)(void*);

struct gdal_exports
{
  fn_newSR pNewSR;
  fn_delSR delSR;
  fn_exportP4 exportP4;
  fn_exportP4 exportWKT;

  fn_importWKT importWKT;
  fn_importP4 importP4;
  fn_free _free;
  fn_morph morphFROM;
  fn_morph morphTO;
};

extern wchar_t arcgis_path[MAX_PATH];

//cashing
std::unordered_map<std::string, std::string> g_wkt2p4;
std::unordered_map<std::string, std::string> g_p4wkt;

const wchar_t* set_gdal_data_path()
{
  static wchar_t path[MAX_PATH] = {0};
  //set gdal data folder
  size_t n = 0;
  ::_wgetenv_s(&n, path, MAX_PATH, L"GDAL_DATA");
  if (n == 0)
  {
    wcscpy_s(path, arcgis_path);
    ::PathAppend(path, L"\\pedata\\gdaldata");
#ifndef _WIN64
    GetShortPathName(path, path, MAX_PATH);
#endif
    ::_wputenv_s(L"GDAL_DATA", path);
  }
  return path;
}

static HMODULE loadGDAL(gdal_exports &ex)
{
  std::wstring dll_path(arcgis_path);
  dll_path += L"\\bin\\gdal18.dll";
  HMODULE hDll = ::LoadLibraryEx(dll_path.c_str(), 0, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
  if (!hDll)return NULL;

  ex.pNewSR = (fn_newSR) GetProcAddress(hDll, "OSRNewSpatialReference");
  if (!ex.pNewSR)
    ex.pNewSR = (fn_newSR) GetProcAddress(hDll, "_OSRNewSpatialReference@4");
  ex.delSR = (fn_delSR)GetProcAddress(hDll, "OSRDestroySpatialReference");
  if (!ex.delSR)
    ex.delSR = (fn_delSR)GetProcAddress(hDll, "_OSRDestroySpatialReference@4");

  ex.exportP4 = (fn_exportP4)GetProcAddress(hDll, "OSRExportToProj4");
  if (!ex.exportP4)
    ex.exportP4 = (fn_exportP4)GetProcAddress(hDll, "_OSRExportToProj4@8");

  ex.exportWKT = (fn_exportP4)GetProcAddress(hDll, "OSRExportToWkt");
  if (!ex.exportWKT)
    ex.exportWKT = (fn_exportP4)GetProcAddress(hDll, "_OSRExportToWkt@8");

  ex.importWKT = (fn_importWKT)GetProcAddress(hDll, "OSRImportFromWkt");
  ex.importP4 = (fn_importP4)GetProcAddress(hDll, "OSRImportFromProj4");
  ex._free = (fn_free)GetProcAddress(hDll, "VSIFree");
  ex.morphFROM = (fn_morph)GetProcAddress(hDll, "OSRMorphFromESRI");
  ex.morphTO = (fn_morph)GetProcAddress(hDll, "OSRMorphToESRI");
  if (ex.pNewSR && ex.importWKT && ex.exportP4 && ex.delSR && ex._free)
    return hDll;
  FreeLibrary(hDll);
  return NULL;
}

inline static SEXP make_safe_return(const std::string& str)
{
  if (str.empty())
    return Rf_ScalarString(R_NaString);
  SEXP ret = tools::newVal(str);
  return ret == R_NaString ? Rf_ScalarString(R_NaString): ret;
}

static std::string fromWkt2P4(const std::string &wkt)
{
  const auto it = g_wkt2p4.find(wkt);
  if (it != g_wkt2p4.end())
    return it->second;

  gdal_exports ex;
  std::string p4;
  HMODULE hDll = loadGDAL(ex);
  if (hDll)
  {
    const char* ww = wkt.c_str();
    void* osr = ex.pNewSR(NULL);
    int e = 0;
    e = ex.importWKT(osr, (char**)&ww);
    if (ex.morphFROM)
      e = ex.morphFROM(osr);
    char* pj4 = 0;
    e = ex.exportP4(osr, &pj4);
    if (e == 0 && pj4)
      p4 = pj4;
    ex._free(pj4);
    ex.delSR(osr);
    //FreeLibrary(hDll);
  }

  p4.erase(p4.find_last_not_of(' ') + 1);
  if (!p4.empty() && !wkt.empty())
  {
    g_p4wkt[p4] = wkt;
    g_wkt2p4[wkt] = p4;
  }
  return p4;
}

static std::string fromP42Wkt(const std::string &p4)
{
  const auto it = g_p4wkt.find(p4);
  if (it != g_p4wkt.end())
    return it->second;

  gdal_exports ex;
  std::string wkt;
  HMODULE hDll = loadGDAL(ex);
  if (hDll)
  {
    void* osr = ex.pNewSR(NULL);
    char* wkt_str = 0;
    int e = 0;
    e = ex.importP4(osr, p4.c_str());
    if (ex.morphTO)
      e = ex.morphTO(osr);
    e = ex.exportWKT(osr, &wkt_str);
    if (e == 0 && wkt_str)
      wkt = wkt_str;
    ex._free(wkt_str);
    ex.delSR(osr);
    //FreeLibrary(hDll);
  }
  wkt.erase(wkt.find_last_not_of(' ') + 1);
  if (!p4.empty() && !wkt.empty())
  {
    g_p4wkt[p4] = wkt;
    g_wkt2p4[wkt] = p4;
  }
  return wkt;
}

static std::string fromWkID2P4(int wkid)
{
  CComPtr<ISpatialReference> ipSR;
  if (!create_sr(wkid, L"", &ipSR))
    return "";
  auto wkt = sr2wkt(ipSR);
  return fromWkt2P4(tools::toUtf8(wkt.c_str()));
}

SEXP R_fromWkt2P4(SEXP e)
{
  int wktid = 0;
  std::string p4;
  if (tools::copy_to(e, wktid))
  {
    p4 = fromWkID2P4(wktid);
  }
  else
  {
    std::string wkt;
    tools::copy_to(e, wkt);
    wkt.erase(wkt.find_last_not_of(' ') + 1);
    if (wkt.empty())
      return Rf_ScalarString(R_NaString);
    p4 = fromWkt2P4(wkt);
  }
  return make_safe_return(p4);
}

SEXP R_fromP42Wkt(SEXP str)
{
  std::string p4;
  tools::copy_to(str, p4);
  p4.erase(p4.find_last_not_of(' ') + 1);
  if (p4.empty())
    return Rf_ScalarString(R_NaString);

  auto wkt = fromP42Wkt(p4);
  return make_safe_return(wkt);
}


