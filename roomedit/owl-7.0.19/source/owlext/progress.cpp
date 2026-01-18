//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// TDialog-derived class which provides a dialog box containing a gauge
// and a text box, and responds to defined messages to update the gauge.
//
// Works in both 16- and 32- bit environments
//
// Original code:
// Copyright (c) 1997 Rich Goldstein, MD
// goldstei@interport.net
//
// May be used without permission, if appropriate credit is given
//
// See THREDPRG.CPP for an example of use of this class
//-------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owlext/progress.h>
#include <owlext/progress.rh>

using namespace owl;

namespace {

//
// Internal messages
//
enum
{
  wmProgressStart_ = WM_USER + 101,
  wmProgress_,
  wmProgressEnd_,
  wmProgressMessage_,
  wmProgressError_
};

//
// Dispatch template specializations for the internal messages
//
template <TMsgId>
struct TDispatch_;

template <> struct TDispatch_<wmProgressStart_>
{
  template <class T, void (T:: * M)(uint id, int range)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  { return (static_cast<T*>(i)->*M)(static_cast<uint>(p1), static_cast<int>(p2)), 0; }
};

template <> struct TDispatch_<wmProgress_>
{
  template <class T, void (T:: * M)(uint id, int current)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  { return (static_cast<T*>(i)->*M)(static_cast<uint>(p1), static_cast<int>(p2)), 0; }
};

template <> struct TDispatch_<wmProgressEnd_>
{
  template <class T, void (T:: * M)(uint id, int returnValue)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  { return (static_cast<T*>(i)->*M)(static_cast<uint>(p1), static_cast<int>(p2)), 0; }
};

template <> struct TDispatch_<wmProgressMessage_>
{
  template <class T, void (T:: * M)(uint id, const tstring&)>
  static TResult Decode(void* i, TParam1 p1, TParam2 p2)
  {
    const auto str = std::unique_ptr<const tchar[]>(reinterpret_cast<LPTSTR>(p2));
    return (static_cast<T*>(i)->*M)(static_cast<uint>(p1), str.get()), 0;
  }
};

template <> struct TDispatch_<wmProgressError_> : TDispatch_<wmProgressMessage_>
{};

} // anonymous namespace

//
// Response table macro for the internal messages
//
#define EV_PROGRESS_(msgId, method) {{msgId}, 0, OWL_DISPATCH(TDispatch_<msgId>::Decode, method)}

namespace OwlExt {

DEFINE_RESPONSE_TABLE1(TProgressDlg, TDialog)
  EV_PROGRESS_(wmProgressStart_, EvProgressStart),
  EV_PROGRESS_(wmProgress_, EvProgress),
  EV_PROGRESS_(wmProgressEnd_, EvProgressEnd),
  EV_PROGRESS_(wmProgressMessage_, EvProgressMessage),
  EV_PROGRESS_(wmProgressError_, EvProgressError),
END_RESPONSE_TABLE;

TProgressDlg::TProgressDlg(TWindow* parent, LPCTSTR text, TResId id, TModule* mod)
  : TDialog(parent, id == TResId(0) ? TResId(IDD_PROGRESS) : id, mod),
  InitialText(text),
  Gauge(this, IDC_GAUGE),
  Text(this, IDC_TEXT)
{
  // Required attributes for this dialog to work properly.
  //
  ModifyStyle(WS_CHILD, WS_VISIBLE);
}

auto TProgressDlg::DoExecute() -> int
{
  // This forces the use of TApplication::BeginModal to execute this dialog.
  //
  return TWindow::DoExecute();
}

void TProgressDlg::SetupWindow()
{
  TDialog::SetupWindow();
  Text.SetText(InitialText.c_str());
}

void TProgressDlg::NotifyProgressStart(uint id, int range)
{
  PostMessage(wmProgressStart_, static_cast<TParam1>(id), static_cast<TParam2>(range));
}

void TProgressDlg::NotifyProgress(uint id, int current)
{
  PostMessage(wmProgress_, static_cast<TParam1>(id), static_cast<TParam2>(current));
}

void TProgressDlg::NotifyProgressEnd(uint id, int returnValue)
{
  PostMessage(wmProgressEnd_, static_cast<TParam1>(id), static_cast<TParam2>(returnValue));
}

void TProgressDlg::NotifyProgressMessage(uint id, const tstring& message)
{
  // Note: The string will be deallocated in TDispatch_<wmProgressMessage_>::Decode.
  //
  LPTSTR s = strnewdup(message.c_str()); 
  PostMessage(wmProgressMessage_, static_cast<TParam1>(id), reinterpret_cast<TParam2>(s));
}

void TProgressDlg::NotifyProgressError(uint id, const tstring& errorMessage)
{
  // Note: The string will be deallocated in TDispatch_<wmProgressError_>::Decode.
  //
  LPTSTR s = strnewdup(errorMessage.c_str());
  PostMessage(wmProgressError_, static_cast<TParam1>(id), reinterpret_cast<TParam2>(s));
}

void TProgressDlg::ProcessComplete(int ret)
{
  // By default, the window is closed when the process completes.
  //
  CloseWindow(ret);
}

void TProgressDlg::EvProgressStart(uint id, int range)
{
  SetRange(0, range);
  if (range <= 0)
    SetMarquee();
}

void TProgressDlg::EvProgress(uint id, int current)
{
  SetValue(current);
}

void TProgressDlg::EvProgressEnd(uint id, int returnValue)
{
  SetValue(Gauge.GetRange().second);
  ProcessComplete(returnValue);
}

void TProgressDlg::EvProgressMessage(uint id, const tstring& message)
{
  Text.SetText(message);
}

void TProgressDlg::EvProgressError(uint id, const tstring& errorMessage)
{
  MessageBox(errorMessage, _T("Error"), MB_OK | MB_ICONSTOP);
}

} // OwlExt namespace

//=============================================================================