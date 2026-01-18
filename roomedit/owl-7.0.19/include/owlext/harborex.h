// ****************************************************************************
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// harborex.h:   header file
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

#ifndef __OWLEXT_HARBOREX_H
#define __OWLEXT_HARBOREX_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H)
  #include <owlext/core.h>
#endif

#include <owl/docking.h>
#include <owl/template.h>

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

// ****************** Forward declaration *************************************
class OWLEXTCLASS THarborEx;
class OWLEXTCLASS THarborManagement;
class OWLEXTCLASS TDockableControlBarEx;
class OWLEXTCLASS TButtonTextGadgetEx;

// ***************** Section strings for use in registry **********************

extern const TCHAR* MainWindowSection;
extern const TCHAR* LeftSection;
extern const TCHAR* TopSection;
extern const TCHAR* RightSection;
extern const TCHAR* BottomSection;
extern const TCHAR* VersionSection;
extern const TCHAR* CmdShowSection;
extern const TCHAR* ToolbarsSection;
extern const TCHAR* TooltipsSection;
extern const TCHAR* AccKeysSection;
extern const TCHAR* DrawGripperSection;
extern const TCHAR* FlatStyleSection;
extern const TCHAR* HintModeSection;
extern const TCHAR* IDSection;
extern const TCHAR* ToolbarSection;
extern const TCHAR* NameSection;
extern const TCHAR* LocationSection;
extern const TCHAR* VisibleSection;
extern const TCHAR* DefaultSection;
extern const TCHAR* GadgetCountSection;
extern const TCHAR* CountSection;
extern const TCHAR* ButtonSection;
extern const TCHAR* BmpIdSection;
extern const TCHAR* DisplayTypeSection;
extern const TCHAR* TextSection;

// ******************** THarborManagement *************************************

class OWLEXTCLASS THarborManagement : virtual public owl::TEventHandler
{
  public:
    THarborManagement(LPCTSTR registryName);
   ~THarborManagement();

    void InsertToolbarMenuItem(owl::TCommandEnabler& ce, int offset);
    // Insert the "Toolbar >" menu item at the menu item specified by ce + offset

    owl::tstring GetRegistryName() { return RegistryName; }
    static int GetInternVersion() { return InternVersion; }
    THarborEx* GetHarbor() { return Harbor; }

    void LoadSettings();
    void SaveSettings();

  protected:

    // simple wrapper functions to harbor
    //
    void CmShowHideCtlBar(owl::uint id);
    void CmCustomize();

    void CmGadgetReset();
    void CmGadgetDelete();
    void CmGadgetCopyImage();
    void CmGadgetPasteImage();
    void CmGadgetAppearance();
    void CmGadgetImage();
    void CmGadgetText();
    void CmGadgetImageText();
    void CmGadgetBeginGroup();

    void CeGadgetReset(owl::TCommandEnabler& ce);
    void CeGadgetDelete(owl::TCommandEnabler& ce);
    void CeGadgetCopyImage(owl::TCommandEnabler& ce);
    void CeGadgetPasteImage(owl::TCommandEnabler& ce);
    void CeGadgetAppearance(owl::TCommandEnabler& ce);
    void CeGadgetImage(owl::TCommandEnabler& ce);
    void CeGadgetText(owl::TCommandEnabler& ce);
    void CeGadgetImageText(owl::TCommandEnabler& ce);
    void CeGadgetBeginGroup(owl::TCommandEnabler& ce);

  protected:
    static int InternVersion;

    THarborEx* Harbor;
    HMENU      PopupMenu;
    owl::tstring RegistryName;

  DECLARE_RESPONSE_TABLE(THarborManagement);
};

// ************************ TDCBData ******************************************

const int MaxTitleLen = 255;

