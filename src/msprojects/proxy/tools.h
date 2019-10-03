#pragma once
#include <vector>
#include <forward_list>
#include <algorithm>
#include <string>
#include <limits>
#include <codecvt>
#include <unordered_map>
#include <exception>
//#include <atlsafe.h>

namespace tools
{
  class protect
  {
    int n;
  public:
    protect(const protect&) = delete;
    constexpr protect(): n(0) {}
    constexpr protect(SEXP v): n(0) { add (v); }
    ~protect() noexcept { release(); }
    inline constexpr SEXP add(SEXP v) noexcept
    {
      if (v == R_NilValue)
        return v;
      try { PROTECT(v); n++; } catch(...){ return R_NilValue; }
      return v;
    }
    inline constexpr void release() noexcept
    {
      if (n == 0) return;
      try { UNPROTECT(n); } catch(...){}
      n = 0;
    }
  };
  class preserved
  {
  protected:
    SEXP sexp;
  public:
    preserved(const preserved&) = delete;
    constexpr preserved() : sexp(R_NilValue) {}
    constexpr preserved(SEXP v) : sexp(R_NilValue) { set(v); }
    ~preserved(){ release(); }
    inline constexpr void set(SEXP v) noexcept
    {
      if (v == sexp) return;
      release();
      if (v != R_NilValue)
      {
        try { ::R_PreserveObject(v); sexp = v;} catch(...){}
      }
    }
    inline constexpr SEXP release() noexcept
    {
      if (sexp == R_NilValue) return R_NilValue;
      SEXP ret = sexp;
      try { ::R_ReleaseObject(sexp); } catch(...){}
      sexp = R_NilValue;
      return ret;
    }
    inline constexpr SEXP get() const noexcept { return sexp; }
  };

  inline static const std::wstring tolower(const std::wstring& str)
  {
    std::wstring tmp(str);
    std::transform(str.begin(), str.end(), tmp.begin(), ::towlower);
    return tmp;
  }

  inline constexpr bool copy_to(const SEXP sexp, std::string &str_out)
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
  inline constexpr bool copy_to(const SEXP sexp, std::wstring &str_out)
  {
    if (!sexp) return false;
    std::string str;
    if (!copy_to(sexp, str))
      return false;

    str_out = fromUtf8(str.c_str());
    return true;
  }

  constexpr bool copy_to(const SEXP sexp, int &out);
  inline constexpr bool copy_to(const SEXP sexp, double &out)
  {
    if (!sexp) return false;
    if (TYPEOF(sexp) != REALSXP)
    {
      int tmp = 0; //allow coerce from int
      if (TYPEOF(sexp) == INTSXP && copy_to(sexp, tmp))
      {
        out = (double)tmp;
        return true;
      }
      ATLTRACE("SEXP type:%s", Rf_type2char(TYPEOF(sexp)));
      return false;
    }

    if (Rf_length(sexp) < 1)
      return false;

    out = REAL(sexp)[0];
    return true;
  }

  inline constexpr bool copy_to(const SEXP sexp, int &out)
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

  inline constexpr bool copy_to(const SEXP sexp, bool &out)
  {
    if (!sexp) return false;
    //if (TYPEOF(sexp) != LGLSXP && Rf_xlength(sexp) < 1)
    //   return false;
    if (Rf_isLogical(sexp) && Rf_xlength(sexp) > 0)
      out = Rf_asLogical(sexp) != Rboolean::FALSE;
    else
    {
      ATLTRACE("SEXP type:%s", Rf_type2char(TYPEOF(sexp)));
      return false;//out = (b == NA_LOGICAL) ? true : (Rboolean) b;
    }
    //out = LOGICAL(sexp)[0] ? true : false;
    return true;
  }

  inline constexpr bool copy_to(const SEXP sexp, std::vector<byte>& out)
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

  inline constexpr bool copy_to(const SEXP sexp, std::vector<std::string>& out)
  {
    if (!sexp) return false;
    if (TYPEOF(sexp) != STRSXP)
    {
      ATLTRACE("SEXP type:%s", Rf_type2char(TYPEOF(sexp)));
      return false;
    }
    try {
    out.resize((std::size_t)Rf_xlength(sexp));
    for (R_xlen_t i = 0, n = (R_xlen_t)out.size(); i < n; i++)
      out[i].assign(Rf_translateChar(STRING_ELT(sexp, i)));
    }catch(...){ return false; }
    return true;
  }

