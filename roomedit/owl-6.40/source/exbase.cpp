//----------------------------------------------------------------------------
// Borland WinSys Library
// Copyright (c) 1994, 1996 by Borland International, All rights
//
/// \file
/// TXBase class implementation.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/private/checks.h>
#include <owl/exbase.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

//------------------------------------------------------------------------------

// Local namespace for UTF conversion

namespace {

  /// Uniform adapter to Windows API for UTF conversion
  /// Returns the number of characters written to the buffer, including any null-terminator.

  int utf16_to_utf8(DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte)
  {return WideCharToMultiByte(CP_UTF8, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, NULL, NULL);}

  /// Uniform adapter to Windows API for UTF conversion
  /// Returns the number of characters written to the buffer, including any null-terminator.

  int utf8_to_utf16(DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
  {return MultiByteToWideChar(CP_UTF8, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);}


  /// Converter function
  /// Passed converter can be utf16_to_utf8 or utf8_to_utf16.

  template <class Result, class Source, class Converter>
  Result convert(const Source& source, Converter* const converter, const DWORD flags = 0)
  {
    Result result; 
    if (!source.empty())
    {
      // Get the size, in characters, of the buffer needed for the translated string, 
      // including the null-terminator.

      int n = (*converter)(flags, &source[0], -1, NULL, 0);
      CHECK(n >= 0);
      WARN(n == 0, _T("String conversion sizing failed, GetLastError() == ") << GetLastError());

      // Translate the string if the size query was successful.

      if (n != 0)
      {
        result.resize(n); // Reserve room, including explicit null-terminator.
        int r = (*converter)(flags, &source[0], -1, &result[0], n);
        CHECK(r >= 0);
        WARN(r == 0, _T("String conversion failed, GetLastError() == ") << GetLastError());
        result.erase(r > 0 ? r - 1 : 0); // Remove the explicit null-terminator, or all if conversion failed.
      }
    }
    return result;
  }


  // Typedefs for string types. 
  // If support is needed for non-standard string types then add alternatives here using 
  // preprocessor directives for conditional compilation.

  typedef std::string narrow_string;
  typedef std::wstring wide_string;


  /// Uniform interface function

  inline narrow_string convert_to_narrow_string(const wide_string& s_utf16)
  {
    return convert<narrow_string>(s_utf16, utf16_to_utf8);
  }


  /// Uniform interface function overload - NOP

  inline const narrow_string& convert_to_narrow_string(const narrow_string& s)
  {
    return s;
  }


  /// Uniform interface functor
  /// Assumes the source is encoded in UTF-8 format.

  template <class Result>
  struct ConverterFromNarrowString
  {
    inline Result operator () (const narrow_string& s_utf8)
    {return convert<Result>(s_utf8, utf8_to_utf16, MB_ERR_INVALID_CHARS);}
  };


  /// Uniform interface functor specialization - NOP

  template <>
  struct ConverterFromNarrowString<narrow_string>
  {
    inline const narrow_string& operator () (const narrow_string& s)
    {return s;}
  };


} // namespace

//------------------------------------------------------------------------------

//
/// Calls the xmsg class's constructor that takes a string parameter and initializes
/// xmsg with the value of the string parameter.
//
TXBase::TXBase(const tstring& msg)
  : std::exception(), str(convert_to_narrow_string(msg))
{}

//
/// Creates a copy of the TXBase object passed in the TXBase parameter.
//
TXBase::TXBase(const TXBase& src)
:  std::exception(), str(src.str)
{}

TXBase::~TXBase() throw()
{}

/// Makes a copy of the exception object.
TXBase* TXBase::Clone() const
{
  return new TXBase(*this);
}

/// Throws the exception object.
void TXBase::Throw()
{
  throw *this;
}

//
/// Constructs a TXBase exception from scratch, and throws it.
//
void TXBase::Raise(const tstring& msg)
{
  TXBase(msg).Throw();
}

const char* TXBase::what() const throw() 
{
  return str.c_str ();
}

TXBase& TXBase::operator=(const TXBase &src)
{
  if (this != &src)
  {
    str = src.str;
  }
  return(*this);
}

tstring TXBase::why() const
{
  return ConverterFromNarrowString<tstring>()(str);
}

//------------------------------------------------------------------------------

} // OWL namespace

