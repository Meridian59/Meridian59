//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementations of Windows light-weight utility APIs:
///    TSHReg, TSHPath
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/shelwapi.h>
#include <owl/module.h>

namespace owl {

////////////////////////////////////////////////////////////////////
//
// class TSHReg
// ~~~~~ ~~~~~~
// delay loading SHLWAPI.DLL
  static const tchar shellStr[]            = _T("SHLWAPI.DLL");

  static const char DuplicateKeyStr[]       = "SHRegDuplicateHKey";
# if defined(UNICODE)
  static const char DeleteEmptyKeyStr[]      = "SHDeleteEmptyKeyW";
  static const char DeleteKeyStr[]          = "SHDeleteKeyW";
  static const char DeleteValueStr[]        = "SHDeleteValueW";
  static const char GetValueStr[]            = "SHGetValueW";
  static const char SetValueStr[]            = "SHSetValueW";
  static const char QueryValueExStr[]        = "SHQueryValueExW";
  static const char EnumKeyExStr[]          = "SHEnumKeyExW";
  static const char EnumValueStr[]           = "SHEnumValueW";
  static const char QueryInfoKeyStr[]         = "SHQueryInfoKeyW";
  static const char CopyKeyStr[]              = "SHCopyKeyW";
  static const char RegGetPathStr[]          = "SHRegGetPathW";
  static const char RegSetPathStr[]         = "SHRegSetPathW";
# else
  static const char DeleteEmptyKeyStr[]      = "SHDeleteEmptyKeyA";
  static const char DeleteKeyStr[]          = "SHDeleteKeyA";
  static const char DeleteValueStr[]        = "SHDeleteValueA";
  static const char GetValueStr[]            = "SHGetValueA";
  static const char SetValueStr[]            = "SHSetValueA";
  static const char QueryValueExStr[]        = "SHQueryValueExA";
  static const char EnumKeyExStr[]          = "SHEnumKeyExA";
  static const char EnumValueStr[]           = "SHEnumValueA";
  static const char QueryInfoKeyStr[]        = "SHQueryInfoKeyA";
  static const char CopyKeyStr[]             = "SHCopyKeyA";
  static const char RegGetPathStr[]         = "SHRegGetPathA";
  static const char RegSetPathStr[]         = "SHRegSetPathA";
# endif


//
// Returns TModule object wrapping the handle of the SHLWAPI.DLL module
//
TModule&
TSHReg::GetModule()
{
  static TModule shellModule(shellStr, true, true, false);
  return shellModule;
}

DWORD
TSHReg::DeleteEmptyKey(HKEY hkey, LPCTSTR pszSubKey)
{
  static TModuleProc2<DWORD,HKEY,LPCTSTR>
         deleteEmptyKey(GetModule(), DeleteEmptyKeyStr);
  return deleteEmptyKey(hkey,pszSubKey);
}

DWORD
TSHReg::DeleteKey(HKEY hkey, LPCTSTR pszSubKey)
{
  static TModuleProc2<DWORD,HKEY,LPCTSTR>
         deleteKey(GetModule(), DeleteKeyStr);
  return deleteKey(hkey,pszSubKey);
}

HKEY
TSHReg::DuplicateKey(HKEY hkey)
{
  static TModuleProc1<HKEY,HKEY>
         duplicateKey(GetModule(), DuplicateKeyStr);
  return duplicateKey(hkey);
}

DWORD
TSHReg::DeleteValue(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue)
{
  static TModuleProc3<DWORD,HKEY,LPCTSTR,LPCTSTR>
         deleteValue(GetModule(), DeleteValueStr);
  return deleteValue(hkey, pszSubKey, pszValue);
}

DWORD
TSHReg::GetValue(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPDWORD pdwType, LPVOID pvData, LPDWORD pcbData)
{
  static TModuleProc6<DWORD,HKEY,LPCTSTR,LPCTSTR,LPDWORD,LPVOID,LPDWORD>
         getValue(GetModule(), GetValueStr);
  return getValue(hkey, pszSubKey, pszValue, pdwType, pvData, pcbData);
}

DWORD
TSHReg::SetValue(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, DWORD dwType, LPCVOID pvData, DWORD cbData)
{
  static TModuleProc6<DWORD,HKEY,LPCTSTR,LPCTSTR,DWORD,LPCVOID,DWORD>
         setValue(GetModule(), SetValueStr);
  return setValue(hkey, pszSubKey, pszValue, dwType, pvData, cbData);
}

DWORD
TSHReg::QueryValueEx(HKEY hkey, LPCTSTR pszValue, LPDWORD pdwReserved, LPDWORD pdwType, LPVOID pvData, LPDWORD pcbData)
{
  static TModuleProc6<DWORD,HKEY,LPCTSTR,LPDWORD,LPDWORD,LPVOID,LPDWORD>
         queryValueEx(GetModule(), QueryValueExStr);
  return queryValueEx(hkey, pszValue, pdwReserved, pdwType, pvData, pcbData);
}

LONG
TSHReg::EnumKeyEx(HKEY hkey, DWORD dwIndex, LPTSTR pszName, LPDWORD pcchName)
{
  static TModuleProc4<LONG,HKEY,DWORD,LPTSTR,LPDWORD>
         enumKeyEx(GetModule(), EnumKeyExStr);
  return enumKeyEx(hkey, dwIndex, pszName, pcchName);         
}

LONG
TSHReg::EnumValue(HKEY hkey, DWORD dwIndex, LPTSTR pszValueName, LPDWORD pcchValueName, LPDWORD pdwType, LPVOID pvData, LPDWORD pcbData)
{
  static TModuleProc7<LONG,HKEY,DWORD,LPTSTR,LPDWORD,LPDWORD,LPVOID,LPDWORD>
         enumValue(GetModule(), EnumValueStr);
  return enumValue(hkey, dwIndex, pszValueName, pcchValueName, pdwType, pvData, pcbData);
}

LONG
TSHReg::QueryInfoKey(HKEY hkey, LPDWORD pcSubKeys, LPDWORD pcchMaxSubKeyLen, LPDWORD pcValues, LPDWORD pcchMaxValueNameLen)
{
  static TModuleProc5<LONG,HKEY,LPDWORD,LPDWORD,LPDWORD,LPDWORD>
         queryInfoKey(GetModule(), QueryInfoKeyStr);
  return queryInfoKey(hkey, pcSubKeys, pcchMaxSubKeyLen, pcValues, pcchMaxValueNameLen);
}

// recursive key copy
DWORD
TSHReg::CopyKey(HKEY hkeySrc, LPCTSTR szSrcSubKey, HKEY hkeyDest, DWORD fReserved)
{
  static TModuleProc4<DWORD,HKEY,LPCTSTR,HKEY,DWORD>
         copyKey(GetModule(), CopyKeyStr);
  return copyKey(hkeySrc, szSrcSubKey, hkeyDest, fReserved);
}

// Getting and setting file system paths with environment variables
DWORD
TSHReg::RegGetPath(HKEY hkey, LPCTSTR pcszSubKey, LPCTSTR pcszValue, LPTSTR pszPath, DWORD dwFlags)
{
  static TModuleProc5<DWORD,HKEY,LPCTSTR,LPCTSTR,LPTSTR,DWORD>
         regGetPath(GetModule(), RegGetPathStr);
  return regGetPath(hkey, pcszSubKey, pcszValue, pszPath, dwFlags);
}

DWORD
TSHReg::RegSetPath(HKEY hkey, LPCTSTR pcszSubKey, LPCTSTR pcszValue, LPCTSTR pcszPath, DWORD dwFlags)
{
  static TModuleProc5<DWORD,HKEY,LPCTSTR,LPCTSTR,LPCTSTR,DWORD>
         regSetPath(GetModule(), RegSetPathStr);
  return regSetPath(hkey, pcszSubKey, pcszValue, pcszPath, dwFlags);
}

////////////////////////////////////////////////////////////////////
//
// class TSHPath
// ~~~~~ ~~~~~~

# if defined(UNICODE)
  static const char AddBackslashStr[]                = "PathAddBackslashW";
  static const char AddExtensionStr[]                = "PathAddExtensionW";
  static const char AppendStr[]                      = "PathAppendW";
  static const char BuildRootStr[]                   = "PathBuildRootW";
  static const char CanonicalizeStr[]                 = "PathCanonicalizeW";
  static const char CombineStr[]                     = "PathCombineW";
  static const char CompactPathStr[]                 = "PathCompactPathW";
  static const char CompactPathExStr[]              = "PathCompactPathExW";
  static const char CommonPrefixStr[]                = "PathCommonPrefixW";
  static const char FileExistsStr[]                 = "PathFileExistsW";
  static const char FindExtensionStr[]               = "PathFindExtensionW";
  static const char FindFileNameStr[]               = "PathFindFileNameW";
  static const char FindNextComponentStr[]           = "PathFindNextComponentW";
  static const char FindOnPathStr[]                 = "PathFindOnPathW";
  static const char GetArgsStr[]                     = "PathGetArgsW";
  static const char FindSuffixArrayStr[]              = "PathFindSuffixArrayW";
  static const char IsLFNFileSpecStr[]                = "PathIsLFNFileSpecW";
  static const char GetCharTypeStr[]                  = "PathGetCharTypeW";
  static const char GetDriveNumberStr[]             = "PathGetDriveNumberW";
  static const char IsDirectoryStr[]                 = "PathIsDirectoryW";
  static const char IsDirectoryEmptyStr[]           = "PathIsDirectoryEmptyW";
  static const char IsFileSpecStr[]                 = "PathIsFileSpecW";
  static const char IsPrefixStr[]                   = "PathIsPrefixW";
  static const char IsRelativeStr[]                 = "PathIsRelativeW";
  static const char IsRootStr[]                     = "PathIsRootW";
  static const char IsSameRootStr[]                   = "PathIsSameRootW";
  static const char IsUNCStr[]                       = "PathIsUNCW";
  static const char IsNetworkPathStr[]               = "PathIsNetworkPathW";
  static const char IsUNCServerStr[]                 = "PathIsUNCServerW";
  static const char IsUNCServerShareStr[]            = "PathIsUNCServerShareW";
  static const char IsContentTypeStr[]               = "PathIsContentTypeW";
  static const char IsURLStr[]                       = "PathIsURLW";
  static const char MakePrettyStr[]                  = "PathMakePrettyW";
  static const char MatchSpecStr[]                  = "PathMatchSpecW";
  static const char ParseIconLocationStr[]          = "PathParseIconLocationW";
  static const char QuoteSpacesStr[]                = "PathQuoteSpacesW";
  static const char RelativePathToStr[]             = "PathRelativePathToW";
  static const char RemoveArgsStr[]                 = "PathRemoveArgsW";
  static const char RemoveBackslashStr[]            = "PathRemoveBackslashW";
  static const char RemoveBlanksStr[]               = "PathRemoveBlanksW";
  static const char RemoveExtensionStr[]            = "PathRemoveExtensionW";
  static const char RemoveFileSpecStr[]             = "PathRemoveFileSpecW";
  static const char RenameExtensionStr[]            = "PathRenameExtensionW";
  static const char SearchAndQualifyStr[]           = "PathSearchAndQualifyW";
  static const char SetDlgItemPathStr[]             = "PathSetDlgItemPathW";
  static const char SkipRootStr[]                   = "PathSkipRootW";
  static const char StripPathStr[]                  = "PathStripPathW";
  static const char StripToRootStr[]                = "PathStripToRootW";
  static const char UnquoteSpacesStr[]              = "PathUnquoteSpacesW";
  static const char MakeSystemFolderStr[]           = "PathMakeSystemFolderW";
  static const char UnmakeSystemFolderStr[]         = "PathUnmakeSystemFolderW";
  static const char IsSystemFolderStr[]             = "PathIsSystemFolderW";
  static const char UndecorateStr[]                 = "PathUndecorateW";
  static const char UnExpandEnvStringsStr[]          = "PathUnExpandEnvStringsW";
  static const char UnExpandEnvStringsForUserStr[]  = "PathUnExpandEnvStringsForUserW";
# else
  static const char AddBackslashStr[]                = "PathAddBackslashA";
  static const char AddExtensionStr[]                = "PathAddExtensionA";
  static const char AppendStr[]                      = "PathAppendA";
  static const char BuildRootStr[]                  = "PathBuildRootA";
  static const char CanonicalizeStr[]                = "PathCanonicalizeA";
  static const char CombineStr[]                    = "PathCombineA";
  static const char CompactPathStr[]                 = "PathCompactPathA";
  static const char CompactPathExStr[]              = "PathCompactPathExA";
  static const char CommonPrefixStr[]                = "PathCommonPrefixA";
  static const char FileExistsStr[]                 = "PathFileExistsA";
  static const char FindExtensionStr[]               = "PathFindExtensionA";
  static const char FindFileNameStr[]               = "PathFindFileNameA";
  static const char FindNextComponentStr[]           = "PathFindNextComponentA";
  static const char FindOnPathStr[]                 = "PathFindOnPathA";
  static const char GetArgsStr[]                     = "PathGetArgsA";
  static const char FindSuffixArrayStr[]              = "PathFindSuffixArrayA";
  static const char IsLFNFileSpecStr[]                = "PathIsLFNFileSpecA";
  static const char GetCharTypeStr[]                  = "PathGetCharTypeA";
  static const char GetDriveNumberStr[]             = "PathGetDriveNumberA";
  static const char IsDirectoryStr[]                 = "PathIsDirectoryA";
  static const char IsDirectoryEmptyStr[]           = "PathIsDirectoryEmptyA";
  static const char IsFileSpecStr[]                 = "PathIsFileSpecA";
  static const char IsPrefixStr[]                   = "PathIsPrefixA";
  static const char IsRelativeStr[]                 = "PathIsRelativeA";
  static const char IsRootStr[]                      = "PathIsRootA";
  static const char IsSameRootStr[]                 = "PathIsSameRootA";
  static const char IsUNCStr[]                       = "PathIsUNCA";
  static const char IsNetworkPathStr[]               = "PathIsNetworkPathA";
  static const char IsUNCServerStr[]                 = "PathIsUNCServerA";
  static const char IsUNCServerShareStr[]            = "PathIsUNCServerShareA";
  static const char IsContentTypeStr[]               = "PathIsContentTypeA";
  static const char IsURLStr[]                       = "PathIsURLA";
  static const char MakePrettyStr[]                  = "PathMakePrettyA";
  static const char MatchSpecStr[]                  = "PathMatchSpecA";
  static const char ParseIconLocationStr[]          = "PathParseIconLocationA";
  static const char QuoteSpacesStr[]                = "PathQuoteSpacesA";
  static const char RelativePathToStr[]             = "PathRelativePathToA";
  static const char RemoveArgsStr[]                 = "PathRemoveArgsA";
  static const char RemoveBackslashStr[]            = "PathRemoveBackslashA";
  static const char RemoveBlanksStr[]               = "PathRemoveBlanksA";
  static const char RemoveExtensionStr[]            = "PathRemoveExtensionA";
  static const char RemoveFileSpecStr[]             = "PathRemoveFileSpecA";
  static const char RenameExtensionStr[]            = "PathRenameExtensionA";
  static const char SearchAndQualifyStr[]           = "PathSearchAndQualifyA";
  static const char SetDlgItemPathStr[]             = "PathSetDlgItemPathA";
  static const char SkipRootStr[]                   = "PathSkipRootA";
  static const char StripPathStr[]                  = "PathStripPathA";
  static const char StripToRootStr[]                = "PathStripToRootA";
  static const char UnquoteSpacesStr[]              = "PathUnquoteSpacesA";
  static const char MakeSystemFolderStr[]           = "PathMakeSystemFolderA";
  static const char UnmakeSystemFolderStr[]         = "PathUnmakeSystemFolderA";
  static const char IsSystemFolderStr[]             = "PathIsSystemFolderA";
  static const char UndecorateStr[]                 = "PathUndecorateA";
  static const char UnExpandEnvStringsStr[]          = "PathUnExpandEnvStringsA";
  static const char UnExpandEnvStringsForUserStr[]  = "PathUnExpandEnvStringsForUserA";
# endif


//
// Returns TModule object wrapping the handle of the SHLWAPI.DLL module
//
TModule&
TSHPath::GetModule()
{
  static TModule shellModule(shellStr, true, true, false);
  return shellModule;
}

LPTSTR
TSHPath::AddBackslash(LPTSTR pszPath)
{
  static TModuleProc1<LPTSTR,LPTSTR>
         addBackslash(GetModule(), AddBackslashStr);
  return addBackslash(pszPath);
}

BOOL
TSHPath::AddExtension(LPTSTR pszPath, LPCTSTR pszExt)
{
  static TModuleProc2<BOOL,LPTSTR,LPCTSTR>
         addExtension(GetModule(), AddExtensionStr);
  return addExtension(pszPath, pszExt);
}

BOOL
TSHPath::Append(LPTSTR pszPath, LPCTSTR pszMore)
{
  static TModuleProc2<BOOL,LPTSTR,LPCTSTR>
         append(GetModule(), AppendStr);
  return append(pszPath, pszMore);
}

LPTSTR
TSHPath::BuildRoot(LPTSTR pszRoot, int iDrive)
{
  static TModuleProc2<LPTSTR,LPTSTR,int>
         buildRoot(GetModule(), BuildRootStr);
  return buildRoot(pszRoot, iDrive);
}

BOOL
TSHPath::Canonicalize(LPTSTR pszBuf, LPCTSTR pszPath)
{
  static TModuleProc2<BOOL,LPTSTR,LPCTSTR>
         canonicalize(GetModule(), CanonicalizeStr);
  return canonicalize(pszBuf, pszPath);
}

LPTSTR
TSHPath::Combine(LPTSTR pszDest, LPCTSTR pszDir, LPCTSTR pszFile)
{
  static TModuleProc3<LPTSTR,LPTSTR,LPCTSTR,LPCTSTR>
         combine(GetModule(), CombineStr);
  return combine(pszDest, pszDir, pszFile);
}

BOOL
TSHPath::CompactPath(HDC hDC, LPTSTR pszPath, UINT dx)
{
  static TModuleProc3<BOOL,HDC,LPTSTR,UINT>
         compactPath(GetModule(), CompactPathStr);
  return compactPath(hDC, pszPath, dx);
}

BOOL
TSHPath::CompactPathEx(LPTSTR pszOut, LPCTSTR pszSrc, UINT cchMax, DWORD dwFlags)
{
  static TModuleProc4<BOOL,LPTSTR,LPCTSTR,UINT,DWORD>
         compactPathEx(GetModule(), CompactPathExStr);
  return compactPathEx(pszOut, pszSrc, cchMax, dwFlags);
}

int
TSHPath::CommonPrefix(LPCTSTR pszFile1, LPCTSTR pszFile2, LPTSTR achPath)
{
  static TModuleProc3<int,LPCTSTR,LPCTSTR,LPTSTR>
         commonPrefix(GetModule(), CommonPrefixStr);
  return commonPrefix(pszFile1, pszFile2, achPath);
}

BOOL
TSHPath::FileExists(LPCTSTR pszPath)
{
  static TModuleProc1<BOOL,LPCTSTR>
         fileExists(GetModule(), FileExistsStr);
  return fileExists(pszPath);
}

LPTSTR
TSHPath::FindExtension(LPCTSTR pszPath)
{
  static TModuleProc1<LPTSTR,LPCTSTR>
         findExtension(GetModule(), FindExtensionStr);
  return findExtension(pszPath);
}

LPTSTR
TSHPath::FindFileName(LPCTSTR pszPath)
{
  static TModuleProc1<LPTSTR,LPCTSTR>
         findFileName(GetModule(), FindFileNameStr);
  return findFileName(pszPath);
}

LPTSTR
TSHPath::FindNextComponent(LPCTSTR pszPath)
{
  static TModuleProc1<LPTSTR,LPCTSTR>
         findNextComponent(GetModule(), FindNextComponentStr);
  return findNextComponent(pszPath);
}

BOOL
TSHPath::FindOnPath(LPTSTR pszPath, LPCTSTR * ppszOtherDirs)
{
  static TModuleProc2<BOOL,LPTSTR,LPCTSTR*>
         findOnPath(GetModule(), FindOnPathStr);
  return findOnPath(pszPath, ppszOtherDirs);
}

LPTSTR
TSHPath::GetArgs(LPCTSTR pszPath)
{
  static TModuleProc1<LPTSTR,LPCTSTR>
         getArgs(GetModule(), GetArgsStr);
  return getArgs(pszPath);
}

LPCTSTR
TSHPath::FindSuffixArray(LPCTSTR pszPath, const LPCTSTR *apszSuffix, int iArraySize)
{
  static TModuleProc3<LPCTSTR,LPCTSTR,const LPCTSTR *,int>
         findSuffixArray(GetModule(), FindSuffixArrayStr);
  return findSuffixArray(pszPath, apszSuffix, iArraySize);
}

BOOL
TSHPath::IsLFNFileSpec(LPCTSTR lpName)
{
  static TModuleProc1<BOOL,LPCTSTR>
         isLFNFileSpec(GetModule(), IsLFNFileSpecStr);
  return isLFNFileSpec(lpName);
}

UINT
TSHPath::GetCharType(tchar ch)
{
  static TModuleProc1<UINT,tchar>
         getCharType(GetModule(), GetCharTypeStr);
  return getCharType(ch);
}

int
TSHPath::GetDriveNumber(LPCTSTR pszPath)
{
  static TModuleProc1<int,LPCTSTR>
         getDriveNumber(GetModule(), GetDriveNumberStr);
  return getDriveNumber(pszPath);
}

BOOL
TSHPath::IsDirectory(LPCTSTR pszPath)
{
  static TModuleProc1<BOOL,LPCTSTR>
         isDirectory(GetModule(), IsDirectoryStr);
  return isDirectory(pszPath);
}

BOOL
TSHPath::IsDirectoryEmpty(LPCTSTR pszPath)
{
  static TModuleProc1<BOOL,LPCTSTR>
         isDirectoryEmpty(GetModule(), IsDirectoryEmptyStr);
  return isDirectoryEmpty(pszPath);
}

BOOL
TSHPath::IsFileSpec(LPCTSTR pszPath)
{
  static TModuleProc1<BOOL,LPCTSTR>
         isFileSpec(GetModule(), IsFileSpecStr);
  return isFileSpec(pszPath);
}

BOOL
TSHPath::IsPrefix(LPCTSTR pszPrefix, LPCTSTR pszPath)
{
  static TModuleProc2<BOOL,LPCTSTR,LPCTSTR>
         isPrefix(GetModule(), IsPrefixStr);
  return isPrefix(pszPrefix, pszPath);
}

BOOL
TSHPath::IsRelative(LPCTSTR pszPath)
{
  static TModuleProc1<BOOL,LPCTSTR>
         isRelative(GetModule(), IsRelativeStr);
  return isRelative(pszPath);
}

BOOL
TSHPath::IsRoot(LPCTSTR pszPath)
{
  static TModuleProc1<BOOL,LPCTSTR>
         isRoot(GetModule(), IsRootStr);
  return isRoot(pszPath);
}

BOOL
TSHPath::IsSameRoot(LPCTSTR pszPath1, LPCTSTR pszPath2)
{
  static TModuleProc2<BOOL,LPCTSTR,LPCTSTR>
         isSameRoot(GetModule(), IsSameRootStr);
  return isSameRoot(pszPath1, pszPath2);
}

BOOL
TSHPath::IsUNC(LPCTSTR pszPath)
{
  static TModuleProc1<BOOL,LPCTSTR>
         isUNC(GetModule(), IsUNCStr);
  return isUNC(pszPath);
}

BOOL
TSHPath::IsNetworkPath(LPCTSTR pszPath)
{
  static TModuleProc1<BOOL,LPCTSTR>
         isNetworkPath(GetModule(), IsNetworkPathStr);
  return isNetworkPath(pszPath);
}

BOOL
TSHPath::IsUNCServer(LPCTSTR pszPath)
{
  static TModuleProc1<BOOL,LPCTSTR>
         isUNCServer(GetModule(), IsUNCServerStr);
  return isUNCServer(pszPath);
}

BOOL
TSHPath::IsUNCServerShare(LPCTSTR pszPath)
{
  static TModuleProc1<BOOL,LPCTSTR>
         isUNCServerShare(GetModule(), IsUNCServerShareStr);
  return isUNCServerShare(pszPath);
}

BOOL
TSHPath::IsContentType(LPCTSTR pszPath, LPCTSTR pszContentType)
{
  static TModuleProc2<BOOL,LPCTSTR,LPCTSTR>
         isContentType(GetModule(), IsContentTypeStr);
  return isContentType(pszPath,pszContentType);
}

BOOL
TSHPath::IsURL(LPCTSTR pszPath)
{
  static TModuleProc1<BOOL,LPCTSTR>
         isURL(GetModule(), IsURLStr);
  return isURL(pszPath);
}

BOOL
TSHPath::MakePretty(LPTSTR pszPath)
{
  static TModuleProc1<BOOL,LPTSTR>
         makePretty(GetModule(), MakePrettyStr);
  return makePretty(pszPath);
}

BOOL
TSHPath::MatchSpec(LPCTSTR pszFile, LPCTSTR pszSpec)
{
  static TModuleProc2<BOOL,LPCTSTR,LPCTSTR>
         matchSpec(GetModule(), MatchSpecStr);
  return matchSpec(pszFile, pszSpec);
}

int
TSHPath::ParseIconLocation(LPTSTR pszIconFile)
{
  static TModuleProc1<int,LPTSTR>
         parseIconLocation(GetModule(), ParseIconLocationStr);
  return parseIconLocation(pszIconFile);
}

void
TSHPath::QuoteSpaces(LPTSTR lpsz)
{
  static TModuleProcV1<LPTSTR>
         quoteSpaces(GetModule(), QuoteSpacesStr);
  quoteSpaces(lpsz);
}

BOOL
TSHPath::RelativePathTo(LPTSTR pszPath, LPCTSTR pszFrom, DWORD dwAttrFrom, LPCTSTR pszTo, DWORD dwAttrTo)
{
  static TModuleProc5<BOOL,LPTSTR,LPCTSTR,DWORD,LPCTSTR,DWORD>
         relativePathTo(GetModule(), RelativePathToStr);
  return relativePathTo(pszPath, pszFrom, dwAttrFrom, pszTo, dwAttrTo);
}

void
TSHPath::RemoveArgs(LPTSTR pszPath)
{
  static TModuleProcV1<LPTSTR>
         removeArgs(GetModule(), RemoveArgsStr);
  removeArgs(pszPath);
}

LPTSTR
TSHPath::RemoveBackslash(LPTSTR pszPath)
{
  static TModuleProc1<LPTSTR,LPTSTR>
         removeBackslash(GetModule(), RemoveBackslashStr);
  return removeBackslash(pszPath);
}

void
TSHPath::RemoveBlanks(LPTSTR pszPath)
{
  static TModuleProcV1<LPTSTR>
         removeBlanks(GetModule(), RemoveBlanksStr);
  removeBlanks(pszPath);
}

void
TSHPath::RemoveExtension(LPTSTR pszPath)
{
  static TModuleProcV1<LPTSTR>
         removeExtension(GetModule(), RemoveExtensionStr);
  removeExtension(pszPath);
}

BOOL
TSHPath::RemoveFileSpec(LPTSTR pszPath)
{
  static TModuleProc1<BOOL,LPTSTR>
         removeFileSpec(GetModule(), RemoveFileSpecStr);
  return removeFileSpec(pszPath);
}

BOOL
TSHPath::RenameExtension(LPTSTR pszPath, LPCTSTR pszExt)
{
  static TModuleProc2<BOOL,LPTSTR,LPCTSTR>
         renameExtension(GetModule(), RenameExtensionStr);
  return renameExtension(pszPath, pszExt);
}

BOOL
TSHPath::SearchAndQualify(LPCTSTR pszPath, LPTSTR pszBuf, UINT cchBuf)
{
  static TModuleProc3<BOOL,LPCTSTR,LPTSTR,UINT>
         searchAndQualify(GetModule(), SearchAndQualifyStr);
  return searchAndQualify(pszPath, pszBuf, cchBuf);
}

void
TSHPath::SetDlgItemPath(HWND hDlg, int id, LPCTSTR pszPath)
{
  static TModuleProcV3<HWND,int,LPCTSTR>
         setDlgItemPath(GetModule(), SetDlgItemPathStr);
  setDlgItemPath(hDlg, id, pszPath);
}

LPTSTR
TSHPath::SkipRoot(LPCTSTR pszPath)
{
  static TModuleProc1<LPTSTR,LPCTSTR>
         skipRoot(GetModule(), SkipRootStr);
  return skipRoot(pszPath);
}

void
TSHPath::StripPath(LPTSTR pszPath)
{
  static TModuleProcV1<LPTSTR>
         stripPath(GetModule(), StripPathStr);
  stripPath(pszPath);
}

BOOL
TSHPath::StripToRoot(LPTSTR pszPath)
{
  static TModuleProc1<BOOL,LPTSTR>
         stripToRoot(GetModule(), StripToRootStr);
  return stripToRoot(pszPath);
}

void
TSHPath::UnquoteSpaces(LPTSTR lpsz)
{
  static TModuleProcV1<LPTSTR>
         unquoteSpaces(GetModule(), UnquoteSpacesStr);
  unquoteSpaces(lpsz);
}

BOOL
TSHPath::MakeSystemFolder(LPCTSTR pszPath)
{
  static TModuleProc1<BOOL,LPCTSTR>
         makeSystemFolder(GetModule(), MakeSystemFolderStr);
  return makeSystemFolder(pszPath);
}

BOOL
TSHPath::UnmakeSystemFolder(LPCTSTR pszPath)
{
  static TModuleProc1<BOOL,LPCTSTR>
         unmakeSystemFolder(GetModule(), UnmakeSystemFolderStr);
  return unmakeSystemFolder(pszPath);
}

BOOL
TSHPath::IsSystemFolder(LPCTSTR pszPath, DWORD dwAttrb)
{
  static TModuleProc2<BOOL,LPCTSTR,DWORD>
         isSystemFolder(GetModule(), IsSystemFolderStr);
  return isSystemFolder(pszPath,dwAttrb);
}

void
TSHPath::Undecorate(LPTSTR pszPath)
{
  static TModuleProcV1<LPTSTR>
         undecorate(GetModule(), UndecorateStr);
  undecorate(pszPath);
}


BOOL
TSHPath::UnExpandEnvStrings(LPCTSTR pszPath, LPTSTR pszBuf, UINT cchBuf)
{
  static TModuleProc3<BOOL,LPCTSTR,LPTSTR,UINT>
         pathUnExpandEnvStrings(GetModule(), UnExpandEnvStringsStr);
  return pathUnExpandEnvStrings(pszPath, pszBuf, cchBuf);
}

BOOL
TSHPath::UnExpandEnvStringsForUser(HANDLE hToken, LPCTSTR pszPath, LPTSTR pszBuf, UINT cchBuf)
{
  static TModuleProc4<BOOL,HANDLE,LPCTSTR,LPTSTR,UINT>
         pathUnExpandEnvStringsForUser(GetModule(), UnExpandEnvStringsForUserStr);
  return pathUnExpandEnvStringsForUser(hToken, pszPath, pszBuf, cchBuf);
}

} // OWL namespace
/* ========================================================================== */

