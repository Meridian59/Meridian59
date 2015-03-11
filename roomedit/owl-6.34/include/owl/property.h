//-------------------------------------------------------------------
// ObjectWindows
// Copyright(c) 1996 by Manic Software. All rights reserved.
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
/// \file
/// Credits:
///      The Property pattern is taken from "Patterns in Practice: A Property
/// Template for C++", from "C++ Report", Nov/Dec 1995, p. 28-33, by Colin
/// Hastie. I've in turn extended it to allow for "true" Delphi-like property
/// syntax by allowing you to assign normal T objects to the TProperty<T>, as well
/// as provide an implict conversion-to-T operator. Note that this implicit
/// conversion is NOT highly held in regard by many C++-ers, and therefore can
/// be \#defined out of existence by \#define-ing STRICT_PROPERTY_SYNTAX before
/// \#include-ing this file.
///      Delphi (and C++Builder) also allow for methods to be used instead of
/// direct access to data members; this, too, is allowed, by making use of the
/// callback mechanism within VDBT/BDTF.H. TProperty<T> can be constructed to
/// take two callbacks, one returning T taking void (get), and the other returning
/// void and taking a const T& (set). These methods will be called in place of
/// accessing the m_content method of Property<>, below. Note that I'm assuming
/// that if you write the 'get', you'll also write the 'set', and that both methods
/// provide their own storage for T (instead of relying on m_content within the
/// TProperty<T>).
///
/// Requirements; TProperty assumes that the type T has meaningful orthodox
/// canonical form semantics (default ctor, copy ctor, assignment op, dtor)
//
//
// Additions:
//          TRefProperty<>
//          TConstRefProperty<>
//          TObjProperty<>
//          TConstObjProperty<>
//          TFuncProperty<>
//          TFuncPropertyGet<>
//          TFuncPropertyPut<>
//          TFuncPropertyVarPut<>
//          TFuncPropertyIdxGet<>
//          TFuncPropertyIdxPut<>
//          TFuncPropertyIdx<>
//
//-------------------------------------------------------------------

#if !defined(OWL_PROPERTY_H)
#define OWL_PROPERTY_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/functor.h>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
//
#include <owl/preclass.h>

/// \addtogroup utility
/// @{

//------------------------------------------------------------------------------
/// simple type/pointer
template <class T>
class TProperty{
  // Object lifetime methods
  //
  public:
    TProperty(){}
    explicit TProperty(const T& t) : Content(t) {}
    ~TProperty() {}

  // Accessors
  //
  public:
    T operator()() const   { return Content; }
#ifndef STRICT_PROPERTY_SYNTAX
    operator T()           { return Content; }
#endif

  // Mutators
  //
  public:
    void operator()(const T& t)           { Content = t; }
#ifndef STRICT_PROPERTY_SYNTAX
    TProperty<T>& operator=(const T& t)   { Content = t; return *this; }
#endif

  // Disallowed methods
  //
  private:
    void operator=(const TProperty<T>&);

  // Internal Data
  //
  private:
    T Content;
};

// simple type/pointer
template <class T>
class TConstProperty{
  // Ctors & Dtors
  //
  public:
    TConstProperty(const T& t) : Content(t) {}
    ~TConstProperty() {}

   // Accessor & Mutator
  //
  public:
    T operator()()              { return Content; }
    const T operator()() const   { return Content; }
#ifndef STRICT_PROPERTY_SYNTAX
    operator T()                 { return Content; }
#endif

  // Disallowed methods
  //
  private:
    void operator=(const TConstProperty<T>&);
    TConstProperty();

  // Data members
  //
  private:
    T Content;
};

//------------------------------------------------------------------------------
// Additions made by Yura Bidus 1998
//------------------------------------------------------------------------------
/// Property container for object
template <class T>
class TObjProperty{
  public:
    explicit TObjProperty(const T& t);
    TObjProperty(T* t);
    TObjProperty();
    ~TObjProperty();

  public:
    operator T*();
    operator const T&() const;
    T* operator->();
    const T& operator*();

    T* operator=( T* t);
    T& operator=( T& t);
    TObjProperty<T>& operator=(const T& t);

    const T& operator()() const;
    void operator()(const T& t);

  private:
    void operator=(const TObjProperty<T>&);

  private:
    T* Content;
};
//------------------------------------------------------------------------------
/// Read only Property container for object
template <class T>
class TConstObjProperty{
  public:
    TConstObjProperty(const T& t):Content(t){}
    ~TConstObjProperty() {}

