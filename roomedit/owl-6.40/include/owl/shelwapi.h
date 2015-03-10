//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definitions of Windows light-weight utility APIs:
///   TSHReg, TSHPath
//
//----------------------------------------------------------------------------

#if !defined(OWL_SHELWAPI_H)
#define OWL_SHELWAPI_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#if !defined(_INC_SHLWAPI)
# include <shlwapi.h>
#endif



namespace owl {

class _OWLCLASS TModule;

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TSHReg
// ~~~~~ ~~~~~~
/// delay loading SHLWAPI.DLL
class _OWLCLASS  TSHReg {
  public:

/// DeleteEmptyKey mimics RegDeleteKey as it behaves on NT -
///   delete the subkey only if it does not contain any subkeys and values.
    static   DWORD      DeleteEmptyKey(HKEY hkey, LPCTSTR pszSubKey);

/// DeleteKey mimics RegDeleteKey as it behaves on Win95 -
///  deletes a subkey and all its values and descendants
    static   DWORD      DeleteKey(HKEY hkey, LPCTSTR pszSubKey);

/// Duplicates a registry key's HKEY handle    
    static   HKEY      DuplicateKey(HKEY hkey);

/// These functions open the key, get/set/delete the value, then close
/// the key.
    static   DWORD      DeleteValue(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue);
    static   DWORD      GetValue(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPDWORD pdwType, LPVOID pvData, LPDWORD pcbData);
    static   DWORD      SetValue(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, DWORD dwType, LPCVOID pvData, DWORD cbData);

/// This functions work just like RegQueryValueEx, except if the
/// data type is REG_EXPAND_SZ, then these will go ahead and expand
/// out the string.  *pdwType will always be massaged to REG_SZ
/// if this happens.  REG_SZ values are also guaranteed to be null
/// terminated.
    static   DWORD      QueryValueEx(HKEY hkey, LPCTSTR pszValue, LPDWORD pdwReserved, LPDWORD pdwType, LPVOID pvData, LPDWORD pcbData);


/// Enumerates the subkeys of the specified open registry key
    static   LONG       EnumKeyEx(HKEY hkey, DWORD dwIndex, LPTSTR pszName, LPDWORD pcchName);

/// Enumerates the values of the specified open registry key
    static   LONG       EnumValue(HKEY hkey, DWORD dwIndex, LPTSTR pszValueName, LPDWORD pcchValueName, LPDWORD pdwType, LPVOID pvData, LPDWORD pcbData);

/// Retrieves information about a specified registry key
    static   LONG       QueryInfoKey(HKEY hkey, LPDWORD pcSubKeys, LPDWORD pcchMaxSubKeyLen, LPDWORD pcValues, LPDWORD pcchMaxValueNameLen);

/// Recursively copies the subkeys and values of the source subkey to the destination key
    static   DWORD      CopyKey(HKEY hkeySrc, LPCTSTR szSrcSubKey, HKEY hkeyDest, DWORD fReserved);

/// Retrieves a file path from the registry, expanding environment variables as needed
    static   DWORD      RegGetPath(HKEY hkey, LPCTSTR pcszSubKey, LPCTSTR pcszValue, LPTSTR pszPath, DWORD dwFlags);

/// Takes a file path, replaces folder names with environment strings, and places the resulting string in the registry
    static   DWORD      RegSetPath(HKEY hkey, LPCTSTR pcszSubKey, LPCTSTR pcszValue, LPCTSTR pcszPath, DWORD dwFlags);

