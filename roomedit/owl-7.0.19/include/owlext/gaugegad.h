//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// Provides TGaugeGadget, a gadget multiply-derived from TGauge and TGadget,
// for use in Gadget Windows.
//
// Original code:
// Copyright (c) 1997 Rich Goldstein, MD
// goldstei@interport.net
// May be used without permission, if appropriate credit is given
//
//-------------------------------------------------------------------
#if !defined(__OWLEXT_GAUGEGAD_H)
#define __OWLEXT_GAUGEGAD_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H)
# include <owlext/core.h>
#endif

// Grab other required OWL headers
//
#include <owl/gauge.h>
#include <owl/gadget.h>
#include <owl/uimetric.h>


namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>


class OWLEXTCLASS TGaugeGadget : public owl::TGadget, public owl::TGauge
{
  // Object lifetime methods
  //
public:
  TGaugeGadget(int id, LPCTSTR title, int width, TBorderStyle = None);
  ~TGaugeGadget();

protected:
  virtual void Paint(owl::TDC& dc, bool erase, owl::TRect& rect);
      // Overriden to change the font to the GagdetWindow font
  virtual void Paint(owl::TDC& dc)
    { owl::TGadget::Paint(dc); }

  virtual void Created();
  virtual void Inserted();
  virtual void Removed();

  virtual void InvalidateRect(const owl::TRect& rect, bool erase = true);
  virtual void Update();
      // Paint now if possible
  virtual void SetBounds(const owl::TRect& rect);

  virtual void GetDesiredSize(owl::TSize& size);
      // Computes the area excluding the borders and margins

  // Internal data
  //
private:
  int _width;
};


// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace


#endif // GSC_GAUGEGAD_H


