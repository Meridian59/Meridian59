//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Classes for window system structure and type encapsulation
//----------------------------------------------------------------------------

#if !defined(OWL_WSYSCLS_H)
#define OWL_WSYSCLS_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/geometry.h>     // TPoint, et. al.
#include <owl/pointer.h>

#if !defined(BI_NOTEMPLATE_H)
#include <owl/template.h>
#endif

#include <shellapi.h>
#include <vector>
#include <utility>

namespace owl {class _OWLCLASS ipstream;};
namespace owl {class _OWLCLASS opstream;};


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TResIdT
/// TResId encapsulates a Windows resource identifier. 
///
/// A Windows resource identifier is a pointer which either stores a pointer to
/// a string that identifies the resource, or a numerical identifier encoded as 
/// a WORD in the lower bits. If the latter, then the higher bits are 0.
///
/// This class does not take ownership of the actual string that the resource 
/// identifier may point to.
//
template <class T>
class _OWLCLASS TResIdT 
{
public:
    typedef const T* TPointer;

    //
    /// Sets the identifier to 0.
    //
    TResIdT() : ResId(0) {}
      
    //
    /// Encapsulates the given Windows resource identifier.
    /// Note: The given pointer may actually encode a WORD in the lower bits.
    //
    TResIdT(TPointer id) : ResId(id) {}

    //
    /// Encapsulates the given numerical Windows resource identifier.
    /// Note that a integer resource identifier is defined as a WORD, and
    /// thus only supports the identifier range 0 to 65535. 
    //
    TResIdT(int id) : ResId(reinterpret_cast<TPointer>(static_cast<WORD>(id))) {
      PRECONDITION(id >= 0 && id <= 65535);
    }

    //
    /// Returns the encapsulated pointer.
    //
    TPointer GetPointerRepresentation() const {return ResId;}

    //
    /// Returns true if this resource identifier encodes an integer value.
    //
    bool IsInt() const {return IS_INTRESOURCE(ResId);}

    //
    /// Returns true if this resource identifier encodes a string pointer.
    //
    bool IsString() const {return !IsInt();}

    //
    /// Returns the encapsulated numerical identifier, provided this resource
    /// identifier encodes a numerical identifier.
    //
    WORD GetInt() const {
      PRECONDITION(IsInt());
      return static_cast<WORD>(reinterpret_cast<ULONG_PTR>(ResId) & 0x0FFFF);
    }

    //
    /// Returns the encapsulated string pointer, provided this resource
    /// identifier encodes a string pointer.
    //
    TPointer GetString() const {
      PRECONDITION(IsString());
      return ResId;
    }

  private:
    TPointer ResId;

  //
  /// Extracts a TResIdT object from is (the given input stream), and copies it to id.
  /// Returns a reference to the resulting stream, allowing the usual chaining of >>
  /// operations.
  ///
  /// NOTE: The caller is responsible for deallocating the memory if the returned TResId
  /// represents a resource string, i.e. when id.IsString() == true.
  //
  friend _OWLCFUNC(ipstream&) operator >>(ipstream& is, TResIdT& id)
  {
    bool isNumeric;
    is >> isNumeric;

    if (isNumeric) 
    {
      long nid;
      is >> nid;
      id = TResIdT<T>(nid);
    }
    else 
    {
      T* s = 0;
      TResIdT<T>::ReadString(is, s);
      id = TResIdT<T>(s);
    }
    return is;
  }

  //
  /// Inserts the given TResIdT object (id) into the opstream (os). Returns a reference
  /// to the resulting stream, allowing the usual chaining of << operations.
  //
  friend _OWLCFUNC(opstream&) operator <<(opstream& os, const TResIdT& id)
  {
    bool isNumeric = id.IsInt();
    os << isNumeric;

    if (isNumeric) 
    {
      long nid = id.GetInt(); 
      os << nid;
    }
    else 
    {
      TResIdT<T>::WriteString(os, id.GetString());
    }
    return os;
  }

  //
  /// Formats and inserts the given TResId object (id) into the ostream (os). Returns
  /// a reference to the resulting stream, allowing the usual chaining of <<
  /// operations.
  //
  friend _OWLCFUNC(std::ostream&)  operator <<(std::ostream& os, const TResIdT& id)
  {
    if (id.IsInt())
      os << id.GetInt();
    else
      os << id.GetString();
    return os;
  }

