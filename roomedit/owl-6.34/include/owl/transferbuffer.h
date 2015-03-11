//
/// \file transferbuffer.h
/// Safe transfer buffers
//
// Copyright © 2010 Vidar Hasfjord 
// Distributed under the OWLNext License (see http://owlnext.sourceforge.net).
//

#ifndef OWL_TRANSFERBUFFER_H
#define OWL_TRANSFERBUFFER_H

#include <owl/window.h>
#include <owl/static.h>
#include <owl/edit.h>
#include <owl/listbox.h>
#include <owl/combobox.h>
#include <owl/combobex.h>
#include <owl/checkbox.h>
#include <owl/radiobut.h>
#include <owl/scrollba.h>
#include <owl/slider.h>
#include <owl/checklst.h>
#include <owl/datetime.h>
#include <owl/hotkey.h>
#include <owl/ipaddres.h>
#include <owl/memcbox.h>
#include <owl/monthcal.h>

namespace owl {

//
/// Serves as the base class for safe transfer buffer windows.
/// The derived class is responsible for upholding a number of invariants.
/// See member AssignField for more information.
//
class _OWLCLASS TTransferBufferWindowBase
  : virtual public TWindow
{
public:

  //
  /// TWindow override
  /// Initiates the transfer of the children data to or from the transfer buffer.
  //
  virtual void TransferData(TTransferDirection direction);

  //
  /// TWindow override
  /// Dispatches to all the children.
  /// Checks that all participating children are bound to a field; throws if not.
  /// See TransferChild.
  //
  virtual uint Transfer(void* buffer, TTransferDirection direction);

  //
  /// Thrown if a a control tries to associate with more than one field.
  //
  class TXPolygamousControl: public TXOwl
  {public: TXPolygamousControl(TWindow& c);};

  //
  /// Thrown if a more than one control tries to associate with the same field.
  //
  class TXFieldConflict: public TXOwl
  {public: TXFieldConflict(TWindow& c);};

  //
  /// Thrown if a transfer is attempted with an unbound control.
  //
  class TXUnboundControl : public TXOwl
  {public: TXUnboundControl(TWindow& c);};

  //
  /// Thrown if a validator attempts to meddle with the transfer.
  //
  class TXMeddlingValidator : public TXOwl
  {public: TXMeddlingValidator(TEdit& c);};

protected:

  //
  /// Initializes the private implementation.
  /// This constructor is protected to not allow stand-alone use of the class.
  //
  TTransferBufferWindowBase();

  //
  /// Required for proper clean-up.
  //
  virtual ~TTransferBufferWindowBase();

  //
  /// Stores the field details and enables transfer for the given control.
  /// Used by derived class templates to create field associations.
  /// The window (control) and field types should be checked for consistency
  /// before calling this function. It is a precondition that the passed window
  /// can transfer data of the right type to the buffer field at the given offset.
  /// Consequently, it is a precondition that the window's transfer buffer is of a
  /// type that has such a field at the given offset. These invariants must be
  /// upheld by the derived class.
  //
  void AssignField(TWindow&, size_t offset, size_t size);

private:

  class TImpl; ///< Private implementation class
  TImpl* pimpl; ///< Private implementation pointer

  TTransferBufferWindowBase (const TTransferBufferWindowBase&); ///< Disabled
  TTransferBufferWindowBase& operator=(const TTransferBufferWindowBase&); ///< Disabled

};

//
/// Provides a customization hook for binding and creating controls.
/// Every control type should have a specialization of this template,
/// containing two function templates; 
///
/// \code
/// template <class TBuffer>
/// void Restrict(TFieldType TBuffer::*, TControlType&);
///
/// template <class TBuffer>
/// TControlType& Create(TFieldType TBuffer::*, TWindow*, int id, ...);
/// \endcode
///
/// These signatures restrict the binding of the control to a field in,
/// and only in, the transfer buffer specified for the window.
/// See TTransferBufferBinderImplementation for a generic implementation.
//
template <class TControlType>
class TTransferBufferBinder
{};

//
/// Provides the type-safe interface needed to set up a safe transfer buffer.
/// This class is responsible for assigning controls to fields in the transfer buffer
/// in a type-safe manner. This means upholding the invariants required by the base
/// class. See TTransferBufferWindowBase::AssignField for more information.
//
template <class TBuffer>
class TTransferBufferWindow
  : public TTransferBufferWindowBase
{
public:

  typedef TBuffer TTransferBuffer; ///< The template argument type

  //
  /// Hides the unsafe version in TWindow.
  //
  void SetTransferBuffer(TBuffer* b)
  {
    TWindow::SetTransferBuffer(b, sizeof(TBuffer));
  }

  //
  /// Maps the given control to the given field.
  //
  template <class TControlType, class TFieldType>
  TControlType& Bind(TFieldType TBuffer::* field, TControlType& c)
  {
    TTransferBufferBinder<TControlType>::Restrict(field, c);

    // Calculate the field offset.
    //
    // We cannot use the `offsetof` macro here, since it requires a name for it's second argument,
    // and we only have the member pointer to the field. We therefore calculate the offset in a
    // similar way to the common implementation of the `offsetof` macro. Note however that this
    // implementation has undefined behaviour since we dereference a null-pointer. In addition, it
    // may fail if the '&' address-operator is overloaded for TFieldType.
    //
    TBuffer* const p = 0; // dummy buffer pointer
    const size_t offset = reinterpret_cast<char*>(&(p->*field)) - reinterpret_cast<char*>(p);

    AssignField(c, offset, sizeof(TFieldType));
    return c;
  }

  //
  /// Creates a new control of the specified type by passing along the given arguments,
  /// then maps the given control to the given field.
  //
  //  TODO: Replace by variadic template (C++0x).
  //
  template <class TControlType, class TFieldType>
  TControlType& Bind(TFieldType TBuffer::* field, int id)
  {
    return Bind(field, TTransferBufferBinder<TControlType>::Create(field, this, id));
  }

  //
  /// Overload; forwards extra constructor arguments to Create.
  //
  template <class TControlType, class TFieldType, class TArg1>
  TControlType& Bind(TFieldType TBuffer::* field, int id, TArg1 a1)
  {
    return Bind(field, TTransferBufferBinder<TControlType>::Create(field, this, id, a1));
  }

  //
  /// Overload; forwards extra constructor arguments to Create.
  //
  template <class TControlType, class TFieldType, class TArg1, class TArg2>
  TControlType& Bind(TFieldType TBuffer::* field, int id, TArg1 a1, TArg2 a2)
  {
    return Bind(field, TTransferBufferBinder<TControlType>::Create(field, this, id, a1, a2));
  }

  //
  /// Overload; forwards extra constructor arguments to Create.
  //
  template <class TControlType, class TFieldType, class TArg1, class TArg2, class TArg3>
  TControlType& Bind(TFieldType TBuffer::* field, int id, TArg1 a1, TArg2 a2, TArg3 a3)
  {
    return Bind(field, TTransferBufferBinder<TControlType>::Create(field, this, id, a1, a2, a3));
  }

  //
  /// Overload; forwards extra constructor arguments to Create.
  //
  template <class TControlType, class TFieldType, class TArg1, class TArg2, class TArg3, class TArg4>
  TControlType& Bind(TFieldType TBuffer::* field, int id, TArg1 a1, TArg2 a2, TArg3 a3, TArg4 a4)
  {
    return Bind(field, TTransferBufferBinder<TControlType>::Create(field, this, id, a1, a2, a3, a4));
  }

  //
  /// Overload; forwards extra constructor arguments to Create.
  //
  template <class TControlType, class TFieldType, class TArg1, class TArg2, class TArg3, class TArg4, class TArg5>
  TControlType& Bind(TFieldType TBuffer::* field, int id, TArg1 a1, TArg2 a2, TArg3 a3, TArg4 a4, TArg5 a5)
  {
    return Bind(field, TTransferBufferBinder<TControlType>::Create(field, this, id, a1, a2, a3, a4, a5));
  }

  //
  /// Overload; forwards extra constructor arguments to Create.
  //
  template <class TControlType, class TFieldType, class TArg1, class TArg2, class TArg3, class TArg4, class TArg5, class TArg6>
  TControlType& Bind(TFieldType TBuffer::* field, int id, TArg1 a1, TArg2 a2, TArg3 a3, TArg4 a4, TArg5 a5, TArg6 a6)
  {
    return Bind(field, TTransferBufferBinder<TControlType>::Create(field, this, id, a1, a2, a3, a4, a5, a6));
  }

  //
  /// Overload; forwards extra constructor arguments to Create.
  //
  template <class TControlType, class TFieldType, class TArg1, class TArg2, class TArg3, class TArg4, class TArg5, class TArg6, class TArg7>
  TControlType& Bind(TFieldType TBuffer::* field, int id, TArg1 a1, TArg2 a2, TArg3 a3, TArg4 a4, TArg5 a5, TArg6 a6, TArg7 a7)
  {
    return Bind(field, TTransferBufferBinder<TControlType>::Create(field, this, id, a1, a2, a3, a4, a5, a6, a7));
  }

};

//
/// Helper; returns the type of a window as an tstring.
/// TODO: Move to a more suitable home where it is more convenient for reuse.
//
template <class T>
tstring GetFullTypeName(T* w)
{
  _USES_CONVERSION;
  return tstring(_A2W(_OBJ_FULLTYPENAME(w)));
}

//
/// Helper for char buffers; restricts the field type to a char array type of the right size.
/// Also configures the control with the right field type setting.
//
template <class TControlType, class TBuffer, uint N>
TControlType& RestrictCharArray(tchar (TBuffer::*)[N], TControlType& c)
{
  WARN(c.GetTextLimit() > 0 && c.GetTextLimit() != N, 
    _T("Corrected text limit and field size mismatch for control #")
    << c.GetId() << _T(" (") << GetFullTypeName(&c) << _T(") ")
    << _T("in window ") << GetFullTypeName(c.GetParent()) << _T("."));

  WARN(true, _T("Character array fields are deprecated for use with transfer buffers. ")
    << _T("Use a standard string class instead for control #")
    << c.GetId() << _T(" (") << GetFullTypeName(&c) << _T(") ")
    << _T("in window ") << GetFullTypeName(c.GetParent()) << _T("."));

  c.SetTextLimit(N);
  c.SetTransferBufferFieldType(TControlType::tbftCharArray);
  return c;
}

//
/// Generic binder implementation; restricts the field type for the given control type.
//
template <class TControlType, class TFieldType>
class TTransferBufferBinderImplementation
{
public:

