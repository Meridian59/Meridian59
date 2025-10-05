//------------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright (c) 1998 by Yura Bidus
//
/// \file
/// TUrlLink static control. Will open the default browser with the given URL
/// when the user clicks on the link.
//
//------------------------------------------------------------------------------

#if !defined(OWLEXT_URLLINK_H)
#define OWLEXT_URLLINK_H

// Macro defining class name (usable in resource definition)
#if !defined(OWL_URLLINK)
#  if defined(UNICODE)
#    define OWL_URLLINK L"OWL_UrlLink"
#  else
#    define OWL_URLLINK "OWL_UrlLink"
#  endif
#endif

#if !defined(RC_INVOKED)

#include <owlext/core.h>
#include <owl/static.h>
#include <owl/color.h>

namespace owl {

  class _OWLCLASS TTooltip;

}

namespace OwlExt {

#include <owl/preclass.h>

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
class OWLEXTCLASS TUrlLink : public owl::TStatic
{
  // Construction/destruction
  public:
    TUrlLink(
      owl::TWindow* parent,
      int id,
      LPCTSTR title,
      int x, int y, int w, int h,
      owl::uint textLimit = 0,
      owl::TModule* module = 0);

    TUrlLink(owl::TWindow* parent, int id, const owl::tstring& title, int x, int y, int w, int h, owl::uint textLimit = 0, owl::TModule* = 0);
    TUrlLink(owl::TWindow* parent, int resourceId, owl::uint textLimit = 0, owl::TModule* module = 0);
    virtual ~TUrlLink();

  public:
  /// \name Operations
  /// @{
    void SetURL(LPCTSTR str);
    void SetURL(const owl::tstring& s) {SetURL(s.c_str());}
    LPCTSTR GetURL() const;

    void SetLinkColor(owl::TColor& color);
    owl::TColor GetLinkColor() const;

    void SetVisitedColor(owl::TColor& color);
    owl::TColor GetVisitedColor() const;

    void SetHoverColor(owl::TColor& color);
    owl::TColor GetHoverColor() const;

    void SetVisited(bool bVisited = true);
    bool GetVisited() const;

    void SetUnderline(bool bUnderline = true);
    bool GetUnderline() const;
    /// @}

    /// \name Retrieves/assigns tooltip of/to window
    /// @{
    auto GetTooltip() const -> owl::TTooltip* override;
    void EnableTooltip(bool enable = true) override;
    void SetTooltip(owl::TTooltip* tooltip);
    /// @}

  protected:
    /// \name Override TWindow virtual member functions
    /// @{
    auto GetWindowClassName() -> owl::TWindowClassName override;
    auto PreProcessMsg(MSG&) -> bool override;
    void SetupWindow() override;
    void GetWindowClass(WNDCLASS&) override;
    void Paint(owl::TDC&, bool erase, owl::TRect&) override;
    /// @}

    /// \name Message Handlers
    /// @{
    void EvPaint();
    void EvLButtonDown(owl::uint modKeys, const owl::TPoint& point);
    void EvMouseMove(owl::uint modKeys, const owl::TPoint& point);
    owl::uint EvGetDlgCode(const MSG* msg);
    void EvSetFont(HFONT hFont, bool redraw);
    HFONT EvGetFont();
    HBRUSH EvCtlColor(HDC hDC, HWND hWndChild, owl::uint ctlType);
    /// @}

    /// \name Implementation
    /// @{
    virtual void SetupCursor();
    virtual HINSTANCE GotoURL(LPCTSTR url, int showcmd);
    virtual void ReportError(int nError);
    /// @}

  public:
    void PositionWindow();

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TUrlLink(const TUrlLink&);
    TUrlLink& operator =(const TUrlLink&);

  protected_data:
    owl::TFont* LinkFont;
    owl::TTooltip* Tooltip;
    owl::TColor LinkColor;    ///< Link color, defaults to blue
    owl::TColor VisitedColor; ///< Visited color, defaults to blue
    owl::TColor HoverColor;   ///< Hover color, defaults to blue
    owl::tstring UrlAddress;
    bool bUnderline;
    bool bVisited;
    bool bOverControl;

  DECLARE_RESPONSE_TABLE(TUrlLink);
};

#include <owl/posclass.h>

//----------------------------------------
// Inlines

inline owl::TTooltip* TUrlLink::GetTooltip() const{
  return Tooltip;
}

//
/// Returns the caption of the control.
//
inline LPCTSTR TUrlLink::GetURL() const{
  return UrlAddress.c_str();
}

//
/// Returns true if the link should be displayed with underlined text; false otherwise.
//
inline bool TUrlLink::GetUnderline() const{
  return bUnderline;
}

//
/// Returns true if the link has been used; false otherwise.
//
inline bool TUrlLink::GetVisited() const{
  return bVisited;
}

//
/// Gets the color used to indicate this control is a hyperlink.
//
inline owl::TColor TUrlLink::GetLinkColor() const {
  return LinkColor;
}

//
/// Gets the color used to display the control after it has been used.
//
inline owl::TColor TUrlLink::GetVisitedColor() const{
  return VisitedColor;
}

//
/// Gets the color used to display the control when the mouse cursor is over the control.
//
inline owl::TColor TUrlLink::GetHoverColor() const{
  return HoverColor;
}

} // OwlExt namespace

#endif  // !RC_INVOKED

#endif // !defined(OWL_URLLINK_H)
