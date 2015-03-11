//
/// \file transfer.h
/// Utilities for transferring data in and out of controls
//
// Part of OWLNext - the next generation Object Windows Library 
// Copyright © 2010-2011 Vidar Hasfjord 
//
// For more information, including license details, see 
// http://owlnext.sourceforge.net
//

#ifndef OWL_TRANSFER_H
#define OWL_TRANSFER_H

#if defined(BI_COMP_BORLANDC) 
# pragma warn -inl // Disable warning "Functions containing 'statement' is not expanded inline".
#endif

#include <owl/dialog.h>
#include <utility>

#if _HAS_TR1
# include <functional>
# include <algorithm>
# include <type_traits>
#endif

namespace owl {

class _OWLCLASS TComboBoxData;
class _OWLCLASS TComboBoxExData;
class _OWLCLASS TCheckListData;
class _OWLCLASS TDateTimePickerData;
class _OWLCLASS TIPAddressBits;
class _OWLCLASS TListBoxData;
struct _OWLCLASS TMonthCalendarData;
struct _OWLCLASS TScrollBarData;
class _OWLCLASS TSystemTime;

//
/// \name Overloaded global functions 
/// Avoid hiding - we are adding overloads of these.
/// @{
//
using ::GetDlgItemText;
using ::SetDlgItemText;
using ::CheckDlgButton;

/// @}

//
/// \name Utility functions for control data transfer
/// @{

//
/// String overload
//
inline _OWLFUNC(tstring) GetDlgItemText(HWND ctrl)
{return TWindow(GetParent(ctrl)).GetDlgItemText(GetDlgCtrlID(ctrl));}

//
/// String overload
//
inline _OWLFUNC(void) SetDlgItemText(HWND ctrl, const tstring& text)
{::SetDlgItemText(GetParent(ctrl), GetDlgCtrlID(ctrl), text.c_str());}

//
/// Returns true if the given control has BST_CHECKED state.
//
inline _OWLFUNC(bool) IsChecked(HWND ctrl)
{return ::IsDlgButtonChecked(GetParent(ctrl), GetDlgCtrlID(ctrl)) == BST_CHECKED;}

//
/// Returns true if the given control has BST_UNCHECKED state.
//
inline _OWLFUNC(bool) IsUnchecked(HWND ctrl)
{return ::IsDlgButtonChecked(GetParent(ctrl), GetDlgCtrlID(ctrl)) == BST_UNCHECKED;}

//
/// Returns true if the given control has BST_INDETERMINATE state.
//
inline _OWLFUNC(bool) IsIndeterminate(HWND ctrl)
{return ::IsDlgButtonChecked(GetParent(ctrl), GetDlgCtrlID(ctrl)) == BST_INDETERMINATE;}

//
/// Sets the state of the given control to BST_CHECKED (or BST_UNCHECKED).
//
inline _OWLFUNC(void) CheckDlgButton(HWND ctrl, bool checked = true)
{::CheckDlgButton(GetParent(ctrl), ::GetDlgCtrlID(ctrl), checked ? BST_CHECKED : BST_UNCHECKED);}

//
/// Sets the state of the given control to BST_UNCHECKED (or BST_CHECKED).
//
inline _OWLFUNC(void) UncheckDlgButton(HWND ctrl, bool unchecked = true)
{::CheckDlgButton(GetParent(ctrl), ::GetDlgCtrlID(ctrl), unchecked ? BST_UNCHECKED : BST_CHECKED);}

//
/// Returns the zero-based index of the selected radiobutton in the group of controls starting
/// with the given control. The function searches controls until the next control with the
/// WS_GROUP is found.
//
_OWLFUNC(int) GetSelectedRadioButtonIndex(HWND firstCtrl);

//
/// Selects the control with the given zero-based index in the group of controls starting with
/// the given control. The search for the control ends when the next control with the WS_GROUP 
/// style is found, i.e. the function will not select a control beyond the first group.
//
_OWLFUNC(void) SetSelectedRadioButtonIndex(HWND firstCtrl, int selIndex);

/// @}

//
/// \name Utility functions for control data transfer - integer child ID overloads
/// @{

inline _OWLFUNC(tstring) GetDlgItemText(HWND parent, int ctrl)
{return TWindow(parent).GetDlgItemText(ctrl);}

inline _OWLFUNC(void) SetDlgItemText(HWND parent, int ctrl, const tstring& text)
{::SetDlgItemText(parent, ctrl, text.c_str());}

inline _OWLFUNC(bool) IsChecked(HWND parent, int ctrl)
{return IsChecked(GetDlgItem(parent, ctrl));}

inline _OWLFUNC(bool) IsUnchecked(HWND parent, int ctrl)
{return IsUnchecked(GetDlgItem(parent, ctrl));}

inline _OWLFUNC(bool) IsIndeterminate(HWND parent, int ctrl)
{return IsIndeterminate(GetDlgItem(parent, ctrl));}

inline _OWLFUNC(void) CheckDlgButton(HWND parent, int ctrl, int state)
{::CheckDlgButton(parent, ctrl, static_cast<UINT>(state));}

inline _OWLFUNC(void) CheckDlgButton(HWND parent, int ctrl, bool checked = true)
{CheckDlgButton(GetDlgItem(parent, ctrl), checked);}

inline _OWLFUNC(void) UncheckDlgButton(HWND parent, int ctrl, bool unchecked = true)
{UncheckDlgButton(GetDlgItem(parent, ctrl), unchecked);}

inline _OWLFUNC(int) GetSelectedRadioButtonIndex(HWND parent, int ctrl)
{return GetSelectedRadioButtonIndex(GetDlgItem(parent, ctrl));}

inline _OWLFUNC(void) SetSelectedRadioButtonIndex(HWND parent, int ctrl, int selIndex)
{SetSelectedRadioButtonIndex(GetDlgItem(parent, ctrl), selIndex);}

/// @}

//
/// Used to pass information to transfer functions.
//
struct TTransferInfo
{
  HWND Window;
  TTransferDirection Operation;
};

//
/// \name Transfer utility functions
/// @{

//
/// Transfers the state of the checkbox to the given bool variable.
//
_OWLFUNC(void) TransferCheckBoxData(const TTransferInfo&, HWND ctrl, bool& b);

//
/// Transfers the state of the checkbox to the given integer variable.
/// This overload supports tri-state check boxes (BST_CHECKED, BST_INDETERMINATE, BST_UNCHECKED).
//
_OWLFUNC(void) TransferCheckBoxData(const TTransferInfo&, HWND ctrl, UINT& state);

//
/// Transfers all the data for a check list box.
//
_OWLFUNC(void) TransferCheckListData(const TTransferInfo&, HWND ctrl, TCheckListData& data);

//
/// Transfers all the data for a combo box control.
//
_OWLFUNC(void) TransferComboBoxData(const TTransferInfo&, HWND ctrl, TComboBoxData& data);

//
/// Transfers the selection index of a combo box control.
//
_OWLFUNC(void) TransferComboBoxData(const TTransferInfo&, HWND ctrl, int& selIndex);

//
/// Transfers the selection string of a combo box control.
//
_OWLFUNC(void) TransferComboBoxData(const TTransferInfo&, HWND ctrl, tstring& selString, bool exact = false);

//
/// Transfers all the data for an extended combo box control.
//
_OWLFUNC(void) TransferComboBoxExData(const TTransferInfo&, HWND ctrl, TComboBoxExData& data);

//
/// Transfers all the data for a date and time picker control.
//
_OWLFUNC(void) TransferDateTimePickerData(const TTransferInfo&, HWND ctrl, TDateTimePickerData& data);

//
/// Transfers the selected date and time of a date and time picker control.
//
_OWLFUNC(void) TransferDateTimePickerData(const TTransferInfo&, HWND ctrl, TSystemTime& selTime);

//
/// Transfers the selected date and time of a date and time picker control as a string.
/// If the string is empty and the transfer direction is tdSetData, the control is set to no "no date", provided 
/// that the control has the DTS_SHOWNONE style. Otherwise the control is set to the default (current) date and time.
/// If the string is non-empty, the transfer direction is tdSetData, and parsing fails, a TXOwl exception is thrown.
//
_OWLFUNC(void) TransferDateTimePickerData(const TTransferInfo&, HWND ctrl, tstring& selTime);

//
/// Transfers the text contents of a control.
//
_OWLFUNC(void) TransferDlgItemText(const TTransferInfo&, HWND ctrl, tstring& text);

//
/// Contains stream formatting parameters for use with TransferDlgItemText.
//
struct TTransferFormat
{
  std::ios_base::fmtflags Flags; ///< See std::ios_base::flags.
  std::streamsize Precision; ///< See std::ios_base::precision.
  std::streamsize Width; ///< See std::ios_base::width.
  tchar Fill; // See std::basic_ios::fill.

