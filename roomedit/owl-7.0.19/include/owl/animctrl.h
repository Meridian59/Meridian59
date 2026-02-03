//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
//
/// \file
/// Definition of class TAnimateCtrl
//----------------------------------------------------------------------------

#if !defined(OWL_ANIMCTRL_H)
#define OWL_ANIMCTRL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/defs.h>
#include <owl/commctrl.h>


namespace owl {

#include <owl/preclass.h>

/// \addtogroup commctrl
/// @{

//
/// Encapsulates the Animation Control (a window that silently displays
/// an Audio Video Interleaved, AVI clip).
/// \note This control can only display uncompressed or RLE-compressed
///       AVI files.
//
class _OWLCLASS TAnimateCtrl : public TControl
{
  public:
    TAnimateCtrl(TWindow* parent, int id, int x, int y, int w, int h,
                 TModule* module = nullptr);
     TAnimateCtrl(TWindow* parent, int resourceId, TModule* module = nullptr);
   ~TAnimateCtrl();

    // Operations on the object
    //
     bool      Open(LPCTSTR fileOrRes, TModule* module=nullptr);
     bool Open(const tstring& fileOrRes, TModule* module = nullptr) {return Open(fileOrRes.c_str(), module);}
    bool      Play(uint16 start = 0, uint16 end = -1, uint repeat = 1);
    bool      Seek(uint16 frame);
    void      Stop();

  protected:
    virtual auto GetWindowClassName() -> TWindowClassName;
};

/// @}

#include <owl/posclass.h>


} // OWL namespace


#endif  // OWL_ANIMCTRL_H
