//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// Provides the classes TProgressThread, which is a progress-notifying thread
// class, and TThreadProgressDlg, which is a Progress dialog that handles
// thread creation and destruction.
//
// Works in both 16- and 32-bit environments!
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
#if !defined (__OWLEXT_THREDPRG_H)
#define __OWLEXT_THREDPRG_H

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H)
# include <owlext/core.h>
#endif

// Other required headers
//
#if !defined(OWL_THREAD_H)
# include <owl/thread.h>
#endif
#ifndef __OWLEXT__PROGRESS_H
# include <owlext/progress.h>
#endif


namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
//
#include <owl/preclass.h>


// This TThread derivative posts progress messages to the window
class OWLEXTCLASS TProgressThread
   : public owl::TThread
{
  // Object lifetime methods
  //
public:
  TProgressThread(UINT id = 0, HWND hTarget = 0);
  ~TProgressThread() {}

  // Internal methods and data
  //
protected:
  void NotifyStart(owl::uint32 len);
  void NotifyProgress(owl::uint32 curr);
  void NotifyEnd();

  HWND  _hTarget;
  UINT  _id;
};


// This dialog class is derived from TProgressDlg, and automatically
// handles thread createion and destruction. The method BuildThread must
// be provided.
class OWLEXTCLASS TThreadProgressDlg : public TProgressDlg
{
  // Object lifetime methods
  //
public:
  TThreadProgressDlg(owl::TWindow* parent, LPCTSTR text,
                     owl::TResId id = IDD_PROGRESS, owl::TModule* mod = 0);
  ~TThreadProgressDlg();

  // Mutators
  //
public:
  virtual TProgressThread* BuildThread() = 0;

  // OWL event-response methods
  //
public:
  void CmCancel();
protected:
  DECLARE_RESPONSE_TABLE(TThreadProgressDlg);
  virtual void SetupWindow();
  virtual void CleanupWindow();
  virtual bool CanClose();

  // Internal data
  //
protected:
  TProgressThread* _othread;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace


#endif

