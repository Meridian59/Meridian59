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

	FILE:         cnflddlg.cpp

	OVERVIEW
	========
    Source file for implementation of TConfirmLineDefDialog (TDialog).      
*/

#include "common.h"
#pragma hdrstop
#include "windeu.h"

#ifndef OWL_CHECKBOX_H
	#include <owl\checkbox.h>
#endif

#ifndef __newers_h
	#include "newers.h"
#endif

#include "cnflddlg.h"


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TConfirmLineDefDialog, TDialog)
//{{TConfirmLineDefDialogRSP_TBL_BEGIN}}
	EV_BN_CLICKED(IDC_LD_SIDEDEF1, SD1CheckClicked),
	EV_BN_CLICKED(IDC_LD_SIDEDEF2, SD2CheckClicked),
	EV_BN_CLICKED(IDC_LD_FLAGS, FlagsClicked),
	EV_BN_CLICKED(IDOK, CmOk),
//{{TConfirmLineDefDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TConfirmLineDefDialog Implementation}}


///////////////////////////////////////////////////////////
// TConfirmLineDefDialog
// ---------------------
//
TConfirmLineDefDialog::TConfirmLineDefDialog (TWindow* parent, TConfirmLineDefDialogXfer *pXFer, TResId resId, TModule* module):
	TDialog(parent, resId, module)
{
	pTypeCheck    = newTCheckBox(this, IDC_LD_TYPE, 0);
	pFlagsCheck   = newTCheckBox(this, IDC_LD_FLAGS, 0);
	pSectorCheck  = newTCheckBox(this, IDC_LD_SECTOR, 0);
	pVertex1Check = newTCheckBox(this, IDC_LD_VERTEX1, 0);
	pVertex2Check = newTCheckBox(this, IDC_LD_VERTEX2, 0);
	pSDCheck[0]   = newTCheckBox(this, IDC_LD_SIDEDEF1, 0);
	pSDCheck[1]   = newTCheckBox(this, IDC_LD_SIDEDEF2, 0);

	pPassPosCheck      = newTCheckBox(this, IDC_PASSPOS, 0);
	pPassNegCheck      = newTCheckBox(this, IDC_PASSNEG, 0);
	pTransPosCheck     = newTCheckBox(this, IDC_TRANSPOS, 0);
	pTransNegCheck     = newTCheckBox(this, IDC_TRANSNEG, 0);
	pFlipPosCheck      = newTCheckBox(this, IDC_FLIPPOS, 0);
	pFlipNegCheck      = newTCheckBox(this, IDC_FLIPNEG, 0);
	pNoMapCheck        = newTCheckBox(this, IDC_NOMAP, 0);
	pMapStartCheck     = newTCheckBox(this, IDC_MAPSTART, 0);
	pNoLookThroughPosCheck = newTCheckBox(this, IDC_NOLOOKTHROUGHPOS, 0);
	pNoLookThroughNegCheck = newTCheckBox(this, IDC_NOLOOKTHROUGHNEG, 0);
	pAbovePosCheck = newTCheckBox(this, IDC_POS_ABOVE, 0);
	pAboveNegCheck = newTCheckBox(this, IDC_NEG_ABOVE, 0);
	pNormalPosCheck = newTCheckBox(this, IDC_POS_NORMAL, 0);
	pNormalNegCheck = newTCheckBox(this, IDC_NEG_NORMAL, 0);
	pBelowPosCheck = newTCheckBox(this, IDC_POS_BELOW, 0);
	pBelowNegCheck = newTCheckBox(this, IDC_NEG_BELOW, 0);
	pNoVTilePosCheck = newTCheckBox(this, IDC_POS_NOVTILE, 0);
	pNoVTileNegCheck = newTCheckBox(this, IDC_NEG_NOVTILE, 0);

	pSDAboveCheck[0] = newTCheckBox(this, IDC_SD1_ABOVE_CHECK, 0);
	pSDAboveCheck[1] = newTCheckBox(this, IDC_SD2_ABOVE_CHECK, 0);

	pSDBelowCheck[0] = newTCheckBox(this, IDC_SD1_BELOW_CHECK, 0);
	pSDBelowCheck[1] = newTCheckBox(this, IDC_SD2_BELOW_CHECK, 0);

	pSDNormalCheck[0] = newTCheckBox(this, IDC_SD1_NORMAL_CHECK, 0);
	pSDNormalCheck[1] = newTCheckBox(this, IDC_SD2_NORMAL_CHECK, 0);

	pSDSectorCheck[0] = newTCheckBox(this, IDC_SD1_SECTOR_CHECK, 0);
	pSDSectorCheck[1] = newTCheckBox(this, IDC_SD2_SECTOR_CHECK, 0);

	pSDXOfsCheck[0] = newTCheckBox(this, IDC_SD1_XOFS_CHECK, 0);
	pSDXOfsCheck[1] = newTCheckBox(this, IDC_SD2_XOFS_CHECK, 0);

	pSDYOfsCheck[0] = newTCheckBox(this, IDC_SD1_YOFS_CHECK, 0);
	pSDYOfsCheck[1] = newTCheckBox(this, IDC_SD2_YOFS_CHECK, 0);

	pIDNumCheck[0] = newTCheckBox(this, IDC_POSID, 0);
	pIDNumCheck[1] = newTCheckBox(this, IDC_NEGID, 0);

	pSpeedCheck[0] = newTCheckBox(this, IDC_POSSPEED, 0);
	pSpeedCheck[1] = newTCheckBox(this, IDC_NEGSPEED, 0);

	pScrollCheck[0] = newTCheckBox(this, IDC_SCROLL1, 0);
	pScrollCheck[1] = newTCheckBox(this, IDC_SCROLL2, 0);

	SetTransferBuffer((void*)pXFer);
}


