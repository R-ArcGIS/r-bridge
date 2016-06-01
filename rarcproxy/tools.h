#pragma once
#include <vector>
#include <algorithm>
#include <string>
#include <limits>

namespace tools
{
  inline bool getCOMError(BSTR* strErr)
  {
    CComQIPtr<IErrorInfo> ipIErrorInfo;
    if (::GetErrorInfo(0, &ipIErrorInfo) != S_OK)
      return false;

    ipIErrorInfo->GetDescription(strErr);
    ATLTRACE(L"\n>>>>COM ERROR: %s", (LPCWSTR)*strErr);
    return ::SysStringLen(*strErr) != 0;
  }

  inline const std::string toUtf8(const wchar_t* value)
  {
    if (value == 0)
      return std::string();

    int n = ::WideCharToMultiByte(CP_UTF8, 0, value, -1, NULL, 0, NULL, NULL);
    if (n == 0)
      return std::string();
    std::string result;
    result.resize(n - 1);
    ::WideCharToMultiByte(CP_UTF8, 0, value, -1, &(result[0]), (int)result.size(), NULL, NULL);
    return result;
  }
  inline std::wstring fromUtf8(const char* value)
  {
    if (value == 0)
      return std::wstring();
    //some optimization
    if (value[0] == L'\n' && value[1] == 0)
      return std::wstring(1, L'\n');

    int n = ::MultiByteToWideChar(CP_UTF8, 0, value, -1, NULL, 0);
    if (n == 0)
      return std::wstring();

    try{
    std::wstring result(n - 1, 0);
    //result.resize(n - 1);
    ::MultiByteToWideChar(CP_UTF8, 0, value, -1, &(result[0]), (int)result.size());
    return result;
    }catch(...){ return std::wstring();}
  }

  class protect
  {
    protect(const protect&);
    int n;
  public:
    protect(): n(0) {}
    protect(SEXP v): n(0) { add (v); }
    ~protect(){ release(); }
    inline SEXP add(SEXP v)
    { 
      if (v == R_NilValue) 
        return v; 
      try { PROTECT(v); n++; } catch(...){ return R_NilValue; } return v;}
    inline void release(){ try { if (n) UNPROTECT(n); }catch(...){} n = 0; }
  };
  class preserved
  {
    preserved(const preserved&);
  protected:
    SEXP sexp;
  public:
    preserved() : sexp(R_NilValue) {}
    preserved(SEXP v) : sexp(R_NilValue) { set(v); }
    ~preserved(){ release(); }
    inline void set(SEXP v) 
    {
      if (v == sexp) return;
      release();
      if (v != R_NilValue)
      {
        try {
          ::R_PreserveObject(v);
          sexp = v;
        }catch(...){}
      }
    }
    inline SEXP release()
    {
      SEXP ret = sexp;
      if (sexp != R_NilValue)
      {
        try { ::R_ReleaseObject(sexp); } catch(...){}
        sexp = R_NilValue;
      }
      return ret;
    }
    SEXP get() const { return sexp; }
  };

  inline static const std::wstring tolower(const std::wstring& str)
  {
    std::wstring tmp(str);
    std::transform(str.begin(), str.end(), tmp.begin(), ::towlower);
    return tmp;
  }

  inline bool copy_to(SEXP sexp, std::string &str_out)
  {
    if (!sexp) return false;
    if (TYPEOF(sexp) != STRSXP)
    {
      if (TYPEOF(sexp) == CHARSXP)
      {
        str_out = CHAR(sexp);
        return true;
      }
      ATLTRACE("not str:%s", Rf_type2char(TYPEOF(sexp)));
      return false;
    }

    if (Rf_length(sexp) < 1)
       return false;
    str_out.assign(Rf_translateCharUTF8(STRING_ELT(sexp, 0)));
    //str_out.assign(Rf_translateChar(STRING_ELT(sexp, 0)));
    return true;
  }
  inline bool copy_to(SEXP sexp, std::wstring &str_out)
  {
    if (!sexp) return false;
    std::string str;
    if (!copy_to(sexp, str))
      return false;

    str_out = fromUtf8(str.c_str());
    return true;
  }

