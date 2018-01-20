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


using namespace owl;

namespace OwlExt {


DEFINE_RESPONSE_TABLE1(TProgressDlg, TDialog)
EV_MESSAGE(WM_GSC_PROGRESS_START, EvProgressStart),
EV_MESSAGE(WM_GSC_PROGRESS, EvProgress),
EV_MESSAGE(WM_GSC_PROGRESS_END, EvProgressEnd),
END_RESPONSE_TABLE;

LRESULT TProgressDlg::EvProgressStart(TParam1, TParam2 range)
{
  SetRange(0, (int)range);
  return 0;
}

LRESULT TProgressDlg::EvProgress(TParam1, TParam2 val)
{
  SetValue((int)val);
  return 0;
}

// gets rid of "l is assigned a value and never used" warnings
#pragma warn -aus
LRESULT TProgressDlg::EvProgressEnd(TParam1, TParam2)
{
  int l;
  int h;

  m_gauge->GetRange(l, h);
  SetValue(h);

  ProcessComplete();

  return 0;
}
// restore "l is assigned a value and never used" warnings
#pragma warn .aus


} // OwlExt namespace

//=============================================================================