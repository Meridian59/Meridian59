// ****************************************************************************
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// gadgetex.h:   header file
// Version:      1.6
// Date:         08.11.1998
// Author:       Dieter Windau
//
// Freeware OWL classes that extents the dockable and gadget system
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// Tested with Borland C++ 5.02, OWL 5.02, OWL6 patch #3 and with Windows
// NT 4.0 SP3 but I think the class should work with Windows 95 too.
// This file is provided "as is" with no expressed or implied warranty.
// Use at your own risk.
//
// This package contains many classes and source that are based on other OWL
// developers work. Very special thanks to Alan Chambers, Christopher Kohlhoff,
// Jo Parrello, Mark Hatsell, Michael Mogensen and Yura Bidus
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@usa.net
//   Web:   http://members.aol.com/softengage/index.htm
// ****************************************************************************

#ifndef __OWLEXT_GADGETEX_H
#define __OWLEXT_GADGETEX_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H)
  #include <owlext/core.h>
#endif

#ifndef   OWL_BUTTONGA_H
#include <owl/buttonga.h>
#endif
#ifndef   OWL_CONTROLG_H
#include <owl/controlg.h>
#endif

#ifndef __OWLEXT_GADGDESC_H
#include <owlext/gadgdesc.h>
#endif

#include <owlext/dockingex.rh>

namespace OwlExt {
// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

// ****************** Forward declaration *************************************
class OWLEXTCLASS TInvisibleGadgetEx;
class OWLEXTCLASS TButtonGadgetEx;
class OWLEXTCLASS TTextButtonGadgetEx;
class OWLEXTCLASS TPopupButtonGadgetEx;
class OWLEXTCLASS TMenuButtonGadgetEx;
class OWLEXTCLASS TRecentFileButtonGadgetEx;
class OWLEXTCLASS TColorButtonGadgetEx;
class OWLEXTCLASS TControlGadgetEx;

// ******************** TInvisibleGadgetEx ************************************

class OWLEXTCLASS TInvisibleGadgetEx : public owl::TGadget {
  public:
    TInvisibleGadgetEx();
    virtual ~TInvisibleGadgetEx();

  protected:
    virtual void Inserted()
      { BorderStyle = None; }
};

// ******************** TButtonGadgetEx ***************************************

class OWLEXTCLASS TButtonGadgetEx : public owl::TButtonGadget
{
  public:
    TButtonGadgetEx(owl::TResId glyphResIdOrIndex,
                    int    id,
                    TType  type = Command,
                    bool   enabled = false, // initial state before cmd enabling
                    TState state = Up,
                    bool   sharedGlyph = false);
    virtual ~TButtonGadgetEx();


    // Some accessors
    //
    void         SetButtonState(TState newState);
    owl::TResId       GetResId() const;

    // Set resId of shown bitmap if Sharing is false or
    // set glyphIndex of shown bitmap if Sharing is true
    //
    virtual void SetResId(owl::TResId glyphResIdOrIndex);

    // Reset the members to default values.
    //
    virtual void Reset();

    // Returns true, if the members have the default values
    //
    virtual bool HasDefaultValues();

    // Override and initiate a WM_COMMAND_ENABLE message
    //
    virtual void CommandEnable();

  protected:
    void         Invalidate();

    virtual void CheckStyleAndState();
    virtual void Paint(owl::TDC& dc);
    virtual void PaintBorder(owl::TDC& dc);

    virtual void MouseEnter(owl::uint modKeys, const owl::TPoint& point);


    virtual void Activate(const owl::TPoint& pt);
    virtual void SyncronizeButtons(int id, TState newState);
    virtual void BetterCheckExclusively();

};

// ******************** TTextButtonGadgetEx ***********************************

class OWLEXTCLASS TButtonTextGadgetEx : public TButtonGadgetEx
{
  public:
    TButtonTextGadgetEx(
      LPCTSTR      commandText,          // command text, if 0 try to load them
      TDisplayType disptype,             // displax type
      owl::TResId       glyphResIdOrIndex,    // see TButtonGadgetEx
      int          id,                   // see TButtonGadgetEx
      TType        type = Command,       // see TButtonGadgetEx
      bool         enabled = false,      // see TButtonGadgetEx
      TState       state = Up,           // see TButtonGadgetEx
      bool         sharedGlyph = false); // see TButtonGadgetEx
    // constructor to set additional command text and display type

