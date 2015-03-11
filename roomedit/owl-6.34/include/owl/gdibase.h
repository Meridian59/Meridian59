//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of base most abstract GDI object class, and associated exception
/// class
//----------------------------------------------------------------------------

#if !defined(OWL_GDIBASE_H)
#define OWL_GDIBASE_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>
#include <owl/color.h>
#include <owl/except.h>



namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup except
/// @{
/// \class TXGdi
// ~~~~~ ~~~~~
/// Describes an exception resulting from GDI failures such as creating too many
/// TWindow device contexts (DCs). This exception occurs, for example, if a DC
/// driver cannot be located or if a device-independent bitmap (DIB) file cannot be
/// read.
/// 
/// The following code from the PAINT.CPP sample program on BC5.0x distribution disk
/// throws a TXGdi exception if a new DIB cannot be created.
/// \code
/// void TCanvas::NewDib(int width, int height, int nColors)
/// { 
///   TDib* dib;
///   try { 
///     dib = new TDib(width, height, nColors);
///   }
///   catch (TXGdi& x) { 
///     MessageBox("Could Not Create DIB", GetApplication()->Name,   MB_OK);
///     return;
/// }
/// \endcode
//
class _OWLCLASS TXGdi : public TXOwl {
  public:
    TXGdi(uint resId = IDS_GDIFAILURE, HANDLE handle = 0);

    static tstring Msg(uint resId, HANDLE);

    virtual TXGdi* Clone() const; // overide
    void Throw();

    static void Raise(uint resId = IDS_GDIFAILURE, HANDLE handle = 0);
};
/// @}

/// \addtogroup base
/// @{
//
/// Flag for Handle ctors to control Handle deletion in dtor
//
enum TAutoDelete { NoAutoDelete, AutoDelete };

//
/// \class TGdiBase
// ~~~~~ ~~~~~~~~
/// Root and abstract class for Windows object wrappers. Provides basic
/// notion of working with a handle, and constructing a C++ object with
/// an aliased handle.
//
class _OWLCLASS TGdiBase {
  protected:
    HANDLE      Handle;       ///< GDI handle of this object
    bool        ShouldDelete; ///< Should object delete GDI handle in dtor?

    // Throws exception if object is bad
    //
    void        CheckValid(uint resId = IDS_GDIFAILURE);
    static void _CheckValid(HANDLE handle, uint resId = IDS_GDIFAILURE);

    // Constructors for use by derived classes only
    //
    TGdiBase();
    TGdiBase(HANDLE handle, TAutoDelete autoDelete = NoAutoDelete);

  private:
    TGdiBase(const TGdiBase&); // Protect against copying & assignment
    TGdiBase& operator =(const TGdiBase&);

};

/// @}

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>


} // OWL namespace


#endif  // OWL_GDIBASE_H
