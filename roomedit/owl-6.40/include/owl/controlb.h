//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TControlBar.
//----------------------------------------------------------------------------

#if !defined(OWL_CONTROLB_H)
#define OWL_CONTROLB_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/gadgetwi.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup gadgets
/// @{
/// \class TControlBar
// ~~~~~ ~~~~~~~~~~~
/// Derived from TGadgetWindow, TControlBar implements a control bar that provides
/// mnemonic access for its button gadgets. The sample MDIFILE.CPP ObjectWindows
/// program from the Borland C++ 5.x distribution displays the following example of a control
/// bar:
/// \image html bm103.BMP
/// 
/// To construct, build, and insert a control bar into a frame window, you can first
/// define the following response table:
/// \code
/// DEFINE_RESPONSE_TABLE1(TMDIFileApp, TApplication)
/// 	EV_COMMAND(CM_FILENEW, CmFileNew),
/// 	EV_COMMAND(CM_FILEOPEN, CmFileOpen),
/// 	EV_COMMAND(CM_SAVESTATE, CmSaveState),
/// 	EV_COMMAND(CM_RESTORESTATE, CmRestoreState),
/// END_RESPONSE_TABLE;
/// \endcode
/// Next, add statements that will construct a main window and load its menu,
/// accelerator table, and icon. Then, to construct, build and insert a control bar
/// into the frame window, insert these statements:
/// \code
/// TControlBar* cb = new TControlBar(frame);
/// cb->Insert(*new TButtonGadget(CM_FILENEW, CM_FILENEW));
/// cb->Insert(*new TButtonGadget(CM_FILEOPEN, CM_FILEOPEN));
/// cb->Insert(*new TButtonGadget(CM_FILESAVE, CM_FILESAVE));
/// cb->Insert(*new TSeparatorGadget(6));
/// cb->Insert(*new TButtonGadget(CM_EDITCUT, CM_EDITCUT));
/// cb->Insert(*new TButtonGadget(CM_EDITCOPY, CM_EDITCOPY));
/// cb->Insert(*new TButtonGadget(CM_EDITPASTE, CM_EDITPASTE));
/// cb->Insert(*new TSeparatorGadget(6));
/// cb->Insert(*new TButtonGadget(CM_EDITUNDO, CM_EDITUNDO));
/// frame->Insert(*cb, TDecoratedFrame::Top);
/// \endcode
//
class _OWLCLASS TControlBar : public TGadgetWindow {
  public:
    TControlBar(TWindow*        parent = 0,
                TTileDirection  direction= Horizontal,
                TFont*          font = new TGadgetWindowFont,
                TModule*        module = 0);
   ~TControlBar();

  DECLARE_CASTABLE;
};
/// @}

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>


} // OWL namespace


#endif  // OWL_CONTROLB_H
