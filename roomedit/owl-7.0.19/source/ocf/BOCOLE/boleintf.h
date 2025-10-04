//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
// Portions Copyright (c) 2009 OWLNext community
//
// $Revision: 1.2 $
//
//  Contains the main interfaces (abstract base classes) used by Bolero to
//  encapsulate OLE2 compound document functionality
//----------------------------------------------------------------------------
#ifndef _BOLEINTF_H
#define _BOLEINTF_H

#ifndef _BOLEDEFS_H
#include <boledefs.h>
#endif

#include "pshpack8.h"

PREDECLARE_INTERFACE( IBClassMgr )
PREDECLARE_INTERFACE( IBPart )
PREDECLARE_INTERFACE( IBService )
PREDECLARE_INTERFACE( IBLinkable)
PREDECLARE_INTERFACE( IBRootLinkable)
PREDECLARE_INTERFACE( IBContains)
PREDECLARE_INTERFACE( IBDocument )
PREDECLARE_INTERFACE( IBContainer )
PREDECLARE_INTERFACE( IBSite )
PREDECLARE_INTERFACE( IBApplication )
PREDECLARE_INTERFACE( IBLinkInfo)
PREDECLARE_INTERFACE( IBDataNegotiator)
PREDECLARE_INTERFACE( IBDataProvider)
PREDECLARE_INTERFACE( IBDataConsumer)
PREDECLARE_INTERFACE( IBWindow)
PREDECLARE_INTERFACE( IBDropDest)
PREDECLARE_INTERFACE( IBClass)
PREDECLARE_INTERFACE( IBOverrideBrowseLinks)
PREDECLARE_INTERFACE( IBSite2)
PREDECLARE_INTERFACE( IBOverrideHelpButton)
PREDECLARE_INTERFACE( IBPart2 )
PREDECLARE_INTERFACE( IBDataProvider2 )
PREDECLARE_INTERFACE( IBService2 )
PREDECLARE_INTERFACE( IBControlSite)
PREDECLARE_INTERFACE( IBControl)
PREDECLARE_INTERFACE( IBEventsHandler )
PREDECLARE_INTERFACE( IBEventClass )
PREDECLARE_INTERFACE( IBSinkList )
PREDECLARE_INTERFACE( IBPicture)
PREDECLARE_INTERFACE( IBDataState)

// IClassMgr abstract base class
//
class _ICLASS IBClassMgr : public IUnknown {
public:

   virtual HRESULT _IFUNC ComponentCreate(
                PIUnknown FAR* ppRet,
                PIUnknown pAggregator,
                BCID            idClass
                )       = 0;
   virtual HRESULT _IFUNC ComponentInfoGet(
                PIUnknown FAR* ppInfo,
                PIUnknown pAggregator,
                BCID            idClass
                )       = 0;
};

// IBDataNegotiator -- Enumerats formats for drag/drop and paste
//
class _ICLASS IBDataNegotiator : public IUnknown {
  public:

    virtual UINT                _IFUNC CountFormats ()                       = 0;
    virtual HRESULT     _IFUNC GetFormat (UINT, BOleFormat FAR*)     = 0;
};

// IBDataConsumer -- Supported by containers who want to accept drag/drop
//                   and paste. No new methods over the negotiator, so
//                   this is just a placeholder for the consumer name.
//
class _ICLASS IBDataConsumer : public IBDataNegotiator {
};

// IBDataProvider -- Supported by parts and/or sites which can be
//                   copied to the OLE2 clipboard
//
class _ICLASS IBDataProvider : public IBDataNegotiator {
  public:

    virtual HANDLE _IFUNC GetFormatData (BOleFormat FAR*)              = 0;
    virtual HRESULT _IFUNC Draw (HDC,LPCRECTL,LPCRECTL,BOleAspect,
                                        BOleDraw bd = BOLE_DRAW_NONE)      = 0;
    virtual HRESULT _IFUNC GetPartSize (LPSIZE)                        = 0;
    virtual HRESULT _IFUNC Save (IStorage FAR*, BOOL fSamAsLoad,
                                                BOOL fRemember)        = 0;
};

