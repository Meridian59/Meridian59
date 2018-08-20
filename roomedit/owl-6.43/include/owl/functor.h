//-----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1996 by Borland International, All Rights Reserved
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
/// C++ Functor template implementation
///
/// The templates meant for actual end use are:
///
/// Void return, N arguments
/// \code
///  TFunctorV0
///  TFunctorV1<P1>
///  TFunctorV2<P1,P2>
///  TFunctorV3<P1,P2,P3>
///  TFunctorV4<P1,P2,P3,P4>
///  TFunctorV5<P1,P2,P3,P4,P5>
/// \endcode
///
/// Return R, N arguments
/// \code
///  TFunctor0<R>
///  TFunctor1<R,P1>
///  TFunctor2<R,P1,P2>
///  TFunctor3<R,P1,P2,P3>
///  TFunctor4<R,P1,P2,P3,P4>
///  TFunctor5<R,P1,P2,P3,P4,P5>
/// \endcode
///
/// The Functor() template function is invoked to create a temporary translator
/// to initialize the actual functor. Currently, the first argument to Functor()
/// must be a dummy pointer to the desired TFunctor template. When templatized
/// member functions are available, this arg will be eliminated.
///
/// Based on Functors described in
///  Rich Hickey's 'Callbacks in C++ using template functors' Feb 95 C++ Report
//
//------------------------------------------------------------------------------

#if !defined(BDTF_H) && !defined(OWL_FUNCTOR_H)
#define BDTF_H
#define OWL_FUNCTOR_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <ctype.h>  // get size_t definition

#if defined(BI_NO_LVAL_CASTS)
#define THUNKREF(th) th
#else
#define THUNKREF(th) const th &
#endif

namespace owl {

#include <owl/preclass.h>

//
/// Base functor class holds all of the actual data
//
class _OWLCLASS TFunctorBase{
  public:
    operator bool() const {return Func || Callee;}
    typedef void (TFunctorBase::*TMemberFunc)();

    TFunctorBase()
       : Func(0),Callee(0) {}
    TFunctorBase(const void* c, const void* f, size_t sz);

    union {
      const void* Func;
      char MemberFunc[sizeof(TMemberFunc)];  ///< ASSUMES this is always enough space
    };
    void* Callee;
};

//------------------------------------------------------------------------------
/// Functor with 0 parameters, void return

class TFunctorV0 : public TFunctorBase {
  public:
    TFunctorV0() {}
    void operator()() const {
      Thunk(*this);
    }

  protected:
    typedef void (*TThunk)(const TFunctorBase&);
    TFunctorV0(THUNKREF(TThunk) t, const void* c, const void* f, size_t sz)
      : TFunctorBase(c,f,sz), Thunk(t) {}

  private:
    TThunk Thunk;
};

template <class Funct>
class TFunctionTranslatorV0 : public TFunctorV0 {
  public:
    TFunctionTranslatorV0(Funct f) : TFunctorV0(Thunk,0,f,0) {}

    static void Thunk(const TFunctorBase& ftor)
    {
      (Funct(ftor.Func))();
    }
};

template <class TCallee, class TMemFunc>
class TMemberTranslatorV0 : public TFunctorV0 {
  public:
    TMemberTranslatorV0(TCallee& c, const TMemFunc& mf)
      : TFunctorV0(Thunk, &c, &mf, sizeof(TMemFunc)) {}

    static void Thunk(const TFunctorBase& ftor)
    {
      TCallee* callee = reinterpret_cast<TCallee*>(ftor.Callee);
      TMemFunc& memFunc(*(TMemFunc*)(void*)ftor.MemberFunc);
      (callee->*memFunc)();
    }
};

template <class TRT>
inline TFunctorV0
Functor(TFunctorV0*, TRT(*f)())
{
  return TFunctionTranslatorV0<TRT(*)()>(f);
}

template <class TCallee, class TRT, class TCallType>
inline TFunctorV0
Functor(TFunctorV0*, TCallee& c, TRT(TCallType::* const& f)())
{
  typedef TRT (TCallType::*TMemFunc)();
  return TMemberTranslatorV0<TCallee, TMemFunc>(c,f);
}

template <class TCallee, class TRT, class TCallType>
inline TFunctorV0
Functor(TFunctorV0*, const TCallee& c, TRT(TCallType::* const& f)() const)
{
  typedef TRT (TCallType::*TMemFunc)() const;
  return TMemberTranslatorV0<const TCallee, TMemFunc>(c,f);
}

//------------------------------------------------------------------------------
/// Functor with 0 parameters, return R

template <class R>
class TFunctor0 : public TFunctorBase{
  public:
    TFunctor0() {}

    R operator()() const {
      return Thunk(*this);
    }

  protected:
    typedef R (*TThunk)(const TFunctorBase&);
    TFunctor0(THUNKREF(TThunk) t, const void* c, const void* f, size_t sz)
      : TFunctorBase(c,f,sz), Thunk(t) {}

  private:
    TThunk Thunk;
};

template <class R, class Funct>
class TFunctionTranslator0 : public TFunctor0<R>{
  public:
    TFunctionTranslator0(Funct f) : TFunctor0<R>(Thunk,0,(void*)f,0) {}

    static R Thunk(const TFunctorBase& ftor)
    {
      return (Funct(ftor.Func))();
    }
};

template <class R, class TCallee, class TMemFunc>
class TMemberTranslator0 : public TFunctor0<R>{
  public:
    TMemberTranslator0(TCallee& c, const TMemFunc& mf)
      : TFunctor0<R>(Thunk, &c, &mf, sizeof(TMemFunc)) {}