  //
  /// Mutator; can be chained, e.g. TTransferFormat().SetFlags(std::ios::showbase).SetWidth(7);
  //
  TTransferFormat& SetFlags(std::ios_base::fmtflags v) {Flags = v; return *this;}

  //
  /// Mutator; can be chained, e.g. TTransferFormat().SetPrecision(2).SetFlags(std::ios::fixed);
  //
  TTransferFormat& SetPrecision(std::streamsize v) {Precision = v; return *this;}

  //
  /// Mutator; can be chained, e.g. TTransferFormat().SetWidth(7).SetFlags(std::ios::scientific);
  //
  TTransferFormat& SetWidth(std::streamsize v) {Width = v; return *this;}

  //
  /// Mutator; can be chained, e.g. TTransferFormat().SetFill(' ').SetFlags(0);
  //
  TTransferFormat& SetFill(tchar v) {Fill = v; return *this;}
};

//
/// Transfers the lexical conversion of the text contents of a control using the given format.
/// For example, numbers will be transferred as their representation as text.
//
template <class T>
void TransferDlgItemText(const TTransferInfo& i, HWND ctrl, T& value, const TTransferFormat& f)
{
  if (i.Operation == tdSetData)
  {
    tostringstream os;
    os.flags(f.Flags);
    if (f.Precision > 0) os.precision(f.Precision);
    if (f.Width > 0) os.width(f.Width);
    if (f.Fill > 0) os.fill (f.Fill);
    os << value;
    tstring s = os.str();
    TransferDlgItemText(i, ctrl, s);
  }
  else if (i.Operation == tdGetData)
  {
    tstring s;
    TransferDlgItemText(i, ctrl, s);
    tistringstream is(s);
    is.flags(f.Flags);
    is >> value;
  }
}

//
/// Overload - won't compile 
/// The sole purpose of this overload is to forbid pointers.
//
template <class T>
void TransferDlgItemText(const TTransferInfo&, HWND, T* value, const TTransferFormat&)
{int pointers_are_not_allowed = value;}

//
/// Transfers the lexical conversion of the text contents of a control.
/// For example, numbers will be transferred as their representation as text.
//
template <class T>
void TransferDlgItemText(const TTransferInfo& i, HWND ctrl, T& value)
{
  // Set precision to support loss-less conversion of IEEE 754 double-precision.
  //
  TransferDlgItemText(i, ctrl, value, TTransferFormat().SetPrecision(17));
}

//
/// Transfers the lexical conversion of the text contents of an edit control.
/// For example, numbers will be transferred as their representation as text.
//
template <class T>
void TransferEditData(const TTransferInfo& i, HWND ctrl, T& value)
{TransferDlgItemText(i, ctrl, value);}

//
/// Transfers the lexical conversion of the text contents of an edit control using the given format.
/// For example, numbers will be transferred as their representation as text.
//
template <class T>
void TransferEditData(const TTransferInfo& i, HWND ctrl, T& value, const TTransferFormat& f)
{TransferDlgItemText(i, ctrl, value, f);}

//
/// Transfers the hotkey value of a hotkey control.
//
_OWLFUNC(void) TransferHotKeyData(const TTransferInfo&, HWND ctrl, uint16& key);

//
/// Transfers the address of a IP address control as an object.
//
_OWLFUNC(void) TransferIPAddressData(const TTransferInfo&, HWND ctrl, TIPAddressBits& data);

//
/// Transfers the address for a IP address control as a 32-bit value.
//
_OWLFUNC(void) TransferIPAddressData(const TTransferInfo&, HWND ctrl, uint32& data);

//
/// Transfers all the data for a list box control.
//
_OWLFUNC(void) TransferListBoxData(const TTransferInfo&, HWND ctrl, TListBoxData& data);

//
/// Transfers the selection index of a list box control.
//
_OWLFUNC(void) TransferListBoxData(const TTransferInfo&, HWND ctrl, int& selIndex);

//
/// Transfers the selection string of a list box control.
//
_OWLFUNC(void) TransferListBoxData(const TTransferInfo&, HWND ctrl, tstring& selString, bool exact = false);

//
/// Transfers all the data for a calendar control.
//
_OWLFUNC(void) TransferMonthCalendarData(const TTransferInfo&, HWND ctrl, TMonthCalendarData& data);

//
/// Transfers the selected date and time of a calendar control.
//
_OWLFUNC(void) TransferMonthCalendarData(const TTransferInfo&, HWND ctrl, TSystemTime& curSel);

//
/// Transfers the selection range of a calendar control.
//
_OWLFUNC(void) TransferMonthCalendarData(const TTransferInfo&, HWND ctrl, std::pair<TSystemTime, TSystemTime>& rangeSel);

//
/// Transfers the index of the selected radio button within a group.
//
_OWLFUNC(void) TransferRadioButtonData(const TTransferInfo&, HWND ctrl, int& selIndex);

//
/// Transfers all the data for a scroll bar control.
//
_OWLFUNC(void) TransferScrollBarData(const TTransferInfo&, HWND ctrl, TScrollBarData& data);

//
/// Transfers the current position of a scroll bar control.
//
_OWLFUNC(void) TransferScrollBarData(const TTransferInfo&, HWND ctrl, int& position);

//
/// Transfers all the data for a slider control.
//
_OWLFUNC(void) TransferSliderData(const TTransferInfo&, HWND ctrl, TScrollBarData& data);

//
/// Transfers the current position of a slider control.
//
_OWLFUNC(void) TransferSliderData(const TTransferInfo&, HWND ctrl, int& position);

//
/// Transfers the lexical conversion of the text contents of a static control.
/// For example, numbers will be transferred as their representation as text.
//
template <class T>
void TransferStaticData(const TTransferInfo& i, HWND ctrl, T& value)
{TransferDlgItemText(i, ctrl, value);}

//
/// Transfers the lexical conversion of the text contents of a static control using the given format.
/// For example, numbers will be transferred as their representation as text.
//
template <class T>
void TransferStaticData(const TTransferInfo& i, HWND ctrl, T& value, const TTransferFormat& f)
{TransferDlgItemText(i, ctrl, value, f);}

/// @}

//
/// \name Transfer utility functions - integer child ID overloads
/// @{

//
/// Transfers the state of the checkbox to the given variable.
//
template <class T>
void TransferCheckBoxData(const TTransferInfo& i, int ctrl, T& value)
{TransferCheckBoxData(i, GetDlgItem(i.Window, ctrl), value);}

//
/// Transfers all the data for an a check list box.
//
inline _OWLFUNC(void) TransferCheckListData(const TTransferInfo& i, int ctrl, TCheckListData& data)
{TransferCheckListData(i, GetDlgItem(i.Window, ctrl), data);}

//
/// Transfers the data or selection index of a combo box control.
//
template <class T>
void TransferComboBoxData(const TTransferInfo& i, int ctrl, T& value)
{TransferComboBoxData(i, GetDlgItem(i.Window, ctrl), value);}

//
/// Transfers the selection string of a combo box control.
//
inline _OWLFUNC(void) TransferComboBoxData(const TTransferInfo& i, int ctrl, tstring& selString, bool exact = false)
{TransferComboBoxData(i, GetDlgItem(i.Window, ctrl), selString, exact);}

//
/// Transfers all the data for an extended combo box control.
//
inline _OWLFUNC(void) TransferComboBoxExData(const TTransferInfo& i, int ctrl, TComboBoxExData& data)
{TransferComboBoxExData(i, GetDlgItem(i.Window, ctrl), data);}

//
/// Transfers the data or selected date and time of a date and time picker.
//
template <class T>
void TransferDateTimePickerData(const TTransferInfo& i, int ctrl, T& value)
{TransferDateTimePickerData(i, GetDlgItem(i.Window, ctrl), value);}

//
/// Transfers the lexical conversion of the text contents of a control.
/// For example, numbers will be transferred as their representation as text.
//
template <class T>
void TransferDlgItemText(const TTransferInfo& i, int ctrl, T& value)
{TransferDlgItemText(i, GetDlgItem(i.Window, ctrl), value);}

//
/// Transfers the lexical conversion of the text contents of a control using the given format.
/// For example, numbers will be transferred as their representation as text.
//
template <class T>
void TransferDlgItemText(const TTransferInfo& i, int ctrl, T& value, const TTransferFormat& f)
{TransferDlgItemText(i, GetDlgItem(i.Window, ctrl), value, f);}

//
/// Transfers the lexical conversion of the text contents of an edit control.
/// For example, numbers will be transferred as their representation as text.
//
template <class T>
void TransferEditData(const TTransferInfo& i, int ctrl, T& value)
{TransferEditData(i, GetDlgItem(i.Window, ctrl), value);}

//
/// Transfers the lexical conversion of the text contents of an edit control using the given format.
/// For example, numbers will be transferred as their representation as text.
//
template <class T>
void TransferEditData(const TTransferInfo& i, int ctrl, T& value, const TTransferFormat& f)
{TransferEditData(i, GetDlgItem(i.Window, ctrl), value, f);}

//
/// Transfers the hotkey value of a hotkey control.
//
inline _OWLFUNC(void) TransferHotKeyData(const TTransferInfo& i, int ctrl, uint16& key)
{TransferHotKeyData(i, GetDlgItem(i.Window, ctrl), key);}

//
/// Transfers the address for a IP address control.
//
template <class T>
void TransferIPAddressData(const TTransferInfo& i, int ctrl, T& value)
{TransferIPAddressData(i, GetDlgItem(i.Window, ctrl), value);}

//
/// Transfers the data or selection index of a list box control.
//
template <class T> 
void TransferListBoxData(const TTransferInfo& i, int ctrl, T& value)
{TransferListBoxData(i, GetDlgItem(i.Window, ctrl), value);}

//
/// Transfers the selection string of a list box control.
//
inline _OWLFUNC(void) TransferListBoxData(const TTransferInfo& i, int ctrl, tstring& selString, bool exact = false)
{TransferListBoxData(i, GetDlgItem(i.Window, ctrl), selString, exact);}

//
/// Transfers the data or selected date and time of a calendar control.
//
template <class T>
void TransferMonthCalendarData(const TTransferInfo& i, int ctrl, T& value)
{TransferMonthCalendarData(i, GetDlgItem(i.Window, ctrl), value);}

//
/// Transfers the index of the selected radio button within a group.
//
inline _OWLFUNC(void) TransferRadioButtonData(const TTransferInfo& i, int ctrl, int& selIndex)
{TransferRadioButtonData(i, GetDlgItem(i.Window, ctrl), selIndex);}

//
/// Transfers the data or current position of a scroll bar control.
//
template <class T>
void TransferScrollBarData(const TTransferInfo& i, int ctrl, T& value)
{TransferScrollBarData(i, GetDlgItem(i.Window, ctrl), value);}

//
/// Transfers the data or current position of a slider control.
//
template <class T>
void TransferSliderData(const TTransferInfo& i, int ctrl, T& value)
{TransferSliderData(i, GetDlgItem(i.Window, ctrl), value);}

//
/// Transfers the lexical conversion of the text contents of a static control.
/// For example, numbers will be transferred as their representation as text.
//
template <class T>
void TransferStaticData(const TTransferInfo& i, int ctrl, T& value)
{TransferStaticData(i, GetDlgItem(i.Window, ctrl), value);}

//
/// Transfers the lexical conversion of the text contents of a static control using the given format.
/// For example, numbers will be transferred as their representation as text.
//
template <class T>
void TransferStaticData(const TTransferInfo& i, int ctrl, T& value, const TTransferFormat& f)
{TransferStaticData(i, GetDlgItem(i.Window, ctrl), value, f);}

/// @}

/// \name Transfer utility functions - DDX-like overloads
/// @{

//
/// Transfers the selection index of a combo box control.
//
inline _OWLFUNC(void) DDX_CBIndex(const TTransferInfo& i, int id, int& selIndex)
{TransferComboBoxData(i, id, selIndex);}

//
/// Transfers the selection string of a combo box control.
//
inline _OWLFUNC(void) DDX_CBString(const TTransferInfo& i, int id, tstring& selString)
{TransferComboBoxData(i, id, selString, false);}

//
/// Transfers the exact selection string of a combo box control.
//
inline _OWLFUNC(void) DDX_CBStringExact(const TTransferInfo& i, int id, tstring& selString)
{TransferComboBoxData(i, id, selString, true);}

//
/// Transfers the state of the checkbox to the given integer variable.
/// This overload supports tri-state check boxes (BST_CHECKED, BST_INDETERMINATE, BST_UNCHECKED).
//
inline _OWLFUNC(void) DDX_Check(const TTransferInfo& i, int id, UINT& state)
{TransferCheckBoxData(i, id, state);}

//
/// Transfers the selected date and time of a date and time picker control.
//
inline _OWLFUNC(void) DDX_DateTimeCtrl(const TTransferInfo& i, int id, TSystemTime& selTime)
{TransferDateTimePickerData(i, id, selTime);}

//
/// Transfers the selected date and time of a date and time picker control as a string.
//
inline _OWLFUNC(void) DDX_DateTimeCtrl(const TTransferInfo& i, int id, tstring& selTime)
{TransferDateTimePickerData(i, id, selTime);}

//
/// Transfers the hotkey value of a hotkey control.
//
inline _OWLFUNC(void) DDX_HotKey(const TTransferInfo& i, int id, uint16& key)
{TransferHotKeyData(i, id, key);}

//
/// Transfers the address for a IP address control as a 32-bit value.
//
inline _OWLFUNC(void) DDX_IPAddress(const TTransferInfo& i, int id, uint32& data)
{TransferIPAddressData(i, id, data);}

//
/// Transfers the selection index of a list box control.
//
inline _OWLFUNC(void) DDX_LBIndex(const TTransferInfo& i, int id, int& selIndex)
{TransferListBoxData(i, id, selIndex);}

//
/// Transfers the selection string of a list box control.
//
inline _OWLFUNC(void) DDX_LBString(const TTransferInfo& i, int id, tstring& selString)
{TransferListBoxData(i, id, selString, false);}

//
/// Transfers the exact selection string of a list box control.
//
inline _OWLFUNC(void) DDX_LBStringExact(const TTransferInfo& i, int id, tstring& selString)
{TransferListBoxData(i, id, selString, true);}

//
/// Transfers the selected date and time of a calendar control.
//
inline _OWLFUNC(void) DDX_MonthCalCtrl(const TTransferInfo& i, int id, TSystemTime& curSel)
{TransferMonthCalendarData(i, id, curSel);}

//
/// Transfers the index of the selected radio button within a group.
//
inline _OWLFUNC(void) DDX_Radio(const TTransferInfo& i, int id, int& selIndex)
{TransferRadioButtonData(i, id, selIndex);}

//
/// Transfers the current position of a scroll bar control.
//
inline _OWLFUNC(void) DDX_Scroll(const TTransferInfo& i, int id, int& position)
{TransferScrollBarData(i, id, position);}

//
/// Transfers all the data for a slider control.
//
inline _OWLFUNC(void) DDX_Slider(const TTransferInfo& i, int id, int& position)
{TransferSliderData(i, id, position);}

//
/// Transfers the lexical conversion of the text contents of a static control.
/// For example, numbers will be transferred as their representation as text.
//
template <class T>
void DDX_Text(const TTransferInfo& i, int id, T& value)
{TransferDlgItemText(i, id, value);}

/// @}

#if _HAS_TR1

namespace detail
{

