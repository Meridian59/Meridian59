//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of TSerializer class
//----------------------------------------------------------------------------

#if !defined(OWL_SERIALZE_H)
#define OWL_SERIALZE_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>
#include <owl/eventhan.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

//
/// \class TSerializer
// ~~~~~ ~~~~~~~~~~~
/// This class sends a block of data to another window.
//
class _OWLCLASS TSerializer {
  public:
    /// Enum for different types of blocks sent from a window to
    /// another. This enum is sent as the wParam.
    //
    enum TBlock {
      End = 0,    ///< end of data, lParam == 0
      Data1,      ///< data is stored in bits 0x000000FF of lParam
      Data2,      ///< data is stored in bits 0x0000FFFF of lParam
      Data3,      ///< data is stored in bits 0x00FFFFFF of lParam
      Data4,      ///< data is stored in bits 0xFFFFFFFF of lParam
      Begin,      ///< beginning of data, lParam length of data
    };
    TSerializer(HWND hwndTarget, uint32 length, void * data);
};

//
/// Serializer window targets should catch the following registered message
/// to receive the block of data.
//
#define SerializerMessage _T("SERIALIZERMESSAGE")

/// \addtogroup mixin
/// @{
/// \class TSerializeReceiver
// ~~~~~ ~~~~~~~~~~~~~~~~~~
/// Mix-in class that automatically puts together the block of data sent by
/// TSerializer.
//
class _OWLCLASS TSerializeReceiver : virtual public TEventHandler {
  public:
    TSerializeReceiver();

    // Derived classes should override this function to copy the received data.
    // passed to it.
    //
    virtual void DataReceived(uint32 length, void * data);

  protected:
    TResult BlockReceived(TParam1, TParam2);

  private:
    char * Data;
    char * CurPtr;
    uint32 Length;

  DECLARE_RESPONSE_TABLE(TSerializeReceiver);
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>


} // OWL namespace


#endif  // OWL_SERIALZE_H
