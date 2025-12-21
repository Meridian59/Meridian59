//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
///   Definition of Compound Document TOcView Class
//----------------------------------------------------------------------------

#if !defined(OCF_OCVIEW_H)
#define OCF_OCVIEW_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <ocf/ocdoc.h>      // Client related
#include <ocf/ocapp.h>      // Client related
#include <owl/dispatch.h>
#include <owl/contain.h>

namespace owl {class _OWLCLASS TRegLink;};
namespace owl {class _OWLCLASS TRegList;};

namespace ocf {
//
// Classes referenced
//
class _ICLASS TOcControl;
class _ICLASS TOcStorage;
class _ICLASS TOcDataProvider;

//
// OCF Mixin classes for users derived application class
//
#if !defined(_OCMCLASS)
# define _OCMCLASS
#endif
class _OCMCLASS TOcContainerHost;
class _OCMCLASS TOcServerHost;

//
// View options flags, set with SetOption
//
enum TOcViewOptions {
  voNoInPlace        = 0x0001, // If FALSE, allow inplace activate in container
  voNoNestedInPlace  = 0x0002, // If FALSE, allow nested inplace activate " "
  voNoInPlaceServer  = 0x0004, // If FALSE, allow server to activate inplace
};

//
/// \class TOcFormat
// ~~~~~ ~~~~~~~~~
/// Clipboard format wrapper
//
class _OCFCLASS TOcFormat {
  public:
    TOcFormat();
    TOcFormat(owl::uint fmtId, LPCTSTR fmtName, LPCTSTR fmtResultName,
              owl::uint fmtMedium, bool fmtIsLinkable,
              owl::uint aspect = 1, owl::uint direction = 1);

    void operator =(const TOcFormatInfo&);
    bool operator ==(const TOcFormat& other) {return owl::ToBool(&other == this);}
    void SetFormatId(owl::uint id) {Id = id;}
    void SetFormatName(LPTSTR name, TOcApp& ocApp);
    void SetFormatName(owl::uint id, TOcApp& ocApp);
    void SetMedium(owl::uint medium) {Medium = (owl::ocrMedium)medium;}
    void SetAspect(owl::uint aspect) {Aspect = aspect;}
    void SetDirection(owl::uint direction) {Direction = direction;}
    void SetLinkable(bool link = true) {IsLinkable = link;}
    void Disable(bool disable = true) {Disabled = disable;}

    owl::uint GetFormatId() const {return Id;}
    LPTSTR GetRegName() {return RegName;}
    LPTSTR GetFormatName() {return Name;}
    owl::uint GetMedium() const {return Medium;}
    owl::uint GetAspect() const {return Aspect;}
    owl::uint GetDirection() const {return Direction;}
    bool IsDisabled() const {return Disabled;}

    void GetFormatInfo(TOcFormatInfo & f);

  private:
    owl::uint      Id;
    _TCHAR    RegName[32];
    _TCHAR    Name[32];
    _TCHAR    ResultName[32];
    owl::ocrMedium Medium;
    bool      IsLinkable;

    owl::uint Aspect;      // last three data members added & maintained by ocf
    owl::uint Direction;
    bool Disabled;    // Format not available
};

//
/// \class TOcFormatList
// ~~~~~ ~~~~~~~~~~~~~
/// Container of clipboard formats with iterator
//
class _OCFCLASS TOcFormatList : public owl::TIPtrArray<TOcFormat*> {
  public:
    TOcFormatList();
   ~TOcFormatList();
    //void operator delete(void* ptr) {TStandardAllocator::operator delete(ptr);}

    TOcFormat*&  operator [](unsigned index)  {  return Base::operator[](index);}
    void  Clear(int del = 1)                  { Base::Flush(del==1);}
    int   Add(TOcFormat* format)              { return Base::Add(format);}
    int   IsEmpty() const                      {  return Base::Empty();}
    int   Find(const TOcFormat* format) const  { return Base::Find((TOcFormat*)format);}
    owl::uint  Count() const                        {  return Base::Size();}
    int   Detach(TOcFormat* item, bool del = false)
            {
              return del ? Base::DestroyItem(item) : Base::DetachItem(item);
            }
    TOcFormat* Find(owl::uint const id) const;