  inline bool copy_to(SEXP sexp, double &out)
  {
    if (!sexp) return false;
    if (TYPEOF(sexp) != REALSXP)
    {
      ATLTRACE("SEXP type:%s", Rf_type2char(TYPEOF(sexp)));
      return false;
    }

    if (Rf_length(sexp) < 1)
      return false;

    out = REAL(sexp)[0];
    return true;
  }

  inline bool copy_to(SEXP sexp, int &out)
  {
    if (!sexp) return false;
    if (TYPEOF(sexp) != INTSXP)
    {
      double tmp = 0.0;
      if (TYPEOF(sexp) == REALSXP && copy_to(sexp, tmp) && tmp == (int)tmp)
      {
        out = (int)tmp;
        return true;
      }
      ATLTRACE("SEXP type:%s", Rf_type2char(TYPEOF(sexp)));
      return false;
    }
    out = INTEGER(sexp)[0];
    return true;
  }

  inline bool copy_to(SEXP sexp, bool &out)
  {
    if (!sexp) return false;
    //if (TYPEOF(sexp) != LGLSXP && Rf_xlength(sexp) < 1)
    //   return false;
    if (Rf_isLogical(sexp) && Rf_xlength(sexp) > 0)
      out = Rf_asLogical(sexp) != 0;//Rboolean::FALSE;
    else
    {
      ATLTRACE("SEXP type:%s", Rf_type2char(TYPEOF(sexp)));
      return false;//out = (b == NA_LOGICAL) ? true : (Rboolean) b;
    }
    //out = LOGICAL(sexp)[0] ? true : false;
    return true;
  }

  inline bool copy_to(SEXP sexp, std::vector<byte>& out)
  {
    if (!sexp) return false;
    if (TYPEOF(sexp) != RAWSXP)
    {
      ATLTRACE("SEXP type:%s", Rf_type2char(TYPEOF(sexp)));
      return false;
    }
    try {
      out.resize((std::size_t)Rf_xlength(sexp));
      for (std::size_t i = 0, n = out.size(); i < n; i++)
        out[i] = RAW(sexp)[i];
    }catch(...){ return false; }
    return true;
  }

  inline bool copy_to(SEXP sexp, std::vector<std::string>& out)
  {
    if (!sexp) return false;
    if (TYPEOF(sexp) != STRSXP)
    {
      ATLTRACE("SEXP type:%s", Rf_type2char(TYPEOF(sexp)));
      return false;
    }
    try {
    out.resize((std::size_t)Rf_xlength(sexp));
    for (R_len_t i = 0, n = (R_len_t)out.size(); i < n; i++)
      out[i].assign(Rf_translateChar(STRING_ELT(sexp, i)));
    }catch(...){ return false; }
    return true;
  }

  inline bool copy_to(SEXP sexp, std::vector<std::wstring>& out)
  {
    if (!sexp) return false;
    if (TYPEOF(sexp) != STRSXP)return false;
    try {
    out.resize((std::size_t)Rf_xlength(sexp));
    for (R_len_t i = 0, n = (R_len_t)out.size(); i < n; i++)
      out[i] = fromUtf8(Rf_translateChar(STRING_ELT(sexp, i)));
    }catch(...){ return false; }
    return true;
  }

  inline bool copy_to(SEXP sexp, std::vector<int>& out)
  {
    if (!sexp) return false;
    if (TYPEOF(sexp) != INTSXP)  return false;
    try {
    out.resize((std::size_t)Rf_xlength(sexp));
    for (std::size_t i = 0, n = out.size(); i < n; i++)
      out[i] = INTEGER(sexp)[i];
    }catch(...){ return false; }
    return true;
  }

  inline bool copy_to(SEXP sexp, std::vector<double>& out)
  {
    if (!sexp) return false;
    if (TYPEOF(sexp) != REALSXP) return false;
    try {
    out.resize((std::size_t)Rf_xlength(sexp));
    for (std::size_t i = 0, n = out.size(); i < n; i++)
      out[i] = REAL(sexp)[i];
    }catch(...){ return false; }
    return true;
  }

