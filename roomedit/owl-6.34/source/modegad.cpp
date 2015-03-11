//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Defines TModeGadget.
///
/// A mode gadget is a gadget that displays the mode of a key.
/// The key code is defined as VK_XXXX in Windows.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/modegad.h>

namespace owl {

OWL_DIAGINFO;

//
/// Initializes the text gadget with the text key's text.
//
TModeGadget::TModeGadget(int vkKey, LPCTSTR text, int id,
  TBorderStyle border, TAlign align, TFont* font)
:
  TTextGadget(id, border, align, 10, text, font), VkKey(vkKey)
{
  SetShrinkWrap(true, true);
}

//
/// String-aware overload
//
TModeGadget::TModeGadget(
  int vkKey, 
  const tstring& text, 
  int id,
  TBorderStyle border, 
  TAlign align, 
  TFont* font
  )
  : TTextGadget(id, border, align, 10, text, font), 
  VkKey(vkKey)
{
  SetShrinkWrap(true, true);
}

//
/// Override from TGadget to update the state of the key.
//
bool
TModeGadget::IdleAction(long count)
{
  TTextGadget::IdleAction(count);
  int state = ::GetKeyState(VkKey);

  // Toggle keys
  //
  if (VkKey == VK_SCROLL || VkKey == VK_INSERT ||
      VkKey == VK_NUMLOCK || VkKey == VK_CAPITAL) {
    if (state & 1)
      SetEnabled(true);
    else
      SetEnabled(false);
  }
  else {
    // This is a regular key, highlite only when pressed
    //
    if (state & (1 << (sizeof(int) * 8 - 1)))
      SetEnabled(true);
    else
      SetEnabled(false);
  }
  return false;
}


} // OWL namespace
/* ========================================================================== */

