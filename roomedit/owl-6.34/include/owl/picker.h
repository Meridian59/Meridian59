//------------------------------------------------------------------------------
// ObjectWindows, Copyright (c) 1998 by Yura Bidus.                           
//                                                                            
// Used ideas from Chris Maunder, Alexander Bischofberger, Dieter Windau,     
//                 Joseph Parrello,                                           
//                                                                            
// THIS CLASS AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF         
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO        
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A             
// PARTICULAR PURPOSE.                                                        
/// \file                                                                            
/// Definition of classes TPickerCell, TColorCell, TBitmapCell, TTextCell,     
/// TGridPicker, TPopupPicker, TColorPicker, TPickerGadget,                    
/// TColorPickerGadget and TBitmapPickerGadget                                 
//------------------------------------------------------------------------------

#if !defined(__OWL_PICKER_H) // Sentry, use file only if it's not already included.
#define __OWL_PICKER_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif


// Macro defining class name (usable in resource definition)
//
#if !defined(OWL_PICKER)
#if defined(UNICODE)
# define OWL_PICKER  L"OWL_Picker"
#else
# define OWL_PICKER  "OWL_Picker"
#endif
#endif

#if !defined(RC_INVOKED)

#include <owl/bitset.h>
#include <owl/buttonga.h>
#include <owl/popupgad.h>


namespace owl {


// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
//
#include <owl/preclass.h>

class _OWLCLASS TGridPicker;
class _OWLCLASS TCelArray;
class _OWLCLASS TTooltip;

/// \addtogroup newctrl
/// @{
/// \class TPickerCell
// ~~~~~ ~~~~~~~~~~~
/// bsabstract base class
//
class _OWLCLASS TPickerCell: public TBitFlags<uint> {
  public:
//FMM
DECLARE_CASTABLE;
    /// Cell border styles. These match TIUBorder's TStyles exactly
    //
    enum TBorderStyle {
      None,             ///<  No border painted at all
      Plain,            ///<  Plain plain window frame
      Raised,           ///<  Status field style raised
      Recessed,         ///<  Status field style recessed
      Embossed,         ///<  Grouping raised emboss bead
      Grooved,          ///<  Grouping groove
      ButtonUp,         ///<  Button in up position
      ButtonDn,         ///<  Button in down position
      WndRaised,        ///<  Raised window outer+inner edge
      WndRecessed,      ///<  Input field & other window recessed
      WellSet,          ///<  Well option set (auto grows + 1)
      Flat,             ///<  Use flat style
    };

    TPickerCell(int id=0,TBorderStyle borderStyle = Flat);
    virtual  ~TPickerCell();

    virtual void Paint(TDC& dc, TRect* r=0, bool drawbrd = true);
    virtual void PaintCell(TDC& dc, TRect& rect) = 0;  // abstract function
    virtual void PaintBorder(TDC& dc, TRect& rect);

    virtual void EnterMouse();
    virtual void LeaveMouse();
    virtual void Select(bool sel=true);

    // get/set
    int             GetId();
    void          SetId(int id);
    TRect&         GetBounds();

    void          SetBorderStyle(TBorderStyle bs);
    TBorderStyle  GetBorderStyle() const;

    // query desired bounds
    virtual void   QueryBounds(TRect& rect);
    virtual bool  PtIn(const TPoint& pt);

  protected:
    enum TCellState{
      csMouseIn      = 0x0001,
      csSelected    = 0x0002,
    };

    virtual void SetParent(TGridPicker* parent);

  protected:
    TRect         Bounds;
    int            Id;
    TGridPicker*  Parent;
    TBorderStyle  BorderStyle;     // Style of border to use

  friend class _OWLCLASS TGridPicker;
};

//
/// \class TColorCell
// ~~~~~ ~~~~~~~~~~
//
class _OWLCLASS TColorCell: public TPickerCell {
  public:
    TColorCell(int id=0, const TColor& clr=TColor::None,TBorderStyle borderStyle = Flat);

    virtual void   PaintCell(TDC& dc, TRect& rect);
    void           SetColor(const TColor& clr);
    const TColor& GetColor() const;

