//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// ObjectComponents BOcOle engine linking & embedding interfaces
//----------------------------------------------------------------------------

#if !defined(OCF_OCBOCOLE_H)
#define OCF_OCBOCOLE_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/geometry.h>    // TPoint, TSize & TRect

#include <ocf/ocobject.h>       // Get ocf structs & enums



namespace ocf {

# include <pshpack8.h>

//
// BOcOle DLL name, entry point and version resource
//
#if _WIN64
# define BOLEDLL _T("BOCOF64.DLL")
# define BOLEDLLW _T("BOCOFW64.DLL")
#elif _WIN32
# define BOLEDLL _T("BOCOF.DLL")
# define BOLEDLLW _T("BOCOFW.DLL")
#elif __APPLE__
  // Mac OS X (or iOS, requires more detection)
#else
  //#error Not a supported platform
# define BOLEDLL _T("BOCOF.so")
# define BOLEDLLW _T("BOCOFW.so")
#endif

#define BOLEBIND "CreateClassMgr"
#define BOLE_FILEVER_LS 0x00010054L
#define BOLE_FILEVER_MS 0x00010001L

//
// GUIDs exported by the BOcOle DLL
//
#if defined(INIT_BGUID)
# define DEFINE_BOLEGUID(name, l) \
         EXTERN_C __declspec(dllexport) const GUID CDECL name = \
         { l, 0, 0, {0xC0,0,0,0,0,0,0,0x46} }
#else
# if defined(_OCFDLL)
#  if defined(_BUILDOCFDLL)
#    define DEFINE_BOLEGUID(name, l) EXTERN_C const GUID CDECL name
#  else
#    define DEFINE_BOLEGUID(name, l) EXTERN_C __declspec(dllimport) const GUID CDECL name
#  endif
# else  // !defined(_OCFDLL)
#     define DEFINE_BOLEGUID(name, l) EXTERN_C const GUID CDECL name
# endif
#endif

DEFINE_BOLEGUID(IID_IBPart,          0x02A104L);
DEFINE_BOLEGUID(IID_IBSite,          0x02A105L);
DEFINE_BOLEGUID(IID_IBContainer,     0x02A106L);
DEFINE_BOLEGUID(IID_IBDocument,      0x02A107L);
DEFINE_BOLEGUID(IID_IBService,       0x02A108L);
DEFINE_BOLEGUID(IID_IBApplication,   0x02A109L);
DEFINE_BOLEGUID(IID_IBClassMgr,      0x02A10AL);
DEFINE_BOLEGUID(IID_IBLinkInfo,      0x02A10BL);
DEFINE_BOLEGUID(IID_IBLinkable,      0x02A10CL);
DEFINE_BOLEGUID(IID_IBRootLinkable,  0x02A10DL);
DEFINE_BOLEGUID(IID_IBContains,      0x02A10EL);
DEFINE_BOLEGUID(IID_BOleDocument,    0x02A10FL);
DEFINE_BOLEGUID(IID_BOlePart,        0x02A110L);
DEFINE_BOLEGUID(IID_IBDataConsumer,  0x02A111L);
DEFINE_BOLEGUID(IID_IBDataProvider,  0x02A112L);
DEFINE_BOLEGUID(IID_IBWindow,        0x02A113L);
DEFINE_BOLEGUID(IID_IBDataNegotiator,0x02A114L);
DEFINE_BOLEGUID(IID_IBDropDest,      0x02A115L);
DEFINE_BOLEGUID(IID_BOleShadowData,  0x02A116L);
DEFINE_BOLEGUID(IID_IBClass,         0x02A117L);
DEFINE_BOLEGUID(IID_IBOverrideBrowseLinks, 0x02A118L);
DEFINE_BOLEGUID(IID_BOleSite,        0x02A119L);
DEFINE_BOLEGUID(IID_IBSite2,         0x02A11AL);
DEFINE_BOLEGUID(IID_IBOverrideHelpButton,  0x02A11BL);
DEFINE_BOLEGUID(IID_IBDataProvider2,  0x02A11CL);
DEFINE_BOLEGUID(IID_IBPart2,          0x02A11DL);
DEFINE_BOLEGUID(IID_IBService2,       0x02A11EL);
DEFINE_BOLEGUID(IID_BOleControl,      0x02A11FL);
DEFINE_BOLEGUID(IID_IBControl,        0x02A120L);
DEFINE_BOLEGUID(IID_IBControlSite,    0x02A121L);
DEFINE_BOLEGUID(IID_BOleFont,         0x02A122L);
DEFINE_BOLEGUID(IID_BOleControlSite,  0x02A123L);
DEFINE_BOLEGUID(IID_IBEventsHandler,  0x02A124L);
DEFINE_BOLEGUID(IID_IBEventClass,    0x02A125L);
DEFINE_BOLEGUID(IID_IBSinkList,      0x02A126L);
DEFINE_BOLEGUID(IID_BOlePicture,      0x02A127L);
//DEFINE_BOLEGUID(IID_BOlePart2,        0x02A127L);
DEFINE_BOLEGUID(IID_IBPicture,       0x02A128L);
DEFINE_BOLEGUID(IID_IBDataState,     0x02A129L);

//
// BOle Class ids for instantiating BOle objects (helpers)
//
typedef owl::uint32 BCID;
const BCID  cidBOleService      = 1L;   //
const BCID  cidBOleDocument     = 2L;   // helper for containers
const BCID  cidBOlePart         = 3L;   // helper for sites
const BCID  cidBOleSite         = 4L;   // helper for parts
const BCID  cidBOleInProcSite   = 5L;   // helper for parts
const BCID  cidBOleContainer    = 6L;   // helper for part documents
const BCID  cidBOleData         = 7L;   // helper for data transfer
const BCID  cidBOleFactory      = 8L;   // helper for class factory
const BCID  cidBOleShadowData   = 9L;   // helper for non delayed rendering
const BCID  cidBOleInProcHandler= 10L;  // helper for dll handler parts
const BCID  cidBOleControl      = 11L;  // helper for controls
const BCID  cidBOleFont         = 12L;  // IBFont implementation
const BCID  cidBOleControlSite  = 13L;  // helper for controls
const BCID  cidBOleControlFactory = 14L;// helper for control class factory
const BCID  cidBOlePicture      = 15L;  // IBPicture implementation

//
// Forward declarations
//
class _ICLASS IBClassMgr;
class _ICLASS IBPart;
class _ICLASS IBService;
class _ICLASS IBLinkable;
class _ICLASS IBRootLinkable;
class _ICLASS IBContains;
class _ICLASS IBDocument;
class _ICLASS IBContainer;
class _ICLASS IBSite;
class _ICLASS IBApplication;
class _ICLASS IBLinkInfo;
class _ICLASS IBDataNegotiator;
class _ICLASS IBDataProvider;
class _ICLASS IBDataConsumer;
class _ICLASS IBWindow;
class _ICLASS IBDropDest;
class _ICLASS IBClass;
class _ICLASS IBOverrideBrowseLinks;
class _ICLASS IBSite2;
class _ICLASS IBOverrideHelpButton;
class _ICLASS IBPart2;
class _ICLASS IBDataProvider2;
class _ICLASS IBService2;
class _ICLASS IBControlSite;
class _ICLASS IBControl;
class _ICLASS IBEventsHandler;
class _ICLASS IBEventClass;
class _ICLASS IBSinkList;
class _ICLASS IBDataState;


// MAP_PIX_TO_LOGHIM / MAP_LOGHIM_TO_PIX --
//
// The OLE2-defined macros for these guys don't round. Our versions
// add half the denominator so that the truncated result of the integer
// division is rounded to the nearest integer.
//
#define HIMETRIC_PER_INCH   2540      // number HIMETRIC units per inch

#ifdef MAP_PIX_TO_LOGHIM
#undef MAP_PIX_TO_LOGHIM
#endif
#define MAP_PIX_TO_LOGHIM(pixel, pixelsPerInch) \
        ((ULONG) ((((ULONG)HIMETRIC_PER_INCH) * pixel + (pixelsPerInch >> 1)) / pixelsPerInch))

#ifdef MAP_LOGHIM_TO_PIX
#undef MAP_LOGHIM_TO_PIX
#endif
#define MAP_LOGHIM_TO_PIX(hiMetric, pixelsPerInch) \
    (UINT) ((((double) hiMetric) * pixelsPerInch + (HIMETRIC_PER_INCH >> 1)) / HIMETRIC_PER_INCH)


//
/// IClassMgr abstract base class
//
class _ICLASS IBClassMgr : public IUnknown {
  public:
   virtual HRESULT _IFUNC ComponentCreate(
      IUnknown * * ppRet,
      IUnknown * pAggregator,
      BCID     classId
     )  = 0;
   virtual HRESULT _IFUNC ComponentInfoGet(
      IUnknown * * ppInfo,
      IUnknown * pAggregator,
      BCID     classId
     )  = 0;
};

//
/// IBDataNegotiator -- Enumerats formats for drag/drop and paste
//
class _ICLASS IBDataNegotiator : public IUnknown {
  public:
    virtual owl::uint     _IFUNC CountFormats()                      = 0;
    virtual HRESULT  _IFUNC GetFormat(owl::uint, TOcFormatInfo *) = 0;
};

//
/// IBDataConsumer -- Supported by containers who want to accept drag/drop
///                   and paste. No new methods over the negotiator, so
///                   this is just a placeholder for the consumer name.
//
class _ICLASS IBDataConsumer : public IBDataNegotiator {
};

//
/// IBDataProvider -- Supported by parts and/or sites which can be
///                   copied to the OLE2 clipboard
//
class _ICLASS IBDataProvider : public IBDataNegotiator {
  public:
    virtual HANDLE  _IFUNC GetFormatData(TOcFormatInfo *)       = 0;
    virtual HRESULT _IFUNC Draw(HDC, const RECTL *,
                                const RECTL *, TOcAspect,
                                TOcDraw bd = drNone)               = 0;
    virtual HRESULT _IFUNC GetPartSize(owl::TSize *)                 = 0;
    virtual HRESULT _IFUNC Save(IStorage *, BOOL fSamAsLoad,
                                BOOL fRemember)                    = 0;
};

//
/// IBDataProvider2 -- new version of IBDataProvider
//
class _ICLASS IBDataProvider2 : public IBDataProvider {
  public:
    virtual HRESULT _IFUNC SetFormatData(TOcFormatInfo * fmt, HANDLE data, BOOL release) = 0;
};

//
/// IBPart -- Supported by server objects
//
class _ICLASS IBPart : public IBDataProvider {
  public:
    virtual HRESULT _IFUNC Init(IBSite *, TOcInitInfo *)     = 0;
    virtual HRESULT _IFUNC Close()                                 = 0;
    virtual HRESULT _IFUNC CanOpenInPlace()                        = 0;
    virtual HRESULT _IFUNC SetPartSize(owl::TSize *)                 = 0;
    virtual HRESULT _IFUNC SetPartPos(owl::TRect *)                  = 0;
    virtual HRESULT _IFUNC Activate(BOOL)                          = 0;
    virtual HRESULT _IFUNC Show(BOOL)                              = 0;
    virtual HRESULT _IFUNC Open(BOOL)                              = 0;
    virtual HRESULT _IFUNC EnumVerbs(TOcVerb *)                 = 0;
    virtual HRESULT _IFUNC DoVerb(owl::uint)                            = 0;
    virtual HWND    _IFUNC OpenInPlace(HWND)                       = 0;
    virtual HRESULT _IFUNC InsertMenus(HMENU, TOcMenuWidths *)  = 0;
    virtual HRESULT _IFUNC ShowTools(BOOL)                         = 0;
    virtual void    _IFUNC FrameResized(const owl::TRect *, BOOL)    = 0;
    virtual HRESULT _IFUNC DragFeedback(owl::TPoint *, BOOL)         = 0;
    virtual HRESULT _IFUNC GetPalette(LOGPALETTE * *)        = 0;
    virtual HRESULT _IFUNC SetHost(IBContainer * pObjContainer) = 0;
    virtual HRESULT _IFUNC DoQueryInterface(const IID & iid, void * * pif)= 0;

