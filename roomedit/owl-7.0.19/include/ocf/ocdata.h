//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// Definition of TOcDataProvider Class
//----------------------------------------------------------------------------

#if !defined(OCF_OCDATA_H)
#define OCF_OCDATA_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <ocf/ocview.h>

namespace ocf {
//
// Callback function for user data clean up
//
typedef void (*TDeleteUserData)(void* userData);

//
/// \class TOcDataProvider
// ~~~~~ ~~~~~~~~~~~~~~~
/// Data Provider object for a container document
//
class _OCFCLASS TOcDataProvider : public TUnknown,
                                  public IBDataProvider {  // !CQ should we use IBDataProvider2?
  public:
    TOcDataProvider(TOcView& ocView, owl::TRegList* regList, IUnknown* outer = 0,
                    void* userData = 0, TDeleteUserData callBack = 0);

    // Object reference & lifetime managment
    //
  public:
    owl::ulong   _IFUNC AddRef() {return GetOuter()->AddRef();}
    owl::ulong   _IFUNC Release() {return GetOuter()->Release();}
    HRESULT _IFUNC QueryInterface(const GUID & iid, void ** iface)
                     {return GetOuter()->QueryInterface(iid, iface);}

    // Supporting methods
    //
    void    Disconnect();
    void    Rename();
    void*   GetUserData() {return UserData;}
    void    SetUserData(void* userData) {UserData = userData;}

  protected:
    // IBDataNegotiator implementation
    //
    UINT     _IFUNC CountFormats();
    HRESULT  _IFUNC GetFormat(owl::uint index, TOcFormatInfo * fmt);

    // IBDataProvider implementation
      // !CQ IBDataProvider2 implementation ???
  //
    HANDLE  _IFUNC  GetFormatData(TOcFormatInfo * fmt);
//    HRESULT _IFUNC  SetFormatData(TOcFormatInfo * fmt, HANDLE data, bool release);
    HRESULT _IFUNC  Draw(HDC dc, const RECTL *  pos, const RECTL * clip,
                         TOcAspect aspect, TOcDraw bd);
    HRESULT _IFUNC  GetPartSize(owl::TSize * size);
    HRESULT _IFUNC  Save(IStorage*, BOOL sameAsLoad, BOOL remember);

    owl::TPoint         Origin;
    owl::TSize          Extent;

  private:
   ~TOcDataProvider();

    // TUnknown overrides
    //
    HRESULT      QueryObject(const IID & iid, void * * iface);

    TOcView&        OcView;
    IUnknown*       BSite;           ///< site helper
    IBSite*         BSiteI;          ///< Site interface
    IBLinkable*     BLSiteI;         ///< for item moniker
    void*           UserData;        ///< pointer to application specific data such as selection
    TDeleteUserData CBDelete;        ///< Call back function for cleaning up user data
};

} // OCF namespace

#endif  // OCF_OCDATA_H
