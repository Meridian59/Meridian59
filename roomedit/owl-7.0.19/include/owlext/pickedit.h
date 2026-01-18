//------------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// PICKEDIT.H
//
// Header file for TPickEditList Class
//
// Original code by Steve Carr (Compuserve: 100251,1571)
//
//------------------------------------------------------------------------------
#if !defined(__OWLEXT_PICKEDIT_H)
#define __OWLEXT_PICKEDIT_H

#include <owlext/popedit.h>

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

class OWLEXTCLASS TPickEditList : public TPopupEdit {
  public:
    TPickEditList(owl::TWindow* parent, int id, LPCTSTR fileName, LPCTSTR text,
                  int x, int y, int w, int h, owl::TResId bmpId,
                  owl::uint textLen = 0, owl::TModule* module = 0);
     TPickEditList(owl::TWindow* parent, int resourceId, LPCTSTR fileName,
                  owl::TResId bmpId, owl::uint textLen = 0, owl::TModule* module = 0);

    ~TPickEditList() ;

    void Clicked();

  protected:
    LPTSTR fileName;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif // __OWLEXT_PICKEDIT_H
