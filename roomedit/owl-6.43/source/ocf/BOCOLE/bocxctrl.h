//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Implements the Bolero half of the OLE2 Control
//
//  BOleControlSite objects impersonate the client application (Container)
//    from the point of view of the Bolero customer who's
//    writing a control (IBPart and IBControl)
//    BOleControlSite aggregate BOleSite or BOleInProcServer and provide
//    implementation for control specific interfaces.
//    BOleControlSite uses IBControl to talk to the control whereas
//    the aggregated BOleInProcServer (BOleSite) uses IBpart.
//    In consequence a control must implement both IBPart and IBControl.
//----------------------------------------------------------------------------
#ifndef _BOCXCTRL_H
#define _BOCXCTRL_H

#ifndef _BOLEIPS_H
#include <boleips.h>
#endif

#ifndef _OLECTL_H_
#include <olectl.h>
#endif

#define BOLE_USE_LIGHT             0
#define BOLE_USE_EXE               1
#define BOLE_USE_DLL               2

#define BOLE_DEFAULT_ENUM_SIZE         10


//****************************************************************
//  class BEventList
//
//****************************************************************
class _ICLASS BEventList : public IEnumConnectionPoints
{

  public:

    BEventList (UINT = BOLE_DEFAULT_ENUM_SIZE);
    ~BEventList ();

    //**** IUnknown methods ****
    ULONG _IFUNC AddRef ();
    ULONG _IFUNC Release ();
    HRESULT _IFUNC QueryInterface(REFIID, LPVOID FAR*);

    //**** IEnumConnectionPoints methods ****
    HRESULT _IFUNC Next (ULONG, LPCONNECTIONPOINT FAR*, ULONG FAR*);
    HRESULT _IFUNC Skip (ULONG);
    HRESULT _IFUNC Reset ();
    HRESULT _IFUNC Clone (LPENUMCONNECTIONPOINTS FAR*);

    // public functions
    inline IConnectionPoint* operator [] (int nPos) { return pConnectionPoints[nPos]; }
    HRESULT AddEventsSet (REFIID, LPUNKNOWN, UINT);
    HRESULT Add (LPCONNECTIONPOINT);
    void FreezeAll (bool);

  private:
    BEventList (BEventList*);
    int Expand ();

  private:
    IConnectionPoint ** pConnectionPoints;
    int nCurrPos;
    int nSize;

    // lifetime counter
    DWORD cRef;
};

//****************************************************************
//  class BOleControlSite
//
//****************************************************************
class _ICLASS BOleControlSite : public BOleComponent,
                      public IBControlSite,
                      public IOleControl,
                      public IProvideClassInfo,
                      public IPersistStreamInit,
                      public IBEventsHandler,
                      public IConnectionPointContainer
{

  public:
    BOleControlSite (BOleClassManager *, IBUnknownMain *);
    ~BOleControlSite();

    //**** IUnknown methods ****
    DEFINE_IUNKNOWN(pObjOuter)
    virtual HRESULT _IFUNC QueryInterfaceMain(REFIID, LPVOID FAR*);

    //**** IBControlSite methods ****
    HRESULT _IFUNC Init (UINT, IBControl*,UINT);
    HRESULT _IFUNC OnPropertyChanged (DISPID);
    HRESULT _IFUNC OnPropertyRequestEdit (DISPID dispid);
    HRESULT _IFUNC OnControlFocus (BOOL);
    HRESULT _IFUNC TransformCoords (POINTL FAR* lpptlHimetric,
                            TPOINTF FAR* lpptfContainer, DWORD flags);

    //**** IOleControl methods ****
    HRESULT _IFUNC GetControlInfo (LPCONTROLINFO);
    HRESULT _IFUNC OnMnemonic (LPMSG);
    HRESULT _IFUNC OnAmbientPropertyChange (DISPID);
    HRESULT _IFUNC FreezeEvents (BOOL);

    //**** IProvideClassInfo methods ****
    HRESULT _IFUNC GetClassInfo (LPTYPEINFO FAR*);

    //**** IPersistStreamInit methods ****
    HRESULT _IFUNC GetClassID (LPCLSID);
    HRESULT _IFUNC IsDirty ();
    HRESULT _IFUNC Load(LPSTREAM);
    HRESULT _IFUNC Save (LPSTREAM, BOOL);
    HRESULT _IFUNC GetSizeMax (ULARGE_INTEGER FAR*);
    HRESULT _IFUNC InitNew ();

    //**** IBEventsHandler methods ****
    HRESULT _IFUNC RegisterEventsSet (REFIID, UINT);
    HRESULT _IFUNC RegisterConnectionPoint (LPCONNECTIONPOINT);
    HRESULT _IFUNC GetSinkListForIID (REFIID, IBSinkList**);

    //**** IConnectionPointContainer methods ****
    HRESULT _IFUNC EnumConnectionPoints (LPENUMCONNECTIONPOINTS FAR*);
    HRESULT _IFUNC FindConnectionPoint (REFIID, LPCONNECTIONPOINT FAR*);

  private:
    IBUnknownMain* pObjInner; // IUnknown of the aggregated Bolero Helper
    BEventList * pEventList; // ConnectionPoints list

    IOleControlSite * pControlSite; // talk to the container

    IBControl * pControl; // talk to the control

    bool fRegConnPoint; // true if a control registered its own connection point
    IBDataState* pDataDirty;

};

#endif

