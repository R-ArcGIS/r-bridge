#pragma once

SEXP R_AoInitialize();
SEXP error_Ret(const char* str_or_UTF8, SEXP retVal);
SEXP arc_error(SEXP e);
SEXP arc_warning(SEXP e);
//void arc_browsehelp(char ** url);
SEXP arc_progress_pos(SEXP arg);
SEXP arc_progress_label(SEXP arg);
SEXP extent2r(const std::vector<double> &ext);
bool r2variant(SEXP r, VARIANT &v);
SEXP R_getEnv();
SEXP R_fromWkt2P4(SEXP str);
SEXP R_fromP42Wkt(SEXP str);
double ole2epoch(double d);
double epoch2ole(double d);
