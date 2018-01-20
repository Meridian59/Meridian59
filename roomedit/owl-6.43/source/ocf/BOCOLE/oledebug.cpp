//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
// $Revision: 1.2 $
//
// OleDebug.cpp -- Some debugging utilities for getting symbolic output
//                 from the OLE2 error codes and enums
//----------------------------------------------------------------------------
#include <bole.h>
#include <olestd.h>
#include <oledebug.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

OLEDBGDATA;

#undef OleDbgOutHResult(lpsz, hr)
#define OleDbgOutHResult(lpsz,hr)   \
        OleDbgPrintScodeAlways(g_szDbgPrefix,lpsz,GetScode(hr))

#define BUFFERSIZE 200
STDAPI_(void) OleDbgPrintScodeAlways(LPSTR lpszPrefix, LPSTR lpszMsg, SCODE sc);


#ifdef OLEDBG

static WORD level = 0;

char * cDebug::prepareBuf (char * pB)
{
  for (int i=0;i<level;i++) {
    *pB++ = '-';
    *pB++ = '-';
  }
  return pB;
}

void cDebug::DebugEnter(char * dc, char * df, LPOLESTR s)
{
  if (DebugMode) {
    char buf [BUFFERSIZE];
    if (s)
      wsprintf (prepareBuf(buf),"Enter %s::%s of object %s\n\r", dc, df, s);
    else
      wsprintf (prepareBuf(buf),"Enter %s::%s\n\r", dc, df);
    OleDbgOut (buf);
  }
  level++;
}

SCODE cDebug::DebugReturn(char * dc, char * df, SCODE sc)
{
  if (level)
    level--;
  if (DebugMode) {
    char buf [BUFFERSIZE];
    wsprintf (prepareBuf(buf),"Leave %s::%s with ", dc, df);
    OleDbgOutScode (buf, sc);
  }
  return sc;
}

HRESULT cDebug::DebugReturn(char * dc, char * df, HRESULT hrErr)
{
  if (level)
    level--;
  if (DebugMode) {
    char buf [BUFFERSIZE];
    wsprintf (prepareBuf(buf),"Leave %s::%s with ", dc, df);
    OleDbgOutHResult (buf, hrErr);
  }
  return hrErr;
}

BOOL cDebug::DebugReturn(char * dc, char * df, BOOL f)
{
  if (level)
    level--;
  if (DebugMode) {
    char buf [BUFFERSIZE];
    wsprintf (prepareBuf(buf),"Leave %s::%s with %s", dc, df, f ? "TRUE" : "FALSE");
    OleDbgOut (buf);
  }
  return f;
}

void cDebug::DebugReturn(char * dc, char * df, REFIID iid, void * P)
{
  if (level)
    level--;
  if (DebugMode) {
    char buf [BUFFERSIZE];
    wsprintf (prepareBuf(buf),"Leave %s::%s with pointer %08lX", dc, df, P);
    OleDbgOut (buf);
  }
}

ULONG cDebug::DebugReturn (char * dc, char * df, LPOLESTR s, ULONG r)
{
  if (level)
    level--;
  if (DebugMode) {
    char buf [BUFFERSIZE];
    wsprintf (prepareBuf(buf),"Refcount in %s::%s for object %s - %ld\n\r", dc, df, s, r);
    OleDbgOut (buf);
  }
  return r;
}

void cDebug::DebugExit (char * dc, char * df)
{
  if (level)
    level--;
  if (DebugMode) {
    char buf [BUFFERSIZE];
    wsprintf (prepareBuf(buf),"Exit %s:%s\n\r", dc, df);
    OleDbgOut (buf);
  }
}

HRESULT cDebug::DebugHResult (LPOLESTR s, HRESULT hrErr)
{
  if (DebugMode) {
    char buf [BUFFERSIZE];
    wsprintf (prepareBuf(buf),"%s ", s);
    OleDbgOutHResult (buf, hrErr);
  }
  return hrErr;
}

ULONG cDebug::DebugRefCnt (LPOLESTR s, ULONG r)
{
  if (DebugMode) {
    char buf [BUFFERSIZE];
    wsprintf (prepareBuf(buf),"Refcount for object %s - %ld\n\r", s, r);
    OleDbgOut (buf);
  }
  return r;
}

void cdecl cDebug::DebugPrintf (LPOLESTR s, ...)
{
  va_list paramList;
  va_start (paramList, s);
  if (DebugMode) {
    char buf1 [BUFFERSIZE];
    vsprintf (buf1, s, paramList);
    char buf [BUFFERSIZE];
    wsprintf (prepareBuf(buf), "%s\n\r", buf1);
    OleDbgOut (buf);
  }
  va_end (paramList);
}

void cDebug::DebugPrint (LPOLESTR pS, const RECT FAR * pR)
{
  if (DebugMode) {
    char buf [BUFFERSIZE];
    if (pR) {
      if (pS)
        wsprintf (prepareBuf(buf), "%s %04X %04X %04X %04X\n\r", pS, pR->left, pR->top, pR->right, pR->bottom);
      else
        wsprintf (prepareBuf(buf), "%04X %04X %04X %04X\n\r", pR->left, pR->top, pR->right, pR->bottom);
    }
    else {
      if (pS)
        wsprintf (prepareBuf(buf), "%s\n\r", pS);
      else
        wsprintf (prepareBuf(buf), "\n\r");
    }
    OleDbgOut (buf);
  }
}

void cDebug::DebugPrint (LPOLESTR pS, const SIZEL FAR * pL)
{
  if (DebugMode) {
    char buf [BUFFERSIZE];
    if (pS)
      wsprintf (prepareBuf(buf), "%s %08lX %08lX\n\r", pS, pL->cx, pL->cy);
    else
      wsprintf (prepareBuf(buf), "%08lX %08lX\n\r", pL->cx, pL->cy);
    OleDbgOut (buf);
  }
}

void cDebug::DebugPrint (LPOLESTR pS, BOOL f)
{
  if (DebugMode) {
    char buf [BUFFERSIZE];
    if (pS)
      wsprintf (prepareBuf(buf), "%s %s\n\r", pS, f ? "TRUE" : "FALSE");
    else
      wsprintf (prepareBuf(buf), "%s\n\r", f ? "TRUE" : "FALSE");
    OleDbgOut (buf);
  }
}

