//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// Definition of TOcPart class
//----------------------------------------------------------------------------

#if !defined(OCF_OCPART_H)
#define OCF_OCPART_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/geometry.h>
#include <owl/contain.h>

#include <ocf/ocbocole.h>
#include <ocf/ocobject.h>
#include <ocf/autodefs.h>


namespace ocf {

//
// Classes referenced
//
class _ICLASS TOcStorage;
class _ICLASS TOcDocument;
class TOcVerb;

//
/// \class TOcPart
// ~~~~~ ~~~~~~~
/// OC part class represents an embeded or linked part in a document
//
class _ICLASS TOcPart : public TUnknown, protected IBSite2 {
  public:
    TOcPart(TOcDocument& document, int id = 0);
    TOcPart(TOcDocument& document, LPCTSTR name);
    TOcPart(TOcDocument& document, TOcInitInfo & initInfo, owl::TRect pos,
            int id = 0);

    virtual bool Init(TOcInitInfo * initInfo, owl::TRect pos);

    void    Delete();

    bool    operator ==(const TOcPart& other) {return owl::ToBool(&other == this);}

    // Delegated doc functions from TOcDocument
    //
    IBPart*     GetBPartI()  {return BPartI;}
    TOcStorage* GetStorage() {return Storage;}
    void        SetStorage(IStorage* storage, bool remember = true);
    bool    Save(bool SameAsLoad = true, bool remember = true);
    bool    Load();
    int     Detach();
    void    FinishLoading();

    // Position & size information
    //
    owl::TPoint  GetPos() const {return Pos;}
    void    SetPos(const owl::TPoint& pos);
    owl::TSize   GetSize() const;
    void    SetSize(const owl::TSize& size);
    owl::TRect   GetRect() const {return owl::TRect(Pos, Size);}
    void    UpdateRect();
    bool    IsVisible(const owl::TRect& logicalRect) const
              {return owl::ToBool(IsVisible() && logicalRect.Touches(GetRect()));}

    // Flag accessor functions
    //
    void    Select(bool select)
              {if (select) Flags |= Selected; else Flags &= ~Selected;}
    bool    IsSelected() const {return owl::ToBool(Flags&Selected);}
    void    SetVisible(bool visible);
    bool    IsVisible() const {return owl::ToBool(Flags&Visible);}
    bool    IsActive() const {return owl::ToBool(Flags&Active);}
    void    SetActive() {Flags |= Active;}
    bool    IsLink() const {return owl::ToBool(Flags&Link);}

    // Naming
    //
    // Changed the following line as there seems to be a problem with the
    // destruction of TString objects when they are constructed from an
    // owl::tstring. This causes an exception when the TString returned from
    // GetName() goes out of scope. MH - 19/06/02
    //
    // TString GetName() {return Name;}
    owl::TString GetName() {return Name.c_str();}
    void    Rename();

    // Pass thru functions to corresponding IBPart
    //
    bool    Save(IStorage* storage, bool sameAsLoad, bool remember);
    bool    Draw(HDC dc, const owl::TRect& pos, const owl::TRect& clip, TOcAspect aspect = asDefault);
    bool    Activate(bool activate);
    bool    Show(bool show) {return HRSucceeded(BPartI->Show(show));}
    bool    Open(bool open) {return HRSucceeded(BPartI->Open(open));}
    HWND    OpenInPlace(HWND hwnd) {return BPartI->OpenInPlace(hwnd);}
    bool    CanOpenInPlace() {return HRSucceeded(BPartI->CanOpenInPlace());}
    bool    Close();
    bool    SetHost(IBContainer * container)
              {return HRSucceeded(BPartI->SetHost(container));}

    bool    EnumVerbs(const TOcVerb&);
    bool    DoVerb(owl::uint);
    LPCOLESTR    GetServerName(TOcPartName partName)
        {return const_cast<LPOLESTR>(BPartI->GetName((TOcPartName)partName));}
    HRESULT QueryServer(const IID & iid, void * * iface);

    bool    SetFormatData(TOcFormatInfo * fmt, HANDLE data, bool release);