  //
  // Helper functions for persistent streams
  // Persistent streams only support narrow-character strings, so the persistent stream operators 
  // for TResIdT use overload resolution on these helpers to select the correct function, 
  // performing string conversion as necessary.
  //

  static void ReadString(ipstream& is, char*& s)
  {s = is.freadString();}

  static void ReadString(ipstream& is, wchar_t*& s)
  {
    TAPointer<char> src(is.freadString());
    int size = ::MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);
    s = new wchar_t[size];
    int r = ::MultiByteToWideChar(CP_ACP, 0, src, -1, s, size);
    WARN(r == 0, _T("String conversion failed, GetLastError() == ") << GetLastError());
  }

  static void WriteString(opstream& os, const char* s)
  {os.fwriteString(s);}

  static void WriteString(opstream& os, const wchar_t* s)
  {
    int size = ::WideCharToMultiByte(CP_ACP, 0, s, -1, NULL, 0, NULL, NULL);
    std::vector<char> dst(size);
    int r = ::WideCharToMultiByte(CP_ACP, 0, s, -1, &dst[0], size, 0, 0);
    WARN(r == 0, _T("String conversion failed, GetLastError() == ") << GetLastError());
    os.fwriteString(&dst[0]);
  }

};

class _OWLCLASS TResId : public TResIdT<tchar>
{
public:
    TResId() : TResIdT<tchar>() {}
    TResId(TPointer id) : TResIdT<tchar>(id) {}
    TResId(int id) : TResIdT<tchar>(id) {}

#if !defined(OWL_STRICT_DATA)

#if defined(OWL5_COMPAT)
    typedef LPTSTR TImplicitConversionResult;
#else 
    typedef LPCTSTR TImplicitConversionResult;
#endif

    //
    /// Typecasting operator that converts this to the pointer representation of
    /// the encapsulated Windows resource identifier.
    ///
    /// This operator is deprecated. Use GetPointerRepresentation instead.
    //
    operator TImplicitConversionResult() const 
    {return const_cast<TImplicitConversionResult>(GetPointerRepresentation());}

#endif

};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

typedef TResIdT<char> TNarrowResId;

} // OWL namespace
//
// MSW only classes
//

# if !defined(OWL_WSYSINC_H)
#   include <owl/wsysinc.h>
# endif

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TDropInfo
// ~~~~~ ~~~~~~~~~
/// TDropInfo is a simple class that supports file-name drag-and-drop operations
/// using the WM_DROPFILES message. A TDropInfo object encapsulates a HDROP handle 
/// returned by the WM_DROPFILES message.
//
class _OWLCLASS TDropInfo {
  public:
    //
    /// Creates a TDropInfo object encapsulating the given handle.
    //
    TDropInfo(HDROP handle) : Handle(handle) {}

    void DragFinish() const;

    // Type Conversion operators
    //
    operator HDROP() {return Handle;}
    operator HDROP() const {return Handle;}

    // Information access
    //
    tstring DragQueryFile(uint index) const;
    uint DragQueryFile(uint index, LPTSTR name, uint nameLen) const;
    uint DragQueryFileCount() const;
    uint DragQueryFileNameLen(uint index) const;
    std::pair<TPoint, bool> DragQueryPoint() const;
    bool DragQueryPoint(TPoint& point) const;
    
  private:
    HDROP  Handle;
};

//
/// \class TFileDroplet
// ~~~~~ ~~~~~~~~~
/// TFileDroplet encapsulates information about a single dropped file, its name,
/// where it was dropped, and whether or not it was in the client area.
//
class _OWLCLASS TFileDroplet 
{
  public:
    TFileDroplet(const tstring& fileName, const TPoint& p, bool inClient);
    TFileDroplet(TDropInfo& drop, int i);
   ~TFileDroplet();

/// Returns true if the address of this object is equal to the address of the
/// compared object.
    bool operator ==(const TFileDroplet& other) const {return &other == this;}

/// Returns the name of the file dropped.
    LPCTSTR    GetName() const {return FileName;}
    
/// Returns the cursor position at which the file was dropped.
    TPoint    GetPoint() const {return Point;}
    
/// Returns true if the drop occurred in the client area.
    bool      GetInClientArea() const {return InClientArea;}