  private:
    typedef owl::TIPtrArray<TOcFormat*> Base;
};

// class TOcFormatListIter
// ~~~~~ ~~~~~~~~~~~~~~~~~
typedef TOcFormatList::Iterator TOcFormatListIter;

//
/// \class TOcView
// ~~~~~ ~~~~~~~
/// The TOcView partner is a container (viewer) of a given (server/client)
/// document.
//
class _ICLASS TOcView : public TUnknown,
                        public IBContainer,
                        public IBContains,
                        public IBDropDest {
  public:
    TOcView(TOcDocument& doc, TOcContainerHost* ch, TOcServerHost* sh,
            owl::TRegList* regList=0, IUnknown* outer=0);

    // !CQ compatibility only
    TOcView(TOcDocument& doc, owl::TRegList* regList=0, IUnknown* outer=0);

    void         SetupWindow(HWND hWin, bool embedded = false);

    virtual void ReleaseObject();

    TOcDocument& GetOcDocument() {return OcDocument;}

    // Helper functions
    //
    IBRootLinkable* GetLinkable() {return BLDocumentI;}

    /// \name Clipboard support
    /// @{
    bool         RegisterClipFormats(owl::TRegList& regList);
    bool         BrowseClipboard(TOcInitInfo& initInfo);
    bool         PasteNative(TOcInitInfo& init, owl::TPoint * where = 0);
    bool         Paste(bool linking = false);
    void         SetOcData(TOcDataProvider* ocData = 0) {OcData = ocData;}
    TOcDataProvider* GetOcData() {return OcData;}
    /// @}

    /// \name View related
    /// @{
    owl::TPoint       GetOrigin() const {return Origin;}
    owl::TRect        GetWindowRect() const;
    void         ScrollWindow(int dx, int dy);
    void         InvalidatePart(TOcPartChangeInfo& changeInfo);
    virtual void Rename();
    TOcLinkView* GetDocLink(LPCTSTR name=0);
    bool         IsOptionSet(owl::uint32 option) const;
    void         SetOption(owl::uint32 bit, bool state);
    /// @}

    /// \name Get/Set active part
    /// @{
    TOcPart*     GetActivePart() {return ActivePart;}
    void         SetActivePart(TOcPart* part) {ActivePart = part;} // Internal use
    bool         ActivatePart(TOcPart* part);
    /// @}

    /// \name IBDocument pass-thrus
    /// @{
    void         EvResize();
    void         EvActivate(bool activate);
    virtual void EvClose();
    virtual bool EvSetFocus(bool set);
    bool         EnumLinks(IBLinkInfo * *);
    bool         BrowseLinks();
    bool         UpdateLinks();
    /// @}

    /// \name Object reference & lifetime managment
    /// For internal OCF use only
    /// @{
    owl::ulong   _IFUNC AddRef() {return GetOuter()->AddRef();}
    owl::ulong   _IFUNC Release() {return GetOuter()->Release();}
    HRESULT _IFUNC QueryInterface(const GUID & iid, void ** iface)
                     {return GetOuter()->QueryInterface(iid, iface);}
    /// @}

  protected:
    /// \name IBContains implementation for BOle to use
    /// @{
    HRESULT _IFUNC Init(LPCOLESTR);
    HRESULT _IFUNC GetPart(IBPart * *, LPCOLESTR);
    /// @}

    /// \name IBDataNegotiator implementation
    /// @{
    owl::uint     _IFUNC CountFormats();
    HRESULT  _IFUNC GetFormat(owl::uint, TOcFormatInfo *);
    /// @}

    /// \name IBWindow implementation
    /// @{
    HWND     _IFUNC GetWindow();
    HRESULT  _IFUNC GetWindowRect(owl::TRect * r);
    LPCOLESTR _IFUNC GetWindowTitle();
    void     _IFUNC AppendWindowTitle(LPCOLESTR title);
    HRESULT  _IFUNC SetStatusText(LPCOLESTR text);
    HRESULT  _IFUNC RequestBorderSpace(const owl::TRect *);
    HRESULT  _IFUNC SetBorderSpace(const owl::TRect *);
    HRESULT  _IFUNC InsertContainerMenus(HMENU, TOcMenuWidths *);
    HRESULT  _IFUNC SetFrameMenu(HMENU);
    void     _IFUNC RestoreUI();
    HRESULT  _IFUNC Accelerator(MSG *);
    HRESULT  _IFUNC GetAccelerators(HACCEL *, int *);
    /// @}

    /// \name IBDropDest implementation
    /// @{
    HRESULT  _IFUNC Drop(TOcInitInfo *, owl::TPoint *, const owl::TRect *);
    void     _IFUNC DragFeedback(owl::TPoint *, const owl::TRect *, TOcMouseAction, owl::uint, HRESULT& hr);
    HRESULT  _IFUNC Scroll(TOcScrollDir scrollDir);
    HRESULT  _IFUNC GetScrollRect(owl::TRect *);
    /// @}

    /// \name IBContainer implementation
    /// @{
    HRESULT  _IFUNC FindDropDest(owl::TPoint *, IBDropDest * *);
    HRESULT  _IFUNC AllowInPlace();
    HRESULT  _IFUNC BringToFront();
    /// @}

  protected:
    // !CQ hacked in to support occtrl for now
    owl::TResult ForwardEvent(int eventId, const void* param);
    owl::TResult ForwardEvent(int eventId, owl::TParam2 param = 0);

  protected:
   ~TOcView();
    void Shutdown();// called from derived class destructor to release helpers

    // TUnknown overrides
    //
    HRESULT      QueryObject(const IID & iid, void * * iface);

    // BOle side support
    //
    IUnknown*       BDocument;   ///< Document helper object
    IBDocument*     BDocumentI;  ///< Document interface on the document
    IUnknown*       BContainer;  ///< Container helper object
    IBRootLinkable* BLDocumentI; ///< RootLinkable interface on the container

    // OC wiring
    //
    TOcApp&         OcApp;        ///< Our OC application object
    TOcDocument&    OcDocument;   ///< Our OC document object

    TOcContainerHost* ContainerHost; ///< The hosting app's container object
    TOcServerHost*    ServerHost;    ///< The hosting app's server object

    // App side support
    //
// !CQ ripped out    HWND            Win;          // Actual app container window
    TOcPart*          ActivePart;   ///< Currently active part, if any

    owl::uint32            Options;

    owl::TPoint            Origin;
    owl::TSize              Extent;
    owl::tstring        WinTitle;
    TOcFormatList      FormatList;
    owl::TString            OrgTitle;     ///< Original window title string
    int                LinkFormat;   ///< adjustment for format count
    owl::TRegList*          RegList;
    TOcDataProvider*  OcData;       ///< Dataprovider representing this view

  friend class _ICLASS TOcControl;
  friend class _ICLASS TOcControlEvent;
  friend class _ICLASS TOcPart;
  friend class _ICLASS TOcDataProvider;
  friend class _ICLASS TOcLinkView;
};

//----------------------------------------------------------------------------

//
/// For viewdrop & viewdrag
//
struct TOcDragDrop {
  TOcInitInfo *  InitInfo;  ///< ViewDrop event only, else 0
  owl::TPoint*            Where;
  owl::TRect*            Pos;
};

//
/// For part adornment painting over part, & painting views
//
struct TOcViewPaint {
  HDC        DC;
  owl::TRect*    Pos;
  owl::TRect*    Clip;
  TOcAspect  Aspect;
  bool      PaintSelection;  ///< paint the selection only
  owl::TString*  Moniker;         ///< moniker if any
  void*      UserData;        ///< User data

};

//
/// Use when doing parts save and load
//
class TOcSaveLoad {
  public:
    TOcSaveLoad()
      : StorageI(0), SameAsLoad(false), Remember(true), SaveSelection(false),
        UserData(0) {}
    TOcSaveLoad(IStorage* storageI, bool sameAsLoad = true, bool remember = false,
                bool saveSelection = false, void* userData = 0)
    {
      StorageI      = storageI;
      SameAsLoad    = sameAsLoad;
      Remember      = remember;
      SaveSelection = saveSelection;
      UserData      = userData;
    }

