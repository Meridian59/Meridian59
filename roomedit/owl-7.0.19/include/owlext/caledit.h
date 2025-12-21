//------------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// CALEDIT.H
//
// Header file for TCalendarEdit Class
//
// Original code by Daniel BERMAN (dberman@inge-com.fr)
//
//------------------------------------------------------------------------------
#if !defined(__OWLEXT_CALEDIT_H)
#define __OWLEXT_CALEDIT_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H) && !defined (__OWLEXT_ALL_H)
# include <owlext/core.h>
#endif

#include <owl/validate.h>

#include <owl/date.h>
#include <owlext/popedit.h>


namespace OwlExt {

class OWLEXTCLASS TPopupCalWin;

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

class OWLEXTCLASS TCalendarEdit : public TPopupEdit {
  public:
    TCalendarEdit(owl::TWindow* parent, int id, LPCTSTR text, int x, int y,
                  int w, int h, owl::uint textLen = 0, owl::TModule* module = 0);
    TCalendarEdit(owl::TWindow* parent, int resourceId, owl::uint textLen = 0,
                  owl::TModule* module = 0);
    ~TCalendarEdit();

    void Clicked() override;
    const owl::TDate&   GetDate();
    void  SetDate(const owl::TDate& date);
    void  SetDate(LPCTSTR str);

  protected:
    TPopupCalWin* CalWin;

    void EvEnable(bool enabled) {TPopupEdit::EvEnable(enabled);}
    void EvChar(owl::uint key, owl::uint repeatCount, owl::uint flags);
    void EvKeyDown(owl::uint key, owl::uint repeatCount, owl::uint flags);
    void EvKillFocus(HWND hWndGetFocus);
    void EvSize(owl::uint sizeType, const owl::TSize& size) {TPopupEdit::EvSize(sizeType, size);}

  private:

    void Init();

    DECLARE_RESPONSE_TABLE(TCalendarEdit);
};


class OWLEXTCLASS TCalendarEditGadget : public TPopupEditGadget{
  public:
    TCalendarEditGadget(owl::TWindow* gadgetWnd, int id, int width = 70,
                        LPCTSTR text = 0);
};


class OWLEXTCLASS TDateValidator : public owl::TValidator{
  public:
    TDateValidator();

    virtual void Error(owl::TWindow* p=0);
    virtual bool IsValid(LPCTSTR str);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif // __OWLEXT_CALENDAREDIT_H
