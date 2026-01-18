//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Defines classes handling Windows resources
//----------------------------------------------------------------------------

#if !defined(OWL_RESOURCE_H)
# define OWL_RESOURCE_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/gdibase.h>
#include <owl/wsysinc.h>
#include <owl/wsyscls.h>


namespace owl {

#include <owl/preclass.h>

// Forward ref.
//
class _OWLCLASS TBitmap;
class _OWLCLASS TGadgetWindow;

//
// Compile-time constants for predefined resource types.
// Note: These values must correspond to the definitions
// in the Windows API.
//
enum
{
   RtDialog = 0x0005, // RT_DIALOG
   RtToolbar = 0x00F1, // RT_TOOLBAR
   RtNewDialog = 0x2005, // RT_NEWDIALOG
};

//
//
//
# if !defined(DLGTEMPLATEEX_DEFINED)
    struct  DLGTEMPLATEEX {
      ushort  version;
      ushort  signature;
      uint32  helpId;
      uint32  exStyle;
      uint32  style;
      ushort  itemCount;
      short   x;
      short   y;
      short   cx;
      short   cy;
      // wchar_t MenuName[];          // Name or ordinal
      // wchar_t ClassName[];         // Name or ordinal
      // wchar_t CaptionText[];
    };
#   define  DLGTEMPLATEEX_DEFINED
# endif

//
/// Structure describing the binary layout of toolbar resources
/// \note Toolbar resource is only available in 32-bit
//
# if !defined(TOOLBARTEMPLATE_DEFINED)
    struct TOOLBARTEMPLATE {
      ushort    version;          ///< Resource version
      ushort    width;            ///< Width of each bitmap glyph
      ushort    height;           ///< Height of each bitmap glyph
      ushort    count;            ///< Number of glyphs
      ushort    ids[1];           ///< Array of ids of glyphs+separators
    };
#   define TOOLBARTEMPLATE_DEFINED
# endif


//
/// \class TToolbarRes
// ~~~~~ ~~~~~~~~~~~
//
class _OWLCLASS TToolbarRes : public TResource<TOOLBARTEMPLATE, RtToolbar> {
  public:
    TToolbarRes(HINSTANCE hInst, TResId id, TAutoDelete del = AutoDelete);
   ~TToolbarRes();

    TBitmap&    GetBitmap();
    int         GetCount() const;
    ushort*     GetIds() const;
    int         GetWidth() const;
    int         GetHeight() const;

  protected:
    TBitmap*    ToolbarBitmap;
    bool        ShouldDelete;
    TOOLBARTEMPLATE& TBInfo() const;
};

//
/// \class TToolbarBldr
// ~~~~~ ~~~~~~~~~~~~
class _OWLCLASS TToolbarBldr {
  public:
    TToolbarBldr(TGadgetWindow& win, TResId id, HINSTANCE hinst = 0);
   ~TToolbarBldr();
    void            Build();

  protected:
    TGadgetWindow&  Win;
    TToolbarRes*    TbarRes;
    TResId          Id;
    HINSTANCE       Hinst;
};

//
/// \class TDialogRes
// ~~~~~ ~~~~~~~~~~
//
class _OWLCLASS TDialogRes {
  public:
    TDialogRes(HINSTANCE hInstance, TResId id);
    TDialogRes(HINSTANCE hInstance, TResId id, LANGID langid);
   ~TDialogRes();

    // Validate resource was found, loaded & locked
    //
    bool            IsOK() const;

    // Retrieve information about the underlying binary template
    //
    bool            IsDialogEx() const;
    DLGTEMPLATEEX*  GetTemplateEx() const;
    DLGTEMPLATE*    GetTemplate() const;
    DWORD           GetSize() const;

    // Querry about info. stored in template
    //
    void      GetRect(TRect& rect) const;

    //
    /// Enumeration of various strings stored with a dialog resource
    //
    enum TDlgResText {
      drtMenuName,              ///< String of menu resource used by dialog
      drtClassName,             ///< Class name of dialog
      drtCaption                ///< Caption of dialog
    };

    //
    // Retrieve a particular string from the resource
    //
    int       GetText(LPTSTR buffer, int size,
                      TDlgResText which = drtCaption) const;

  protected:
    TResource<DLGTEMPLATE, static_cast<int>(RtDialog)>*      OldResource;
    TResource<DLGTEMPLATEEX, static_cast<int>(RtNewDialog)>* NewResource;
};

#include <owl/posclass.h>


} // OWL namespace

#endif  //  OWL_RESOURCE_H