// IBDataProvider2 -- new version of IBDataProvider
//
class _ICLASS IBDataProvider2 : public IBDataProvider {
  public:

    virtual HRESULT _IFUNC SetFormatData (BOleFormat FAR* fmt, HANDLE data, BOOL release) = 0;
};

// IBPart -- Supported by server objects
//
class _ICLASS IBPart : public IBDataProvider {
  public:

    virtual HRESULT _IFUNC Init (PIBSite,BOleInitInfo FAR*)         = 0;
    virtual HRESULT _IFUNC Close ()                                                         = 0;
    virtual HRESULT _IFUNC CanOpenInPlace ()                        = 0;
    virtual HRESULT _IFUNC SetPartSize (LPSIZE)                     = 0;
    virtual HRESULT _IFUNC SetPartPos (LPRECT)                      = 0;
    virtual HRESULT _IFUNC Activate (BOOL)                          = 0;
    virtual HRESULT _IFUNC Show (BOOL)                              = 0;
    virtual HRESULT _IFUNC Open (BOOL)                              = 0;
    virtual HRESULT _IFUNC EnumVerbs (BOleVerb FAR*)                = 0;
    virtual HRESULT _IFUNC DoVerb (UINT)                            = 0;
    virtual HWND    _IFUNC OpenInPlace (HWND)                       = 0;
    virtual HRESULT _IFUNC InsertMenus (HMENU,BOleMenuWidths FAR*)  = 0;
    virtual HRESULT _IFUNC ShowTools (BOOL)                         = 0;
    virtual void    _IFUNC FrameResized (LPCRECT,BOOL)              = 0;
    virtual HRESULT _IFUNC DragFeedback (LPPOINT,BOOL)              = 0;
    virtual HRESULT _IFUNC GetPalette (LPLOGPALETTE FAR*)           = 0;
    virtual HRESULT _IFUNC SetHost (PIBContainer pObjContainer)     = 0;
    virtual HRESULT _IFUNC DoQueryInterface(REFIID iid, void FAR* FAR* pif)= 0;

    virtual LPOLESTR _IFUNC GetName (BOlePartName)                  = 0;
};


// IBPart2 -- Supported by server objects
//
class _ICLASS IBPart2 : public IBPart,
                        public IBDataProvider2 {
};

// IService abstract base class
//
class _ICLASS IBService : public IUnknown {
  public:

    virtual void     _IFUNC OnResize ()                           = 0;
    virtual void     _IFUNC OnActivate (BOOL)                     = 0;
    virtual HRESULT  _IFUNC OnModalDialog (BOOL)                  = 0;
    virtual HRESULT  _IFUNC OnSetFocus(BOOL)                      = 0;

    virtual HRESULT  _IFUNC Init (PIBApplication)                 = 0;
    virtual HRESULT  _IFUNC RegisterClass (LPCOLESTR,PIBClassMgr,BCID,
                              BOOL fEmbedInSelf, BOOL fSingleUse) = 0;
    virtual HRESULT  _IFUNC UnregisterClass (LPCOLESTR)           = 0;
    virtual HRESULT  _IFUNC TranslateAccel (LPMSG)                = 0;
    virtual BOleHelp _IFUNC HelpMode (BOleHelp)                   = 0;
    virtual HRESULT  _IFUNC CanClose ()                           = 0;

    virtual HRESULT  _IFUNC BrowseClipboard (BOleInitInfo FAR*)    = 0;
    virtual HRESULT  _IFUNC Browse (BOleInitInfo FAR*)            = 0;

    virtual HRESULT        _IFUNC Paste (BOleInitInfo FAR*)                                        = 0;
    virtual HRESULT  _IFUNC Clip (PIBDataProvider, BOOL, BOOL, BOOL) = 0;
    virtual HRESULT  _IFUNC Drag (PIBDataProvider, BOleDropAction, BOleDropAction FAR*)                 = 0;
    virtual IStorage FAR* _IFUNC CreateStorageOnFile (LPCOLESTR, BOOL) = 0;
    virtual HRESULT _IFUNC ConvertUI (PIBPart, BOOL,
                                 BOleConvertInfo FAR*)       = 0;
    virtual HRESULT _IFUNC ConvertGuts (PIBPart, BOOL,
                                   BOleConvertInfo FAR*)     = 0;
    virtual BOleMenuEnable _IFUNC EnableEditMenu (BOleMenuEnable,
                                                  PIBDataConsumer)= 0;
};