    static R Thunk(const TFunctorBase& ftor)
    {
      TCallee* callee = reinterpret_cast<TCallee*>(ftor.Callee);
      TMemFunc& memFunc(*(TMemFunc*)(void*)ftor.MemberFunc);
      return (callee->*memFunc)();
    }
};

template <class R, class TRT>
inline TFunctor0<R>
Functor(TFunctor0<R>*, TRT(*f)())
{
  return TFunctionTranslator0<R,TRT(*)()>(f);
}

template <class R, class TCallee, class TRT, class TCallType>
inline TFunctor0<R>
Functor(TFunctor0<R>*, TCallee& c, TRT(TCallType::* const& f)())
{
  typedef TRT (TCallType::*TMemFunc)();
  return TMemberTranslator0<R, TCallee, TMemFunc>(c,f);
}

template <class R, class TCallee, class TRT, class TCallType>
inline TFunctor0<R>
Functor(TFunctor0<R>*, const TCallee& c, TRT(TCallType::* const& f)() const)
{
  typedef TRT (TCallType::*TMemFunc)() const;
  return TMemberTranslator0<R, const TCallee, TMemFunc>(c,f);
}

//------------------------------------------------------------------------------
/// Functor with 1 parameter, void return

template <class P1>
class TFunctorV1 : public TFunctorBase{
  public:
    TFunctorV1() {}

    void operator()(P1 p1) const
    {
      Thunk(*this, p1);
    }

  protected:
    typedef void (*TThunk)(const TFunctorBase&, P1);
    TFunctorV1(THUNKREF(TThunk) t, const void* c, const void* f, size_t sz)
      : TFunctorBase(c,f,sz), Thunk(t) {}

  private:
    TThunk Thunk;
};

template <class P1, class Funct>
class TFunctionTranslatorV1 : public TFunctorV1<P1>{
  public:
    TFunctionTranslatorV1(Funct f) : TFunctorV1<P1>(Thunk,0,(void*)f,0) {}

    static void Thunk(const TFunctorBase& ftor, P1 p1)
    {
      (Funct(ftor.Func))(p1);
    }
};

template <class P1, class TCallee, class TMemFunc>
class TMemberTranslatorV1 : public TFunctorV1<P1>{
  public:
    TMemberTranslatorV1(TCallee& c, const TMemFunc& mf)
      : TFunctorV1<P1>(Thunk, &c, &mf, sizeof(TMemFunc)) {}

    static void Thunk(const TFunctorBase& ftor, P1 p1)
    {
      TCallee* callee = reinterpret_cast<TCallee*>(ftor.Callee);
      TMemFunc& memFunc(*(TMemFunc*)(void*)ftor.MemberFunc);
      (callee->*memFunc)(p1);
    }
};

template <class P1, class TRT, class TP1>
inline TFunctorV1<P1>
Functor(TFunctorV1<P1>*, TRT(*f)(TP1))
{
  return TFunctionTranslatorV1<P1, TRT(*)(TP1)>(f);
}

template <class P1, class TCallee, class TRT, class TCallType, class TP1>
inline TFunctorV1<P1>
Functor(TFunctorV1<P1>*, TCallee& c, TRT (TCallType::* const& f)(TP1))
{
  typedef TRT (TCallType::*TMemFunc)(TP1);
  return TMemberTranslatorV1<P1, TCallee, TMemFunc>(c,f);
}

template <class P1, class TCallee, class TRT, class TCallType, class TP1>
inline TFunctorV1<P1>
Functor(TFunctorV1<P1>*, const TCallee& c, TRT(TCallType::* const& f)(TP1) const)
{
  typedef TRT (TCallType::*TMemFunc)(TP1) const;
  return TMemberTranslatorV1<P1, const TCallee, TMemFunc>(c,f);
}

//------------------------------------------------------------------------------
/// Functor with 1 parameter, return R

template <class R, class P1>
class TFunctor1 : public TFunctorBase{
  public:
    TFunctor1() {}

    R operator()(P1 p1) const
    {
      return Thunk(*this, p1);
    }

  protected:
    typedef R (*TThunk)(const TFunctorBase&, P1);
    TFunctor1(THUNKREF(TThunk) t, const void* c, const void* f, size_t sz)
      : TFunctorBase(c,f,sz), Thunk(t) {}

  private:
    TThunk Thunk;
};

template <class R, class P1, class Funct>
class TFunctionTranslator1 : public TFunctor1<R,P1>{
  public:
    TFunctionTranslator1(Funct f) : TFunctor1<R,P1>(Thunk,0,(void*)f,0) {}

    static R Thunk(const TFunctorBase& ftor, P1 p1)
    {
      return (Funct(ftor.Func))(p1);
    }
};

template <class R, class P1, class TCallee, class TMemFunc>
class TMemberTranslator1 : public TFunctor1<R,P1>{
  public:
    TMemberTranslator1(TCallee& c, const TMemFunc& mf)
      : TFunctor1<R,P1>(Thunk, &c, &mf, sizeof(TMemFunc)) {}