    virtual LPOLESTR _IFUNC GetName(TOcPartName)                   = 0;
};

//
/// IBPart2 -- Supported by server objects
//
class _ICLASS IBPart2 : public IBPart,
                        public IBDataProvider2 {
};

//
/// IService abstract base class
//
class _ICLASS IBService : public IUnknown {
  public:
    virtual void     _IFUNC OnResize()                           = 0;
    virtual void     _IFUNC OnActivate(BOOL)                     = 0;
    virtual HRESULT  _IFUNC OnModalDialog(BOOL)                  = 0;
    virtual HRESULT  _IFUNC OnSetFocus(BOOL)                     = 0;

    virtual HRESULT  _IFUNC Init(IBApplication *)             = 0;
    virtual HRESULT  _IFUNC RegisterClass(LPCOLESTR, IBClassMgr *,
                                          BCID, BOOL embedInSelf,
                                          BOOL singleUse) = 0;
    virtual HRESULT  _IFUNC UnregisterClass(LPCOLESTR)           = 0;
    virtual HRESULT  _IFUNC TranslateAccel(MSG *)             = 0;
    virtual TOcHelp  _IFUNC HelpMode(TOcHelp)                    = 0;
    virtual HRESULT  _IFUNC CanClose()                           = 0;

    virtual HRESULT  _IFUNC BrowseClipboard(TOcInitInfo *)    = 0;
    virtual HRESULT  _IFUNC Browse(TOcInitInfo *)             = 0;