  template <class TBuffer>
  static void Restrict(TFieldType TBuffer::*, TControlType&)
  {}

  //
  // TODO: Replace by variadic template (C++0x).
  //
  template <class TBuffer>
  static TControlType& Create(TFieldType TBuffer::*, TWindow* p, int id)
  {
    return *new TControlType(p, id);
  }

  template <class TBuffer, class TArg1>
  static TControlType& Create(TFieldType TBuffer::*, TWindow* p, int id, 
    TArg1 a1)
  {
    return *new TControlType(p, id, a1);
  }

  template <class TBuffer, class TArg1, class TArg2>
  static TControlType& Create(TFieldType TBuffer::*, TWindow* p, int id, 
    TArg1 a1, TArg2 a2)
  {
    return *new TControlType(p, id, a1, a2);
  }

  template <class TBuffer, class TArg1, class TArg2, class TArg3>
  static TControlType& Create(TFieldType TBuffer::*, TWindow* p, int id, 
    TArg1 a1, TArg2 a2, TArg3 a3)
  {
    return *new TControlType(p, id, a1, a2, a3);
  }

  template <class TBuffer, class TArg1, class TArg2, class TArg3, class TArg4>
  static TControlType& Create(TFieldType TBuffer::*, TWindow* p, int id, 
    TArg1 a1, TArg2 a2, TArg3 a3, TArg4 a4)
  {
    return *new TControlType(p, id, a1, a2, a3, a4);
  }