  inline constexpr bool copy_to(const SEXP sexp, std::vector<std::wstring>& out, bool na_as_empty = false)
  {
    if (!sexp) return false;
    if (TYPEOF(sexp) != STRSXP)
    {
      ATLTRACE("SEXP type:%s", Rf_type2char(TYPEOF(sexp)));
      return false;
    }
    try {
    out.resize((std::size_t)Rf_xlength(sexp));
    if (na_as_empty)
    {
      for (R_xlen_t i = 0, n = (R_xlen_t)out.size(); i < n; i++)
      {
        auto s = STRING_ELT(sexp, i);
        if (s != NA_STRING)
          out[i] = fromUtf8(Rf_translateCharUTF8(s));
      }
    }
    else
      for (R_xlen_t i = 0, n = (R_xlen_t)out.size(); i < n; i++)
        out[i] = fromUtf8(Rf_translateCharUTF8(STRING_ELT(sexp, i)));
    }catch(...){ return false; }
    return true;
  }

  inline constexpr bool copy_to(const SEXP sexp, std::vector<int>& out)
  {
    if (!sexp) return false;
    if (TYPEOF(sexp) != INTSXP)
    {
      ATLTRACE("SEXP type:%s", Rf_type2char(TYPEOF(sexp)));
      return false;
    }
    try {
    out.resize((std::size_t)Rf_xlength(sexp));
    for (std::size_t i = 0, n = out.size(); i < n; i++)
      out[i] = INTEGER(sexp)[i];
    }catch(...){ return false; }
    return true;
  }

  inline constexpr bool copy_to(const SEXP sexp, std::vector<double>& out)
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

  inline constexpr bool copy_to(const SEXP sexp, std::vector<bool>& out)
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
/*
  inline bool copy_to(SEXP sexp, SEXP& out)
  {
    if (!sexp) return false;
    out = sexp;
    return true;
  }
*/
  static inline constexpr SEXPTYPE vartype2rtype(const VARTYPE vt) noexcept
  {
    switch(vt & VT_TYPEMASK)
    {
      case VT_EMPTY:
      case VT_NULL: return NILSXP;
      case VT_BOOL: return LGLSXP;
      case VT_I1:
      case VT_UI1: //return RAWSXP;
      case VT_I2:
      case VT_UI2:
      case VT_I4:
      case VT_UI4:  return INTSXP;
      case VT_R4:
      case VT_R8:   return REALSXP;
      case VT_BSTR: return STRSXP;//CHARSXP;
      case VT_VARIANT: return VECSXP;
      default:
          ATLASSERT(0);
          return NILSXP;
    }
  }

  class SafeArrayHelper
  {
  private:
      mutable bool m_locked;
      SAFEARRAY* m_psa;
      VARTYPE m_vt;
  public:
    SafeArrayHelper(SAFEARRAY *psa) : m_locked(false), m_psa(psa)
    {
      ATLASSERT(psa != nullptr);
      m_vt = VarType(psa);
    }
    ~SafeArrayHelper()
    {
      if (m_locked)
        ::SafeArrayUnlock(m_psa);
    }
    constexpr SAFEARRAY* Detach()
    {
      if (m_locked)
      {
        ::SafeArrayUnlock(m_psa);
        m_locked = false;
      }
      auto tmp = m_psa;
      m_psa = nullptr;
      return tmp;
    }
    constexpr const SAFEARRAY* psa() const { return m_psa; }

    template<class S>
    constexpr S get(size_t i) const
    {
      if (!m_locked)
        m_locked = ::SafeArrayLock(m_psa) == S_OK;
      LONG lLBound = 0;
      ATLASSERT(::SafeArrayGetLBound(m_psa, 1, &lLBound) == S_OK && lLBound == 0);
      return *(((S*)m_psa->pvData) + i);
    }
    template<>
    int get<int>(size_t i) const
    {
      if (!m_locked)
        m_locked = ::SafeArrayLock(m_psa) == S_OK;
      LONG lLBound = 0;
      ATLASSERT(::SafeArrayGetLBound(m_psa, 1, &lLBound) == S_OK && lLBound == 0);
      switch (m_vt)
      {
        case VT_I1:
        case VT_UI1:
        {
          const auto v = *(((unsigned char*)m_psa->pvData) + i);
          return v;
        }
        case VT_I2:
        case VT_UI2:
        {
          const auto v = *(((unsigned short*)m_psa->pvData) + i);
          return v;
        }
        case VT_UI4:
        case VT_I4:
          return *(((int*)m_psa->pvData) + i);
        default:
          ATLASSERT(0);
          return 0;
      }
    }
    constexpr size_t GetCount() const { return GetCount(m_psa); }
    constexpr VARTYPE VarType() const noexcept { return m_vt; }