  //
  // Implementation of indirect transfer
  //
  // This implementation is used by all the indirect Transfer* functions, passing the specific transfer 
  // function for the respective control as the last parameter. Rather than pass a function pointer as
  // the 'transfer' parameter, clients should pass a functor wrapper so that overload resolution is 
  // deferred until the actual call. This technique allows this function to serve as an indirect call to 
  // a whole set of overloaded transfer functions.
  //
  // The type of data to be transferred is deduced from the return type of the passed 'get' functor.
  //
  template <class TCtrl, class TGet, class TSet, class TTransfer>
  void Transfer(const TTransferInfo& i, const TCtrl& ctrl, TGet get, TSet set, TTransfer transfer)
  {
    typedef typename std::tr1::result_of<TGet()>::type TDataConstRef;
    typedef typename std::tr1::remove_reference<TDataConstRef>::type TDataConst;
    typedef typename std::tr1::remove_cv<TDataConst>::type TData;
    TData data(get()); // Always initialize; the transfer function may treat it as an in-out parameter.
    transfer(i, ctrl, data);
    if (i.Operation == tdGetData) set(data);
  }

  //
  // Implementation of indirect transfer, with one extra transfer parameter
  //
  // See the Transfer(const TTransferInfo&, const TCtrl&, TGet, TSet, TTransfer) for details.
  //
  template <class TCtrl, class TArg1, class TGet, class TSet, class TTransfer>
  void Transfer(const TTransferInfo& i, const TCtrl& ctrl, TArg1 arg1, TGet get, TSet set, TTransfer transfer)
  {
    typedef typename std::tr1::result_of<TGet()>::type TDataConstRef;
    typedef typename std::tr1::remove_reference<TDataConstRef>::type TDataConst;
    typedef typename std::tr1::remove_cv<TDataConst>::type TData;
    TData data(get()); // Always initialize; the transfer function may treat it as an in-out parameter.
    transfer(i, ctrl, data, arg1);
    if (i.Operation == tdGetData) set(data);
  }