    // Object reference & lifetime managment
    // For internal OCF use only
    //
    owl::ulong   _IFUNC AddRef() {return GetOuter()->AddRef();}
    owl::ulong   _IFUNC Release() {return GetOuter()->Release();}
    HRESULT _IFUNC QueryInterface(const GUID & iid, void ** iface)
                     {return GetOuter()->QueryInterface(iid, iface);}

  protected:
   ~TOcPart();

    virtual bool InitObj (TOcInitInfo * initInfo);

    // TUnknown virtual overrides
    //
    HRESULT      QueryObject(const IID & iid, void * * iface);

    // IBSite implementation for BOle to use
    //
    HRESULT   _IFUNC SiteShow(BOOL);
    HRESULT   _IFUNC DiscardUndo();
    HRESULT   _IFUNC GetSiteRect(owl::TRect *, owl::TRect *);
    HRESULT   _IFUNC SetSiteRect(const owl::TRect *);
    HRESULT   _IFUNC SetSiteExtent(const owl::TSize *);
    HRESULT   _IFUNC GetZoom(TOcScaleInfo * scale);
    void      _IFUNC Invalidate(TOcInvalidate);
    void      _IFUNC OnSetFocus(BOOL set);
    HRESULT   _IFUNC Init(IBDataProvider *, IBPart *, LPCOLESTR, BOOL);
    void      _IFUNC Disconnect();

    // IBSite2 implementation for BOle to use
    //
    HRESULT   _IFUNC GetParentWindow(HWND *);

  protected:
    IUnknown*      BPart;      ///< Our corresponding helper object & interfaces
    IBPart*        BPartI;
    IBLinkable*    BLPartI;

    TOcDocument&  OcDocument; ///< The OC document we are imbeded in
    TOcStorage*    Storage;    ///< The storage we are in

    owl::tstring    Name;
    owl::TPoint        Pos;
    owl::TSize          Size;
    enum TFlag { Visible=0x01, Selected=0x02, Active=0x04, Link=0x08,
                 Closing=0x10, OcxDefault=0x20, OcxCancel=0x40 };
    owl::uint16       Flags;      ///< Is this part Selected/Visible/Active/ALink?

  friend class TOcPartCollection;
};

//
/// \class TOcPartCollection
// ~~~~~ ~~~~~~~~~~~~~~~~~
/// Container of parts with iterator
//
class _OCFCLASS TOcPartCollection : public owl::TPtrArray<TOcPart*> {
  private:
    typedef owl::TPtrArray<TOcPart*> Base;
  public:
    TOcPartCollection();
   ~TOcPartCollection();

    void   Clear();
    int   Add(TOcPart* const& part)
              { return Base::Add(part); }
    int IsEmpty() const {return Base::Empty();}
    int Find(TOcPart* const& part)
             { return Base::Find(part);}
    virtual unsigned Count() const {return Base::Size();}
    int Detach(TOcPart* const& part, int del = 0);

    TOcPart*   Locate(owl::TPoint& point);
    bool       SelectAll(bool select = false);
};

//
// class TOcPartCollectionIter
// ~~~~~ ~~~~~~~~~~~~~~~~~~~~~
typedef TOcPartCollection::Iterator TOcPartCollectionIter;

//
// class TOcVerb
// ~~~~~ ~~~~~~~
class _OCFCLASS TOcVerb {
  public:
    TOcVerb();

  public:
    LPCOLESTR   TypeName;
    LPCOLESTR   VerbName;
    owl::uint        VerbIndex;
    bool        CanDirty;
};

//
/// \class TOcPartChangeInfo
// ~~~~~ ~~~~~~~~~~~~~~~~~
/// View/Data change info
//
class _OCFCLASS TOcPartChangeInfo {
  public:
    TOcPartChangeInfo(TOcPart* part, TOcInvalidate type = invView)
      : Part(part), Type(type) {}

    bool     IsDataChange() {return Type & invData;}
    bool     IsViewChange() {return Type & invView;}

    void     SetDataChange() {Type |= invData;}
    void     SetViewChange() {Type |= invView;}

    TOcPart* GetPart() {return Part;}
    void     SetPart(TOcPart* part) {Part = part;}

    int      GetType() {return Type;}

  protected:
    int         Type;  // what type of change
    TOcPart*    Part;
};


} // OCF namespace

#endif  // OCF_OCPART_H
