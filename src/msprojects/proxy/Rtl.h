#pragma once
#ifndef RTL_H
  #define RTL_H
#endif

namespace rtl
{
  //forward declare
  template <class T, class F, F f, class... Args>
  static SEXP _member_fn(SEXP self, Args... a);
}
#include "tools.h"

constexpr char SLOT_PTR_NAME[] = ".ptr";
//#define SLOT_CLASS_NAME ".ptr_class"
namespace rtl
{
  class object
  {
  public:
    static SEXP s_name_ptr() { static SEXP name_ptr = Rf_install(SLOT_PTR_NAME); return name_ptr; }
    bool validate(SEXP self){ return true; }
    virtual ~object(){}
  };

  template <class T>
  static void destroyT(SEXP p)
  {
    if (p != nullptr && TYPEOF(p) == EXTPTRSXP)
    {
      auto ptr = reinterpret_cast<T*>(R_ExternalPtrAddr(p));
      delete ptr;
      R_ClearExternalPtr(p);
    }
  }

  inline static SEXP getExt(SEXP s4)
  {
    if (IS_S4_OBJECT(s4))
      return R_do_slot(s4, object::s_name_ptr());
    else
      return Rf_getAttrib(s4, object::s_name_ptr());
  }

  template <class T>
  inline SEXP createT(SEXP s4)
  {
    ATLASSERT(getCObject<T>(s4) == nullptr);

    auto obj = std::make_unique<T>();
    if (obj->validate(s4))
    {
      tools::protect pt;
      SEXP exptr = pt.add(R_MakeExternalPtr(obj.get(), R_NilValue, R_NilValue));
      if (exptr != nullptr)
      {
        R_RegisterCFinalizerEx(exptr, &destroyT<T>, TRUE);
        obj.release();
      }

      if (IS_S4_OBJECT(s4))
        R_do_slot_assign(s4, object::s_name_ptr(), exptr);
      else
        Rf_setAttrib(s4, object::s_name_ptr(), exptr);

      //R_do_slot_assign(s4, Rf_install(SLOT_CLASS_NAME), tools::newVal(T::class_name));
      return exptr;
    }
    else
    {
      ATLASSERT(0);
    }
    return R_NilValue;
  }

  template <class T>
  inline static SEXP release_internalsT(SEXP s4)
  {
    SEXP ptr = getExt(s4);
    if (ptr)
    {
      destroyT<T>(ptr);
      if (IS_S4_OBJECT(s4))
        R_do_slot_assign(s4, object::s_name_ptr(), R_NilValue);
      else
        Rf_setAttrib(s4, object::s_name_ptr(), R_NilValue);
      //R_ReleaseObject(ptr);
    }
    return R_NilValue;
  }

  template <class T>
  inline static T* getCObject(SEXP s4)
  {
    SEXP ptr = getExt(s4);
    if (TYPEOF(ptr) == EXTPTRSXP)
      return reinterpret_cast<T*>(R_ExternalPtrAddr(ptr));
    return nullptr;
  }

  template <class T, class F, F f, class... Args>
  static SEXP _member_fn(SEXP self, Args... a)
  {
    T* obj = getCObject<T>(self);
    ATLASSERT(obj);
    if (obj) return (obj->*f)(std::forward<Args>(a)...);
    return R_NilValue;
  }

#if 0

  template <class T, SEXP(T::*fn)(SEXP)>
  inline SEXP ext_N(SEXP args)
  {
    args = CDR(args);//skip name
    SEXP self = CAR(args);
    T* obj = getCObject<T>(self);
    ATLASSERT(obj);
    if (obj) return (obj->*fn)(CDR(args));
    return R_NilValue;
  }
#endif
}

#define BEGIN_CALL_MAP(x) public:\
  static const R_CallMethodDef* get_CallMethods(){\
  typedef x _T; static const R_CallMethodDef m[] = {

#define END_CALL_MAP() {NULL, NULL, 0}}; return m; }

#define BEGIN_EXTERNAL_MAP(x) public:\
  static R_ExternalMethodDef* get_ExtMethods(){\
  typedef x _T; static R_ExternalMethodDef m[] = {

#define END_EXTERNAL_MAP() {NULL, NULL, 0}}; return m; }

#if 0
#define END_CALLMETHOD2(super) {NULL, NULL, 0}};\
  static std::vector<R_CallMethodDef> m2(&m[0], &m[_countof(m)]);\
  R_CallMethodDef* ms = super::get_CallMethods();\
  while (ms->name)                               \
    m2.push_back(*ms);                           \
  m2.push_back(*ms);                             \
  return &m2[0];                                 \
 }
#endif