///////////////////////////////////////////////////////////
// TConfirmLineDefDialog
// ---------------------
//
TConfirmLineDefDialog::~TConfirmLineDefDialog ()
{
	Destroy();
}


///////////////////////////////////////////////////////////
// TConfirmLineDefDialog
// ---------------------
//
void TConfirmLineDefDialog::SetupWindow ()
{
	TDialog::SetupWindow();
	::CenterWindow (this);

	TransferData (tdSetData);
	EnableFlags ();
	EnableSD (0);
	EnableSD (1);
}



///////////////////////////////////////////////////////////
// TConfirmLineDefDialog
// ---------------------
//
void TConfirmLineDefDialog::CmOk ()
{
	// Check dialog box can be closed
	if ( ! CanClose () )
		return;

	TConfirmLineDefDialogXfer *pXFer =
		(TConfirmLineDefDialogXfer *)TransferBuffer;

	TransferData (tdGetData);
	if ( pXFer->pFlagsCheck == FALSE )
	{
	   pXFer->pPassPosCheck = FALSE;
	   pXFer->pPassNegCheck = FALSE;
	   pXFer->pTransPosCheck = FALSE;
	   pXFer->pTransNegCheck = FALSE;
	   pXFer->pFlipPosCheck = FALSE;
	   pXFer->pFlipNegCheck = FALSE;
	   pXFer->pNoMapCheck = FALSE;
	   pXFer->pMapStartCheck = FALSE;
	   pXFer->pNoLookThroughPosCheck = FALSE;
	   pXFer->pNoLookThroughNegCheck = FALSE;
	   pXFer->pAbovePosCheck = FALSE;
	   pXFer->pAboveNegCheck = FALSE;
	   pXFer->pNormalPosCheck = FALSE;
	   pXFer->pNormalNegCheck = FALSE;
	   pXFer->pBelowPosCheck = FALSE;
	   pXFer->pBelowNegCheck = FALSE;
	   pXFer->pNoVTilePosCheck = FALSE;
	   pXFer->pNoVTileNegCheck = FALSE;
	}

	for (int i = 0 ; i < 2 ; i++)
	{
		if ( pXFer->pSDCheck[i] == FALSE )
		{
			pSDAboveCheck[i] = FALSE;
			pSDBelowCheck[i] = FALSE;
			pSDNormalCheck[i] = FALSE;
			pSDSectorCheck[i] = FALSE;
			pSDXOfsCheck[i] = FALSE;
			pSDYOfsCheck[i] = FALSE;
			pIDNumCheck[i] = FALSE;
			pSpeedCheck[i] = FALSE;
		}
	}

	TDialog::CmOk();
}


///////////////////////////////////////////////////////////
// TConfirmLineDefDialog
// ---------------------
//
void TConfirmLineDefDialog::EnableFlags()
{
   BOOL EnableFL = (pFlagsCheck->GetCheck() == BF_CHECKED);
   pPassPosCheck->EnableWindow(EnableFL);
   pPassNegCheck->EnableWindow(EnableFL);
   pTransPosCheck->EnableWindow(EnableFL);
   pTransNegCheck->EnableWindow(EnableFL);
   pFlipPosCheck->EnableWindow(EnableFL);
   pFlipNegCheck->EnableWindow(EnableFL);
   pNoMapCheck->EnableWindow(EnableFL);
   pMapStartCheck->EnableWindow(EnableFL);
   pNoLookThroughPosCheck->EnableWindow(EnableFL);
   pNoLookThroughNegCheck->EnableWindow(EnableFL);
   pAbovePosCheck->EnableWindow(EnableFL);
   pAboveNegCheck->EnableWindow(EnableFL);
   pNormalPosCheck->EnableWindow(EnableFL);
   pNormalNegCheck->EnableWindow(EnableFL);
   pBelowPosCheck->EnableWindow(EnableFL);
   pBelowNegCheck->EnableWindow(EnableFL);
   pNoVTilePosCheck->EnableWindow(EnableFL);
   pNoVTileNegCheck->EnableWindow(EnableFL);
   pScrollCheck[0]->EnableWindow(EnableFL);
   pScrollCheck[1]->EnableWindow(EnableFL);
}


///////////////////////////////////////////////////////////
// TConfirmLineDefDialog
// ---------------------
//
void TConfirmLineDefDialog::EnableSD(int sdnum)
{
   assert (sdnum == 0 || sdnum == 1);
   BOOL EnableFL = (pSDCheck[sdnum]->GetCheck() == BF_CHECKED);
   
   pSDAboveCheck[sdnum]->EnableWindow (EnableFL);
   pSDBelowCheck[sdnum]->EnableWindow (EnableFL);
   pSDNormalCheck[sdnum]->EnableWindow (EnableFL);
   pSDSectorCheck[sdnum]->EnableWindow (EnableFL);
   pSDXOfsCheck[sdnum]->EnableWindow (EnableFL);
   pSDYOfsCheck[sdnum]->EnableWindow (EnableFL);
   pSpeedCheck[sdnum]->EnableWindow (EnableFL);
   pIDNumCheck[sdnum]->EnableWindow (EnableFL);
}


///////////////////////////////////////////////////////////
// TConfirmLineDefDialog
// ---------------------
//
void TConfirmLineDefDialog::FlagsClicked ()
{
	EnableFlags ();
}


///////////////////////////////////////////////////////////
// TConfirmLineDefDialog
// ---------------------
//
void TConfirmLineDefDialog::SD1CheckClicked ()
{
	EnableSD (0);
}


///////////////////////////////////////////////////////////
// TConfirmLineDefDialog
// ---------------------
//
void TConfirmLineDefDialog::SD2CheckClicked ()
{
	EnableSD (1);
}

