//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implements TMci and TMciHiddenWindow
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/mci.h>

namespace owl {

OWL_DIAGINFO;

/////////////////////////////////////////////////////////////
  static const tchar winmmStr[]            = _T("WINMM.DLL");
# if defined(UNICODE)
    static const char mciSendCommandStr[] = "mciSendCommandW";
# else
    static const char mciSendCommandStr[] = "mciSendCommandA";
# endif

//
// Returns TModule object wrapping the handle of the WINMM.DLL module
//
TModule&
TWinMM::GetModule()
{
  static TModule winmmModule(winmmStr, true, true, false);
  return winmmModule;
}

//
// Invokes 'mciSendCommand' indirectly
//
MCIERROR
TWinMM::mciSendCommand(MCIDEVICEID deviceId, UINT msg, DWORD_PTR flags, DWORD_PTR param)
{
  static TModuleProc4<MCIERROR, MCIDEVICEID, UINT, DWORD_PTR, DWORD_PTR>
    mci_SendCommand(GetModule(), mciSendCommandStr);
  return mci_SendCommand(deviceId, msg, flags, param);
};

/////////////////////////////////////////////////////////////////////////////////
//
//
//
DEFINE_RESPONSE_TABLE1(TMciHiddenWindow, TWindow)
  EV_MESSAGE(MM_MCINOTIFY, EvMciNotify),
END_RESPONSE_TABLE;

//
/// A hidden window created for the sole purpose of catching MCI messages.
//
TMciHiddenWindow::TMciHiddenWindow(TMci& mci, TModule* module)
:
  TWindow(nullptr, nullptr, module),
  Mci(mci)
{
  Attr.Style = WS_POPUP;
}

//
/// Notifies the MCI class that the MCI event has finished.
//
TResult
TMciHiddenWindow::EvMciNotify(TParam1 wp, TParam2 lp)
{
  TResult retVal = Mci.MciNotify(wp, lp);
  Mci.SetBusy(false);
  return retVal;
}

//
/// Creates a hidden window for catching messages.
//
TMci::TMci()
:
  DeviceId(0),
  WaitingForNotification(false)
{
  // Create a hidden window for notifications
  //
  Window = new TMciHiddenWindow(*this);
  Window->Create();
}

//
/// If the MCI device is still open, this closes it now. Deletes the hidden window.
//
TMci::~TMci()
{
  if (DeviceId != 0)
    Close();
  delete Window;
}

//
/// Returns the callback. If the window exists, the handle of the window is returned.
//
HWND
TMci::GetCallbackWindow() const
{
  if (Window)
    return Window->GetHandle();
  return nullptr;
}

//
/// Sends the MCI command message to the device if it's not busy.
//
MCIERROR
TMci::SendCommand(UINT msg, DWORD_PTR flags, DWORD_PTR param)
{
  if (IsBusy())
    return MCI_NOTIFY_ABORTED;

  return SendCommand(GetDeviceId(), msg, flags, param);
}

//
/// Opens the MCI device.
//
MCIERROR
TMci::Open(const MCI_OPEN_PARMS& parms, DWORD_PTR flags)
{
  MCIERROR retVal = SendCommand(0, MCI_OPEN, flags, reinterpret_cast<DWORD_PTR>(&parms));
  if (retVal == 0) {
    // success
    //
    DeviceId = parms.wDeviceID;
  }
  return retVal;
}

//
/// Stops the MCI device and closes it.
//
MCIERROR
TMci::Close()
{
  SetBusy(false);
  Stop(MCI_WAIT);
  return SendCommand(MCI_CLOSE, MCI_WAIT, 0);
}

//
/// Stops the MCI device.
//
MCIERROR
TMci::Stop(DWORD_PTR flags)
{
  SetBusy(false);
  MCI_GENERIC_PARMS parms;

  if (flags & MCI_NOTIFY)
    parms.dwCallback = reinterpret_cast<DWORD_PTR>(GetCallbackWindow());

  MCIERROR retVal = SendCommand(MCI_STOP, flags, reinterpret_cast<DWORD_PTR>(&parms));
  SetBusyIfNeeded(flags);
  return retVal;
}

//
/// Pauses the MCI device.
//
MCIERROR
TMci::Pause(DWORD_PTR flags)
{
  SetBusy(false);
  MCI_GENERIC_PARMS parms;

  if (flags & MCI_NOTIFY)
    parms.dwCallback = reinterpret_cast<DWORD_PTR>(GetCallbackWindow());

  MCIERROR retVal = SendCommand(MCI_PAUSE, flags, reinterpret_cast<DWORD_PTR>(&parms));
  SetBusyIfNeeded(flags);
  return retVal;
}

//
/// Resumes playing of the MCI device.
//
MCIERROR
TMci::Resume(DWORD_PTR flags)
{
  SetBusy(false);
  MCI_GENERIC_PARMS parms;

  if (flags & MCI_NOTIFY)
    parms.dwCallback = reinterpret_cast<DWORD_PTR>(GetCallbackWindow());

  MCIERROR retVal = SendCommand(MCI_RESUME, flags, reinterpret_cast<DWORD_PTR>(&parms));
  SetBusyIfNeeded(flags);
  return retVal;
}

//
// Sets the busy flag if the given flags include MCI_NOTIFY.
//
void
TMci::SetBusyIfNeeded(DWORD_PTR flags)
{
  if (flags & MCI_NOTIFY)
    SetBusy(true);
}

//
/// Plays the MCI device.
//
MCIERROR
TMci::Play(const MCI_PLAY_PARMS& parms, DWORD_PTR flags)
{
  MCIERROR retVal = SendCommand(MCI_PLAY, flags, reinterpret_cast<DWORD_PTR>(&parms));
  SetBusyIfNeeded(flags);
  return retVal;
}

//
/// Seeks a particular position on the MCI device. This function requires the
/// parameters to be specified in a structure.
//
MCIERROR
TMci::Seek(const MCI_SEEK_PARMS& parms, DWORD_PTR flags)
{
  MCIERROR retVal = SendCommand(MCI_SEEK, flags, reinterpret_cast<DWORD_PTR>(&parms));
  SetBusyIfNeeded(flags);
  return retVal;
}

//
/// Seeks a particular position on the MCI device. This function is provided for
/// convenience.
//
MCIERROR
TMci::Seek(DWORD to, DWORD_PTR flags)
{
  MCI_SEEK_PARMS parms;

  if (flags & MCI_NOTIFY)
    parms.dwCallback = reinterpret_cast<DWORD_PTR>(GetCallbackWindow());

  parms.dwTo = to;

  return Seek(parms, flags);
}

//
/// Loads the file into the MCI device.
//
MCIERROR
TMci::Load(const tstring& fileName, DWORD_PTR flags)
{
  MCI_LOAD_PARMS parms;

  flags |= MCI_LOAD_FILE;

  if (flags & MCI_NOTIFY)
    parms.dwCallback = reinterpret_cast<DWORD_PTR>(GetCallbackWindow());
  parms.lpfilename = fileName.c_str();
  return SendCommand(MCI_LOAD, flags, reinterpret_cast<DWORD_PTR>(&parms));
}

//
/// The default MciNotify is to return 0.
//
TResult
TMci::MciNotify(TParam1, TParam2)
{
  return 0;
}

//
/// Sends the MCI command message to the opened device.
//
MCIERROR
TMci::SendCommand(MCIDEVICEID deviceId, UINT msg, DWORD_PTR flags, DWORD_PTR param)
{
  return TWinMM::mciSendCommand(deviceId, msg, flags, param);
}

} // OWL namespace
/* ========================================================================== */