    static R Thunk(const TFunctorBase& ftor, P1 p1)
    {
      TCallee* callee = reinterpret_cast<TCallee*>(ftor.Callee);
      TMemFunc& memFunc(*(TMemFunc*)(void*)ftor.MemberFunc);
      return (callee->*memFunc)(p1);
    }
};

template <class R, class P1, class TRT, class TP1>
inline TFunctor1<R,P1>
Functor(TFunctor1<R,P1>*, TRT(*f)(TP1))
{
  return TFunctionTranslator1<R, P1, TRT(*)(TP1)>(f);
}

template <class R, class P1, class TCallee, class TRT, class TCallType, class TP1>
inline TFunctor1<R,P1>
Functor(TFunctor1<R,P1>*, TCallee& c, TRT(TCallType::* const& f)(TP1))
{
  typedef TRT (TCallType::*TMemFunc)(TP1);
  return TMemberTranslator1<R, P1, TCallee, TMemFunc>(c,f);
}

template <class R, class P1, class TCallee, class TRT, class TCallType, class TP1>
inline TFunctor1<R,P1>
Functor(TFunctor1<R,P1>*, const TCallee& c, TRT(TCallType::* const& f)(TP1) const)
{
  typedef TRT (TCallType::*TMemFunc)(TP1) const;
  return TMemberTranslator1<R, P1, const TCallee, TMemFunc>(c,f);
}

//------------------------------------------------------------------------------
/// Functor with 2 parameters, void return

template <class P1, class P2>
class TFunctorV2 : public TFunctorBase{
  public:
    TFunctorV2() {}

    void operator()(P1 p1, P2 p2) const
    {
      Thunk(*this, p1, p2);
    }

  protected:
    typedef void (*TThunk)(const TFunctorBase&, P1, P2);
    TFunctorV2(THUNKREF(TThunk) t, const void* c, const void* f, size_t sz)
      : TFunctorBase(c,f,sz), Thunk(t) {}

  private:
    TThunk Thunk;
};

template <class P1, class P2, class Funct>
class TFunctionTranslatorV2 : public TFunctorV2<P1,P2>{
  public:
    TFunctionTranslatorV2(Funct f) : TFunctorV2<P1,P2>(Thunk,0,(void*)f,0) {}

    static void Thunk(const TFunctorBase& ftor, P1 p1, P2 p2)
    {
      (Funct(ftor.Func))(p1, p2);
    }
};

template <class P1, class P2, class TCallee, class TMemFunc>
class TMemberTranslatorV2 : public TFunctorV2<P1,P2>{
  public:
    TMemberTranslatorV2(TCallee& c, const TMemFunc& mf)
      : TFunctorV2<P1,P2>(Thunk, &c, &mf, sizeof(TMemFunc)) {}

    static void Thunk(const TFunctorBase& ftor, P1 p1, P2 p2)
    {
      TCallee* callee = reinterpret_cast<TCallee*>(ftor.Callee);
      TMemFunc& memFunc(*(TMemFunc*)(void*)ftor.MemberFunc);
      (callee->*memFunc)(p1, p2);
    }
};

template <class P1, class P2, class TRT, class TP1, class TP2>
inline TFunctorV2<P1,P2>
Functor(TFunctorV2<P1,P2>*, TRT (*f)(TP1,TP2))
{
  return TFunctionTranslatorV2<P1, P2, TRT(*)(TP1,TP2)>(f);
}

template <class P1, class P2,
          class TCallee, class TRT, class TCallType,
          class TP1, class TP2>
inline TFunctorV2<P1,P2>
Functor(TFunctorV2<P1,P2>*, TCallee& c, TRT (TCallType::* const& f)(TP1,TP2))
{
  typedef TRT (TCallType::*TMemFunc)(TP1,TP2);
  return TMemberTranslatorV2<P1, P2, TCallee, TMemFunc>(c,f);
}

template <class P1, class P2,
          class TCallee, class TRT, class TCallType,
          class TP1, class TP2>
inline TFunctorV2<P1,P2>
Functor(TFunctorV2<P1,P2>*, const TCallee& c, TRT(TCallType::* const& f)(TP1,TP2) const)
{
  typedef TRT (TCallType::*TMemFunc)(TP1,TP2) const;
  return TMemberTranslatorV2<P1, P2, const TCallee, TMemFunc>(c,f);
}

//------------------------------------------------------------------------------
/// Functor with 2 parameters, return R

template <class R, class P1, class P2>
class TFunctor2 : public TFunctorBase{
  public:
    TFunctor2() {}

    R operator()(P1 p1, P2 p2) const
    {
      return Thunk(*this, p1, p2);
    }

    operator bool() const {return bool();} //JJH exported for COOLEDIT

  protected:
    typedef R (*TThunk)(const TFunctorBase&, P1, P2);
    TFunctor2(THUNKREF(TThunk) t, const void* c, const void* f, size_t sz)
      : TFunctorBase(c,f,sz), Thunk(t) {}

  private:
    TThunk Thunk;
};

template <class R, class P1, class P2, class Funct>
class TFunctionTranslator2 : public TFunctor2<R,P1,P2>{
  public:
    TFunctionTranslator2(Funct f) : TFunctor2<R,P1,P2>(Thunk,0,(void*)f,0) {}

    static R Thunk(const TFunctorBase& ftor, P1 p1, P2 p2)
    {
      return (Funct(ftor.Func))(p1, p2);
    }
};

template <class R, class P1, class P2, class TCallee, class TMemFunc>
class TMemberTranslator2 : public TFunctor2<R,P1,P2>{
  public:
    TMemberTranslator2(TCallee& c, const TMemFunc& mf)
      : TFunctor2<R,P1,P2>(Thunk, &c, &mf, sizeof(TMemFunc)) {}