    //dimantion 0
    static constexpr size_t GetCount(SAFEARRAY* psa)
    {
      if (psa == nullptr) return 0;
      ATLASSERT(::SafeArrayGetDim(psa) == 1);
      //LONG lLBound = 0, lUBound = 0;
      //HRESULT hRes = ::SafeArrayGetLBound(psa, 1, &lLBound);
      //ATLASSERT(SUCCEEDED(hRes));
      //hRes = ::SafeArrayGetUBound(psa, 1, &lUBound);
      //ATLASSERT(SUCCEEDED(hRes));
      //return (lUBound - lLBound + 1);
      ATLASSERT(psa->rgsabound[0].lLbound == 0);
      return (size_t)psa->rgsabound[0].cElements;
    }

    static constexpr VARTYPE VarType(SAFEARRAY* psa) noexcept
    {
      VARTYPE vt = {VT_EMPTY};
      if (psa)
        ::SafeArrayGetVartype(psa, &vt);
      return vt;
    }
  };

  inline SEXP newVal(const char* const* vals, size_t n)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(STRSXP, n));
    for (std::size_t i = 0; i < n; i++)
    {
      const char *str = vals[i];
      SET_STRING_ELT(sexp, i, (str == nullptr || str[0] == 0) ? R_NaString : Rf_mkCharCE(str, CE_UTF8));
    }
    return sexp;
  }
  inline SEXP newVal(const char* val){ return newVal(&val, 1);}

  inline SEXP newVal(const wchar_t* const* vals, size_t n)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(STRSXP, n));
    for (std::size_t i = 0; i < n; i++)
    {
      const wchar_t *str = vals[i];
      SET_STRING_ELT(sexp, i, (str == nullptr || str[0] == 0) ? R_NaString : Rf_mkCharCE(toUtf8(str).c_str(), CE_UTF8));
    }
    return sexp;
  }
  inline SEXP newVal(const wchar_t* val){ return newVal(&val, 1);}

  inline SEXP newVal(const std::string* vals, size_t n)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(STRSXP, n));
    for (std::size_t i = 0; i < n; i++)
    {
      const auto& str = vals[i];
      SET_STRING_ELT(sexp, i, str.empty() ? R_NaString : Rf_mkCharCE(str.c_str(), CE_UTF8));
    }
    return sexp;
  }
  inline SEXP newVal(const std::string& val) { return newVal(&val, 1); }

  inline SEXP newVal(const std::wstring* vals, size_t n)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(STRSXP, n));
    for (std::size_t i = 0; i < n; i++)
    {
      const auto& str = vals[i];
      SET_STRING_ELT(sexp, i, str.empty() ? R_NaString : Rf_mkCharCE(toUtf8(str.c_str()).c_str(), CE_UTF8));
    }
    return sexp;
  }
  inline SEXP newVal(const std::wstring& val) { return newVal(&val, 1); }

  inline SEXP newVal(const SEXP* vals, size_t n)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(VECSXP, n));
    for (size_t i = 0; i < n; i++)
      SET_VECTOR_ELT(sexp, i, vals[i]);
    return sexp;
  }
  inline SEXP newVal(SEXP value) { return value; }

  inline SEXP newVal(const double* vals, size_t n)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(REALSXP, n));
    std::memcpy(REAL(sexp), vals, n * sizeof(double));
    return sexp;
  }
  inline SEXP newVal(const double val) { return newVal(&val, 1); }

  inline SEXP newVal(const int* vals, size_t n)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(INTSXP, n));
    std::memcpy(INTEGER(sexp), vals, n * sizeof(int));
    return sexp;
  }
  inline SEXP newVal(const int val) { return newVal(&val, 1); }
  inline SEXP newVal(const long* vals, size_t n)
  {
    static_assert(sizeof(int) == sizeof(long), "TODO");
    return newVal((const int*)vals, n);
  }

  /*inline SEXP newBoolVal(const bool *it, size_t n)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(LGLSXP, n));
    for (std::size_t i = 0; i < n; i++, ++it)
      LOGICAL(sexp)[i] = *it ? TRUE : FALSE;
    return sexp;
  }*/
  inline SEXP newVal(const bool value) { return Rf_ScalarLogical(value ? TRUE : FALSE); }

  inline SEXP newVal(const byte* vals, size_t n)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(RAWSXP, n));
    std::memcpy(RAW(sexp), vals, n * sizeof(byte));
    return sexp;
  }

  template<class T>
  inline SEXP newVal(const std::vector<T>& value)
  {
    return newVal(value.data(), value.size());
  }

  template <class T, size_t SIZE>
  inline SEXP newVal(const std::array<T, SIZE>& value)
  {
    return newVal(value.data(), SIZE);
  }

  template <class T>
  inline SEXP newVal(const std::initializer_list<T> &&values)
  {
    return newVal(values.begin(), values.size());
  }

  template<>
  inline SEXP newVal<bool>(const std::vector<bool>& value)
  {
    //return newBoolVal(value.data(), value.size());
    protect pt;
    const auto n = value.size();
    SEXP sexp = pt.add(Rf_allocVector(LGLSXP, n));
    std::size_t i = 0;
    for (const auto v : value)
      LOGICAL(sexp)[i++] = v ? TRUE : FALSE;
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

  /*inline SEXP newPair(SEXP head, SEXP tail, protect& pt)
  {
    if (head != NULL && head != R_NilValue && 
        tail != NULL && tail != R_NilValue)
      return pt.add(Rf_list2(head , tail));
    return pt.add(Rf_cons(head , tail));
  }*/
  /*inline SEXP nameElements(SEXP xptr, const std::vector<std::string>& names)
  {
  SEXP head = xptr;
  for (int i = 0; !Rf_isNull(xptr); xptr = CDR(xptr))
  {
  SET_TAG(xptr, newVal(names[i++]));
  }
  return head;
  }*/

  template<class T>
  inline SEXP nameIt(SEXP obj, const T &name)
  {
    //protect pt;
    Rf_setAttrib(obj, R_NamesSymbol, newVal(name));
    return obj;
  }
  template<class T>
  inline SEXP nameIt(SEXP obj, const std::initializer_list<T> &&name)
  {
    //protect pt;
    Rf_setAttrib(obj, R_NamesSymbol, newVal(std::move(name)));
    return obj;
  }

  inline R_xlen_t size(const SEXP sexp)
  {
    if (Rf_isNull(sexp)) return 0;
    return Rf_xlength(sexp);
  }

  /*inline bool push_back(SEXP source, SEXP val)
  {
    Rf_listAppend(source, val);
    / *SEXP last = source;
    while (!Rf_isNull(CDR(last)))
      last = CDR(last);
    SETCDR(last, val);* /
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
  };*/
  class vector_iterator: public preserved
  {
    vector_iterator(const vector_iterator&) = delete;
    mutable std::vector<std::string> m_names;
  public:
    vector_iterator(SEXP l = R_NilValue) : preserved(l)
    {
    }
    constexpr void set(SEXP val){ preserved::set(val); }
    inline std::size_t size() const { return tools::size(sexp); }
    inline SEXP at(std::size_t i) const
    {
      if (TYPEOF(sexp) == VECSXP)
        return VECTOR_ELT(sexp, (R_xlen_t)i);
      if (TYPEOF(sexp) == STRSXP)
        return STRING_ELT(sexp, (R_xlen_t)i);

      ATLTRACE("SEXP type:%s", Rf_type2char(TYPEOF(sexp)));
      ATLASSERT(0); //not a vector
      return 0;
    }
    inline SEXP at(const std::string &name) const
    {
      std::size_t i = idx(name);
      if (i == (std::size_t)-1)
        return NULL;
      return at(i);
    }
    inline SEXP set(std::size_t i, SEXP val)
    {
      if (TYPEOF(sexp) == VECSXP)
        return SET_VECTOR_ELT(sexp, (R_xlen_t)i, val);
      if (TYPEOF(sexp) == STRSXP)
        return SET_STRING_ELT(sexp, (R_xlen_t)i, val), val;
      ATLASSERT(0);
    }
    inline std::size_t idx(const std::string &name) const
    {
      const auto& nm = names();
      if (nm.empty())
        return (std::size_t)-1;
      auto it = std::find(nm.begin(), nm.end(), name);
      if (it == nm.end())return (std::size_t)-1;
      return std::distance(nm.begin(), it);
    }
    inline const std::vector<std::string>& names() const
    {
      if (m_names.empty())
        getNames(sexp, m_names);
      return m_names;
    }
    //void push_back(SEXP val){ }
  };

  SEXP newVal(const VARIANT &v);

  class listGeneric
  {
    std::size_t m_capacity;
    std::size_t m_size;
    preserved m_p;
    std::vector<std::string> m_namesU8;
  public:
    listGeneric(std::size_t reserve) :
        m_p(Rf_allocVector(VECSXP, (R_xlen_t)reserve)),
        m_capacity(reserve),
        m_size(0)
    {
    }
    template<typename T>
    SEXP push_back(const T &val)
    {
      return _push_back(tools::newVal(val));
    }
    template<typename T>
    SEXP push_back(const T &val, const std::wstring &name)
    {
      return _push_back(tools::newVal(val), toUtf8(name.c_str()).c_str());
    }
    template<typename T>
    SEXP push_back(const T &val, const std::string &name)
    {
      return _push_back(tools::newVal(val), name.c_str());
    }
    SEXP get()
    {
      if (m_size != m_capacity)
        resize(m_size);
      if (!m_namesU8.empty())
      {
        //auto nm = GET_NAMES(m_p.get());
        auto nm = Rf_getAttrib(m_p.get(), R_NamesSymbol);
        if (Rf_isNull(nm) || LENGTH(nm) != m_namesU8.size())
          return tools::nameIt(m_p.get(), m_namesU8);
      }
      return m_p.get();
    }

    /*SEXP reset()
    {
      m_size = m_capacity = 0;
      return m_p.release();
    }*/
  private:
    SEXP _push_back(SEXP val, const char* nameU8 = NULL)
    {
      if (nameU8)
        m_namesU8.push_back(nameU8);

      if (m_size == m_capacity)
      {
        tools::protect pt;
        pt.add(val);
        resize(m_size + 1 + (std::size_t)(m_size / 4));
      }
      SET_VECTOR_ELT(m_p.get(), m_size++, val);
      return val;
    }
    void resize(std::size_t n)
    {
      if (n == 0) n = 1;
      protect pt;
      SEXP v1 = pt.add(Rf_allocVector(VECSXP, n));
      SEXP v0 = m_p.get();
      for (std::size_t i = 0; i < m_size; i++)
        SET_VECTOR_ELT(v1, i, VECTOR_ELT(v0, i));
      m_capacity = n;
      m_p.set(v1);
    }
  };

  SEXP newVal(SAFEARRAY *psa);

  inline SEXP newVal(SAFEARRAY* psaNames, SAFEARRAY* psaVals)
  {
    auto vtN = SafeArrayHelper::VarType(psaNames);

    ATLASSERT(vtN == VT_BSTR);
    size_t sizeN = SafeArrayHelper::GetCount(psaNames);
    size_t sizeV = SafeArrayHelper::GetCount(psaVals);
    ATLASSERT(sizeN == sizeV);
    if (sizeN != sizeV)
      return R_NilValue;

    return Rf_setAttrib(newVal(psaVals), R_NamesSymbol, newVal(psaNames));
  }

  inline SEXP newVal(SAFEARRAY *psa)
  {
    SEXPTYPE st = vartype2rtype(SafeArrayHelper::VarType(psa));
    if (st == NILSXP)
      return R_NilValue;

    if (st == CHARSXP) //upgrade to string vector
      st = STRSXP;

    size_t size = SafeArrayHelper::GetCount(psa);
    SafeArrayHelper sa(psa);

    if (st == VECSXP)
    {
      //make named list
      if (size == 2 && sa.get<VARIANT>(0).vt == (VT_ARRAY|VT_BSTR) && (sa.get<VARIANT>(1).vt & VT_ARRAY) == VT_ARRAY)
      {
        auto names = sa.get<VARIANT>(0).parray;
        auto values = sa.get<VARIANT>(1).parray;
        if (SafeArrayHelper::GetCount(names) == SafeArrayHelper::GetCount(values))
          return newVal(names, values);
      }
      listGeneric list(size);
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
        for (size_t i = 0; i < size; i++)
          LOGICAL(sexp)[i] = sa.get<VARIANT_BOOL>(i) != VARIANT_FALSE;
      }break;
      case INTSXP:
      {
        for (size_t i = 0; i < size; i++)
          INTEGER(sexp)[i] = sa.get<int>(i);
      }break;
      case REALSXP:
      {
        for (size_t i = 0; i < size; i++)
          REAL(sexp)[i] = sa.get<double>(i);
      }break;
      case STRSXP:
      {
        for (size_t i = 0; i < size; i++)
        {
          auto str = toUtf8(sa.get<BSTR>(i));
          SET_STRING_ELT(sexp, (R_xlen_t)i, str.empty() ? R_NaString : Rf_mkCharCE(str.c_str(), CE_UTF8));
        }
      }break;
      default:
        ATLASSERT(0);
        return R_NilValue;
    }
    return sexp;
  }