  private:
    LPTSTR  FileName;
    TPoint  Point;
    bool    InClientArea;

    // Hidden to prevent accidental copying or assignment
    //
    TFileDroplet(const TFileDroplet&);
    TFileDroplet& operator =(const TFileDroplet&);
};

template <class T>            class TIPtrArray;
template <class T, class T1>  class TPtrArrayIterator;

typedef TIPtrArray<TFileDroplet*>   TFileDropletList;
typedef TPtrArrayIterator<TFileDroplet*, TFileDropletList > TFileDropletListIter;

//
/// \class TProcInstance
// ~~~~~ ~~~~~~~~~~~~~
/// A ProcInstance object. This encapsulates the MakeProcInstance call, which is
/// really only needed in old Win3.X real mode. This exists now for Owl 2.x
/// compatibility only
//
class TProcInstance {
  public:
    TProcInstance(FARPROC p) {Instance = FARPROC(p);}

    operator FARPROC() {return Instance;}

  private:
    FARPROC Instance;
};



//
// FILETIME and SYSTEMTIME  wrappers
//
class _OWLCLASS TSystemTime;
class _OWLCLASS TTime;

/// TFileTime is a class derived from the structure FILETIME.
/// 
/// The FILETIME struct is defined as
/// \code
/// struct tagFILETIME { 
/// uint32 dwLowDateTime;
/// uint32 dwHighDateTime;
/// } FILETIME;
/// \endcode
class _OWLCLASS TFileTime: public FILETIME 
{
  public:
    /// Constructs a empty TFileTime object.
    TFileTime() { dwLowDateTime=0; dwHighDateTime=0;}
    
    TFileTime(const TSystemTime& tm);
    
    /// Constructs a TFileTime object from given TFileTime.
    TFileTime(const TFileTime& tm){memcpy(this, &tm, sizeof(tm));}
    
    /// Constructs a TFileTime object from given FILETIME.
    TFileTime(const FILETIME& tm){memcpy(this, &tm, sizeof(tm));}
    
    TFileTime(const TTime& tm);
    
    TFileTime& operator=(const TTime& tm);

    bool ToLocalTime();
    bool ToLocalTime(TFileTime& tm) const 
          { return ::FileTimeToLocalFileTime((FILETIME*)this, &tm); }

    bool ToUniversalTime();
    bool ToUniversalTime(TFileTime& tm) const 
          { return ::LocalFileTimeToFileTime(this, &tm); }

    // Boolean operators.
    bool operator <  ( const TFileTime & t ) const;
    bool operator <= ( const TFileTime & t ) const;
    bool operator >  ( const TFileTime & t ) const;
    bool operator >= ( const TFileTime & t ) const;
    bool operator == ( const TFileTime & t ) const;
    bool operator != ( const TFileTime & t ) const;

    // Read or write times on persistent streams
    friend _OWLCFUNC(opstream &) operator << ( opstream  & s, const TFileTime  & d );
    friend _OWLCFUNC(ipstream &) operator >> ( ipstream  & s, TFileTime  & d );
};


//
/// \class TSystemTime
// ~~~~~ ~~~~~~~~~~~
/// TSystemTime is a class derived from the structure SYSTEMTIME.
/// 
/// The SYSTEMTIME struct is defined as
/// \code
/// typedef struct _SYSTEMTIME  { 
///     WORD wYear;
///     WORD wMonth;
///     WORD wDayOfWeek;
///     WORD wDay;
///     WORD wHour;
///     WORD wMinute;
///     WORD wSecond;
///     WORD wMilliseconds;
/// } SYSTEMTIME;
/// \endcode
//
class _OWLCLASS TSystemTime: public SYSTEMTIME {
  public:
    TSystemTime() { memset(this,0,sizeof(SYSTEMTIME)); }
    TSystemTime(uint y, uint m, uint d, uint h=0, uint mn=0, uint s=0, uint dw=0,uint ms=0);
    TSystemTime(const TSystemTime& tm) { memcpy(this, &tm, sizeof(tm)); }
    TSystemTime(const TFileTime& tm)   { ::FileTimeToSystemTime(&tm, this); }
    TSystemTime(const SYSTEMTIME& tm)  { memcpy(this, &tm, sizeof(tm)); }