    static R Thunk(const TFunctorBase& ftor, P1 p1, P2 p2)
    {
      TCallee* callee = reinterpret_cast<TCallee*>(ftor.Callee);
      TMemFunc& memFunc(*(TMemFunc*)(void*)ftor.MemberFunc);
      return (callee->*memFunc)(p1, p2);
    }
};

template <class R, class P1, class P2, class TRT, class TP1, class TP2>
inline TFunctor2<R,P1,P2>
Functor(TFunctor2<R,P1,P2>*, TRT(*f)(TP1,TP2))
{
  return TFunctionTranslator2<R, P1, P2, TRT(*)(TP1,TP2)>(f);
}

template <class R, class P1, class P2,
          class TCallee, class TRT, class TCallType,
          class TP1, class TP2>
inline TFunctor2<R,P1,P2>
Functor(TFunctor2<R,P1,P2>*, TCallee& c, TRT(TCallType::* const& f)(TP1,TP2))
{
  typedef TRT (TCallType::*TMemFunc)(TP1,TP2);
  return TMemberTranslator2<R, P1, P2, TCallee, TMemFunc>(c,f);
}

template <class R, class P1, class P2,
          class TCallee, class TRT, class TCallType,
          class TP1, class TP2>
inline TFunctor2<R,P1,P2>
Functor(TFunctor2<R,P1,P2>*, const TCallee& c, TRT(TCallType::* const& f)(TP1,TP2) const)
{
  typedef TRT (TCallType::*TMemFunc)(TP1,TP2) const;
  return TMemberTranslator2<R, P1, P2, const TCallee, TMemFunc>(c,f);
}

//------------------------------------------------------------------------------
/// Functor with 3 parameters, void return

template <class P1, class P2, class P3>
class TFunctorV3 : public TFunctorBase{
  public:
    TFunctorV3() {}

    void operator()(P1 p1, P2 p2, P3 p3) const
    {
      Thunk(*this, p1, p2, p3);
    }

  protected:
    typedef void (*TThunk)(const TFunctorBase&, P1, P2, P3);
    TFunctorV3(THUNKREF(TThunk) t, const void* c, const void* f, size_t sz)
      : TFunctorBase(c,f,sz), Thunk(t) {}

  private:
    TThunk Thunk;
};

template <class P1, class P2, class P3, class Funct>
class TFunctionTranslatorV3 : public TFunctorV3<P1,P2,P3>{
  public:
    TFunctionTranslatorV3(Funct f) : TFunctorV3<P1,P2,P3>(Thunk,0,(void*)f,0) {}

    static void Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3)
    {
      (Funct(ftor.Func))(p1, p2, p3);
    }
};

template <class P1, class P2, class P3, class TCallee, class TMemFunc>
class TMemberTranslatorV3 : public TFunctorV3<P1,P2,P3>{
  public:
    TMemberTranslatorV3(TCallee& c, const TMemFunc& mf)
      : TFunctorV3<P1,P2,P3>(Thunk, &c, &mf, sizeof(TMemFunc)) {}

    static void Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3)
    {
      TCallee* callee = reinterpret_cast<TCallee*>(ftor.Callee);
      TMemFunc& memFunc(*(TMemFunc*)(void*)ftor.MemberFunc);
      (callee->*memFunc)(p1, p2, p3);
    }
};

template <class P1, class P2, class P3,
          class TRT, class TP1, class TP2, class TP3>
inline TFunctorV3<P1,P2,P3>
Functor(TFunctorV3<P1,P2,P3>*, TRT(*f)(TP1,TP2,TP3))
{
  return TFunctionTranslatorV3<P1, P2, P3, TRT(*)(TP1,TP2,TP3)>(f);
}

template <class P1, class P2, class P3,
          class TCallee, class TRT, class CallType,
          class TP1, class TP2, class TP3>
inline TFunctorV3<P1,P2,P3>
Functor(TFunctorV3<P1,P2,P3>*, TCallee& c, TRT(CallType::* const& f)(TP1,TP2,TP3))
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3);
  return TMemberTranslatorV3<P1, P2, P3, TCallee, TMemFunc>(c,f);
}

template <class P1, class P2, class P3,
          class TCallee, class TRT, class CallType,
          class TP1, class TP2, class TP3>
inline TFunctorV3<P1,P2,P3>
Functor(TFunctorV3<P1,P2,P3>*, const TCallee& c, TRT(CallType::* const& f)(TP1,TP2,TP3) const)
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3) const;
  return TMemberTranslatorV3<P1, P2, P3, const TCallee, TMemFunc>(c,f);
}

//------------------------------------------------------------------------------
/// Functor with 3 parameters, return R

template <class R, class P1, class P2, class P3>
class TFunctor3 : public TFunctorBase{
  public:
    TFunctor3() {}

    R operator()(P1 p1, P2 p2, P3 p3) const
    {
      return Thunk(*this, p1, p2, p3);
    }

  protected:
    typedef R (*TThunk)(const TFunctorBase&, P1, P2, P3);
    TFunctor3(THUNKREF(TThunk) t, const void* c, const void* f, size_t sz)
      : TFunctorBase(c,f,sz), Thunk(t) {}

  private:
    TThunk Thunk;
};

template <class R, class P1, class P2, class P3, class Funct>
class TFunctionTranslator3 : public TFunctor3<R,P1,P2,P3>{
  public:
    TFunctionTranslator3(Funct f) : TFunctor3<R,P1,P2,P3>(Thunk,0,(void*)f,0) {}

    static R Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3)
    {
      return (Funct(ftor.Func))(p1, p2, p3);
    }
};

template <class R, class P1, class P2, class P3, class TCallee, class TMemFunc>
class TMemberTranslator3 : public TFunctor3<R,P1,P2,P3>{
  public:
    TMemberTranslator3(TCallee& c, const TMemFunc& mf)
      : TFunctor3<R,P1,P2,P3>(Thunk, &c, &mf, sizeof(TMemFunc)) {}