  inline bool copy_to(SEXP sexp, std::vector<bool>& out)
  {
    if (!sexp) return false;
    if (TYPEOF(sexp) != LGLSXP)
    {
      ATLTRACE("SEXP type:%s", Rf_type2char(TYPEOF(sexp)));
      return false;
    }

    try {
    out.resize((std::size_t)Rf_xlength(sexp));
    for (std::size_t i = 0, n = out.size(); i < n; i++)
      out[i] = LOGICAL(sexp)[i] ? true : false;
    }catch(...){ return false; }
    return true;
  }

  inline SEXP newVal(const char* str)
  {
    //return Rf_mkString(str);
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(STRSXP, 1));
    SET_STRING_ELT(sexp, (R_len_t)0, Rf_mkCharCE(str, CE_UTF8));
    return sexp;
  }
  inline SEXP newVal(const std::string& str)
  {
    return newVal(str.c_str());
  }

  inline SEXP newVal(const std::wstring& str)
  {
    return newVal(toUtf8(str.c_str()));
  }

  inline SEXP newVal(int value)
  {
    return Rf_ScalarInteger(value);
  }
  inline SEXP newVal(double value)
  {
    return Rf_ScalarReal(value);
  }
  /*inline SEXP newVal(bool value)
  {
    return pt.add(Rf_ScalarLogical(value ? TRUE : FALSE));
    //SEXP sexp = pt.add(Rf_allocVector(LGLSXP, 1));
    //*(LOGICAL(sexp)) = value ? TRUE : FALSE;
    //return sexp;
  }*/
  inline SEXP newVal(bool value)
  {
    return Rf_ScalarLogical(value ? TRUE : FALSE);
  }

  inline SEXPTYPE vartype2rtype(VARTYPE vt)
  {
    switch(vt)
    {
      case VT_NULL: return NILSXP;
      case VT_BOOL: return LGLSXP;
      case VT_I4:   return INTSXP;
      case VT_R8:   return REALSXP;
      case VT_BSTR: return CHARSXP;
      case VT_VARIANT: return VECSXP;
      default:
          ATLASSERT(0);
          return NILSXP;
    }
  }

  /*template<class T>
  struct SEXPhelper
  {
    CComSafeArray<T> sa;
    SEXP sexp;
    SEXPhelper(const SAFEARRAY& psa)
    {
      sa.Attach(&psa);
    }
    ~SEXPhelper()
    {
      sa.Detach();
    }

    template<class S>
    S get(size_t i) { return (S)sa.GetAt(i); }
  };*/

  inline SEXP newVal(const std::vector<double>& value)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(REALSXP, value.size()));
    for (std::size_t i = 0, n = value.size(); i < n; i++) 
      REAL(sexp)[i] = value[i];
    return sexp;
  }
  inline SEXP newVal(const std::vector<int>& value)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(INTSXP, value.size()));
    for (std::size_t i = 0, n = value.size(); i < n; i++) 
      INTEGER(sexp)[i] = value[i];
    return sexp;
  }
  
  inline SEXP newVal(const std::vector<bool>& value)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(LGLSXP, value.size()));
    for (std::size_t i = 0, n = value.size(); i < n; i++) 
      LOGICAL(sexp)[i] = value[i];
    return sexp;
  }
  
  inline SEXP newVal(const std::vector<byte>& value)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(RAWSXP, value.size()));
    for (std::size_t i = 0, n = value.size(); i < n; i++) 
      RAW(sexp)[i] = value[i];
    return sexp;
  }
  
  inline SEXP newVal(const std::vector<std::string>& value)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(STRSXP, value.size()));
    for (std::size_t i = 0, n = value.size(); i < n; i++) 
    {
      if (value[i].empty())
        SET_STRING_ELT(sexp, (R_len_t)i, R_NaString);
      else
        SET_STRING_ELT(sexp, (R_len_t)i, Rf_mkCharCE(value[i].c_str(), CE_UTF8));
    }
    return sexp;
  }

  inline SEXP newVal(const std::vector<std::wstring>& value)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(STRSXP, value.size()));
    for (std::size_t i = 0, n = value.size(); i < n; i++) 
    {
      if (value[i].empty())
        SET_STRING_ELT(sexp, (R_len_t)i, R_NaString);
      else
        SET_STRING_ELT(sexp, (R_len_t)i, Rf_mkCharCE(toUtf8(value[i].c_str()).c_str(), CE_UTF8));
    }
    return sexp;
  }
  
  inline SEXP newVal(const std::vector<SEXP>& value)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(VECSXP, value.size()));
    for (R_len_t i = 0, n = (R_len_t)value.size(); i < n; i++) 
      SET_VECTOR_ELT(sexp, i, value[(std::size_t)i]);
    return sexp;
  }

  //inline bool getAttr(SEXP obj, LPCSTR attr_name)
  //{
  //  SEXP sexp = Rf_getAttrib(shape, Rf_mkChar(attr_name));
  //}
  inline bool getNames(SEXP obj, std::vector<std::string>& names)
  {
    SEXP attr_names = Rf_getAttrib(obj, R_NamesSymbol);
    return copy_to(attr_names, names);
  }

  inline bool getNames(SEXP obj, std::vector<std::wstring>& names)
  {
    SEXP attr_names = Rf_getAttrib(obj, R_NamesSymbol);
    return copy_to(attr_names, names);
  }

  inline SEXP newPair(SEXP head, SEXP tail, protect& pt)
  {
    if (head != NULL && head != R_NilValue && 
        tail != NULL && tail != R_NilValue)
      return pt.add(Rf_list2(head , tail));
    return pt.add(Rf_cons(head , tail));
  }
  
  inline SEXP nameIt(SEXP obj, const std::string &name)
  {
    protect pt;
    Rf_setAttrib(pt.add(obj), R_NamesSymbol, newVal(name));
    return obj;
  }

  inline SEXP nameIt(SEXP obj, const std::wstring &name)
  {
    protect pt;
    Rf_setAttrib(pt.add(obj), R_NamesSymbol, newVal(name));
    return obj;
  }

  inline SEXP nameElements(SEXP xptr, const std::vector<std::string>& names)
  {
    SEXP head = xptr;
    for (int i = 0; !Rf_isNull(xptr); xptr = CDR(xptr))
    {
      SET_TAG(xptr, newVal(names[i++]));
    }
    return head;
  }
  inline SEXP nameIt(SEXP obj, const std::vector<std::string>& names)
  {
    tools::protect pt;
    Rf_setAttrib(pt.add(obj), R_NamesSymbol, newVal(names));
    return obj;
  }

  inline SEXP nameIt(SEXP obj, const std::vector<std::wstring>& names)
  {
    tools::protect pt;
    Rf_setAttrib(pt.add(obj), R_NamesSymbol, newVal(names));
    return obj;
  }

  inline R_xlen_t size(SEXP sexp)
  {
    if (Rf_isNull(sexp)) return 0;
    return Rf_xlength(sexp);
  }
  
  inline bool push_back(SEXP source, SEXP val)
  {
    Rf_listAppend(source, val);
    /*SEXP last = source;
    while (!Rf_isNull(CDR(last)))
      last = CDR(last);
    SETCDR(last, val);*/
    return true;
  }
  class list_iterator
  {
    preserved head;
    SEXP pos;
    list_iterator(list_iterator&){}
  public:
    list_iterator(SEXP l = R_NilValue) : head(l), pos(l){}
    void set(SEXP l) { head.set(l); pos = head.get(); }
    SEXP next()
    {
      SEXP ret = pos;
      if (!Rf_isNull(ret)) 
        pos = CDR(pos);
      return ret;
    }
    void reset(){ pos = head.get(); }
  };
  class vectorGeneric : public preserved
  {
    vectorGeneric(const vectorGeneric&){}
  public:
    vectorGeneric(SEXP l = R_NilValue) : preserved(l)
    {
    }
    void set(SEXP val){ preserved::set(val); }
    std::size_t size() const { return tools::size(sexp); }
    SEXP at(std::size_t i) const
    {
      if (TYPEOF(sexp) == VECSXP)
        return VECTOR_ELT(sexp, (R_len_t)i);
      if (TYPEOF(sexp) == STRSXP)
        return STRING_ELT(sexp, (R_len_t)i);

      ATLTRACE("SEXP type:%s", Rf_type2char(TYPEOF(sexp)));
      ATLASSERT(0); //not a vector
      return 0;
    }
    SEXP at(const std::string &name) const
    {
      std::size_t i = idx(name);
      if (i == (std::size_t)-1)
        return NULL;
      return at(i);
    }
    SEXP set(std::size_t i, SEXP val)
    {
      if (TYPEOF(sexp) == VECSXP)
        return SET_VECTOR_ELT(sexp, (R_len_t)i, val);
      if (TYPEOF(sexp) == STRSXP)
        return SET_STRING_ELT(sexp, (R_len_t)i, val), val;
      ATLASSERT(0);
    }
    std::size_t idx(const std::string &name) const 
    {
      std::vector<std::string> names;
      getNames(sexp, names);
      if (names.empty()) 
        return (std::size_t)-1;
      std::vector<std::string>::iterator it = std::find(names.begin(), names.end(), name);
      if (it == names.end())return (std::size_t)-1;
      return std::distance(names.begin(), it);
    }
    //void push_back(SEXP val){ }
  };
  SEXP newVal(const VARIANT &v);

  template <typename T1, typename T2> struct is_same { enum { value = false }; };
  template <typename T> struct is_same<T,T> { enum { value = true }; };

  class listGeneric //: protected preserved
  {
    std::size_t m_size2;
    std::size_t m_size;
    preserved m_p;
    std::vector<std::string> m_namesU8;
  public:
    listGeneric(std::size_t reserve) :
        m_p(Rf_allocVector(VECSXP, (R_len_t)reserve)),
        m_size2(reserve),
        m_size(0)
    {
    }
    SEXP push_back(SEXP val, const char* nameU8 = NULL)
    {
      if (nameU8)
        m_namesU8.push_back(nameU8);

      if (m_size == m_size2)
      {
        tools::protect pt;
        pt.add(val);
        resize(m_size2 + 1 + (std::size_t)(m_size2/4));
      }
      SET_VECTOR_ELT(m_p.get(), (R_len_t)(m_size++), val);
      return val;
    }

    template<typename T>
    SEXP push_back(const T &val, const LPCWSTR name = NULL)
    {
      if (is_same<T, SEXP>::value)
        return push_back(val, toUtf8(name).c_str());
      return push_back(tools::newVal(val), toUtf8(name).c_str());
    }
    template<typename T>
    SEXP push_back(const T &val, const std::wstring &name)
    {
      return push_back<T>(val, name.c_str());
    }
    template<typename T>
    SEXP push_back(const T &val, const std::string &name)
    {
      if (is_same<T, SEXP>::value)
        return push_back(val, name.c_str());
      return push_back(tools::newVal(val), name.c_str());
    }
    SEXP get()
    {
      if (m_size != m_size2)
        resize(m_size);
      if (!m_namesU8.empty())
        return tools::nameIt(m_p.get(), m_namesU8);
      return m_p.get();
    }

    SEXP reset()
    {
      m_size = m_size2 = 0;
      return m_p.release();
    }
  private:
    void resize(std::size_t n)
    {
      if (n == 0) n = 1;
      protect pt;
      SEXP v1 = pt.add(Rf_allocVector(VECSXP, (R_len_t)n));
      SEXP v0 = m_p.get();
      for (std::size_t i = 0; i < m_size; i++)
        SET_VECTOR_ELT(v1, (R_len_t)i, VECTOR_ELT(v0, (R_len_t)i));
      m_size2 = n;
      m_p.set(v1);
    }
  };
