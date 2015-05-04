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

	FILE:         cnfthdlg.cpp

	OVERVIEW
	========
	Source file for implementation of TConfirmThingDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#include "cnfthdlg.h"

#ifndef OWL_CHECKBOX_H
	#include <owl\checkbox.h>
#endif

#ifndef __newers_h
	#include <newers.h>
#endif



//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TConfirmThingDialog, TDialog)
//{{TConfirmThingDialogRSP_TBL_BEGIN}}
	EV_BN_CLICKED(IDOK, CmOk),
//{{TConfirmThingDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TConfirmThingDialog Implementation}}


// static TConfirmThingDialogXfer TConfirmThingDialogData;

///////////////////////////////////////////////////////////
// TConfirmThingDialog
// -------------------
//
TConfirmThingDialog::TConfirmThingDialog (TWindow* parent, TConfirmThingDialogXfer &XFer, TResId resId, TModule* module):
	TDialog(parent, resId, module)
{
//{{TConfirmThingDialogXFER_USE}}
	pAngleCheck   = newTCheckBox(this, IDC_ANGLE_CHECK, 0);
	pTypeCheck    = newTCheckBox(this, IDC_TYPE_CHECK, 0);
	pXPosCheck    = newTCheckBox(this, IDC_XPOS_CHECK, 0);
	pYPosCheck 	  = newTCheckBox(this, IDC_YPOS_CHECK, 0);
	pDeafCheck 	  = newTCheckBox(this, IDC_CONF_DEAF_CHECK, 0);
    pLevel12Check = newTCheckBox(this, IDC_CONF_LEVEL12_CHECK, 0);
    pLevel3Check  = newTCheckBox(this, IDC_CONF_LEVEL3_CHECK, 0);
    pLevel45Check = newTCheckBox(this, IDC_CONF_LEVEL45_CHECK, 0);
    pMultiCheck   = newTCheckBox(this, IDC_CONF_MULTI_CHECK, 0);

	SetTransferBuffer((void*)&XFer);
//{{TConfirmThingDialogXFER_USE_END}}
}


///////////////////////////////////////////////////////////
// TConfirmThingDialog
// -------------------
//
TConfirmThingDialog::~TConfirmThingDialog ()
{
	Destroy();
}


///////////////////////////////////////////////////////////
// TConfirmThingDialog
// -------------------
//
void TConfirmThingDialog::SetupWindow ()
{
	TDialog::SetupWindow();
	::CenterWindow (this);
}


///////////////////////////////////////////////////////////
// TConfirmThingDialog
// -------------------
//
void TConfirmThingDialog::CmOk ()
{
	TDialog::CmOk();
}