    static R Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3)
    {
      TCallee* callee = reinterpret_cast<TCallee*>(ftor.Callee);
      TMemFunc& memFunc(*(TMemFunc*)(void*)ftor.MemberFunc);
      return (callee->*memFunc)(p1, p2, p3);
    }
};

template <class R, class P1, class P2, class P3,
          class TRT, class TP1, class TP2, class TP3>
inline TFunctor3<R,P1,P2,P3>
Functor(TFunctor3<R,P1,P2,P3>*, TRT(*f)(TP1,TP2,TP3))
{
  return TFunctionTranslator3<R, P1, P2, P3, TRT(*)(TP1,TP2,TP3)>(f);
}

template <class R, class P1, class P2, class P3,
          class TCallee, class TRT, class CallType,
          class TP1, class TP2, class TP3>
inline TFunctor3<R,P1,P2,P3>
Functor(TFunctor3<R,P1,P2,P3>*, TCallee& c, TRT (CallType::* const& f)(TP1,TP2,TP3))
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3);
  return TMemberTranslator3<R, P1, P2, P3, TCallee, TMemFunc>(c,f);
}

template <class R, class P1, class P2, class P3,
          class TCallee, class TRT, class CallType,
          class TP1, class TP2, class TP3>
inline TFunctor3<R,P1,P2,P3>
Functor(TFunctor3<R,P1,P2,P3>*, const TCallee& c, TRT(CallType::* const& f)(TP1,TP2,TP3) const)
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3) const;
  return TMemberTranslator3<R, P1, P2, P3, const TCallee, TMemFunc>(c,f);
}

//------------------------------------------------------------------------------
/// Functor with 4 parameters, void return

template <class P1, class P2, class P3, class P4>
class TFunctorV4 : public TFunctorBase{
  public:
    TFunctorV4() {}

    void operator()(P1 p1, P2 p2, P3 p3, P4 p4) const
    {
      Thunk(*this, p1, p2, p3, p4);
    }

  protected:
    typedef void (*TThunk)(const TFunctorBase&, P1, P2, P3, P4);
    TFunctorV4(THUNKREF(TThunk) t, const void* c, const void* f, size_t sz)
      : TFunctorBase(c,f,sz), Thunk(t) {}

  private:
    TThunk Thunk;
};

template <class P1, class P2, class P3, class P4, class Funct>
class TFunctionTranslatorV4 : public TFunctorV4<P1,P2,P3,P4>{
  public:
    TFunctionTranslatorV4(Funct f) : TFunctorV4<P1,P2,P3,P4>(Thunk,0,(void*)f,0) {}

    static void Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3, P4 p4)
    {
      (Funct(ftor.Func))(p1, p2, p3, p4);
    }
};

template <class P1, class P2, class P3, class P4, class TCallee, class TMemFunc>
class TMemberTranslatorV4 : public TFunctorV4<P1,P2,P3,P4>{
  public:
    TMemberTranslatorV4(TCallee& c, const TMemFunc& mf)
      : TFunctorV4<P1,P2,P3,P4>(Thunk, &c, &mf, sizeof(TMemFunc)) {}

    static void Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3, P4 p4)
    {
      TCallee* callee = reinterpret_cast<TCallee*>(ftor.Callee);
      TMemFunc& memFunc(*(TMemFunc*)(void*)ftor.MemberFunc);
      (callee->*memFunc)(p1, p2, p3, p4);
    }
};

template <class P1, class P2, class P3, class P4,
          class TRT, class TP1, class TP2, class TP3, class TP4>
inline TFunctorV4<P1,P2,P3,P4>
Functor(TFunctorV4<P1,P2,P3,P4>*, TRT(*f)(TP1,TP2,TP3,TP4))
{
  return TFunctionTranslatorV4<P1, P2, P3, P4, TRT(*)(TP1,TP2,TP3,TP4)>(f);
}

template <class P1, class P2, class P3, class P4, class TCallee,
          class TRT, class CallType,
          class TP1, class TP2, class TP3, class TP4>
inline TFunctorV4<P1,P2,P3,P4>
Functor(TFunctorV4<P1,P2,P3,P4>*, TCallee& c, TRT(CallType::* const& f)(TP1,TP2,TP3,TP4))
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3,TP4);
  return TMemberTranslatorV4<P1, P2, P3, P4, TCallee, TMemFunc>(c,f);
}

template <class P1, class P2, class P3, class P4,
          class TCallee, class TRT, class CallType,
          class TP1, class TP2, class TP3, class TP4>
inline TFunctorV4<P1,P2,P3,P4>
Functor(TFunctorV4<P1,P2,P3,P4>*, const TCallee& c, TRT(CallType::* const& f)(TP1,TP2,TP3,TP4) const)
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3,TP4) const;
  return TMemberTranslatorV4<P1, P2, P3, P4, const TCallee, TMemFunc>(c,f);
}

//------------------------------------------------------------------------------
/// Functor with 4 parameters, return R

template <class R, class P1, class P2, class P3, class P4>
class TFunctor4 : public TFunctorBase{
  public:
    TFunctor4() {}

    R operator()(P1 p1, P2 p2, P3 p3, P4 p4) const
    {
      return Thunk(*this, p1, p2, p3, p4);
    }

  protected:
    typedef R (*TThunk)(const TFunctorBase&, P1, P2, P3, P4);
    TFunctor4(THUNKREF(TThunk) t, const void* c, const void* f, size_t sz)
      : TFunctorBase(c,f,sz), Thunk(t) {}

  private:
    TThunk Thunk;
};

template <class R, class P1, class P2, class P3, class P4, class Funct>
class TFunctionTranslator4 : public TFunctor4<R,P1,P2,P3,P4>{
  public:
    TFunctionTranslator4(Funct f) : TFunctor4<R,P1,P2,P3,P4>(Thunk,0,(void*)f,0) {}

