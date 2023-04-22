#pragma once

#ifndef SAFEQ_H
#define SAFEQ_H

#include <list>
#include <variant>

template <class T>
class safeQ
{
public:
  typedef T value_type;

  constexpr safeQ()
  {
    ::InitializeCriticalSectionAndSpinCount(&m_cs, 0x80000400);
    m_hDataIn = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);
  }
  constexpr ~safeQ()
  {
    ATLASSERT(m_list.empty());//call clear(pred)
    ::DeleteCriticalSection(&m_cs);
    ::CloseHandle(m_hDataIn);
  }
  size_t push_back(T&& v)
  {
    ::EnterCriticalSection(&m_cs);

    size_t n = m_list.size();
    try
    {
      m_list.push_back(std::move(v));
      n = m_list.size();
      //SetEvent(m_hDataIn);
    }catch(...){ }

    ::LeaveCriticalSection(&m_cs);
    // this way it will not lock in pop() but ... ??
    ::SetEvent(m_hDataIn);

    return n;
  }

  bool pop(T& v, DWORD wait)
  {
    while (true)
    {
      DWORD dw = ::WaitForSingleObject(m_hDataIn, wait);

      switch (dw)
      {
        case WAIT_OBJECT_0:
          ::EnterCriticalSection(&m_cs);
          if (m_list.empty())
          {
            ::ResetEvent(m_hDataIn);
            ::LeaveCriticalSection(&m_cs);
            break;
          }
          v = std::move(m_list.front());
          m_list.pop_front();

          if (m_list.empty())
            ::ResetEvent(m_hDataIn);
          ::LeaveCriticalSection(&m_cs);
          return true;
        default:  return false;
      }
    }
    return false;
  }

  constexpr void lock(){ ::EnterCriticalSection(&m_cs); }
  constexpr void unlock(){ ::LeaveCriticalSection(&m_cs); }

  constexpr HANDLE handle() const { return m_hDataIn; }

  constexpr void clear()
  {
    ::EnterCriticalSection(&m_cs);
    m_list.clear();
    ::ResetEvent(m_hDataIn);
    ::LeaveCriticalSection(&m_cs);
  }

  /*template <class F>
  void for_each(F pred)
  {
    ::EnterCriticalSection(&m_cs);
    for(const auto &v : m_list)
      pred(v);
    ::LeaveCriticalSection(&m_cs);
  }*/

private:
  //TODO use c++ mutex
  CRITICAL_SECTION m_cs;
  HANDLE m_hDataIn;
  std::list<T> m_list;
};

// to balance calls between threads
struct tchannel
{
  static tchannel& singleton();

  enum class value_type : uint32_t{
    _UNINIT,
    //R_INIT,
    R_PROMPT, //string
    R_CMD, //int
    R_TXT_OUT, //string
    FN_CALL, //void
    FN_CALL_RET, //SEXP
    INTERNAL
  };
  class value
  {
  public:
    value_type type;
    using value_t = std::variant<std::monostate, SEXP, int, const void*, std::wstring>;
    explicit value() noexcept : type(value_type::_UNINIT) { }
    explicit value(value_type t, SEXP val) : type(t), _v(val) { validate(); if (val) ::R_PreserveObject(val); }
    explicit value(value_type t, int val) noexcept : type(t), _v(val) { validate(); }
    explicit value(value_type t, const void* val) noexcept : type(t), _v(val){ validate(); }
    explicit value(value_type t, const std::wstring& val) noexcept : type(t), _v(val) { validate(); }

    value(const value& c) = delete;
    value(value&& c) noexcept : type(c.type)
    {
      move(c._v, _v);
      c.type = value_type::_UNINIT;
    }
    ~value()
    {
      if (SEXP ptr = _v.index() == 1 ? std::get<1>(_v) : nullptr; ptr != nullptr)
        ::R_ReleaseObject(ptr);
    }

    constexpr bool empty() const noexcept { return _v.index() == 0; }
    value& operator=(value& c) = delete;
    value& operator=(value&& c) noexcept
    {
      type = c.type;
      move(c._v, _v);
      c.type = value_type::_UNINIT;
      return (*this);
    }

    constexpr SEXP data_sexp() const { return _v.index() == 1 ? std::get<1>(_v) : nullptr; }
    constexpr int cmd_id() const { return _v.index() == 2 ? std::get<2>(_v) : -1; }
    constexpr const void* data() const { return _v.index() == 3 ? std::get<3>(_v) : nullptr; }
    constexpr const std::wstring_view data_str() const { return _v.index() == 4 ? std::get<4>(_v) : std::wstring_view{}; }
  private:
    value_t _v;
    constexpr void validate() const noexcept
    {
      switch (type)
      {
        case value_type::R_PROMPT:
        case value_type::R_TXT_OUT:
          ATLASSERT(std::holds_alternative<std::wstring>(_v)); 
        break;
        case value_type::R_CMD: ATLASSERT(std::holds_alternative<int>(_v)); break;
        case value_type::FN_CALL: ATLASSERT(std::holds_alternative<const void*>(_v)); break;
        case value_type::FN_CALL_RET: ATLASSERT(std::holds_alternative<SEXP>(_v)); break;
        default: ATLASSERT(false); break;
      }
    }
    static void move(value_t& from, value_t& to) noexcept
    {
      try {
        SEXP ptr_from = from.index() == 1 ? std::get<1>(from) : nullptr;
        SEXP ptr_to = to.index() == 1 ? std::get<1>(to) : nullptr;
        to = std::move(from);
        if (ptr_from != ptr_to && ptr_to != nullptr)
          ::R_ReleaseObject(ptr_to);
      }
      catch (...) { }
      from = value_t();
    }
  };
  safeQ<value> to_thread;
  safeQ<value> from_thread;
  std::string all_errors_text;
public:
  static bool try_process(const tchannel::value &p)
  {
    if (p.type == value_type::INTERNAL)
    {
      ((const gp_eval*)p.data())->_call();
      return true;
    }
    return false;
  }
  static void gp_thread(std::function<void()> fn)
  {
    extern DWORD g_main_TID;
    if (g_main_TID != ::GetCurrentThreadId())
    {
      gp_eval e(fn);
      e.wait();
    }
    else
      fn();
  }
private:
  struct gp_eval
  {
    using Fn = std::function<void()>;
    gp_eval(Fn fn) :m_fn(fn), m_h(::CreateEvent(NULL, TRUE, FALSE, nullptr)){ }
    ~gp_eval() { ::CloseHandle(m_h); }
    void _call() const
    {
      m_fn();
      ::SetEvent(m_h);
    }
    Fn m_fn;
    HANDLE m_h;
    void wait()
    {
      singleton().from_thread.push_back(tchannel::value{ value_type::INTERNAL, (const void*)this });
      ::WaitForSingleObject(m_h, INFINITE);
    }
  };
};
#endif