// class that encapsulate relevant TDockableToolBarEx data
class OWLEXTCLASS TDCBData
{
public:
  TDockableControlBarEx* DCB;
  owl::TDockingSlip*          Slip; // if DCB is not visible this is 0
  owl::TRect                  Rect; // in screen coordinates
  TCHAR                  Title[MaxTitleLen];

public:
  TDCBData()
    { DCB=0; Slip=0; Rect=owl::TRect(0,0,0,0); *Title = 0; }
  TDCBData(const TDCBData& data)
    { *this = data;  }
  ~TDCBData() { }

  TDCBData& operator =(const TDCBData& d);

  bool operator ==(const TDCBData& d) const
    { return memcmp(this, &d, sizeof(d)) == 0 ? true : false; }
  bool operator !=(const TDCBData& d) const
    { return !(*this == d); }
};

// ****************** THarborEx ***********************************************

class OWLEXTCLASS TToolbarCustomizeSheet;

class OWLEXTCLASS THarborEx : public owl::THarbor
{
  friend class THarborManagement;
  friend class TButtonGadgetEx;
  friend class TToolbarCustomizeSheet;
  friend class TToolbarPage;

  public:
    enum TGadgetType {
      IsNoGadget,
      IsToolbarGadget,
      IsRefGadget
    };

  public:
    THarborEx(owl::TDecoratedFrame& df,
              bool leftSlip   = true,
              bool topSlip    = true,
              bool rightSlip  = true,
              bool bottomSlip = false);
    // If you create one of the four TEdgeSlips in THarbor before you insert
    // decorations in TDecoratedFrame all the TDockable's in THarbor insert near
    // the main window borders. Some examples:
    // - if you don't create top and left slip, e.g. a inserted ruler
    //   is not near the client window of the decorated frame
    // - if you create bottom slip here, e.g. a inserted statusbar
    //   is not always at bottom of the decorated frame
    virtual ~THarborEx();

    // Called by dockable control bars
    //
    bool GadgetDraggingBegin();

    bool IsShowTooltips() { return ShowTooltips; }
    void SetShowTooltips(bool showTooltips) { ShowTooltips = showTooltips; }
    bool IsWithAccel() { return WithAccel; }
    void SetWithAccel(bool b) { WithAccel = b; }
    owl::TGadgetWindow::THintMode GetHintMode() { return HintMode; }
    void SetHintMode(owl::TGadgetWindow::THintMode HintMode);
    bool IsDrawGripper() { return DrawGripper; }
    void SetDrawGripper(bool drawGripper);
    // Get/set options for all dockable toolbars in harbor

    virtual bool PreProcessAppMsg(MSG& msg);
    // Translate some messages for DockingEx system
    // To be called from TApplication ProcessAppMsg(...)
    // return true if the message is processed

    virtual int PostGetHintText(owl::uint id, LPTSTR buf, int size, owl::THintText hintType);
    // Retrieves the hint text associated with a particular Id
    // To be called from TDecoratedFrame GetHintText(...)
    // If ShowTooltips == false, the tooltips will be deleted
    // If WithAccel == false, the accelerator keys defined by resource in style:
    // " (key)" will be hidden.

    bool IsDCBTitleValid(LPCTSTR title);
    // return true, if the title don't exist. Title should be a unique identifier

    int  GetFreeDCBId();
    // returns a unused id for a new dockable controlbar. -1 if an error occurs

    bool IsCustomizeMode() { return CustomizeMode; }
    // returns true if the system is in customize mode

    void SetCustGadget(owl::TGadgetWindow* gw, owl::TGadget* gadget);
    void SetCustGadget(int refId);
    void SetCustGadgetNull();

    TGadgetType    GetCustGadgetType() { return CustGadgetType; }
    owl::TGadgetWindow* GetCustWindow()     { return CustWindow; }
    owl::TGadget*       GetCustGadget()     { return CustGadget; }
    int            GetCustGadgetRef()  { return CustGadgetRef; }

    void CheckBeforeInsertDefaultGadget(int id);
    void InsertDefaultGadgets();
    void InsertDefaultGadget(int idx);

    HMENU GetToolbarMenu(bool getDCBData = true);
    HMENU GetGadgetMenu();
    void  UpdateShowHideCtlBar();

  protected:

    auto ConstructEdgeSlip(owl::TDecoratedFrame&, owl::TAbsLocation) -> owl::TEdgeSlip* override;
    // Factory function overridden to use TEdgeSlipEx in lace of TEdgeSlip

    auto ConstructFloatingSlip(owl::TDecoratedFrame&, int x, int y, owl::TWindow*) -> owl::TFloatingSlip* override;
    // Factory function overridden to use TFloatingSlipEx in lace of TFloatingSlip

    virtual TToolbarCustomizeSheet* CreateCustomizeDlg(owl::TDecoratedFrame& frame);
    virtual void ExecuteGadgetDialog(owl::TDecoratedFrame& frame, TButtonTextGadgetEx* btg);

    // Mouse handlers - invoked when harbor grabs capture
    //
    void     EvMouseMove(owl::uint modKeys, const owl::TPoint& point);
    void     EvLButtonUp(owl::uint modKeys, const owl::TPoint& point);
    void     EvLButtonDblClk(owl::uint modKeys, const owl::TPoint& point);

    void  SetCustomizeMode(bool b);
    void  GetDCBData();
    TDockableControlBarEx* GetHitDCB(const owl::TPoint& screenPoint);
    void  ShowDragDropLine(bool show);
    void  EraseCustomizeFrame();
    void  InsertDCBData(TDockableControlBarEx* cb, owl::TDockingSlip* slip);

    void  CmShowHideCtlBar(owl::uint id);
    void  CmCustomize();

    void  CmGadgetReset();
    void  CmGadgetDelete();
    void  CmGadgetCopyImage();
    void  CmGadgetPasteImage();
    void  CmGadgetAppearance();
    void  CmGadgetImage();
    void  CmGadgetText();
    void  CmGadgetImageText();
    void  CmGadgetBeginGroup();

    void  CeGadgetReset(owl::TCommandEnabler& ce);
    void  CeGadgetDelete(owl::TCommandEnabler& ce);
    void  CeGadgetCopyImage(owl::TCommandEnabler& ce);
    void  CeGadgetPasteImage(owl::TCommandEnabler& ce);
    void  CeGadgetAppearance(owl::TCommandEnabler& ce);
    void  CeGadgetImage(owl::TCommandEnabler& ce);
    void  CeGadgetText(owl::TCommandEnabler& ce);
    void  CeGadgetImageText(owl::TCommandEnabler& ce);
    void  CeGadgetBeginGroup(owl::TCommandEnabler& ce);

    void  LoadSettings(THarborManagement& harborMan);
    void  SaveSettings(THarborManagement& harborMan);

  protected:

    typedef enum tagGadgetState {
      IsNone,
      IsCut,
      IsCopy,
      IsDelete
    } TGadgetState;

    owl::TDecoratedFrame& DecoratedFrame;

    bool             ShowTooltips;  // Show tooltips for all dockable controlbars
    bool             WithAccel;     // Show accelerator key in tooltips
    bool             DrawGripper;   // Draw gripper for all edge slips
    owl::TGadgetWindow::THintMode
                     HintMode;      // Hint mode for all dockable controlbars
    bool             CustomizeMode; // True, if the customize dialog is open
//  bool             LargeButtons;  // Future: draw button gadgets in large style
//  bool             GrayButtons;   // Future: draw button gadgets in gray colors

    bool             GadgetDragging;// True, if we are in gadget dragging mode
    HCURSOR          CutPasteCursor;
    HCURSOR          CopyCursor;
    HCURSOR          DeleteCursor;
    HCURSOR          OldCursor;     // Cursor, before gadget dragging started

    owl::TMemoryDC*       MemDC;
    owl::TBitmap*         MemBmp;
    owl::TScreenDC*       DragScreenDC;
    owl::TRect            DragDropRect;
    owl::TPoint           DragDropP1;    // P1 of line that is drawn for drag & drop
    owl::TPoint           DragDropP2;    // P2 of line that is drawn for drag & drop
    bool             DragDropLineVisible;
    owl::TGadget*         DragDropGadget;
    owl::TGadgetWindow*   DragDropWindow;
    owl::TGadgetWindow::TPlacement
                     DragDropPlacement;
    TGadgetState     DragDropGadgetState;