  public:
    T& operator()()                { return Content; }
    const T& operator()() const   { return Content; }

    operator T*()                 { return &Content;}
    operator const T&()           { return Content; }
    T* operator->()               { return &Content;}
    const T& operator*()          { return Content; }

  private:
    void operator=(const TConstObjProperty<T>&){}
    TConstObjProperty(){}

  private:
    T& Content;
};
//------------------------------------------------------------------------------
// array simple type
template <class T,class I>
class TArrayProperty{
  public:
    class TSetter{
      public:
        I Index;
        operator T()         { return Content; }
        T operator=( T value){ Content[Index]= value; return Content[Index];}
        TSetter& operator=(const TSetter& val){Content=val.Content;return *this;}
      protected:
        T* Content;
    }Name;
    friend class TSetter;

    TArrayProperty(T* value)      { Name.Content = value;}
    TSetter& operator[]( I index) { Name.Index = index; return Name; }

  private:
    TArrayProperty<T,I>& operator=( const TArrayProperty<T,I>& ){;} // not allowed
};
//------------------------------------------------------------------------------
/// array read only array
template <class T,class I>
class TConstArrayProperty{
  public:
    class TGetter{
#if defined(BI_COMP_GNUC)
      friend class TConstArrayProperty;
#else
      friend TConstArrayProperty;
#endif
      public:
        I Index;

        T& operator()()              { return Content[Index]; }
        const T& operator()() const  { return Content[Index]; }
        operator T*()               { return &Content[Index];}
        operator const T&()         { return Content[Index]; }
        const T& operator*()        { return Content[Index]; }

      protected:
        T* Content;
    }Name;
    friend class TGetter;

    TConstArrayProperty(T* value)  { Name.Content = value;}
    TGetter& operator[](I index)   { Name.Index = index; return Name; }
  private:
    TConstArrayProperty<T,I>& operator=( const TConstArrayProperty<T,I>& ); // not allowed
    T* Content;
};
//------------------------------------------------------------------------------
/// array read only array
template <class T,class I>
class TConstObjArrayProperty
{
  public:
    class TGetter
  {
      public:
        I Index;

  T& operator()()              
    { return Content[Index]; }
  const T& operator()() const  
    { return Content[Index]; }
  operator T*()               
    { return &Content[Index];}
  operator const T&()         
    { return Content[Index]; }
  T* operator->()             
    { return &Content[Index];}
  const T& operator*()        
    { return Content[Index]; }

      protected:
        T* Content;
    }Name;
    friend class TGetter;

    TConstObjArrayProperty(T* value)  { Name.Content = value;}
    TGetter& operator[](I index)      { Name.Index = index; return Name; }

  private:
    TConstObjArrayProperty<T,I>& operator=( const TConstObjArrayProperty<T,I>&){;} // not allowed
};
//------------------------------------------------------------------------------

//
// Functors
//
template <class T>
class TFuncProperty{
  public:
    TFuncProperty(const TFunctor0<T> get, const TFunctorV1<T> put)
                 :Get(get),Put(put){}
    ~TFuncProperty() {}

  public:
    T operator()() const   { return Get(); }
    operator T()           { return Get(); }
    void operator()(const T& t) { Put(t); }
    TFuncProperty<T>& operator=(const T& t) { Put(t); return *this; }

  private:
    void operator=(const TFuncProperty<T>&){}
    TFuncProperty(){}

    TFunctor0<T>     Get;
    TFunctorV1<T>   Put;
};

//
// get/put object
//
template <class T>
class TFuncObjProperty{
  public:
    TFuncObjProperty(const TFunctor0<T&> get, const TFunctorV1<const T&> put)
                 :Get(get),Put(put){}
    ~TFuncObjProperty() {}

  public:
    // getters
    operator T*()              { return &Get(); }
    operator const T&() const  { return Get(); }
    T* operator->()            { return &Get();}
    const T& operator*()       { return Get(); }

    //set/get
    //T* operator=( T* t)        { return Put(t); return &Get();}
    T& operator=( T& t)        { Put(t); return Get(); }
    TFuncObjProperty<T>& operator=(const T& t){ Put(t); return *this; }
    //setters
    const T& operator()() const { return Get(); }
    void operator()(const T& t) { Put(t);}

  private:
    void operator=(const TFuncObjProperty<T>&){}
    TFuncObjProperty(){}