  //
  // Implementation of indirect transfer using member functions
  //
  // See the Transfer(const TTransferInfo&, const TCtrl&, TGet, TSet, TTransfer) for details.
  //
  template <class TCtrl, class TDataSource, class R, class A, class TTransfer>
  void Transfer(const TTransferInfo& i, const TCtrl& ctrl, TDataSource* d, R (TDataSource::*get)() const, void (TDataSource::*set)(A), TTransfer transfer)
  {Transfer(i, ctrl, std::tr1::bind(get, d), std::tr1::bind(set, d, std::tr1::placeholders::_1), transfer);}

  //
  // Implementation of indirect transfer, with one extra transfer parameter, using member functions
  //
  // See the Transfer(const TTransferInfo&, const TCtrl&, TGet, TSet, TTransfer) for details.
  //
  template <class TCtrl, class TArg1, class TDataSource, class R, class A, class TTransfer>
  void Transfer(const TTransferInfo& i, const TCtrl& ctrl, TArg1 arg1, TDataSource* d, R (TDataSource::*get)() const, void (TDataSource::*set)(A), TTransfer transfer)
  {Transfer(i, ctrl, arg1, std::tr1::bind(get, d), std::tr1::bind(set, d, std::tr1::placeholders::_1), transfer);}

  //
  // Functor for deferred overload resolution
  //
  struct TTransferCheckBoxData
  {
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data)
    {TransferCheckBoxData(i, ctrl, data);}
  };