  protected:
    TColor Color;
};

//
/// \class TBitmapCell
// ~~~~~ ~~~~~~~~~~~
//
class _OWLCLASS TBitmapCell: public TPickerCell {
  public:
    TBitmapCell(int id=0, TCelArray* array=0, int index=0,TBorderStyle borderStyle = Flat);
    virtual  ~TBitmapCell();

    virtual void   PaintCell(TDC& dc, TRect& rect);
    virtual void   QueryBounds(TRect& rect);
    void           SetIndex(int index);
    int           GetIndex();
    void           SetCelArray(TCelArray* array, TAutoDelete=NoAutoDelete);
    TCelArray*     GetCelArray();

  protected:
    TCelArray*  CelArray;
    int         Index;
    bool         ShouldDelete;
};


//
/// \class TTextCell
// ~~~~~ ~~~~~~~~~
//
class _OWLCLASS TTextCell: public TPickerCell {
  public:
    TTextCell(int id=0, const tstring& text = tstring(), TBorderStyle borderStyle = Flat);

    virtual void   PaintCell(TDC& dc, TRect& rect);
    void            SetText(const tstring& text);
    const tstring& GetText() const;
    void            SetFormat(uint format);
    const uint     GetFormat() const;

  protected:
    tstring    Text;
    uint          Format;
};


class TPickerCellArray;
//    TIPtrArray<TPickerCell*> Cells;

//
/// \class TGridPicker
// ~~~~~ ~~~~~~~~~~~~
//
class _OWLCLASS TGridPicker : public TControl, public TBitFlags<uint> {
  public:
    enum TPickerStyles{       // Have to be exactly as TCellState
      psTrackMouse     = 0x0001,
      psShrinkToCells = 0x0002,
      psFlatSyle      = 0x0004,
      psMessageTips   = 0x0008,
      psLocalTips     = 0x0010,
    };

    TGridPicker(TWindow*  parent,
                int       id,
                int x, int y, int w=0, int h=0,
                TModule*   module = 0);
    TGridPicker(TWindow* parent,
                int resourceId,
                TModule* module = 0);
    ~TGridPicker();

    void           SetSelection(int sel = 0);
    int           GetSelection();
    TPickerCell*   GetSelectedCell();
    int           GetFocus();
    TPickerCell*   GetFocusedCell(); // return focused cell
    TPickerCell*   GetDefaultCell(); // return child of default cell
    TPickerCell*   GetCustomCell();  // return child of default custom cell

    void           SetColumns(int colmn = 4);
    int           GetColumns();

    void           SetFont(const TFont& font);
    TFont*         GetFont();

    void           SetTarget(const TWindow* wnd);
    TWindow*      GetTarget();

    void           AddCell(TPickerCell* cells, bool select = false);
    TPickerCell*   RemoveCell(int loc);

    // styles
    void  ModifyPopStyle(uint off, uint on);
    uint  GetPopStyle();
    virtual bool   IsFlatStyle();
    void   EnableDefault(LPCTSTR text=0, int resId = 0, TPickerCell* cl=0); // =0 disables it

    void EnableDefault(const tstring& text, int resId = 0, TPickerCell* cl = 0)
    {EnableDefault(text.c_str(), resId, cl);}

    void   EnableButton(LPCTSTR text=0, int resId = 0, TPickerCell* cl=0);  // =0 disables it

    void EnableButton(const tstring& text, int resId = 0, TPickerCell* cl = 0)
    {EnableButton(text.c_str(), resId, cl);}

    // Retrieves/assigns tooltip of/to window
    //
    TTooltip* GetTooltip() const;
    void       EnableTooltip(bool enable=true);
    void      SetTooltip(TTooltip* tooltip);

    // Changes the margins and initiates a layout session
    void  SetMargins(TMargins& margins);
    // relayout cells - called internally
    virtual void Layout();

  protected:
    enum TCellState{  // publip the same as TPickerStyles
      ppTrackMouse  = 0x0001,
      ppShrCells    = 0x0002,
      ppFlatStyle   = 0x0004,
      ppMessageTips = 0x0008,
      ppShowTipsCell= 0x0010,