#if 0
  inline SEXP newVal(SAFEARRAY& psa)
  {
    VARTYPE vt = {VT_EMPTY};
    ::SafeArrayGetVartype(&psa, &vt);
    LONG size = 0;
    LONG lLBound = 0, lUBound = 0;
    ::SafeArrayGetLBound(&psa, 1, &lLBound);
    ::SafeArrayGetUBound(&psa, 1, &lUBound);
    size = lUBound - lLBound + 1;
    ATLASSERT(size > 0); //???
    SEXPTYPE st = vartype2rtype(vt);
    if (st == NILSXP)
      return R_NilValue;

    if (st == VECSXP)
    {
      listGeneric list(size);
      SEXPhelper<VARIANT> sa(psa);
      for (size_t i = 0; i < size; i++)
        list.push_back(sa.get<VARIANT>(i));
      return list.get();
    }

    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(st, size));
    switch(st)
    {
      case LGLSXP:
      {
         SEXPhelper<VARIANT_BOOL> sa(psa);
         for (size_t i = 0; i < size; i++)
           LOGICAL(sexp)[i] = sa.get<VARIANT_BOOL>(i) != VARIANT_FALSE;
      }break;
      case INTSXP:
      {
        SEXPhelper<int> sa(psa);
        for (size_t i = 0; i < size; i++)
          INTEGER(sexp)[i] = sa.get<int>(i);
      }break;
      case REALSXP:
      {
        SEXPhelper<double> sa(psa);
        for (size_t i = 0; i < size; i++)
          REAL(sexp)[i] = sa.get<double>(i);
      }break;
      case CHARSXP:
      {
        SEXPhelper<BSTR> sa(psa);
        for (size_t i = 0; i < size; i++)
        {
          auto str = toUtf8(sa.get<BSTR>(i));
          if (str.empty())
            SET_STRING_ELT(sexp, (R_len_t)i, R_NaString);
          else
            SET_STRING_ELT(sexp, (R_len_t)i, Rf_mkCharCE(str.c_str(), CE_UTF8));
        }
      }break;
      default:
        ATLASSERT(0);
        return R_NilValue;
    }
    return sexp;
  }