    TGadgetType      CustGadgetType;  // State of the actual customize gadget
    owl::TGadget*         CustGadget;      // Actual customize gadget
    owl::TGadgetWindow*   CustWindow;      // GadgetWindow of CustGadget
    int              CustGadgetRef;   // Id of reference CustGadget

    owl::TResId           CopiedBmpResId;

    TToolbarCustomizeSheet* CustomizeDlg;

    owl::TIPtrArray<TDCBData*> DCBData;

  DECLARE_RESPONSE_TABLE(THarborEx);
};

// ******************** Inline Implementations ********************************

inline void THarborManagement::CmShowHideCtlBar(owl::uint id)
  { PRECONDITION(Harbor); Harbor->CmShowHideCtlBar(id); }
inline void THarborManagement::CmCustomize()
  { PRECONDITION(Harbor); Harbor->CmCustomize(); }

inline void THarborManagement::CmGadgetReset()
   { PRECONDITION(Harbor); Harbor->CmGadgetReset(); }
inline void THarborManagement::CmGadgetDelete()
   { PRECONDITION(Harbor); Harbor->CmGadgetDelete(); }
inline void THarborManagement::CmGadgetCopyImage()
  { PRECONDITION(Harbor); Harbor->CmGadgetCopyImage(); }
inline void THarborManagement::CmGadgetPasteImage()
  { PRECONDITION(Harbor); Harbor->CmGadgetPasteImage(); }
inline void THarborManagement::CmGadgetAppearance()
  { PRECONDITION(Harbor); Harbor->CmGadgetAppearance(); }
inline void THarborManagement::CmGadgetImage()
   { PRECONDITION(Harbor); Harbor->CmGadgetImage(); }
inline void THarborManagement::CmGadgetText()
  { PRECONDITION(Harbor); Harbor->CmGadgetText(); }
inline void THarborManagement::CmGadgetImageText()
   { PRECONDITION(Harbor); Harbor->CmGadgetImageText(); }
inline void THarborManagement::CmGadgetBeginGroup()
   { PRECONDITION(Harbor); Harbor->CmGadgetBeginGroup(); }

inline void THarborManagement::CeGadgetReset(owl::TCommandEnabler& ce)
   { PRECONDITION(Harbor); Harbor->CeGadgetReset(ce); }
inline void THarborManagement::CeGadgetDelete(owl::TCommandEnabler& ce)
   { PRECONDITION(Harbor); Harbor->CeGadgetDelete(ce); }
inline void THarborManagement::CeGadgetCopyImage(owl::TCommandEnabler& ce)
   { PRECONDITION(Harbor); Harbor->CeGadgetCopyImage(ce); }
inline void THarborManagement::CeGadgetPasteImage(owl::TCommandEnabler& ce)
   { PRECONDITION(Harbor); Harbor->CeGadgetPasteImage(ce); }
inline void THarborManagement::CeGadgetAppearance(owl::TCommandEnabler& ce)
   { PRECONDITION(Harbor); Harbor->CeGadgetAppearance(ce); }
inline void THarborManagement::CeGadgetImage(owl::TCommandEnabler& ce)
   { PRECONDITION(Harbor); Harbor->CeGadgetImage(ce); }
inline void THarborManagement::CeGadgetText(owl::TCommandEnabler& ce)
   { PRECONDITION(Harbor); Harbor->CeGadgetText(ce); }
inline void THarborManagement::CeGadgetImageText(owl::TCommandEnabler& ce)
   { PRECONDITION(Harbor); Harbor->CeGadgetImageText(ce); }
inline void THarborManagement::CeGadgetBeginGroup(owl::TCommandEnabler& ce)
   { PRECONDITION(Harbor); Harbor->CeGadgetBeginGroup(ce); }

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>
} // OwlExt namespace

#endif