      // internal usage
      ppFocus        = 0x0040,
      ppPushed      = 0x0080,
      ppShowDefItem  = 0x0100,
      ppShowCustItem= 0x0200,
      ppButtonSel   = 0x0400,
      ppCustomOpen  = 0x0800,
      ppSelected    = 0x1000,
    };

    // inherited virtuals
    virtual owl::TWindow::TGetClassNameReturnType GetClassName();
    virtual void     GetWindowClass(WNDCLASS& wndClass);
    virtual void     Paint(TDC& dc, bool erase, TRect& rect);
    virtual void     SetupWindow();
    virtual bool    PreProcessMsg(MSG& msg);

  protected: // event handlers
    //JJH - removed, no definition provided
    #if !defined(STRIP_UNDEFINED)
    uint EvGetDlgCode(const MSG* msg) { return TControl::EvGetDlgCode(msg); }
    void EvActivateApp(bool active, HTASK hTask) { TControl::EvActivateApp(active, hTask); }
    #endif
    void EvSetFocus(THandle hWndLostFocus);
    void EvKillFocus(THandle hWndGetFocus);
    void EvLButtonDown(uint modKeys, const TPoint& point);
    void EvLButtonDblClk(uint modKeys, const TPoint& point);
    void EvLButtonUp(uint modKeys, const TPoint& point);
    void EvMouseMove(uint modKeys, const TPoint& point);
    void EvKeyDown(uint key, uint repeatCount, uint flags);
    void EvKeyUp(uint key, uint repeatCount, uint flags);
    void EvCancelMode();

    // New Virtuals
    /// set bounds for cell
    virtual void     SetBounds(TPickerCell& cell, const TRect& rect);
    /// load tiptext from resource
    virtual tstring  LoadTipText(int resId);
    /// send WM_MENUSELECT and WM_ENTERIDLE to frame window
    virtual void    SetStatusBarMessage(int id);
    /// paint divider
    virtual void     PaintDivider(TDC& dc, const TRect& rect);
    /// call user defined custom box if pressed custom button
    virtual bool     CallCustomBox();
    /// notify parent on event -> BN_CLICKED and BN_HILITE
    virtual void    NotifyParent(uint code);
    /// do noting here
    virtual void    ExitPicker(bool status = false);

    // Internal Utility functions
    void   InitPicker();
    void  ClearCapture();
    int    Contains(const TPoint& point);
    void  GetCellRect(TRect& rect);
    void  ClearCell();
    void  RefocusCell(int loc);
    void  ApplyChanges();

    // for derived classes
    int            Add(TPickerCell* cell);
    TPickerCell*  Remove(int loc);
    TPickerCell*   GetCell(int loc);
    uint           Size();

  protected:
    // TIPtrArray<TPickerCell*> Cells;
    TPickerCellArray*  Cells;
    int           Selected;
    int           Focused;
    int            NRows;
    int            NColumns;
    int           TextHeight;
    TMargins      Margins;
    TFont*        Font;
    TWindow*      Target;
    TTooltip*      Tooltip;

    DECLARE_RESPONSE_TABLE(TGridPicker);
};

//
/// \class TPopupPicker
// ~~~~~ ~~~~~~~~~~~~
//
class _OWLCLASS TPopupPicker : public TGridPicker{
  public:
    TPopupPicker(TWindow*  parent,
                 int       id,
                 int x, int y, int w=0, int h=0,
                 TModule*   module = 0);
    TPopupPicker(TWindow* parent,
                 int resourceId,
                 TModule* module = 0);

    virtual bool  ShowPicker(TPoint& p, TRect& rect);
    virtual void  Paint(TDC& dc, bool erase, TRect& rect);

  protected:
    void EvActivateApp(bool active, HTASK hTask);
    void EvKillFocus(THandle hWndGetFocus );
    void EvLButtonDown(uint modKeys, const TPoint& point);
    virtual void  GetWindowClass(WNDCLASS& wndClass);
    virtual void  ExitPicker(bool status = false);

