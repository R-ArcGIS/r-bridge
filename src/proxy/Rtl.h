#pragma once
#ifndef RTL_H
  #define RTL_H
#endif

#include "tools.h"

#define SLOT_PTR_NAME ".ptr"
#define SLOT_CLASS_NAME ".ptr_class"
namespace rtl
{
  template <class T>
  void destroyT(SEXP p)
  {
    if (TYPEOF(p) == EXTPTRSXP)
    {
      auto ptr = reinterpret_cast<T*>(R_ExternalPtrAddr(p));
      delete ptr;
    }
  }

  template <class T>
  inline SEXP createT(SEXP s4)
  {
    T* obj = getCObject<T>(s4);
    if (obj)
    {
      //validate type
      SEXP cls = R_do_slot(s4, Rf_install(SLOT_CLASS_NAME));
      std::string c_name;
      tools::copy_to(cls, c_name);
      ATLASSERT(c_name == T::class_name);
      return s4;
    }

    SEXP exptr= R_NilValue;
    tools::protect pt;
    obj = new T();
    if (obj->validate(s4))
    {
      exptr = pt.add(R_MakeExternalPtr(obj, R_NilValue, R_NilValue));
      R_RegisterCFinalizerEx(exptr, &destroyT<T>, TRUE);
      R_do_slot_assign(s4, Rf_install(SLOT_PTR_NAME), exptr);
      R_do_slot_assign(s4, Rf_install(SLOT_CLASS_NAME), tools::newVal(T::class_name));
    }
    else
    {
      ATLASSERT(0);
      delete obj;
      s4 = R_NilValue;
    }
    return s4;
  }

  template <class T>
  T* getCObject(SEXP s4)
  {
    //tools::protect pt;
    SEXP slot_ptr = Rf_install(SLOT_PTR_NAME);
    if (!R_has_slot(s4, slot_ptr))
      return NULL;
    SEXP ptr = R_do_slot(s4, slot_ptr);
    return reinterpret_cast<T*>(R_ExternalPtrAddr(ptr));
  }

  /*template <class T, SEXP(T::*fn)(SEXP)>
  inline SEXP call_s0(SEXP self)
  {
    T* obj = getCObject<T>(self);
    ATLASSERT(obj);
    if (obj) return (obj->*fn)(self);
    return R_NilValue;
  }*/
  template <class T, SEXP(T::*fn)()>
  inline SEXP call_0(SEXP self)
  {
    T* obj = getCObject<T>(self);
    ATLASSERT(obj);
    if (obj) return (obj->*fn)();
    return R_NilValue;
  }
  
  template <class T, SEXP(T::*fn)(SEXP)>
  inline SEXP call_1(SEXP self, SEXP a1)
  {
    T* obj = getCObject<T>(self);
    ATLASSERT(obj);
    if (obj) return (obj->*fn)(a1);
    return R_NilValue;
  }
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
  /*template <class T, SEXP(T::*fn)(SEXP, SEXP)>
  inline SEXP call_s1(SEXP self, SEXP a1)
  {
    T* obj = getCObject<T>(self);
    ATLASSERT(obj);
    if (obj) return (obj->*fn)(self, a1);
    return R_NilValue;
  }*/
  
  template <class T, SEXP(T::*fn)(SEXP, SEXP)>
  inline SEXP call_2(SEXP self, SEXP a1, SEXP a2)
  {
    T* obj = getCObject<T>(self);
    ATLASSERT(obj);
    if (obj) return (obj->*fn)(a1, a2);
    return R_NilValue;
  }
  template <class T, SEXP(T::*fn)(SEXP, SEXP, SEXP)>
  inline SEXP call_3(SEXP self, SEXP a1, SEXP a2, SEXP a3)
  {
    T* obj = getCObject<T>(self);
    ATLASSERT(obj);
    if (obj) return (obj->*fn)(a1, a2, a3);
    return R_NilValue;
  }

  template <class Tbase>
  class object
  {
  public:
    bool validate(SEXP self){ return true; }
  };
}

#define BEGIN_CALL_MAP(x) public:\
  static R_CallMethodDef* get_CallMethods(){\
  typedef x _T; static R_CallMethodDef m[] = {

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
