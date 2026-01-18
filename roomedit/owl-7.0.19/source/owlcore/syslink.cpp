//
/// \file
/// Implementation of class TSysLink
//
// Part of OWLNext - the next generation Object Windows Library
// Copyright (c) 2014 Vidar Hasfjord
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//

#include <owl/pch.h>
#include <owl/syslink.h>
#include <shellapi.h>

namespace owl
{

//
/// Constructs a SysLink from arguments.
/// The provided text can include HTML anchor tags, and if so, these will be rendered as links.
//
TSysLink::TSysLink(
  TWindow* parent,
  int id,
  const tstring& markupText,
  const TRect& r,
  TModule* module
  )
  : TControl(parent, id, markupText, r.Left(), r.Right(), r.Width(), r.Height(), module)
{
  InitializeCommonControls(ICC_LINK_CLASS);
}

//
/// Constructs a SysLink from resource.
//
TSysLink::TSysLink(TWindow* parent, int resourceId, TModule* module)
  : TControl(parent, resourceId, module)
{
  InitializeCommonControls(ICC_LINK_CLASS);
}

//
/// Constructs an alias.
//
TSysLink::TSysLink(THandle hWnd, TModule* module)
  : TControl(hWnd, module)
{
  InitializeCommonControls(ICC_LINK_CLASS);
}

//
/// Retrieves the preferred height of the control for the control's current width.
/// Returns the preferred height of the control, in pixels.
//
auto TSysLink::GetIdealHeight() -> int const
{
  const auto r = SendMessage(LM_GETIDEALHEIGHT);
  return static_cast<int>(r); // preferred height
}

//
/// Retrieves the ideal size of the control for a given maximum width.
/// Returns the preferred height of the control, in pixels.
//
auto TSysLink::GetIdealSize(int maxWidth, TSize& out) -> int const
{
  const auto p1 = static_cast<TParam1>(maxWidth);
  const auto p2 = reinterpret_cast<TParam2>(&out);
  const auto r = SendMessage(LM_GETIDEALSIZE, p1, p2);
  return static_cast<int>(r); // preferred height
}

//
/// Functional-style overload.
//
auto TSysLink::GetIdealSize(int maxWidth) -> TSize const
{
  auto s = TSize{};
  [[maybe_unused]] const auto h = GetIdealSize(maxWidth, s);
  CHECK(h == s.Y());
  return s;
}

//
/// Retrieves the states and attributes of the specified anchor item.
/// Note that you need to set the LIF_ITEMINDEX flag in LITEM::mask and provide the index of the
/// requested item in LITEM::iLink before calling the function. You also need to set LITEM::mask
/// and LITEM::stateMask to include the states and attributes you want to query. See MSDN:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb760710.aspx
/// Returns `true` if successful.
//
auto TSysLink::GetItem(LITEM& inOut) -> bool const
{
  PRECONDITION(inOut.mask & LIF_ITEMINDEX);
  const auto p2 = reinterpret_cast<TParam2>(&inOut);
  const auto r = SendMessage(LM_GETITEM, 0, p2);
  return r != FALSE;
}

//
/// Functional-style overload.
/// Returns an LITEM with all available state and attribute information filled in.
//
auto TSysLink::GetItem(int index) -> LITEM const
{
  auto i = LITEM
  {
    0xFFFFFFFF, // mask
    index,
    0, // state
    0xFFFFFFFF // stateMask
  };
  const auto r = GetItem(i);
  if (!r) throw TXOwl(_T("TSysLink::GetItem failed"));
  return i;
}

//
/// Returns the ID attribute of the specified anchor item.
//
auto TSysLink::GetId(int index) -> tstring const
{
  _USES_CONVERSION_A;
  return _W2A_A(GetItem(index).szID);
}

//
/// Returns the HREF attribute of the specified anchor item.
//
auto TSysLink::GetUrl(int index) -> tstring const
{
  _USES_CONVERSION_A;
  return _W2A_A(GetItem(index).szUrl);
}

//
/// Sets the states and attributes of the specified anchor item.
/// Note that you need to set the LIF_ITEMINDEX flag in LITEM::mask and provide the index of the
/// requested item in LITEM::iLink before calling the function. You also need to set LITEM::mask
/// and LITEM::stateMask to include the states and attributes you want to modify. See MSDN:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb760710.aspx
/// Returns `true` if successful.
//
auto TSysLink::SetItem(const LITEM& i) -> bool
{
  PRECONDITION(i.mask & LIF_ITEMINDEX);
  const auto p2 = reinterpret_cast<TParam2>(&i);
  const auto r = SendMessage(LM_SETITEM, 0, p2);
  return r != FALSE;
}

//
/// Performs a hit test on this control and returns information about the link hit, if any.
/// Note that you need to set LHITTESTINFO::pt with the location of the hit-test before calling
/// the function, and the location must be set in client coordinates (not screen coordinates).
/// See MSDN:
/// http://msdn.microsoft.com/en-us/library/windows/desktop/bb760708.aspx
/// Returns `true` if a link was hit.
//
auto TSysLink::HitTest(LHITTESTINFO& inOut) -> bool const
{
  const auto p2 = reinterpret_cast<TParam2>(&inOut);
  const auto r = SendMessage(LM_HITTEST, 0, p2);
  return r != FALSE;
}

//
/// Performs a hit-test on the given location.
/// If an anchor item is hit, information about the item is returned in a unique_ptr. Otherwise a
/// null-pointer is returned.
//
// TODO: When std::optional is available, use it instead of unique_ptr here.
//
auto TSysLink::HitTest(const TPoint& p) -> std::unique_ptr<LITEM> const
{
  auto i = LHITTESTINFO{p};
  const auto r = HitTest(i);
  return r ? std::make_unique<LITEM>(i.item) : nullptr;
}

//
/// Returns the class name for a SysLink control.
//
auto TSysLink::GetWindowClassName() -> TWindowClassName
{
  PRECONDITION(WC_LINK == std::wstring{L"SysLink"});
  return TWindowClassName{_T("SysLink")};
}

//-------------------------------------------------------------------------------------------------

DEFINE_RESPONSE_TABLE1(TAutoSysLink, TSysLink)
  EV_NM_CLICK(UINT_MAX, EvFollowLink),
  EV_NM_RETURN(UINT_MAX, EvFollowLink),
END_RESPONSE_TABLE;

//
/// Constructs a SysLink from arguments.
/// The provided text can include HTML anchor tags, and if so, these will be rendered as links.
//
TAutoSysLink::TAutoSysLink(
  TWindow* parent,
  int id,
  const tstring& markupText,
  const TRect& r,
  TModule* module
  )
  : TSysLink(parent, id, markupText, r, module)
{}

//
/// Constructs a SysLink from resource.
//
TAutoSysLink::TAutoSysLink(TWindow* parent, int resourceId, TModule* module)
  : TSysLink(parent, resourceId, module)
{}

//
/// Constructs an alias.
//
TAutoSysLink::TAutoSysLink(THandle hWnd, TModule* module)
  : TSysLink(hWnd, module)
{}

void TAutoSysLink::EvFollowLink()
{
  auto a = reinterpret_cast<PNMLINK>(GetApplication()->GetCurrentEvent().Param2);
  CHECK(a);
  ShellExecuteW(nullptr, L"open", a->item.szUrl, nullptr, nullptr, SW_SHOW);
}

} // OWL namespace