#endif

  inline SEXP newVal(const VARIANT &v)
  {
    switch(v.vt)
    {
      case VT_NULL: return R_NilValue;
      case VT_BOOL: return newVal(v.boolVal != 0);
      case VT_INT:
      case VT_I4:
      case VT_UINT:
      case VT_UI4: return newVal((int)v.lVal);
      case VT_R4: return newVal((double)v.fltVal);
      case VT_R8: return newVal(v.dblVal);
      case VT_BSTR: return newVal(toUtf8(v.bstrVal));
      default:
        /*if ((v.vt & VT_ARRAY) == VT_ARRAY)
          return newVal(*v.parray);*/
        ATLASSERT(0);
        return R_NilValue;
    }
    return R_NilValue;
  }

  template<class T> 
  inline T castVar(const VARIANT &v)
  {
    ATLASSERT(0);
    return T;
  }
  template<>
  inline double castVar<double>(const VARIANT &var)
  { 
    return (var.vt == VT_EMPTY) ? std::numeric_limits<double>::quiet_NaN() : var.dblVal;
  }
  template<> 
  inline int castVar<int>(const VARIANT &var)
  { 
    return var.vt == VT_EMPTY ? R_NaInt : var.lVal;
  }
  template<>
  inline bool castVar<bool>(const VARIANT &var)
  { 
    return (var.vt == VT_EMPTY) ? false : var.boolVal != VARIANT_FALSE;
  }
  template<>
  inline std::wstring castVar<std::wstring>(const VARIANT &var)
  { 
    if (var.vt != VT_EMPTY)
      return var.bstrVal;
    return L"";
  }

  inline SEXP func_eval(const char* fn_name, SEXP param1 = NULL, SEXP param2 = NULL, SEXP param3 = NULL, SEXP param4 = NULL)
  {
    protect pt;
    SEXP call = NULL;
    if (param1 == NULL) 
      call = pt.add(::Rf_lang1(Rf_install(fn_name)));
    else if (param1 != NULL && param2 == NULL) 
      call = pt.add(::Rf_lang2(Rf_install(fn_name), param1));
    else if (param1 != NULL && param2 != NULL && param3 == NULL)
      call = pt.add(::Rf_lang3(Rf_install(fn_name), param1, param2));
    else if (param1 != NULL && param2 != NULL && param3 != NULL && param4 == NULL)
      call = pt.add(::Rf_lang4(Rf_install(fn_name), param1, param2, param3));
    else if (param1 != NULL && param2 != NULL && param3 != NULL && param4 != NULL)
      call = pt.add(::Rf_lang5(Rf_install(fn_name), param1, param2, param3, param4));
    else { ATLASSERT(0); return NULL;}
    ATLASSERT(call);
    return ::Rf_eval(call, R_GlobalEnv);
  }
}