    virtual HRESULT  _IFUNC Paste(TOcInitInfo *)              = 0;
    virtual HRESULT  _IFUNC Clip(IBDataProvider *, BOOL, BOOL,
                                 BOOL)                           = 0;
    virtual HRESULT  _IFUNC Drag(IBDataProvider *, TOcDropAction,
                                 TOcDropAction *)             = 0;
    virtual IStorage * _IFUNC CreateStorageOnFile(LPCOLESTR,
                                                     BOOL)       = 0;
    virtual HRESULT _IFUNC ConvertUI(IBPart *, BOOL,
                                     TOcConvertInfo *)        = 0;
    virtual HRESULT _IFUNC ConvertGuts(IBPart *, BOOL,
                                       TOcConvertInfo *)      = 0;
    virtual TOcMenuEnable _IFUNC EnableEditMenu(TOcMenuEnable,
                                                IBDataConsumer *)= 0;
};

class _ICLASS IBService2 : public IBService {
  public:
    virtual HRESULT  _IFUNC BrowseControls(TOcInitInfo *)    = 0;
    virtual HRESULT  _IFUNC RegisterControlClass(LPCOLESTR, IBClassMgr *,
                                          BCID, BOOL embedInSelf,
                                          BOOL singleUse) = 0;
    virtual HRESULT  _IFUNC UnregisterControlClass(LPCOLESTR)    = 0;
};

//
/// ILinkable abstract base class
//
class _ICLASS IBLinkable : public IUnknown {
  public:
    virtual HRESULT _IFUNC GetMoniker(IMoniker * * ppMon)    = 0;
    virtual HRESULT _IFUNC OnRename(IBLinkable * pContainer,
                                    LPCOLESTR name)                = 0;
                            // pass 0 name when Doc closes
                            // pass boledoc for pContainer if root part(file)
};

//
//
//
class _ICLASS IBRootLinkable : public IBLinkable {
  public:
   virtual HRESULT _IFUNC Init(IBContains * pC, LPCOLESTR szProgId) = 0;
   virtual HRESULT _IFUNC SetMoniker(IMoniker * pMon)               = 0;
};
//
// on BOleLinkDoc used by Part servers and Linking Containers
// on BOleSite    used by parts
//

//
/// IContains abstract base class
//
class _ICLASS IBContains : public IUnknown {
  public:
    virtual HRESULT _IFUNC Init(LPCOLESTR)                               = 0;
    virtual HRESULT _IFUNC GetPart(IBPart * *, LPCOLESTR)          = 0;
};
//
// on linkable containers (all Containers with BOleLinkDoc helper)
// on linkable server documents (with BOleLinkDoc helper)
// on intermediate parts (in addition to IPart)
//

//
/// IBDocument -- Supported by BOleDocument one per container child window
//
class _ICLASS IBDocument : public IUnknown {
  public:
    virtual HRESULT _IFUNC Init(IBContainer *)                  = 0;
    virtual void    _IFUNC OnResize()                              = 0;
    virtual void    _IFUNC OnActivate(BOOL)                        = 0;
    virtual void    _IFUNC OnClose()                               = 0;
    virtual HRESULT _IFUNC OnSetFocus(BOOL)                        = 0;

