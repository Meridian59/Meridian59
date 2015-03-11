//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of the TTooltip class and helper objects
//----------------------------------------------------------------------------
#if !defined(OWL_TOOLTIP_H)
#define OWL_TOOLTIP_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>
#include <owl/commctrl.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

class _OWLCLASS TFont;
class _OWLCLASS TTooltip;

//
/// \class TToolInfo
// ~~~~~ ~~~~~~~~~
/// TToolInfo contains information about a particular tool (a tool is
/// either a window or an application-defined rectangular area within
/// a window's client area). For example, it contains the text
/// to be displayed in the tooltip window.
//
class _OWLCLASS TToolInfo : public TOOLINFO {
  public:

    // Default constructor of TToolInfo
    //
    TToolInfo(bool allocCache = false);

    // Constructor for a tooltip associated with a rectangular area
    // within a window's client area. The 'window' receives
    // 'TTN_NEEDTEXT' notifications if 'txt' is not provided.
    //
    TToolInfo(HWND window, const TRect& rect, uint toolId,
              LPCTSTR txt = LPSTR_TEXTCALLBACK);

    // String overload
    //
    TToolInfo(HWND window, const TRect& rect, uint toolId, const tstring& txt);

    // Constructor for a tooltip associated with a rectangular area
    // within a window's client area. The 'txtRes' specifies the ID
    // of the string to be used by the tooltip window.
    //
    TToolInfo(HWND window, const TRect&, uint toolId,
              int resId,  HINSTANCE txtResModule);

    // Constructor for tool implemented as windows (eg. child/controls).
    // 'parent' receives the 'TTN_NEEDTEXT' notification in case where
    // the default LPTSTR_TEXTCALLBACK is used for the 'txt' parameter.
    //
    TToolInfo(HWND parent, HWND toolHwnd,
              LPCTSTR txt = LPSTR_TEXTCALLBACK);

    // String overload
    //
    TToolInfo(HWND parent, HWND toolHwnd, const tstring& txt);

    // Constructor for tool implemented as windows (eg. child/controls).
    // 'resId' identifies the resource ID of the string to be used by
    // the tooltip window.
    //
    TToolInfo(HWND parent, HWND toolHwnd, int resId, HINSTANCE strResModule);

    /// \name Set data members of the TOOLINFO structure which identify each tool
    /// @{
    void        SetToolInfo(HWND toolWin, uint id, const TRect& rc);
    void        SetToolInfo(HWND toolWin, uint id);
    void        SetToolInfo(HWND toolWin, HWND parent);
    /// @}

    /// \name Set the text associated with a tooltip and the tool rectangles
    /// @{
    void        SetText(int resId, HINSTANCE hinstance);
    void        SetText(LPCTSTR text, bool copy = true);
    void        SetText(const tstring& text) {SetText(text.c_str(), true);}
    void        SetRect(const TRect& rect);
    /// @}

    //
    // Set flag: TTF_SUBCLASS, 
    // use this for controls in Modal Dialog only
    //
    void        EnableSubclassing(bool enable = true);

    // Helper routine which returns the 'HWND'/rect associated with a
    // particular tool.
    //
    HWND        GetToolWindow() const;
    void        GetToolRect(TRect& rect) const;

    // Helper routine to help determine whether a particular
    // location is pertinent to a tool
    //
    bool        IsPointInTool(HWND win, const TPoint& pt) const;

    // Copy ctr & assignment operator
    //
    TToolInfo(const TToolInfo&);
    TToolInfo& operator =(const TToolInfo&);

    // Checks whether two TToolInfo structures refer to the same tool
    //
    bool    operator==(const TToolInfo& ti) const;

    // Return/clear cached text
    //
    LPCTSTR GetCacheText() const;
    void    FlushCacheText();

  private:
    TAPointer<tchar> CacheText;
};

//
/// \class TTooltipHitTestInfo
// ~~~~~ ~~~~~~~~~~~~~~~~~~~
/// TTooltipHitTestInfo is a thin [very thin] wrapper around
/// the TTHITTESTINFO structure, used to determined whether a
/// point is within the bounding rectangle of a particular tool.
/// It's a place-holder for future ObjectWindows enhancements to
/// tooltip hit-testing.
//
class _OWLCLASS TTooltipHitTestInfo : public TTHITTESTINFO {
};

