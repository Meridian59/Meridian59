//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of classes TSerializer and TSerializeReceiver
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/serialze.h>
#include <owl/windowev.h>
#include <stdio.h>

namespace owl {

OWL_DIAGINFO;

//
/// Breaks down the data into blocks and sends each block to the window via
/// SendMessage.
///
/// wParam of the SendMessage is of type TBlock which signifies what
/// lParam contains.
//
TSerializer::TSerializer(HWND hwndTarget, uint32 length, void * data)
{
  if (!::IsWindow(hwndTarget) || length == 0)
    return;
  TMsgId msg = ::RegisterWindowMessage(SerializerMessage);

  ::SendMessage(hwndTarget, msg, Begin, length);

  // Send blocks 4 at a time
  //
  uint32* dataBlocks = (uint32*)data;
  while (length > 3) {
    ::SendMessage(hwndTarget, msg, Data4, *dataBlocks);
    dataBlocks += 1;
    length -= 4;
  }

  // Block was an even multiple of 4
  //
  if (length == 0) {
    ::SendMessage(hwndTarget, msg, End, 0);
    return;
  }

  // Length must be either 1, 2, or 3
  //
  uint8* dataBytes = (uint8*)dataBlocks;
  uint32 finalBlock = 0;

  if (length == 3)
    finalBlock += 0x100L * dataBytes[2];

  if (length >= 2)
    finalBlock += 0x10000L * dataBytes[1];

  finalBlock += 0x1000000L * dataBytes[0];

  ::SendMessage(hwndTarget, msg, int(length), finalBlock);
  ::SendMessage(hwndTarget, msg, End, 0);
}

//----------------------------------------------------------------------------


DEFINE_RESPONSE_TABLE1(TSerializeReceiver, TEventHandler)
  EV_REGISTERED(SerializerMessage, BlockReceived),
END_RESPONSE_TABLE;

//
// Constructor
//
TSerializeReceiver::TSerializeReceiver()
:
  TEventHandler(), Data(0), CurPtr(0), Length(0)
{
}

//
/// Automatically puts the data blocks back together.
//
TResult
TSerializeReceiver::BlockReceived(TParam1 param1, TParam2 param2)
{
  switch (param1) {
    case TSerializer::Begin: {
      Length = param2;
      Data = new char[Length];
      CurPtr = Data;
      return 0;
    }

    case TSerializer::End: {
      DataReceived(Length, Data);
      delete[] Data;
      Data = 0;
      return 0;
    }

    case TSerializer::Data4: {
      uint32* ptr = (uint32*)CurPtr;
      *ptr = param2;
      CurPtr += 4;
      return 0;
    }

    case TSerializer::Data1:
    case TSerializer::Data2:
    case TSerializer::Data3:
      // Fall through
      //
      break;

    default: // ignored, unknown block type
      return 0;
  }

  // Unpack the last remaining bytes
  //
  *CurPtr++ = (char)HiUint8(HiUint16(param2));

  if (param1 >= TSerializer::Data2)
    *CurPtr++ = (char)LoUint8(HiUint16(param2));

  if (param1 == TSerializer::Data3)
    *CurPtr++ = (char)HiUint8(LoUint16(param2));

  return 0;
}

//
/// This virtual function will be called whenever the data has been reconstructed.
/// \note Derived classes should override this function to copy the data because it will
/// be deleted when this function returns.
//
void
TSerializeReceiver::DataReceived(uint32 /*length*/, void * /*data*/)
{
}

} // OWL namespace
/* ========================================================================== */

