//
/// \file codepages.h
/// Utilities for accessing and setting code page information
//
// Part of OWLNext - the next generation Object Windows Library
// Copyright (c) 2014 by Joe Slater
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//

#if !defined(OWL_CODEPAGES_H)
#define OWL_CODEPAGES_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/defs.h>
#include <mbctype.h>
#include <vector>

#if defined(__WINCE__)
#define OWL_SET_OEMCP_(idCodePage) static_cast<bool>(::SetOEMCP(static_cast<UINT>(idCodePage)))
#define OWL_SET_ANSICP_(idCodePage) static_cast<bool>(::SetACP(static_cast<UINT>(idCodePage)))
#else
#define OWL_SET_OEMCP_(idCodePage) false
#define OWL_SET_ANSICP_(idCodePage) false
#endif

namespace owl
{

//
/// Encapsulates Windows code page functionality.
//
/// TCodePages is a static class and instantiation is not allowed.
//
/// The code page list does not get loaded until accessed using `GetCodePageList`
/// or `GetIndex`. Loading is performed only once and is persistent until the
/// application terminates.
//
/// The following sample code illustrates loading the code page list in a TComboBox
/// control, retrieving the current code page identifier, initializing the selection of
/// the control to the currently selected code page identifier, and changing the
/// current code page to the user selection.
//
/// \code
/// TComboBox* cbox = new TComboBox(/*...*/);
/// // ...
/// for (auto &cp : TCodePages::GetCodePageList())
///   cbox->AddString(cp.Name);
/// auto iCur = TCodePages::GetIndex(TCodePages::GetAnsiCodePageId());
/// if (iCur != -1) cbox->SetSelIndex(iCur);
/// // ...
/// auto iSel = cbox->GetSelIndex();
/// if (iSel != -1) TCodePages::SetAnsiCodePageId(TCodePages::GetCodePageList().at(iSel).CodePageId);
/// \endcode
//
class TCodePages
{

  public:
    //
    /// Define the code page, code page object, and container types.
    //
    using TCodePageId = uint;
    using TCodePage = struct { TCodePageId CodePageId; bool Installed; tstring Name; };
    using TCodePageList = std::vector<TCodePage>;

    //
    /// \name Code Page List Operations
    /// @{

    static auto GetCodePageList() -> const TCodePageList&;
    static auto GetIndex(TCodePageId idCodePage) -> int;

    /// @}

    //
    /// \name Code Page WINAPI Wrapper Operations
    /// @{

    //
    /// Retrieves the current OEM code page identifier.
    //
    /// Wrapper function for the Windows API `GetOEMCP`.
    //
    /// \return current OEM code page identifier.
    //
    /// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/dd318114.aspx
    //
    static auto GetOemCodePageId() -> TCodePageId
    { return static_cast<TCodePageId>(::GetOEMCP()); }

    //
    /// Sets the current OEM code page identifier.
    //
    /// Wrapper function for the Windows API `SetOEMCP`.
    //
    /// @param[in] idCodePage is the code page identifier to set.
    //
    /// \note Avaiable for WINCE only.
    //
    /// \return true if successful.
    //
    /// \sa http://msdn.microsoft.com/en-us/library/gg155426.aspx
    //
    static auto SetOemCodePageId(TCodePageId idCodePage) -> bool
    { return OWL_SET_OEMCP_(idCodePage); }

    //
    /// Retrieves the current ANSI code page identifier.
    //
    /// Wrapper function for the Windows API `GetACP`.
    //
    /// \return current ANSI code page identifier.
    //
    /// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/dd318070.aspx
    //
    static auto GetAnsiCodePageId() -> TCodePageId
    { return static_cast<TCodePageId>(::GetACP()); }

    //
    /// Sets the current ANSI code page identifier.
    //
    /// Wrapper function for the Windows API `SetACP`.
    //
    /// @param[in] idCodePage is the code page identifier to set.
    //
    /// \note Avaiable for WINCE only.
    //
    /// \return true if successful.
    //
    /// \sa http://msdn.microsoft.com/en-us/library/gg155975.aspx
    //
    static auto SetAnsiCodePageId(TCodePageId idCodePage) -> bool
    { return OWL_SET_ANSICP_(idCodePage); }

    //
    /// Retieves a CPINFOEX structure.
    //
    /// Wrapper function for the Windows API `GetCPInfoEx`.
    //
    /// @param[in] idCodePage is the code page identifier to use.
    //
    /// \return the CPINFOEX structure.
    //
    /// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/dd318081.aspx
    /// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/dd317781.aspx
    //
    static auto GetCodePageInfoEx(TCodePageId idCodePage) -> CPINFOEX
    {
      CPINFOEX cpInfoEx = { 0 };
      ::GetCPInfoEx(idCodePage, 0, &cpInfoEx);
      return cpInfoEx;
    }

    //
    /// Retieves a CPINFO structure.
    //
    /// Wrapper function for the Windows API `GetCPInfo`.
    //
    /// @param[in] idCodePage is the code page identifier to use.
    //
    /// \return the CPINFO structure.
    //
    /// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/dd318078.aspx
    /// \sa http://msdn.microsoft.com/en-us/library/windows/desktop/dd317780.aspx
    //
    static auto GetCodePageInfo(TCodePageId idCodePage) -> CPINFO
    {
      CPINFO cpInfo = { 0 };
      ::GetCPInfo(idCodePage, &cpInfo);
      return cpInfo;
    }

    //
    /// Retieves the multi-byte code page identifier.
    //
    /// Wrapper function for the Windows API `_getmbcp`.
    //
    /// \return the multi-byte code page identifier.
    //
    /// \sa http://msdn.microsoft.com/en-us/library/4115fwez.aspx
    //
    static auto GetMultiByteCodePage() -> TCodePageId
    { return static_cast<TCodePageId>(::_getmbcp()); }

    //
    /// Sets the multi-byte code page identifier.
    //
    /// Wrapper function for the Windows API `_setmbcp`.
    //
    /// @param[in] idCodePage is the code page identifier to set.
    //
    /// \return true if successful.
    //
    /// \sa http://msdn.microsoft.com/en-us/library/883tf19a.aspx
    //
    static auto SetMultiByteCodePage(TCodePageId idCodePage) -> bool
    { return !::_setmbcp(static_cast<int>(idCodePage)); }

    /// @}

  private:
    //
    /// \name Code Page Internal Operations
    /// @{

    //
    /// Internally implements the Meyers Singleton technique for building the code page list once, thread-safely.
    //
    static auto GetInstance() -> const TCodePages&
    {
      static TCodePages c;
      return c;
    }

    TCodePages();

    /// @}

}; // class TCodePages

} // owl namespace

// Release macro definitions; they need not persist.
//
#undef OWL_SET_OEMCP_
#undef OWL_SET_ANSICP_

#endif  // OWL_CODEPAGES_H
