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
// See THREDPRG.CPP for an example of use of this class
//
// May be used without permission, if appropriate credit is given
//
//-------------------------------------------------------------------
#if !defined (__OWLEXT_PROGRESS_H)
#define __OWLEXT_PROGRESS_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H) && !defined (__OWLEXT_ALL_H)
# include <owlext/core.h>
#endif


#include <owl/dialog.h>
#include <owl/static.h>
#include <owl/progressbar.h>


namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

// Note: Internally uses messages WM_USER + 101 through WM_USER + 105. Do not override in derived class!
class OWLEXTCLASS TProgressDlg : public owl::TDialog {
  // Internal (class-specific) constants
  //
  public:
  // Object lifetime methods
  //
  public:
    TProgressDlg(owl::TWindow* parent, LPCTSTR text,
                 owl::TResId id = owl::TResId(0),
                 owl::TModule* mod = 0);

  // OWL-overridden methods
  //
  public:
    // This forces the use of TApplication::BeginModal to
    // execute this dialog.
    auto DoExecute() -> int override;

  // Mutators
  //
  public:
    virtual void ProcessComplete(int ret = IDOK);

    void SetMessage(LPCTSTR msg)
      { Text.SetText(msg); }
    void SetRange(int l, int h)
      { Gauge.SetRange(l, h); }
    void SetValue(int x)
      { Gauge.SetValue(x); }
    void SetMarquee(bool marquee = true, owl::uint updateInterval = 0)
      { Gauge.SetMarquee(marquee, updateInterval); }

    void NotifyProgressStart(owl::uint id, int range);
    void NotifyProgress(owl::uint id, int current);
    void NotifyProgressEnd(owl::uint id, int returnValue = IDOK);
    void NotifyProgressMessage(owl::uint id, const owl::tstring& message);
    void NotifyProgressError(owl::uint id, const owl::tstring& errorMessage);

  protected:
    void SetupWindow() override;

    // Event handlers
    //
    virtual void EvProgressStart(owl::uint id, int range);
    virtual void EvProgress(owl::uint id, int current);
    virtual void EvProgressEnd(owl::uint id, int returnValue);
    virtual void EvProgressMessage(owl::uint id, const owl::tstring& message);
    virtual void EvProgressError(owl::uint id, const owl::tstring& errorMessage);

  // Internal data
  //
  private:
    owl::tstring InitialText;
    owl::TProgressBar Gauge;
    owl::TStatic Text;

  DECLARE_RESPONSE_TABLE(TProgressDlg);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace


#endif

