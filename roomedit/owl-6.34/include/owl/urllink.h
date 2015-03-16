//------------------------------------------------------------------------------
// ObjectWindows. Copyright (c) 1998 by Yura Bidus  
//
/// \file
/// TUrlLink static control. Will open the default browser with the given URL
/// when the user clicks on the link.
//
//------------------------------------------------------------------------------

#if !defined(OWL_URLLINK_H)
#define OWL_URLLINK_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

// Macro defining class name (usable in resource definition)
#if !defined(OWL_URLLINK)
#  if defined(UNICODE)
#    define OWL_URLLINK L"OWL_UrlLink"
#  else
#    define OWL_URLLINK "OWL_UrlLink"
#  endif
#endif

#if !defined(RC_INVOKED)

#include <owl/static.h>
#include <owl/color.h>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

class _OWLCLASS TTooltip;

//
/// \class TUrlLink
// ~~~~~ ~~~~~~~~
/// Derived from TStatic, TUrlLink will open the default browser with the given URL
/// when the user clicks on the link.
/// 
/// This is a simple hyperlink control that can be plugged into any dialog. The
/// hyperlink is initially colored blue, but changes color when the cursor is over
/// it, and after the user has clicked on it. The cursor that appears when the mouse
/// pointer is over the link can easily be set using SetCursor(), as can the link
/// colors and underlining. The default cursor is a small pointing hand (cursor from
/// OWL.DLL or #106 from the winhlp32.exe). There is also a ToolTip for the link
/// that displays the underlying URL of the control. The control can auto-size
/// itself to fit the size of the caption (to preserve a true hyperlink look and
/// feel). The resizing will honor the SS_CENTERIMAGE, SS_LEFT, SS_RIGHT and
/// SS_CENTER flags. To actually follow the link, "ShellExecute" is called to open
/// the URL, but if this fails, then the registry is examined in order to find a
/// likely candidate for .html files. If one is found then this it is launched with
/// the hope that it can handle the URL string supplied. In any case, an error
/// message is displayed on failure.
///
/// Examples
/// - In a dialog constructor:
/// \code
/// TUrlLink* link = new TUrlLink(this, IDC_LINK);
/// link->SetURL("www.automedi.com");
/// \endcode
/// - In a dialog resource:
/// \code
///  CONTROL "Go To AutoMedia.", IDC_LINK, "OWL_UrlLink", SS_LEFT | WS_CHILD |
/// WS_VISIBLE, 52, 32, 96, 12, 0
/// \endcode
/// \image html bm269.BMP
//
class _OWLCLASS TUrlLink : public TStatic 
{
  // Construction/destruction
  public:
    TUrlLink(TWindow* parent, int id,
             LPCTSTR title,
             int x, int y, int w, int h,
             uint textLimit = 0,
             TModule* module = 0);

    TUrlLink(TWindow* parent, int id, const tstring& title, int x, int y, int w, int h, uint textLimit = 0, TModule* = 0);

    TUrlLink(TWindow* parent, int resourceId, 
             uint textLimit = 0, TModule* module = 0);
    virtual ~TUrlLink();

  public:
  /// \name Operations
  /// @{
    void     SetURL(LPCTSTR str);
    void SetURL(const tstring& s) {SetURL(s.c_str());}
    LPCTSTR GetURL() const;

    void      SetLinkColor(TColor& color);
    TColor  GetLinkColor() const;

    void     SetVisitedColor(TColor& color);
    TColor  GetVisitedColor() const;

    void     SetHoverColor(TColor& color);
    TColor  GetHoverColor() const;

    void     SetVisited(bool bVisited = true);
    bool     GetVisited() const;

    void     SetUnderline(bool bUnderline = true);
    bool     GetUnderline() const;
    /// @}

    /// \name Retrieves/assigns tooltip of/to window
    /// @{
    TTooltip* GetTooltip() const;
    void       EnableTooltip(bool enable=true);
    void      SetTooltip(TTooltip* tooltip);
    /// @}

  protected:
    /// \name Override TWindow virtual member functions
    /// @{
    virtual TGetClassNameReturnType GetClassName();
    bool    PreProcessMsg(MSG& msg);
    void    SetupWindow();
    void    GetWindowClass(WNDCLASS& wndClass);
    void    Paint(TDC& dc, bool erase, TRect& rect);
    /// @}

    /// \name Message Handlers
    /// @{
    void    EvPaint();
    void    EvLButtonDown(uint modKeys, const TPoint& point);
    void    EvMouseMove(uint modKeys, const TPoint& point);
    uint    EvGetDlgCode(const MSG* msg);
    void    EvSetFont(HFONT hFont, bool redraw);
    HFONT   EvGetFont();
    HBRUSH  EvCtlColor(HDC hDC, HWND hWndChild, uint ctlType);
    /// @}

    /// \name Implementation
    /// @{
    virtual void SetupCursor();
    virtual HINSTANCE GotoURL(LPCTSTR url, int showcmd);
    virtual void ReportError(int nError);
    /// @}

  public:
    void    PositionWindow();

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TUrlLink(const TUrlLink&);
    TUrlLink& operator =(const TUrlLink&);

  protected_data:
    TFont*    LinkFont;
    TTooltip*  Tooltip;
    TColor     LinkColor;    ///< Link color, defaults to blue
    TColor     VisitedColor; ///< Visited color, defaults to blue
    TColor     HoverColor;   ///< Hover color, defaults to blue
    tstring UrlAddress;
    bool      bUnderline;
    bool      bVisited;
    bool      bOverControl;

  DECLARE_RESPONSE_TABLE(TUrlLink);
  //DECLARE_STREAMABLE(_OWLCLASS, TUrlLink, 1);
  DECLARE_STREAMABLE_OWL(TUrlLink, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TUrlLink );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------
// inlines
inline TTooltip* TUrlLink::GetTooltip() const{
  return Tooltip;
}
/// Returns the caption of the control.
inline LPCTSTR TUrlLink::GetURL() const{
  return UrlAddress.c_str();
}
/// Returns true if the link should be displayed with underlined text; false
/// otherwise.
inline bool TUrlLink::GetUnderline() const{
  return bUnderline;
}
/// Returns true if the link has been used; false otherwise.
inline bool TUrlLink::GetVisited() const{
  return bVisited;
}
/// Gets the color used to indicate this control is a hyperlink.
inline TColor TUrlLink::GetLinkColor() const {
  return LinkColor;
}
/// Gets the color used to display the control after it has been used.
inline TColor TUrlLink::GetVisitedColor() const{
  return VisitedColor;
}
/// Gets the color used to display the control when the mouse cursor is over the
/// control.
inline TColor TUrlLink::GetHoverColor() const{
  return HoverColor;
}

} // OWL namespace

#endif  // !RC_INVOKED

#endif // !defined(OWL_URLLINK_H)