  public:
    IStorage * StorageI;
    bool          SameAsLoad;      ///< Same IStorage as one used for loading
    bool          Remember;        ///< whether to keep the IStorage ptr after save/load
    bool          SaveSelection;   ///< whether to save only the selection
    void*         UserData;        ///< User data
};

//
//
//
struct TOcToolBarInfo {
  bool  Show;        ///< whether this is a show or a hide
  HWND  HFrame;      ///< Container frame for show, Server frame for hide
  HWND  HLeftTB;
  HWND  HTopTB;
  HWND  HRightTB;
  HWND  HBottomTB;
};

//
/// Used to obtain the size of the rectangle that encloses the selection
//
class TOcPartSize {
  public:
    TOcPartSize()
    :
      PartRect(0, 0, 0, 0),
      Selection(false),
      Moniker(0),
      UserData(0)
    {
    }

    TOcPartSize(owl::TRect rect, bool selection = false, owl::TString* moniker = 0, void* userData = 0)
    :
      PartRect(rect),
      Selection(selection),
      Moniker(moniker),
      UserData(userData)
    {
    }

    TOcPartSize(bool selection, owl::TString* moniker = 0, void* userData = 0)
    :
      PartRect(0, 0, 0, 0),
      Selection(selection),
      Moniker(moniker),
      UserData(userData)
    {
    }