    static R Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3, P4 p4)
    {
      return (Funct(ftor.Func))(p1, p2, p3, p4);
    }
};

template <class R, class P1, class P2, class P3, class P4, class TCallee, class TMemFunc>
class TMemberTranslator4 : public TFunctor4<R,P1,P2,P3,P4>{
  public:
    TMemberTranslator4(TCallee& c, const TMemFunc& mf)
      : TFunctor4<R,P1,P2,P3,P4>(Thunk, &c, &mf, sizeof(TMemFunc)) {}

    static R Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3, P4 p4)
    {
      TCallee* callee = reinterpret_cast<TCallee*>(ftor.Callee);
      TMemFunc& memFunc(*(TMemFunc*)(void*)ftor.MemberFunc);
      return (callee->*memFunc)(p1, p2, p3, p4);
    }
};

template <class R, class P1, class P2, class P3, class P4,
          class TRT, class TP1, class TP2, class TP3, class TP4>
inline TFunctor4<R,P1,P2,P3,P4>
Functor(TFunctor4<R,P1,P2,P3,P4>*, TRT(*f)(TP1,TP2,TP3,TP4))
{
  return TFunctionTranslator4<R, P1, P2, P3, P4, TRT(*)(TP1,TP2,TP3,TP4)>(f);
}

template <class R, class P1, class P2, class P3, class P4,
          class TCallee, class TRT, class CallType,
          class TP1, class TP2, class TP3, class TP4>
inline TFunctor4<R,P1,P2,P3,P4>
Functor(TFunctor4<R,P1,P2,P3,P4>*, TCallee& c, TRT(CallType::* const& f)(TP1,TP2,TP3,TP4))
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3,TP4);
  return TMemberTranslator4<R, P1, P2, P3, P4, TCallee, TMemFunc>(c,f);
}

template <class R, class P1, class P2, class P3, class P4,
          class TCallee, class TRT, class CallType,
          class TP1, class TP2, class TP3, class TP4>
inline TFunctor4<R,P1,P2,P3,P4>
Functor(TFunctor4<R,P1,P2,P3,P4>*, const TCallee& c, TRT(CallType::* const& f)(TP1,TP2,TP3,TP4) const)
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3,TP4) const;
  return TMemberTranslator4<R, P1, P2, P3, P4, const TCallee, TMemFunc>(c,f);
}

//------------------------------------------------------------------------------
/// Functor with 5 parameters, void return

template <class P1, class P2, class P3, class P4, class P5>
class TFunctorV5 : public TFunctorBase{
  public:
    TFunctorV5() {}

    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const
    {
      Thunk(*this, p1, p2, p3, p4, p5);
    }

  protected:
    typedef void (*TThunk)(const TFunctorBase&, P1, P2, P3, P4, P5);
    TFunctorV5(THUNKREF(TThunk) t, const void* c, const void* f, size_t sz)
      : TFunctorBase(c,f,sz), Thunk(t) {}

  private:
    TThunk Thunk;
};

template <class P1, class P2, class P3, class P4, class P5, class Funct>
class TFunctionTranslatorV5 : public TFunctorV5<P1,P2,P3,P4,P5>{
  public:
    TFunctionTranslatorV5(Funct f) : TFunctorV5<P1,P2,P3,P4,P5>(Thunk,0,(void*)f,0) {}

    static void Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
      (Funct(ftor.Func))(p1, p2, p3, p4, p5);
    }
};

template <class P1, class P2, class P3, class P4, class P5, class TCallee, class TMemFunc>
class TMemberTranslatorV5 : public TFunctorV5<P1,P2,P3,P4,P5>{
  public:
    TMemberTranslatorV5(TCallee& c, const TMemFunc& mf)
      : TFunctorV5<P1,P2,P3,P4,P5>(Thunk, &c, &mf, sizeof(TMemFunc)) {}

    static void Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
      TCallee* callee = reinterpret_cast<TCallee*>(ftor.Callee);
      TMemFunc& memFunc(*(TMemFunc*)(void*)ftor.MemberFunc);
      (callee->*memFunc)(p1, p2, p3, p4, p5);
    }
};

template <class P1, class P2, class P3, class P4, class P5,
          class TRT, class TP1, class TP2, class TP3, class TP4, class TP5>
inline TFunctorV5<P1,P2,P3,P4,P5>
Functor(TFunctorV5<P1,P2,P3,P4,P5>*, TRT(*f)(TP1,TP2,TP3,TP4,TP5))
{
  return TFunctionTranslatorV5<P1, P2, P3, P4, P5, TRT(*)(TP1,TP2,TP3,TP4,TP5)>(f);
}

template <class P1, class P2, class P3, class P4, class P5,
          class TCallee, class TRT, class CallType,
          class TP1, class TP2, class TP3, class TP4, class TP5>
inline TFunctorV5<P1,P2,P3,P4,P5>
Functor(TFunctorV5<P1,P2,P3,P4,P5>*, TCallee& c, TRT(CallType::* const& f)(TP1,TP2,TP3,TP4,TP5))
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3,TP4,TP5);
  return TMemberTranslatorV5<P1, P2, P3, P4, P5, TCallee, TMemFunc>(c,f);
}

template <class P1, class P2, class P3, class P4, class P5,
          class TCallee, class TRT, class CallType,
          class TP1, class TP2, class TP3, class TP4, class TP5>