    TFunctor0<T&>         Get;
    TFunctorV1<const T&>   Put;

};

//
template <class T>
class TFuncPropertyGet{
  public:
    TFuncPropertyGet(const TFunctor0<T> get):Get(get){}
    ~TFuncPropertyGet() {}

  public:
    T operator()()              { return Get(); }
    const T operator()() const   { return Get(); }

  private:
    void operator=(const TFuncPropertyGet<T>&){}
    TFuncPropertyGet(){}

    TFunctor0<T>   Get;
};

//
template <class T>
class TFuncObjPropertyGet{
  public:
    TFuncObjPropertyGet(const TFunctor0<T&> get):Get(get){}
    ~TFuncObjPropertyGet() {}

  public:
    T& operator()()               { return Get(); }
    const T& operator()() const   { return Get(); }
    operator T*()                 { return &Get(); }
    operator const T&()           { return Get(); }
    T* operator->()               { return &Get();}
    const T& operator*()          { return Get(); }

  private:
    void operator=(const TFuncObjPropertyGet<T>&){}
    TFuncObjPropertyGet(){}

    TFunctor0<T&>  Get;
};

//
template <class T>
class TFuncPropertyPut{
  public:
    TFuncPropertyPut(const TFunctorV1<T> put):Put(put){}
    ~TFuncPropertyPut() {}

  public:
    void operator()(const T& t)                { Put(t);               }
    TFuncPropertyPut<T>& operator=(const T& t) { Put(t); return *this; }

  private:
    void operator=(const TFuncPropertyPut<T>&){}
    TFuncPropertyPut(){}

  TFunctorV1<T> Put;
};

//
template <class T>
class TFuncObjPropertyPut{
  public:
    TFuncObjPropertyPut(const TFunctorV1<const T&> put):Put(put){}
    ~TFuncObjPropertyPut() {}

  public:
    void operator()(const T& t)                   { Put(t);               }
    TFuncObjPropertyPut<T>& operator=(const T& t) { Put(t); return *this; }

  private:
    void operator=(const TFuncObjPropertyPut<T>&){}
    TFuncObjPropertyPut(){}

  TFunctorV1<const T&> Put;
};

//
template <class T>
class TFuncPropertyVarPut{
  public:
    TFuncPropertyVarPut(const T& var, const TFunctorV1<T> put)
                        :Content(var),Put(put){}
    ~TFuncPropertyVarPut(){}

  public:
    T operator()() const      { return Content; }
    operator T()               { return Content; }

    void operator()(const T& t) { Put(t); }
    TFuncPropertyVarPut<T>& operator=(const T& t) { Put(t); return *this; }

  private:
    void operator=(const TFuncPropertyVarPut<T>&);
    TFuncPropertyVarPut();

   const T&       Content;
  TFunctorV1<T> Put;
};


//
template <class T>
class TFuncObjPropertyVarPut{
  public:
    TFuncObjPropertyVarPut(T& var, const TFunctorV1<const T&> put)
                        :Content(&var),Put(put){}
    ~TFuncObjPropertyVarPut(){}

  public:
    // getter
    T& operator()()               { return *Content; }
    const T& operator()() const   { return *Content; }
    operator T*()                 { return Content;  }
    operator const T&()           { return *Content; }
    T* operator->()               { return Content;  }
    const T& operator*()          { return *Content; }

  public:
    //T* operator=( T* t)        { return Put(t); return &Content;}
    T& operator=( T& t)          { Put(t); return *Content; }
    TFuncObjPropertyVarPut<T>& operator=(const T& t) { Put(t); return *this; }
    void operator()(const T& t){ Put(t);}

  private:
    void operator=(const TFuncObjPropertyVarPut<T>&){}
    TFuncObjPropertyVarPut(){}

    TFunctorV1<const T&> Put;
     T* Content;
};

//
template <class T, int index>
class TFuncPropertyIdxGet{
  public:
    TFuncPropertyIdxGet(const TFunctor1<T,int> get):Get(get){}
    ~TFuncPropertyIdxGet() {}

  public:
    T operator()()              { return Get(index); }
    const T operator()() const   { return Get(index); }

  private:
    void operator=(const TFuncPropertyIdxGet<T,index>&){}
    TFuncPropertyIdxGet(){}

    TFunctor1<T,int>   Get;
};