  //
  // Functor for deferred overload resolution
  //
  struct TTransferCheckListData
  {
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data)
    {TransferCheckListData(i, ctrl, data);}
  };

  //
  // Functor for deferred overload resolution
  //
  struct TTransferComboBoxData
  {
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data)
    {TransferComboBoxData(i, ctrl, data);}

    template <class TCtrl, class TData, class TArg1>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data, TArg1 arg1)
    {TransferComboBoxData(i, ctrl, data, arg1);}
  };

  //
  // Functor for deferred overload resolution
  //
  struct TTransferComboBoxExData
  {
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data)
    {TransferComboBoxExData(i, ctrl, data);}
  };

  //
  // Functor for deferred overload resolution
  //
  struct TTransferDateTimePickerData
  {
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data)
    {TransferDateTimePickerData(i, ctrl, data);}
  };

  //
  // Functor for deferred overload resolution
  //
  struct TTransferDlgItemText
  {
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data)
    {TransferDlgItemText(i, ctrl, data);}
  
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data, const TTransferFormat& f)
    {TransferDlgItemText(i, ctrl, data, f);}
  };

  //
  // Functor for deferred overload resolution
  //
  struct TTransferEditData
  {
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data)
    {TransferEditData(i, ctrl, data);}

    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data, const TTransferFormat& f)
    {TransferEditData(i, ctrl, data, f);}
  };

  //
  // Functor for deferred overload resolution
  //
  struct TTransferHotKeyData
  {
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data)
    {TransferHotKeyData(i, ctrl, data);}
  };

  //
  // Functor for deferred overload resolution
  //
  struct TTransferIPAddressData
  {
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data)
    {TransferIPAddressData(i, ctrl, data);}
  };

  //
  // Functor for deferred overload resolution
  //
  struct TTransferListBoxData
  {
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data)
    {TransferListBoxData(i, ctrl, data);}

    template <class TCtrl, class TData, class TArg1>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data, TArg1 arg1)
    {TransferListBoxData(i, ctrl, data, arg1);}
  };

  //
  // Functor for deferred overload resolution
  //
  struct TTransferMonthCalendarData
  {
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data)
    {TransferMonthCalendarData(i, ctrl, data);}
  };

  //
  // Functor for deferred overload resolution
  //
  struct TTransferRadioButtonData
  {
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data)
    {TransferRadioButtonData(i, ctrl, data);}
  };

  //
  // Functor for deferred overload resolution
  //
  struct TTransferScrollBarData
  {
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data)
    {TransferScrollBarData(i, ctrl, data);}
  };

  //
  // Functor for deferred overload resolution
  //
  struct TTransferSliderData
  {
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data)
    {TransferSliderData(i, ctrl, data);}
  };

  //
  // Functor for deferred overload resolution
  //
  struct TTransferStaticData
  {
    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data)
    {TransferStaticData(i, ctrl, data);}

    template <class TCtrl, class TData>
    void operator()(const TTransferInfo& i, const TCtrl& ctrl, TData& data, const TTransferFormat& f)
    {TransferStaticData(i, ctrl, data, f);}
  };

}

/// \name Transfer utility functions - indirect transfer via member functions
/// @{

//
/// Transfers the state of the checkbox using the given data source and member functions.
//
template <class TCtrl, class D, class R, class A>
void TransferCheckBoxData(const TTransferInfo& i, const TCtrl& ctrl, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, d, get, set, detail::TTransferCheckBoxData());}

//
/// Transfers data for a check list box using the given data source and member functions.
//
template <class TCtrl, class D, class R, class A>
void TransferCheckListData(const TTransferInfo& i, const TCtrl& ctrl, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, d, get, set, detail::TTransferCheckListData());}

//
/// Transfers data for a combo box using the given data source and member functions.
//
template <class TCtrl, class D, class R, class A>
void TransferComboBoxData(const TTransferInfo& i, const TCtrl& ctrl, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, d, get, set, detail::TTransferComboBoxData());}

//
/// Transfers data for a combo box using the given data source and member functions.
/// This overload supports an extra transfer function argument.
//
template <class TCtrl, class TArg1, class D, class R, class A>
void TransferComboBoxData(const TTransferInfo& i, const TCtrl& ctrl, TArg1 arg1, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, arg1, d, get, set, detail::TTransferComboBoxData());}

//
/// Transfers data for a date and time picker control using the given data source and member functions.
//
template <class TCtrl, class D, class R, class A>
void TransferDateTimePickerData(const TTransferInfo& i, const TCtrl& ctrl, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, d, get, set, detail::TTransferDateTimePickerData());}

//
/// Transfers the text contents of a control using the given data source and member functions.
//
template <class TCtrl, class D, class R, class A>
void TransferDlgItemText(const TTransferInfo& i, const TCtrl& ctrl, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, d, get, set, detail::TTransferDlgItemText());}

//
/// Transfers the text contents of a control using the given data source and member functions.
/// The given format is used to format the text when transferring data to the control.
//
template <class TCtrl, class D, class R, class A>
void TransferDlgItemText(const TTransferInfo& i, const TCtrl& ctrl, const TTransferFormat& f, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, f, d, get, set, detail::TTransferDlgItemText());}

//
/// Transfers data for a edit control using the given data source and member functions.
//
template <class TCtrl, class D, class R, class A>
void TransferEditData(const TTransferInfo& i, const TCtrl& ctrl, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, d, get, set, detail::TTransferEditData());}

//
/// Transfers data for a edit control using the given data source and member functions.
/// The given format is used to format the text when transferring data to the control.
//
template <class TCtrl, class D, class R, class A>
void TransferEditData(const TTransferInfo& i, const TCtrl& ctrl, const TTransferFormat& f, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, f, d, get, set, detail::TTransferEditData());}

//
/// Transfers data for a hotkey control using the given data source and member functions.
//
template <class TCtrl, class D, class R, class A>
void TransferHotKeyData(const TTransferInfo& i, const TCtrl& ctrl, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, d, get, set, detail::TTransferHotKeyData());}

//
/// Transfers data for an IP address control using the given data source and member functions.
//
template <class TCtrl, class D, class R, class A>
void TransferIPAddressData(const TTransferInfo& i, const TCtrl& ctrl, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, d, get, set, detail::TTransferIPAddressData());}

//
/// Transfers data for a list box using the given data source and member functions.
//
template <class TCtrl, class D, class R, class A>
void TransferListBoxData(const TTransferInfo& i, const TCtrl& ctrl, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, d, get, set, detail::TTransferListBoxData());}

