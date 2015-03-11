//------------------------------------------------------------------------------
// ObjectWindows 1998 by Yura Bidus
//
/// \file
/// Another implementation of Flat controls.
/// It use ideas from Joseph Parrello, Dieter Windau,and Kirk Stowell
///
/// Definition of classes TFlatComboBox,TFlatEdit, TFlatListBox
/// Definition of classes TGadgetComboBox, TGadgetEdit, TGadgetListBox
//------------------------------------------------------------------------------

#if !defined(OWL_FLATCTRL_H)
#define OWL_FLATCTRL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/combobox.h>
#include <owl/edit.h>
#include <owl/gadgetwi.h>
#include <owl/bitset.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup newctrl
/// @{

class _OWLCLASS TFlatPainter : public TBitFlags<uint>{
  public:
DECLARE_CASTABLE;
    void          SetFlatStyle(bool flat=true);
    bool          GetFlatStyle();

  protected:
    virtual bool IsFlat();
    void Paint(TDC& dc, TRect& rect);
    bool IdleAction(long idleCount);

    enum TState{
      fpFlatLook= 0x0001,
      fpMouseIn = 0x0002,
    };
};

class _OWLCLASS TFlatComboBox : public TComboBox, public TFlatPainter {
  public:
    TFlatComboBox(TWindow* parent, int id, int x, int y, int w, int h,
                  uint32 style, uint textLimit, TModule* module=0);
    TFlatComboBox(TWindow* parent, int resourceId, uint textLen = 0,
                  TModule* module = 0);

    virtual bool IdleAction(long idleCount);

  protected:
    virtual void SetupWindow();
    void EvPaint();

     DECLARE_RESPONSE_TABLE(TFlatComboBox);
};

//
/// For use with TGadgetWindow
//
class _OWLCLASS TGadgetComboBox : public TFlatComboBox{
  public:
    TGadgetComboBox(TWindow* parent, int id, int x, int y, int w, int h,
                  uint32 style, uint textLimit, TModule* module=0);
    TGadgetComboBox(TWindow* parent, int resourceId, uint textLen = 0,
                  TModule* module = 0);

  protected:
    virtual bool IsFlat();
};

//
//
//
class _OWLCLASS TFlatEdit : public TEdit, public TFlatPainter  {
  public:
    TFlatEdit(TWindow* parent, int id, LPCTSTR text, int x, int y, int w, int h,
              uint textLimit=0, bool multiline=false,TModule* module=0);

    TFlatEdit(
      TWindow* parent, 
      int id,
      const tstring& text, 
      int x, int y, int w, int h,
      uint textLimit = 0, 
      bool multiline = false,
      TModule* = 0);

    TFlatEdit(TWindow* parent, int resourceId, uint textLimit = 0,
              TModule* module = 0);

    virtual bool IdleAction(long idleCount);

  protected:
    virtual void SetupWindow();
    void EvPaint();

   DECLARE_RESPONSE_TABLE(TFlatEdit);
};

//
/// For use with TGadgetWindow
//
class _OWLCLASS TGadgetEdit : public TFlatEdit {
  public:
    TGadgetEdit(TWindow* parent, int id, LPCTSTR text, int x, int y,
              int w, int h,  uint textLimit=0, bool multiline=false,
              TModule* module=0);

    TGadgetEdit(
      TWindow* parent, 
      int id,
      const tstring& text, 
      int x, int y, int w, int h,
      uint textLimit = 0, 
      bool multiline = false,
      TModule* = 0);

    TGadgetEdit(TWindow* parent, int resourceId, uint textLimit = 0,
              TModule* module = 0);

  protected:
    virtual bool IsFlat();
};

//
/// \class TFlatListBox
// ~~~~~ ~~~~~~~~~~~~
//
class _OWLCLASS TFlatListBox : public TListBox, public TFlatPainter {
  public:
    TFlatListBox(TWindow* parent, int Id, int x, int y, int w, int h,
                 TModule* module = 0);
    TFlatListBox(TWindow* parent, int resourceId, TModule* module = 0);

    virtual bool IdleAction(long idleCount);

  protected:
    virtual void SetupWindow();
    void EvPaint();

   DECLARE_RESPONSE_TABLE(TFlatListBox);
};

//
/// For use with TGadgetWindow
//
class _OWLCLASS TGadgetListBox : public TFlatListBox {
  public:
    TGadgetListBox(TWindow* parent, int Id, int x, int y, int w, int h, TModule* module = 0);
    TGadgetListBox(TWindow* parent, int resourceId, TModule* module = 0);

