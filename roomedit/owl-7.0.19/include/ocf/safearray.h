#if !defined(_SAFEARRAY_H)
#define _SAFEARRAY_H

#if !defined(_OLE2_H_)
# include <ocf/autodefs.h>
#endif

//
// Class VarType
// ~~~~~ ~~~~~~~
// VarType provides OLE interpretations to C++ basic types.
//
template<class T>
class VarType {
  public:
    virtual VARTYPE OLEType() = 0;
};

template<>
class VarType<bool> {
  public:
    VARTYPE OLEType()
    {
      return VT_BOOL;
    }
};

template<>
class VarType<char> {
  public:
    VARTYPE OLEType()
    {
      return VT_I1;
    }
};

template<>
class VarType<double> {
  public:
    VARTYPE OLEType()
    {
      return VT_R8;
    }
};

template<>
class VarType<float> {
  public:
    VARTYPE OLEType()
    {
      return VT_R4;
    }
};

template<>
class VarType<int> {
  public:
    VARTYPE OLEType()
    {
      return VT_INT;
    }
};

template<>
class VarType<long> {
  public:
    VARTYPE OLEType()
    {
      return VT_I4;
    }
};

template<>
class VarType<short> {
  public:
    VARTYPE OLEType()
    {
      return VT_I2;
    }
};

template<>
class VarType<unsigned char> {
  public:
    VARTYPE OLEType()
    {
      return VT_UI1;
    }
};

template<>
class VarType<unsigned int> {
  public:
    VARTYPE OLEType()
    {
      return VT_UINT;
    }
};

template<>
class VarType<unsigned long> {
  public:
    VARTYPE OLEType()
    {
      return VT_UI4;
    }
};

template<>
class VarType<unsigned short> {
  public:
    VARTYPE OLEType()
    {
      return VT_UI2;
    }
};

//
// Class TSafeArray
// ~~~~~ ~~~~~~~~~~
// TSafeArray encapsulates the properties of OLE's SAFEARRAY and
// provides easy access to individual members of it.
//
template<class T>
class TSafeArray : public TAutoVal {
  public:
    TSafeArray(int n);
   ~TSafeArray();

    // Indexing operator
    //
    T& operator[](int index);
};

// TSafeArray inline functions
//
template<class T>
inline
TSafeArray<T>::TSafeArray(int n)
:
 TAutoVal()
{
  SAFEARRAYBOUND bound;
  bound.lLbound   = 0;
  bound.cElements = n;

  VARTYPE varType = VarType<T>().OLEType();

  ((VARIANT*)this)->vt     = (VARTYPE)(varType | VT_ARRAY);
  ((VARIANT*)this)->parray = ::SafeArrayCreate(varType, 1, &bound);
}

template<class T>
inline
TSafeArray<T>::~TSafeArray()
{
  // TAutoVal::~Clear() takes care of freeing the array!
}

template<class T>
T&
TSafeArray<T>::operator[](int index)
{
  T* tmp = (T*)(GetArray()->pvData);
  return tmp[index];
}

#endif  // _SAFEARRAY_H