class _ICLASS IBService2 : public IBService {
  public:
    virtual HRESULT  _IFUNC BrowseControls (BOleInitInfo FAR*)    = 0;
    virtual HRESULT  _IFUNC RegisterControlClass (LPCOLESTR,PIBClassMgr,BCID,
                              BOOL fEmbedInSelf, BOOL fSingleUse) = 0;
    virtual HRESULT  _IFUNC UnregisterControlClass (LPCOLESTR)    = 0;
};

// ILinkable abstract base class
//
class _ICLASS IBLinkable : public IUnknown {
  public:
    virtual HRESULT _IFUNC GetMoniker(IMoniker FAR* FAR* ppMon)    = 0;
    virtual HRESULT _IFUNC OnRename(PIBLinkable pContainer, LPCOLESTR szName) = 0;
                    // pass NULL szName when Doc closes
                    // pass boledoc for pContainer if root part (file)
};
class _ICLASS IBRootLinkable : public IBLinkable {
  public:
    virtual HRESULT _IFUNC Init (PIBContains pC, LPCOLESTR szProgId) = 0;
    virtual HRESULT _IFUNC SetMoniker (IMoniker FAR* pMon)           = 0;
};
/*
 * on BOleLinkDoc used by Part servers and Linking Containers
 * on BOleSite          used by parts
 */

// IContains abstract base class
//
class _ICLASS IBContains : public IUnknown {
  public:
    virtual HRESULT _IFUNC Init(LPCOLESTR)                               = 0;
    virtual HRESULT _IFUNC GetPart(PIBPart FAR*, LPCOLESTR)              = 0;
};
/*
 * on linkable containers (all Containers with BOleLinkDoc helper)
 * on linkable server documents (with BOleLinkDoc helper)
 * on intermediate parts  (in addition to IPart)
 */

// IBDocument -- Supported by BOleDocument one per container child window
//
class _ICLASS IBDocument : public IUnknown {
  public:
    virtual HRESULT _IFUNC Init (PIBContainer)                         = 0;
    virtual void    _IFUNC OnResize ()                                 = 0;
    virtual void    _IFUNC OnActivate (BOOL)                           = 0;
    virtual void    _IFUNC OnClose ()                                  = 0;
    virtual HRESULT _IFUNC OnSetFocus(BOOL)                            = 0;

    virtual HRESULT _IFUNC EnumLinks (PIBLinkInfo FAR*)                = 0;
    virtual HRESULT _IFUNC BrowseLinks ()                              = 0;
    virtual HRESULT _IFUNC UpdateLinks ()                              = 0;
};



// IBWindow -- Supported on child window *and* frame window of container apps
//
class _ICLASS IBWindow : public IUnknown {
public:
    virtual HWND     _IFUNC GetWindow ()                               = 0;
    virtual HRESULT  _IFUNC GetWindowRect (LPRECT)                     = 0;
    virtual LPCOLESTR _IFUNC GetWindowTitle ()                         = 0;
    virtual void     _IFUNC AppendWindowTitle (LPCOLESTR)              = 0;
    virtual HRESULT  _IFUNC SetStatusText (LPCOLESTR)                  = 0;

    virtual HRESULT  _IFUNC RequestBorderSpace (LPCRECT)               = 0;
    virtual HRESULT  _IFUNC SetBorderSpace (LPCRECT)                   = 0;

    virtual HRESULT  _IFUNC InsertContainerMenus (HMENU,BOleMenuWidths FAR*)    = 0;
    virtual HRESULT  _IFUNC SetFrameMenu (HMENU)                       = 0;
    virtual void     _IFUNC RestoreUI ()                               = 0;
    virtual HRESULT  _IFUNC Accelerator (LPMSG)                                                           = 0;
         virtual HRESULT  _IFUNC GetAccelerators (HACCEL FAR*, int FAR*)    = 0;
};