// VARIANT and PROPVARIANT
  inline SEXP newVal(const VARIANT &v)
  {
    switch(v.vt)
    {
      case VT_EMPTY:
      case VT_NULL: return R_NilValue;
      case VT_BOOL: return newVal(v.boolVal != 0);
      case VT_UI1:
      case VT_I1: return newVal((int)v.bVal);
      case VT_UI2:
      case VT_I2: return newVal((int)v.iVal);
      case VT_INT:
      case VT_I4:
      case VT_UINT:
      case VT_UI4: return newVal((int)v.lVal);
      case VT_R4: return newVal((double)v.fltVal);
      case VT_R8: return newVal(v.dblVal);
      case VT_BSTR: return newVal(toUtf8(v.bstrVal));
      case VT_SAFEARRAY: ATLASSERT(0); return newVal(v.parray); //did not tested. usually use VT_ARRAY bit
      case VT_UNKNOWN:
        ::OutputDebugString(L"***** skip VARIANT(VT_UNKNOWN)");
        return R_NilValue;
      default:
        if ((v.vt & VT_ARRAY) == VT_ARRAY)
          return newVal(v.parray);
        ATLASSERT(0);
        return R_NilValue;
    }
    return R_NilValue;
  }
  inline SEXP newVal(const CAC &data)
  {
    return newVal((byte*)data.pElems, data.cElems);
  }
  inline SEXP newVal(const CAL &data)
  {
    return newVal(data.pElems, data.cElems);
  }
  inline SEXP newVal(const CADBL &data)
  {
    return newVal(data.pElems, data.cElems);
  }
  inline SEXP newVal(const CABSTR &data)
  {
    return newVal(data.pElems, data.cElems);
  }
  inline SEXP newVal(const CALPSTR &data)
  {
    return newVal(data.pElems, data.cElems);
  }
  inline SEXP newVal(const CALPWSTR &data)
  {
    return newVal(data.pElems, data.cElems);
  }

  inline SEXP newVal(const PROPVARIANT &pv);
  inline SEXP newVal(const CAPROPVARIANT &data)
  {
    protect pt;
    SEXP sexp = pt.add(Rf_allocVector(VECSXP, data.cElems));
    for (ULONG i = 0; i < data.cElems; i++)
      SET_VECTOR_ELT(sexp, i, newVal(data.pElems[i]));
    return sexp;
  }

  inline SEXP newVal(const PROPVARIANT &pv)
  {
    if (pv.vt == VT_NULL || pv.vt == VT_EMPTY)
      return R_NilValue;
    switch (pv.vt)
    {
      case VT_NULL: return R_NilValue;
      case (VT_UI1|VT_VECTOR):     return newVal(pv.cac);
      case (VT_I4|VT_VECTOR):      return newVal(pv.cal);
      case (VT_R8|VT_VECTOR):      return newVal(pv.cadbl);
      case (VT_BSTR|VT_VECTOR):    return newVal(pv.cabstr);
      case (VT_LPSTR|VT_VECTOR):   return newVal(pv.calpstr);
      case (VT_LPWSTR|VT_VECTOR):  return newVal(pv.calpwstr);
      case (VT_VARIANT|VT_VECTOR): return newVal(pv.capropvar);

      case VT_BSTR:    return newVal(pv.bstrVal);
      case VT_LPWSTR:  return newVal(pv.pwszVal);
      case VT_LPSTR:   return newVal(pv.pszVal);
      case VT_I4:      return newVal(pv.lVal);
      case VT_R8:      return newVal(pv.dblVal);

      default: ATLASSERT(0); return R_NilValue;
    }
    return R_NilValue;
  }

  typedef std::unordered_map<std::string, SEXP> args_map_type;

  inline static args_map_type pairlist2args_map(SEXP args)
  {
    std::unordered_map<std::string, SEXP> ret;
    if (args == R_NilValue)
      return ret;

    ATLASSERT(TYPEOF(args) == LISTSXP);
    //args = CDR(args); /* skip 'name' */
    //const char *me = CHAR(PRINTNAME(TAG(args)));
    for(int i = 0; args != R_NilValue; i++, args = CDR(args))
    {
      SEXP v = CAR(args);
      const char *name = Rf_isNull(TAG(args)) ? nullptr : CHAR(PRINTNAME(TAG(args)));
      if (name != nullptr)
        ret[name] = v;
      else
        ret[std::to_string(i)] = v;
    }
    return ret;
  }

  inline static bool r2variant(const SEXP r, VARIANT &v, const bool allow_multi_value = false)
  {
    v.vt = VT_EMPTY;
    if (r == NULL || Rf_isNull(r))
      return v.vt = VT_NULL, true;
    auto n = tools::size(r);
    if (n == 0)
      return v.vt = VT_NULL, true;

    switch(TYPEOF(r))
    {
      case CHARSXP:
      case STRSXP:
      {
        tools::protect pt;
        SEXP x = pt.add(Rf_asChar(r));
        std::wstring str;
        if (tools::copy_to(x, str))
        {
          v.vt = VT_BSTR;
          v.bstrVal = ::SysAllocString(str.c_str());
        }
      }break;
      case LGLSXP:
      {
        bool val = false;
        tools::copy_to(r, val);
        v.vt = VT_BOOL;
        v.boolVal = val ? VARIANT_TRUE : VARIANT_FALSE;
      }break;
      case INTSXP:
      {
        int val = 0;
        tools::copy_to(r, val);
        v.vt = VT_I4;
        v.lVal = val;
      }break;
      case REALSXP:
      {
        double val = 0.0;
        tools::copy_to(r, val);
        v.vt = VT_R8;
        v.dblVal = val;
      }break;
      case VECSXP://multi values
      {
        ATLASSERT(allow_multi_value);
        if (allow_multi_value)
        {
          auto sa_names = ::SafeArrayCreateVector(VT_BSTR, 0, (ULONG)n);
          auto sa = ::SafeArrayCreateVector(VT_VARIANT, 0, (ULONG)n);
          VARIANT *ptr = nullptr;
          ::SafeArrayAccessData(sa, (void**)&ptr);
          BSTR *ptr_names = nullptr;
          ::SafeArrayAccessData(sa_names, (void**)&ptr_names);
          vector_iterator list(r);
          for (const auto&it : list.names())
          {
            *ptr_names = ::SysAllocString(fromUtf8(it.c_str()).c_str());
            ptr_names++;
          }
          ::SafeArrayUnaccessData(sa_names);

          for (R_xlen_t i = 0; i < n; i++)
            r2variant(list.at(i), ptr[i]);
          ::SafeArrayUnaccessData(sa);

          v.vt = VT_ARRAY|VT_VARIANT;
          v.parray = ::SafeArrayCreateVector(VT_VARIANT, 0, (ULONG)2);
          ::SafeArrayAccessData(v.parray, (void**)&ptr);
          ptr[0].vt = VT_ARRAY|VT_BSTR; ptr[0].parray = sa_names;
          ptr[1].vt = VT_ARRAY|VT_VARIANT; ptr[1].parray = sa;
          ::SafeArrayUnaccessData(sa);
        }
      }break;
      default:
        ATLASSERT(0); //unknow type
        break;
    }
    return v.vt != VT_EMPTY;
  }

  struct argument_converter
  {
    argument_converter()=delete;
    template <class R>
    static constexpr bool arg2val(SEXP arg, R& out)
    {
      if (arg == nullptr) return false;
      return tools::copy_to(arg, out);
    }
    template <>
    static constexpr bool arg2val<SEXP>(SEXP arg, SEXP& out) noexcept
    {
      if (arg == nullptr) return false;
      out = arg;
      return true;
    }
    template <>
    static constexpr bool arg2val<VARIANT>(SEXP arg, VARIANT& out)
    {
      if (arg == nullptr) return false;
      ATLASSERT(out.vt == VT_EMPTY);
      return r2variant(arg, out);
    }
    template <>
    static constexpr bool arg2val<arcobject::sr_type>(SEXP arg, arcobject::sr_type& val)
    {
      if (arg == nullptr) return false;
      arcobject::sr_type from_sr(SEXP sr);
      val = from_sr(arg);
      return true;
    }
    /// return 0 - ok, 1 - missing, 2 - convert error
    template <bool throw_if_failed, bool remove_from_map, class KV>
    static constexpr int get_arg(typename std::conditional<remove_from_map, tools::args_map_type, const tools::args_map_type>::type& m, KV&& p)
    {
      //static_assert((remove_from_map && std::is_const<decltype(m)>::value == false) || remove_from_map == false, "incorrect flag/type");
      auto it = m.find(std::get<0>(p));
      SEXP arg = it == m.end() ? nullptr : it->second;
      if (remove_from_map && it != m.end())
        const_cast<tools::args_map_type&>(m).erase(it);
      int ret = 1;
      if (arg != nullptr)
        ret = arg2val(arg, std::get<1>(p)) ? 0 : 2;

      if (ret && throw_if_failed)
      {
        switch (ret)
        {
          default: ATLASSERT(false);
          case 1: throw std::invalid_argument(std::string("missing argument: ") + std::get<0>(p));
          case 2: throw std::domain_error(std::string("incorrect type, argument: ") + std::get<0>(p));
        }
      }
      return ret;
    }
  };

  /// return tuple<int...>, where 0 - ok, 1 - argument missing, 2 - convert error
  template <bool throw_if_failed, bool remove_from_map = false, class C = tools::argument_converter, class... KV>
  constexpr auto unpack_args(typename std::conditional<remove_from_map, tools::args_map_type, const tools::args_map_type>::type& m, KV&&... args)
  {
    return std::make_tuple(C::template get_arg<throw_if_failed, remove_from_map>(m, std::forward<KV>(args))...);
  }
}