inline TFunctorV5<P1,P2,P3,P4,P5>
Functor(TFunctorV5<P1,P2,P3,P4,P5>*, const TCallee& c, TRT(CallType::* const& f)(TP1,TP2,TP3,TP4,TP5) const)
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3,TP4,TP5) const;
  return TMemberTranslatorV5<P1, P2, P3, P4, P5, const TCallee, TMemFunc>(c,f);
}

//------------------------------------------------------------------------------
/// Functor with 5 parameters, return R

template <class R, class P1, class P2, class P3, class P4, class P5>
class TFunctor5 : public TFunctorBase{
  public:
    TFunctor5() {}

    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) const
    {
      return Thunk(*this, p1, p2, p3, p4, p5);
    }

  protected:
    typedef R (*TThunk)(const TFunctorBase&, P1, P2, P3, P4, P5);
    TFunctor5(THUNKREF(TThunk) t, const void* c, const void* f, size_t sz)
      : TFunctorBase(c,f,sz), Thunk(t) {}

  private:
    TThunk Thunk;
};

template <class R, class P1, class P2, class P3, class P4, class P5, class Funct>
class TFunctionTranslator5 : public TFunctor5<R,P1,P2,P3,P4,P5>{
  public:
    TFunctionTranslator5(Funct f) : TFunctor5<R,P1,P2,P3,P4,P5>(Thunk,0,(void*)f,0) {}

    static R Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
      return (Funct(ftor.Func))(p1, p2, p3, p4, p5);
    }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class TCallee, class TMemFunc>
class TMemberTranslator5 : public TFunctor5<R,P1,P2,P3,P4,P5>{
  public:
    TMemberTranslator5(TCallee& c, const TMemFunc& mf)
      : TFunctor5<R,P1,P2,P3,P4,P5>(Thunk, &c, &mf, sizeof(TMemFunc)) {}

    static R Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
    {
      TCallee* callee = reinterpret_cast<TCallee*>(ftor.Callee);
      TMemFunc& memFunc(*(TMemFunc*)(void*)ftor.MemberFunc);
      return (callee->*memFunc)(p1, p2, p3, p4, p5);
    }
};

template <class R, class P1, class P2, class P3, class P4, class P5,
          class TRT, class TP1, class TP2, class TP3, class TP4, class TP5>
inline TFunctor5<R,P1,P2,P3,P4,P5>
Functor(TFunctor5<R,P1,P2,P3,P4,P5>*, TRT(*f)(TP1,TP2,TP3,TP4,TP5))
{
  return TFunctionTranslator5<R, P1, P2, P3, P4, P5, TRT(*)(TP1,TP2,TP3,TP4,TP5)>(f);
}

template <class R, class P1, class P2, class P3, class P4, class P5,
          class TCallee, class TRT, class CallType,
          class TP1, class TP2, class TP3, class TP4, class TP5>
inline TFunctor5<R,P1,P2,P3,P4,P5>
Functor(TFunctor5<R,P1,P2,P3,P4,P5>*, TCallee& c, TRT(CallType::* const& f)(TP1,TP2,TP3,TP4,TP5))
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3,TP4,TP5);
  return TMemberTranslator5<R, P1, P2, P3, P4, P5, TCallee, TMemFunc>(c,f);
}

template <class R, class P1, class P2, class P3, class P4, class P5,
          class TCallee, class TRT, class CallType,
          class TP1, class TP2, class TP3, class TP4, class TP5>
inline TFunctor5<R,P1,P2,P3,P4,P5>
Functor(TFunctor5<R,P1,P2,P3,P4,P5>*, const TCallee& c, TRT(CallType::* const& f)(TP1,TP2,TP3,TP4,TP5) const)
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3,TP4,TP5) const;
  return TMemberTranslator5<R, P1, P2, P3, P4, P5, const TCallee, TMemFunc>(c,f);
}

//------------------------------------------------------------------------------
/// Functor with 6 parameters, void return

template <class P1, class P2, class P3, class P4, class P5, class P6>
class TFunctorV6 : public TFunctorBase{
  public:
    TFunctorV6() {}

    void operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) const
    {
      Thunk(*this, p1, p2, p3, p4, p5, p6);
    }

  protected:
    typedef void (*TThunk)(const TFunctorBase&, P1, P2, P3, P4, P5, P6);
    TFunctorV6(THUNKREF(TThunk) t, const void* c, const void* f, size_t sz)
      : TFunctorBase(c,f,sz), Thunk(t) {}

  private:
    TThunk Thunk;
};

template <class P1, class P2, class P3, class P4, class P5, class P6, class Funct>
class TFunctionTranslatorV6 : public TFunctorV6<P1,P2,P3,P4,P5,P6>{
  public:
    TFunctionTranslatorV6(Funct f) : TFunctorV6<P1,P2,P3,P4,P5,P6>(Thunk,0,(void*)f,0) {}

    static void Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    {
      (Funct(ftor.Func))(p1, p2, p3, p4, p5, p6);
    }
};

template <class P1, class P2, class P3, class P4, class P5, class P6, class TCallee, class TMemFunc>
class TMemberTranslatorV6 : public TFunctorV6<P1,P2,P3,P4,P5,P6>{
  public:
    TMemberTranslatorV6(TCallee& c, const TMemFunc& mf)
      : TFunctorV6<P1,P2,P3,P4,P5,P6>(Thunk, &c, &mf, sizeof(TMemFunc)) {}

    static void Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    {
      TCallee* callee = reinterpret_cast<TCallee*>(ftor.Callee);
      TMemFunc& memFunc(*(TMemFunc*)(void*)ftor.MemberFunc);
      (callee->*memFunc)(p1, p2, p3, p4, p5, p6);
    }
};