// IBContainer -- Supported by container app's document window
//
class _ICLASS IBContainer : public IBWindow {
  public:
    virtual HRESULT  _IFUNC FindDropDest (LPPOINT, PIBDropDest FAR*) = 0;
    virtual HRESULT  _IFUNC AllowInPlace ()                          = 0;
    virtual HRESULT  _IFUNC BringToFront ()                          = 0;
};

// IBDropDest -- Supported by container app's document window and/or other
//               areas inside the window.
//
class _ICLASS IBDropDest : public IBDataConsumer {
  public:
    virtual HRESULT  _IFUNC Drop (BOleInitInfo FAR*, LPPOINT, LPCRECT) = 0;
    virtual void     _IFUNC DragFeedback (LPPOINT, LPCRECT,
                                      BOleMouseAction, UINT, HRESULT&) = 0;
    virtual HRESULT  _IFUNC Scroll (BOleScroll scrollDir)              = 0;
    virtual HRESULT  _IFUNC GetScrollRect (LPRECT)                     = 0;
};

// IBApplication -- Supported at frame window/app object of container apps
//
class _ICLASS IBApplication : public IBWindow {
  public:

    virtual LPCOLESTR   _IFUNC GetAppName ()                           = 0;
    virtual BOleHelp _IFUNC HelpMode (BOleHelp)                        = 0;
    virtual HRESULT  _IFUNC CanLink ()                                 = 0;
    virtual HRESULT  _IFUNC CanEmbed ()                                = 0;
    virtual HRESULT  _IFUNC IsMDI ()                                   = 0;
    virtual HRESULT  _IFUNC OnModalDialog (BOOL)                       = 0;
    virtual void     _IFUNC DialogHelpNotify (BOleDialogHelp)          = 0;
    virtual void           _IFUNC ShutdownMaybe ()                                                                   = 0;
};

// IBSite -- Supported by container apps for each linked/embedded object
//
class _ICLASS IBSite : public IUnknown {
  public:

    virtual HRESULT _IFUNC SiteShow (BOOL)                             = 0;
    virtual HRESULT _IFUNC DiscardUndo ()                              = 0;
    virtual HRESULT _IFUNC GetSiteRect (LPRECT,LPRECT)                 = 0;
    virtual HRESULT _IFUNC SetSiteRect (LPCRECT)                       = 0;
    virtual HRESULT _IFUNC SetSiteExtent (LPCSIZE)                     = 0;
    virtual HRESULT _IFUNC GetZoom(BOleScaleFactor FAR* pScale)        = 0;
    virtual void    _IFUNC Invalidate (BOleInvalidate)                 = 0;
    virtual void    _IFUNC OnSetFocus(BOOL)                            = 0;
    virtual HRESULT _IFUNC Init (PIBDataProvider, PIBPart, LPCOLESTR, BOOL)     = 0;
    virtual void    _IFUNC Disconnect ()                                               = 0;
};

// IBSite2 -- Add flexibility to have parent of site not be container
//
class _ICLASS IBSite2 : public IBSite {
  public:
    virtual HRESULT _IFUNC GetParentWindow (HWND FAR*)                 = 0;
};


// IBDataState
// this interface is used to give access to the internal dirty flag
// (only used internally by controls)
//
class _ICLASS IBDataState: public IUnknown {
  public:
    virtual void _IFUNC SetDirty (BOOL)                               = 0;
    virtual HRESULT _IFUNC IsDirty ()                                  = 0;
    virtual HRESULT _IFUNC GetClassID (LPCLSID)                        = 0;
};


// IBControlSite
//

//class _ICLASS IDispatch;   // forward class references
//class _ICLASS ITypeInfo;   // forward class references
//typedef long DISPID;


class _ICLASS IBControlSite : public IUnknown {
  public:
    virtual HRESULT _IFUNC Init (UINT, IBControl*, UINT)             = 0;
    virtual HRESULT _IFUNC OnPropertyChanged (DISPID dispid)         = 0;
    virtual HRESULT _IFUNC OnPropertyRequestEdit (DISPID dispid)     = 0;
    virtual HRESULT _IFUNC OnControlFocus (BOOL fGotFocus)           = 0;
    virtual HRESULT _IFUNC TransformCoords (POINTL FAR* lpptlHimetric,
                            TPOINTF FAR* lpptfContainer, DWORD flags) = 0;
};

