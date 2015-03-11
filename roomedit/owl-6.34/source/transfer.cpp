//
/// \file transfer.cpp
/// Utilities for transferring data in and out of controls
//
// Part of OWLNext - the next generation Object Windows Library 
// Copyright © 2010-2011 Vidar Hasfjord 
//
// For more information, including license details, see 
// http://owlnext.sourceforge.net
//

#include <owl/pch.h>
#include <owl/transfer.h>

#include <owl/edit.h>
#include <owl/listbox.h>
#include <owl/combobox.h>
#include <owl/combobex.h>
#include <owl/checkbox.h>
#include <owl/checklst.h>
#include <owl/radiobut.h>
#include <owl/scrollba.h>
#include <owl/slider.h>
#include <owl/static.h>
#include <owl/datetime.h>
#include <owl/hotkey.h>
#include <owl/ipaddres.h>
#include <owl/monthcal.h>
#include <owl/appdict.h>
#include <owl/except.rh>

namespace owl {

namespace
{

  template <class TRadioButtonFunc>
  TRadioButtonFunc IterateRadioButtons(HWND first, TRadioButtonFunc f)
  {
    HWND h = first;
    for (int i = 0; h != NULL; ++i)
    {
      TRadioButton c(h); 
      if (!f(c, i))
        break;
      h = c.GetWindow(GW_HWNDNEXT);
      if (GetWindowLong(h, GWL_STYLE) & WS_GROUP) // at next group?
        break;
    }
    return f;
  }

  struct TSetRadioButton
  {
    int index;
    bool operator()(TRadioButton& c, int i) const
    {
      c.Check(i == index);
      return true;
    }
  };

  struct TGetRadioButton
  {
    int index;
    bool operator()(TRadioButton& c, int i)
    {
      bool found = c.IsWindowEnabled() && c.IsChecked();
      if (found) index = i;
      return !found;
    }
  };

} // namespace

void SetSelectedRadioButtonIndex(HWND first, int selIndex)
{
  TSetRadioButton f = {selIndex};
  IterateRadioButtons(first, f);
}

int GetSelectedRadioButtonIndex(HWND first)
{
  TGetRadioButton f = {-1};
  return IterateRadioButtons(first, f).index;
}

void TransferCheckBoxData(const TTransferInfo& i, HWND ctrl, bool& b)
{
  if (i.Operation == tdSetData)
    CheckDlgButton(ctrl, b);
  else if (i.Operation == tdGetData)
    b = IsChecked(ctrl);
}

void TransferCheckBoxData(const TTransferInfo& i, HWND ctrl, UINT& state)
{
  if (i.Operation == tdSetData)
    CheckDlgButton(GetParent(ctrl), GetDlgCtrlID(ctrl), state);
  else if (i.Operation == tdGetData)
    state = IsDlgButtonChecked(GetParent(ctrl), GetDlgCtrlID(ctrl));
}

namespace
{