  public:
    owl::TRect      PartRect;     ///< rect enclosing the whole/part of the embedded object
    bool      Selection;    ///< whether we want rect for the whole or part(selection)
    owl::TString*  Moniker;      ///< we want the selection rect for moniker
    void*      UserData;     ///< User data
};

//
/// Used to obtain the item name for building monikers
//
class TOcItemName {
  public:
    TOcItemName() : Selection(false) {}
    TOcItemName(bool selection) : Selection(selection) {}

  public:
    owl::TString Name;        ///< Item moniker
    bool    Selection;   ///< Whether we want name for the whole or part(selection)
};

//
/// Used to obtain the native clipboard format data
//
class TOcFormatData {
  public:
    TOcFormatData(TOcFormat& format, void* userData = 0, HANDLE handle = 0,
                  bool paste = false, owl::TPoint * where = 0)
    :
      Format(format), UserData(userData), Handle(handle),
      Paste(paste), Where(where) {}

  public:
    TOcFormat&  Format;        ///< Clipboard format
    void*        UserData;      ///< User data for mapping to a selection
    HANDLE      Handle;        ///< clipboard data in handle
    bool        Paste;         ///< are we doing a paste?
    owl::TPoint *  Where;   ///< Where to drop this data
};

//
/// Used to obtain the item name for building monikers
//
class TOcItemLink {
  public:
    TOcItemLink(LPCTSTR moniker, TOcLinkView* view)
    :
      Moniker(moniker),
      OcLinkView(view)
    {
    }

  public:
    LPCTSTR        Moniker;      ///< item moniker

/// The TOcLinkView connector object associated with this view.
    TOcLinkView*  OcLinkView;
};

//
// class TOcScaleFactor
// ~~~~~ ~~~~~~~~~~~~~~
class _OCFCLASS TOcScaleFactor {
  public:
    TOcScaleFactor();
    TOcScaleFactor(const owl::TRect& siteRect, const owl::TSize& partSize);
    TOcScaleFactor(const TOcScaleInfo & scaleInfo);

    TOcScaleFactor& operator =(const TOcScaleInfo & scaleInfo);
    TOcScaleFactor& operator =(const TOcScaleFactor& scaleFactor);
    void GetScaleFactor(TOcScaleInfo & scaleInfo) const;

    bool IsZoomed();
    void SetScale(owl::uint16 percent);
    owl::uint16 GetScale();
    void Reset();

  public:
    owl::TSize SiteSize;
    owl::TSize  PartSize;
};

/// \cond
_OCFCFUNC(owl::ipstream&) operator >>(owl::ipstream& is, TOcScaleFactor& sf);
_OCFCFUNC(owl::opstream&) operator <<(owl::opstream& os, const TOcScaleFactor& sf);
/// \endcond

//
// class TOcSiteRect
// ~~~~~ ~~~~~~~~~~~
class _OCFCLASS TOcSiteRect {
  public:
    TOcSiteRect();
    TOcSiteRect(TOcPart *part, const owl::TRect& rect);
    owl::TRect      Rect;
    TOcPart*  Part;
};

//----------------------------------------------------------------------------

//
// class TOcContainerHost
// ~~~~~ ~~~~~~~~~~~~~~~~
class _OCMCLASS TOcContainerHost {
  public:
    // OC-Host support
    //
    virtual void  ReleaseOcObject() = 0;

    // Container methods
    //
    virtual bool    EvOcViewOpenDoc(LPCTSTR path) = 0;
    virtual LPCTSTR EvOcViewTitle() = 0;
    virtual void    EvOcViewSetTitle(LPCTSTR title) = 0;

    virtual bool    EvOcViewBorderSpaceReq(const owl::TRect * rect) = 0;
    virtual bool    EvOcViewBorderSpaceSet(const owl::TRect * rect) = 0;
    virtual bool    EvOcViewDrag(TOcDragDrop & ddInfo) = 0;
    virtual bool    EvOcViewDrop(TOcDragDrop & ddInfo) = 0;
    virtual bool    EvOcViewScroll(TOcScrollDir scrollDir) = 0;
    virtual bool    EvOcPartInvalid(TOcPartChangeInfo& part) = 0;
    virtual bool    EvOcViewPasteObject(TOcInitInfo& init) = 0;