template <class P1, class P2, class P3, class P4, class P5, class P6,
          class TRT, class TP1, class TP2, class TP3, class TP4, class TP5, class TP6>
inline TFunctorV6<P1,P2,P3,P4,P5,P6>
Functor(TFunctorV6<P1,P2,P3,P4,P5,P6>*, TRT(*f)(TP1,TP2,TP3,TP4,TP5,TP6))
{
  return TFunctionTranslatorV6<P1, P2, P3, P4, P5, P6, TRT(*)(TP1,TP2,TP3,TP4,TP5,TP6)>(f);
}

template <class P1, class P2, class P3, class P4, class P5, class P6,
          class TCallee, class TRT, class CallType,
          class TP1, class TP2, class TP3, class TP4, class TP5, class TP6>
inline TFunctorV6<P1,P2,P3,P4,P5,P6>
Functor(TFunctorV6<P1,P2,P3,P4,P5,P6>*, TCallee& c, TRT(CallType::* const& f)(TP1,TP2,TP3,TP4,TP5,TP6))
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3,TP4,TP5,TP6);
  return TMemberTranslatorV6<P1, P2, P3, P4, P5, P6, TCallee, TMemFunc>(c,f);
}

template <class P1, class P2, class P3, class P4, class P5, class P6,
          class TCallee, class TRT, class CallType,
          class TP1, class TP2, class TP3, class TP4, class TP5, class TP6>
inline TFunctorV6<P1,P2,P3,P4,P5,P6>
Functor(TFunctorV6<P1,P2,P3,P4,P5,P6>*, const TCallee& c, TRT(CallType::* const& f)(TP1,TP2,TP3,TP4,TP5,TP6) const)
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3,TP4,TP5,TP6) const;
  return TMemberTranslatorV6<P1, P2, P3, P4, P5, P6, const TCallee, TMemFunc>(c,f);
}

//------------------------------------------------------------------------------
/// Functor with 6 parameters, return R

template <class R, class P1, class P2, class P3, class P4, class P5, class P6>
class TFunctor6 : public TFunctorBase{
  public:
    TFunctor6() {}

    R operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6) const
    {
      return Thunk(*this, p1, p2, p3, p4, p5, p6);
    }

  protected:
    typedef R (*TThunk)(const TFunctorBase&, P1, P2, P3, P4, P5, P6);
    TFunctor6(THUNKREF(TThunk) t, const void* c, const void* f, size_t sz)
      : TFunctorBase(c,f,sz), Thunk(t) {}

  private:
    TThunk Thunk;
};

template <class R, class P1, class P2, class P3, class P4, class P5, class P6, class Funct>
class TFunctionTranslator6 : public TFunctor6<R,P1,P2,P3,P4,P5,P6>{
  public:
    TFunctionTranslator6(Funct f) : TFunctor6<R,P1,P2,P3,P4,P5,P6>(Thunk,0,(void*)f,0) {}

    static R Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    {
      return (Funct(ftor.Func))(p1, p2, p3, p4, p5, p6);
    }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class P6, class TCallee, class TMemFunc>
class TMemberTranslator6 : public TFunctor6<R,P1,P2,P3,P4,P5,P6>{
  public:
    TMemberTranslator6(TCallee& c, const TMemFunc& mf)
      : TFunctor6<R,P1,P2,P3,P4,P5,P6>(Thunk, &c, &mf, sizeof(TMemFunc)) {}

    static R Thunk(const TFunctorBase& ftor, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
    {
      TCallee* callee = reinterpret_cast<TCallee*>(ftor.Callee);
      TMemFunc& memFunc(*(TMemFunc*)(void*)ftor.MemberFunc);
      return (callee->*memFunc)(p1, p2, p3, p4, p5, p6);
    }
};

template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class TRT, class TP1, class TP2, class TP3, class TP4, class TP5, class TP6>
inline TFunctor6<R,P1,P2,P3,P4,P5,P6>
Functor(TFunctor6<R,P1,P2,P3,P4,P5,P6>*, TRT(*f)(TP1,TP2,TP3,TP4,TP5,TP6))
{
  return TFunctionTranslator6<R, P1, P2, P3, P4, P5, P6, TRT(*)(TP1,TP2,TP3,TP4,TP5,TP6)>(f);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class TCallee, class TRT, class CallType,
          class TP1, class TP2, class TP3, class TP4, class TP5, class TP6>
inline TFunctor6<R,P1,P2,P3,P4,P5,P6>
Functor(TFunctor6<R,P1,P2,P3,P4,P5,P6>*, TCallee& c, TRT(CallType::* const& f)(TP1,TP2,TP3,TP4,TP5,TP6))
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3,TP4,TP5,TP6);
  return TMemberTranslator6<R, P1, P2, P3, P4, P5, P6, TCallee, TMemFunc>(c,f);
}

template <class R, class P1, class P2, class P3, class P4, class P5, class P6,
          class TCallee, class TRT, class CallType,
          class TP1, class TP2, class TP3, class TP4, class TP5, class TP6>
inline TFunctor6<R,P1,P2,P3,P4,P5,P6>
Functor(TFunctor6<R,P1,P2,P3,P4,P5,P6>*, const TCallee& c, TRT(CallType::* const& f)(TP1,TP2,TP3,TP4,TP5,TP6) const)
{
  typedef TRT (CallType::*TMemFunc)(TP1,TP2,TP3,TP4,TP5,TP6) const;
  return TMemberTranslator6<R, P1, P2, P3, P4, P5, P6, const TCallee, TMemFunc>(c,f);
}

#include <owl/posclass.h>

} // OWL namespace

#endif // BDTF_H
