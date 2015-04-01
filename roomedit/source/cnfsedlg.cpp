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

	FILE:         cnfsedlg.cpp

	OVERVIEW
	========
	Source file for implementation of TConfirmSectorDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#ifndef __cnfsedlg_h
	#include "cnfsedlg.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_CHECKBOX_H
	#include <owl\checkbox.h>
#endif

#ifndef __newers_h
	#include "newers.h"		// new...
#endif


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TConfirmSectorDialog, TDialog)
//{{TConfirmSectorDialogRSP_TBL_BEGIN}}
	EV_BN_CLICKED(IDOK, CmOk),
//{{TConfirmSectorDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TConfirmSectorDialog Implementation}}

///////////////////////////////////////////////////////////
// TConfirmSectorDialog
// --------------------
//
TConfirmSectorDialog::TConfirmSectorDialog (TWindow* parent, TConfirmSectorDialogXfer &Xfer, TResId resId, TModule* module):
	TDialog(parent, resId, module)
{
//{{TConfirmSectorDialogXFER_USE}}
	pSpecialCheck        = newTCheckBox(this, IDC_SPECIAL_CHECK, 0);
	pTagCheck            = newTCheckBox(this, IDC_TAG_CHECK, 0);
	pLightCheck          = newTCheckBox(this, IDC_LIGHT_CHECK, 0);
	pTextureOffsetCheck  = newTCheckBox(this, IDC_TEXTURE_OFFSETS_CHECK, 0);
	pFloorTextureCheck   = newTCheckBox(this, IDC_FLOOR_TEXTURE_CHECK, 0);
	pFloorHeightCheck    = newTCheckBox(this, IDC_FLOOR_HEIGHT_CHECK, 0);
	pCeilingTextureCheck = newTCheckBox(this, IDC_CEILING_TEXTURE_CHECK, 0);
	pCeilingHeightCheck  = newTCheckBox(this, IDC_CEILING_HEIGHT_CHECK, 0);
	pIDNumCheck          = newTCheckBox(this, IDC_ID, 0);
	pSpeedCheck          = newTCheckBox(this, IDC_SPEED, 0);
	pDepthCheck          = newTCheckBox(this, IDC_DEPTH, 0);
	pScrollCheck         = newTCheckBox(this, IDC_SCROLL, 0);
	pSlopeCheck          = newTCheckBox(this, IDC_SLOPES, 0);

	SetTransferBuffer(&Xfer);
//{{TConfirmSectorDialogXFER_USE_END}}
}


///////////////////////////////////////////////////////////
// TConfirmSectorDialog
// --------------------
//
TConfirmSectorDialog::~TConfirmSectorDialog ()
{
	Destroy();
}


///////////////////////////////////////////////////////////
// TConfirmSectorDialog
// --------------------
//
void TConfirmSectorDialog::SetupWindow ()
{
	TDialog::SetupWindow();
	::CenterWindow (this);
}


///////////////////////////////////////////////////////////
// TConfirmSectorDialog
// --------------------
//
void TConfirmSectorDialog::CmOk ()
{
	TDialog::CmOk();
}