    // From site (TOcPart)
    //
//    virtual bool   EvOcViewGetSiteRect(owl::TRect * rect) = 0;
//    virtual bool   EvOcViewSetSiteRect(owl::TRect * rect) = 0;
    virtual bool    EvOcViewGetSiteRect(TOcSiteRect& sr) = 0;
    virtual bool    EvOcViewSetSiteRect(TOcSiteRect& sr) = 0;
    virtual bool    EvOcViewPartActivate(TOcPart& ocPart) = 0;
    virtual bool    EvOcViewGetScale(TOcScaleFactor& scaleFactor) = 0;

    virtual HWND   EvOcGetWindow() const = 0;

    // For compatibility, don't need to implement
    //
    virtual void  SetWindow(HWND) {}
};

/*
//
// class TOcCtrlContainerHost
// ~~~~~ ~~~~~~~~~~~~~~~~~~~~
class _OCMCLASS TOcCtrlContainerHost {
  public:
    // OC-Host support
    //
    virtual void  ReleaseOcObject() = 0;

    // Control methods
    //
    virtual bool   EvOcViewTransformCoords(owl::uint verb) = 0;

    // Ambient property methods
    //
    virtual bool   EvOcAmbientGetBackColor(long* rgb) = 0;
    virtual bool   EvOcAmbientGetForeColor(long* rgb) = 0;
    virtual bool   EvOcAmbientGetLocaleID(long* locale) = 0;
    virtual bool   EvOcAmbientGetTextAlign(short* align) = 0;
    virtual bool   EvOcAmbientGetMessageReflect(bool* msgReflect) = 0;
    virtual bool   EvOcAmbientGetUserMode(bool* mode) = 0;
    virtual bool   EvOcAmbientGetUIDead(bool* dead) = 0;
    virtual bool   EvOcAmbientGetShowGrabHandles(bool* show) = 0;
    virtual bool   EvOcAmbientGetShowHatching(bool* show) = 0;
    virtual bool   EvOcAmbientGetSupportsMnemonics(bool* support) = 0;
    virtual bool   EvOcAmbientGetDisplayName(owl::TString** name) = 0;
    virtual bool   EvOcAmbientGetScaleUnits(owl::TString** units) = 0;
    virtual bool   EvOcAmbientGetFont(IDispatch** font) = 0;

    virtual bool   EvOcAmbientSetBackColor(long rgb) = 0;
    virtual bool   EvOcAmbientSetForeColor(long rgb) = 0;
    virtual bool   EvOcAmbientSetLocaleID(long locale) = 0;
    virtual bool   EvOcAmbientSetTextAlign(short align) = 0;
    virtual bool   EvOcAmbientSetMessageReflect(bool msgReflect) = 0;
    virtual bool   EvOcAmbientSetUserMode(bool mode) = 0;
    virtual bool   EvOcAmbientSetUIDead(bool dead) = 0;
    virtual bool   EvOcAmbientSetShowGrabHandles(bool show) = 0;
    virtual bool   EvOcAmbientSetShowHatching(bool show) = 0;
    virtual bool   EvOcAmbientSetSupportsMnemonics(bool support) = 0;
    virtual bool   EvOcAmbientSetDisplayName(owl::TString* name) = 0;
    virtual bool   EvOcAmbientSetScaleUnits(owl::TString* units) = 0;
    virtual bool   EvOcAmbientSetFont(IDispatch* font) = 0;

    // Ctrl event methods
    //
    virtual bool   EvOcCtrlClick(TCtrlEvent* pev) = 0;
    virtual bool   EvOcCtrlDblClick(TCtrlEvent* pev) = 0;
    virtual bool   EvOcCtrlMouseDown(TCtrlMouseEvent* pev) = 0;
    virtual bool   EvOcCtrlMouseMove(TCtrlMouseEvent* pev) = 0;
    virtual bool   EvOcCtrlMouseUp(TCtrlMouseEvent* pev) = 0;
    virtual bool   EvOcCtrlKeyDown(TCtrlKeyEvent* pev) = 0;
    virtual bool   EvOcCtrlKeyUp(TCtrlKeyEvent* pev) = 0;
    virtual bool   EvOcCtrlErrorEvent(TCtrlErrorEvent* pev) = 0;
    virtual bool   EvOcCtrlFocus(TCtrlFocusEvent* pev) = 0;
    virtual bool   EvOcCtrlPropertyChange(TCtrlPropertyEvent* pev) = 0;
    virtual bool   EvOcCtrlPropertyRequestEdit(TCtrlPropertyEvent* pev) = 0;
    virtual bool   EvOcCtrlCustomEvent(TCtrlCustomEvent* pev) = 0;
};
*/

//
// class TOcServerHost
// ~~~~~ ~~~~~~~~~~~~~
class _OCMCLASS TOcServerHost {
  public:
    // OC-Host support
    //
    virtual void  ReleaseOcObject() = 0;