//
/// \class TTooltipEnabler
// ~~~~~ ~~~~~~~~~~~~~~~
/// TTooltipEnabler is the object forwarded along the command-chain to
/// retrieve the tip text of a tool. The object is not a 'true' command
/// enabler [i.e. invoking SetCheck or Enable do not modify the state of the
/// command associated with the tool]; however, by using the 'CommandEnabler'
/// setup retrieving the text of tools, the potential of the commands are
/// given first crack at customizing the text.
//
class _OWLCLASS TTooltipEnabler : public TCommandEnabler {
  public:
    TTooltipEnabler(TTooltipText& tt, TWindow::THandle hReceiver);

    // Override member functions of TCommandEnabler
    //
    virtual void SetText(LPCTSTR text);
    virtual void SetCheck(int check);

    void SetText(const tstring& text) {SetText(text.c_str());}

  protected_data:

    /// Reference to structure accompanying a notification requesting
    /// for the text of a particular tool.
    //
    TTooltipText& TipText;
};

//
/// \class TTooltip
// ~~~~~ ~~~~~~~~
/// TTooltip encapsulates a tooltip window - i.e. a small popup window
/// that displays a single line of descriptive test giving the purpose
/// of the item underneath the current cursor location.
//
class _OWLCLASS TTooltip : public TControl {
  public:
    TTooltip(TWindow* parent, bool alwaysTip = true, TModule* module = 0);

    //
    /// Constructor that allows wfAutoCreate to be passed as an argument, so that the tooltip
    /// is automatically created when the parent is.
    ///
    /// No other flags are supported and will cause an assertion in diagnostics builds.
    /// The tooltip is initialized with the default style TTS_ALWAYSTIP, meaning that the tooltip
    /// will be active regardless of whether its parent is active or inactive. You can override the
    /// tooltip styles using ModifyStyle and MofifyExStyle.
    //
    TTooltip(TWindow* parent, TWindowFlag, TModule* module = 0);

    TTooltip(HWND hWnd, TModule* module = 0);

    ~TTooltip();

    // 
    /// Factory for TTooltip
    /// If the given parent has a valid handle (IsWindow), i.e. has itself already been created,
    /// then the underlying tool-tip is created immediately by calling Create. Otherwise, the flag
    /// wfAutoCreate is set, so that the tool-tip is created when the parent is.
    //
    static TTooltip* Make(TWindow* parent, bool alwaysTip = true, TModule* = 0);

    /// Set state of tooltip
    //
    void        Activate(bool activate = true);

    // Add/remove/enumerate tools
    //
    bool        AddTool(const TToolInfo&);
    void        DeleteTool(const TToolInfo&);
    bool        EnumTools(uint index, TToolInfo&) const;

    //
    /// Adds a tip for the given control.
    //
    bool AddTool(HWND ctrl, const tstring& tip, bool enableSubclassing = true);

    //
    /// Adds a tip for the control identified by the given identifier.
    //
    bool AddTool(int ctrlId, const tstring& tip, bool enableSubclassing = true)
    {return AddTool(GetParent()->GetDlgItem(ctrlId), tip, enableSubclassing);}

    //
    /// Adds a tip for the given control; the tip is given as a resource string id.
    //
    bool AddTool(HWND ctrl, int tipResId, TModule* m = 0, bool enableSubclassing = true);

    //
    /// Adds a tip for the control identified by the given identifier; the tip is given as a resource string id.
    //
    bool AddTool(int ctrlId, int tipResId, TModule* m = 0, bool enableSubclassing = true)
    {return AddTool(GetParent()->GetDlgItem(ctrlId), tipResId, m, enableSubclassing);}

    //
    /// Adds a tip for the given area in the parent window.
    /// The tip area (tool) is identified by the given tool ID.
    //
    bool AddTool(const TRect& tipArea, uint toolId, const tstring& tip, bool enableSubclassing = true);

