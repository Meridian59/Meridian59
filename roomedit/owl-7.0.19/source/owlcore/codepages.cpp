//
/// \file codepages.cpp
/// Utilities for accessing and setting code page information
//
// Part of OWLNext - the next generation Object Windows Library
// Copyright (c) 2014 by Joe Slater
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//

#include <owl/pch.h>

#include <owl/codepages.h>
#include <algorithm>

namespace owl
{

//
// Hide the internal helper functions and variables so they are excluded from
// public view by using an anonymous namespace.
//
namespace
{

  //
  // Container for the code page list.
  //
  TCodePages::TCodePageList CodePageList_;

  //
  // Uses the binary search std routines to find the given code page identifier.
  //
  auto FindCodePage_(TCodePages::TCodePageId idCodePage) -> TCodePages::TCodePageList::iterator
  {
    return std::lower_bound(begin(CodePageList_), end(CodePageList_), TCodePages::TCodePage{idCodePage, false, tstring()});
  }

} // anonymous namespace

//
/// Contructs the code page list thread-safely, used only by GetInstance.
//
// Uses ::EnumSystemCodePages to query supported and installed code pages.
//
// Reference:
//   http://msdn.microsoft.com/en-us/library/windows/desktop/dd317825.aspx
//   http://msdn.microsoft.com/en-us/library/windows/desktop/dd317809.aspx
//
TCodePages::TCodePages()
{
  struct TLocal
  {

    static auto CALLBACK AddSupported(LPTSTR codePageString) -> BOOL
    {
      auto idCodePage = static_cast<const TCodePages::TCodePageId>(_ttol(codePageString));
      auto s = tstring{TCodePages::GetCodePageInfoEx(idCodePage).CodePageName};
      if (s.empty()) s = to_tstring(idCodePage); // Use code page value for empty names
      CodePageList_.push_back({idCodePage, false, s});
      return TRUE;
    }

    static auto CALLBACK InitInstalled(LPTSTR codePageString) -> BOOL
    {
      auto idCodePage = static_cast<const TCodePages::TCodePageId>(_ttol(codePageString));
      auto found = FindCodePage_(idCodePage);
      if (found != end(CodePageList_)) found->Installed = true;
      return TRUE;
    }

  };

  // Build the list of supported code pages sorted.
  //
  ::EnumSystemCodePages(&TLocal::AddSupported, CP_SUPPORTED);
  sort(begin(CodePageList_), end(CodePageList_));

  // Update the list of supported code pages to indicate whether installed.
  //
  ::EnumSystemCodePages(&TLocal::InitInstalled, CP_INSTALLED);
}

//
/// Searches the code page list for a code page identifier.
//
/// @param[in] idCodePage is the code page identifier to find.
//
/// \return the index of the code page list for the identifier or -1 if not found.
//
auto TCodePages::GetIndex(TCodePageId idCodePage) -> int
{
  GetInstance();
  auto found = FindCodePage_(idCodePage);
  return (found != end(CodePageList_)) ? static_cast<int>(distance(begin(CodePageList_), found)) : -1;
}

//
/// Retrieves the code page list.
//
/// \return the code page list.
//
auto TCodePages::GetCodePageList() -> const TCodePageList&
{
  GetInstance();
  return CodePageList_;
}

//
// Define comparison operator overloads needed for std sorting and searching (sorts by code page identifier)
//
auto operator==(const TCodePages::TCodePage& lhs, const TCodePages::TCodePage& rhs) -> bool
{ return lhs.CodePageId == rhs.CodePageId; }

auto operator!=(const TCodePages::TCodePage& lhs, const TCodePages::TCodePage& rhs) -> bool
{ return lhs.CodePageId != rhs.CodePageId; }

auto operator<=(const TCodePages::TCodePage& lhs, const TCodePages::TCodePage& rhs) -> bool
{ return lhs.CodePageId <= rhs.CodePageId; }

auto operator>=(const TCodePages::TCodePage& lhs, const TCodePages::TCodePage& rhs) -> bool
{ return lhs.CodePageId >= rhs.CodePageId; }

auto operator<(const TCodePages::TCodePage& lhs, const TCodePages::TCodePage& rhs) -> bool
{ return lhs.CodePageId < rhs.CodePageId; }

auto operator>(const TCodePages::TCodePage& lhs, const TCodePages::TCodePage& rhs) -> bool
{ return lhs.CodePageId > rhs.CodePageId; }

} // owl namespace