    // Data server methods
    //
    virtual bool   EvOcViewGetItemName(TOcItemName& item) = 0;
    virtual bool   EvOcViewClipData(TOcFormatData & format) = 0;
    virtual bool   EvOcViewSetData(TOcFormatData & format) = 0;
    virtual bool   EvOcViewPartSize(TOcPartSize & size) = 0;
    virtual bool   EvOcViewSavePart(TOcSaveLoad & ocSave) = 0;
    virtual bool   EvOcViewPaint(TOcViewPaint & vp) = 0;

    // Link server methods
    //
    virtual bool   EvOcViewAttachWindow(bool attach) = 0;
    virtual void   EvOcViewSetTitle(LPCTSTR title) = 0; // Open editing
    virtual bool   EvOcViewDrag(TOcDragDrop & ddInfo) = 0;   // drag from server
    virtual bool   EvOcViewSetLink(TOcLinkView& view) = 0;
    virtual bool   EvOcViewBreakLink(TOcLinkView& view) = 0;
    virtual bool   EvOcViewGetPalette(LOGPALETTE * * palette) = 0;

    // Embed server methods
    //
    virtual bool   EvOcViewLoadPart(TOcSaveLoad & ocLoad) = 0;
    virtual bool   EvOcViewInsMenus(TOcMenuDescr & sharedMenu) = 0;
    virtual bool   EvOcViewShowTools(TOcToolBarInfo & tbi) = 0;

    virtual bool   EvOcViewClose() = 0;
    virtual bool   EvOcViewSetScale(TOcScaleFactor& scaleFactor) = 0;
    virtual bool   EvOcViewDoVerb(owl::uint verb) = 0;

    virtual HWND   EvOcGetWindow() const = 0;

    // For compatibility, don't need to implement
    //
    virtual void  SetWindow(HWND) {}
};

//----------------------------------------------------------------------------

//
// class TOcContainerHostMsg
// ~~~~~ ~~~~~~~~~~~~~~~~~~~
class _OCMCLASS TOcContainerHostMsg : public TOcContainerHost {
  public:
    TOcContainerHostMsg() : Wnd(0) {}

    // OC-Host support
    //
    virtual void    ReleaseOcObject() {delete this;}

    // Container methods
    //
    virtual bool    EvOcViewOpenDoc(LPCTSTR path);
    virtual LPCTSTR EvOcViewTitle();
    virtual void    EvOcViewSetTitle(LPCTSTR title);

    virtual bool    EvOcViewBorderSpaceReq(const owl::TRect * rect);
    virtual bool    EvOcViewBorderSpaceSet(const owl::TRect * rect);
    virtual bool    EvOcViewDrag(TOcDragDrop & ddInfo);
    virtual bool    EvOcViewDrop(TOcDragDrop & ddInfo);
    virtual bool    EvOcViewScroll(TOcScrollDir scrollDir);
    virtual bool    EvOcPartInvalid(TOcPartChangeInfo& part);
    virtual bool    EvOcViewPasteObject(TOcInitInfo& init);

    // From site (TOcPart)
    //
//    virtual bool   EvOcViewGetSiteRect(owl::TRect * rect);
//    virtual bool   EvOcViewSetSiteRect(owl::TRect * rect);
    virtual bool    EvOcViewGetSiteRect(TOcSiteRect& sr);
    virtual bool    EvOcViewSetSiteRect(TOcSiteRect& sr);
    virtual bool    EvOcViewPartActivate(TOcPart& ocPart);
    virtual bool    EvOcViewGetScale(TOcScaleFactor& scaleFactor);

    virtual HWND    EvOcGetWindow() const {return Wnd;}

    // For compatibility
    //
    virtual void    SetWindow(HWND hWnd) {Wnd = hWnd;}

  protected:
    owl::TResult ForwardEvent(int eventId, const void* param);
    owl::TResult ForwardEvent(int eventId, owl::TParam2 param = 0);

  private:
    HWND   Wnd;  // For sending messages to real host window, & window ops

  friend TOcView;  // !CQ hacked in to support occtrl for now
};

//
// class TOcServerHostMsg
// ~~~~~ ~~~~~~~~~~~~~~~~
class _OCMCLASS TOcServerHostMsg : public TOcServerHost {
  public:
    TOcServerHostMsg() : Wnd(0) {}