SEXP error_Ret(const char* str_UTF8, SEXP retVal = R_NilValue);

template<bool bCheckCOM>
inline SEXP showError(const char* strErrDefaultUTF8 = "unknown error")
{
  if (bCheckCOM)
  {
    auto str = _api->getLastComError();
    if (!str.empty())
      return error_Ret(toUtf8(str.c_str()).c_str());
  }
  return error_Ret(strErrDefaultUTF8);
}

using FN0 = SEXP(*)();
using FN1 = SEXP(*)(SEXP);
using FN2 = SEXP(*)(SEXP, SEXP);
using FN3 = SEXP(*)(SEXP, SEXP, SEXP);
using FN4 = SEXP(*)(SEXP, SEXP, SEXP, SEXP);

struct fn
{
  const int narg;
  union U {
    const FN0 f0;
    const FN1 f1;
    const FN2 f2;
    const FN3 f3;
    const FN4 f4;
    constexpr U(const FN0& f) : f0(f){}
    constexpr U(const FN1& f) : f1(f){}
    constexpr U(const FN2& f) : f2(f){}
    constexpr U(const FN3& f) : f3(f){}
    constexpr U(const FN4& f) : f4(f){}
  }f;
  const SEXP args[4];
  fn() = delete;

  template<class F, class... Args>
  constexpr fn(const F& fn, Args&&... a) : narg(sizeof...(Args)), f(fn), args{a...} {}

//private:
  constexpr SEXP call() const
  {
    switch (narg)
    {
      case 4: return f.f4(args[0], args[1], args[2], args[3]);
      case 3: return f.f3(args[0], args[1], args[2]);
      case 2: return f.f2(args[0], args[1]);
      case 1: return f.f1(args[0]);
      case 0: return f.f0();
      default:
        ATLASSERT(0);
        return R_NilValue;
    }
  }

  template<class T, class F, F f, class ...A>
  static SEXP R(SEXP self, A... a)
  {
    return fn::wrap({ &rtl::template _member_fn<T, F, f, A...>, self, std::forward<A>(a)...});
  }

  template<class F, F f, class ...A>
  static SEXP R(A... a)
  {
    return fn::wrap({f, std::forward<A>(a)...});
  }

private:
  static SEXP wrap(const fn &it);
};
