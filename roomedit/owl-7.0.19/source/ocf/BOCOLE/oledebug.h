//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
// $Revision: 1.2 $
//
//  Some debugging utilities for getting symbolic output from the OLE2 error
//  codes and enums
//----------------------------------------------------------------------------
#ifndef _OLEDEBUG_H
#define _OLEDEBUG_H

#undef OLEDBG
// to enable the OLEDBG stuff, #define OLEDBG and add oledebug.obj in
// the makefile.

class IUnknownWatcher : public IUnknown
{
  public:

    IUnknownWatcher( IUnknown *Data, char *Name );
    // IUnknown
    virtual HRESULT _IFUNC QueryInterface(REFIID iid, void FAR* FAR* pif);
    virtual ULONG _IFUNC AddRef();
    virtual ULONG _IFUNC Release();

  protected:

    int      nRef();
    IUnknown *pUnk;
    char     Name[40];
};

class IMonikerWatcher : public IMoniker
{
public:
  IMonikerWatcher( IMoniker *pMoniker, char *Name );

  // *** IUnknown methods ***
  virtual HRESULT _IFUNC QueryInterface(REFIID riid, LPVOID FAR* ppvObj);
  virtual ULONG _IFUNC AddRef();
  virtual ULONG _IFUNC Release();

  // *** IPersist methods ***
  virtual HRESULT _IFUNC GetClassID(LPCLSID lpClassID);

  // *** IPersistStream methods ***
  virtual HRESULT _IFUNC IsDirty();
  virtual HRESULT _IFUNC Load(LPSTREAM pStm);
  virtual HRESULT _IFUNC Save(LPSTREAM pStm, BOOL fClearDirty);
  virtual HRESULT _IFUNC GetSizeMax(ULARGE_INTEGER FAR * pcbSize);

  // *** IMoniker methods ***
  virtual HRESULT _IFUNC BindToObject(LPBC pbc, LPMONIKER pmkToLeft,
    REFIID riidResult, LPVOID FAR* ppvResult);
  virtual HRESULT _IFUNC BindToStorage(LPBC pbc, LPMONIKER pmkToLeft,
    REFIID riid, LPVOID FAR* ppvObj);
  virtual HRESULT _IFUNC Reduce(LPBC pbc, DWORD dwReduceHowFar, LPMONIKER FAR*
    ppmkToLeft, LPMONIKER FAR * ppmkReduced);
  virtual HRESULT _IFUNC ComposeWith(LPMONIKER pmkRight, BOOL fOnlyIfNotGeneric,
    LPMONIKER FAR* ppmkComposite);
  virtual HRESULT _IFUNC Enum(BOOL fForward, LPENUMMONIKER FAR* ppenumMoniker);
  virtual HRESULT _IFUNC IsEqual(LPMONIKER pmkOtherMoniker);
  virtual HRESULT _IFUNC Hash(LPDWORD pdwHash);
  virtual HRESULT _IFUNC IsRunning(LPBC pbc, LPMONIKER pmkToLeft, LPMONIKER
    pmkNewlyRunning);
  virtual HRESULT _IFUNC GetTimeOfLastChange(LPBC pbc, LPMONIKER pmkToLeft,
    FILETIME FAR* pfiletime);
  virtual HRESULT _IFUNC Inverse(LPMONIKER FAR* ppmk);
  virtual HRESULT _IFUNC CommonPrefixWith(LPMONIKER pmkOther, LPMONIKER FAR*
    ppmkPrefix);
  virtual HRESULT _IFUNC RelativePathTo(LPMONIKER pmkOther, LPMONIKER FAR*
    ppmkRelPath);
  virtual HRESULT _IFUNC GetDisplayName(LPBC pbc, LPMONIKER pmkToLeft,
    LPSTR FAR* lplpszDisplayName);
  virtual HRESULT _IFUNC ParseDisplayName(LPBC pbc, LPMONIKER pmkToLeft,
    LPSTR lpszDisplayName, ULONG FAR* pchEaten,
    LPMONIKER FAR* ppmkOut);
  virtual HRESULT _IFUNC IsSystemMoniker(LPDWORD pdwMksys);

 protected:

  IMoniker *   pMoniker;
  char    Name[40];
};

class ReleaseWatcher : public IUnknown
{
  public:

    ReleaseWatcher( IUnknown *watched ) : Watched( watched ), nRef( 1 ){}
    virtual HRESULT _IFUNC QueryInterface(REFIID, LPVOID FAR *);
    virtual ULONG   _IFUNC AddRef();
    virtual ULONG   _IFUNC Release();
    IUnknown *Watched;
    ULONG nRef;
};

#include <assert.h>

#ifdef OLEDBG  /* put at end of class in order to not change member offsets */

class _ICLASS cDebug
{
  protected:

    int          DebugMode;
    char*        DebugClass;
    char*        DebugFunc;
    void         DebugEnter(char * dc, char * df, LPOLESTR s);
    SCODE        DebugReturn(char * dc, char * df, SCODE);
    HRESULT      DebugReturn(char * dc, char * df, HRESULT);
    BOOL         DebugReturn(char * dc, char * df, BOOL);
    ULONG        DebugReturn(char * dc, char * df, LPOLESTR s, ULONG);
    void         DebugReturn(char * dc, char * df, REFIID, void *);
    void         DebugExit(char * dc, char * df);
    HRESULT      DebugHResult(LPOLESTR, HRESULT);
    ULONG        DebugRefCnt (LPOLESTR, ULONG);
    void         cdecl DebugPrintf(LPOLESTR, ...);
    int          SetDebugMode(int m){DebugMode = m; return 1;}
    char *       prepareBuf (char *);
    void         DebugPrint (REFIID riid);
    void         DebugPrint (LPOLESTR pS, const RECT FAR * pR);
    void         DebugPrint (LPOLESTR pS, const SIZEL FAR * pL);
    void         DebugPrint (LPOLESTR pS, BOOL f);
    cDebug() : DebugMode (1) {}
};

