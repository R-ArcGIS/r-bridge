#pragma once

SEXP R_AoInitialize();
SEXP error_Ret(const char* str_or_UTF8, SEXP retVal);
SEXP arc_error(SEXP e);
SEXP arc_warning(SEXP e);
//void arc_browsehelp(char ** url);
SEXP arc_progress_pos(SEXP arg);
SEXP arc_progress_label(SEXP arg);
SEXP extent2r(const std::array<double, 4> &ext);
SEXP R_getEnv();
SEXP R_fromWkt2P4(SEXP str);
SEXP R_fromP42Wkt(SEXP str);
double ole2epoch(double d);
double epoch2ole(double d);
arcobject::sr_type from_sr(SEXP sr);
SEXP forward_from_keyvalue_variant(VARIANT &v);
std::wstring normalize_path(const std::wstring& path);
SEXP R_delete(SEXP spath);
SEXP arc_Portal(SEXP surl, SEXP suser, SEXP spw, SEXP stoken);
void print_sys_env();