    //
    /// Adds a tip for the given area in the parent window.
    /// The tip area (tool) is identified by the given tool ID.
    /// The tip is given as a string resource id.
    //
    bool AddTool(const TRect& tipArea, uint toolId, int tipResId, TModule* m = 0, bool enableSubclassing = true);

    // Update/retrive attribute of tools
    //
    bool        GetCurrentTool(TToolInfo&) const;
    void        GetToolText(TToolInfo&) const;
    uint        GetToolCount() const;
    bool        GetToolInfo(TToolInfo&) const;
    void        SetToolInfo(const TToolInfo&);
    void        UpdateTipText(const TToolInfo&);

    bool        HitTest(TTooltipHitTestInfo&) const;
    void        NewToolRect(const TToolInfo&);
    void        RelayEvent(MSG&);

    // New IE messages
    //
    void        GetMargins(int& left, int& top, int& right, int& bottom) const; // Version 4.70
    void        SetMargins(int left, int top, int right, int bottom); // Version 4.70

    void        SetDelayTime(uint flag, int delay); // Version 4.70
    int          GetDelayTime(uint flag) const; // Version 4.70

    TResult     GetMaxTipWidth() const; // Version 4.70
    void        SetMaxTipWidth(int width); // Version 4.70

    TColor       GetBkColor() const;
    void         SetBkColor(const TColor& clr);

    TColor       GetTextColor() const;
    void         SetTextColor(const TColor& clr);

    void         HideTip();
    void        TrackActivate(const TToolInfo& info, bool activate = true); // Version 4.70.
    void        TrackPosition(const TPoint& pos);    
    void         Update();

  protected:
    // Override TWindow virtual member functions
    //
    virtual TGetClassNameReturnType GetClassName();

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TTooltip(const TTooltip&);
    TTooltip& operator =(const TTooltip&);


};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementation
//

//
/// Sets the text of this tool by providing a string resource identifier and the
/// handle of the instance containing that resource.
//
inline void TToolInfo::SetText(int resId, HINSTANCE hinstance)
{
  lpszText = CONST_CAST(LPTSTR, MAKEINTRESOURCE(resId));
  hinst = hinstance;
}

//
/// Sets the bounding rectangle of the tool. The coordinates are relative to the
/// upper-left corner of the client area of the window.
/// \note	This flag is only valid if the tool is a rectangle within the window and
/// not a control parented to the window.
//
inline void TToolInfo::SetRect(const TRect& rc)
{
  rect = rc;
}

//
/// Enable Subclassing have to be called for Dialog controls
//
inline void TToolInfo::EnableSubclassing(bool enable)
{
  enable ? (uFlags |= TTF_SUBCLASS) : (uFlags &= ~TTF_SUBCLASS);
}

//
/// If the IDs of two tools (or the Window Handles of two tools
/// for cases when 'uFlags & TTF_IDISHWND') are similar and the
/// handle of the window containing them matches, we'll infer that
/// the structures are referring to the same tool.
//
inline bool TToolInfo::operator==(const TToolInfo& ti) const
{
  return (uId == ti.uId && hwnd == ti.hwnd) ? true : false;
}

//
//
//
inline LPCTSTR
TToolInfo::GetCacheText() const
{
  return (LPTSTR)(CONST_CAST(TToolInfo*, this)->CacheText);
}

//
//
//
inline void
TToolInfo::FlushCacheText()
{
  CacheText = 0;
}

//----------------------------------------------------------------------------
// When ObjectWindows relies exclusively on the native implementation of
// TOOLTIPS, the methods to manipulate the control are simple wrappers around
// predefined TOOLTIP messages (TTM_xxxx).
//

//
/// Activates or deactivates the tooltip control. If 'activate' is true,
/// the tooltip control is activated, If it is false the tooltip control
/// is deactivated.
//
inline void TTooltip::Activate(bool activate)
{
  SendMessage(TTM_ACTIVATE, TParam1(activate));
}

//
/// Registers a tool with the tooltip control. The 'TToolInfo' parameter
/// contains information that the tooltip control needs to display text
/// for the tool.
/// \note This method does not check for duplicate tool ids!
//
inline bool TTooltip::AddTool(const TToolInfo& ti)
{
  return SendMessage(TTM_ADDTOOL, 0, TParam2(&ti)) != 0;
}

