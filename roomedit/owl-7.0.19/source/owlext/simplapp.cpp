//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// TSimpleApp definition
//-------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owl/appdict.h>

#include <owlext/simplapp.h>

using namespace owl;

namespace OwlExt {

//
// Diagnostic group declarations/definitions
//
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlExtSimpleApp, 1, 0);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                    TSimpleApp
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Globals and statics
TAppDictionary TSimpleApp::s_appDictionary;
static TModule* module = 0; // just local variable
// Implementation
TSimpleApp::TSimpleApp(TFrameWindow* fwndw, LPCTSTR name)
:
TApplication(  name, module, &TSimpleApp::s_appDictionary),
m_window(fwndw)
{
}

void
TSimpleApp::InitMainWindow()
{
  SetMainWindow(m_window);
}

bool
TSimpleApp::IdleAction(long idleCount)
{
  // The idea here is fairly straightforward--if our main window has shut down,
  // then post a quit message to our queue. However, we also need to remove the
  // "alias" app already in the TAppDictionary.
  if (idleCount==0){
    if (GetMainWindow()->GetHandle() == 0){
      TRACEX(OwlExtSimpleApp, 1, "TSimpleApp has no main window anymore!");
      ::PostQuitMessage(0);
    }
    TApplication* app = s_appDictionary.GetApplication(::GetCurrentProcessId());
    if (app && app->GetMainWindow() && app->GetMainWindow()->GetHandle()==0){
      TRACEX(OwlExtSimpleApp, 1, "Alias app has no main window anymore, either!");
      s_appDictionary.Remove(app);
    }
  }
  return true;
}

} // OwlExt namespace
