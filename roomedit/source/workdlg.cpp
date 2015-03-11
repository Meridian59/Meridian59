/*----------------------------------------------------------------------------*
 | This file is part of WinDEU, the port of DEU to Windows.                   |
 | WinDEU was created by the DEU team:                                        |
 |  Renaud Paquay, Raphael Quinet, Brendon Wyber and others...                |
 |                                                                            |
 | DEU is an open project: if you think that you can contribute, please join  |
 | the DEU team.  You will be credited for any code (or ideas) included in    |
 | the next version of the program.                                           |
 |                                                                            |
 | If you want to make any modifications and re-distribute them on your own,  |
 | you must follow the conditions of the WinDEU license. Read the file        |
 | LICENSE or README.TXT in the top directory.  If do not  have a copy of     |
 | these files, you can request them from any member of the DEU team, or by   |
 | mail: Raphael Quinet, Rue des Martyrs 9, B-4550 Nandrin (Belgium).         |
 |                                                                            |
 | This program comes with absolutely no warranty.  Use it at your own risks! |
 *----------------------------------------------------------------------------*

	Project WinDEU
	DEU team
	Jul-Dec 1994, Jan-Mar 1995

	FILE:         workdlg.cpp

	OVERVIEW
	========
	Source file for implementation of TWorkDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#include "workdlg.h"

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_STATIC_H
	#include <owl\static.h>
#endif

#ifndef OWL_GAUGE_H
	#include <owl\gauge.h>
#endif

//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TWorkDialog, TDialog)
//{{TWorkDialogRSP_TBL_BEGIN}}
	EV_BN_CLICKED(IDCANCEL, CmCancel),
//{{TWorkDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TWorkDialog Implementation}}


TWorkDialog::TWorkDialog (TWindow* parent, TResId resId, TModule* module):
	TDialog(parent, resId, module)
{
	Cancel = FALSE;
	vmin = 0;
	vmax = 100;

	pGaugeStatic = new TStatic(this, IDC_GAUGE_STATIC);
	pWorkStatic = new TStatic(this, IDC_WORK_TEXT);
	pGauge = new TGauge (this, "%d%%", IDC_GAUGE_STATIC+1,
						 0, 0, 10, 10);
	pGauge->SetRange(0, 100);
}


TWorkDialog::~TWorkDialog ()
{
	Destroy();
}


void TWorkDialog::CmCancel ()
{
	Cancel = TRUE;
}


void TWorkDialog::SetupWindow ()
{
	TDialog::SetupWindow();
	::CenterWindow(this);

	// Disable main window
	GetApplication()->GetMainWindow()->EnableWindow(FALSE);

	TRect gRect;
	TPoint TopLeft, BottomRight;

	pGaugeStatic->GetWindowRect (gRect);
	TopLeft     = TPoint(gRect.left + 2, gRect.top + 2);
	BottomRight = TPoint(gRect.right - 2, gRect.bottom - 2);
	ScreenToClient(TopLeft);
	ScreenToClient(BottomRight);

	pGauge->MoveWindow(TRect (TopLeft, BottomRight));
}


void TWorkDialog::Destroy (int retVal)
{
	// Enable main window
	GetApplication()->GetMainWindow()->EnableWindow(TRUE);

	TDialog::Destroy(retVal);
}


void TWorkDialog::SetValue(int value)
{
	if ( vmin - vmax == 0 )
		pGauge->SetValue (100);
	else
		pGauge->SetValue( (int)((LONG)(value - vmin) * 100 / (vmax - vmin)) );
	pGauge->UpdateWindow();
}