    static  TModule&   GetModule();
};


////////////////////////////////////////////////////////////////////////////////


//
/// \class TSHPath
// ~~~~~ ~~~~~~
/// delay loading SHLWAPI.DLL
class _OWLCLASS  TSHPath {
  public:

/// Adds a backslash to the end of a string to create the correct syntax for a path.
/// If the source path already has a trailing backslash, no backslash will be added
    static LPTSTR      AddBackslash(LPTSTR pszPath);

/// Adds a file extension to a path string
    static BOOL        AddExtension(LPTSTR pszPath, LPCTSTR pszExt);

/// Appends one path to the end of another
    static BOOL       Append(LPTSTR pszPath, LPCTSTR pMore);

/// Creates a root path from a given drive number    
    static LPTSTR      BuildRoot(LPTSTR pszRoot, int iDrive);

/// Canonicalizes a path
    static BOOL       Canonicalize(LPTSTR pszBuf, LPCTSTR pszPath);

/// Concatenates two strings that represent properly formed paths into one path,
///  as well as any relative path pieces
    static LPTSTR      Combine(LPTSTR pszDest, LPCTSTR pszDir, LPCTSTR pszFile);

/// Truncates a file path to fit within a given pixel width
///  by replacing path components with ellipses    
    static BOOL       CompactPath(HDC hDC, LPTSTR pszPath, UINT dx);
    static BOOL       CompactPathEx(LPTSTR pszOut, LPCTSTR pszSrc, UINT cchMax, DWORD dwFlags);

/// Compares two paths to determine if they share a common prefix.
/// A prefix is one of these types: "C:\\", ".", "..", "..\\".     
    static int        CommonPrefix(LPCTSTR pszFile1, LPCTSTR pszFile2, LPTSTR achPath);

/// Determines whether a path to a file system object
///  such as a file or directory is valid
    static BOOL       FileExists(LPCTSTR pszPath);

/// Searches a path for an extension    
    static LPTSTR      FindExtension(LPCTSTR pszPath);

/// Searches a path for a file name
    static LPTSTR      FindFileName(LPCTSTR pszPath);

/// Parses a path for the next path component    
    static LPTSTR      FindNextComponent(LPCTSTR pszPath);

/// Searches for a file    
    static BOOL       FindOnPath(LPTSTR pszPath, LPCTSTR * ppszOtherDirs);

/// Finds the command line arguments within a given path    
    static LPTSTR      GetArgs(LPCTSTR pszPath);

/// Determines if a given file name has one of a list of suffixes
    static LPCTSTR     FindSuffixArray(LPCTSTR pszPath, const LPCTSTR *apszSuffix, int iArraySize);

/// Determines whether or not a file name is in long format    
    static BOOL       IsLFNFileSpec(LPCTSTR lpName);

/// Determines the type of character with respect to a path
    static UINT       GetCharType(tchar ch);

/// Searches a path for a drive letter within the range of 'A' to 'Z'
///  and returns the corresponding drive number    
    static int        GetDriveNumber(LPCTSTR pszPath);

/// Verifies that a path is a valid directory    
    static BOOL       IsDirectory(LPCTSTR pszPath);

/// Determines whether or not a specified path is an empty directory    
    static BOOL       IsDirectoryEmpty(LPCTSTR pszPath);

/// Searches a path for any path delimiting characters (for example, ':' or '\' ).
/// If there are no path delimiting characters present,
///  the path is considered to be a File Spec path
    static BOOL       IsFileSpec(LPCTSTR pszPath);

/// Searches a path to determine if it contains a valid prefix
///  of the type passed by pszPrefix.
/// A prefix is one of these types: "C:\\", ".", "..", "..\\"
    static BOOL       IsPrefix(LPCTSTR pszPrefix, LPCTSTR pszPath);

/// Searches a path and determines if it is relative    
    static BOOL       IsRelative(LPCTSTR pszPath);

/// Parses a path to determine if it is a directory root    
    static BOOL       IsRoot(LPCTSTR pszPath);

/// Compares two paths to determine if they have a common root component    
    static BOOL       IsSameRoot(LPCTSTR pszPath1, LPCTSTR pszPath2);

/// Determines if the string is a valid UNC (universal naming convention)
///  for a server and share path    
    static BOOL       IsUNC(LPCTSTR pszPath);

/// Determines whether a path string represents a network resource    
    static BOOL       IsNetworkPath(LPCTSTR pszPath);

/// Determines if a string is a valid UNC (universal naming convention)
///  for a server path only    
    static BOOL       IsUNCServer(LPCTSTR pszPath);

/// Determines if a string is a valid universal naming convention (UNC)
///  share path, \\\\server\\share    
    static BOOL       IsUNCServerShare(LPCTSTR pszPath);

/// Determines if a file's registered content type matches
///  the specified content type.
/// This function obtains the content type for the specified file type
///  and compares that string with the pszContentType.
/// The comparison is not case sensitive
    static BOOL       IsContentType(LPCTSTR pszPath, LPCTSTR pszContentType);

/// Tests a given string to determine if it conforms to a valid URL format    
    static BOOL       IsURL(LPCTSTR pszPath);

/// Converts a path to all lowercase characters
///  to give the path a consistent appearance    
    static BOOL       MakePretty(LPTSTR pszPath);

/// Searches a string using a DOS wild card match type    
    static BOOL       MatchSpec(LPCTSTR pszFile, LPCTSTR pszSpec);

/// Parses a file location string containing a file location and icon index,
///  and returns separate values    
    static int        ParseIconLocation(LPTSTR pszIconFile);

/// Searches a path for spaces. If spaces are found,
///  the entire path is enclosed in quotation marks    
    static void       QuoteSpaces(LPTSTR lpsz);

/// Creates a relative path from one file or folder to another    
    static BOOL       RelativePathTo(LPTSTR pszPath, LPCTSTR pszFrom, DWORD dwAttrFrom, LPCTSTR pszTo, DWORD dwAttrTo);

/// Removes any arguments from a given path    
    static void       RemoveArgs(LPTSTR pszPath);

/// Removes the trailing backslash from a given path
    static LPTSTR      RemoveBackslash(LPTSTR pszPath);

/// Removes all leading and trailing spaces from a string    
    static void       RemoveBlanks(LPTSTR pszPath);

/// Removes the file extension from a path, if there is one    
    static void       RemoveExtension(LPTSTR pszPath);

/// Removes the trailing file name and backslash from a path, if it has them
    static BOOL       RemoveFileSpec(LPTSTR pszPath);

/// Replaces the extension of a file name with a new extension.
/// If the file name does not contain an extension,
///  the extension will be attached to the end of the string    
    static BOOL       RenameExtension(LPTSTR pszPath, LPCTSTR pszExt);

/// Determines if a given path is correctly formatted and fully qualified    
    static BOOL       SearchAndQualify(LPCTSTR pszPath, LPTSTR pszBuf, UINT cchBuf);

/// Sets the text of a child control in a window or dialog box,
///  using PathCompactPath to make sure the path fits in the control    
    static void       SetDlgItemPath(HWND hDlg, int id, LPCTSTR pszPath);

/// Parses a path, ignoring the drive letter or UNC server/share path parts    
    static LPTSTR      SkipRoot(LPCTSTR pszPath);

/// Removes the path portion of a fully qualified path and file
    static void       StripPath(LPTSTR pszPath);

/// Removes all parts of the path except for the root information
    static BOOL       StripToRoot(LPTSTR pszPath);

/// Removes quotes from the beginning and end of a path
    static void       UnquoteSpaces(LPTSTR lpsz);

/// Gives an existing folder the proper attributes to become a system folder
    static BOOL       MakeSystemFolder(LPCTSTR pszPath);

/// Removes the attributes from a folder that make it a system folder.
/// This folder must actually exist in the file system
    static BOOL       UnmakeSystemFolder(LPCTSTR pszPath);

/// Determines if an existing folder contains the attributes
///  that make it a system folder.
/// Alternately indicates if certain attributes qualify
///  a folder to be a system folder
    static BOOL       IsSystemFolder(LPCTSTR pszPath, DWORD dwAttrb);

/// Removes the decoration from a path string
    static void       Undecorate(LPTSTR pszPath);

/// Takes a fully qualified path, and replaces several folder names
///  with their associated environment string
    static BOOL       UnExpandEnvStrings(LPCTSTR pszPath, LPTSTR pszBuf, UINT cchBuf);
    static BOOL       UnExpandEnvStringsForUser(HANDLE hToken, LPCTSTR pszPath, LPTSTR pszBuf, UINT cchBuf);


    static  TModule&  GetModule();
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OWL namespace


#endif  // OWL_SHELWAPI_H

