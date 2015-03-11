//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of TDecoratedMDIFrame class
//----------------------------------------------------------------------------

#if !defined(OWL_DECMDIFR_H)
#define OWL_DECMDIFR_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/mdi.h>
#include <owl/decframe.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup decor
/// @{
/// \class TDecoratedMDIFrame
// ~~~~~ ~~~~~~~~~~~~~~~~~~
/// Derived from both TMDIFrame and TDecoratedFrame, TDecoratedMDIFrame is an MDI
/// frame that supports decorated child windows.
/// TDecorated MDIFrame supports custom toolbars. You can insert one set of
/// decorations (for example, toolbars and rulers) into a decorated frame. When a
/// different set of tools is needed, you can remove the previous set and reinsert
/// another set of decorations. However, be sure to remove all of the unwanted
/// decorations from the adjusted sides (that is, the top, left, bottom, and right)
/// before reinserting a new set.
//
class _OWLCLASS TDecoratedMDIFrame : public TMDIFrame,
                                     public TDecoratedFrame {
  public:
    TDecoratedMDIFrame(LPCTSTR          title,
                       TResId          menuResId,
                       TMDIClient&     clientWnd = *new TMDIClient,
                       bool            trackMenuSelection = false,
                       TModule*        module = 0);

    TDecoratedMDIFrame(
      const tstring& title, 
      TResId menu, 
      TMDIClient& client = *new TMDIClient,
      bool trackMenuSelection = false,
      TModule* module = 0);

    virtual TResult DefWindowProc(TMsgId, TParam1, TParam2);

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TDecoratedMDIFrame(const TDecoratedMDIFrame&);
    TDecoratedMDIFrame& operator =(const TDecoratedMDIFrame&);

  DECLARE_RESPONSE_TABLE(TDecoratedMDIFrame);
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TDecoratedMDIFrame, 1);
  DECLARE_STREAMABLE_OWL(TDecoratedMDIFrame, 1);
};
/// @}

// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TDecoratedMDIFrame );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

typedef TDecoratedMDIFrame TDecoratedMdiFrame;


} // OWL namespace


#endif  // OWL_DECMDIFR_H

