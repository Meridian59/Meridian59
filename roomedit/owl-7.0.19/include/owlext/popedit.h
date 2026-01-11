//------------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// POPEDIT.H
//
// Header file for TPopupEdit Class
//
// Original code by Daniel BERMAN (dberman@inge-com.fr)
//
//------------------------------------------------------------------------------
#if !defined(__OWLEXT_POPEDIT_H)
#define __OWLEXT_POPEDIT_H

#include <owlext/defs.h>

#include <owl/edit.h>
#include <owl/gdiobjec.h>
#include <owl/controlb.h>
#include <owl/controlg.h>

namespace OwlExt {

class TPopupButton;

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

class OWLEXTCLASS TPopupEdit : public owl::TEdit {
  public:

    TPopupEdit(owl::TWindow* parent, int id, LPCTSTR text, int x, int y,
               int w, int h, owl::TResId bmpId, owl::uint textLen = 0,
               owl::TModule* module = 0);
    TPopupEdit(owl::TWindow* parent, int resourceId, owl::TResId bmpId,
               owl::uint textLen = 0, owl::TModule* module = 0);

    virtual ~TPopupEdit();

    virtual void  Clicked() {};

    void           SysColorChange();

  protected:

    TPopupButton* PopupBtn;

    void SetupWindow() override;

    void  EvEnable(bool enabled);
    void  EvKeyDown(owl::uint key, owl::uint repeatCount, owl::uint flags);
    void  EvMouseMove(owl::uint modKeys, const owl::TPoint& point);
    void  EvShowWindow(bool show, owl::uint status);
    void  EvSize(owl::uint sizeType, const owl::TSize& size);
    void  EvSysKeyDown(owl::uint key, owl::uint repeatCount, owl::uint flags);

  private:

    owl::TFont* Font;

    void Init(owl::TResId bmpId);

    DECLARE_RESPONSE_TABLE(TPopupEdit);
};


class OWLEXTCLASS TToolBar : public owl::TControlBar {
  public:
    TToolBar(owl::TWindow* parent = 0, TTileDirection direction = Horizontal,
             owl::TFont* = nullptr,
             owl::TModule* module = 0);
  protected:
    bool IdleAction(long idleCount);
};


class OWLEXTCLASS TPopupEditGadget : public owl::TControlGadget{
  public:
    TPopupEditGadget(owl::TWindow& control, TBorderStyle = None);

  protected:
    void MouseEnter(owl::uint modKeys, const owl::TPoint& pt);
    void MouseLeave(owl::uint modKeys, const owl::TPoint& pt);
    void SysColorChange();
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

bool DrawMaskedBmp(HDC hDC, int x, int y, HBITMAP hbm, int XOrigin, int nWidth);


} // OwlExt namespace

#endif  // __OWLEXT_POPEDIT_H