  template <class TBuffer, class TArg1, class TArg2, class TArg3, class TArg4, class TArg5>
  static TControlType& Create(TFieldType TBuffer::*, TWindow* p, int id, 
    TArg1 a1, TArg2 a2, TArg3 a3, TArg4 a4, TArg5 a5)
  {
    return *new TControlType(p, id, a1, a2, a3, a4, a5);
  }

  template <class TBuffer, class TArg1, class TArg2, class TArg3, class TArg4, class TArg5, class TArg6>
  static TControlType& Create(TFieldType TBuffer::*, TWindow* p, int id, 
    TArg1 a1, TArg2 a2, TArg3 a3, TArg4 a4, TArg5 a5, TArg6 a6)
  {
    return *new TControlType(p, id, a1, a2, a3, a4, a5, a6);
  }

  template <class TBuffer, class TArg1, class TArg2, class TArg3, class TArg4, class TArg5, class TArg6, class TArg7>
  static TControlType& Create(TFieldType TBuffer::*, TWindow* p, int id, 
    TArg1 a1, TArg2 a2, TArg3 a3, TArg4 a4, TArg5 a5, TArg6 a6, TArg7 a7)
  {
    return *new TControlType(p, id, a1, a2, a3, a4, a5, a6, a7);
  }

};

//
/// Binder for TStatic; restricts the possible field types to character array or tstring.
//
template <>
class TTransferBufferBinder<TStatic>
{
public:

  template <class TBuffer, uint N>
  static void Restrict(tchar (TBuffer::* field)[N], TStatic& c)
  {
    RestrictCharArray(field, c);
  }

  template <class TBuffer>
  static void Restrict(tstring TBuffer::* field, TStatic& c)
  {
    c.SetTransferBufferFieldType(TStatic::tbftString);
  }

  template <class TBuffer, uint N>
  static TStatic& Create(tchar (TBuffer::*)[N], TWindow* p, int id, 
    uint textLimit = 0, TModule* m = 0)
  {
    return *new TStatic(p, id, textLimit, m);
  }

  template <class TBuffer>
  static TStatic& Create(tstring TBuffer::*, TWindow* p, int id, 
    uint textLimit = 0, TModule* m = 0)
  {
    return *new TStatic(p, id, textLimit, m);
  }

};

//
/// Binder for TEdit; restricts the possible field types to character array or tstring.
//
template <>
class TTransferBufferBinder<TEdit>
{
public:

  template <class TBuffer, uint N>
  static void Restrict(tchar (TBuffer::* field)[N], TEdit& c)
  {
    RestrictCharArray(field, c);
  }

  template <class TBuffer>
  static void Restrict(tstring TBuffer::* field, TEdit& c)
  {
    c.SetTransferBufferFieldType(TStatic::tbftString);
  }

  template <class TBuffer, uint N>
  static TEdit& Create(tchar (TBuffer::*)[N], TWindow* p, int id, 
    uint textLimit = 0, TModule* m = 0)
  {
    return *new TEdit(p, id, textLimit, m);
  }

