//------------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// CALCEDIT.H
//
// Header file for TCalcEdit Class
//
// Original code by Steve Carr (Compuserve: 100251,1571)
//
//------------------------------------------------------------------------------
#if !defined(__OWLEXT_CALCEDIT_H)
#define __OWLEXT_CALCEDIT_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H) && !defined (__OWLEXT_ALL_H)
# include <owlext/core.h>
#endif

#include <owlext/popedit.h>

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

class OWLEXTCLASS TCalcEdit : public TPopupEdit {
  public:
    TCalcEdit(owl::TWindow* parent, int resourceId,
            owl::uint width, owl::uint decimals = 0, double minValue = 0, double maxValue = 0,
            owl::TModule* module = 0);
    TCalcEdit(owl::TWindow* parent, int id, double value, int x, int y,
            int w, int h, owl::uint width,
            owl::uint decimals= 0, double minValue = 0, double maxValue = 0,
            owl::TModule* module = 0);

    void Clicked();

    void SetMax( double d );
    void SetMin( double d );
    void operator = (double d);
    operator double();


    owl::uint GetDecimals() { return decimals; }

  private:
    owl::TDialog*  aCalc;
     owl::uint       decimals;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif  //__OWLEXT_CALCEDIT_H