//
template <class T, int index>
class TFuncObjPropertyIdxGet{
  public:
    TFuncObjPropertyIdxGet(const TFunctor1<T&,int> get):Get(get){}
    ~TFuncObjPropertyIdxGet() {}

  public:
    T& operator()()               { return Get(index); }
    const T& operator()() const   { return Get(index); }
    operator T*()                 { return &Get(index);}
    operator const T&()           { return Get(index); }
    T* operator->()               { return &Get(index);}
    const T& operator*()          { return Get(index); }

  private:
    void operator=(const TFuncObjPropertyIdxGet<T,index>&){}
    TFuncObjPropertyIdxGet(){}

  TFunctor1<T&,int> Get;
};

//
template <class T, int index>
class TFuncPropertyIdxPut{
  public:
    TFuncPropertyIdxPut(const TFunctorV2<int,T> put):Put(put){}
    ~TFuncPropertyIdxPut() {}

  public:
    void operator()(const T& t) { Put(index, t); }
    TFuncPropertyIdxPut<T,index>& operator=(const T& t) { Put(index,t); return *this; }

  private:
    void operator=(const TFuncPropertyIdxPut<T,index>&);
    TFuncPropertyIdxPut();

  TFunctorV2<int,T> Put;
};

//
template <class T, int index>
class TFuncObjPropertyIdxPut{
  public:
    TFuncObjPropertyIdxPut(const TFunctorV2<int,const T&> put):Put(put){}
    ~TFuncObjPropertyIdxPut() {}

  public:
    void operator()(const T& t) { Put(index, t); }
    TFuncObjPropertyIdxPut<T,index>& operator=(const T& t) { Put(index,t); return *this; }

  private:
    void operator=(const TFuncObjPropertyIdxPut<T,index>&);
    TFuncObjPropertyIdxPut();

  TFunctorV2<int,const T&> Put;
};

//
template <class T, int index>
class TFuncPropertyIdx{
  public:
    TFuncPropertyIdx(const TFunctor1<T,int> get,
                     const TFunctorV2<int,T> put):
                     Get(get),Put(put){}
    ~TFuncPropertyIdx() {}

  public:
    T operator()() const   { return Get(index); }
    operator T()           { return Get(index); }

    void operator()(const T& t) { Put(index, t); }
    TFuncPropertyIdx<T,index>& operator=(const T& t){Put(index,t);return *this;}

  private:
    void operator=(const TFuncPropertyIdx<T,index>&);
    TFuncPropertyIdx();

  TFunctor1<T,int>   Get;
  TFunctorV2<int,T>  Put;
};

//
template <class T, int index>
class TFuncObjPropertyIdx{
  public:
    TFuncObjPropertyIdx(const TFunctor1<T&,int> get,
                     const TFunctorV2<int,const T&> put):
                     Get(get),Put(put){}
    ~TFuncObjPropertyIdx() {}

  public:
    // getters
    operator T*()              { return &Get(index);}
    operator const T&() const { return Get(index); }
    T* operator->()           { return &Get(index);}
    const T& operator*()      { return Get(index); }

    //set/get
    //T* operator=( T* t)       { Put(index,t); return &Get(index);}
    T& operator=( T& t)       { Put(index,t); return Get(index); }
    TFuncObjPropertyIdx<T,index>& operator=(const T& t){ Put(index,t); return *this;}

    //setters
    const T& operator()() const  { return Get(index);  }
    void operator()(const T& t) { Put(index,t);}

  private:
    void operator=(const TFuncObjPropertyIdx<T,index>&){}
    TFuncObjPropertyIdx(){}

    TFunctor1<T&,int>          Get;
    TFunctorV2<int,const T&>  Put;
};

// support macros
//
#define GETFUNCTOR(T,func) \
        Functor((TFunctor0<T>*)0, func)
#define PUTFUNCTOR(T,func) \
        Functor((TFunctorV1<T>*)0, func)
#define GETPUTFUNCTOR(T,funcGet,funcPut) \
        Functor((TFunctor0<T>*)0, funcGet),\
        Functor((TFunctorV1<T>*)0, funcPut)
//
#define GETOBJFUNCTOR(T,func) \
        Functor((TFunctor0<T&>*)0, func)
#define PUTOBJFUNCTOR(T,func) \
        Functor((TFunctorV1<const T&>*)0, func)
#define GETPUTOBJFUNCTOR(T,funcGet,funcPut) \
        Functor((TFunctor0<T&>*)0, funcGet),\
        Functor((TFunctorV1<const T&>*)0, funcPut)