//
/// Removes a tool from the tooltip control. You must invoke the
/// 'SetToolHandle' or 'SetToolId' method of TToolInfo to identify
/// the tool to remove. [i.e. the 'hwnd' or 'uId' members of the
/// TToolInfo must identify the tool].
//
inline void TTooltip::DeleteTool(const TToolInfo& ti)
{
  SendMessage(TTM_DELTOOL, 0, TParam2(&ti));
}

//
/// Retrieves the information that the tooltip control maintains about the specified
/// tool. Returns true if successul or false otherwise.
/// \note The index is zero-based and the TToolInfo structure receives the
/// information about the tool.
//
inline bool TTooltip::EnumTools(uint index, TToolInfo& ti) const
{
  return CONST_CAST(TTooltip*, this)->SendMessage(TTM_ENUMTOOLS,
                                                  index, TParam2(&ti)) != 0;
}

//
/// Retrieves information about the current tool. The 'TToolInfo'
/// parameter receives the information. Returns true if successful or
/// false otherwise.
//
inline bool TTooltip::GetCurrentTool(TToolInfo& ti) const
{
  return CONST_CAST(TTooltip*, this)->SendMessage(TTM_GETCURRENTTOOL,
                                                  0, TParam2(&ti)) != 0;
}

//
/// Retrieves the text associated with the specified tool. You must 
/// invoke the 'SetToolHandle' or 'SetToolId' method of TToolInfo to 
/// identify the tool. [i.e. the 'hwnd' or 'uId' members of the
/// TToolInfo must identify the tool].
//
inline void TTooltip::GetToolText(TToolInfo& ti) const
{
  CONST_CAST(TTooltip*, this)->SendMessage(TTM_GETTEXT, 0, TParam2(&ti));
}

//
/// Returns the number of tools currently registered with the tooltip control.
//
inline uint TTooltip::GetToolCount() const
{
  return (uint)CONST_CAST(TTooltip*, this)->SendMessage(TTM_GETTOOLCOUNT);
}

//
/// Retrieves the information that the tooltip control maintains about
/// a tool. You must invoke the 'SetToolHandle' or 'SetToolId' method 
/// of TToolInfo to identify the tool. [i.e. the 'hwnd' or 'uId' members
/// of the TToolInfo must identify the tool].
//
inline bool TTooltip::GetToolInfo(TToolInfo& ti) const
{
  return CONST_CAST(TTooltip*, this)->SendMessage(TTM_GETTOOLINFO, 0,
                                                  TParam2(&ti)) != 0;
}

//
/// Sets the information that the tooltip control maintains for a
/// particular tool. You must invoke the 'SetToolHandle' or 
/// 'SetToolId' method of TToolInfo to identify the tool. 
/// [i.e. the 'hwnd' or 'uId' members of the TToolInfo must 
/// identify the tool].
//
inline void TTooltip::SetToolInfo(const TToolInfo& ti)
{
  SendMessage(TTM_SETTOOLINFO, 0, TParam2(&ti));
}

//
/// Sets the text of a tool. You must invoke the 'SetToolHandle' or
/// 'SetToolId' method of TToolInfo to identify the tool.
/// [i.e. the 'hwnd' or 'uId' members of the TToolInfo must 
/// identify the tool]  
//
inline void TTooltip::UpdateTipText(const TToolInfo& ti)
{
  SendMessage(TTM_UPDATETIPTEXT, 0, TParam2(&ti));
}

//
/// Determines whether a tool is within a specified point. The method
/// also retrieves information about the tool if one is identified at
/// that location. Returns true if a tool if found at the location or
/// false otherwise.
//
inline bool TTooltip::HitTest(TTooltipHitTestInfo& ttHTInfo) const
{
  return CONST_CAST(TTooltip*, this)->SendMessage(TTM_HITTEST, 0,
                                                  TParam2(&ttHTInfo)) != 0;
}

//
/// Updates the bounding rectangle of a tool. You must invoke the 'SetToolId'
/// method of TToolInfo to identify the tool. [i.e. the 'uId' member
/// of the TToolInfo must identify the tool].
//
inline void TTooltip::NewToolRect(const TToolInfo& ti)
{
  SendMessage(TTM_NEWTOOLRECT, 0, TParam2(&ti));
}