struct cDebugEnumRecord
{
  LPOLESTR pEnumS;
  WORD  eValue;
};

class _ICLASS cDebugEnum
{
  cDebugEnumRecord * pEnumRecs;
  WORD count;
  LPOLESTR pEnumName;
  static char enumBuf [200];
public:
  cDebugEnum (LPOLESTR pN, cDebugEnumRecord *pR, WORD c) : pEnumName (pN), pEnumRecs (pR), count (c) {}

  void Print (DWORD e);

  LPOLESTR buf () { return enumBuf; }
};

extern cDebugEnum cOLERENDER;
extern cDebugEnum cOLEVERBATTRIB;
extern cDebugEnum cUSERCLASSTYPE;
extern cDebugEnum cOLEMISC;
extern cDebugEnum cOLECLOSE;
extern cDebugEnum cOLEGETMONIKER;
extern cDebugEnum cOLEWHICHMK;
extern cDebugEnum cBINDSPEED;
extern cDebugEnum cOLECONTF;
extern cDebugEnum cOLEUPDATE;
extern cDebugEnum cOLELINKBIND;
extern cDebugEnum cDVASPECT;
extern cDebugEnum cTYMED;
extern cDebugEnum cDATADIR;
extern cDebugEnum cADVF;
extern cDebugEnum cMEMCTX;
extern cDebugEnum cCLSCTX;
extern cDebugEnum cREGCLS;
extern cDebugEnum cMSHLFLAGS;
extern cDebugEnum cMSHCTX;
extern cDebugEnum cCALLTYPE;
extern cDebugEnum cSERVERCALL;
extern cDebugEnum cPENDINGTYPE;
extern cDebugEnum cPENDINGMSG;
extern cDebugEnum cBIND_FLAGS;
extern cDebugEnum cKSYS;
extern cDebugEnum cMKRREDUCE;
extern cDebugEnum cSTGC;
extern cDebugEnum cSTGTY;
extern cDebugEnum cSTREAM_SEEK;
extern cDebugEnum cLOCKTYPE;
extern cDebugEnum cSTGMOVE;
extern cDebugEnum cSTATFLAG;
extern cDebugEnum cOLEUIPASTEFLAG;

#else
class _ICLASS cDebug;
class _ICLASS cDebugEnum;
#endif

#ifdef OLEDBG
#  define OLEIMP(m,a) HRESULT _IFUNC _export OLE_IMPLEMENTATION::m a { \
                         DebugClass=OLE_INTERFACE; DebugFunc=#m; DebugEnter(OLE_INTERFACE,#m,0);
#  define OLEIMPRET(r,m,a) r _IFUNC OLE_IMPLEMENTATION::m a { \
                         DebugClass=OLE_INTERFACE; DebugFunc=#m; DebugEnter(OLE_INTERFACE,#m,0);
#  define OLEENTER DebugEnter(OLE_INTERFACE, OLE_FUNC, NULL);
#  define OLEENTERS(s) DebugEnter(OLE_INTERFACE, OLE_FUNC, s);
#  define OLERET(s) return ResultFromScode (DebugReturn(OLE_INTERFACE, OLE_FUNC, s))
#  define OLERES(r) return DebugReturn(OLE_INTERFACE, OLE_FUNC, r)
#  define OLEREF(s, r) return DebugReturn(OLE_INTERFACE, OLE_FUNC, s, r)
#  define OLEBRET(b) return DebugReturn(OLE_INTERFACE, OLE_FUNC, b)
#  define OLEPRET(id,p) return DebugReturn(OLE_INTERFACE, OLE_FUNC, id, p),p
#  define OLEEXIT DebugExit(OLE_INTERFACE, OLE_FUNC)
#  define OLEIID(id) DebugPrint (id)
#  define OLEHRES(s,r)  DebugHResult (s, r)
#  define OLEREFCNT(s,r) DebugRefCnt (s,r)
#  define OLEPRINTF1(l) DebugPrintf(l)
#  define OLEPRINTF2(l1,l2) DebugPrintf(l1,l2)
#  define OLEPRINTF3(l1,l2,l3) DebugPrintf(l1,l2,l3)
#  define OLEPRINTF4(l1,l2,l3,l4) DebugPrintf(l1,l2,l3,l4)
#  define OLEPRINTF5(l1,l2,l3,l4,l5) DebugPrintf(l1,l2,l3,l4,l5)
#  define OLEENUM(E,v) E.Print(v);DebugPrintf(E.buf())
#else
#  define OLEIMP(m,a) HRESULT _IFUNC _export OLE_IMPLEMENTATION::m a {
#  define OLEIMPRET(r,m,a) r _IFUNC OLE_IMPLEMENTATION::m a {
// #  define OLEENTER

#  define OLEENTER(s)
#  define OLEPRINTF1(l)
#  define OLEPRINTF2(l1,l2)
#  define OLEPRINTF3(l1,l2,l3)
#  define OLEPRINTF4(l1,l2,l3,l4)
#  define OLEPRINTF5(l1,l2,l3,l4,l5)
#  define OLEIID(id)
#  define OLEHRES(s,r) (r)
#  define OLERET(s) return ResultFromScode (s)
#  define OLERES(r) return r
#  define OLEREF(s, r) return r
#  define OLEREFCNT(s,r) (r)
#  define OLEEXIT
#  define OLEBRET(b) return b
#  define OLEPRET(id,p) return p
#  define OLEENUM(E,v)
#endif

#define OLE_INTERFACE DebugClass
#define OLE_FUNC DebugFunc

#endif