  DECLARE_RESPONSE_TABLE(TPopupPicker);
};

//
// class TColorPicker
// ~~~~~ ~~~~~~~~~~~~
//
class _OWLCLASS TColorPicker: public TPopupPicker{
  public:
    TColorPicker(TWindow*  parent,
                 int       id,
                 int x, int y, int w=0, int h=0,
                 TModule*   module = 0);
    TColorPicker(TWindow* parent,
                 int resourceId,
                 TModule* module = 0);
    ~TColorPicker();

    // Virtual functions overriding
    virtual void   Paint(TDC& dc, bool erase, TRect& rect);
    virtual void   SetupWindow();
    virtual void   NotifyParent(uint code);

    void           SetCustomColors(TColor* colors);
    const TColor&  GetCustomColor() const;

    // Event handlers
  protected:
    void EvPaletteChanged(THandle hWndPalChg);
    bool EvQueryNewPalette();

    virtual bool     CallCustomBox();

  protected_data:
    TColor     CustomColor;
    TPalette*  Palette;
    TColor*   CustomColors;
    TPalette*  WorkPalette;
    bool      PaletteRealized;

  DECLARE_RESPONSE_TABLE(TColorPicker);
};

//
// class TPickerGadget
// ~~~~~ ~~~~~~~~~~~~~
//
class _OWLCLASS TPickerGadget : public TPopupButtonGadget {
  public:
    TPickerGadget(TPopupPicker& picker,
                  TResId          glyphResIdOrIndex,
                  TStyle         style = sBitmapText,
                  TPopupType    poptype = DownArrowAction,
                  TType         type = Command,
                  bool          enabled = false,
                  TState         state = Up,
                  bool          sharedGlyph = false,
                  uint           numChars = 4);
    virtual ~TPickerGadget();

    virtual void  PopupAction();
    TGridPicker*  GetPicker();
    TPickerCell*  GetSelCel();

  protected:
    virtual void Created();
    virtual void Inserted();

    // call on exit from Picker if not canceled
    virtual void GetSelection();

  protected:
    TPopupPicker* Picker;
    TPickerCell*  Selected;        ///< currect selected cell
};


//
// class TColorPickerGadget
// ~~~~~ ~~~~~~~~~~~~~~~~~~
//
class _OWLCLASS TColorPickerGadget : public TPickerGadget {
  public:
    TColorPickerGadget(  TColorPicker&  picker,
                        TResId          glyphResIdOrIndex,
                        TStyle         style = sBitmapText,
                        TPopupType    poptype = DownArrowAction,
                        TType         type = Command,
                        bool          enabled = false,
                        TState         state = Up,
                        bool          sharedGlyph = false,
                        uint           numChars = 4);
  protected:
    virtual void GetSelection();

    virtual void PaintFace(TDC& dc, const TRect& rect);

    // helper function
    virtual void GetFillRect(TRect& rect, const TRect& faceRect);

  protected:
    TColor  ColorSel;         ///< currect selected color
};

//
/// \class TBitmapPickerGadget
// ~~~~~ ~~~~~~~~~~~~~~~~~~~
//
class _OWLCLASS TBitmapPickerGadget : public TPickerGadget {
  public:
    TBitmapPickerGadget(TPopupPicker&  picker,
                        TResId          glyphResIdOrIndex,
                        TStyle         style = sBitmapText,
                        TPopupType    poptype = DownArrowAction,
                        TType         type = Command,
                        bool          enabled = false,
                        TState         state = Up,
                        bool          sharedGlyph = false,
                        uint           numChars = 4);

  protected:
    // override virtuals
    virtual void  BuildCelArray();
    virtual void  GetSelection();

