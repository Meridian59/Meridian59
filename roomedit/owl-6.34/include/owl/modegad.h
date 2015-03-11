//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of mode-tracking text gadget class TModeGadget.
//----------------------------------------------------------------------------

#if !defined(OWL_MODEGAD_H)
#define OWL_MODEGAD_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/textgadg.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup gadgets
/// @{
/// \class TModeGadget
// ~~~~~ ~~~~~~~~~~~
/// TModeGadget is a mode-tracking text gadget class.
class _OWLCLASS TModeGadget : public TTextGadget {
  public:
    TModeGadget(int vkKey, LPCTSTR text, int id = 0,
                TBorderStyle = Recessed, TAlign = Center,
                TFont* font = 0 /*new TGadgetWindowFont*/);

    TModeGadget(
      int vkKey, 
      const tstring& text, 
      int id = 0,
      TBorderStyle = Recessed, 
      TAlign = Center,
      TFont* font = 0);

    // Override from TGadget
    //
    bool IdleAction(long count);

  private:
    int VkKey;
};
/// @}

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>


} // OWL namespace


#endif // OWL_MODEGAD_H