//
/// Transfers data for a list box using the given data source and member functions.
/// This overload supports an extra transfer function argument.
//
template <class TCtrl, class TArg1, class D, class R, class A>
void TransferListBoxData(const TTransferInfo& i, const TCtrl& ctrl, TArg1 arg1, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, arg1, d, get, set, detail::TTransferListBoxData());}

//
/// Transfers data for a list box using the given data source and member functions.
//
template <class TCtrl, class D, class R, class A>
void TransferMonthCalendarData(const TTransferInfo& i, const TCtrl& ctrl, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, d, get, set, detail::TTransferMonthCalendarData());}

//
/// Transfers the index of the selected radio button within a group using the given data source and member functions.
//
template <class TCtrl, class D, class R, class A>
void TransferRadioButtonData(const TTransferInfo& i, const TCtrl& ctrl, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, d, get, set, detail::TTransferRadioButtonData());}

//
/// Transfers data for a scroll bar control using the given data source and member functions.
//
template <class TCtrl, class D, class R, class A>
void TransferScrollBarData(const TTransferInfo& i, const TCtrl& ctrl, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, d, get, set, detail::TTransferScrollBarData());}

//
/// Transfers data for a slider control using the given data source and member functions.
//
template <class TCtrl, class D, class R, class A>
void TransferSliderData(const TTransferInfo& i, const TCtrl& ctrl, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, d, get, set, detail::TTransferSliderData());}

//
/// Transfers data for a static control using the given data source and member functions.
//
template <class TCtrl, class D, class R, class A>
void TransferStaticData(const TTransferInfo& i, const TCtrl& ctrl, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, d, get, set, detail::TTransferStaticData());}

//
/// Transfers data for a static control using the given data source and member functions.
/// The given format is used to format the text when transferring data to the control.
//
template <class TCtrl, class D, class R, class A>
void TransferStaticData(const TTransferInfo& i, const TCtrl& ctrl, const TTransferFormat& f, D* d, R (D::*get)() const, void (D::*set)(A))
{detail::Transfer(i, ctrl, f, d, get, set, detail::TTransferStaticData());}

/// @}

/// \name Transfer utility functions - indirect transfer via functors
/// @{

//
/// Transfers the state of the checkbox using the given functors.
//
template <class TCtrl, class G, class S>
void TransferCheckBoxData(const TTransferInfo& i, const TCtrl& ctrl, G get, S set)
{detail::Transfer(i, ctrl, get, set, detail::TTransferCheckBoxData());}

//
/// Transfers data for a check list box using the given functors.
//
template <class TCtrl, class G, class S>
void TransferCheckListData(const TTransferInfo& i, const TCtrl& ctrl, G get, S set)
{detail::Transfer(i, ctrl, get, set, detail::TTransferCheckListData());}

//
/// Transfers data for a combo box using the given functors.
//
template <class TCtrl, class G, class S>
void TransferComboBoxData(const TTransferInfo& i, const TCtrl& ctrl, G get, S set)
{detail::Transfer(i, ctrl, get, set, detail::TTransferComboBoxData());}

//
/// Transfers data for a combo box using the given functors.
/// This overload supports an extra transfer function argument.
//
template <class TCtrl, class TArg1, class G, class S>
void TransferComboBoxData(const TTransferInfo& i, const TCtrl& ctrl, TArg1 arg1, G get, S set)
{detail::Transfer(i, ctrl, arg1, get, set, detail::TTransferComboBoxData());}

//
/// Transfers data for a date and time picker control using the given functors.
//
template <class TCtrl, class G, class S>
void TransferDateTimePickerData(const TTransferInfo& i, const TCtrl& ctrl, G get, S set)
{detail::Transfer(i, ctrl, get, set, detail::TTransferDateTimePickerData());}

//
/// Transfers the text contents of a control using the given functors.
//
template <class TCtrl, class G, class S>
void TransferDlgItemText(const TTransferInfo& i, const TCtrl& ctrl, G get, S set)
{detail::Transfer(i, ctrl, get, set, detail::TTransferDlgItemText());}

//
/// Transfers the text contents of a control using the given functors.
/// The given format is used to format the text when transferring data to the control.
//
template <class TCtrl, class G, class S>
void TransferDlgItemText(const TTransferInfo& i, const TCtrl& ctrl, const TTransferFormat& f, G get, S set)
{detail::Transfer(i, ctrl, f, get, set, detail::TTransferDlgItemText());}

//
/// Transfers data for a edit control using the given functors.
//
template <class TCtrl, class G, class S>
void TransferEditData(const TTransferInfo& i, const TCtrl& ctrl, G get, S set)
{detail::Transfer(i, ctrl, get, set, detail::TTransferEditData());}

//
/// Transfers data for a edit control using the given functors.
/// The given format is used to format the text when transferring data to the control.
//
template <class TCtrl, class G, class S>
void TransferEditData(const TTransferInfo& i, const TCtrl& ctrl, const TTransferFormat& f, G get, S set)
{detail::Transfer(i, ctrl, f, get, set, detail::TTransferEditData());}

//
/// Transfers data for a hotkey control using the given functors.
//
template <class TCtrl, class G, class S>
void TransferHotKeyData(const TTransferInfo& i, const TCtrl& ctrl, G get, S set)
{detail::Transfer(i, ctrl, get, set, detail::TTransferHotKeyData());}

//
/// Transfers data for an IP address control using the given functors.
//
template <class TCtrl, class G, class S>
void TransferIPAddressData(const TTransferInfo& i, const TCtrl& ctrl, G get, S set)
{detail::Transfer(i, ctrl, get, set, detail::TTransferIPAddressData());}

//
/// Transfers data for a list box using the given functors.
//
template <class TCtrl, class G, class S>
void TransferListBoxData(const TTransferInfo& i, const TCtrl& ctrl, G get, S set)
{detail::Transfer(i, ctrl, get, set, detail::TTransferListBoxData());}

//
/// Transfers data for a list box using the given functors.
/// This overload supports an extra transfer function argument.
//
template <class TCtrl, class TArg1, class G, class S>
void TransferListBoxData(const TTransferInfo& i, const TCtrl& ctrl, TArg1 arg1, G get, S set)
{detail::Transfer(i, ctrl, arg1, get, set, detail::TTransferListBoxData());}

//
/// Transfers data for a list box using the given functors.
//
template <class TCtrl, class G, class S>
void TransferMonthCalendarData(const TTransferInfo& i, const TCtrl& ctrl, G get, S set)
{detail::Transfer(i, ctrl, get, set, detail::TTransferMonthCalendarData());}

//
/// Transfers the index of the selected radio button within a group using the given functors.
//
template <class TCtrl, class G, class S>
void TransferRadioButtonData(const TTransferInfo& i, const TCtrl& ctrl, G get, S set)
{detail::Transfer(i, ctrl, get, set, detail::TTransferRadioButtonData());}