    // utility to copy bitmap into CellArray
    void CopyCellBitmap();
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//
// Inlines
//

//
// class TPickerCell
// ~~~~~ ~~~~~~~~~~~
//
inline 
TPickerCell::TPickerCell(int id, TBorderStyle borderStyle)
:
 Id(id), Parent(0)
{
  SetBorderStyle(borderStyle);
}

//
inline
TPickerCell::~TPickerCell()
{
}

//
inline int
TPickerCell::GetId()
{
  return Id;
}

//
inline void
TPickerCell::SetId(int id)
{
  Id = id;
}

//
inline TRect&
TPickerCell::GetBounds()
{
  return Bounds;
}

//
inline bool
TPickerCell::PtIn(const TPoint& pt)
{
  return Bounds.Contains(pt);
}

//
inline void
TPickerCell::SetParent(TGridPicker* parent)
{
  Parent=parent;
}

//
/// Return the borderstyle the cell is using
//
inline TPickerCell::TBorderStyle 
TPickerCell::GetBorderStyle() const 
{
  return BorderStyle;
}

//
// class TColorCell
// ~~~~~ ~~~~~~~~~~
//
inline
TColorCell::TColorCell(int id, const TColor& clr,TBorderStyle borderStyle)
:
  TPickerCell(id,borderStyle),Color(clr)
{
}

//
inline void
TColorCell::SetColor(const TColor& clr)
{
  Color = clr;
}

//
inline const TColor&
TColorCell::GetColor() const
{
  return Color;
}

//
// class TBitmapCell
// ~~~~~ ~~~~~~~~~~~
//
inline void
TBitmapCell::SetIndex(int index)
{
  Index = index;
}

//
inline int
TBitmapCell::GetIndex()
{
  return Index;
}

//
inline TCelArray*
TBitmapCell::GetCelArray()
{
  return CelArray;
}

//
// class TTextCell
// ~~~~~ ~~~~~~~~~
//
inline
TTextCell::TTextCell(int id, const tstring& text,TBorderStyle borderStyle)
:
  TPickerCell(id,borderStyle),
  Text(text),
  Format(DT_CENTER|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOCLIP)
{
}

//
inline void
TTextCell::SetText(const tstring& text)
{
  Text = text;
}

//
inline const tstring&
TTextCell::GetText() const
{
  return Text;
}

//
inline void
TTextCell::SetFormat(uint format)
{
  Format = format;
}

//
inline const uint 
TTextCell::GetFormat() const 
{
  return Format;
}

//
// class TGridPicker
// ~~~~~ ~~~~~~~~~~~~
//
inline int 
TGridPicker::GetSelection() 
{
  return Selected-3;
}

//
inline TTooltip* 
TGridPicker::GetTooltip() const 
{
  return Tooltip;
}

//
inline TPickerCell* 
TGridPicker::GetSelectedCell() 
{
  return Selected < 0 ? 0 : GetCell(Selected);
}

//
inline int 
TGridPicker::GetFocus() 
{
  return Focused-3;
}

//
inline TPickerCell* 
TGridPicker::GetFocusedCell() 
{
  return Focused < 0 ? 0 : GetCell(Focused);
}

//
inline int 
TGridPicker::GetColumns() 
{
  return NColumns;
}

//
inline TFont* 
TGridPicker::GetFont() 
{
  return Font;
}

//
inline uint 
TGridPicker::GetPopStyle() 
{
  return Bits&(ppTrackMouse|ppShrCells|ppFlatStyle|ppFlatStyle|
               ppMessageTips|ppShowTipsCell);
}

//
inline bool 
TGridPicker::IsFlatStyle() 
{
  return IsSet(ppFlatStyle);
}

//
inline void 
TGridPicker::SetBounds(TPickerCell& cell, const TRect& rect) 
{
  cell.Bounds = rect;
}

//
// class TColorPicker
// ~~~~~ ~~~~~~~~~~~~

//
inline const TColor&
TColorPicker::GetCustomColor() const 
{
  return CustomColor;
}

//
// class TPickerGadget
// ~~~~~ ~~~~~~~~~~~~~
//
inline TGridPicker* 
TPickerGadget::GetPicker()
{
  return Picker;
}
//
inline TPickerCell* 
TPickerGadget::GetSelCel()
{
  return Selected;
}

//==============================================================================

} // OWL namespace

#endif // RC_INVOKED

#endif  // __OWLEXT_PICKER_H

