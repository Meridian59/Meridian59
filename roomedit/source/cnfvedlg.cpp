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

	FILE:         cnfvedlg.cpp

	OVERVIEW
	========
	Source file for implementation of TConfirmVertexDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#include "cnfvedlg.h"


//{{TConfirmVertexDialog Implementation}}


TConfirmVertexDialog::TConfirmVertexDialog (TWindow* parent, TConfirmVertexDialogXfer &Xfer, TResId resId, TModule* module):
	TDialog(parent, resId, module)
{
//{{TConfirmVertexDialogXFER_USE}}
	pXPosCheck = new TCheckBox(this, IDC_CONFVE_XPOS, 0);
	pYPosCheck = new TCheckBox(this, IDC_CONFVE_YPOS, 0);

	SetTransferBuffer((void*)&Xfer);
//{{TConfirmVertexDialogXFER_USE_END}}
}


TConfirmVertexDialog::~TConfirmVertexDialog ()
{
	Destroy();
}


void TConfirmVertexDialog::SetupWindow ()
{
    TDialog::SetupWindow();
    ::CenterWindow(this);
}