// IBControl
//
class _ICLASS IBControl : public IBPart //, public IBDataProvider2
{
  public:
    virtual HRESULT _IFUNC GetEventIID (IID *iidEvent)                 = 0;
    virtual HRESULT _IFUNC GetEventTypeInfo (ITypeInfo FAR * FAR *ppTypeInfo) = 0;
    virtual HRESULT _IFUNC GetCtrlDispatch (IDispatch FAR * FAR *ppDispatch)  = 0;
    virtual HRESULT _IFUNC SetEventDispatch (IDispatch *)              = 0;
    virtual HRESULT _IFUNC SetAmbientDispatch (IDispatch *)            = 0;

    // standard control methods - other methods access thru GetCtrlDispatch ()
    virtual HRESULT _IFUNC Refresh ()                                  = 0;
    virtual HRESULT _IFUNC DoClick ()                                  = 0;
    virtual HRESULT _IFUNC AboutBox ()                                 = 0;

    virtual HRESULT _IFUNC AmbientChanged (DISPID dispid)              = 0;

    // function passed down by BOleControlSite
    virtual HRESULT _IFUNC GetControlInfo (LPCONTROLINFO)              = 0;
    virtual HRESULT _IFUNC OnMnemonic (LPMSG)                           = 0;
    virtual HRESULT _IFUNC FreezeEvents (BOOL)                           = 0;
    virtual HRESULT _IFUNC GetClassInfo (LPTYPEINFO FAR*)              = 0;
    virtual HRESULT _IFUNC GetSizeMax (ULARGE_INTEGER FAR*)              = 0;
    virtual HRESULT _IFUNC Save (LPSTREAM)                              = 0;
};

// IBSinkList
//
class _ICLASS IBSinkList : public IUnknown {
  public:
    virtual HRESULT _IFUNC NextSink (LPVOID FAR*)                     = 0;
    virtual HRESULT _IFUNC GetSinkAt (int, LPVOID FAR*)               = 0;
    virtual HRESULT _IFUNC Reset ()                                     = 0;
};

// IBEventsHandler
//
class _ICLASS IBEventsHandler : public IUnknown {
  public:
    virtual HRESULT _IFUNC RegisterEventsSet (REFIID, UINT)               = 0;
    virtual HRESULT _IFUNC RegisterConnectionPoint (LPCONNECTIONPOINT)   = 0;
    virtual HRESULT _IFUNC GetSinkListForIID (REFIID, IBSinkList**)      = 0;
};

// IBEventClass
//
class _ICLASS IBEventClass : public IUnknown {
  public:
    virtual HRESULT _IFUNC GetSinkList (IBSinkList **)                  = 0;
    virtual HRESULT _IFUNC FreezeEvents (bool)                          = 0;
};

// IBPicture
//
class _ICLASS IBPicture : public IUnknown {
  public:
   virtual HRESULT _IFUNC GetHandle (HANDLE FAR* phandle)   = 0;
   virtual HRESULT _IFUNC Draw (HDC hdc, long x, long y, long cx, long cy,
                            long xSrc, long ySrc, long cxSrc, long cySrc,
                            LPCRECT lprcWBounds)           = 0;
    virtual HRESULT _IFUNC PictChanged ()                    = 0;
    virtual HRESULT _IFUNC SetPictureDesc (LPVOID pPD)       = 0;  // LPVOID = PICTDESC *
};

// IBLinkInfo -- Supported by BOleParts which are link objects
//
class _ICLASS IBLinkInfo : public IUnknown {
  public:
    virtual HRESULT _IFUNC SourceBreak ()                              = 0;
    virtual HRESULT _IFUNC SourceGet (LPOLESTR, UINT count)            = 0;
    virtual HRESULT _IFUNC SourceSet (LPOLESTR)                        = 0;
    virtual HRESULT _IFUNC SourceOpen ()                               = 0;
    virtual HRESULT _IFUNC UpdateGet (BOleLinkUpdate FAR*)             = 0;
    virtual HRESULT _IFUNC UpdateSet (BOleLinkUpdate)                  = 0;
    virtual HRESULT _IFUNC UpdateNow ()                                = 0;
};