  //
  // Returns a reference to an TWindow derivative object given its window handle.
  // The last parameter is not used and is just a workaround for a BC++ 5.02 compiler bug.
  //
  template <class TControlType>
  TControlType& GetControlObject(HWND const parent, HWND ctrl, TControlType* = 0)
  {
    PRECONDITION(GetApplicationObject());
    TApplication& a = *GetApplicationObject();
    TWindow* w = a.GetWindowPtr(ctrl);
    if (!w) TXWindow::Raise(a.GetWindowPtr(parent), IDS_TRANSFERCONTROLMISSING);
    return dynamic_cast<TControlType&>(*w);
  }

}

void TransferCheckListData(const TTransferInfo& i, HWND ctrl, TCheckListData& data)
{
  // A TCheckList object must exist; locate it.
  //
  TCheckList& c = GetControlObject<TCheckList>(i.Window, ctrl, static_cast<TCheckList*>(0));
  c.Transfer(data, i.Operation);
}

void TransferComboBoxData(const TTransferInfo& i, HWND ctrl, TComboBoxData& data)
{
  TComboBox c(ctrl);
  c.Transfer(data, i.Operation);
}

void TransferComboBoxData(const TTransferInfo& i, HWND ctrl, int& selIndex)
{
  if (i.Operation == tdSetData) 
  {
    TComboBox c(ctrl);
    c.SetSelIndex(selIndex);
  }
  else if (i.Operation == tdGetData)
  {
    TComboBox c(ctrl);
    selIndex = c.GetSelIndex();
  }
}

void TransferComboBoxData(const TTransferInfo& i, HWND ctrl, tstring& selString, bool exact)
{
  if (i.Operation == tdSetData) 
  {
    TComboBox c(ctrl);
    if (exact)
      c.SetSelStringExact(selString);
    else
      c.SetSelString(selString);
  }
  else if (i.Operation == tdGetData)
  {
    TComboBox c(ctrl);
    selString = c.GetSelString();
  }
}

void TransferComboBoxExData(const TTransferInfo& i, HWND ctrl, TComboBoxExData& data)
{
  TComboBoxEx c(ctrl);
  c.Transfer(data, i.Operation);
}

void TransferDlgItemText(const TTransferInfo& i, HWND ctrl, tstring& text)
{
  if (i.Operation == tdSetData)
  {
    SetDlgItemText(ctrl, text);
  }
  else if (i.Operation == tdGetData)
  {
    text = GetDlgItemText(ctrl);
  }
}

void TransferDateTimePickerData(const TTransferInfo& i, HWND ctrl, TDateTimePickerData& data)
{
  TDateTimePicker c(ctrl);
  c.Transfer(data, i.Operation);
}

void TransferDateTimePickerData(const TTransferInfo& i, HWND ctrl, TSystemTime& selTime)
{
  if (i.Operation == tdSetData) 
  {
    TDateTimePicker c(ctrl);
    c.SetTime(selTime);
  }
  else if (i.Operation == tdGetData)
  {
    TDateTimePicker c(ctrl);
    selTime = c.GetTime();
  }
}

void TransferDateTimePickerData(const TTransferInfo& i, HWND ctrl, tstring& selTime)
{
  if (i.Operation == tdSetData) 
  {
    TDateTimePicker c(ctrl);
    if (selTime.length() == 0)
      c.SetNoTime();
    else
      c.SetTime(selTime); // Throws on parsing failure!
  }
  else if (i.Operation == tdGetData)
  {
    TDateTimePicker c(ctrl);
    selTime = c.GetWindowText();
  }
}

void TransferHotKeyData(const TTransferInfo& i, HWND ctrl, uint16& key)
{
  THotKey c(ctrl);
  c.Transfer(key, i.Operation);
}

void TransferIPAddressData(const TTransferInfo& i, HWND ctrl, TIPAddressBits& address)
{
  TIPAddress c(ctrl);
  c.Transfer(address, i.Operation);
}

void TransferIPAddressData(const TTransferInfo& i, HWND ctrl, uint32& address)
{
  if (i.Operation == tdSetData) 
  {
    TIPAddress c(ctrl);
    c.SetAddress(address);
  }
  else if (i.Operation == tdGetData)
  {
    TIPAddress c(ctrl);
    address = c.GetAddress();
  }
}

void TransferListBoxData(const TTransferInfo& i, HWND ctrl, TListBoxData& data)
{
  TListBox c(ctrl);
  c.Transfer(data, i.Operation);
}

void TransferListBoxData(const TTransferInfo& i, HWND ctrl, int& selIndex)
{
  if (i.Operation == tdSetData) 
  {
    TListBox c(ctrl);
    c.SetSelIndex(selIndex);
  }
  else if (i.Operation == tdGetData)
  {
    TListBox c(ctrl);
    selIndex = c.GetSelIndex();
  }
}

void TransferListBoxData(const TTransferInfo& i, HWND ctrl, tstring& selString, bool exact)
{
  if (i.Operation == tdSetData) 
  {
    TListBox c(ctrl);
    if (exact)
      c.SetSelStringExact(selString);
    else
      c.SetSelString(selString);
  }
  else if (i.Operation == tdGetData)
  {
    TListBox c(ctrl);
    selString = c.GetSelString();
  }
}

void TransferMonthCalendarData(const TTransferInfo& i, HWND ctrl, TMonthCalendarData& data)
{
  TMonthCalendar c(ctrl);
  c.Transfer(data, i.Operation);
}

void TransferMonthCalendarData(const TTransferInfo& i, HWND ctrl, TSystemTime& curSel)
{
  if (i.Operation == tdSetData) 
  {
    TMonthCalendar c(ctrl);
    c.SetCurSel(curSel);
  }
  else if (i.Operation == tdGetData)
  {
    TMonthCalendar c(ctrl);
    c.GetCurSel(curSel);
  }
}

void TransferMonthCalendarData(const TTransferInfo& i, HWND ctrl, std::pair<TSystemTime, TSystemTime>& rangeSel)
{
  if (i.Operation == tdSetData) 
  {
    TMonthCalendar c(ctrl);
    c.SetSelRange(rangeSel.first, rangeSel.second);
  }
  else if (i.Operation == tdGetData)
  {
    TMonthCalendar c(ctrl);
    c.GetSelRange(rangeSel.first, rangeSel.second);
  }
}

void TransferRadioButtonData(const TTransferInfo& i, HWND ctrl, int& selIndex)
{
  if (i.Operation == tdSetData) 
  {
    SetSelectedRadioButtonIndex(ctrl, selIndex);
  }
  else if (i.Operation == tdGetData)
  {
    selIndex = GetSelectedRadioButtonIndex(ctrl);
  }
}

void TransferScrollBarData(const TTransferInfo& i, HWND ctrl, TScrollBarData& data)
{
  TScrollBar c(ctrl);
  c.Transfer(data, i.Operation);
}

void TransferScrollBarData(const TTransferInfo& i, HWND ctrl, int& position)
{
  if (i.Operation == tdSetData) 
  {
    TScrollBar c(ctrl);
    c.SetPosition(position);
  }
  else if (i.Operation == tdGetData)
  {
    TScrollBar c(ctrl);
    position = c.GetPosition();
  }
}

void TransferSliderData(const TTransferInfo& i, HWND ctrl, TScrollBarData& data)
{
  TSlider c(ctrl);
  c.Transfer(data, i.Operation);
}

void TransferSliderData(const TTransferInfo& i, HWND ctrl, int& position)
{
  if (i.Operation == tdSetData) 
  {
    TSlider c(ctrl);
    c.SetPosition(position);
  }
  else if (i.Operation == tdGetData)
  {
    TSlider c(ctrl);
    position = c.GetPosition();
  }
}

#if _HAS_TR1 // TDelegatedTransferWindow requires std::tr1::function

TDelegatedTransferWindow::TDelegatedTransferWindow(TTransferFunction f)
: TransferFunction(f)
{}

void TDelegatedTransferWindow::TransferData(TTransferDirection d)
{
  TTransferInfo i = {GetHandle(), d};
  TransferFunction(i);
}

TDelegatedTransferDialog::TDelegatedTransferDialog(TWindow* parent, TResId id, TTransferFunction f, TModule* module)
: TDialog(parent, id, module),
  TDelegatedTransferWindow(f)
{}

#endif

} // OWL namespace