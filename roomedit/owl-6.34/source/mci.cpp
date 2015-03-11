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
TWinMM::mciSendCommand(MCIDEVICEID p1, UINT p2, DWORD p3, DWORD p4)
{
  static TModuleProc4<MCIERROR,MCIDEVICEID,UINT,DWORD,DWORD>
         mci_SendCommand(GetModule(), mciSendCommandStr);
  return mci_SendCommand(p1, p2, p3, p4);
};

/////////////////////////////////////////////////////////////////////////////////
//
//
//
DEFINE_RESPONSE_TABLE1(TMciHiddenWindow, TWindow)
  EV_MESSAGE(MM_MCINOTIFY, MciNotify),
END_RESPONSE_TABLE;

//
/// A hidden window created for the sole purpose of catching MCI messages.
//
TMciHiddenWindow::TMciHiddenWindow(TMci& mci, TModule* module)
:
  TWindow(0, 0, module),
  Mci(mci)
{
  Attr.Style = WS_POPUP;
}

//
/// Notifies the MCI class that the MCI event has finished.
//
TResult
TMciHiddenWindow::MciNotify(TParam1 wp, TParam2 lp)
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
uint32
TMci::GetCallback() const
{
  if (Window)
    return uint32(HWND(*Window));
  return 0;
}

//
/// Sends the MCI command to the device if it's not busy.
//
uint32
TMci::SendCommand(uint msg, uint32 command, uint32 param)
{
  if (IsBusy())
    return MCI_NOTIFY_ABORTED;

  return SendCommand(GetDeviceId(), msg, command, param);
}

//
/// Opens the MCI device.
//
uint32
TMci::Open(MCI_OPEN_PARMS parms, uint32 command)
{
  uint32 retVal = SendCommand(0, MCI_OPEN, command, (uint32)(void *)&parms);
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
uint32
TMci::Close()
{
  SetBusy(false);
  Stop(MCI_WAIT);
  return SendCommand(MCI_CLOSE, MCI_WAIT, 0);
}

//
/// Stops the MCI device.
//
uint32
TMci::Stop(uint32 flags)
{
  SetBusy(false);
  MCI_GENERIC_PARMS parms;

  if (flags & MCI_NOTIFY)
    parms.dwCallback = GetCallback();

  uint32 retVal = SendCommand(MCI_STOP, flags, (uint32)(void *)&parms);
  SetBusyIfNeeded(flags);
  return retVal;
}

//
/// Pauses the MCI device.
//
uint32
TMci::Pause(uint32 flags)
{
  SetBusy(false);
  MCI_GENERIC_PARMS parms;

  if (flags & MCI_NOTIFY)
    parms.dwCallback = GetCallback();

  uint32 retVal = SendCommand(MCI_PAUSE, flags, (uint32)(void *)&parms);
  SetBusyIfNeeded(flags);
  return retVal;
}

//
/// Resumes playing of the MCI device.
//
uint32
TMci::Resume(uint32 flags)
{
  SetBusy(false);
  MCI_GENERIC_PARMS parms;

  if (flags & MCI_NOTIFY)
    parms.dwCallback = GetCallback();

  uint32 retVal = SendCommand(MCI_RESUME, flags, (uint32)(void *)&parms);
  SetBusyIfNeeded(flags);
  return retVal;
}

//
// Set the busy flag if the command included MCI_NOTIFY.
//
void
TMci::SetBusyIfNeeded(uint32 command)
{
  if (command & MCI_NOTIFY)
    SetBusy(true);
}

//
/// Plays the MCI device.
//
uint32
TMci::Play(MCI_PLAY_PARMS parms, uint32 flags)
{
  uint32 retVal = SendCommand(MCI_PLAY, flags, (uint32)(void *)&parms);
  SetBusyIfNeeded(flags);
  return retVal;
}

//
/// Seeks a particular position on the MCI device. This function requires the
/// parameters to be specified in a structure.
//
uint32
TMci::Seek(MCI_SEEK_PARMS parms, uint32 flags)
{
  uint32 retVal = SendCommand(MCI_SEEK, flags, (uint32)(void *)&parms);
  SetBusyIfNeeded(flags);
  return retVal;
}

//
/// Seeks a particular position on the MCI device. This function is provided for
/// convenience.
//
uint32
TMci::Seek(uint32 to, uint32 flags)
{
  MCI_SEEK_PARMS parms;

  if (flags & MCI_NOTIFY)
    parms.dwCallback = GetCallback();

  parms.dwTo = to;

  return Seek(parms, flags);
}

//
/// Loads the file into the MCI device.
//
uint32
TMci::Load(const tstring& fileName, uint32 flags)
{
  MCI_LOAD_PARMS parms;

  flags |= MCI_LOAD_FILE;

  if (flags & MCI_NOTIFY)
    parms.dwCallback = GetCallback();
  parms.lpfilename = fileName.c_str();
  return SendCommand(MCI_LOAD, flags, (uint32)(void *)&parms);
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
/// Sends the MCI command to the opened device.
//
uint32
TMci::SendCommand(uint deviceId, uint msg, uint32 command, uint32 param)
{
  return TWinMM::mciSendCommand(deviceId, msg, command, param);
}

} // OWL namespace
/* ========================================================================== */

