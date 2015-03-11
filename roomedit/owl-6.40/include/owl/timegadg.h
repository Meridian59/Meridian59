//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TTimeGadget
//----------------------------------------------------------------------------

#if !defined(OWL_TIMEGADG_H)
#define OWL_TIMEGADG_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/textgadg.h>
#include <owl/time.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TTimeGadget
// ~~~~~ ~~~~~~~~~~~
/// A gadget for displaying a time and/or date message.
//
class _OWLCLASS TTimeGadget : public TTextGadget {
  public:

/// Function prototype of callback invoked to retrieve the correct time.
    typedef void (*TGetTimeFunc)(tstring&);

    TTimeGadget(TGetTimeFunc timeFunc = &TTimeGadget::GetSystemTime,
                int id = 0,
                TBorderStyle = Recessed, 
                TAlign = Center,
                uint numChars = 12, 
                LPCTSTR text = 0,
                TFont* font = 0 /*new TGadgetWindowFont*/);

    TTimeGadget(
      TGetTimeFunc timeFunc,
      int id,
      TBorderStyle, 
      TAlign,
      uint numChars, 
      const tstring& text,
      TFont* font = 0);

    // Override from TGadget
    //
    void  Created();
    bool  IdleAction(long count);

    static void GetTTime(tstring&);
    static void GetSystemTime(tstring&);
    static void GetLocalTime(tstring&);

  private:
    TGetTimeFunc TimeFunc;
};

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>


} // OWL namespace


#endif  // OWL_TIMEGADG_H