void cDebug::DebugPrint (REFIID riid)
{
#if 0
  if (DebugMode) {
    char buf [BUFFERSIZE];
    char * P = prepareBuf (buf);
    LPOLESTR * ppC = 0;
    if (StringFromIID(riid, ppC) == NOERROR)
      strcpy (P, *ppC);
    else if (StringFromCLSID(riid, ppC) != NOERROR)
      strcpy (P, *ppC);
    else
      wsprintf (P, "IID not recog 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,  riid.Data3);
    OleDbgOut (buf);
  }
#else
  if (DebugMode) {
    char buf [BUFFERSIZE];
   if (riid == IID_IUnknown)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IUnknown 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IClassFactory)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IClassFactory 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IMalloc)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IMalloc 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IMarshal)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IMarshal 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IRpcChannel)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IRpcChannel 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IRpcStub)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IRpcStub 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IStubManager)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IStubManager 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IRpcProxy)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IRpcProxy 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IProxyManager)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IProxyManager 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IPSFactory)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IPSFactory 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_ILockBytes)
   {
      wsprintf (prepareBuf(buf), "IID = IID_ILockBytes 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IStorage)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IStorage 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IStream)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IStream 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IEnumSTATSTG)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IEnumSTATSTG 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IBindCtx)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IBindCtx 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IMoniker)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IMoniker 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IRunningObjectTable)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IRunningObjectTable 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IInternalMoniker)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IInternalMoniker 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IRootStorage)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IRootStorage 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IDfReserved1)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IDfReserved1 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IDfReserved2)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IDfReserved2 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IDfReserved3)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IDfReserved3 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IMessageFilter)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IMessageFilter 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
#ifdef  INCLUDE_IDISPATCH
   if (riid == IID_IDispatch)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IDispatch 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_ITypeInfo)
   {
      wsprintf (prepareBuf(buf), "IID = IID_ITypeInfo 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_ITypeLib)
   {
      wsprintf (prepareBuf(buf), "IID = IID_ITypeLib 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_ITypeComp)
   {
      wsprintf (prepareBuf(buf), "IID = IID_ITypeComp 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IEnumVARIANT)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IEnumVARIANT 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
#endif  // INCLUDE_IDISPATCH
   if (riid == IID_IEnumUnknown)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IEnumUnknown 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IEnumString)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IEnumString 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IEnumMoniker)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IEnumMoniker 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IEnumFORMATETC)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IEnumFORMATETC 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IEnumOLEVERB)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IEnumOLEVERB 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IEnumSTATDATA)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IEnumSTATDATA 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IEnumGeneric)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IEnumGeneric 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IEnumHolder)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IEnumHolder 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IEnumCallback)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IEnumCallback 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IPersistStream)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IPersistStream 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IPersistStorage)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IPersistStorage 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IPersistFile)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IPersistFile 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IPersist)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IPersist 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IViewObject)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IViewObject 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IDataObject)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IDataObject 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IAdviseSink)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IAdviseSink 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IDataAdviseHolder)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IDataAdviseHolder 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IOleAdviseHolder)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IOleAdviseHolder 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IOleObject)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IOleObject 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IOleInPlaceObject)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IOleInPlaceObject 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IOleWindow)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IOleWindow 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IOleInPlaceUIWindow)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IOleInPlaceUIWindow 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IOleInPlaceFrame)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IOleInPlaceFrame 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IOleInPlaceActiveObject)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IOleInPlaceActiveObject 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IOleClientSite)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IOleClientSite 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IOleInPlaceSite)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IOleInPlaceSite 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IParseDisplayName)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IParseDisplayName 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IOleContainer)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IOleContainer 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IOleItemContainer)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IOleItemContainer 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IOleLink)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IOleLink 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IOleCache)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IOleCache 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IOleManager)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IOleManager 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IOlePresObj)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IOlePresObj 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IDropSource)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IDropSource 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IDropTarget)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IDropTarget 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IDebug)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IDebug 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }
   if (riid == IID_IDebugStream)
   {
      wsprintf (prepareBuf(buf), "IID = IID_IDebugStream 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
               riid.Data3);
      goto ll_return;
   }

   wsprintf (prepareBuf(buf), "   IID not recog 0x%lx.%x.%x\n\r", riid.Data1, riid.Data2,
            riid.Data3);

ll_return:
    OleDbgOut (buf);
  }
#endif
}

