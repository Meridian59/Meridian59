//----------------------------------------------------------------------------
// Borland WinSys Library
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TProfile class
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/profile.h>
#include <owl/private/memory.h>
#include <limits>
#include <owl/except.h>

namespace owl {

//
/// Constructs a TProfile object for the indicated section within the profile file
/// specified by filename.  If the file name is not provided, the file defaults to
/// the system profile file; for example, WIN.INI under Windows .
//
TProfile::TProfile(LPCTSTR section, LPCTSTR filename)
{
  Init(section, filename);
}

//
/// String-aware overload
//
TProfile::TProfile(const tstring& section, const tstring& filename)
{
  Init(
    section.empty() ? 0 : section.c_str(), 
    filename.empty() ? 0 : filename.c_str());
}

void TProfile::Init(LPCTSTR section, LPCTSTR filename)
{
  Section  = section ? strnewdup(section) : 0;
#if defined(UNICODE)
  USES_CONVERSION;
#endif
  // Use OpenFile to track down the given filename
  //   if can't find it, use copy of original name,
  //   if found, use copy of full path
  //
  if (filename) {
    OFSTRUCT ofs;
    ofs.cBytes = sizeof ofs;
#if defined(UNICODE)
    FileName = strnewdup(
      (OpenFile(W2A(filename), &ofs, OF_EXIST) == HFILE_ERROR) ? filename : A2W(ofs.szPathName)
    );
#else
    FileName = strnewdup(
    //JJH
    #if defined(__GNUC__)
      (OpenFile(filename, &ofs, OF_EXIST) == HFILE_ERROR) ? filename : (LPCTSTR)ofs.szPathName
    #else
      (OpenFile(filename, &ofs, OF_EXIST) == HFILE_ERROR) ? filename : ofs.szPathName
    #endif
    );
#endif
  }
  else {
    FileName = 0;
  }
}

//
/// Destroys  the TProfile object.
/// Cleans up buffers
//
TProfile::~TProfile()
{
  delete[] FileName;
  delete[] Section;
}

//
/// Looks up and returns the integer value associated with the given string, key. If
/// key is not found, the default value, defaultInt, is returned.
//
int
TProfile::GetInt(LPCTSTR key, int defaultInt)
{
  return FileName
    ? ::GetPrivateProfileInt(Section, key, defaultInt, FileName)
    : ::GetProfileInt(Section, key, defaultInt);
}

//
/// Looks up and returns the string value associated with the given key string. The
/// string value is copied into buff, up to buffSize bytes.  If the key is not
/// found, defaultString  provides the default value. If a 0 key is passed, all
/// section values are returned in buff.
//
bool
TProfile::GetString(LPCTSTR key, LPTSTR buff, unsigned buffSize,
                    LPCTSTR defaultString)
{
  return FileName
    ? ::GetPrivateProfileString(Section, key, defaultString?defaultString:_T(""), buff, buffSize, FileName)
    : ::GetProfileString(Section, key, defaultString?defaultString:_T(""), buff, buffSize);
}

tstring 
TProfile::GetString(const tstring& key, const tstring& defaultString)
{
  tstring buf;
  tstring::size_type buf_size = 256;
  const tstring::size_type limit = std::numeric_limits<tstring::size_type>::max();
  for (;;)
  {
    buf.resize(buf_size);
    DWORD n = FileName ? 
      ::GetPrivateProfileString(Section, key.c_str(), defaultString.c_str(), &buf[0], buf.size(), FileName) : 
      ::GetProfileString(Section, key.c_str(), defaultString.c_str(), &buf[0], buf.size());
    if (n < buf_size - 2) // Sure it's not truncated? (See doc for GetProfileString.)
    {
      buf.resize(n); // Shrink to contents.
      break;
    }
    if (buf_size > limit / 2)
      throw TXOwl(_T("TProfile::GetString: String is too large"));
    buf_size *= 2;
  }
  return buf;
}

//
/// Looks up the key and replaces its value with the integer value passed (int). If
/// the key is not found, WriteInt makes a new entry. Returns true if successful.
//
bool
TProfile::WriteInt(LPCTSTR key, int value)
{
  tchar buf[16];
#if defined __GNUC__
  _tprintf(buf, "%d", value);
#else
  _itot(value, buf, 10);
#endif
  return WriteString(key, buf);
}

//
/// Looks up the key and replaces its value with the string value passed (str). If
/// the key is not found, WriteString makes a new entry. Returns true if successful.
//
bool
TProfile::WriteString(LPCTSTR key, LPCTSTR str)
{
  return FileName
    ? ::WritePrivateProfileString(Section, key, str, FileName)
    : ::WriteProfileString(Section, key, str);
}

//
/// Makes sure that all written profile values are flushed to the actual file.
//
void
TProfile::Flush()
{
  if (FileName)
    ::WritePrivateProfileString(0, 0, 0, FileName);
}

} // OWL namespace
/* ========================================================================== */