//
/// Passes a mouse message to the tooltip control for processing.
//
inline void TTooltip::RelayEvent(MSG& msg)
{
  SendMessage(TTM_RELAYEVENT, 0, TParam2(&msg));
}

//
/// Sets the initial, reshow, and autopopup durations for a tooltip control. The
/// 'flag' parameter can be one of the following:
/// - \c \b TTDT_AUTOMATIC: Automatically calculates the initial, reshow, and autopopup
/// duration based on 'delay'.
/// - \c \b TTDT_AUTOPOP: Sets the length of time before the tooltip window is hidden if the
/// cursor remains:
///
///    stationary in the tool's bounding rectangle
///
///    after the tooltip window has disappeared.
/// - \c \b TTDT_INITIAL: Sets the length of time that the cursor must remain stationary
/// within the bounding rectangle before the tooltip window is displayed.
/// - \c \b TTDT_RESHOW: Sets the length of time before subsequent tooltip windows are
/// displayed when the cursor is moved from one tool to another.
///
/// \note The 'delay' duration is in milliseconds.
//
inline void TTooltip::SetDelayTime(uint flag, int delay)
{
  SendMessage(TTM_SETDELAYTIME, flag, TParam2(delay));
}

// Version 4.70
inline int TTooltip::GetDelayTime(uint flag) const
{
  return static_cast<int>(CONST_CAST(TTooltip*, this)->SendMessage(TTM_GETDELAYTIME,flag));
}

// Version 4.70
inline void TTooltip::GetMargins(int& left, int& top, int& right, int& bottom) const
{
  TRect rec;
  CONST_CAST(TTooltip*, this)->SendMessage(TTM_GETMARGIN,0,TParam2(&rec));
  left    = rec.left;
  top     = rec.top;
  right   = rec.right;
  bottom  = rec.bottom;
}

// Version 4.70
inline void TTooltip::SetMargins(int left, int top, int right, int bottom)
{
  TRect rec(left,top,right,bottom);
  SendMessage(TTM_SETMARGIN,0,TParam2(&rec));
}

// Version 4.70
inline TResult TTooltip::GetMaxTipWidth() const
{
  return CONST_CAST(TTooltip*, this)->SendMessage(TTM_GETMAXTIPWIDTH);
}

// Version 4.70
inline void TTooltip::SetMaxTipWidth(int width)
{
  SendMessage(TTM_SETMAXTIPWIDTH,0,TParam2(width));
}

// Version 4.70
inline TColor TTooltip::GetBkColor() const
{
  return static_cast<COLORREF>(CONST_CAST(TTooltip*, this)->SendMessage(TTM_GETTIPBKCOLOR));
}

// Version 4.70
inline void TTooltip::SetBkColor(const TColor& clr)
{
  SendMessage(TTM_SETTIPBKCOLOR,TParam2(clr));
}

// Version 4.70
inline TColor TTooltip::GetTextColor() const
{
  return static_cast<COLORREF>(CONST_CAST(TTooltip*, this)->SendMessage(TTM_GETTIPTEXTCOLOR));
}

// Version 4.70
inline void TTooltip::SetTextColor(const TColor& clr)
{
  SendMessage(TTM_SETTIPTEXTCOLOR,TParam2(clr));
}

// Version 4.70
inline void TTooltip::HideTip()
{
  SendMessage(TTM_POP);
}

// Version 4.70
inline void TTooltip::TrackActivate(const TToolInfo& info, bool activate)
{
  SendMessage(TTM_TRACKACTIVATE,TParam1((BOOL)activate),TParam2(&info));
}

// Version 4.70
inline void TTooltip::TrackPosition(const TPoint& pos)
{
  SendMessage(TTM_TRACKPOSITION,0,TParam2(MkUint32((int16)pos.x,(int16)pos.y)));
}

// Version 4.71
inline void TTooltip::Update()
{
  SendMessage(TTM_UPDATE);
}



} // OWL namespace


#endif  // OWL_TOOLTIP_H