#define CASE_SCODE(sc)  \
    case sc: \
      lstrcpy((LPSTR)szErrName, (LPSTR)#sc); \
      break;

STDAPI_(void) OleDbgPrintScodeAlways(LPSTR lpszPrefix, LPSTR lpszMsg, SCODE sc)
{
  char szBuf[256];
  char szErrName[40];

  switch (sc) {

    /* SCODE's defined in SCODE.H */

    CASE_SCODE(S_OK)
    CASE_SCODE(S_FALSE)
    CASE_SCODE(E_UNEXPECTED)
    CASE_SCODE(E_OUTOFMEMORY)
    CASE_SCODE(E_INVALIDARG)
    CASE_SCODE(E_NOINTERFACE)
    CASE_SCODE(E_POINTER)
    CASE_SCODE(E_HANDLE)
    CASE_SCODE(E_ABORT)
    CASE_SCODE(E_FAIL)
    CASE_SCODE(E_ACCESSDENIED)

    /* SCODE's defined in OLE2.H */

    CASE_SCODE(OLE_E_OLEVERB)
    CASE_SCODE(OLE_E_ADVF)
    CASE_SCODE(OLE_E_ENUM_NOMORE)
    CASE_SCODE(OLE_E_ADVISENOTSUPPORTED)
    CASE_SCODE(OLE_E_NOCONNECTION)
    CASE_SCODE(OLE_E_NOTRUNNING)
    CASE_SCODE(OLE_E_NOCACHE)
    CASE_SCODE(OLE_E_BLANK)
    CASE_SCODE(OLE_E_CLASSDIFF)
    CASE_SCODE(OLE_E_CANT_GETMONIKER)
    CASE_SCODE(OLE_E_CANT_BINDTOSOURCE)
    CASE_SCODE(OLE_E_STATIC)
    CASE_SCODE(OLE_E_PROMPTSAVECANCELLED)
    CASE_SCODE(OLE_E_INVALIDRECT)
    CASE_SCODE(OLE_E_WRONGCOMPOBJ)
    CASE_SCODE(OLE_E_INVALIDHWND)
    CASE_SCODE(OLE_E_NOT_INPLACEACTIVE)
    CASE_SCODE(OLE_E_CANTCONVERT)
    CASE_SCODE(OLE_E_NOSTORAGE)

    CASE_SCODE(DV_E_FORMATETC)
    CASE_SCODE(DV_E_DVTARGETDEVICE)
    CASE_SCODE(DV_E_STGMEDIUM)
    CASE_SCODE(DV_E_STATDATA)
    CASE_SCODE(DV_E_LINDEX)
    CASE_SCODE(DV_E_TYMED)
    CASE_SCODE(DV_E_CLIPFORMAT)
    CASE_SCODE(DV_E_DVASPECT)
    CASE_SCODE(DV_E_DVTARGETDEVICE_SIZE)
    CASE_SCODE(DV_E_NOIVIEWOBJECT)

    CASE_SCODE(OLE_S_USEREG)
    CASE_SCODE(OLE_S_STATIC)
    CASE_SCODE(OLE_S_MAC_CLIPFORMAT)

    CASE_SCODE(CONVERT10_E_OLESTREAM_GET)
    CASE_SCODE(CONVERT10_E_OLESTREAM_PUT)
    CASE_SCODE(CONVERT10_E_OLESTREAM_FMT)
    CASE_SCODE(CONVERT10_E_OLESTREAM_BITMAP_TO_DIB)
    CASE_SCODE(CONVERT10_E_STG_FMT)
    CASE_SCODE(CONVERT10_E_STG_NO_STD_STREAM)
    CASE_SCODE(CONVERT10_E_STG_DIB_TO_BITMAP)
    CASE_SCODE(CONVERT10_S_NO_PRESENTATION)

    CASE_SCODE(CLIPBRD_E_CANT_OPEN)
    CASE_SCODE(CLIPBRD_E_CANT_EMPTY)
    CASE_SCODE(CLIPBRD_E_CANT_SET)
    CASE_SCODE(CLIPBRD_E_BAD_DATA)
    CASE_SCODE(CLIPBRD_E_CANT_CLOSE)

    CASE_SCODE(DRAGDROP_E_NOTREGISTERED)
    CASE_SCODE(DRAGDROP_E_ALREADYREGISTERED)
    CASE_SCODE(DRAGDROP_E_INVALIDHWND)
    CASE_SCODE(DRAGDROP_S_DROP)
    CASE_SCODE(DRAGDROP_S_CANCEL)
    CASE_SCODE(DRAGDROP_S_USEDEFAULTCURSORS)

    CASE_SCODE(OLEOBJ_E_NOVERBS)
    CASE_SCODE(OLEOBJ_E_INVALIDVERB)
    CASE_SCODE(OLEOBJ_S_INVALIDVERB)
    CASE_SCODE(OLEOBJ_S_CANNOT_DOVERB_NOW)
    CASE_SCODE(OLEOBJ_S_INVALIDHWND)
    CASE_SCODE(INPLACE_E_NOTUNDOABLE)
    CASE_SCODE(INPLACE_E_NOTOOLSPACE)
    CASE_SCODE(INPLACE_S_TRUNCATED)

    /* SCODE's defined in COMPOBJ.H */

    CASE_SCODE(CO_E_NOTINITIALIZED)
    CASE_SCODE(CO_E_ALREADYINITIALIZED)
    CASE_SCODE(CO_E_CANTDETERMINECLASS)
    CASE_SCODE(CO_E_CLASSSTRING)
    CASE_SCODE(CO_E_IIDSTRING)
    CASE_SCODE(CO_E_APPNOTFOUND)
    CASE_SCODE(CO_E_APPSINGLEUSE)
    CASE_SCODE(CO_E_ERRORINAPP)
    CASE_SCODE(CO_E_DLLNOTFOUND)
    CASE_SCODE(CO_E_ERRORINDLL)
    CASE_SCODE(CO_E_WRONGOSFORAPP)
    CASE_SCODE(CO_E_OBJNOTREG)
    CASE_SCODE(CO_E_OBJISREG)
    CASE_SCODE(CO_E_OBJNOTCONNECTED)
    CASE_SCODE(CO_E_APPDIDNTREG)
    CASE_SCODE(CLASS_E_NOAGGREGATION)
    CASE_SCODE(CLASS_E_CLASSNOTAVAILABLE)
    CASE_SCODE(REGDB_E_READREGDB)
    CASE_SCODE(REGDB_E_WRITEREGDB)
    CASE_SCODE(REGDB_E_KEYMISSING)
    CASE_SCODE(REGDB_E_INVALIDVALUE)
    CASE_SCODE(REGDB_E_CLASSNOTREG)
    CASE_SCODE(REGDB_E_IIDNOTREG)
    CASE_SCODE(RPC_E_CALL_REJECTED)
    CASE_SCODE(RPC_E_CALL_CANCELED)
    CASE_SCODE(RPC_E_CANTPOST_INSENDCALL)
    CASE_SCODE(RPC_E_CANTCALLOUT_INASYNCCALL)
    CASE_SCODE(RPC_E_CANTCALLOUT_INEXTERNALCALL)
    CASE_SCODE(RPC_E_CONNECTION_TERMINATED)
    CASE_SCODE(RPC_E_SERVER_DIED)
    CASE_SCODE(RPC_E_CLIENT_DIED)
    CASE_SCODE(RPC_E_INVALID_DATAPACKET)
    CASE_SCODE(RPC_E_CANTTRANSMIT_CALL)
    CASE_SCODE(RPC_E_CLIENT_CANTMARSHAL_DATA)
    CASE_SCODE(RPC_E_CLIENT_CANTUNMARSHAL_DATA)
    CASE_SCODE(RPC_E_SERVER_CANTMARSHAL_DATA)
    CASE_SCODE(RPC_E_SERVER_CANTUNMARSHAL_DATA)
    CASE_SCODE(RPC_E_INVALID_DATA)
    CASE_SCODE(RPC_E_INVALID_PARAMETER)
    CASE_SCODE(RPC_E_CANTCALLOUT_AGAIN)
    CASE_SCODE(RPC_E_UNEXPECTED)

    /* SCODE's defined in DVOBJ.H */

    CASE_SCODE(DATA_S_SAMEFORMATETC)
    CASE_SCODE(VIEW_E_DRAW)
    CASE_SCODE(VIEW_S_ALREADY_FROZEN)
    CASE_SCODE(CACHE_E_NOCACHE_UPDATED)
    CASE_SCODE(CACHE_S_FORMATETC_NOTSUPPORTED)
    CASE_SCODE(CACHE_S_SAMECACHE)
    CASE_SCODE(CACHE_S_SOMECACHES_NOTUPDATED)

    /* SCODE's defined in STORAGE.H */

    CASE_SCODE(STG_E_INVALIDFUNCTION)
    CASE_SCODE(STG_E_FILENOTFOUND)
    CASE_SCODE(STG_E_PATHNOTFOUND)
    CASE_SCODE(STG_E_TOOMANYOPENFILES)
    CASE_SCODE(STG_E_ACCESSDENIED)
    CASE_SCODE(STG_E_INVALIDHANDLE)
    CASE_SCODE(STG_E_INSUFFICIENTMEMORY)
    CASE_SCODE(STG_E_INVALIDPOINTER)
    CASE_SCODE(STG_E_NOMOREFILES)
    CASE_SCODE(STG_E_DISKISWRITEPROTECTED)
    CASE_SCODE(STG_E_SEEKERROR)
    CASE_SCODE(STG_E_WRITEFAULT)
    CASE_SCODE(STG_E_READFAULT)
    CASE_SCODE(STG_E_SHAREVIOLATION)
    CASE_SCODE(STG_E_LOCKVIOLATION)
    CASE_SCODE(STG_E_FILEALREADYEXISTS)
    CASE_SCODE(STG_E_INVALIDPARAMETER)
    CASE_SCODE(STG_E_MEDIUMFULL)
    CASE_SCODE(STG_E_ABNORMALAPIEXIT)
    CASE_SCODE(STG_E_INVALIDHEADER)
    CASE_SCODE(STG_E_INVALIDNAME)
    CASE_SCODE(STG_E_UNKNOWN)
    CASE_SCODE(STG_E_UNIMPLEMENTEDFUNCTION)
    CASE_SCODE(STG_E_INVALIDFLAG)
    CASE_SCODE(STG_E_INUSE)
    CASE_SCODE(STG_E_NOTCURRENT)
    CASE_SCODE(STG_E_REVERTED)
    CASE_SCODE(STG_E_CANTSAVE)
    CASE_SCODE(STG_E_OLDFORMAT)
    CASE_SCODE(STG_E_OLDDLL)
    CASE_SCODE(STG_E_SHAREREQUIRED)
    CASE_SCODE(STG_E_NOTFILEBASEDSTORAGE)
    CASE_SCODE(STG_E_EXTANTMARSHALLINGS)
    CASE_SCODE(STG_S_CONVERTED)

    /* SCODE's defined in STORAGE.H */

    CASE_SCODE(MK_E_CONNECTMANUALLY)
    CASE_SCODE(MK_E_EXCEEDEDDEADLINE)
    CASE_SCODE(MK_E_NEEDGENERIC)
    CASE_SCODE(MK_E_UNAVAILABLE)
    CASE_SCODE(MK_E_SYNTAX)
    CASE_SCODE(MK_E_NOOBJECT)
    CASE_SCODE(MK_E_INVALIDEXTENSION)
    CASE_SCODE(MK_E_INTERMEDIATEINTERFACENOTSUPPORTED)
    CASE_SCODE(MK_E_NOTBINDABLE)
    CASE_SCODE(MK_E_NOTBOUND)
    CASE_SCODE(MK_E_CANTOPENFILE)
    CASE_SCODE(MK_E_MUSTBOTHERUSER)
    CASE_SCODE(MK_E_NOINVERSE)
    CASE_SCODE(MK_E_NOSTORAGE)
    CASE_SCODE(MK_E_NOPREFIX)
    CASE_SCODE(MK_S_REDUCED_TO_SELF)
    CASE_SCODE(MK_S_ME)
    CASE_SCODE(MK_S_HIM)
    CASE_SCODE(MK_S_US)
    CASE_SCODE(MK_S_MONIKERALREADYREGISTERED)

    default:
      lstrcpy(szErrName, "UNKNOWN SCODE");
  }

  wsprintf(szBuf, "%s %s (0x%lx)\n", lpszMsg, (LPSTR)szErrName, sc);
  OleDbgPrintAlways(lpszPrefix, szBuf, 0);
}

char cDebugEnum::enumBuf [200];

void cDebugEnum::Print (DWORD e)
{
  for (WORD w = 0; w < count; w++)
    if (pEnumRecs [w].eValue == (WORD)e) {
      wsprintf (enumBuf, "%s:%s", pEnumName, pEnumRecs [w].pEnumS);
      return;
    }
  wsprintf (enumBuf, "%s:%s", pEnumName, "**INVALID**");
}

cDebugEnumRecord rOLERENDER [] =
{
  {"OLERENDER_NONE", OLERENDER_NONE},
  {"OLERENDER_DRAW", OLERENDER_DRAW},
  {"OLERENDER_FORMAT", OLERENDER_FORMAT},
  {"OLERENDER_ASIS", OLERENDER_ASIS}
};

cDebugEnum cOLERENDER ("OLERENDER", rOLERENDER, sizeof (rOLERENDER) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rOLEVERBATTRIB [] =
{
  {"OLEVERBATTRIB_NEVERDIRTIES", OLEVERBATTRIB_NEVERDIRTIES},
  {"OLEVERBATTRIB_ONCONTAINERMENU", OLEVERBATTRIB_ONCONTAINERMENU}
};

cDebugEnum cOLEVERBATTRIB ("OLEVERBATTRIB", rOLEVERBATTRIB, sizeof (rOLEVERBATTRIB) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rUSERCLASSTYPE [] =
{
  {"USERCLASSTYPE_FULL", USERCLASSTYPE_FULL},
  {"USERCLASSTYPE_SHORT", USERCLASSTYPE_SHORT},
  {"USERCLASSTYPE_APPNAME", USERCLASSTYPE_APPNAME},
};

cDebugEnum cUSERCLASSTYPE ("USERCLASSTYPE", rUSERCLASSTYPE, sizeof (rUSERCLASSTYPE) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rOLEMISC [] =
{
  {"OLEMISC_RECOMPOSEONRESIZE", OLEMISC_RECOMPOSEONRESIZE},
  {"OLEMISC_ONLYICONIC", OLEMISC_ONLYICONIC},
  {"OLEMISC_INSERTNOTREPLACE", OLEMISC_INSERTNOTREPLACE},
  {"OLEMISC_STATIC", OLEMISC_STATIC},
  {"OLEMISC_CANTLINKINSIDE", OLEMISC_CANTLINKINSIDE},
  {"OLEMISC_CANLINKBYOLE1", OLEMISC_CANLINKBYOLE1},
  {"OLEMISC_ISLINKOBJECT", OLEMISC_ISLINKOBJECT},
  {"OLEMISC_INSIDEOUT", OLEMISC_INSIDEOUT},
  {"OLEMISC_ACTIVATEWHENVISIBLE", OLEMISC_ACTIVATEWHENVISIBLE}
};

cDebugEnum cOLEMISC ("OLEMISC", rOLEMISC, sizeof (rOLEMISC) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rOLECLOSE [] =
{
  {"OLECLOSE_SAVEIFDIRTY", OLECLOSE_SAVEIFDIRTY},
  {"OLECLOSE_NOSAVE", OLECLOSE_NOSAVE},
  {"OLECLOSE_PROMPTSAVE", OLECLOSE_PROMPTSAVE}
};

cDebugEnum cOLECLOSE ("OLECLOSE", rOLECLOSE, sizeof (rOLECLOSE) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rOLEGETMONIKER [] =
{
  {"OLEGETMONIKER_ONLYIFTHERE", OLEGETMONIKER_ONLYIFTHERE},
  {"OLEGETMONIKER_FORCEASSIGN", OLEGETMONIKER_FORCEASSIGN},
  {"OLEGETMONIKER_UNASSIGN", OLEGETMONIKER_UNASSIGN},
  {"OLEGETMONIKER_TEMPFORUSER", OLEGETMONIKER_TEMPFORUSER}
};

cDebugEnum cOLEGETMONIKER ("OLEGETMONIKER", rOLEGETMONIKER, sizeof (rOLEGETMONIKER) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rOLEWHICHMK [] =
{
  {"OLEWHICHMK_CONTAINER", OLEWHICHMK_CONTAINER},
  {"OLEWHICHMK_OBJREL", OLEWHICHMK_OBJREL},
  {"OLEWHICHMK_OBJFULL", OLEWHICHMK_OBJFULL}
};

cDebugEnum cOLEWHICHMK ("OLEWHICHMK", rOLEWHICHMK, sizeof (rOLEWHICHMK) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rBINDSPEED [] =
{
  {"BINDSPEED_INDEFINITE", BINDSPEED_INDEFINITE},
  {"BINDSPEED_MODERATE", BINDSPEED_MODERATE},
  {"BINDSPEED_IMMEDIATE", BINDSPEED_IMMEDIATE}
};

cDebugEnum cBINDSPEED ("BINDSPEED", rBINDSPEED, sizeof (rBINDSPEED) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rOLECONTF [] =
{
  {"OLECONTF_EMBEDDINGS", OLECONTF_EMBEDDINGS},
  {"OLECONTF_LINKS", OLECONTF_LINKS},
  {"OLECONTF_OTHERS", OLECONTF_OTHERS},
  {"OLECONTF_ONLYUSER", OLECONTF_ONLYUSER},
  {"OLECONTF_ONLYIFRUNNING", OLECONTF_ONLYIFRUNNING}
};

cDebugEnum cOLECONTF ("OLECONTF", rOLECONTF, sizeof (rOLECONTF) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rOLEUPDATE [] =
{
  {"OLEUPDATE_ALWAYS", OLEUPDATE_ALWAYS},
  {"OLEUPDATE_ONCALL", OLEUPDATE_ONCALL}
};

cDebugEnum cOLEUPDATE ("OLEUPDATE", rOLEUPDATE, sizeof (rOLEUPDATE) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rOLELINKBIND [] =
{
  {"OLELINKBIND_EVENIFCLASSDIFF", OLELINKBIND_EVENIFCLASSDIFF},
};

cDebugEnum cOLELINKBIND ("OLELINKBIND", rOLELINKBIND, sizeof (rOLELINKBIND) / sizeof (cDebugEnumRecord));


cDebugEnumRecord rDVASPECT [] =
{
  {"DVASPECT_CONTENT", DVASPECT_CONTENT},
  {"DVASPECT_THUMBNAIL", DVASPECT_THUMBNAIL},
  {"DVASPECT_ICON", DVASPECT_ICON},
  {"DVASPECT_DOCPRINT", DVASPECT_DOCPRINT}
};

cDebugEnum cDVASPECT ("DVASPECT", rDVASPECT, sizeof (rDVASPECT) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rTYMED [] =
{
  {"TYMED_HGLOBAL", TYMED_HGLOBAL},
  {"TYMED_FILE", TYMED_FILE},
  {"TYMED_ISTREAM", TYMED_ISTREAM},
  {"TYMED_ISTORAGE", TYMED_ISTORAGE},
  {"TYMED_GDI", TYMED_GDI},
  {"TYMED_MFPICT", TYMED_MFPICT},
  {"TYMED_NULL", TYMED_NULL}
};

cDebugEnum cTYMED ("TYMED", rTYMED, sizeof (rTYMED) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rDATADIR [] =
{
  {"DATADIR_GET", DATADIR_GET},
  {"DATADIR_SET", DATADIR_SET},
};

cDebugEnum cDATADIR ("DATADIR", rDATADIR, sizeof (rDATADIR) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rADVF [] =
{
  {"ADVF_NODATA", ADVF_NODATA},
  {"ADVF_PRIMEFIRST", ADVF_PRIMEFIRST},
  {"ADVF_ONLYONCE", ADVF_ONLYONCE},
  {"ADVF_DATAONSTOP", ADVF_DATAONSTOP},
  {"ADVFCACHE_NOHANDLER", ADVFCACHE_NOHANDLER},
  {"ADVFCACHE_FORCEBUILTIN", ADVFCACHE_FORCEBUILTIN},
  {"ADVFCACHE_ONSAVE", ADVFCACHE_ONSAVE}
};

cDebugEnum cADVF ("ADVF", rADVF, sizeof (rADVF) / sizeof (cDebugEnumRecord));


cDebugEnumRecord rMEMCTX [] =
{
  {"MEMCTX_TASK", MEMCTX_TASK},
  {"MEMCTX_SHARED", MEMCTX_SHARED},
#ifdef _MAC
   {"MEMCTX_MACSYSTEM", MEMCTX_MACSYSTEM},
#endif
  {"MEMCTX_UNKNOWN", MEMCTX_UNKNOWN},
  {"MEMCTX_SAME", MEMCTX_SAME},
};

cDebugEnum cMEMCTX ("MEMCTX", rMEMCTX, sizeof (rMEMCTX) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rCLSCTX [] =
{
  {"CLSCTX_INPROC_SERVER", CLSCTX_INPROC_SERVER},
  {"CLSCTX_INPROC_HANDLER", CLSCTX_INPROC_HANDLER},
  {"CLSCTX_LOCAL_SERVER", CLSCTX_LOCAL_SERVER}
};

cDebugEnum cCLSCTX ("CLSCTX", rCLSCTX, sizeof (rCLSCTX) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rREGCLS [] =
{
  {"REGCLS_SINGLEUSE", REGCLS_SINGLEUSE},
  {"REGCLS_MULTIPLEUSE", REGCLS_MULTIPLEUSE}
};

cDebugEnum cREGCLS ("REGCLS", rREGCLS, sizeof (rREGCLS) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rMSHLFLAGS [] =
{
  {"MSHLFLAGS_NORMAL", MSHLFLAGS_NORMAL},
  {"MSHLFLAGS_TABLESTRONG", MSHLFLAGS_TABLESTRONG},
  {"MSHLFLAGS_TABLEWEAK", MSHLFLAGS_TABLEWEAK}
};

cDebugEnum cMSHLFLAGS ("MSHLFLAGS", rMSHLFLAGS, sizeof (rMSHLFLAGS) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rMSHCTX [] =
{
  {"MSHCTX_LOCAL", MSHCTX_LOCAL},
  {"MSHCTX_NOSHAREDMEM", MSHCTX_NOSHAREDMEM},
};

cDebugEnum cMSHCTX ("MSHCTX", rMSHCTX, sizeof (rMSHCTX) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rCALLTYPE [] =
{
  {"CALLTYPE_TOPLEVEL", CALLTYPE_TOPLEVEL},
  {"CALLTYPE_NESTED", CALLTYPE_NESTED},
  {"CALLTYPE_ASYNC", CALLTYPE_ASYNC},
  {"CALLTYPE_TOPLEVEL_CALLPENDING", CALLTYPE_TOPLEVEL_CALLPENDING},
  {"CALLTYPE_ASYNC_CALLPENDING", CALLTYPE_ASYNC_CALLPENDING}
};

cDebugEnum cCALLTYPE ("CALLTYPE", rCALLTYPE, sizeof (rCALLTYPE) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rSERVERCALL [] =
{
  {"SERVERCALL_ISHANDLED", SERVERCALL_ISHANDLED},
  {"SERVERCALL_REJECTED", SERVERCALL_REJECTED},
  {"SERVERCALL_RETRYLATER", SERVERCALL_RETRYLATER}
};

cDebugEnum cSERVERCALL ("SERVERCALL", rSERVERCALL, sizeof (rSERVERCALL) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rPENDINGTYPE [] =
{
  {"PENDINGTYPE_TOPLEVEL", PENDINGTYPE_TOPLEVEL},
  {"PENDINGTYPE_NESTED", PENDINGTYPE_NESTED}
};

cDebugEnum cPENDINGTYPE ("PENDINGTYPE", rPENDINGTYPE, sizeof (rPENDINGTYPE) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rPENDINGMSG [] =
{
  {"PENDINGMSG_CANCELCALL", PENDINGMSG_CANCELCALL},
  {"PENDINGMSG_WAITNOPROCESS", PENDINGMSG_WAITNOPROCESS},
  {"PENDINGMSG_WAITDEFPROCESS", PENDINGMSG_WAITDEFPROCESS}

};

cDebugEnum cPENDINGMSG ("PENDINGMSG", rPENDINGMSG, sizeof (rPENDINGMSG) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rBIND_FLAGS [] =
{
  {"BIND_MAYBOTHERUSER", BIND_MAYBOTHERUSER},
  {"BIND_JUSTTESTEXISTENCE", BIND_JUSTTESTEXISTENCE}
};

cDebugEnum cBIND_FLAGS ("BIND_FLAGS", rBIND_FLAGS, sizeof (rBIND_FLAGS) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rKSYS [] =
{
  {"MKSYS_NONE", MKSYS_NONE},
  {"MKSYS_GENERICCOMPOSITE", MKSYS_GENERICCOMPOSITE},
  {"MKSYS_FILEMONIKER", MKSYS_FILEMONIKER},
  {"MKSYS_ANTIMONIKER", MKSYS_ANTIMONIKER},
  {"MKSYS_ITEMMONIKER", MKSYS_ITEMMONIKER},
  {"MKSYS_POINTERMONIKER", MKSYS_POINTERMONIKER}
};

cDebugEnum cKSYS ("KSYS", rKSYS, sizeof (rKSYS) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rMKRREDUCE [] =
{
  {"MKRREDUCE_ONE", MKRREDUCE_ONE},
  {"MKRREDUCE_TOUSER", MKRREDUCE_TOUSER},
  {"MKRREDUCE_THROUGHUSER", MKRREDUCE_THROUGHUSER},
  {"MKRREDUCE_ALL", MKRREDUCE_ALL}
};

cDebugEnum cMKRREDUCE ("MKRREDUCE", rMKRREDUCE, sizeof (rMKRREDUCE) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rSTGC [] =
{
  {"STGC_DEFAULT", STGC_DEFAULT},
  {"STGC_OVERWRITE", STGC_OVERWRITE},
  {"STGC_ONLYIFCURRENT", STGC_ONLYIFCURRENT},
  {"STGC_DANGEROUSLYCOMMITMERELYTODISKCACHE", STGC_DANGEROUSLYCOMMITMERELYTODISKCACHE}
};

cDebugEnum cSTGC ("STGC", rSTGC, sizeof (rSTGC) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rSTGTY [] =
{
  {"STGTY_STORAGE", STGTY_STORAGE},
  {"STGTY_STREAM", STGTY_STREAM},
  {"STGTY_LOCKBYTES", STGTY_LOCKBYTES},
  {"STGTY_PROPERTY", STGTY_PROPERTY}
};

cDebugEnum cSTGTY ("STGTY", rSTGTY, sizeof (rSTGTY) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rSTREAM_SEEK [] =
{
  {"STREAM_SEEK_SET", STREAM_SEEK_SET},
  {"STREAM_SEEK_CUR", STREAM_SEEK_CUR},
  {"STREAM_SEEK_END", STREAM_SEEK_END}
};

cDebugEnum cSTREAM_SEEK ("STREAM_SEEK", rSTREAM_SEEK, sizeof (rSTREAM_SEEK) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rLOCKTYPE [] =
{
  {"LOCK_WRITE", LOCK_WRITE},
  {"LOCK_EXCLUSIVE", LOCK_EXCLUSIVE},
  {"LOCK_ONLYONCE", LOCK_ONLYONCE}
};

cDebugEnum cLOCKTYPE ("LOCKTYPE", rLOCKTYPE, sizeof (rLOCKTYPE) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rSTGMOVE [] =
{
  {"STGMOVE_MOVE", STGMOVE_MOVE},
  {"STGMOVE_COPY", STGMOVE_COPY}
};

cDebugEnum cSTGMOVE ("STGMOVE", rSTGMOVE, sizeof (rSTGMOVE) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rSTATFLAG [] =
{
  {"STATFLAG_DEFAULT", STATFLAG_DEFAULT},
  {"STATFLAG_NONAME", STATFLAG_NONAME}
};

cDebugEnum cSTATFLAG ("STATFLAG", rSTATFLAG, sizeof (rSTATFLAG) / sizeof (cDebugEnumRecord));

cDebugEnumRecord rOLEUIPASTEFLAG [] =
{
   {"OLEUIPASTE_PASTEONLY", OLEUIPASTE_PASTEONLY},
   {"OLEUIPASTE_PASTE", OLEUIPASTE_PASTE},
   {"OLEUIPASTE_LINKANYTYPE", OLEUIPASTE_LINKANYTYPE},
   {"OLEUIPASTE_LINKTYPE1", OLEUIPASTE_LINKTYPE1},
   {"OLEUIPASTE_LINKTYPE2", OLEUIPASTE_LINKTYPE2},
   {"OLEUIPASTE_LINKTYPE3", OLEUIPASTE_LINKTYPE3},
   {"OLEUIPASTE_LINKTYPE4", OLEUIPASTE_LINKTYPE4},
   {"OLEUIPASTE_LINKTYPE5", OLEUIPASTE_LINKTYPE5},
   {"OLEUIPASTE_LINKTYPE6", OLEUIPASTE_LINKTYPE6},
   {"OLEUIPASTE_LINKTYPE7", OLEUIPASTE_LINKTYPE7},
   {"OLEUIPASTE_LINKTYPE8", OLEUIPASTE_LINKTYPE8}
};

cDebugEnum cOLEUIPASTEFLAG ("OLEUIPASTEFLAG", rOLEUIPASTEFLAG, sizeof (rOLEUIPASTEFLAG) / sizeof (cDebugEnumRecord));

#endif

#ifdef  _DEBUG
//**************************************************************************
//
// IUnknownWatcher
//
//**************************************************************************

IUnknownWatcher::IUnknownWatcher( IUnknown *punk, char *name ) : pUnk( punk ){
  strncpy( Name, name, sizeof( Name ) );
  char tmp[80];
  wsprintf( tmp, "%s %04x:%04x created, nRef=%d\n\r", Name, HIWORD((DWORD)pUnk),LOWORD((DWORD)pUnk), nRef() );
  OutputDebugString( tmp );
}
HRESULT _IFUNC IUnknownWatcher::QueryInterface(REFIID iid, void FAR* FAR* pif){
  HRESULT hr;
  if( iid == IID_IUnknown ){
    *pif = this;
    pUnk->AddRef();
    hr = ResultFromScode(S_OK);
  } else
    hr = pUnk->QueryInterface( iid, pif );
  if( hr == ResultFromScode(S_OK) ){
    char tmp[80];
    wsprintf( tmp, "%s %04x:%04x QI OK, nRef=%d\n\r", Name, HIWORD((DWORD)pUnk),LOWORD((DWORD)pUnk), nRef() );
    OutputDebugString( tmp );
  }
  return hr;
}
ULONG _IFUNC IUnknownWatcher::AddRef(){
  int i = pUnk->AddRef();
  char tmp[80];
  wsprintf( tmp, "%s %04x:%04x AddRef to %d\n\r", Name, HIWORD((DWORD)pUnk),LOWORD((DWORD)pUnk), i );
  OutputDebugString( tmp );
  return i;
}
ULONG _IFUNC IUnknownWatcher::Release(){
  int i = pUnk->Release();
  char tmp[80];
  wsprintf( tmp, "%s %04x:%04x Released to %d\n\r", Name, HIWORD((DWORD)pUnk),LOWORD((DWORD)pUnk), i );
  OutputDebugString( tmp );
  if( i-1 == 0 ){
    wsprintf( tmp, "%s %04x:%04x DELETED\n\r", Name, HIWORD((DWORD)pUnk),LOWORD((DWORD)pUnk) );
    OutputDebugString( tmp );
    delete this;
    return 0;
  }
  return i;
}
int IUnknownWatcher::nRef(){
  int i = pUnk->AddRef();
  pUnk->Release();
  return i;
}

#endif

#ifdef  _DEBUG
//**************************************************************************
//
// IMonikerWatcher
//
//**************************************************************************

IMonikerWatcher::IMonikerWatcher( IMoniker *pmoniker, char *name ) : pMoniker( pmoniker ){
  strncpy( Name, name, sizeof( Name ) );
  char tmp[80];
  int i = pMoniker->AddRef();
  pMoniker->Release();
  wsprintf( tmp, "%s %04x:%04x created, nRef=%d\n\r",
    Name, HIWORD((DWORD)pMoniker),LOWORD((DWORD)pMoniker), i-1 );
  OutputDebugString( tmp );
}

// *** IUnknown methods ***
HRESULT _IFUNC
IMonikerWatcher::QueryInterface(REFIID riid, LPVOID FAR* ppvObj) {
  HRESULT hr = ResultFromScode(S_OK);
  int i;
  if( riid == IID_IUnknown ){
    *ppvObj = (IUnknown * )this;
    i = pMoniker->AddRef();
  } else
  if( riid == IID_IMoniker ){
    *ppvObj = (IMoniker * )this;
    i = pMoniker->AddRef();
  } else
    hr = pMoniker->QueryInterface( riid, ppvObj );
  if( hr == ResultFromScode(S_OK) ){
    char tmp[80];
    i = pMoniker->AddRef();
    pMoniker->Release();
    i--;
  wsprintf( tmp, "%s %04x:%04x QI-OK, nRef=%d\n\r",
    Name, HIWORD((DWORD)pMoniker),LOWORD((DWORD)pMoniker), i );
    OutputDebugString( tmp );
  }
  return hr;
}

ULONG _IFUNC
IMonikerWatcher::AddRef(){
  int i = pMoniker->AddRef();
  char tmp[80];
  wsprintf( tmp, "%s %04x:%04x AddRef, nRef=%d\n\r",
    Name, HIWORD((DWORD)pMoniker),LOWORD((DWORD)pMoniker), i );
  OutputDebugString( tmp );
  return i;
}

ULONG _IFUNC
IMonikerWatcher::Release() {
  int i = pMoniker->Release();
  char tmp[80];
  wsprintf( tmp, "%s %04x:%04x Release, nRef=%d\n\r",
    Name, HIWORD((DWORD)pMoniker),LOWORD((DWORD)pMoniker), i );
  OutputDebugString( tmp );
  if( i == 0 ){
    wsprintf( tmp, "%s %04x:%04x DELETED\n\r",
      Name, HIWORD((DWORD)pMoniker),LOWORD((DWORD)pMoniker) );
    OutputDebugString( tmp );
    delete this;
    return 0;
  }
  return i;
}


// *** IPersist methods ***
HRESULT _IFUNC
IMonikerWatcher::GetClassID(LPCLSID lpClassID) {
  HRESULT hr;
  hr = pMoniker->GetClassID(lpClassID) ;
  return hr;
}


// *** IPersistStream methods ***
HRESULT _IFUNC
IMonikerWatcher::IsDirty() {
  HRESULT hr;
  hr = pMoniker->IsDirty() ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::Load(LPSTREAM pStm) {
  HRESULT hr;
  hr = pMoniker->Load(pStm) ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::Save(LPSTREAM pStm, BOOL fClearDirty) {
  HRESULT hr;
  hr = pMoniker->Save(pStm, fClearDirty) ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::GetSizeMax(ULARGE_INTEGER FAR * pcbSize) {
  HRESULT hr;
  hr = pMoniker->GetSizeMax(pcbSize) ;
  return hr;
}


// *** IMoniker methods ***
HRESULT _IFUNC
IMonikerWatcher::BindToObject(LPBC pbc, LPMONIKER pmkToLeft, REFIID riidResult, LPVOID FAR* ppvResult) {
  HRESULT hr;
  hr = pMoniker->BindToObject(pbc, pmkToLeft, riidResult, ppvResult) ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::BindToStorage(LPBC pbc, LPMONIKER pmkToLeft, REFIID riid, LPVOID FAR* ppvObj) {
  HRESULT hr;
  hr = pMoniker->BindToStorage(pbc, pmkToLeft, riid, ppvObj) ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::Reduce(LPBC pbc, DWORD dwReduceHowFar, LPMONIKER FAR*ppmkToLeft, LPMONIKER FAR * ppmkReduced) {
  HRESULT hr;
  hr = pMoniker->Reduce(pbc, dwReduceHowFar, ppmkToLeft, ppmkReduced) ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::ComposeWith(LPMONIKER pmkRight, BOOL fOnlyIfNotGeneric, LPMONIKER FAR* ppmkComposite) {
  HRESULT hr;
  hr = pMoniker->ComposeWith(pmkRight, fOnlyIfNotGeneric, ppmkComposite) ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::Enum(BOOL fForward, LPENUMMONIKER FAR* ppenumMoniker){
  HRESULT hr;
  hr = pMoniker->Enum(fForward, ppenumMoniker);
  return hr;
}


HRESULT _IFUNC
IMonikerWatcher::IsEqual(LPMONIKER pmkOtherMoniker) {
  HRESULT hr;
  hr = pMoniker->IsEqual(pmkOtherMoniker) ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::Hash(LPDWORD pdwHash) {
  HRESULT hr;
  hr = pMoniker->Hash(pdwHash) ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::IsRunning(LPBC pbc, LPMONIKER pmkToLeft, LPMONIKER pmkNewlyRunning) {
  HRESULT hr;
  hr = pMoniker->IsRunning(pbc, pmkToLeft, pmkNewlyRunning) ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::GetTimeOfLastChange(LPBC pbc, LPMONIKER pmkToLeft, FILETIME FAR* pfiletime) {
  HRESULT hr;
  hr = pMoniker->GetTimeOfLastChange(pbc, pmkToLeft, pfiletime) ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::Inverse(LPMONIKER FAR* ppmk) {
  HRESULT hr;
  hr = pMoniker->Inverse(ppmk) ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::CommonPrefixWith(LPMONIKER pmkOther, LPMONIKER FAR*ppmkPrefix) {
  HRESULT hr;
  hr = pMoniker->CommonPrefixWith(pmkOther, ppmkPrefix) ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::RelativePathTo(LPMONIKER pmkOther, LPMONIKER FAR*ppmkRelPath) {
  HRESULT hr;
  hr = pMoniker->RelativePathTo(pmkOther, ppmkRelPath) ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::GetDisplayName(LPBC pbc, LPMONIKER pmkToLeft, LPSTR FAR* lplpszDisplayName) {
  HRESULT hr;
  hr = pMoniker->GetDisplayName(pbc, pmkToLeft, lplpszDisplayName) ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::ParseDisplayName(LPBC pbc, LPMONIKER pmkToLeft, LPSTR lpszDisplayName, ULONG FAR* pchEaten, LPMONIKER FAR* ppmkOut) {
  HRESULT hr;
  hr = pMoniker->ParseDisplayName(pbc, pmkToLeft, lpszDisplayName, pchEaten, ppmkOut) ;
  return hr;
}

HRESULT _IFUNC
IMonikerWatcher::IsSystemMoniker(LPDWORD pdwMksys) {
  HRESULT hr;
  hr = pMoniker->IsSystemMoniker(pdwMksys) ;
  return hr;
}
#endif



#ifdef  _DEBUG
//**************************************************************************
//
// ReleaseWatcher
//
//**************************************************************************

HRESULT _IFUNC
ReleaseWatcher::QueryInterface( REFIID, LPVOID FAR *ppid ){
  *ppid = NULL;
  return ResultFromScode( E_NOINTERFACE );
}
ULONG   _IFUNC
ReleaseWatcher::AddRef(){
  return ++nRef;
}
ULONG   _IFUNC
ReleaseWatcher::Release(){
  return --nRef;
}

#endif

