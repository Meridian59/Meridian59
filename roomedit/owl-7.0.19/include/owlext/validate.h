//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
//
//Original code by David Foringer
//
//-------------------------------------------------------------------
#if !defined (__OWLEXT_VALIDATE_H)
#define __OWLEXT_VALIDATE_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H) && !defined (__OWLEXT_ALL_H)
# include <owlext/core.h>
#endif

#include <owl/validate.h>

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

//
// class TDoubleValidator
//

class OWLEXTCLASS TDoubleValidator : public owl::TFilterValidator {
  public:
    TDoubleValidator(double minValue, double maxValue);

    // Override TValidator's virtuals
    //
    void Error(owl::TWindow* owner) override;
    bool IsValid(LPCTSTR str) override;
    bool IsValidInput(LPTSTR str, bool /*suppressFill*/) override;
    owl::uint Transfer(LPTSTR str, void* buffer, owl::TTransferDirection direction) override;
    int Adjust(owl::tstring& text, int& begPos, int& endPos, int amount) override;

  protected:
    double  GetMin()  {return Min;};
    void    SetMin(double minValue)  {Min=minValue;};
    double  GetMax()  {return Max;}
    void    SetMax(double maxValue)  {Max=maxValue;};

    double Min;
    double Max;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif  // __OWLEXT_VALIDATE_H