  protected:
    virtual bool IsFlat();
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//
// Inline functions
//

//
// class TFlatPainter
// ~~~~~ ~~~~~~~~~~~~
//
inline bool 
TFlatPainter::GetFlatStyle()
{
  return IsFlat();
}

//
inline bool 
TFlatPainter::IsFlat()
{
  return IsSet(fpFlatLook);
}

//
// class TFlatComboBox
// ~~~~~ ~~~~~~~~~~~~~
//

//
inline
TFlatComboBox::TFlatComboBox(TWindow* parent, int id, int x, int y, int w, int h,
                             uint32 style, uint textLimit, TModule* module)
:
  TComboBox(parent, id, x, y, w, h, style, textLimit,module)
{
}

//
inline
TFlatComboBox::TFlatComboBox(TWindow* parent, int resourceId, uint textLen,
                             TModule* module)
:
  TComboBox(parent,resourceId, textLen,module)
{
}

//
inline bool
TFlatComboBox::IdleAction(long idleCount)
{
  TFlatPainter::IdleAction(idleCount);
  return TComboBox::IdleAction(idleCount);
}


//
// class TGadgetComboBox
// ~~~~~ ~~~~~~~~~~~~~~~
//

//
inline
TGadgetComboBox::TGadgetComboBox(TWindow* parent, int id, int x, int y, int w,
                          int h, uint32 style, uint textLimit, TModule* module)
:
  TFlatComboBox(parent, id, x, y, w, h,style, textLimit, module)
{
}

//
inline
TGadgetComboBox::TGadgetComboBox(TWindow* parent, int resourceId, uint textLen,
                                 TModule* module)
:
  TFlatComboBox(parent, resourceId, textLen, module)
{
}

//
// class TFlatEdit
// ~~~~~ ~~~~~~~~~
//

//
inline
TFlatEdit::TFlatEdit(TWindow* parent, int id, LPCTSTR text, int x, int y, int w,
                     int h, uint textLimit, bool multiline,TModule* module)
:
  TEdit(parent,id,text,x,y,w,h,textLimit,multiline,module)
{
}

inline
TFlatEdit::TFlatEdit(
  TWindow* parent, 
  int id, 
  const tstring& text, 
  int x, int y, int w, int h, 
  uint textLimit, 
  bool multiline, 
  TModule* module
  )
  : TEdit(parent, id, text, x, y, w, h, textLimit, multiline, module)
{}

//
inline
TFlatEdit::TFlatEdit(TWindow* parent, int resourceId, uint textLimit,
                     TModule* module)
:
  TEdit(parent, resourceId, textLimit, module)
{
}

//
inline bool
TFlatEdit::IdleAction(long idleCount)
{
  TFlatPainter::IdleAction(idleCount);
  return TEdit::IdleAction(idleCount);
}

//
// class TGadgetEdit
// ~~~~~ ~~~~~~~~~~~
//

//
inline
TGadgetEdit::TGadgetEdit(TWindow* parent, int id, LPCTSTR text, int x, int y,
              int w, int h,  uint textLimit, bool multiline, TModule* module)
:
  TFlatEdit(parent, id, text, x, y,w, h, textLimit, multiline, module)
{
}

inline
TGadgetEdit::TGadgetEdit(
  TWindow* parent, 
  int id, 
  const tstring& text, 
  int x, int y, int w, int h, 
  uint textLimit, 
  bool multiline, 
  TModule* module
  )
  : TFlatEdit(parent, id, text, x, y, w, h, textLimit, multiline, module)
{}

//
inline
TGadgetEdit::TGadgetEdit(TWindow* parent, int resourceId, uint textLimit,
                         TModule* module)
:
  TFlatEdit(parent, resourceId, textLimit,module)
{
}

//
// class TFlatListBox
// ~~~~~ ~~~~~~~~~~~~
//

//
inline
TFlatListBox::TFlatListBox(TWindow* parent, int Id, int x, int y, int w, int h,
                           TModule* module)
:
  TListBox(parent, Id, x, y, w, h, module)
{
}

//
inline
TFlatListBox::TFlatListBox(TWindow* parent, int resourceId, TModule* module)
:
  TListBox(parent, resourceId, module)
{
}

//
inline bool
TFlatListBox::IdleAction(long idleCount)
{
  TFlatPainter::IdleAction(idleCount);
  return TListBox::IdleAction(idleCount);
}

//
// class TGadgetListBox
// ~~~~~ ~~~~~~~~~~~~~~
//
inline
TGadgetListBox::TGadgetListBox(TWindow* parent, int Id, int x, int y, int w,
                               int h, TModule* module)
:
  TFlatListBox(parent, Id, x, y, w, h, module)
{
}

//
inline
TGadgetListBox::TGadgetListBox(TWindow* parent, int resourceId, TModule* module)
:
  TFlatListBox(parent, resourceId, module)
{
}

//
//
//
} // OWL namespace

#endif // OWL_FLATCTRL_H