//
#define GETMFUNCTOR(T,type,memberFunc) \
        Functor((TFunctor0<T>*)0, *this, &type::memberFunc)
#define PUTMFUNCTOR(T,type,memberFunc) \
        Functor((TFunctorV1<T>*)0, *this, &type::memberFunc)
#define GETPUTMFUNCTOR(T,type,memberGet,memberPut) \
        Functor((TFunctor0<T>*)0, *this, &type::memberGet),\
        Functor((TFunctorV1<T>*)0, *this, &type::memberPut)

#define GETOBJMFUNCTOR(T,type,memberFunc) \
        Functor((TFunctor0<T&>*)0, *this, &type::memberFunc)
#define PUTOBJMFUNCTOR(T,type,memberFunc) \
        Functor((TFunctorV1<const T&>*)0, *this, &type::memberFunc)
#define GETPUTOBJMFUNCTOR(T,type,memberGet,memberPut) \
        Functor((TFunctor0<T&>*)0, *this, &type::memberGet),\
        Functor((TFunctorV1<const T&>*)0, *this, &type::memberPut)
//
#define IDXGETFUNCTOR(T,func) \
        Functor((TFunctor1<T,int>*)0, func)
#define IDXPUTFUNCTOR(T,func) \
        Functor((TFunctorV2<int,T>*)0, func)
#define IDXGETPUTFUNCTOR(T,funcGet,funcPut) \
        Functor((TFunctor1<T,int>*)0, funcGet),\
        Functor((TFunctorV2<int,T>*)0, funcPut)
#define IDXGETPUTOBJFUNCTOR(T,funcGet,funcPut) \
        Functor((TFunctor1<T&,int>*)0, funcGet),\
        Functor((TFunctorV2<int,const T&>*)0, funcPut)
//
#define IDXGETMFUNCTOR(T,type,memberFunc) \
        Functor((TFunctor1<T,int>*)0, *this, &type::memberFunc)
#define IDXPUTMFUNCTOR(T,type,memberFunc) \
        Functor((TFunctorV2<int,T>*)0, *this, &type::memberFunc)
#define IDXGETPUTMFUNCTOR(T,type,memberGet,memberPut) \
        Functor((TFunctor1<T,int>*)0, *this, &type::memberGet),\
        Functor((TFunctorV2<int,T>*)0, *this, &type::memberPut)
#define IDXGETPUTOBJMFUNCTOR(T,type,memberGet,memberPut) \
        Functor((TFunctor1<T,int>*)0, *this, &type::memberGet),\
        Functor((TFunctorV2<int,const T&>*)0, *this, &type::memberPut)

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//
// inlines
//

// TObjProperty inlines
//
template <class T> inline
TObjProperty<T>::TObjProperty(const T& t)
{
  Content = new T(t);
}

template <class T> inline
TObjProperty<T>::TObjProperty(T* t)
:
  Content(t)
{
}

//
template <class T> inline
TObjProperty<T>::TObjProperty()
:
  Content(0)
{
}

//
template <class T> inline
TObjProperty<T>::~TObjProperty()
{
  delete Content;
}

//
template <class T> inline
TObjProperty<T>::operator T*()
{
  return Content;
}

//
template <class T> inline
TObjProperty<T>::operator const T&() const
{
  PRECONDITION(Content);
  return *Content;
}

//
template <class T> inline T*
TObjProperty<T>::operator->()
{
  return Content;
}

//
template <class T> inline const T&
TObjProperty<T>::operator*()
{
  PRECONDITION(Content);
  return *Content;
}

//
template <class T> inline T*
TObjProperty<T>::operator=( T* t)
{
  delete Content;
  Content = t;
  return Content;
}

//
template <class T> inline T&
TObjProperty<T>::operator=( T& t)
{
  delete Content;
  Content = new T(t);
  return *Content;
}

//
template <class T> inline const T&
TObjProperty<T>::operator()() const
{
  PRECONDITION(Content);
  return *Content;
}

//
template <class T> inline void
TObjProperty<T>::operator()(const T& t)
{
  delete Content;
  Content = new T(t);
}

//
template <class T> inline TObjProperty<T>&
TObjProperty<T>::operator=(const T& t)
{
  delete Content;
  Content = new T(t);
  return *this;
}

} // OWL namespace

//------------------------------------------------------------------------------
#endif