    virtual ~TButtonTextGadgetEx();

    TDisplayType GetDisplayType() { return DispType; }
    virtual void SetDisplayType(TDisplayType dispType);

    owl::tstring       GetCommandText() { return CommandText; }
    virtual void SetCommandText(const owl::tstring& commandText);

    virtual void GetDesiredSize(owl::TSize& size);
    virtual void SetBounds(const owl::TRect& rect);

    // Reset the members to default values.
    //
    virtual void Reset();

    // Returns true, if the members have the default values
    //
    virtual bool HasDefaultValues();

  protected:
    virtual void Created();
    virtual void Paint(owl::TDC& dc);
    virtual void PaintText(owl::TDC& dc);

    virtual void TryToLoadCommandText();

    virtual owl::TDib* GetGlyphDib();

  protected:
    owl::tstring       CommandText;
    TDisplayType DispType;
};

// ******************** TPopupButtonGadgetEx **********************************

// This should be a abstract class.
// There is no descriptot class for this gadget

class OWLEXTCLASS TPopupButtonGadgetEx : public TButtonTextGadgetEx
{
  public:
    TPopupButtonGadgetEx(
      TPopupType   popupType,
      LPCTSTR      commandText,          // see TButtonTextGadget
      TDisplayType disptype,             // see TButtonTextGadget
      owl::TResId       glyphResIdOrIndex,    // see TButtonGadgetEx
      int          id,                   // see TButtonGadgetEx
      TType        type = Command,       // see TButtonGadgetEx
      bool         enabled = false,      // see TButtonGadgetEx
      TState       state = Up,           // see TButtonGadgetEx
      bool         sharedGlyph = false); // see TButtonGadgetEx

  public:
    virtual void GetDesiredSize(owl::TSize& size);

    // Start the PopupAction if the user click
    // 1) into the button if PopupType != DownArrowAction
    // 2) into the arrow  if PopupType == DownArrowAction
    // This function do nothing, overwrite them with functionallity
    //
    virtual void PopupActionStart();

    // This function must be called to show the button
    // include the down arrow in normal state
    //
    virtual void PopupActionEnd();

  protected:
    // Invoked by mouse-down & mouse enter events. sets member data "Pressed"
    // to true and highlights the button
    //
    virtual void  BeginPressed(const owl::TPoint& p);

    // Invoked by mouse exit events. sets member data "Pressed" to false and
    // paints the button in its current state
    //
    virtual void  CancelPressed(const owl::TPoint& p, bool mstate=false);

    // Captures the mouse if "TrackMouse" is set.
    //
    virtual void  LButtonDown(owl::uint modKeys, const owl::TPoint& point);

    // Releases the mouse capture if "TrackMouse" is set.
    //
    virtual void  LButtonUp(owl::uint modKeys, const owl::TPoint& point);

    // 1. PaintButton
    // 2. PaintSeparator
    // 3. PaintDownArrow
    //
    virtual void Paint(owl::TDC& dc);

    // Paint the left button area
    //
    virtual void PaintButton(owl::TDC& dc);

    // Paint the separator between button and down arrow
    //
    virtual void PaintSeparator(owl::TDC& dc);

    // Paint the down arrow
    //
    virtual void PaintDownArrow(owl::TDC& dc);

  protected:
    TPopupType PopupType;
    bool       DownArrowPressed;
    bool       ButtonHasCapture;
};

// ******************** TMenuButtonGadgetEx ***********************************

class OWLEXTCLASS TMenuButtonGadgetEx : public TPopupButtonGadgetEx
{
  public:
    TMenuButtonGadgetEx(
      TPopupType   popupType,
      HMENU        hmenu,                // adress that points to a menu
      owl::TWindow*     cmdTarget,            // commando target window
      LPCTSTR      commandText,          // see TButtonTextGadget
      TDisplayType disptype,             // see TButtonTextGadget
      owl::TResId       glyphResIdOrIndex,    // see TButtonGadgetEx
      int          id,                   // see TButtonGadgetEx
      TType        type = Command,       // see TButtonGadgetEx
      bool         enabled = false,      // see TButtonGadgetEx
      TState       state = Up,           // see TButtonGadgetEx
      bool         sharedGlyph = false); // see TButtonGadgetEx