  template <class TBuffer>
  static TEdit& Create(tstring TBuffer::*, TWindow* p, int id, 
    uint textLimit = 0, TModule* m = 0)
  {
    return *new TEdit(p, id, textLimit, m);
  }

};

//
/// Binder for TListBox; restricts the field type to TListBoxData.
//
template <>
class TTransferBufferBinder<TListBox>
  : public TTransferBufferBinderImplementation<TListBox, TListBoxData>
{};

//
/// Binder for TComboBox; restricts the field type to TComboBoxData.
//
template <>
class TTransferBufferBinder<TComboBox>
  : public TTransferBufferBinderImplementation<TComboBox, TComboBoxData>
{};

//
/// Binder for TComboBoxEx; restricts the field type to TComboBoxExData.
//
template <>
class TTransferBufferBinder<TComboBoxEx>
  : public TTransferBufferBinderImplementation<TComboBoxEx, TComboBoxExData>
{};

//
/// Binder for TCheckBox; restricts the field type to WORD (uint16).
//
template <>
class TTransferBufferBinder<TCheckBox>
  : public TTransferBufferBinderImplementation<TCheckBox, WORD>
{};

//
/// Binder for TRadioButton; restricts the field type to WORD (uint16).
//
template <>
class TTransferBufferBinder<TRadioButton>
  : public TTransferBufferBinderImplementation<TRadioButton, WORD>
{};

//
/// Binder for TScrollBar; restricts the field type to TScrollBarData.
//
template <>
class TTransferBufferBinder<TScrollBar>
  : public TTransferBufferBinderImplementation<TScrollBar, TScrollBarData>
{};

//
/// Binder for TSlider; restricts the field type to TScrollBarData.
//
template <>
class TTransferBufferBinder<TSlider>
  : public TTransferBufferBinderImplementation<TSlider, TScrollBarData>
{};

//
/// Binder for TCheckList; restricts the field type to TCheckListData.
//
template <>
class TTransferBufferBinder<TCheckList>
  : public TTransferBufferBinderImplementation<TCheckList, TCheckListData>
{};

//
/// Binder for TDateTimePicker; restricts the field type to TDateTimePickerData.
//
template <>
class TTransferBufferBinder<TDateTimePicker>
  : public TTransferBufferBinderImplementation<TDateTimePicker, TDateTimePickerData>
{};

//
/// Binder for THotKey; restricts the field type to uint16.
//
template <>
class TTransferBufferBinder<THotKey> 
  : public TTransferBufferBinderImplementation<THotKey, uint16>
{};

//
/// Binder for TIPAddress; restricts the field type to TIPAddressBits.
//
template <>
class TTransferBufferBinder<TIPAddress> 
  : public TTransferBufferBinderImplementation<TIPAddress, TIPAddressBits>
{};

//
/// Binder for TMemComboBox; restricts the possible field types to character array, tstring or TComboBoxData.
//
template <>
class TTransferBufferBinder<TMemComboBox>
{
public:

  template <class TBuffer, uint N>
  static void Restrict(tchar (TBuffer::* field)[N], TMemComboBox& c)
  {
    RestrictCharArray(field, c);
  }

  template <class TBuffer>
  static void Restrict(tstring TBuffer::* field, TMemComboBox& c)
  {
    c.SetTransferBufferFieldType(TMemComboBox::tbftString);
  }

  template <class TBuffer>
  static void Restrict(TComboBoxData TBuffer::* field, TMemComboBox& c)
  {
    c.SetTransferBufferFieldType(TMemComboBox::tbftComboBoxData);
  }

  template <class TBuffer, uint N>
  static TMemComboBox& Create(tchar (TBuffer::*)[N], TWindow* p, int id, 
    const tstring& name, uint textLimit = 255, uint itemLimit = 25, TModule* m = 0)
  {
    return *new TMemComboBox(p, id, name, textLimit, itemLimit, m);
  }

  template <class TBuffer>
  static TMemComboBox& Create(tstring TBuffer::*, TWindow* p, int id, 
    const tstring& name, uint textLimit = 255, uint itemLimit = 25, TModule* m = 0)
  {
    return *new TMemComboBox(p, id, name, textLimit, itemLimit, m);
  }

  template <class TBuffer>
  static TMemComboBox& Create(TComboBoxData TBuffer::*, TWindow* p, int id, 
    const tstring& name, uint textLimit = 255, uint itemLimit = 25, TModule* m = 0)
  {
    return *new TMemComboBox(p, id, name, textLimit, itemLimit, m);
  }

};

//
/// Binder for TMonthCalendar; restricts the field type to TMonthCalendarData.
//
template <>
class TTransferBufferBinder<TMonthCalendar> 
  : public TTransferBufferBinderImplementation<TMonthCalendar, TMonthCalendarData>
{};

} // OWL namespace

#endif
