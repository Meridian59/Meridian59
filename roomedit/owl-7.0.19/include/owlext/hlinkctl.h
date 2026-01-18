// ****************************************************************************
// OWL Extensions (OWLEXT) Class Library
// Copyright (C) 1998 by Dieter Windau
// All rights reserved
//
// HLinkCtl.h:   header file
// Version:      1.0
// Date:         17.06.1998
// Author:       Dieter Windau
//
// THLinkCtrl is a freeware OWL class that supports Internet Hyperlinks from
// standard Windows applications just like they are displayed in Web browsers.
//
// You are free to use/modify this code but leave this header intact.
// May not be sold for profit.
//
// Tested with Borland C++ 5.02, OWL 5.02 under Windows NT 4.0 SP3 but I think
// the class would work with Windows 95 too.
// This file is provided "as is" with no expressed or implied warranty.
// Use at your own risk.
//
// This code is based on MFC class CHLinkCtrl by PJ Naughter
// Very special thanks to PJ Naughter:
//   EMail: pjn@indigo.ie
//   Web:   http://indigo.ie/~pjn
//
// Please send me bug reports, bug fixes, enhancements, requests, etc., and
// I'll try to keep this in next versions:
//   EMail: dieter.windau@usa.net
//   Web:   http://www.members.aol.com/SoftEngage
// ****************************************************************************
#if !defined(__OWLEXT_HLINKCTL_H)
#define __OWLEXT_HLINKCTL_H
#define __HLINKCTL_H

#include <owlext/defs.h>

#ifndef   OWL_EDIT_H
#include <owl/edit.h>
#endif
#ifndef   OWL_PROPSHT_H
#include <owl/propsht.h>
#endif

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

// ***************************** THLinkCtrl ***********************************

class OWLEXTCLASS THLinkCtrl: public owl::TEdit
{
public:
  //Constructors / Destructors
  THLinkCtrl(owl::TWindow* parent, int Id, LPCTSTR text, int x, int y, int w,
             int h, owl::uint textLimit = 0, bool multiline = false,
             owl::TModule* module = 0);
  THLinkCtrl(owl::TWindow* parent, int resourceId, owl::uint textLimit = 0,
             owl::TModule* module = 0);
  virtual ~THLinkCtrl();

  //Set or get the hyperlink to use
  void SetHyperLink(const owl::tstring& sActualLink);
  void SetHyperLink(const owl::tstring& sActualLink,
                    const owl::tstring& sLinkDescription,
                    bool bShrinkToFit=true);
  owl::tstring GetActualHyperLink() const { return m_sActualLink; };

  //Set or get the hyperlink description (really just the window text)
  void SetHyperLinkDescription(const owl::tstring& sDescription);
  owl::tstring GetHyperLinkDescription() const;

  //Set or get the hyperlink color
  void SetLinkColor(const owl::TColor& color);
  owl::TColor GetLinkColor() { return m_Color; };

  //Set or get the hyperlink color for visited links
  void SetVisitedLinkColor(const owl::TColor& color);
  owl::TColor GetVisitedLinkColor() { return m_VisitedColor; };

  //Set or get the hyperlink color for highlighted links
  void SetHighlightLinkColor(const owl::TColor& color);
  owl::TColor GetHighlightLinkColor() { return m_HighlightColor; };
  void SetUseHighlightColor(bool bUseHighlight) { m_bUseHighlight = bUseHighlight; };

  //Set or get whether the hyperlink should use an underlined font
  void SetUseUnderlinedFont(bool bUnderline);
  bool GetUseUnderlinedFont() { return m_bUnderline; };

  //Set or get whether the hyperlink should use a drop shadow
  void SetUseShadow(bool bUseShadow);
  bool GetUseShadow() { return m_bUseShadow; };

  //Set or get whether the hyperlink should show a popup menu on right button click
  void SetShowPopupMenu(bool bShowPopupMenu) { m_bShowPopupMenu = bShowPopupMenu; }
  bool GetShowPopupMenu() { return m_bShowPopupMenu; }

  //Gets whether the hyperlink has been visited
  bool GetVisited() { return m_State == ST_VISITED; };

  //Gets whether the window is automatically adjusted to
  //the size of the description text displayed
  bool GetShrinkToFit()  { return m_bShrinkToFit; }

  //Saves the hyperlink to an actual shortcut on file
#ifndef HLINK_NOOLE
  bool Save(const owl::tstring& sFilename) const;
#endif

  //Displays the properties dialog for this URL
  void ShowProperties() const;

  //Connects to the URL
  bool Open() const;

protected:
  enum State
  {
    ST_NOT_VISITED,
    ST_VISITED,
    ST_HIGHLIGHTED
  };

  void Init();

  bool EvSetCursor(THandle hWndCursor, owl::uint hitTest, owl::uint mouseMsg);
  void EvLButtonDown(owl::uint modKeys, const owl::TPoint& point);
  void EvMouseMove(UINT modKeys, const owl::TPoint& point);
  bool EvEraseBkgnd(HDC hdc);
  void EvPaint();
  void EvCopyShortcut();
  void EvProperties();
  void EvOpen();
  void EvSetFocus(HWND hWndLostFocus);
  void EvContextMenu(HWND childHwnd, int x, int y);

#ifndef HLINK_NOOLE
  void EvAddToFavorites();
  void EvAddToDesktop();
#endif

  void SetActualHyperLink(const owl::tstring& sActualLink);
  void ShrinkToFitEditBox();
  void HighLight(bool state);

#ifndef HLINK_NOOLE
  bool AddToSpecialFolder(int nFolder) const;
  bool OpenUsingCom() const;
#endif
  bool OpenUsingShellExecute() const;

  owl::tstring   m_sLinkDescription;
  owl::tstring   m_sActualLink;
  HCURSOR  m_hLinkCursor;
  HCURSOR  m_hArrowCursor;
  owl::TColor    m_Color;
  owl::TColor    m_VisitedColor;
  bool     m_bShrinkToFit;
  bool     m_bUseHighlight;
  owl::TColor    m_HighlightColor;
  State    m_State;
  bool     m_bUnderline;
  bool     m_bUseShadow;
  State    m_OldState;
  bool     m_bShowingContext;
  bool     m_bShowPopupMenu;

  DECLARE_RESPONSE_TABLE(THLinkCtrl);
};

class OWLEXTCLASS THLinkPage: public owl::TPropertyPage {
  public:
    THLinkPage(owl::TPropertySheet* parent);
    ~THLinkPage();
    void SetBuddy(const THLinkCtrl* pBuddy)
      { m_pBuddy = pBuddy; };

  protected:
    void SetupWindow();

  protected:
    const THLinkCtrl* m_pBuddy;
};

class OWLEXTCLASS THLinkSheet: public owl::TPropertySheet {
  public:
    THLinkSheet(owl::TWindow* parent);
    virtual ~THLinkSheet();
    void SetBuddy(const THLinkCtrl* pBuddy)
      { m_page1->SetBuddy(pBuddy); }

  protected:
    THLinkPage* m_page1;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif //__OWLEXT_HLINKCTL_H

