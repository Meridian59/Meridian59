//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// Definition of TOcLinkView Class
//----------------------------------------------------------------------------

#if !defined(OCF_OCLINK_H)
#define OCF_OCLINK_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/geometry.h>

#include <owl/contain.h>
#include <owl/template.h>

#include <ocf/ocbocole.h>
#include <ocf/ocreg.h>
#include <ocf/ocobject.h>




namespace ocf {

class _ICLASS TOcView;

//
/// \class TOcLinkView
// ~~~~~ ~~~~~~~~~~~
/// Link Viewer object for a server document
//
class _ICLASS TOcLinkView : public TUnknown,
//                            public IBPart2 {
                            public IBPart {
  public:
    TOcLinkView(TOcView* ocView, owl::TRegList* regList = 0, IUnknown* outer = 0);
    int Detach();

    // IBSite pass-thrus
    //
    void    Invalidate(TOcInvalidate);
    void    Disconnect();

    // Misc status accessors, etc.
    //
    void     GetLinkRect();
    void     SetMoniker(LPCTSTR name);
    owl::TString& GetMoniker() {return Moniker;};

    // Object reference & lifetime managment
    //
    owl::ulong   _IFUNC AddRef() {return GetOuter()->AddRef();}
    owl::ulong   _IFUNC Release() {return GetOuter()->Release();}
    HRESULT _IFUNC QueryInterface(const GUID & iid, void ** iface)
                     {return GetOuter()->QueryInterface(iid, iface);}

  protected:
    // TUnknown virtual overrides
    //
    HRESULT      QueryObject(const IID & iid, void * * iface);

    // IBDataNegotiator implementation
    //
    owl::uint     _IFUNC CountFormats();
    HRESULT  _IFUNC GetFormat(owl::uint index, TOcFormatInfo * fmt);

    // IBDataProvider2 implementation
    //
    HANDLE  _IFUNC  GetFormatData(TOcFormatInfo *);
    HRESULT _IFUNC  Draw(HDC, const RECTL *, const RECTL *, TOcAspect, TOcDraw bd);
    HRESULT _IFUNC  GetPartSize(owl::TSize *);
    HRESULT _IFUNC  Save(IStorage*, BOOL sameAsLoad, BOOL remember);
//    HRESULT _IFUNC  SetFormatData(TOcFormatInfo * fmt, HANDLE data, bool release);

    // IBPart2 implementation
    //
    HRESULT _IFUNC  Init(IBSite *, TOcInitInfo *);
    HRESULT _IFUNC  Close();
    HRESULT _IFUNC  CanOpenInPlace();
    HRESULT _IFUNC  SetPartSize(owl::TSize *);
    HRESULT _IFUNC  SetPartPos(owl::TRect *);
    HRESULT _IFUNC  Activate(BOOL);
    HRESULT _IFUNC  Show(BOOL);
    HRESULT _IFUNC  Open(BOOL);
    HRESULT _IFUNC  EnumVerbs(TOcVerb *);
    HRESULT _IFUNC  DoVerb(owl::uint);
    HWND    _IFUNC  OpenInPlace(HWND);
    HRESULT _IFUNC  InsertMenus(HMENU, TOcMenuWidths *);
    HRESULT _IFUNC  ShowTools(BOOL);
    void    _IFUNC  FrameResized(const owl::TRect *, BOOL);
    HRESULT _IFUNC  DragFeedback(owl::TPoint *, BOOL);
    HRESULT _IFUNC  GetPalette(LOGPALETTE * *);
    HRESULT _IFUNC  SetHost(IBContainer * objContainer);
    HRESULT _IFUNC  DoQueryInterface(const IID & iid, void * * pif);
    LPOLESTR _IFUNC GetName(TOcPartName);

    owl::TPoint    Origin;
    owl::TSize     Extent;

  private:
   ~TOcLinkView();

    TOcView*        OcView;          ///< Our hosting view that link is on
    IUnknown*       BSite;           ///< In-place site helper object
    IBSite*         BSiteI;          ///< Site interface
    IBApplication*  BAppI;           ///< Site's application interface

    owl::TString          Moniker;

  friend class TOcLinkCollection;
};

//
/// \class TOcLinkCollection
// ~~~~~ ~~~~~~~~~~~~~~~~~
/// Container of link views with iterator
//
class _OCFCLASS TOcLinkCollection : public owl::TPtrArray<TOcLinkView*> {
  private:
    typedef owl::TPtrArray<TOcLinkView*> Base;
    //friend class TOcLinkCollectionIter;
  public:
    TOcLinkCollection();
   ~TOcLinkCollection();
    //void operator delete(void* ptr) {TStandardAllocator::operator delete(ptr);}

    void Clear();
    int Add(TOcLinkView* const& View)
            { return Base::Add(View);}
    int IsEmpty() const {return Base::Empty();}
    int Find(TOcLinkView* const& view)
            { return Base::Find(view);}
    virtual unsigned Count() const {return Base::Size();}
    int Detach(TOcLinkView* const& view, int del = 0);
    TOcLinkView* Find(owl::TString const moniker) const;
};
typedef TOcLinkCollection TOcViewCollection;  // For compatibility only

//
/// Iterator for view collection
//
typedef TOcLinkCollection::Iterator TOcLinkCollectionIter;

//
/// Iterator for view collection.
/// For compatibility only.
//
typedef TOcLinkCollectionIter TOcViewCollectionIter;

} // OCF namespace

#endif  // OCF_OCLINK_H
