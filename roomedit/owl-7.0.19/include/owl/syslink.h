//
/// \file
/// Definition of class TSysLink
//
// Part of OWLNext - the next generation Object Windows Library
// Copyright (c) 2014 Vidar Hasfjord
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//

#ifndef OWL_SENTRY_SYSLINK_H
#define OWL_SENTRY_SYSLINK_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/commctrl.h>
#include <memory>

namespace owl
{

/// \addtogroup commctrl
/// @{

//
/// Encapsulates the SysLink common control, a Windows control that provides a convenient way to
/// embed hypertext links in a window.
///
/// The SysLink control is defined in version 6 of the Windows common controls, and it requires a
/// manifest or directive that specifies that version 6 of the DLL should be used if it is
/// available. For more information, see the Windows documentation:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb760704.aspx
//
class _OWLCLASS TSysLink
  : public TControl
{
public:

  TSysLink(TWindow* parent, int id, const tstring& markupText, const TRect&, TModule* module = nullptr);
  TSysLink(TWindow* parent, int resourceId, TModule* module = nullptr);
  explicit TSysLink(THandle hWnd, TModule* module = nullptr);

  TSysLink(const TSysLink&) = delete;
  TSysLink& operator =(const TSysLink&) = delete;

  auto GetIdealHeight() -> int const;
  auto GetIdealSize(int maxWidth, TSize& out) -> int const;
  auto GetIdealSize(int maxWidth) -> TSize const;
  auto GetItem(LITEM& inOut) -> bool const;
  auto GetItem(int index) -> LITEM const;
  auto GetId(int index) -> tstring const;
  auto GetUrl(int index) -> tstring const;

  auto SetItem(const LITEM&) -> bool;

  auto HitTest(LHITTESTINFO& inOut) -> bool const;
  auto HitTest(const TPoint&) -> std::unique_ptr<LITEM> const;

protected:

  // TWindow overrides
  //
  auto GetWindowClassName() -> TWindowClassName override;

};

//
/// Derives from TSysLink and provides automatic handling of link activation.
/// Links are followed using the ShellExecute function with the "open" command.
//
class _OWLCLASS TAutoSysLink
  : public TSysLink
{
public:

  TAutoSysLink(TWindow* parent, int id, const tstring& markupText, const TRect&, TModule* module = nullptr);
  TAutoSysLink(TWindow* parent, int resourceId, TModule* module = nullptr);
  explicit TAutoSysLink(THandle hWnd, TModule* module = nullptr);

private:

  // Event handlers
  //
  void EvFollowLink();

  DECLARE_RESPONSE_TABLE(TAutoSysLink);
};

/// @}

} // OWL namespace

#endif
