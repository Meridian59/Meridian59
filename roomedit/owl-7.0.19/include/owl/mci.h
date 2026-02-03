//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Windows MCI (Media Control Interface) encapsulation classes.
//----------------------------------------------------------------------------

#if !defined(OWL_MCI_H)
#define OWL_MCI_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/window.h>
#include <mmsystem.h>


namespace owl {

#include <owl/preclass.h>

class _OWLCLASS TMci;

//
/// \class TMciHiddenWindow
// ~~~~~ ~~~~~~~~~~~~~~~~
/// A private TWindow derivative used by TMci to capture events.
//
class _OWLCLASS TMciHiddenWindow : public TWindow {
  public:
    TMciHiddenWindow(TMci&, TModule* = 0);

    TResult EvMciNotify(TParam1, TParam2);

  private:
    TMci& Mci;

  DECLARE_RESPONSE_TABLE(TMciHiddenWindow);
};

//
/// \class TMci
// ~~~~~ ~~~~
/// TMci is a Windows MCI (Media Control Interface) encapsulation class.
//
class _OWLCLASS TMci {
  public:
    // Constructors and destructors
    //
    TMci();
    virtual ~TMci();

    // Available commands on any MCI device
    //
    MCIERROR Open(const MCI_OPEN_PARMS&, DWORD_PTR flags = 0);
    MCIERROR Close();
    MCIERROR Play(const MCI_PLAY_PARMS&, DWORD_PTR flags = 0);
    MCIERROR Stop(DWORD_PTR flags = 0);
    MCIERROR Pause(DWORD_PTR flags = 0);
    MCIERROR Resume(DWORD_PTR flags = 0);
    MCIERROR Seek(const MCI_SEEK_PARMS&, DWORD_PTR flags = 0);
    MCIERROR Seek(DWORD to, DWORD_PTR flags = 0);
    MCIERROR Load(const tstring& fileName, DWORD_PTR flags = 0);

    // Virtual function to override in derived classes to know when
    // an MCI event is finished.
    //
    virtual TResult MciNotify(TParam1, TParam2);

    // State information
    //
    bool    IsBusy() const;
    void    SetBusy(bool);

    // General wrappers and properties
    //
    MCIDEVICEID GetDeviceId() const;
    HWND GetCallbackWindow() const;
    MCIERROR SendCommand(UINT msg, DWORD_PTR flags, DWORD_PTR param);
    static MCIERROR SendCommand(MCIDEVICEID deviceId, UINT msg, DWORD_PTR flags, DWORD_PTR param);

  private:
    void     SetBusyIfNeeded(DWORD_PTR flags);

    MCIDEVICEID DeviceId;                    ///< Id for the MCI device
    bool     WaitingForNotification;      ///< Flag for asynchronous busy
    TWindow* Window;                      ///< Owner
};

//
/// \class TMciWaveAudio
// ~~~~~ ~~~~~~~~~~~~~
/// Wrapper to play .WAV files.
//
class _OWLCLASS TMciWaveAudio : public TMci {
  public:
    TMciWaveAudio(LPCTSTR elementName = 0, LPCTSTR deviceName = 0,
      uint16 id = 0);

    TMciWaveAudio(const tstring& elementName, const tstring& deviceName, uint16 id = 0);

    MCIERROR Play(DWORD_PTR flags = 0, DWORD from = 0, DWORD to = 0);
};

//
/// \class TWinMM
// ~~~~~ ~~~~~~
/// delay loading WINMM.DLL
class _OWLCLASS TWinMM {
  public:
    static MCIERROR mciSendCommand(MCIDEVICEID, UINT msg, DWORD_PTR flags, DWORD_PTR param);
    static TModule&  GetModule();
};

#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns the ID of the open MCI device.
//
inline uint TMci::GetDeviceId() const {
  return DeviceId;
}

//
/// Returns true if the MCI is currently busy doing something.
//
inline bool TMci::IsBusy() const {
  return WaitingForNotification;
}

//
/// Sets the busy flag for the MCI device.
//
inline void TMci::SetBusy(bool b) {
  WaitingForNotification = b;
}


} // OWL namespace

#endif  // OWL_MCI_H