    virtual HRESULT _IFUNC EnumLinks(IBLinkInfo * *)         = 0;
    virtual HRESULT _IFUNC BrowseLinks()                           = 0;
    virtual HRESULT _IFUNC UpdateLinks()                           = 0;
};

//
/// IBWindow -- Supported on child window *and* frame window of container apps
//
class _ICLASS IBWindow : public IUnknown {
  public:
    virtual HWND     _IFUNC GetWindow()                               = 0;
    virtual HRESULT  _IFUNC GetWindowRect(owl::TRect *)                 = 0;
    virtual LPCOLESTR _IFUNC GetWindowTitle()                         = 0;
    virtual void     _IFUNC AppendWindowTitle(LPCOLESTR)              = 0;
    virtual HRESULT  _IFUNC SetStatusText(LPCOLESTR)                  = 0;

    virtual HRESULT  _IFUNC RequestBorderSpace(const owl::TRect *)      = 0;
    virtual HRESULT  _IFUNC SetBorderSpace(const owl::TRect *)          = 0;

    virtual HRESULT  _IFUNC InsertContainerMenus(HMENU,
                                                 TOcMenuWidths *)  = 0;
    virtual HRESULT  _IFUNC SetFrameMenu(HMENU)                       = 0;
    virtual void     _IFUNC RestoreUI()                               = 0;
    virtual HRESULT  _IFUNC Accelerator(MSG *)                     = 0;
    virtual HRESULT  _IFUNC GetAccelerators(HACCEL *, int *)    = 0;
};

//
/// IBContainer -- Supported by container app's document window
//
class _ICLASS IBContainer : public IBWindow {
  public:
    virtual HRESULT  _IFUNC FindDropDest(owl::TPoint *, IBDropDest * *)=0;
    virtual HRESULT  _IFUNC AllowInPlace()                          = 0;
    virtual HRESULT  _IFUNC BringToFront()                          = 0;
};

//
/// IBDropDest -- Supported by container app's document window and/or other
///               areas inside the window.
//
class _ICLASS IBDropDest : public IBDataConsumer {
  public:
    virtual HRESULT  _IFUNC Drop(TOcInitInfo *, owl::TPoint *,
                                 const owl::TRect *)                    = 0;
    virtual void     _IFUNC DragFeedback(owl::TPoint *, const owl::TRect *,
                                         TOcMouseAction, owl::uint, HRESULT&) = 0;
    virtual HRESULT  _IFUNC Scroll(TOcScrollDir scrollDir)            = 0;
    virtual HRESULT  _IFUNC GetScrollRect(owl::TRect *)                 = 0;
};

//
/// IBApplication -- Supported at frame window/app object of container apps
//
class _ICLASS IBApplication : public IBWindow {
  public:
    virtual LPCOLESTR   _IFUNC GetAppName()                           = 0;
    virtual TOcHelp  _IFUNC HelpMode(TOcHelp)                         = 0;
    virtual HRESULT  _IFUNC CanLink()                                 = 0;
    virtual HRESULT  _IFUNC CanEmbed()                                = 0;
    virtual HRESULT  _IFUNC IsMDI()                                   = 0;
    virtual HRESULT  _IFUNC OnModalDialog(BOOL)                       = 0;
    virtual void     _IFUNC DialogHelpNotify(TOcDialogHelp)           = 0;
    virtual void     _IFUNC ShutdownMaybe()                           = 0;
};

//
/// IBSite -- Supported by container apps for each linked/embedded object
//
class _ICLASS IBSite : public IUnknown {
  public:
    virtual HRESULT _IFUNC SiteShow(BOOL)                             = 0;
    virtual HRESULT _IFUNC DiscardUndo()                              = 0;
    virtual HRESULT _IFUNC GetSiteRect(owl::TRect *, owl::TRect *)        = 0;
    virtual HRESULT _IFUNC SetSiteRect(const owl::TRect *)              = 0;
    virtual HRESULT _IFUNC SetSiteExtent(const owl::TSize *)            = 0;
    virtual HRESULT _IFUNC GetZoom(TOcScaleInfo * pScale)          = 0;
    virtual void    _IFUNC Invalidate(TOcInvalidate)                  = 0;
    virtual void    _IFUNC OnSetFocus(BOOL)                           = 0;
    virtual HRESULT _IFUNC Init(IBDataProvider *, IBPart *,
                                LPCOLESTR, BOOL)                      = 0;
    virtual void    _IFUNC Disconnect()                               = 0;
};

/// IBSite2 -- Add flexibility to have parent of site not be container
//
class _ICLASS IBSite2 : public IBSite {
   public:
     virtual HRESULT _IFUNC GetParentWindow(HWND *)               = 0;
};

// IBDataState
//
class _ICLASS IBDataState: public IUnknown {
  public:
    virtual void _IFUNC SetDirty (BOOL)                               = 0;
    virtual HRESULT _IFUNC IsDirty ()                                  = 0;
    virtual HRESULT _IFUNC GetClassID (LPCLSID)                        = 0;
};

//
// IBControlSite
//
class _ICLASS IBControlSite : public IUnknown
{
  public:
    virtual HRESULT _IFUNC Init(UINT, IBControl*, UINT)              = 0;
    virtual HRESULT _IFUNC OnPropertyChanged(DISPID dispid)          = 0;
    virtual HRESULT _IFUNC OnPropertyRequestEdit(DISPID dispid)      = 0;
    virtual HRESULT _IFUNC OnControlFocus(BOOL fGotFocus)            = 0;
    virtual HRESULT _IFUNC TransformCoords(owl::TPointL * lpptlHimetric,
                            owl::TPointF * lpptfContainer, DWORD flags)  = 0;
};

//
// IBControl
//
class _ICLASS IBControl : public IBPart //, public IBDataProvider2
{
  public:
    virtual HRESULT _IFUNC GetEventIID (IID *iidEvent)                 = 0;
    virtual HRESULT _IFUNC GetEventTypeInfo (ITypeInfo * * ppTypeInfo) = 0;
    virtual HRESULT _IFUNC GetCtrlDispatch (IDispatch * * ppDispatch)  = 0;
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
    virtual HRESULT _IFUNC GetClassInfo (LPTYPEINFO *)              = 0;
    virtual HRESULT _IFUNC GetSizeMax (ULARGE_INTEGER *)              = 0;
    virtual HRESULT _IFUNC Save (LPSTREAM)                              = 0;
};

//
// IBSinkList
//
class _ICLASS IBSinkList : public IUnknown {
  public:
    virtual HRESULT _IFUNC NextSink(LPVOID *)                     = 0;
    virtual HRESULT _IFUNC GetSinkAt(int, LPVOID *)               = 0;
    virtual HRESULT _IFUNC Reset()                                     = 0;
};

//
// IBEventsHandler
//
class _ICLASS IBEventsHandler : public IUnknown {
  public:
    virtual HRESULT _IFUNC RegisterEventsSet(REFIID, UINT)               = 0;
    virtual HRESULT _IFUNC RegisterConnectionPoint(LPCONNECTIONPOINT)   = 0;
    virtual HRESULT _IFUNC GetSinkListForIID(REFIID, IBSinkList**)      = 0;
};

//
// IBEventClass
//
class _ICLASS IBEventClass : public IUnknown {
  public:
    virtual HRESULT _IFUNC GetSinkList(IBSinkList **)                  = 0;
    virtual HRESULT _IFUNC FreezeEvents (BOOL)                          = 0;

};

//
/// IBLinkInfo -- Supported by BOleParts which are link objects
//
class _ICLASS IBLinkInfo : public IUnknown {
  public:
    virtual HRESULT _IFUNC SourceBreak()                              = 0;
    virtual HRESULT _IFUNC SourceGet(LPOLESTR, owl::uint count)            = 0;
    virtual HRESULT _IFUNC SourceSet(LPOLESTR)                        = 0;
    virtual HRESULT _IFUNC SourceOpen()                               = 0;
    virtual HRESULT _IFUNC UpdateGet(TOcLinkUpdate *)              = 0;
    virtual HRESULT _IFUNC UpdateSet(TOcLinkUpdate)                   = 0;
    virtual HRESULT _IFUNC UpdateNow()                                = 0;
};

//
/// IBClass -- Supported by BOleFactory
//
class _ICLASS IBClass : public IClassFactory {
  public:
    virtual HRESULT _IFUNC Init(BOOL selfEmbed, LPCOLESTR pszProgId,
                                IBClassMgr* pCM, BCID rid)            = 0;
    virtual HRESULT _IFUNC Register(BOOL singleUse)                   = 0;
    virtual HRESULT _IFUNC Revoke()                                   = 0;
    virtual HRESULT _IFUNC GetClassID(CLSID * pOut)                = 0;
    virtual BOOL _IFUNC IsA(CLSID & pOut)                          = 0;
    virtual BOOL _IFUNC AllowEmbedFromSelf()                          = 0;
    virtual HRESULT _IFUNC GetEmbeddingHelperFactory(LPUNKNOWN *ppF)  = 0;
};

//
/// IBOverrideBrowseLinks -- Supported by containers(who aggregate BOleDoc)
///                          who want to override our BrowseLinks dialog.
//
class _ICLASS IBOverrideBrowseLinks : public IUnknown {
  public:
    virtual HRESULT    _IFUNC BrowseLinks()                        = 0;
};

/// IBOverrideHelpButton -- Supported by container apps (who aggregate
///                         BOleService) who want to suppress the help
///                         button in our dialogs.
//
class _ICLASS IBOverrideHelpButton : public IUnknown {
  public:
    virtual HRESULT    _IFUNC ShowHelpButton(TOcDialogHelp) = 0;
};

//----------------------------------------------------------------------------
// Will eliminate this inline QI in favor of TUnknown version next rev.
//
inline HRESULT _IFUNC IUnknown_QueryInterface(IUnknown *, REFIID iid, LPVOID * pif)
{
  // To avoid handing out different IUnknown pointers for
  // the same Windows Object don't match here.
  // Only match in the main IUnknown in the outermost aggregator.
  //
  return ResultFromScode(E_NOINTERFACE);
}

#define DEFINE_INLINE_QI(I, P) \
inline HRESULT _IFUNC I##_QueryInterface(I * This, REFIID iid, LPVOID * pif) \
{                                            \
  return (iid == IID_##I) ?                  \
    (*pif = This), This->AddRef(), HR_OK : \
    P##_QueryInterface(This, iid, pif);      \
}

#define DEFINE_INLINE_QI2(I, P1, P2) \
inline HRESULT _IFUNC I##_QueryInterface(I * This, REFIID iid, LPVOID * pif) \
{                                            \
  return (iid == IID_##I) ?                  \
    (*pif = This), ((P1*)This)->AddRef(), HR_OK : \
    SUCCEEDED(P1##_QueryInterface(This, iid, pif))? HR_OK :\
    P2##_QueryInterface(This, iid, pif);      \
}

DEFINE_INLINE_QI(IBDataNegotiator, IUnknown)
DEFINE_INLINE_QI(IBDataConsumer, IUnknown)
DEFINE_INLINE_QI(IBDropDest, IBDataConsumer)
DEFINE_INLINE_QI(IBDataProvider, IBDataNegotiator)
DEFINE_INLINE_QI(IBPart, IBDataProvider)

DEFINE_INLINE_QI(IBWindow, IUnknown)
DEFINE_INLINE_QI(IBContainer, IBWindow)
DEFINE_INLINE_QI(IBApplication, IBWindow)

DEFINE_INLINE_QI(IBDocument, IUnknown)
DEFINE_INLINE_QI(IBService, IUnknown)
DEFINE_INLINE_QI(IBClassMgr, IUnknown)
DEFINE_INLINE_QI(IBLinkInfo, IUnknown)
DEFINE_INLINE_QI(IBSite, IUnknown)
DEFINE_INLINE_QI(IBSite2, IBSite)
DEFINE_INLINE_QI(IBContains, IUnknown)

DEFINE_INLINE_QI(IBLinkable, IUnknown)
DEFINE_INLINE_QI(IBRootLinkable, IBLinkable)
DEFINE_INLINE_QI(IClassFactory, IUnknown)
DEFINE_INLINE_QI(IBClass, IClassFactory)

DEFINE_INLINE_QI(IBOverrideBrowseLinks, IUnknown)
DEFINE_INLINE_QI(IBOverrideHelpButton, IUnknown)
DEFINE_INLINE_QI(IBDataProvider2, IBDataProvider)
DEFINE_INLINE_QI2(IBPart2, IBPart, IBDataProvider2)

DEFINE_INLINE_QI(IBService2, IBService)
DEFINE_INLINE_QI(IBControl, IBPart)
DEFINE_INLINE_QI(IBControlSite, IUnknown)
DEFINE_INLINE_QI(IBEventsHandler, IUnknown)
DEFINE_INLINE_QI(IBEventClass, IUnknown)
DEFINE_INLINE_QI(IBSinkList, IUnknown)
DEFINE_INLINE_QI(IBDataState, IUnknown)

//----------------------------------------------------------------------------

# include <poppack.h>

} // OCF namespace

#endif  // OCF_OCBOCOLE_H