struct fn_struct
{
  typedef SEXP (*f6)(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
  typedef SEXP (*f5)(SEXP, SEXP, SEXP, SEXP, SEXP);
  typedef SEXP (*f4)(SEXP, SEXP, SEXP, SEXP);
  typedef SEXP (*f3)(SEXP, SEXP, SEXP);
  typedef SEXP (*f2)(SEXP, SEXP);
  typedef SEXP (*f1)(SEXP);
  typedef SEXP (*f0)();

  int nargs;
  void* (*fn)();

  SEXP args[6];
  SEXP call() const
  {
    SEXP ret = NULL;
    switch (nargs)
    {
      case 4: ret = (*(f4)fn)(args[0], args[1], args[2], args[3]); break;
      case 3: ret = (*(f3)fn)(args[0], args[1], args[2]); break;
      case 2: ret = (*(f2)fn)(args[0], args[1]);          break;
      case 1: ret = (*(f1)fn)(args[0]);                   break;
      case 0: ret = (*(f0)fn)();                          break;
      default:
          ATLASSERT(0);
        break;
    }
    return ret;
  }
  SEXP external(SEXP args)
  {
    SEXP ret = NULL;
    switch (nargs)
    { 
      case 4: ret = (*(f4)fn)(CDR(args), CDR(args), CDR(args), CDR(args)); break;
      case 3: ret = (*(f3)fn)(CDR(args), CDR(args), CDR(args)); break;
      case 2: ret = (*(f2)fn)(CDR(args), CDR(args));          break;
      case 1: ret = (*(f1)fn)(CDR(args));                   break;
      case 0: ret = (*(f0)fn)();                          break;
      default:
          ATLASSERT(0);
        break;
    }
    return ret;
  }
};

SEXP R_fnN(fn_struct &it);

template<SEXP (*fn)(SEXP,SEXP,SEXP,SEXP)>
SEXP R_fn6(SEXP x1, SEXP x2, SEXP x3, SEXP x4, SEXP x5, SEXP x6)
{
  fn_struct it = {6, (DL_FUNC)fn, {x1, x2, x3, x4, x5, x6}};
  return R_fnN(it);
}

template<SEXP (*fn)(SEXP,SEXP,SEXP,SEXP)>
SEXP R_fn4(SEXP x1, SEXP x2, SEXP x3, SEXP x4)
{
  fn_struct it = {4, (DL_FUNC)fn, {x1, x2, x3, x4}};
  return R_fnN(it);
}
template<SEXP (*fn)(SEXP,SEXP,SEXP)>
SEXP R_fn3(SEXP x1, SEXP x2, SEXP x3)
{
  fn_struct it = {3, (DL_FUNC)fn, {x1, x2, x3}};
  return R_fnN(it);
}
template<SEXP (*fn)(SEXP,SEXP)>
SEXP R_fn2(SEXP x1, SEXP x2)
{
  fn_struct it = {2, (DL_FUNC)fn, {x1, x2}};
  return R_fnN(it);
}
template<SEXP (*fn)(SEXP)>
SEXP R_fn1(SEXP x1)
{
  fn_struct it = {1, (DL_FUNC)fn, {x1}};
  return R_fnN(it);
}

template<SEXP (*fn)()>
SEXP R_fn0()
{
  fn_struct it = {0, (DL_FUNC)fn};
  return R_fnN(it);
}
template<int nargs, DL_FUNC fn>
SEXP R_fnEx(SEXP args)
{
  fn_struct it = {nargs, (DL_FUNC)fn};
  return it.external(args);
}
/*template<SEXP (*fn)()>
SEXP R_fnEx0(SEXP args)
{
  fn_struct it = {0, (DL_FUNC)fn};
  return it.external(args);
}*/

SEXP error_Ret(const char* str_or_UTF8, SEXP retVal = R_NilValue);

template<bool bCheckCOM>
inline SEXP showError(const wchar_t* strErrDefault = L"unknown error")
{
  _bstr_t strErr;
  if (bCheckCOM)
  {
    if (!tools::getCOMError(strErr.GetAddress()))
      strErr = strErrDefault;
  }
  else
    strErr = strErrDefault;
  
  return error_Ret(tools::toUtf8(strErr).c_str());
}
