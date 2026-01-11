//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Declares the application-level object for "global" stuff. You should have
//  one service object per Bolero connection.
//----------------------------------------------------------------------------
#ifndef _BOLESVC_H
#define _BOLESVC_H

#include <stdlib.h>
#include "bolecomp.h"
#include "bolesite.h"

extern "C" {
#include "ole2ui.h" // for OLEUIPASTEENTRY - also includes oledlg.h
}

class _ICLASS BOleDocument;
class _ICLASS BOleFact;

struct BOleFactNode;

class _ICLASS BOleService : public BOleComponent,
                            public IBService2,
                            public IOleInPlaceFrame
{

protected:

  IBApplication  *pApp;
  POINTL          dragLastPt;
  BOleHelp        helpMode;
  BOleFactNode   *pFirstFactNode;

  TCHAR           dynamicScopePath[_MAX_PATH];
  CLSID           dynamicScopeClsid;
  IDataObject    *pClipboardData;
  PIBWindow       pSubclassedWindow;

  HWND            helpWnd;
public:
  WNDPROC         lpfnHelpWndProc;

public:

  // BOleService methods
  //
  BOleDocument   *pActiveDoc;
  BOleSite       *pFocusedSite;

  virtual  void _IFUNC SetActiveDoc (BOleDocument FAR *pD);
  virtual  BOleDocument FAR* _IFUNC GetActiveDoc ();
  void     _IFUNC SetHelpMode (BOOL);
  IBApplication *GetApplication () {return pApp;}
  virtual void SetFocusedSite( BOleSite *pSite ){ pFocusedSite = pSite; }

  static POINT    pixPerIn;

  // Factory accessors

  HRESULT AddClassFactory(IBClass *pF);
  HRESULT RemoveClassFactory(REFCLSID cid);
  HRESULT RemoveClassFactory(LPCOLESTR szProgId);
  HRESULT FindClassFactory(REFCLSID cid, IBClass **pF);
  HRESULT FindClassFactory(LPCOLESTR szProgId, IBClass **pF);

  // For clip format negotiation in drag/drop and cut/copy
  //
  LPFORMATETC  clipList;
  UINT         clipCount;
  BOOL         clipOkToLink;
  BOOL         clipOkToEmbed;

  LPFORMATETC  dropList;
  UINT         dropCount;
  BOOL         dropOkToLink;
  BOOL         dropOkToEmbed;

  BOOL         fUseDropList;

public:

  BOleService (BOleClassManager *pF, IBUnknownMain * pOuter);
  ~BOleService ();
  HGLOBAL _IFUNC GetDataFromDataObject (LPDATAOBJECT, CLIPFORMAT,
                     DVTARGETDEVICE FAR*, DWORD, LPSTGMEDIUM);
  void  EnterBOleDialog (HWND , HHOOK *, HTASK *);
  void  ExitBOleDialog ();
  virtual UINT _IFUNC ExcludeOurselves (LPCLSID FAR*);
  void FlushClipboardData(BOOL bCheck = TRUE);
  BOOL IsOnClipboard(IDataObject *);
  BOOL _IFUNC ShowHelpButton (BOleDialogHelp);

public:

  DEFINE_IUNKNOWN(pObjOuter)

  // IUnknownMain methods
  //
  virtual HRESULT _IFUNC QueryInterfaceMain(REFIID iid, LPVOID FAR* pif);

  // IService methods
  //
  virtual HRESULT _IFUNC Init(PIBApplication pApp );
  virtual HRESULT _IFUNC RegisterClass (LPCOLESTR szProgId, IBClassMgr *pCM, BCID regId, BOOL, BOOL);
  virtual HRESULT _IFUNC UnregisterClass (LPCOLESTR);
  virtual HRESULT _IFUNC RegisterControlClass (LPCOLESTR szProgId, IBClassMgr *pCM, BCID regId, BOOL, BOOL);
  virtual HRESULT _IFUNC UnregisterControlClass (LPCOLESTR);
  virtual void    _IFUNC OnResize ();
  virtual void    _IFUNC OnActivate (BOOL fActivate);
  virtual HRESULT _IFUNC OnModalDialog (BOOL);
  virtual HRESULT _IFUNC OnSetFocus(BOOL);
  virtual HRESULT _IFUNC TranslateAccel (LPMSG);
  virtual BOleHelp _IFUNC HelpMode (BOleHelp);
  virtual HRESULT _IFUNC CanClose();

  virtual HRESULT _IFUNC Browse (BOleInitInfo*);
  virtual HRESULT _IFUNC BrowseControls (BOleInitInfo*);
  virtual HRESULT _IFUNC BrowseClipboard (BOleInitInfo FAR*);
  virtual HRESULT _IFUNC Paste (BOleInitInfo FAR*);
  virtual HRESULT _IFUNC Clip (PIBDataProvider, BOOL, BOOL, BOOL);
  virtual HRESULT _IFUNC Drag (PIBDataProvider, BOleDropAction, BOleDropAction FAR*);

  virtual HMETAFILE _IFUNC GetPasteIcon (LPDATAOBJECT);

  virtual BOOL _IFUNC PasteHelper (BOleInitInfo FAR*, BOOL);
  virtual BOOL _IFUNC FormatHelper (LPFORMATETC FAR*, UINT FAR*, BOOL&,
                                                                              BOOL&, PIBDataProvider);
  void _IFUNC RemoveLinkFromClipList();
  void NotifyClipboardEmptied();

  virtual LPSTORAGE _IFUNC CreateStorageOnFile (LPCOLESTR, BOOL);
  virtual HRESULT   _IFUNC ConvertUI (PIBPart, BOOL, BOleConvertInfo FAR*);
  virtual HRESULT   _IFUNC ConvertGuts (PIBPart, BOOL, BOleConvertInfo FAR*);
  virtual BOleMenuEnable _IFUNC EnableEditMenu (BOleMenuEnable, PIBDataConsumer);

  // Implementation methods
  //
  virtual CLIPFORMAT _IFUNC MatchPriorityClipFormat(LPDATAOBJECT,
    LPOLEUIPASTEENTRY, int);
  virtual void _IFUNC MarkPasteEntries(LPDATAOBJECT,LPOLEUIPASTEENTRY,int);
  virtual void _IFUNC UnloadObjects (REFCLSID);

public:

  // IOleWindow methods, base of IOleInPlaceUIWindow and IOleInPlaceFrame
  //
  HRESULT _IFUNC GetWindow(HWND FAR* lphwnd);
  HRESULT _IFUNC ContextSensitiveHelp(BOOL fEnterMode);

  // IOleInPlaceUIWindow methods, base of IOleInPlaceFrame
  //
  HRESULT _IFUNC GetBorder(LPRECT lprectBorder);
  HRESULT _IFUNC RequestBorderSpace(LPCRECT lprectWidths);
  HRESULT _IFUNC SetBorderSpace(LPCBORDERWIDTHS lpborderwidths);
  HRESULT _IFUNC SetActiveObject(IOleInPlaceActiveObject* lpActiveObject,
           LPCOLESTR lpszObjName);

  // IOleInPlaceFrame methods
  //
  HRESULT _IFUNC InsertMenus(HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidts);
  HRESULT _IFUNC SetMenu(HMENU hmenuShared, HOLEMENU holemenu, HWND hwndActiveObject);
  HRESULT _IFUNC RemoveMenus(HMENU hmenuShared);
  HRESULT _IFUNC SetStatusText(LPCOLESTR lpszStatusText);
  HRESULT _IFUNC EnableModeless(BOOL fEnable);
  HRESULT _IFUNC TranslateAccelerator(LPMSG lpmsg, WORD wID);

};

// solves a casting in a ternary issue
//
inline IBWindow * getNegotiator(IBApplication *pApp, IBWindow *pContainer)
{
  return (GetScode(pApp->IsMDI()) == S_OK) ?      pApp : pContainer;
}

#endif
