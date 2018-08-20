//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
// $Revision: 1.2 $
//
//  Implements the Bolero half of the OLE2 control object.
//    BOleControl objects impersonate the OCX from the point of view of the
//    Bolero customer who's writing a OCX container.
//----------------------------------------------------------------------------
#ifndef _BOLECTRL_H
#define _BOLECTRL_H

#ifndef _BOLEPART_H
#include "bolepart.h"
#endif


class _ICLASS BOleControl : public IBControl,
                            public BOlePart,
                            public IOleControlSite,
                            public IPropertyNotifySink
{
protected:
   IDispatch     *pIEvents;
   IDispatch     *pIAmbients;
   IDispatch     *pCtrlDispatch;
   IOleControl   *pOleCtrl;
   IBControlSite *pCSite;
   BOleControl   *pNextCtrl, *pPrevCtrl;

   IID            iidEvents;   // iid for object's events interface
   DWORD          ConnEvents;  // returned from connection point advise
   DWORD          ConnProp;    // returned from connection point advise
   CONTROLINFO    CtrlInfo;

   BOOL    ObjectEventsIID (IID *piid);
   HRESULT InvokeStdMethod (DISPID id);

private:
  void AddToCtrlList ();
  void RemoveFromCtrlList ();

public:
   BOleControl (BOleClassManager *pF, IBUnknownMain * pO, BOleDocument * pOD);
   ~BOleControl ();

   BOleControl *GetNextControl () {return pNextCtrl;}
   IOleControl *GetOleControl ()  {return pOleCtrl;}
   HRESULT     _IFUNC BOleCreate (BOleInitInfo *pBI, DWORD dwRenderOpt,
                                                      LPVOID FAR *ppv);

// IBControl methods
   virtual HRESULT _IFUNC GetEventIID (IID *iidEvent);
   virtual HRESULT _IFUNC GetEventTypeInfo (LPTYPEINFO *ppTypeInfo);
   virtual HRESULT _IFUNC GetCtrlDispatch (IDispatch FAR * FAR *ppDispatch);
   virtual HRESULT _IFUNC SetEventDispatch (IDispatch *);
   virtual HRESULT _IFUNC SetAmbientDispatch (IDispatch *);
   virtual HRESULT _IFUNC AmbientChanged (DISPID dispid);

   // standard control methods - other methods by IDispatch
   virtual HRESULT _IFUNC Refresh ();
   virtual HRESULT _IFUNC DoClick ();
   virtual HRESULT _IFUNC AboutBox ();

        // function passed down by BOleControlSite (typically NOIMPL) here
   virtual HRESULT _IFUNC GetControlInfo (LPCONTROLINFO pCI);
   virtual HRESULT _IFUNC OnMnemonic (LPMSG pMsg);
   virtual HRESULT _IFUNC FreezeEvents (BOOL bFreeze);
   virtual HRESULT _IFUNC GetClassInfo (LPTYPEINFO FAR* ppTI);
   virtual HRESULT _IFUNC GetSizeMax (ULARGE_INTEGER FAR*);
   virtual HRESULT _IFUNC Save (LPSTREAM);

public:
   // IUnknown Methods
   DEFINE_IUNKNOWN(pObjOuter);
   virtual HRESULT _IFUNC QueryInterfaceMain(REFIID iid, LPVOID FAR* pif);

// IOleControlSite methods
   HRESULT _IFUNC OnControlInfoChanged (void);
   HRESULT _IFUNC LockInPlaceActive (BOOL fLock);
   HRESULT _IFUNC GetExtendedControl (LPDISPATCH FAR* ppDisp);
   HRESULT _IFUNC TransformCoords (POINTL FAR* lpptlHimetric,
                                 POINTF FAR* lpptfContainer, DWORD flags);
   HRESULT _IFUNC TranslateAccelerator (LPMSG lpMsg, DWORD grfModifiers);
   HRESULT _IFUNC OnFocus (BOOL fGotFocus);
   HRESULT _IFUNC ShowPropertyFrame (void);

   // IPropertyNotifySink methods
   HRESULT _IFUNC OnChanged (DISPID dispid);
   HRESULT _IFUNC OnRequestEdit (DISPID dispid);

public:
// IBPart methods
   virtual HRESULT _IFUNC Init(PIBSite, BOleInitInfo FAR*);

   virtual HRESULT _IFUNC Open (BOOL bOpen)                {return BOlePart::Open (bOpen);};
   virtual HRESULT _IFUNC Show (BOOL bShow)                {return BOlePart::Show (bShow);};
   virtual HRESULT _IFUNC Close ()                         {return BOlePart::Close ();}
   virtual HRESULT _IFUNC Activate (BOOL fActivate)        {return BOlePart::Activate (fActivate);}
   virtual HWND    _IFUNC OpenInPlace (HWND hWnd)          {return BOlePart::OpenInPlace (hWnd);}
   virtual HRESULT _IFUNC CanOpenInPlace ()                {return BOlePart::CanOpenInPlace ();}
   virtual HRESULT _IFUNC GetPalette (LPLOGPALETTE FAR*ppPal) {return BOlePart::GetPalette (ppPal);};
   virtual HRESULT _IFUNC SetHost (IBContainer FAR *pObjContainer) {return BOlePart::SetHost (pObjContainer);};
   virtual LPOLESTR _IFUNC GetName (BOlePartName nameCode) {return BOlePart::GetName (nameCode);};
   virtual HRESULT _IFUNC GetPartSize (LPSIZE pSize)       {return BOlePart::GetPartSize (pSize);};
   virtual HRESULT _IFUNC SetPartSize (LPSIZE pSize)       {return BOlePart::SetPartSize (pSize);} ;
   virtual HRESULT _IFUNC SetPartPos (LPRECT pRect)        {return BOlePart::SetPartPos (pRect);};
   virtual HRESULT _IFUNC Draw (HDC hdc, LPCRECTL lpExtent, LPCRECTL lpClip, BOleAspect aspect, BOleDraw flags) \
                     {return BOlePart::Draw (hdc, lpExtent, lpClip, aspect, flags);};
   virtual HRESULT _IFUNC InsertMenus (HMENU, BOleMenuWidths*)  {return ResultFromScode (E_NOTIMPL);};
   virtual void    _IFUNC FrameResized (const RECT *, BOOL) {};
   virtual HRESULT _IFUNC ShowTools (BOOL)                 {return ResultFromScode (E_NOTIMPL);};
   virtual HRESULT _IFUNC EnumVerbs (BOleVerb *pV)         {return BOlePart::EnumVerbs (pV);};
   virtual HRESULT _IFUNC DragFeedback(POINT*,BOOL)        {return ResultFromScode (E_NOTIMPL);};
   virtual HRESULT _IFUNC DoVerb (UINT verb)               {return BOlePart::DoVerb (verb);};
   virtual HRESULT _IFUNC DoQueryInterface(REFIID iid, void FAR* FAR* pif)   {return BOlePart::DoQueryInterface(iid, pif);};

//Clipboard methods inherited from IBDataProvider
   virtual UINT    _IFUNC CountFormats ()                  {return BOlePart::CountFormats();};
   virtual HRESULT _IFUNC GetFormat (UINT i, BOleFormat *Fmt)  {return BOlePart::GetFormat (i, Fmt);};
   virtual HANDLE  _IFUNC GetFormatData (BOleFormat *pf)   {return BOlePart::GetFormatData (pf);};
   virtual HRESULT _IFUNC Save (PIStorage pStg, BOOL fLoad, BOOL fRemember)  \
                     {return BOlePart::Save (pStg, fLoad, fRemember);};

};


#endif
