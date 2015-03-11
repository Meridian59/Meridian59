//----------------------------------------------------------------------------
// Borland WinSys Library
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of TProfile class
//----------------------------------------------------------------------------

#if !defined(OWL_PROFILE_H)
#define OWL_PROFILE_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>


namespace owl {
//FMM had to include LPCTSTR on HPUX for some reason
#ifndef LPCTSTR
#include <windows.h>
#endif

/// \addtogroup winsys
/// @{
/// \class TProfile
// ~~~~~ ~~~~~~~~
/// An instance of TProfile encapsulates a setting within a system file, often
/// referred to as a profile or initialization file. Examples of this type of file
/// include the Windows initialization files SYSTEM.INI and WIN.INI. Within the
/// system file itself, the individual settings are grouped within sections. For
/// example,
/// \code 
/// [Diagnostics]	; section name
/// Enabled=0    	; setting
/// \endcode
/// For a setting, the value to the left of the equal sign is called the key. The
/// value to the right of the equal sign, the value, can be either an integer or a
/// string data type.
//
class _OWLCLASS TProfile  
{
  public:
    // Use system global profile for filename==0
    //
    TProfile(LPCTSTR section, LPCTSTR filename = 0);
    TProfile(const tstring& section, const tstring& filename = tstring());
   ~TProfile();

    int GetInt(LPCTSTR key, int defaultInt = 0);

    int GetInt(const tstring& key, int defaultInt = 0)
    {return GetInt(key.c_str(), defaultInt);}

    // Returns all section values if key==0
    //
    bool GetString(LPCTSTR key, LPTSTR buff, unsigned buffSize, LPCTSTR defaultString = 0);

    bool GetString(const tstring& key, LPTSTR buff, unsigned buffSize, const tstring& defaultString = tstring())
    {return GetString(key.c_str(), buff, buffSize, defaultString.empty() ? 0 : defaultString.c_str());}

    tstring GetString(const tstring& key, const tstring& defaultString = tstring());

    bool WriteInt(LPCTSTR key, int value);

    bool WriteInt(const tstring& key, int value)
    {return WriteInt(key.c_str(), value);}

    bool WriteString(LPCTSTR key, LPCTSTR str);

    bool WriteString(const tstring& key, LPCTSTR str)
    {return WriteString(key.c_str(), str);}

    bool WriteString(const tstring& key, const tstring& str)
    {return WriteString(key.c_str(), str.c_str());}

    void Flush();

  protected:
    LPTSTR Section;     ///< Name of the section to use
    LPTSTR FileName;    ///< File name of the .INI file

    void Init(LPCTSTR section, LPCTSTR filename);
};
/// @}

} // OWL namespace


#endif  // OWL_PROFILE_H
