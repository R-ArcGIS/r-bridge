#pragma once

SEXP R_getEnv();

SEXP gpvalue2any(IGPValue* pVal);
bool any2gpvalue(SEXP r, IGPValue* pVal);

