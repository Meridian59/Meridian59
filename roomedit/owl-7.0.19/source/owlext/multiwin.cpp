//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// TMultiFrame: TFrameWindow that manages "owned" TFrameWindows
//-------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owlext/multiwin.h>

using namespace owl;

namespace OwlExt {

using namespace std;

//
// Diagnostic group declarations/definitions
//
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlExtMultiFrame, 1, 0);


TMultiFrame::TMultiFrame(TWindow* parent, LPCTSTR title, TWindow* client,
             bool shrinkToClient, TModule* module)
             :
TFrameWindow(parent, title, client, shrinkToClient, module)
{
  TRACEX(OwlExtMultiFrame, 1, _T("TMultiFrame constructed at ") << hex << (void*)this << dec);

  m_frameArray = new TObjectArray<Datum>;
}
TMultiFrame::TMultiFrame(HWND hWnd, TModule* module)
:
TFrameWindow(hWnd, module)
{
  TRACEX(OwlExtMultiFrame, 2, _T("TMultiFrame constructed at ") << hex << (void*)this << dec);

  m_frameArray = new TObjectArray<Datum>;
}
TMultiFrame::~TMultiFrame()
{
  for (TObjectArray<Datum>::Iterator iter(*m_frameArray); iter; iter++) {
    TRACEX(OwlExtMultiFrame, 1,
      _T("TMultiFrame::~TMultiFrame considering ") << hex << (void*)iter.Current().m_frame
      << dec << _T(" for destruction"));
    if (iter.Current().m_ownsIt){
      TRACEX(OwlExtMultiFrame, 1, _T("TMultiFrame::~TMultiFrame destructing it"));
      delete iter.Current().m_frame;
    }
  }
  delete m_frameArray;
  TRACEX(OwlExtMultiFrame, 2, _T("TMultiFrame at ") << hex << (void*)this << dec << _T(" destructed"));
}

bool
TMultiFrame::Create()
{
  TRACEX(OwlExtMultiFrame, 2, _T("TMultiFrame(") << hex << (void*)this << dec << _T(")::Create() invoked"));

  bool retval = TFrameWindow::Create();

  // Create all frames in m_frameArray
  for (TObjectArray<Datum>::Iterator iter(*m_frameArray); iter; iter++){
    // Must turn off wfDeleteOnClose flag for these frame-windows; otherwise,
    // they'll be deleted when the user closes the associated widget, and we'll
    // have a time-bomb on our hands, waiting for us to call delete on the
    // pointer that was already deleted a LONG time ago. The culprit lies in
    // TWindow::EvClose()--see there for details.
    if (iter.Current().m_frame->IsFlagSet(wfDeleteOnClose))
      iter.Current().m_frame->ClearFlag(wfDeleteOnClose);

    if (iter.Current().m_frame->Create()!=true){
      TRACEX(OwlExtMultiFrame, 1, _T("TMultiFrame(") << hex << (void*)this << dec << _T(")::Create(): ")
        _T("Create() invoked on owned frame ") << hex << iter.Current().m_frame << dec << _T(" failed"));
      return false;
    }
    else
      iter.Current().m_frame->ShowWindow(SW_SHOW);
  }
  return retval;
}
void TMultiFrame::Destroy(int retVal)
{
  TRACEX(OwlExtMultiFrame, 2, _T("TMultiFrame(") << hex << (void*)this << dec << _T(")::Destroy() invoked"));

  // Destroy all frames in m_frameArray
  for (TObjectArray<Datum>::Iterator iter(*m_frameArray); iter; iter++)
    iter.Current().m_frame->Destroy(retVal);

  TFrameWindow::Destroy(retVal);
}
bool TMultiFrame::CanClose()
{
  // Check all frames first
  for (TObjectArray<Datum>::Iterator iter(*m_frameArray); iter; iter++)
    if (iter.Current().m_frame->CanClose()!=true)
      return false;

  return TFrameWindow::CanClose();
}

uint TMultiFrame::GetWindowCount()
{
  return m_frameArray->GetItemsInContainer();
}

TFrameWindow* TMultiFrame::operator[](unsigned int index)
{
  return (*m_frameArray)[index].m_frame;
}

bool TMultiFrame::AddFrame(TFrameWindow* frame, bool multiframeOwnsIt)
{
  TRACEX(OwlExtMultiFrame, 2, _T("TMultiFrame(") << hex << (void*)this << dec << _T(")::AddFrame() invoked"));

  // Add this window into the collection
  Datum datum(frame, multiframeOwnsIt);
  m_frameArray->Add(datum);

  // If we've been created, create this window and show it
  if (GetHandle() != NULL){
    datum.m_frame->Create();
    datum.m_frame->ShowWindow(SW_SHOW);
  }

  return true;
}


bool
TMultiFrame::RemoveFrame(TFrameWindow* frame)
{
  TRACEX(OwlExtMultiFrame, 2, _T("TMultiFrame(") << hex << (void*)this << dec << _T(")::RemoveFrame invoked"));

  // Remove this window from the collection
  for (TObjectArray<Datum>::Iterator iter(*m_frameArray); iter; iter++){
    if (iter.Current().m_frame == frame){
      if (iter.Current().m_ownsIt)
        delete iter.Current().m_frame;
      m_frameArray->DestroyItem(iter.Current());
      return true;
    }
  }

  return false;
}


bool
TMultiFrame::RemoveAllFrames()
{
  // Remove all windows from the collection
  for (TObjectArray<Datum>::Iterator iter(*m_frameArray); iter; iter++){
    if (iter.Current().m_ownsIt)
      delete iter.Current().m_frame;
    m_frameArray->DestroyItem(iter.Current());
  }
  return true;
}

} // OwlExt namespace

