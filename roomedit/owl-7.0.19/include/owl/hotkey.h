//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class THotKey
//----------------------------------------------------------------------------

#if !defined(OWL_HOTKEY_H)
#define OWL_HOTKEY_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/commctrl.h>


namespace owl {

#include <owl/preclass.h>

/// \addtogroup commctrl
/// @{
/// \class THotKey
// ~~~~~ ~~~~~~~
/// THotKey encapsulates the hot-key control, a window that allows the user to
/// enter a combination of keystrokes to be used as a a hot key.
/// [A hot key is a key combination that the user can press to perform an
/// action quickly].
//
class _OWLCLASS THotKey : public TControl {
  public:
    THotKey(TWindow* parent, int id, int x, int y, int w, int h, TModule* module = 0);
    THotKey(TWindow* parent, int resourceId, TModule* module = 0);
    THotKey(THandle hWnd, TModule* module = 0);

    // Set and retrieve the hot key virtual code.
    //
    uint16  GetHotKey();
    void    SetHotKey(uint16 hotKey);
    void    SetHotKey(uint8 vk, uint8 mods);

    /// Set the invalid key combinations for this control
    //
    void    SetRules(uint16 invalid, uint16 defFlag);

    /// Safe overload
    //
    uint Transfer(uint16& key, TTransferDirection direction) {return Transfer(&key, direction);}

  protected:

    // Override TWindow virtual member functions
    //
    uint Transfer(void* buffer, TTransferDirection direction);
    virtual auto GetWindowClassName() -> TWindowClassName;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    THotKey(const THotKey&);
    THotKey& operator =(const THotKey&);

};

/// @}

#include <owl/posclass.h>


} // OWL namespace


#endif  // OWL_HOTKEY_H
