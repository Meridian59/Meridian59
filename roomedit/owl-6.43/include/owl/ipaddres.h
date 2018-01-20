//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
// Definition of class TIPAddress.
//----------------------------------------------------------------------------

#if !defined(OWL_IPADDRES_H)
#define OWL_IPADDRES_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/control.h>
#include <owl/commctrl.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup newctrl
/// @{
/// \class TIPAddressBits
// ~~~~~ ~~~~~~~~~~~~~~
//
/// Also used as transfer buffer
//
class _OWLCLASS TIPAddressBits {
  public:
    TIPAddressBits(uint32 address = 0):Address(address){}
    TIPAddressBits(int fld0, int fld1, int fld2, int fld3)
             : Address(MakeIPAddress(fld0,fld1,fld2,fld3)){}

    static uint32 MakeIPAddress(int f0, int f1, int f2, int f3)
     {  return ((uint32)(((uint32)(f0)<<24)+((uint32)(f1)<<16)+((uint32)(f2)<<8)+((uint32)(f3)))); }

    void SetAddress(int f0, int f1, int f2, int f3)    { Address = MakeIPAddress(f0,f1,f2,f3);  }
    void SetAddress(uint32 address) {Address = address;}

    int GetFirst()    { return ((Address>>24)& 0xff);  }
    int GetSecond()    { return ((Address>>16) & 0xff); }
    int GetThird()    { return ((Address>>8) & 0xff);  }
    int GetFourth()   { return (Address & 0xff);       }

    operator uint32() { return Address;}

    uint32  Address;
};


//
/// \class TIPAddress
// ~~~~~ ~~~~~~~~~~
//
class _OWLCLASS TIPAddress : public TControl {
  public:
    enum TIPFields{
      ifFirst,
      ifSecond,
      ifThird,
      ifForth,
    };

    TIPAddress(TWindow* parent, int id, int x, int y, int w, int h, TModule* module = 0);
    TIPAddress(TWindow* parent, int resourceId, TModule* module = 0);
    TIPAddress(THandle hWnd, TModule* module = 0);
    ~TIPAddress();

    void   Clear();

    void   SetAddress(const TIPAddressBits& address);
    int   GetAddress(TIPAddressBits& address) const;
    TIPAddressBits GetAddress() const {TIPAddressBits address; GetAddress(address); return address;}

    bool  IsBlank() const;
    void  SetFocus(const TIPFields& fld);

    bool  SetRange(const TIPFields& fld, int minv, int maxv);

    /// Safe overload
    //
    uint Transfer(TIPAddressBits& address, TTransferDirection direction) {return Transfer(&address, direction);}

  protected:

    // Override TWindow virtual member functions
    //
    uint Transfer(void* buffer, TTransferDirection direction);
    virtual TGetClassNameReturnType GetClassName();

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TIPAddress(const TIPAddress&);
    TIPAddress& operator =(const TIPAddress&);

///YB Need to be consistent - Common Controls are not streamable for the
///YB the most part. This guy only invokes TControl's version anyway.
///YB
///YB DECLARE_STREAMABLE_FROM_BASE(_OWLCLASS, owl::TIPAddress, 1);
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//
// inlines
//

//
inline void TIPAddress::Clear(){
  PRECONDITION(GetHandle());
  SendMessage(IPM_CLEARADDRESS, 0, 0);
}

//
inline void TIPAddress::SetAddress(const TIPAddressBits& address){
  PRECONDITION(GetHandle());
  SendMessage(IPM_SETADDRESS, 0, TParam2((uint32)(TIPAddressBits&)address));
}

//
inline int TIPAddress::GetAddress(TIPAddressBits& address) const{
  PRECONDITION(GetHandle());
  return (int)((TIPAddress*)this)->SendMessage(IPM_GETADDRESS, 0, TParam2(&address.Address));
}

//
inline bool TIPAddress::IsBlank() const{
  PRECONDITION(GetHandle());
  return ToBool(((TIPAddress*)this)->SendMessage(IPM_ISBLANK, 0, 0));
}

//
inline void TIPAddress::SetFocus(const TIPFields& fld){
  PRECONDITION(GetHandle());
  SendMessage(IPM_SETFOCUS, TParam1(fld), 0);
}

//
inline bool TIPAddress::SetRange(const TIPFields& fld, int minv, int maxv){
  PRECONDITION(GetHandle());
  return ToBool(SendMessage(IPM_SETRANGE, TParam1(fld), TParam2(MAKEIPRANGE(minv,maxv))));
}


} // OWL namespace

#endif  // OWL_MONTHCAL_H