// IBClass -- Supported by BOleFactory
//
class _ICLASS IBClass : public IClassFactory {
  public:
    virtual HRESULT _IFUNC Init(BOOL fSelfEmbed, LPCOLESTR pszProgId, IBClassMgr *pCM, BCID rid) = 0;
    virtual HRESULT _IFUNC Register(BOOL fSingleUse) = 0;
    virtual HRESULT _IFUNC Revoke()                                                                                 = 0;
    virtual HRESULT       _IFUNC GetClassID(LPCLSID pOut)                = 0;
    virtual BOOL  _IFUNC IsA(REFCLSID pOut)                                                                 = 0;
    virtual BOOL _IFUNC AllowEmbedFromSelf()                                                        = 0;
    virtual HRESULT _IFUNC GetEmbeddingHelperFactory(LPUNKNOWN *ppF)  = 0;
};

// IBOverrideBrowseLinks -- Supported by containers (who aggregate BOleDoc)
//                          who want to override our BrowseLinks dialog.
//
class _ICLASS IBOverrideBrowseLinks : public IUnknown {
  public:
    virtual HRESULT    _IFUNC BrowseLinks ()                        = 0;
};

// IBOverrideHelpButton -- Supported by container apps (who aggregate
//                         BOleService) who want to suppress the help
//                         button in our dialogs.
//
class _ICLASS IBOverrideHelpButton : public IUnknown {
  public:
    virtual HRESULT    _IFUNC ShowHelpButton (BOleDialogHelp) = 0;
};

DEFINE_INLINE_QI( IBDataNegotiator, IUnknown)
DEFINE_INLINE_QI( IBDataConsumer, IUnknown)
DEFINE_INLINE_QI( IBDropDest, IBDataConsumer)
DEFINE_INLINE_QI( IBDataProvider, IBDataNegotiator)
DEFINE_INLINE_QI( IBPart, IBDataProvider)

DEFINE_INLINE_QI( IBWindow, IUnknown)
DEFINE_INLINE_QI( IBContainer, IBWindow)
DEFINE_INLINE_QI( IBApplication, IBWindow)

DEFINE_INLINE_QI( IBDocument, IUnknown)
DEFINE_INLINE_QI( IBService, IUnknown)
DEFINE_INLINE_QI( IBClassMgr, IUnknown)
DEFINE_INLINE_QI( IBLinkInfo, IUnknown)
DEFINE_INLINE_QI( IBSite, IUnknown)
DEFINE_INLINE_QI( IBSite2, IBSite)
DEFINE_INLINE_QI( IBContains, IUnknown)

DEFINE_INLINE_QI( IBLinkable, IUnknown)
DEFINE_INLINE_QI( IBRootLinkable, IBLinkable)
DEFINE_INLINE_QI( IClassFactory, IUnknown)
DEFINE_INLINE_QI( IBClass, IClassFactory)

DEFINE_INLINE_QI( IBOverrideBrowseLinks, IUnknown)
DEFINE_INLINE_QI( IBOverrideHelpButton, IUnknown)

DEFINE_INLINE_QI( IBDataProvider2, IBDataProvider)
DEFINE_INLINE_QI2(IBPart2, IBPart, IBDataProvider2)

DEFINE_INLINE_QI( IBService2, IBService)
DEFINE_INLINE_QI( IBControl, IBPart)
DEFINE_INLINE_QI( IBControlSite, IUnknown)
DEFINE_INLINE_QI( IBEventsHandler, IUnknown)
DEFINE_INLINE_QI( IBEventClass, IUnknown)
DEFINE_INLINE_QI( IBSinkList, IUnknown)
DEFINE_INLINE_QI( IBPicture, IUnknown)
DEFINE_INLINE_QI( IBDataState, IUnknown)


#include "poppack.h"

#endif