    // OC-Host support
    //
    virtual void  ReleaseOcObject() {delete this;}

    // Data server methods
    //
    virtual bool   EvOcViewGetItemName(TOcItemName& item);
    virtual bool   EvOcViewClipData(TOcFormatData & format);
    virtual bool   EvOcViewSetData(TOcFormatData & format);
    virtual bool   EvOcViewPartSize(TOcPartSize & size);
    virtual bool   EvOcViewSavePart(TOcSaveLoad & ocSave);
    virtual bool   EvOcViewPaint(TOcViewPaint & vp);

    // Link server methods
    //
    virtual bool   EvOcViewAttachWindow(bool attach);
    virtual void   EvOcViewSetTitle(LPCTSTR title); // Open editing
    virtual bool   EvOcViewDrag(TOcDragDrop & ddInfo);   // drag from server
    virtual bool   EvOcViewSetLink(TOcLinkView& view);
    virtual bool   EvOcViewBreakLink(TOcLinkView& view);
    virtual bool   EvOcViewGetPalette(LOGPALETTE * * palette);

    // Embed server methods
    //
    virtual bool   EvOcViewLoadPart(TOcSaveLoad & ocLoad);
    virtual bool   EvOcViewInsMenus(TOcMenuDescr & sharedMenu);
    virtual bool   EvOcViewShowTools(TOcToolBarInfo & tbi);

    virtual bool   EvOcViewClose();
    virtual bool   EvOcViewSetScale(TOcScaleFactor& scaleFactor);
    virtual bool   EvOcViewDoVerb(owl::uint verb);

    virtual HWND   EvOcGetWindow() const {return Wnd;}

    // For compatibility
    //
    virtual void  SetWindow(HWND hWnd) {Wnd = hWnd;}

  protected:
    owl::TResult ForwardEvent(int eventId, const void* param);
    owl::TResult ForwardEvent(int eventId, owl::TParam2 param = 0);

  private:
    HWND   Wnd;  // For sending messages to real host window
};

// !CQ hacked in to support occtrl for now
//
inline owl::TResult
TOcView::ForwardEvent(int eventId, const void* param)
{
  return dynamic_cast<TOcContainerHostMsg*>(ContainerHost)->
           ForwardEvent(eventId, param);
}

inline owl::TResult
TOcView::ForwardEvent(int eventId, owl::TParam2 param)
{
  return dynamic_cast<TOcContainerHostMsg*>(ContainerHost)->
           ForwardEvent(eventId, param);
}


//----------------------------------------------------------------------------
// Subdispatch IDs for TOcView clients
//
                                    // Container
#define OC_VIEWOPENDOC        0x0200  // ask container to open itself
#define OC_VIEWTITLE          0x0201  // Get view window title
#define OC_VIEWSETTITLE       0x0202  // Set view window title
#define OC_VIEWBORDERSPACEREQ 0x0203  // Request/Set border space in view
#define OC_VIEWBORDERSPACESET 0x0204  // Request/Set border space in view
#define OC_VIEWDRAG           0x0205  // Provide drag feedback (c&s)
#define OC_VIEWDROP           0x0206  // Accept a drop
#define OC_VIEWSCROLL         0x0207  // Scroll view
#define OC_VIEWPARTINVALID    0x0208  // a part needs repainting
#define OC_VIEWPASTEOBJECT    0x0209  // let container know a part is pasted

#define OC_VIEWGETSITERECT    0x020A  // Ask container for the site rect
#define OC_VIEWSETSITERECT    0x020B  // Ask container to set the site rect
#define OC_VIEWPARTACTIVATE   0x020C  // Let container know the part is activated
#define OC_VIEWGETSCALE       0x020D  // Ask container to give scaling info

                                    // Data Server
#define OC_VIEWGETITEMNAME    0x0301  // Ask container to name its content or selection
#define OC_VIEWCLIPDATA       0x0302  // Ask for a clip data
#define OC_VIEWSETDATA        0x0303  // Set format data into server
#define OC_VIEWPARTSIZE       0x0304  // Ask server for a its extent
#define OC_VIEWSAVEPART       0x0305  // Ask server to save document
#define OC_VIEWPAINT          0x0306  // Paint remote view. Like WM_PAINT + extra

                                    // Link Server
#define OC_VIEWATTACHWINDOW   0x0310  // RL: ask server to attach to its owner window
#define OC_VIEWSETLINK        0x0311  // Ask to establish link to item
#define OC_VIEWBREAKLINK      0x0312  // Ask server to break link to item
#define OC_VIEWGETPALETTE     0x0313  // RL: ask server for its logical palette