    // Set the flags. Default: TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON
    //
    static void SetTrackPopupMenuFlags(owl::uint flags) { Flags = flags; }
    static owl::uint GetTrackPopupMenuFlags() { return Flags; }

    virtual void PopupActionStart();

  protected:
    static owl::uint Flags;

    HMENU    hMenu;
    owl::TWindow* CmdTarget;
};

// ******************** TRecentFileButtonGadgetEx *****************************

class OWLEXTCLASS TRecentFilesGadgetEx : public TMenuButtonGadgetEx
{
  public:
    TRecentFilesGadgetEx(
      LPCTSTR      commandText,          // see TButtonTextGadget
      TDisplayType disptype,             // see TButtonTextGadget
      owl::TResId       glyphResIdOrIndex,    // see TButtonGadgetEx
      int          id,                   // see TButtonGadgetEx
      TType        type = Command,       // see TButtonGadgetEx
      bool         enabled = false,      // see TButtonGadgetEx
      TState       state = Up,           // see TButtonGadgetEx
      bool         sharedGlyph = false); // see TButtonGadgetEx

    virtual void PopupActionStart();
};

// ******************** TColorButtonGadgetEx **********************************

class TColorPicker;

class OWLEXTCLASS TColorButtonGadgetEx : public TPopupButtonGadgetEx
{
  public:
    static owl::TRect   DefaultFillRect;

  public:
    TColorButtonGadgetEx(
      TPopupType   popupType,            // see TPopupButtonGadget
      const TColorPickerData& data,      // see TColorPicker
      owl::TColor       startColorSel,        // start color that is selected
      owl::TRect*       fillRect,             // rect that will be filled with color
      owl::TWindow*     parentWindow,         // parent window of picker
      LPCTSTR      commandText,          // see TButtonTextGadget
      TDisplayType disptype,             // see TButtonTextGadget
      owl::TResId       glyphResIdOrIndex,    // see TButtonGadgetEx
      int          id,                   // see TButtonGadgetEx
      TType        type = Command,       // see TButtonGadgetEx
      bool         enabled = false,      // see TButtonGadgetEx
      TState       state = Up,           // see TButtonGadgetEx
      bool         sharedGlyph = false); // see TButtonGadgetEx

    virtual void  PopupActionStart();

    virtual bool  IdleAction(long idleCount);

  protected:
    virtual void Created();

    virtual void PaintButton(owl::TDC& dc);

    virtual void Activate(const owl::TPoint& pt);

  protected:
    TColorPickerData Data;
    owl::TColor        ColorSel;         // currect selected color
    owl::TRect*        FillRect;         // rect that will be filled with color
                                    // in gadget coordinates
    owl::TWindow*      ParentWindow;     // parent window of picker
    TColorPicker* ColorPickerWindow;
    bool          CheckingMode;
};

// ******************** TControlGadgetEx **************************************

class OWLEXTCLASS TControlGadgetEx : public owl::TControlGadget {// public TGadget {
  public:
    TControlGadgetEx(owl::TWindow& control, TBorderStyle style = None);

   ~TControlGadgetEx();
   // don't delete the control
   // This will be done in TControlGadgetDesc

  protected:
    void           Created();
    void           Inserted();
    void           Removed();

    void           Paint(owl::TDC& dc);

    virtual bool   IdleAction(long idleCount);

  protected_data:
    bool           IsCursorInControl;

  private:
    // Hidden to prevent accidental copying or assignment
    //
//    TControlGadgetEx(const TControlGadgetEx&);
//    TControlGadgetEx& operator =(const TControlGadgetEx&);
};

// ******************** Inline Implementations ********************************


inline owl::TResId TButtonGadgetEx::GetResId() const {
  return ResId;
}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>
} // OwlExt namespace

#endif