//
/// Transfers data for a scroll bar control using the given functors.
//
template <class TCtrl, class G, class S>
void TransferScrollBarData(const TTransferInfo& i, const TCtrl& ctrl, G get, S set)
{detail::Transfer(i, ctrl, get, set, detail::TTransferScrollBarData());}

//
/// Transfers data for a slider control using the given functors.
//
template <class TCtrl, class G, class S>
void TransferSliderData(const TTransferInfo& i, const TCtrl& ctrl, G get, S set)
{detail::Transfer(i, ctrl, get, set, detail::TTransferSliderData());}

//
/// Transfers data for a static control using the given functors.
//
template <class TCtrl, class G, class S>
void TransferStaticData(const TTransferInfo& i, const TCtrl& ctrl, G get, S set)
{detail::Transfer(i, ctrl, get, set, detail::TTransferStaticData());}

//
/// Transfers data for a static control using the given functors.
/// The given format is used to format the text when transferring data to the control.
//
template <class TCtrl, class G, class S>
void TransferStaticData(const TTransferInfo& i, const TCtrl& ctrl, const TTransferFormat& f, G get, S set)
{detail::Transfer(i, ctrl, f, get, set, detail::TTransferStaticData());}

/// @}

#endif

//
/// Mix-in class template providing support for data transfer to and from controls
//
template <class TDataSource>
class TTransferWindow 
  : virtual public TWindow
{
public:

  TTransferWindow()
    : DataSource(0)
  {}

  TTransferWindow(TDataSource& ds)
    : DataSource(&ds)
  {}

  //
  /// Transfer the state of the window into the given data source.
  //
  void GetData(TDataSource& ds) const
  {
    TTransferInfo i = {GetHandle(), tdGetData};
    const_cast<TTransferWindow*>(this)->DoTransferData(i, ds);
  }

  //
  /// Function style overload
  /// Requires that the data source type is a value type with a default constructor and
  /// a copy constructor. For types without a default constructor or copy constructor, 
  /// use GetData(TDataSource&) instead.
  //
  TDataSource GetData() const
  {
    TDataSource ds;
    GetData(ds);
    return ds;
  }

  //
  /// Transfers the state of the given data source into the window.
  //
  void SetData(const TDataSource& ds)
  {
    TTransferInfo i = {GetHandle(), tdSetData};
    DoTransferData(i, const_cast<TDataSource&>(ds));
  }

  //
  /// TWindow override
  /// Initialises the transfer info and dispatches to DoTransferData.
  //
  virtual void TransferData(TTransferDirection d)
  {
    if (!DataSource) return;
    TTransferInfo i = {GetHandle(), d};
    DoTransferData(i, *DataSource);
  }

  //
  /// \name Convenience functions for working with dialog items
  /// @{
  //

  //
  /// Enables (or disables) the given control.
  //
  void EnableDlgItem(int ctrl, bool enabled = true)
  {::EnableWindow(GetDlgItem(ctrl), enabled);}

  //
  /// Enables (or disables) all the controls in the given sequence.
  //
  void EnableDlgItem(const int* firstCtrl, const int* lastCtrl, bool enabled = true)
  {while (firstCtrl != lastCtrl) EnableDlgItem(*firstCtrl++, enabled);}

  //
  /// Enables (or disables) all the controls in the given array.
  //
  template <int N>
  void EnableDlgItem(const int (&id)[N], bool enabled = true)
  {EnableDlgItem(id, id + N, enabled);}  

  //
  /// Disables (or enables) the given control.
  //
  void DisableDlgItem(int ctrl, bool disabled = true)
  {EnableDlgItem(ctrl, !disabled);}

  //
  /// Disables (or enables) all the controls in the given sequence.
  //
  void DisableDlgItem(const int* firstCtrl, const int* lastCtrl, bool disabled = true)
  {EnableDlgItem(firstCtrl, lastCtrl, !disabled);}

  //
  /// Disables (or enables) all the controls in the given array.
  //
  template <int N>
  void DisableDlgItem(const int (&id)[N], bool disabled = true)
  {EnableDlgItem(id, !disabled);}  

  //
  /// Shows (or hides) the given control.
  //
  void ShowDlgItem(int ctrl, bool visible = true)
  {::ShowWindow(GetDlgItem(ctrl), visible ? SW_SHOW : SW_HIDE);}

  //
  /// Shows (or hides) all the controls in the given sequence.
  //
  void ShowDlgItem(const int* firstCtrl, const int* lastCtrl, bool visible = true)
  {while (firstCtrl != lastCtrl) ShowDlgItem(*firstCtrl++, visible);}

  //
  /// Shows (or hides) all the controls in the given array.
  //
  template <int N>
  void ShowDlgItem(const int (&id)[N], bool visible = true)
  {ShowDlgItem(id, id + N, visible);}  

  //
  /// Hides (or shows) the given control.
  //
  void HideDlgItem(int ctrl, bool hidden = true)
  {ShowDlgItem(ctrl, !hidden);}

  //
  /// Hides (or shows) all the controls in the given sequence.
  //
  bool HideDlgItem(const int* firstCtrl, const int* lastCtrl, bool hidden = true)
  {ShowDlgItem(firstCtrl, lastCtrl, !hidden);}

  //
  /// Hides (or shows) all the controls in the given array.
  //
  template <int N>
  void HideDlgItem(const int (&id)[N], bool hidden = true)
  {ShowDlgItem(id, !hidden);}  

  //
  /// Returns true if the given control has BST_CHECKED state.
  //
  bool IsChecked(int ctrl) const
  {return IsDlgButtonChecked(ctrl) == BST_CHECKED;}

  //
  /// Returns true if any control in the given sequence has BST_CHECKED state.
  //
  bool IsChecked(const int* firstCtrl, const int* lastCtrl) const
  {while (firstCtrl != lastCtrl) if (IsChecked(*firstCtrl++)) return true; return false;}

  //
  /// Returns true if any control in the given array has BST_CHECKED state.
  //
  template <int N>
  bool IsChecked(const int (&id)[N]) const
  {return IsChecked(id, id + N);}  

  //
  /// Returns true if the given control has BST_UNCHECKED state.
  //
  bool IsUnchecked(int ctrl) const
  {return IsDlgButtonChecked(ctrl) == BST_UNCHECKED;}

  //
  /// Returns true if any control in the given sequence has BST_UNCHECKED state.
  //
  bool IsUnchecked(const int* firstCtrl, const int* lastCtrl) const
  {while (firstCtrl != lastCtrl) if (IsUnchecked(*firstCtrl++)) return true; return false;}

  //
  /// Returns true if any control in the given array has BST_UNCHECKED state.
  //
  template <int N>
  bool IsUnchecked(const int (&id)[N]) const
  {return IsUnchecked(id, id + N);}  

  //
  /// Returns true if the given control has BST_INDETERMINATE state.
  //
  bool IsIndeterminate(int ctrl) const
  {return IsDlgButtonChecked(ctrl) == BST_INDETERMINATE;}

  //
  /// Returns true if any control in the given sequence has BST_INDETERMINATE state.
  //
  bool IsIndeterminate(const int* firstCtrl, const int* lastCtrl) const
  {while (firstCtrl != lastCtrl) if (IsIndeterminate(*firstCtrl++)) return true; return false;}

  //
  /// Returns true if any control in the given array has BST_INDETERMINATE state.
  //
  template <int N>
  bool IsIndeterminate(const int (&id)[N]) const
  {return IsIndeterminate(id, id + N);}  

  //
  /// using TWindow::CheckDlgButton;
  //
  void CheckDlgButton(int ctrl, uint state)
  {TWindow::CheckDlgButton(ctrl, state);}

  //
  /// Resolves an integer state argument.
  /// The button state macros; BST_CHECKED etc.; are of type int while the TWindow
  /// function takes an uint.
  //
  void CheckDlgButton(int ctrl, int state)
  {TWindow::CheckDlgButton(ctrl, static_cast<uint>(state));}

  //
  /// Sets the state of the given control to BST_CHECKED (or BST_UNCHECKED).
  //
  void CheckDlgButton(int ctrl, bool checked = true)
  {CheckDlgButton(ctrl, checked ? BST_CHECKED : BST_UNCHECKED);}

  //
  /// Sets the state of the given control to BST_UNCHECKED (or BST_CHECKED).
  //
  void UncheckDlgButton(int ctrl, bool unchecked = true)
  {CheckDlgButton(ctrl, unchecked ? BST_UNCHECKED : BST_CHECKED);}

  //
  /// See owl::GetSelectedRadioButtonIndex.
  //
  int GetSelectedRadioButtonIndex(HWND firstCtrl) const
  {return owl::GetSelectedRadioButtonIndex(firstCtrl);}

  //
  /// See owl::GetSelectedRadioButtonIndex.
  //
  int GetSelectedRadioButtonIndex(int firstCtrl) const
  {return GetSelectedRadioButtonIndex(GetDlgItem(firstCtrl));}

  //
  /// See owl::SetSelectedRadioButtonIndex.
  //
  void SetSelectedRadioButtonIndex(HWND firstCtrl, int selIndex)
  {owl::SetSelectedRadioButtonIndex(firstCtrl, selIndex);}

  //
  /// See owl::SetSelectedRadioButtonIndex.
  //
  void SetSelectedRadioButtonIndex(int firstCtrl, int selIndex)
  {SetSelectedRadioButtonIndex(GetDlgItem(firstCtrl), selIndex);}

  /// @}

protected:

  TDataSource* DataSource;

  //
  /// Transfers data between window and data source in the direction specified by the given transfer information.
  /// Default implementation; forwards the call to DoGetData or DoSetData, depending on the transfer direction.
  //
  virtual void DoTransferData(const TTransferInfo& i, TDataSource& ds)
  {
    if (i.Operation == tdGetData)
      DoGetData(i, ds);
    else if (i.Operation == tdSetData)
      DoSetData(i, ds);
  }

  //
  /// Transfers the state of the window into the given data source.
  /// Default implementation; does nothing.
  //
  virtual void DoGetData(const TTransferInfo&, TDataSource&) const
  {}

  //
  /// Transfers the state of the given data source into the window.
  /// Default implementation; does nothing.
  //
  virtual void DoSetData(const TTransferInfo&, const TDataSource&)
  {}

};

