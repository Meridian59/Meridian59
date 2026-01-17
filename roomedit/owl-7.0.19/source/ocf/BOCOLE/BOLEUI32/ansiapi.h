/*
 * ansiapi.h
 * This file contains prototypes of ANSI version of OLE apis
 * and mapping fooA to foo
 */

#ifndef _ANSIAPI_INCL
#define _ANSIAPI_INCL

#define UNICODEOLE32

#define WASANSI() {                                                           \
                   OutputDebugString("\n\r\t\tUnicode string was ANSI!\r\n"); \
                   Assert(0);                                                 \
                   _asm { int 3 }                                             \
                  }



#ifndef UNICODEOLE32

#define W2A(w, a, cb)     lstrcpy (a, w)
#define A2W(a, w, cb)     lstrcpy (w, a)

#define FREELOCALSTRING(p)
#define MAKE_STR_LOCAL_COPYW2A(s, l)  l = s
#define MAKE_STR_LOCAL_COPYA2W(s, l)  l = s

#define OLESTRCPY                lstrcpy
#define OLESTRCAT                lstrcat
#define OLESTRLEN                lstrlen
#define OLESTRCMP                lstrcmp
#define OLESTRCMPI               lstrcmpi

#define DeleteFile_AW            DeleteFile
#define _lopen_AW                _lopen
#define _lcreat_AW               _lcreat
#define GlobalAddAtom_AW         GlobalAddAtom
#define GlobalGetAtomName_AW     GlobalGetAtomName
#define RegOpenKey_AW            RegOpenKey

#define CoLoadLibraryA   CoLoadLibrary

#define StringFromCLSID2A    StringFromCLSID2
#define StringFromIID2A      StringFromIID2
#define StringFromGUID2A     StringFromGUID2
#define CLSIDFromProgIDA     CLSIDFromProgID
#define CLSIDFromStringA     CLSIDFromString
#define ProgIDFromCLSIDA     ProgIDFromCLSID
#define StringFromCLSIDA     StringFromCLSID

#define UtDupStringA2W       UtDupString

// Storage APIs
#define StgOpenStorageA      StgOpenStorage

// IPersistFile ansi translation

#define IPersistFile_LoadA(pf, file, mode) (pf)->Load(file, mode)

// IMoniker ansi translation

#define IMoniker_GetDisplayNameA(pm, p1, p2, p3) (pm)->GetDisplayName(p1,p2,p3)

#define CreateFileMonikerA   CreateFileMoniker

#else  // UNICODEOLE32


#define W2A(w, a, cb)     WideCharToMultiByte(                              \
                                               CP_ACP,                      \
                                               0,                           \
                                               w,                           \
                                               -1,                          \
                                               a,                           \
                                               cb,                          \
                                               NULL,                        \
                                               NULL)

#define A2W(a, w, cb)     MultiByteToWideChar(                              \
                                               CP_ACP,                      \
                                               0,                           \
                                               a,                           \
                                               -1,                          \
                                               w,                           \
                                               cb)


#define FREELOCALSTRING(p)      delete (p)
#define MAKE_STR_LOCAL_COPYW2A(s, l)  {                                   \
                                      l = UtDupStringW2A(s);              \
                                      if (!l) {                           \
                                         return ResultFromScode(S_OOM);   \
                                      }                                   \
                                   }

#define MAKE_STR_LOCAL_COPYA2W(s, l)  {                                   \
                                      l = UtDupStringA2W(s);              \
                                      if (!l) {                           \
                                         return ResultFromScode(S_OOM);   \
                                      }                                   \
                                   }

#define OLESTRCPY                lstrcpyW
#define OLESTRCAT                lstrcatW
#define OLESTRLEN                lstrlenW
#define OLESTRCMP                lstrcmpW
#define OLESTRCMPI               lstrcmpiW

#define DeleteFile_AW            DeleteFileW
#define _lopen_AW                _lopenW
#define _lcreat_AW               _lcreatW
#define GlobalAddAtom_AW         GlobalAddAtomW
#define GlobalGetAtomName_AW     GlobalGetAtomNameW
#define RegOpenKey_AW            RegOpenKeyW

STDAPI_(HINSTANCE) CoLoadLibraryA(LPSTR lpszLibName, BOOL bAutoFree);

#define StringFromCLSID2A(rclsid, lpsz, cbMax) \
    StringFromGUID2A(rclsid, lpsz, cbMax)

#define StringFromIID2A(riid, lpsz, cbMax) \
    StringFromGUID2A(riid, lpsz, cbMax)


STDAPI_(int)  StringFromGUID2A(REFGUID rguid, LPSTR lpsz, int cbMax);
STDAPI        CLSIDFromProgIDA(LPCSTR szProgID, LPCLSID pclsid);
STDAPI        CLSIDFromStringA(LPSTR lpsz, LPCLSID lpclsid);
STDAPI        StringFromCLSIDA(REFCLSID rclsid, LPSTR FAR* lplpsz);
STDAPI        ProgIDFromCLSIDA (REFCLSID clsid, LPSTR FAR* lplpszProgID);

LPWSTR UtDupStringA2W(LPCSTR pSrc);
LPSTR  UtDupStringW2A(LPCWSTR pSrc);

// Storage APIs
STDAPI StgOpenStorageA(LPCSTR pwcsName,IStorage FAR *pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage FAR * FAR *ppstgOpen);

// IPersistFile ansi translation

HRESULT IPersistFile_LoadA(LPPERSISTFILE pIPF, LPSTR szFile, DWORD dwMode);

// IMoniker ansi translation

HRESULT IMoniker_GetDisplayNameA(LPMONIKER pm, LPBC p1, LPMONIKER p2, LPSTR FAR *p3);

//OLEAPI CreateFileMonikerA ( LPSTR lpszPathName, LPMONIKER FAR * ppmk );

#endif // !UNICODEOLE32

#endif //  _ANSIAPI_INCL
