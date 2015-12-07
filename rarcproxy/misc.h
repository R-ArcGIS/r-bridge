#pragma once

SEXP R_AoInitialize();
SEXP error_Ret(const char* str_or_UTF8, SEXP retVal);
SEXP arc_error(SEXP e);
SEXP arc_warning(SEXP e);
void arc_browsehelp(char ** url);
SEXP arc_progress_pos(SEXP arg);
SEXP arc_progress_label(SEXP arg);
double ole2epoch(double d);
double epoch2ole(double d);