    // 
    /// Parses the given date and time string using the optional locale.
    /// The user default locale is used if no locale is specified.
    /// Throws an TXOwl exception if parsing fails.
    //
    TSystemTime(const tstring&, LCID = 0);

    uint16   GetYear()         { return wYear;         }
    uint16   GetYear() const    { return wYear;         }
    void     SetYear(uint y)   { wYear = (uint16)y;    }

    uint16   GetMonth()         { return wMonth;        }
    uint16   GetMonth() const  { return wMonth;        }
    void     SetMonth(uint m)  { wMonth = (uint16)m;    }

    uint16   GetDayOfWeek()     { return wDayOfWeek;    }
    uint16   GetDayOfWeek() const { return wDayOfWeek;    }
    void     SetDayOfWeek(uint dw)  { wDayOfWeek = (int16)dw;  }

    uint16   GetDay()           { return wDay;          }
    uint16   GetDay() const    { return wDay;          }
    void     SetDay(uint d)     { wDay = (uint16)d;      }

    uint16  GetHour()         { return wHour;          }
    uint16  GetHour() const    { return wHour;          }
    void    SetHour(uint h)   { wHour = (uint16)h;    }

    uint16   GetMinute()       { return wMinute;        }
    uint16   GetMinute() const  { return wMinute;        }
    void    SetMinute(uint m)  { wMinute = (uint16)m;  }

    uint16   GetSecond()       { return wSecond;        }
    uint16   GetSecond() const  { return wSecond;        }
    void    SetSecond(uint s) { wSecond = (uint16)s;  }

    uint16   GetMilliseconds()  { return wMilliseconds;  }
    uint16   GetMilliseconds()  const { return wMilliseconds;  }
    void    SetMilliseconds(uint ms){ wMilliseconds = (uint16)ms;  }

    TFileTime GetFileTime() { return TFileTime(*this);}
    static TSystemTime LocalTime()  { TSystemTime tm; ::GetLocalTime(&tm); return tm;  }
    static TSystemTime SystemTime() { TSystemTime tm; ::GetSystemTime(&tm); return tm; }

    TSystemTime& operator=(const TFileTime& ft)   { *this = TSystemTime(ft); return *this; }

    // Boolean operators.
    bool operator <  ( const TSystemTime  & t ) const;
    bool operator <= ( const TSystemTime  & t ) const;
    bool operator >  ( const TSystemTime  & t ) const;
    bool operator >= ( const TSystemTime  & t ) const;
    bool operator == ( const TSystemTime  & t ) const;
    bool operator != ( const TSystemTime  & t ) const;

};

