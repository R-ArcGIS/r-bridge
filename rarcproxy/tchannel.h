#pragma once

#ifndef SAFEQ_H
#define SAFEQ_H

#include <list>

template <class T>
class safeQ
{
public:
  typedef T value_type;

  safeQ()
  {
    ::InitializeCriticalSectionAndSpinCount(&m_cs, 0x80000400);
    m_hDataIn = ::CreateEvent(NULL, TRUE, FALSE, NULL);
  }
  ~safeQ()
  {
    ATLASSERT(m_list.empty());//call clear(pred)
    ::DeleteCriticalSection(&m_cs);
    ::CloseHandle(m_hDataIn);
  }
  size_t push(const T& v)
  {
    ::EnterCriticalSection(&m_cs);

    size_t n = m_list.size();
    try
    {
      m_list.push_back(v);
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
          v = m_list.front();
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

  void lock(){ ::EnterCriticalSection(&m_cs); }
  void unlock(){ ::LeaveCriticalSection(&m_cs); }

  HANDLE handle() const { return m_hDataIn; }

  void clear()
  {
    ::EnterCriticalSection(&m_cs);
    m_list.clear();
    ::ResetEvent(m_hDataIn);
    ::LeaveCriticalSection(&m_cs);
  }

  template <class F>
  void for_each(F pred)
  {
    ::EnterCriticalSection(&m_cs);
    std::for_each(m_list.begin(), m_list.end(), pred);
    ::LeaveCriticalSection(&m_cs);
  }

private:
  CRITICAL_SECTION m_cs;
  HANDLE  m_hDataIn;
  std::list<T> m_list;
};

// locking channel
// to balance call between threads
struct fn_struct;
struct tchannel
{
  static tchannel& singleton();

  enum {
    _UNINIT,
    R_INIT,
    R_PROMPT,
    R_CMD,
    R_TXT_OUT,
    FN_CALL,
    FN_CALL_RET,
  };
  struct value
  {
    int type;
    union
    {
      void* _void;
      const fn_struct* data_fn;
      SEXP data_sexp;
      int data_int;
      std::wstring* data_str;
    };
    inline bool empty() const { return _void == NULL; }
    value(): type (_UNINIT), _void(NULL){}
    explicit value(int t, int val) : type(t), data_int(val) {}
    explicit value(int t, const fn_struct* val): type(t), data_fn(val){}
    explicit value(int t, SEXP val): type(t), data_sexp(val)
    {
      if (val) ::R_PreserveObject(val);
    }
    explicit value(int t, std::wstring* val) : type(t), data_str(val){}

    value(const value& c) : type(c.type), _void(c._void) //takes ownership
    { 
      const_cast<value*>(&c)->type = _UNINIT;
      const_cast<value*>(&c)->_void = NULL; 
    }
    ~value()
    {
      replace();
    }

    value& operator=(value& c)//takes ownership
    {
      replace(c.type, c._void);
      c.type = _UNINIT; c._void = NULL; 
      return (*this);
    }
  private:
    void replace(int t = _UNINIT, void* newVal = NULL)
    {
      if (t == type && newVal == _void)
        return;
      if (type == R_PROMPT || type == R_TXT_OUT)
        delete data_str;
      else if (type == FN_CALL_RET && data_sexp)
      {
        ::R_ReleaseObject(data_sexp);
      }
      type = t;
      _void = newVal;
    }
  };
  safeQ<value> to_thread;
  safeQ<value> from_thread;
  std::string all_errors_text;
};

#endif