                                    // Embed Server
#define OC_VIEWLOADPART       0x0321  // R: ask server to load document
#define OC_VIEWINSMENUS       0x0322  // R: ask server to insert its menus in mbar
#define OC_VIEWSHOWTOOLS      0x0323  // R: ask server to show its tools
#define OC_VIEWCLOSE          0x0324  // R: tell server to close this remote view
#define OC_VIEWSETSCALE       0x0325  // R: ask server to handle scaling
#define OC_VIEWDOVERB         0x0326  // R: ask server to do a verb

// Control event notification from control part to container view
//
#define OC_VIEWTRANSFORMCOORDS         0x0330 // C: view to transform ctrl coords
#define OC_CTRLEVENT_FOCUS             0x0340  // Ctrl got/lost focus notify
#define OC_CTRLEVENT_PROPERTYCHANGE    0x0341  // Ctrl prop changed notify
#define OC_CTRLEVENT_PROPERTYREQUESTEDIT 0x0342// Ctrl prop request edit
#define OC_CTRLEVENT_CLICK             0x0343  // Ctrl click notify
#define OC_CTRLEVENT_DBLCLICK          0x0344  // Ctrl dblclick notify
#define OC_CTRLEVENT_MOUSEDOWN         0x0345  // Ctrl mouse down notify
#define OC_CTRLEVENT_MOUSEMOVE         0x0346  // Ctrl mouse move notify
#define OC_CTRLEVENT_MOUSEUP           0x0347  // Ctrl mouse up notify
#define OC_CTRLEVENT_KEYDOWN           0x0348  // Ctrl key down notify
#define OC_CTRLEVENT_KEYUP             0x0349  // Ctrl key up notify
#define OC_CTRLEVENT_ERROREVENT        0x034A  // Ctrl error event notify
#define OC_CTRLEVENT_CUSTOMEVENT       0x034B  // Ctrl custom event notify

// Control request for view ambient properties from container view
//
#define OC_AMBIENT_GETBACKCOLOR        0x0350
#define OC_AMBIENT_GETFORECOLOR        0x0351
#define OC_AMBIENT_GETSHOWGRABHANDLES  0x0352
#define OC_AMBIENT_GETUIDEAD           0x0353
#define OC_AMBIENT_GETSUPPORTSMNEMONICS 0x0354
#define OC_AMBIENT_GETSHOWHATCHING     0x0355
#define OC_AMBIENT_GETDISPLAYASDEFAULT 0x0356
#define OC_AMBIENT_GETTEXTALIGN        0x0357
#define OC_AMBIENT_GETMESSAGEREFLECT   0x0358
#define OC_AMBIENT_GETLOCALEID         0x0359
#define OC_AMBIENT_GETUSERMODE         0x035A
#define OC_AMBIENT_GETDISPLAYNAME      0x035B
#define OC_AMBIENT_GETSCALEUNITS       0x035C
#define OC_AMBIENT_GETFONT             0x035D

// Control set view ambient properties
//
#define OC_AMBIENT_SETBACKCOLOR        0x0360
#define OC_AMBIENT_SETFORECOLOR        0x0361
#define OC_AMBIENT_SETSHOWGRABHANDLES  0x0362
#define OC_AMBIENT_SETUIDEAD           0x0363
#define OC_AMBIENT_SETSUPPORTSMNEMONICS 0x0364
#define OC_AMBIENT_SETSHOWHATCHING     0x0365
#define OC_AMBIENT_SETDISPLAYASDEFAULT 0x0366
#define OC_AMBIENT_SETTEXTALIGN        0x0367
#define OC_AMBIENT_SETMESSAGEREFLECT   0x0368
#define OC_AMBIENT_SETLOCALEID         0x0369
#define OC_AMBIENT_SETUSERMODE         0x036A
#define OC_AMBIENT_SETDISPLAYNAME      0x036B
#define OC_AMBIENT_SETSCALEUNITS       0x036C
#define OC_AMBIENT_SETFONT             0x036D

#define OC_USEREVENT                   0xF000  // User defined events

//----------------------------------------------------------------------------
// Inline Implementations
//

//
inline bool TOcView::IsOptionSet(owl::uint32 option) const
{
  return owl::ToBool(Options & option);
}

//
inline void TOcView::SetOption(owl::uint32 bit, bool state)
{
  if (state)
    Options |= bit;
  else
    Options &= ~bit;
}

} // OCF namespace

#endif  // OCF_OCVIEW_H