/// \class TResource
//
/// TResource simplifies access to a resource by encapsulating
/// the find, load, lock and free steps for accessing a resource.
///   - 'T' represents a structure which defines the binary layout of the resource.
///   - 'resType' is a constant string that defines the resource type.
//
/// For example,
/// \code
///     typedef TResource<DLGTEMPLATE, (int)RT_DIALOG> TDlgResource;
///     TDlgResource dlgInfo(hInstance, IDD_ABOUTDLG);
///     DLGTEMPLATE* pDlgTmpl = dlgInfo;
/// \endcode
//
//template <class T, const tchar * resType>
template <class T, int resType> class /*_OWLCLASS*/ TResource {
  public:
    TResource(HINSTANCE hModule, TResId resId);
    TResource(HINSTANCE hModule, TResId resid, LANGID langid);
   ~TResource();
    bool      IsOK() const;     ///< Confirms whether resource was found
    operator  T*();             ///< Conversion operator to point to structure representing binary layout of the resource...

  protected:
    HGLOBAL   MemHandle;        ///< Handle of resource
    T*        MemPtr;           ///< Pointer to locked resource
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//
// inlines
//
inline bool TFileTime::operator <  ( const TFileTime  & t ) const{
//JJH
#ifdef __GNUC__
  return ::CompareFileTime((FILETIME*) this, (FILETIME*) &t) < 0;
#else
  return ::CompareFileTime(this, &t) < 0;
#endif
}

//
inline bool TFileTime::operator <= ( const TFileTime  & t ) const{
  return !(*this > t);
}

//
inline bool TFileTime::operator >  ( const TFileTime  & t ) const{
//JJH
#ifdef __GNUC__
  return ::CompareFileTime((FILETIME*) this, (FILETIME*) &t) > 0;
#else
  return ::CompareFileTime((FILETIME*) this, (FILETIME*) &t) > 0;
#endif
}

//
inline bool TFileTime::operator >= ( const TFileTime  & t ) const{
  return !(*this < t);
}

//
inline bool TFileTime::operator != ( const TFileTime  & t ) const{
  return !(*this == t);
}

//
inline bool TFileTime::operator == ( const TFileTime  & t ) const{
//JJH
#ifdef __GNUC__
  return ::CompareFileTime((FILETIME*)this, (FILETIME*)&t) == 0;
#else
  return ::CompareFileTime(this, &t) == 0;
#endif
}

//
/// Constructs a TFileTime object from system time.
inline TFileTime::TFileTime(const TSystemTime& tm) {
  ::SystemTimeToFileTime(&tm, this);
}

//
inline TSystemTime::TSystemTime(uint y, uint m, uint d, uint h, uint mn,
                                uint s, uint dw, uint ms)
{
  wYear=(uint16)y;
  wMonth=(uint16)m;
  wDay=(uint16)d;
  wHour=(uint16)h;
  wMinute=(uint16)mn;
  wSecond= (uint16)s;
  wDayOfWeek=(uint16)dw;
  wMilliseconds=(uint16)ms;
}

//
inline bool TSystemTime::operator < ( const TSystemTime  & t ) const{
  return TFileTime(*this) < TFileTime(t);
}

//
inline bool TSystemTime::operator <= ( const TSystemTime  & t ) const{
  return !(*this > t);
}

//
inline bool TSystemTime::operator >  ( const TSystemTime  & t ) const{
  return TFileTime(*this) > TFileTime(t);
}

//
inline bool TSystemTime::operator >= ( const TSystemTime  & t ) const{
  return !(*this < t);
}

//
inline bool TSystemTime::operator == ( const TSystemTime  & t ) const{
  return TFileTime(*this) == TFileTime(t);
}

//
inline bool TSystemTime::operator != ( const TSystemTime  & t ) const{
  return !(*this == t);
}

//
/// Loads & locks the specified resource..
//
//template <class T, const tchar * resType>
template <class T, int resType>
TResource<T, resType>::TResource(HINSTANCE hModule, TResId resId)
:
  MemHandle(0),
  MemPtr(0)
{
  HRSRC resHandle = FindResource(hModule, resId, (tchar *)resType);
  if (resHandle) {
    MemHandle = LoadResource(hModule, resHandle);
    if (MemHandle)
      MemPtr = (T*)LockResource(MemHandle);
  }
}

//
/// Loads & locks a resource of the type 'resType' from the module
/// (hModule). Accepts a LanguageID for localized resources.
//
//template <class T, const tchar * resType>
template <class T, int resType>
TResource<T, resType>::TResource(HINSTANCE hModule, TResId resId, LANGID lcid)
:
  MemHandle(0),
  MemPtr(0)
{
  HRSRC resHandle = FindResourceEx(hModule, (tchar *)resType, resId, lcid);
  if (resHandle) {
    MemHandle = LoadResource(hModule, resHandle);
    if (MemHandle)
      MemPtr = (T*)LockResource(MemHandle);
  }
}

//
/// Unlocks and frees the resource loaded earlier.
/// \note Unlocking and freeing of resources is not necessary in WIN32.
//
//template <class T, const tchar * resType>
template <class T, int resType>
TResource<T, resType>::~TResource()
{
  // NOTE: UnlockResource and FreeResource are not necessary (albeit harmless)
  // in 32-bit
  //
  MemHandle = 0;
  MemPtr = 0;
}

//
/// Returns true if resource was successfully loaded or false otherwise.
//
//template <class T, const tchar * resType> bool
template <class T, int resType> bool
TResource<T, resType>::IsOK() const
{
  return ((MemHandle != 0) && (MemPtr != 0));
}

//
/// Returns a pointer to the locked resource.
/// \note This operator eliminates the need to explicitly cast
///       LPVOID to a structure representing the actual layout
///       of individual resource types.
//
//template <class T, const tchar * resType>
template <class T, int resType>
TResource<T, resType>::operator T* ()
{
  PRECONDITION(IsOK());
  return MemPtr;
}

} // OWL namespace



#endif  // OWL_WINCLASS_H