//
/// Dialog class derived from TTransferWindow
/// Provides convenient initialization of the dialog and transfer window bases.
//
template <class TDataSource, class TDialogBase = TDialog>
class TTransferDialog
  : virtual public TTransferWindow<TDataSource>,
    public TDialogBase
{
public:

  TTransferDialog(TWindow* parent, TResId id, TDataSource& ds, TModule* m = 0)
    : TDialogBase(parent, id, m)
  {DataSource = &ds;}

};

#if _HAS_TR1

//
/// TWindow mix-in
/// Delegates the job of transferring data to the given function object.
//
class _OWLCLASS TDelegatedTransferWindow
  : virtual public TWindow
{
public:

  typedef void TTransferFunctionSignature(const TTransferInfo&);
  typedef std::tr1::function<TTransferFunctionSignature> TTransferFunction;

  TDelegatedTransferWindow(TTransferFunction f);

  //
  /// TWindow override
  /// Dispatches to the transfer function.
  //
  virtual void TransferData(TTransferDirection d);

private:

  TTransferFunction TransferFunction;
};

//
/// Dialog derivative of TDelegatedTransferWindow
//
class _OWLCLASS TDelegatedTransferDialog
  : public TDialog,
    public TDelegatedTransferWindow
{
public:

  TDelegatedTransferDialog(TWindow* parent, TResId id, TTransferFunction f, TModule* module = 0);

};

//
/// Maps between application-specific value and a selection index, such as a radio-button group 
/// index or a list box selection.
//
template <class TValue>
class TSelectionIndexMapper
{
public:

  //
  /// Constructs a new mapper given an array of values, a getter and a setter.
  /// The order of the values passed determines the mapping of the selection indexes.
  //
  template <int N, class TGet, class TSet>
  TSelectionIndexMapper(const TValue (&values)[N], TGet get, TSet set)
    : Begin(&values[0]), End(&values[0] + N), Getter(get), Setter(set)
  {}

  //
  /// Overload; specialized for member functions
  //
  template <int N, class D, class R, class A>
  TSelectionIndexMapper(const TValue (&values)[N], D* d, R (D::*get)() const, void (D::*set)(A))
    : Begin(&values[0]), End(&values[0] + N), 
    Getter(std::tr1::bind(get, d)), 
    Setter(std::tr1::bind(set, d, std::tr1::placeholders::_1))
  {}

  // 
  /// Uses the stored getter to get the application-specific value, then converts it to a selection
  /// index. Returns N, the size of the value array, if the conversion can not be done.
  //
  int Get() const 
  {return static_cast<int>(std::find(Begin, End, Getter()) - Begin);}
  
  // 
  /// Converts the given selection index to an application-specific value, then calls the stored 
  /// setter, passing the value as an argument. Does nothing if the conversion can not be done.
  //
  void Set(int index) 
  {
    if (index < 0 || index > static_cast<int>(End - Begin)) return;
    Setter(Begin[index]);
  }

private:

  const TValue* Begin;
  const TValue* End;
  std::tr1::function<TValue()> Getter;
  std::tr1::function<void(TValue)> Setter;
};

#endif

} // OWL namespace

#if defined(BI_COMP_BORLANDC) 
# pragma warn .inl // Restore warning "Functions containing 'statement' is not expanded inline".
#endif

#endif
