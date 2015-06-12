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

	FILE:         lineedit.cpp

	OVERVIEW
	========
	Source file for implementation of TLineDefEditDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#include "bsp.h"
#include "lineedit.h"
#include "levels.h"

#ifndef OWL_LISTBOX_H
#include <owl\listbox.h>
#endif

#ifndef OWL_EDIT_H
#include <owl\edit.h>
#endif

#ifndef OWL_RADIOBUT_H
#include <owl\radiobut.h>
#endif

#ifndef OWL_CHECKBOX_H
#include <owl\checkbox.h>
#endif

#ifndef OWL_VALIDATE_H
#include <owl\validate.h>
#endif

#ifndef __cnflddlg_h
#include "cnflddlg.h"
#endif

#ifndef __names_h
#include "names.h"
#endif

#ifndef __newers_h
#include "newers.h"		// new...
#endif

#ifndef __cnflddlg_h
#include "cnflddlg.h"
#endif

#ifndef __viewbmp_h
#include "viewbmp.h"
#endif

// HELP context IDs
#include "windeuhl.h"


/////////////////////////////////////////////////////////////////////
//
// DOOM1 & HERETIC LineDef types
//
/////////////////////////////////////////////////////////////////////
const SHORT D1_DoorsTab[] =
{
   // DOOM1/HERETIC
   0,	// Normal
   1, 26, 27, 28, 63, 29, 90, 4, 31, 32, 34, 33, 61, 103, 50, 86,
   2, 46, 42, 75, 3, 76, 16,
   -1
   };

const SHORT D1_CeilingsTab[] =
{
   // DOOM1/HERETIC
   43, 41, 72, 44, 40,
   -1
   };

const SHORT D1_RaiseFloorsTab[] =
{
   // DOOM1/HERETIC
   69, 18, 20, 22, 95, 47, 5, 14, 30, 93, 59, 66, 67, 15, 92,
   58, 96, 64, 101, 91, 24, 65, 94, 56,
   -1
   };

const SHORT D1_LowerFloorsTab[] =
{
   // DOOM1/HERETIC
   45, 60, 102, 82, 83, 19, 38, 70, 71, 98, 36, 23, 84, 68, 37,
   9, 21,
   -1
   };

const SHORT D1_MovingThingsTab[] =
{
   // DOOM1/HERETIC
   62, 88, 10, 77, 6, 73, 74, 57, 87, 53, 89, 54, 7, 8,
   -1
   };

const SHORT D1_SpecialTab[] =
{
   // DOOM1/HERETIC
   48, 11, 52, 51, 97, 39, 81, 13, 79, 35, 80, 12, 104, 17,
   -1
   };

#define LD_DOORS		0
#define LD_CEILINGS		1
#define LD_RAISEFLOORS	2
#define LD_LOWERFLOORS	3
#define LD_MOVINGTHINGS	4
#define LD_SPECIAL      5

const SHORT *D1_LineDefSetTab[] =
{
   D1_DoorsTab,
   D1_CeilingsTab,
   D1_RaiseFloorsTab,
   D1_LowerFloorsTab,
   D1_MovingThingsTab,
   D1_SpecialTab
   };

// const NB_LINEDEFSET = sizeof(LineDefSetTab) / sizeof (int *);
const int NB_LINEDEFSET = 6;


/////////////////////////////////////////////////////////////////////
//
// DOOM2 new LineDefs
//
/////////////////////////////////////////////////////////////////////
const SHORT D2_DoorsTab[] =
{
   // DOOM1/HERETIC
   0,	// Normal
   1, 26, 27, 28, 63, 29, 90, 4, 31, 32, 34, 33, 61, 103, 50, 86,
   2, 46, 42, 75, 3, 76, 16,
   // DOOM2
   99, 100, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
   116, 117, 118, 133, 134, 135, 136, 137,
   -1
   };

const SHORT D2_CeilingsTab[] =
{
   // DOOM1/HERETIC
   43, 41, 72, 44, 40,
   // DOOM2
   25, 49, 85, 141,
   -1
   };

const SHORT D2_RaiseFloorsTab[] =
{
   // DOOM1/HERETIC
   69, 18, 20, 22, 95, 47, 5, 14, 30, 93, 59, 66, 67, 15, 92,
   58, 96, 64, 101, 91, 24, 65, 94, 56,
   // DOOM2
   55, 119, 128, 129, 130, 131, 132, 140,
   -1
   };

const SHORT D2_LowerFloorsTab[] =
{
   // DOOM1/HERETIC
   45, 60, 102, 82, 83, 19, 38, 70, 71, 98, 36, 23, 84, 68, 37,
   9, 21,
   -1
   };

const SHORT D2_MovingThingsTab[] =
{
   // DOOM1/HERETIC
   62, 88, 10, 77, 6, 73, 74, 57, 87, 53, 89, 54, 7, 8,
   // DOOM2
   120, 121, 122, 123, 127,
   -1
   };

const SHORT D2_SpecialTab[] =
{
   // DOOM1/HERETIC
   48, 11, 52, 51, 97, 39, 81, 13, 79, 35, 80, 12, 104, 17,
   // DOOM2
   125, 126, 124, 78, 138, 139,
   -1
   };

const SHORT *D2_LineDefSetTab[] =
{
   D2_DoorsTab,
   D2_CeilingsTab,
   D2_RaiseFloorsTab,
   D2_LowerFloorsTab,
   D2_MovingThingsTab,
   D2_SpecialTab
   };


inline const SHORT *GetLineDefTab(int i)
{
   if ( DoomVersion == 2 )
      return D2_LineDefSetTab[i];
   else
      return D1_LineDefSetTab[i];
}


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TLineDefEditDialog, TDialog)
//{{TLineDefEditDialogRSP_TBL_BEGIN}}
//EV_BN_CLICKED(IDC_DOORS, DoorsClicked),
//EV_BN_CLICKED(IDC_CEILINGS, CeilingsClicked),
//EV_BN_CLICKED(IDC_LOWER_FLOORS, LowerFloorsClicked),
//EV_BN_CLICKED(IDC_RAISE_FLOORS, RaiseFloorsClicked),
//EV_BN_CLICKED(IDC_MOVING_THINGS, MovingThingsClicked),
//EV_BN_CLICKED(IDC_SPECIAL, SpecialClicked),
EV_BN_CLICKED(IDC_SD1_CLEAR_ABOVE, SD1ClearAboveClicked),
EV_BN_CLICKED(IDC_SD1_CLEAR_BELOW, SD1ClearBelowClicked),
EV_BN_CLICKED(IDC_SD1_CLEAR_NORMAL, SD1ClearNormalClicked),
EV_BN_CLICKED(IDC_SD1_TO_NORMAL, SD1ToNormalClicked),
EV_BN_CLICKED(IDC_SD1_TO_ABOVE, SD1ToAboveClicked),
EV_BN_CLICKED(IDC_SD1_TO_BELOW, SD1ToBelowClicked),
EV_BN_CLICKED(IDC_SD2_CLEAR_ABOVE, SD2ClearAboveClicked),
EV_BN_CLICKED(IDC_SD2_CLEAR_BELOW, SD2ClearBelowClicked),
EV_BN_CLICKED(IDC_SD2_CLEAR_NORMAL, SD2ClearNormalClicked),
EV_BN_CLICKED(IDC_SD2_TO_ABOVE, SD2ToAboveClicked),
EV_BN_CLICKED(IDC_SD2_TO_BELOW, SD2ToBelowClicked),
EV_BN_CLICKED(IDC_SD2_TO_NORMAL, SD2ToNormalClicked),
//EV_LBN_SELCHANGE(IDC_LINEDEF_LIST, LineDefSelChange),
EV_LBN_DBLCLK(IDC_TEXTURE_LIST, TextureListDBLClick),
EV_EN_KILLFOCUS(IDC_SD1, SD1Killfocus),
EV_EN_KILLFOCUS(IDC_SD2, SD2Killfocus),
EV_LBN_SELCHANGE(IDC_TEXTURE_LIST, TextureSelchange),
EV_BN_CLICKED(IDC_SD1_NEW, SD1NewClicked),
EV_BN_CLICKED(IDC_SD2_NEW, SD2NewClicked),
EV_BN_CLICKED(IDOK, CmOk),
EV_WM_LBUTTONDOWN,
EV_WM_LBUTTONDBLCLK,
EV_BN_CLICKED(IDC_PASSPOS, PassPosClicked),
EV_BN_CLICKED(IDC_PASSNEG, PassNegClicked),
EV_BN_CLICKED(IDC_TRANSPOS, TransPosClicked),
EV_BN_CLICKED(IDC_TRANSNEG, TransNegClicked),
EV_BN_CLICKED(IDC_FLIPPOS, FlipPosClicked),
EV_BN_CLICKED(IDC_FLIPNEG, FlipNegClicked),
EV_BN_CLICKED(IDC_NOMAP, NoMapClicked),
EV_BN_CLICKED(IDC_MAPSTART, MapStartClicked),
EV_BN_CLICKED(IDC_NOLOOKTHROUGHPOS, NoLookThroughPosClicked),
EV_BN_CLICKED(IDC_NOLOOKTHROUGHNEG, NoLookThroughNegClicked),
EV_BN_CLICKED(IDC_POS_ABOVE, AbovePosClicked),
EV_BN_CLICKED(IDC_NEG_ABOVE, AboveNegClicked),
EV_BN_CLICKED(IDC_POS_NORMAL, NormalPosClicked),
EV_BN_CLICKED(IDC_NEG_NORMAL, NormalNegClicked),
EV_BN_CLICKED(IDC_POS_BELOW, BelowPosClicked),
EV_BN_CLICKED(IDC_NEG_BELOW, BelowNegClicked),
EV_BN_CLICKED(IDC_POS_NOVTILE, NoVTilePosClicked),
EV_BN_CLICKED(IDC_NEG_NOVTILE, NoVTileNegClicked),
EV_BN_CLICKED(IDC_SCROLLN1, ScrollPosClicked),
EV_BN_CLICKED(IDC_SCROLLNE1, ScrollPosClicked),
EV_BN_CLICKED(IDC_SCROLLE1, ScrollPosClicked),
EV_BN_CLICKED(IDC_SCROLLSE1, ScrollPosClicked),
EV_BN_CLICKED(IDC_SCROLLS1, ScrollPosClicked),
EV_BN_CLICKED(IDC_SCROLLSW1, ScrollPosClicked),
EV_BN_CLICKED(IDC_SCROLLW1, ScrollPosClicked),
EV_BN_CLICKED(IDC_SCROLLNW1, ScrollPosClicked),
EV_BN_CLICKED(IDC_SCROLLNONE1, ScrollPosClicked),
EV_BN_CLICKED(IDC_SCROLLSLOW1, ScrollPosClicked),
EV_BN_CLICKED(IDC_SCROLLMEDIUM1, ScrollPosClicked),
EV_BN_CLICKED(IDC_SCROLLFAST1, ScrollPosClicked),
EV_BN_CLICKED(IDC_SCROLLN2, ScrollNegClicked),
EV_BN_CLICKED(IDC_SCROLLNE2, ScrollNegClicked),
EV_BN_CLICKED(IDC_SCROLLE2, ScrollNegClicked),
EV_BN_CLICKED(IDC_SCROLLSE2, ScrollNegClicked),
EV_BN_CLICKED(IDC_SCROLLS2, ScrollNegClicked),
EV_BN_CLICKED(IDC_SCROLLSW2, ScrollNegClicked),
EV_BN_CLICKED(IDC_SCROLLW2, ScrollNegClicked),
EV_BN_CLICKED(IDC_SCROLLNW2, ScrollNegClicked),
EV_BN_CLICKED(IDC_SCROLLNONE2, ScrollNegClicked),
EV_BN_CLICKED(IDC_SCROLLSLOW2, ScrollNegClicked),
EV_BN_CLICKED(IDC_SCROLLMEDIUM2, ScrollNegClicked),
EV_BN_CLICKED(IDC_SCROLLFAST2, ScrollNegClicked),
//{{TLineDefEditDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TLineDefEditDialog Implementation}}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
TLineDefEditDialog::TLineDefEditDialog (TWindow* parent, SelPtr Selected, TResId resId, TModule* module):
TDialog(parent, resId, module)
{
   assert (Selected != NULL);
   assert (Selected->objnum >= 0  && Selected->objnum < NumLineDefs);
   
   LineDefSet = -1;
   TextureName[0] = '\0';
   SelLineDefs = Selected;
   CurLineDef = LineDefs[Selected->objnum];
   memset(&ConfirmData, 0, sizeof(ConfirmData));
   
   // Crete object for texture view, but do not create dialog box.
   pWTextureDialog = NULL;
   
   pPassPosCheck      = newTCheckBox(this, IDC_PASSPOS, 0);
   pPassNegCheck      = newTCheckBox(this, IDC_PASSNEG, 0);
   pTransPosCheck     = newTCheckBox(this, IDC_TRANSPOS, 0);
   pTransNegCheck     = newTCheckBox(this, IDC_TRANSNEG, 0);
   pFlipPosCheck      = newTCheckBox(this, IDC_FLIPPOS, 0);
   pFlipNegCheck      = newTCheckBox(this, IDC_FLIPNEG, 0);
   pNoMapCheck        = newTCheckBox(this, IDC_NOMAP, 0);
   pMapStartCheck     = newTCheckBox(this, IDC_MAPSTART, 0);
   pPosNoLookThroughCheck = newTCheckBox(this, IDC_NOLOOKTHROUGHPOS, 0);
   pNegNoLookThroughCheck = newTCheckBox(this, IDC_NOLOOKTHROUGHNEG, 0);
   pPosAboveCheck = newTCheckBox(this, IDC_POS_ABOVE, 0);
   pNegAboveCheck = newTCheckBox(this, IDC_NEG_ABOVE, 0);
   pPosNormalCheck = newTCheckBox(this, IDC_POS_NORMAL, 0);
   pNegNormalCheck = newTCheckBox(this, IDC_NEG_NORMAL, 0);
   pPosBelowCheck = newTCheckBox(this, IDC_POS_BELOW, 0);
   pNegBelowCheck = newTCheckBox(this, IDC_NEG_BELOW, 0);
   pPosNoVTile = newTCheckBox(this, IDC_POS_NOVTILE, 0);
   pNegNoVTile = newTCheckBox(this, IDC_NEG_NOVTILE, 0);
   
   pVertex1Edit   = newTEdit(this, IDC_VERTEX2, 9);
   pVertex2Edit   = newTEdit(this, IDC_VERTEX1, 9);
   
   pSDEdit[0] = newTEdit(this, IDC_SD1, 9);
   pSDEdit[1] = newTEdit(this, IDC_SD2, 9);
   
   pSDXPosEdit[0]     = newTEdit(this, IDC_SD1_XPOS, 9);
   pSDYPosEdit[0]     = newTEdit(this, IDC_SD1_YPOS, 9);
   pSDSectorEdit[0]   = newTEdit(this, IDC_SD1_SECTOR, 9);
   pSDAboveStatic[0]  = newTStatic(this, IDC_SD1_ABOVE, MAX_BITMAPNAME);
   pSDBelowStatic[0]  = newTStatic(this, IDC_SD1_BELOW, MAX_BITMAPNAME);
   pSDNormalStatic[0] = newTStatic(this, IDC_SD1_NORMAL, MAX_BITMAPNAME);
   pIDNumEdit[0]      = newTEdit(this, IDC_POSID, 9);
   pSpeedEdit[0]      = newTEdit(this, IDC_POSSPEED, 9);
   
   pSDXPosEdit[1]     = newTEdit(this, IDC_SD2_XPOS, 9);
   pSDYPosEdit[1]     = newTEdit(this, IDC_SD2_YPOS, 9);
   pSDSectorEdit[1]   = newTEdit(this, IDC_SD2_SECTOR, 9);
   pSDAboveStatic[1]  = newTStatic(this, IDC_SD2_ABOVE, MAX_BITMAPNAME);
   pSDBelowStatic[1]  = newTStatic(this, IDC_SD2_BELOW, MAX_BITMAPNAME);
   pSDNormalStatic[1] = newTStatic(this, IDC_SD2_NORMAL, MAX_BITMAPNAME);
   pIDNumEdit[1]      = newTEdit(this, IDC_NEGID, 9);
   pSpeedEdit[1]      = newTEdit(this, IDC_NEGSPEED, 9);
   
   pTextureList = newTListBox(this, IDC_TEXTURE_LIST);
//   pLineDefList = newTListBox(this, IDC_LINEDEF_LIST);
   
   pSDToAbove[0]     = newTButton(this, IDC_SD1_TO_ABOVE);
   pSDToBelow[0]     = newTButton(this, IDC_SD1_TO_BELOW);
   pSDToNormal[0]    = newTButton(this, IDC_SD1_TO_NORMAL);
   pSDClearAbove[0]  = newTButton(this, IDC_SD1_CLEAR_ABOVE);
   pSDClearBelow[0]  = newTButton(this, IDC_SD1_CLEAR_BELOW);
   pSDClearNormal[0] = newTButton(this, IDC_SD1_CLEAR_NORMAL);
   
   pSDToAbove[1]     = newTButton(this, IDC_SD2_TO_ABOVE);
   pSDToBelow[1]     = newTButton(this, IDC_SD2_TO_BELOW);
   pSDToNormal[1]    = newTButton(this, IDC_SD2_TO_NORMAL);
   pSDClearAbove[1]  = newTButton(this, IDC_SD2_CLEAR_ABOVE);
   pSDClearBelow[1]  = newTButton(this, IDC_SD2_CLEAR_BELOW);
   pSDClearNormal[1] = newTButton(this, IDC_SD2_CLEAR_NORMAL);

   pScrollNRadio[0]   = newTRadioButton(this, IDC_SCROLLN1);
   pScrollNERadio[0]  = newTRadioButton(this, IDC_SCROLLNE1);
   pScrollERadio[0]   = newTRadioButton(this, IDC_SCROLLE1);
   pScrollSERadio[0]  = newTRadioButton(this, IDC_SCROLLSE1);
   pScrollSRadio[0]   = newTRadioButton(this, IDC_SCROLLS1);
   pScrollSWRadio[0]  = newTRadioButton(this, IDC_SCROLLSW1);
   pScrollWRadio[0]   = newTRadioButton(this, IDC_SCROLLW1);
   pScrollNWRadio[0]  = newTRadioButton(this, IDC_SCROLLNW1);
   pScrollNoneRadio[0]  = newTRadioButton(this, IDC_SCROLLNONE1);
   pScrollSlowRadio[0]  = newTRadioButton(this, IDC_SCROLLSLOW1);
   pScrollMediumRadio[0]  = newTRadioButton(this, IDC_SCROLLMEDIUM1);
   pScrollFastRadio[0]  = newTRadioButton(this, IDC_SCROLLFAST1);

   pScrollNRadio[1]   = newTRadioButton(this, IDC_SCROLLN2);
   pScrollNERadio[1]  = newTRadioButton(this, IDC_SCROLLNE2);
   pScrollERadio[1]   = newTRadioButton(this, IDC_SCROLLE2);
   pScrollSERadio[1]  = newTRadioButton(this, IDC_SCROLLSE2);
   pScrollSRadio[1]   = newTRadioButton(this, IDC_SCROLLS2);
   pScrollSWRadio[1]  = newTRadioButton(this, IDC_SCROLLSW2);
   pScrollWRadio[1]   = newTRadioButton(this, IDC_SCROLLW2);
   pScrollNWRadio[1]  = newTRadioButton(this, IDC_SCROLLNW2);
   pScrollNoneRadio[1]  = newTRadioButton(this, IDC_SCROLLNONE2);
   pScrollSlowRadio[1]  = newTRadioButton(this, IDC_SCROLLSLOW2);
   pScrollMediumRadio[1]  = newTRadioButton(this, IDC_SCROLLMEDIUM2);
   pScrollFastRadio[1]  = newTRadioButton(this, IDC_SCROLLFAST2);
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
TLineDefEditDialog::~TLineDefEditDialog ()
{
   Destroy();
   delete pWTextureDialog;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SetupWindow ()
{
   TDialog::SetupWindow();
   ::CenterWindow (this);
   
//   InitLineDefSet();
//   SetLineDefList();
   SetTextureList();
   SetLineDef();
   SetSideDef(0);
   SetSideDef(1);
   
   // Setup validators for edit controls
   pSDEdit[0]->SetValidator (new TRangeValidator (-1, NumSideDefs-1));
   pSDEdit[1]->SetValidator (new TRangeValidator (-1, NumSideDefs-1));
   pVertex1Edit->SetValidator (new TRangeValidator (-1, NumVertexes-1));
   pVertex2Edit->SetValidator (new TRangeValidator (-1, NumVertexes-1));
   pIDNumEdit[0]->SetValidator (new TRangeValidator (0, 65535));
   pIDNumEdit[1]->SetValidator (new TRangeValidator (0, 65535));
   pSpeedEdit[0]->SetValidator (new TRangeValidator (0, 255));
   pSpeedEdit[1]->SetValidator (new TRangeValidator (0, 255));
   
   pSDSectorEdit[0]->SetValidator (new TRangeValidator (-1, NumSectors-1));
   pSDSectorEdit[1]->SetValidator (new TRangeValidator (-1, NumSectors-1));
   pSDXPosEdit[0]->SetValidator (new TRangeValidator (-32767, 32767));
   pSDYPosEdit[0]->SetValidator (new TRangeValidator (-999, 999));
   pSDXPosEdit[1]->SetValidator (new TRangeValidator (-32767, 32767));
   pSDYPosEdit[1]->SetValidator (new TRangeValidator (-999, 999));
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::CmOk ()
{
   // Cannot close if edit controls not valid
   if ( ! CanClose() )
      return;
   
   SHORT ldnum = SelLineDefs->objnum;
   
   // Get CurLineDef info from controls
   GetLineDef ();
   
   // Check that we have two different SideDef numbers
   if (CurLineDef.sidedef1 != -1  &&
       CurLineDef.sidedef1 == CurLineDef.sidedef2)
   {
      MessageBox ("The two SideDef numbers MUST be different !",
		  "Error",
		  MB_OK | MB_ICONEXCLAMATION);
      return;
   }
   
   // Did the user made changes ?
   LineDef *pLineDef = &LineDefs[ldnum];
   if ( memcmp (&CurLineDef, pLineDef, sizeof (CurLineDef)) != 0 )
      MadeChanges = TRUE;
   
   if (CurLineDef.start    != pLineDef->start    ||
       CurLineDef.end      != pLineDef->end      ||
       CurLineDef.sidedef1 != pLineDef->sidedef1 ||
       CurLineDef.sidedef2 != pLineDef->sidedef2 )
   {
      MadeMapChanges = TRUE;
   }
   
   *pLineDef = CurLineDef;
   
   // Save SideDef 1
   if ( CurLineDef.sidedef1 >= 0 )
   {
      GetSideDef (0);
      SideDefs[CurLineDef.sidedef1] = CurSD[0];
   }
   
   // Save SideDef 2
   if ( CurLineDef.sidedef2 >= 0 )
   {
      GetSideDef (1);
      SideDefs[CurLineDef.sidedef2] = CurSD[1];
   }
   
   // Copy to the other selected LineDef's
   if ( SelLineDefs->next != NULL )
   {
      SET_HELP_CONTEXT(Confirming_copy_of_LineDef_attributes);
      if (TConfirmLineDefDialog(this, &ConfirmData).Execute() == IDOK)
      {
	 //
	 BOOL CopySD1Number = FALSE;
	 BOOL CopySD2Number = FALSE;
	 
	 // Check that we can copy SideDef data from first LineDef
	 if ( CurLineDef.sidedef1 < 0  &&  ConfirmData.pSDCheck[0] == TRUE)
	 {
	    if ( Expert )
	       CopySD1Number = TRUE;
	    else
	       CopySD1Number = Confirm(
				       "The first SideDef number of the LineDef %d is "
				       "\"-1\" (meaning there's no SideDef).\nDo you want "
				       "WinDEU to copy this number to the others "
				       "selected LineDefs (meaning they will have no "
				       "first SideDef) ?");
	 }
	 
	 if ( CurLineDef.sidedef2 < 0  &&  ConfirmData.pSDCheck[1] == TRUE )
	 {
	    if ( Expert )
	       CopySD2Number = TRUE;
	    else
	       CopySD2Number = Confirm(
				       "The second SideDef number of the LineDef %d is "
				       "\"-1\" (meaning there's no SideDef).\nDo you want "
				       "WinDEU to copy this number to the others "
				       "selected LineDefs (meaning they will have no "
				       "second SideDef) ?");
	    /*
	       Notify ("Cannot copy the %s SideDef data from the LineDef %d "
		       "to the other selected LineDefs, because LineDef %d "
		       "doesn't have such a SideDef.\n"
		       "The copy process will continue normaly, except for "
		       "this SideDef data.",
		       CurLineDef.sidedef1 < 0 ? "first" : "second",
		       SelLineDefs->objnum, SelLineDefs->objnum);
	    */
	 }
	 
	 // Copy data to each other selected LineDef
	 for (SelPtr LdPtr = SelLineDefs->next ;
	      LdPtr != NULL ;
	      LdPtr = LdPtr->next)
	 {
	    LineDef *pSelLineDef = &LineDefs[LdPtr->objnum];
	    LineDef LineDefBefore = *pSelLineDef;
	    
	    if ( ConfirmData.pTypeCheck )
	       pSelLineDef->type = CurLineDef.type;
	    
	    if ( ConfirmData.pSectorCheck )
	       pSelLineDef->tag = CurLineDef.tag;
	    
	    if ( ConfirmData.pVertex1Check )
	       pSelLineDef->start = CurLineDef.start;
	    
	    if ( ConfirmData.pVertex2Check )
	       pSelLineDef->end = CurLineDef.end;
	    
	    // Copy SideDef 1 data (or number if -1)
	    if ( CopySD1Number )
	    {
	       LineDefs[LdPtr->objnum].sidedef1 = CurLineDef.sidedef1;
	       MadeChanges = TRUE;
	       MadeMapChanges = TRUE;
	    }
	    else
	       CopySDData (0, LdPtr->objnum, ConfirmData);
	    
	    // Copy SideDef 2 data (or number if -1)
	    if ( CopySD2Number )
	    {
	       LineDefs[LdPtr->objnum].sidedef2 = CurLineDef.sidedef2;
	       MadeChanges = TRUE;
	       MadeMapChanges = TRUE;
	    }
	    else
	       CopySDData (1, LdPtr->objnum, ConfirmData);
	    
	    // Copy each individual flag
	    if ( ConfirmData.pFlagsCheck )
	    {
	       int lflags = pSelLineDef->blak_flags;
	       int cflags = CurLineDef.blak_flags;
	       if ( ConfirmData.pPassPosCheck )
		  if ( cflags & BF_POS_PASSABLE )	lflags |= BF_POS_PASSABLE;
		  else					lflags &= ~BF_POS_PASSABLE;
	       
	       if ( ConfirmData.pPassNegCheck )
		  if ( cflags & BF_NEG_PASSABLE )	lflags |= BF_NEG_PASSABLE;
		  else					lflags &= ~BF_NEG_PASSABLE;
	       
	       if ( ConfirmData.pTransPosCheck )
		  if ( cflags & BF_POS_TRANSPARENT )	lflags |= BF_POS_TRANSPARENT;
		  else					lflags &= ~BF_POS_TRANSPARENT;
	       
	       if ( ConfirmData.pTransNegCheck )
		  if ( cflags & BF_NEG_TRANSPARENT )	lflags |= BF_NEG_TRANSPARENT;
		  else					lflags &= ~BF_NEG_TRANSPARENT;
	       
	       if ( ConfirmData.pFlipPosCheck )
		  if ( cflags & BF_POS_BACKWARDS )	lflags |= BF_POS_BACKWARDS;
		  else					lflags &= ~BF_POS_BACKWARDS;
	       
	       if ( ConfirmData.pFlipNegCheck )
		  if ( cflags & BF_NEG_BACKWARDS )	lflags |= BF_NEG_BACKWARDS;
		  else					lflags &= ~BF_NEG_BACKWARDS;

	       if ( ConfirmData.pNoMapCheck )
		  if ( cflags & BF_MAP_NEVER )	lflags |= BF_MAP_NEVER;
		  else					lflags &= ~BF_MAP_NEVER;
	       
	       if ( ConfirmData.pMapStartCheck )
		  if ( cflags & BF_MAP_ALWAYS )	lflags |= BF_MAP_ALWAYS;
		  else					lflags &= ~BF_MAP_ALWAYS;

	       if ( ConfirmData.pNoLookThroughPosCheck )
		  if ( cflags & BF_POS_NOLOOKTHROUGH )	lflags |= BF_POS_NOLOOKTHROUGH;
		  else					lflags &= ~BF_POS_NOLOOKTHROUGH;

	       if ( ConfirmData.pNoLookThroughNegCheck )
		  if ( cflags & BF_NEG_NOLOOKTHROUGH )	lflags |= BF_NEG_NOLOOKTHROUGH;
		  else					lflags &= ~BF_NEG_NOLOOKTHROUGH;

	       if ( ConfirmData.pAbovePosCheck )
		  if ( cflags & BF_POS_ABOVE_BUP )	lflags |= BF_POS_ABOVE_BUP;
		  else				        lflags &= ~BF_POS_ABOVE_BUP;

	       if ( ConfirmData.pAboveNegCheck )
		  if ( cflags & BF_NEG_ABOVE_BUP )	lflags |= BF_NEG_ABOVE_BUP;
		  else				        lflags &= ~BF_NEG_ABOVE_BUP;

	       if ( ConfirmData.pNormalPosCheck )
		  if ( cflags & BF_POS_NORMAL_TDOWN )	lflags |= BF_POS_NORMAL_TDOWN;
		  else				        lflags &= ~BF_POS_NORMAL_TDOWN;

	       if ( ConfirmData.pNormalNegCheck )
		  if ( cflags & BF_NEG_NORMAL_TDOWN )	lflags |= BF_NEG_NORMAL_TDOWN;
		  else				        lflags &= ~BF_NEG_NORMAL_TDOWN;

	       if ( ConfirmData.pBelowPosCheck )
		  if ( cflags & BF_POS_BELOW_TDOWN )	lflags |= BF_POS_BELOW_TDOWN;
		  else				        lflags &= ~BF_POS_BELOW_TDOWN;

	       if ( ConfirmData.pBelowNegCheck )
		  if ( cflags & BF_NEG_BELOW_TDOWN )	lflags |= BF_NEG_BELOW_TDOWN;
		  else				        lflags &= ~BF_NEG_BELOW_TDOWN;
	       
	       if ( ConfirmData.pNoVTilePosCheck )
		  if ( cflags & BF_POS_NO_VTILE )	lflags |= BF_POS_NO_VTILE;
		  else				        lflags &= ~BF_POS_NO_VTILE;

	       if ( ConfirmData.pNoVTileNegCheck )
		  if ( cflags & BF_NEG_NO_VTILE )	lflags |= BF_NEG_NO_VTILE;
		  else				        lflags &= ~BF_NEG_NO_VTILE;

	       if (ConfirmData.pScrollCheck[0])
	       {
		  lflags = (lflags & ~0x00300000) | (WallScrollPosSpeed(cflags) << 20);
		  lflags = (lflags & ~0x01C00000) | (WallScrollPosDirection(cflags) << 22);
	       }

	       if (ConfirmData.pScrollCheck[1])
	       {
		  lflags = (lflags & ~0x06000000) | (WallScrollNegSpeed(cflags) << 25);
		  lflags = (lflags & ~0x38000000) | (WallScrollNegDirection(cflags) << 27);
	       }

	       pSelLineDef->blak_flags = lflags;
	    }
	    
	    // Did we made changes?
	    if ( memcmp (pSelLineDef, &LineDefBefore, sizeof (CurLineDef)) != 0 )
	       MadeChanges = TRUE;
	    
	    if (LineDefBefore.start    != pSelLineDef->start    ||
		LineDefBefore.end      != pSelLineDef->end      ||
		LineDefBefore.sidedef1 != pSelLineDef->sidedef1 ||
		LineDefBefore.sidedef2 != pSelLineDef->sidedef2 )
	    {
	       MadeMapChanges = TRUE;
	    }
	 }
      }
      RESTORE_HELP_CONTEXT();
   }
   
   // Close Dialog box
   TDialog::CmOk();
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//  Get the the LineDef data from the Controls (to CurLineDef)
void TLineDefEditDialog::GetLineDef ()
{
   char str[10];
   
   // Get LineDef info and setup Confirm data flags
   pSDEdit[0]->GetText (str, 6);
   // May already be TRUE (if New SideDef), so use |=
   ConfirmData.pSDCheck[0] |= (CurLineDef.sidedef1 != atoi (str));
   CurLineDef.sidedef1 = atoi (str);
   
   pSDEdit[1]->GetText (str, 6);
   // May already be TRUE (if New SideDef), so use |=
   ConfirmData.pSDCheck[1] |= (CurLineDef.sidedef2 != atoi (str));
   CurLineDef.sidedef2 = atoi (str);
   
   pVertex1Edit->GetText (str, 6);
   ConfirmData.pVertex1Check = (CurLineDef.start != atoi (str));
   CurLineDef.start = atoi (str);
   
   pVertex2Edit->GetText (str, 6);
   ConfirmData.pVertex2Check = (CurLineDef.end != atoi (str));
   CurLineDef.end = atoi (str);
   
   // Get LineDef flags
   int flags = 0;
   if ( pPassPosCheck->GetCheck() == BF_CHECKED )	flags |= BF_POS_PASSABLE;
   if ( pPassNegCheck->GetCheck() == BF_CHECKED )       flags |= BF_NEG_PASSABLE;
   if ( pTransPosCheck->GetCheck() == BF_CHECKED )      flags |= BF_POS_TRANSPARENT;
   if ( pTransNegCheck->GetCheck() == BF_CHECKED )      flags |= BF_NEG_TRANSPARENT;
   if ( pFlipPosCheck->GetCheck() == BF_CHECKED )       flags |= BF_POS_BACKWARDS;
   if ( pFlipNegCheck->GetCheck() == BF_CHECKED )       flags |= BF_NEG_BACKWARDS;
   if ( pNoMapCheck->GetCheck() == BF_CHECKED )         flags |= BF_MAP_NEVER;
   if ( pMapStartCheck->GetCheck() == BF_CHECKED )      flags |= BF_MAP_ALWAYS;
   if ( pPosNoLookThroughCheck->GetCheck() == BF_CHECKED )      flags |= BF_POS_NOLOOKTHROUGH;
   if ( pNegNoLookThroughCheck->GetCheck() == BF_CHECKED )      flags |= BF_NEG_NOLOOKTHROUGH;
   if ( pPosAboveCheck->GetCheck() == BF_CHECKED )      flags |= BF_POS_ABOVE_BUP;
   if ( pNegAboveCheck->GetCheck() == BF_CHECKED )      flags |= BF_NEG_ABOVE_BUP;
   if ( pPosNormalCheck->GetCheck() == BF_CHECKED )     flags |= BF_POS_NORMAL_TDOWN;
   if ( pNegNormalCheck->GetCheck() == BF_CHECKED )     flags |= BF_NEG_NORMAL_TDOWN;
   if ( pPosBelowCheck->GetCheck() == BF_CHECKED )      flags |= BF_POS_BELOW_TDOWN;
   if ( pNegBelowCheck->GetCheck() == BF_CHECKED )      flags |= BF_NEG_BELOW_TDOWN;
   if ( pPosNoVTile->GetCheck() == BF_CHECKED )         flags |= BF_POS_NO_VTILE;
   if ( pNegNoVTile->GetCheck() == BF_CHECKED )         flags |= BF_NEG_NO_VTILE;

   if (pScrollNRadio[0]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_N << 22;
   else if (pScrollNERadio[0]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_NE << 22;
   else if (pScrollERadio[0]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_E << 22;
   else if (pScrollSERadio[0]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_SE << 22;
   else if (pScrollSRadio[0]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_S << 22;
   else if (pScrollSWRadio[0]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_SW << 22;
   else if (pScrollWRadio[0]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_W << 22;
   else if (pScrollNWRadio[0]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_NW << 22;

   if (pScrollSlowRadio[0]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_SLOW << 20;
   else if (pScrollMediumRadio[0]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_MEDIUM << 20;
   else if (pScrollFastRadio[0]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_FAST << 20;

   if (pScrollNRadio[1]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_N << 27;
   else if (pScrollNERadio[1]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_NE << 27;
   else if (pScrollERadio[1]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_E << 27;
   else if (pScrollSERadio[1]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_SE << 27;
   else if (pScrollSRadio[1]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_S << 27;
   else if (pScrollSWRadio[1]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_SW << 27;
   else if (pScrollWRadio[1]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_W << 27;
   else if (pScrollNWRadio[1]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_NW << 27;

   if (pScrollSlowRadio[1]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_SLOW << 25;
   else if (pScrollMediumRadio[1]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_MEDIUM << 25;
   else if (pScrollFastRadio[1]->GetCheck() == BF_CHECKED)
      flags |= SCROLL_FAST << 25;

   CurLineDef.blak_flags = flags;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//  Get SideDef 'sdnum' from Controls (to CurSD[sdnum])
void TLineDefEditDialog::GetSideDef (int sdnum)
{
   assert (sdnum == 0  ||  sdnum == 1);
   assert ( (sdnum == 0 && CurLineDef.sidedef1 < NumSideDefs) ||
	   (sdnum == 1 && CurLineDef.sidedef2 < NumSideDefs) );
   
   // Get SideDef info from controls
   char str[MAX_BITMAPNAME + 1];
   pSDXPosEdit[sdnum]->GetText (str, 6);
   ConfirmData.pSDXOfsCheck[sdnum] = (CurSD[sdnum].xoff != (SHORT)atoi (str));
   // May already be true (new SideDef), so use |=
   ConfirmData.pSDCheck[sdnum] |= ConfirmData.pSDXOfsCheck[sdnum];
   CurSD[sdnum].xoff = (SHORT)atoi (str);
   
   pSDYPosEdit[sdnum]->GetText (str, 6);
   ConfirmData.pSDYOfsCheck[sdnum] = (CurSD[sdnum].yoff != (SHORT)atoi (str));
   // May already be true (new SideDef), so use |=
   ConfirmData.pSDCheck[sdnum] |= ConfirmData.pSDYOfsCheck[sdnum];
   CurSD[sdnum].yoff = (SHORT)atoi (str);
   
   pSDSectorEdit[sdnum]->GetText (str, 6);
   ConfirmData.pSDSectorCheck[sdnum] = (CurSD[sdnum].sector != (SHORT)atoi (str));
   // May already be true (new SideDef), so use |=
   ConfirmData.pSDCheck[sdnum] |= ConfirmData.pSDSectorCheck[sdnum];
   CurSD[sdnum].sector = (SHORT)atoi (str);

   pSpeedEdit[sdnum]->GetText (str, 6);
   ConfirmData.pSpeedCheck[sdnum] = (CurSD[sdnum].animate_speed != (SHORT)atoi (str));
   // May already be true (new SideDef), so use |=
   ConfirmData.pSDCheck[sdnum] |= ConfirmData.pSpeedCheck[sdnum];
   CurSD[sdnum].animate_speed = (BYTE)atoi (str);

   pIDNumEdit[sdnum]->GetText (str, 6);
   ConfirmData.pIDNumCheck[sdnum] = (CurSD[sdnum].user_id != (SHORT)atoi (str));
   // May already be true (new SideDef), so use |=
   ConfirmData.pSDCheck[sdnum] |= ConfirmData.pIDNumCheck[sdnum];
   CurSD[sdnum].user_id = (SHORT)atoi (str);

   // Copy texture names & types to sidedef info   
   pSDAboveStatic[sdnum]->GetText (str, MAX_BITMAPNAME);
   memset(CurSD[sdnum].tex1, 0, MAX_BITMAPNAME);
   strncpy (CurSD[sdnum].tex1, str, MAX_BITMAPNAME);
   CurSD[sdnum].type1 = TextureToNumber(str);

   pSDBelowStatic[sdnum]->GetText (str, MAX_BITMAPNAME);
   memset(CurSD[sdnum].tex2, 0, MAX_BITMAPNAME);
   strncpy (CurSD[sdnum].tex2, str, MAX_BITMAPNAME);
   CurSD[sdnum].type2 = TextureToNumber(str);
   
   pSDNormalStatic[sdnum]->GetText (str, MAX_BITMAPNAME);
   memset(CurSD[sdnum].tex3, 0, MAX_BITMAPNAME);
   strncpy (CurSD[sdnum].tex3, str, MAX_BITMAPNAME);
   CurSD[sdnum].type3 = TextureToNumber(str);
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::CopySDData (int sdnum, SHORT DestLD,
				     TConfirmLineDefDialogXfer &xfer)
{
   assert (sdnum == 0 || sdnum == 1);
   assert (DestLD >= 0 && DestLD < NumLineDefs);
   
   // Do not copy if SideDef CheckBox not checked
   if ( xfer.pSDCheck[sdnum] == FALSE )
      return;
   
   LineDef *pDestLineDef = &LineDefs[DestLD];
   
   // Retreive source and dest. SideDef numbers
   SHORT SrcSD, DestSD;
   if ( sdnum == 0 )
   {
      SrcSD = CurLineDef.sidedef1;
      DestSD = pDestLineDef->sidedef1;
   }
   else
   {
      SrcSD = CurLineDef.sidedef2;
      DestSD = pDestLineDef->sidedef2;
   }
   
   // Do not copy if no SideDef data for the source LineDef
   // (a warning message was displayed before)
   if ( SrcSD < 0 )
      return;
   
   // Check the destination SideDef
   if ( DestSD < 0 )
   {
      // By default, a new SideDef is created for the destination LineDef
      if ( Expert ||
	  Confirm ("The LineDef %d doesn't currently have a %s SideDef, so "
		   "it's impossible to copy the SideDef data from the "
		   "LineDef %d. \n\nDo you want to create a NEW %s SideDef "
		   "for the LineDef %d ?"
		   "\n(If not, this LineDef will be skipped.)",
		   DestLD, sdnum == 0 ? "first" : "second",
		   SelLineDefs->objnum,
		   sdnum == 0 ? "first" : "second", DestLD) == TRUE )
      {
	 // Create new SideDef copied from the source SideDef
	 InsertObject (OBJ_SIDEDEFS, SrcSD, 0, 0);
	 
	 // Link it to the destination LineDef
	 DestSD = NumSideDefs - 1;
	 if ( sdnum == 0 )		pDestLineDef->sidedef1 = DestSD;
	 else               		pDestLineDef->sidedef2 = DestSD;
      }
      // Either we created a SideDef copied from the source SideDef,
      // or we skip the copy of theSideDef. So it's OK to quit
      return;
   }
   
   // Copy selected data members to the Dest. SideDef
   SideDef *pSrcSideDef = &SideDefs[SrcSD];
   SideDef *pDestSideDef = &SideDefs[DestSD];
   
   if ( xfer.pSDAboveCheck[sdnum] )
   {
      strncpy (pDestSideDef->tex1, pSrcSideDef->tex1, MAX_BITMAPNAME);
      pDestSideDef->type1 = TextureToNumber(pDestSideDef->tex1);
   }
   
   if ( xfer.pSDBelowCheck[sdnum] )
   {
      strncpy (pDestSideDef->tex2, pSrcSideDef->tex2, MAX_BITMAPNAME);
      pDestSideDef->type2 = TextureToNumber(pDestSideDef->tex2);
   }
   
   if ( xfer.pSDNormalCheck[sdnum] )
   {
      strncpy (pDestSideDef->tex3, pSrcSideDef->tex3, MAX_BITMAPNAME);
      pDestSideDef->type3 = TextureToNumber(pDestSideDef->tex3);
   }
   
   if ( xfer.pSDSectorCheck[sdnum] )
      pDestSideDef->sector = pSrcSideDef->sector;
   
   if ( xfer.pSDXOfsCheck[sdnum] )
      pDestSideDef->xoff = pSrcSideDef->xoff;
   
   if ( xfer.pSDYOfsCheck[sdnum] )
      pDestSideDef->yoff = pSrcSideDef->yoff;

   if ( xfer.pSpeedCheck[sdnum] )
      pDestSideDef->animate_speed = pSrcSideDef->animate_speed;

   if ( xfer.pIDNumCheck[sdnum] )
      pDestSideDef->user_id = pSrcSideDef->user_id;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//  Retrieve LineDefSet from the first selected LineDef type.
//  If no set found, LineDefSet = -1
void TLineDefEditDialog::InitLineDefSet ()
{
   SHORT type;
   
   LineDefSet = -1;
   for (int i = 0 ; i < NB_LINEDEFSET  &&  LineDefSet < 0 ; i++)
   {
      for (int j = 0 ;
	   (type = GetLineDefTab(i)[j]) != -1;
	   j++)
      {
	 if ( type == CurLineDef.type )
	 {
	    LineDefSet = i;
	    break;
	 }
      }
   }

}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//  Show the list of type for the LineDefSet set.
//  If LineDefSet == -1, UNKNOWN is displayed
void TLineDefEditDialog::SetLineDefList ()
{
   assert (pLineDefList->IsWindow());
   SHORT type;
   int TabStops[] = { 5*4, 5*4 + 5*4 };
   
   pLineDefList->ClearList();
   pLineDefList->SetTabStops (2, TabStops);
   
   if ( LineDefSet < 0  ||  LineDefSet >= NB_LINEDEFSET )
   {
      char str[50];
      
      wsprintf (str, "[%d]\tUnknown ???(DO NOT USE)", CurLineDef.type);
      pLineDefList->AddString (str);
      return;
   }
   
   for ( int i = 0 ;
	(type = GetLineDefTab(LineDefSet)[i]) != -1;
	i++)
   {
      char str[128];
      char ldname[128];
      
      strcpy (ldname, GetLineDefTypeLongName (type));
      
      // UGLY, but it works
      ldname[2] = '\t';
      wsprintf (str, "[%d]\t%s", type, ldname);
      pLineDefList->AddString (str);
      
      if ( CurLineDef.type == type )
	 pLineDefList->SetSelString (str, -1);
   }
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SetTextureList ()
{
   assert (pTextureList->IsWindow());
   
   // Add texture1 and texture 2 names
   int TabStops[] = { 24 * 4, 24 * 4 + 4 * 4};
   pTextureList->SetTabStops (2, TabStops);
   
   // Add wall textures entries entries
   for (SHORT ti = 0 ; ti < NumWTexture ; ti++)
   {
      char str[100];
      
      wsprintf (str, "%s\t%3d\t%3d", WTexture[ti]->Name, WTexture[ti]->Width,
		WTexture[ti]->Height);
      
      // Adds the texture name in the list
      pTextureList->AddString (str);
   }
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//	Setup Controls for the current LineDef (CurLineDef)
//  and copy the two SideDefs data to CurSD[0] and CurSD[1]
void TLineDefEditDialog::SetLineDef ()
{
   assert (CurLineDef.sidedef1 == -1  || (USHORT)CurLineDef.sidedef1 < NumSideDefs);
   assert (CurLineDef.sidedef2 == -1  || (USHORT)CurLineDef.sidedef2 < NumSideDefs);
   char str[10];
   
   // Init. current sidedef's
   if ( CurLineDef.sidedef1 >= 0 )
      CurSD[0] = SideDefs[CurLineDef.sidedef1];
   
   if ( CurLineDef.sidedef2 >= 0 )
      CurSD[1] = SideDefs[CurLineDef.sidedef2];
   
   // Show LineDef info
   wsprintf (str, "%d", CurLineDef.sidedef1);
   pSDEdit[0]->SetText (str);
   
   wsprintf (str, "%d", CurLineDef.sidedef2);
   pSDEdit[1]->SetText (str);

   wsprintf (str, "%d", CurLineDef.start);
   pVertex1Edit->SetText (str);
   
   wsprintf (str, "%d", CurLineDef.end);
   pVertex2Edit->SetText (str);

   // Setup flags checkboxes
   int flags = CurLineDef.blak_flags;
   pPassPosCheck->SetCheck   (flags & BF_POS_PASSABLE ? BF_CHECKED : BF_UNCHECKED);
   pPassNegCheck->SetCheck   (flags & BF_NEG_PASSABLE ? BF_CHECKED : BF_UNCHECKED);
   pTransPosCheck->SetCheck  (flags & BF_POS_TRANSPARENT ? BF_CHECKED : BF_UNCHECKED);
   pTransNegCheck->SetCheck  (flags & BF_NEG_TRANSPARENT ? BF_CHECKED : BF_UNCHECKED);
   pFlipPosCheck->SetCheck   (flags & BF_POS_BACKWARDS ? BF_CHECKED : BF_UNCHECKED);
   pFlipNegCheck->SetCheck   (flags & BF_NEG_BACKWARDS ? BF_CHECKED : BF_UNCHECKED);
   pNoMapCheck->SetCheck     (flags & BF_MAP_NEVER ? BF_CHECKED : BF_UNCHECKED);
   pMapStartCheck->SetCheck  (flags & BF_MAP_ALWAYS ? BF_CHECKED : BF_UNCHECKED);
   pPosNoLookThroughCheck->SetCheck (flags & BF_POS_NOLOOKTHROUGH ? BF_CHECKED : BF_UNCHECKED);
   pNegNoLookThroughCheck->SetCheck (flags & BF_NEG_NOLOOKTHROUGH ? BF_CHECKED : BF_UNCHECKED);
   pPosAboveCheck->SetCheck (flags & BF_POS_ABOVE_BUP ? BF_CHECKED : BF_UNCHECKED);
   pNegAboveCheck->SetCheck (flags & BF_NEG_ABOVE_BUP ? BF_CHECKED : BF_UNCHECKED);
   pPosNormalCheck->SetCheck (flags & BF_POS_NORMAL_TDOWN ? BF_CHECKED : BF_UNCHECKED);
   pNegNormalCheck->SetCheck (flags & BF_NEG_NORMAL_TDOWN ? BF_CHECKED : BF_UNCHECKED);
   pPosBelowCheck->SetCheck (flags & BF_POS_BELOW_TDOWN ? BF_CHECKED : BF_UNCHECKED);
   pNegBelowCheck->SetCheck (flags & BF_NEG_BELOW_TDOWN ? BF_CHECKED : BF_UNCHECKED);
   pPosNoVTile->SetCheck    (flags & BF_POS_NO_VTILE ? BF_CHECKED : BF_UNCHECKED);
   pNegNoVTile->SetCheck    (flags & BF_NEG_NO_VTILE ? BF_CHECKED : BF_UNCHECKED);

   switch (WallScrollPosDirection(flags))
   {
   case SCROLL_N:
      pScrollNRadio[0]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_NE:
      pScrollNERadio[0]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_E:
      pScrollERadio[0]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_SE:
      pScrollSERadio[0]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_S:
      pScrollSRadio[0]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_SW:
      pScrollSWRadio[0]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_W:
      pScrollWRadio[0]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_NW:
      pScrollNWRadio[0]->SetCheck(BF_CHECKED);
      break;
   }

   switch (WallScrollPosSpeed(flags))
   {
   case SCROLL_NONE:
      pScrollNoneRadio[0]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_SLOW:
      pScrollSlowRadio[0]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_MEDIUM:
      pScrollMediumRadio[0]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_FAST:
      pScrollFastRadio[0]->SetCheck(BF_CHECKED);
      break;
   }

   switch (WallScrollNegDirection(flags))
   {
   case SCROLL_N:
      pScrollNRadio[1]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_NE:
      pScrollNERadio[1]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_E:
      pScrollERadio[1]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_SE:
      pScrollSERadio[1]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_S:
      pScrollSRadio[1]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_SW:
      pScrollSWRadio[1]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_W:
      pScrollWRadio[1]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_NW:
      pScrollNWRadio[1]->SetCheck(BF_CHECKED);
      break;
   }

   switch (WallScrollNegSpeed(flags))
   {
   case SCROLL_NONE:
      pScrollNoneRadio[1]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_SLOW:
      pScrollSlowRadio[1]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_MEDIUM:
      pScrollMediumRadio[1]->SetCheck(BF_CHECKED);
      break;
   case SCROLL_FAST:
      pScrollFastRadio[1]->SetCheck(BF_CHECKED);
      break;
   }

}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//   Setup the the SideDef 'sdnum' controls. The temporary struct
//   CurSD[sdnum] must be initialized before calling this function.
void TLineDefEditDialog::SetSideDef (int sdnum)
{
   assert (sdnum == 0  ||  sdnum == 1);
   assert ( (sdnum == 0 && CurLineDef.sidedef1 < NumSideDefs) ||
	   (sdnum == 1 && CurLineDef.sidedef2 < NumSideDefs) );
   BOOL EnableSD = TRUE;	// Enable Control's ?
   
   if ( (sdnum == 0 && CurLineDef.sidedef1 < 0) ||
       (sdnum == 1 && CurLineDef.sidedef2 < 0) )
      EnableSD = FALSE;
   
   // Disable controls if sidedef < 0, or Enable them
   pSDXPosEdit[sdnum]->EnableWindow (EnableSD);
   pSDYPosEdit[sdnum]->EnableWindow (EnableSD);
   pSDSectorEdit[sdnum]->EnableWindow (EnableSD);
   pSDAboveStatic[sdnum]->EnableWindow (EnableSD);
   pSDNormalStatic[sdnum]->EnableWindow (EnableSD);
   pSDBelowStatic[sdnum]->EnableWindow (EnableSD);
   pIDNumEdit[sdnum]->EnableWindow (EnableSD);
   pSpeedEdit[sdnum]->EnableWindow (EnableSD);
   
   pSDToAbove[sdnum]->EnableWindow (EnableSD);
   pSDToBelow[sdnum]->EnableWindow (EnableSD);
   pSDToNormal[sdnum]->EnableWindow (EnableSD);
   pSDClearAbove[sdnum]->EnableWindow (EnableSD);
   pSDClearBelow[sdnum]->EnableWindow (EnableSD);
   pSDClearNormal[sdnum]->EnableWindow (EnableSD);
   
   // Set empty values if sidedef < 0
   if ( (sdnum == 0 && CurLineDef.sidedef1 < 0) ||
       (sdnum == 1 && CurLineDef.sidedef2 < 0) )
   {
      pSDXPosEdit[sdnum]->SetText ("0");
      pSDYPosEdit[sdnum]->SetText ("0");
      pSDSectorEdit[sdnum]->SetText ("-1");
      pSDAboveStatic[sdnum]->SetText ("-");
      pSDNormalStatic[sdnum]->SetText ("-");
      pSDBelowStatic[sdnum]->SetText ("-");
      pIDNumEdit[sdnum]->SetText ("0");
      pSpeedEdit[sdnum]->SetText ("0");
      return;
   }
   
   // Setup SideDef controls
   char str[MAX_BITMAPNAME + 1];
   wsprintf (str, "%d", CurSD[sdnum].xoff);
   pSDXPosEdit[sdnum]->SetText(str);
   
   wsprintf (str, "%d", CurSD[sdnum].yoff);
   pSDYPosEdit[sdnum]->SetText(str);
   
   wsprintf (str, "%d", CurSD[sdnum].sector);
   pSDSectorEdit[sdnum]->SetText(str);

   wsprintf (str, "%d", CurSD[sdnum].user_id);
   pIDNumEdit[sdnum]->SetText (str);

   wsprintf (str, "%d", CurSD[sdnum].animate_speed);
   pSpeedEdit[sdnum]->SetText(str);
   
   str[MAX_BITMAPNAME] = '\0';
   strncpy (str, CurSD[sdnum].tex1, MAX_BITMAPNAME);
   pSDAboveStatic[sdnum]->SetText (str);
   
   strncpy (str, CurSD[sdnum].tex2, MAX_BITMAPNAME);
   pSDBelowStatic[sdnum]->SetText (str);
   
   strncpy (str, CurSD[sdnum].tex3, MAX_BITMAPNAME);
	pSDNormalStatic[sdnum]->SetText (str);

}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//  Set LineDef set list to DOORS
void TLineDefEditDialog::DoorsClicked ()
{
   LineDefSet = LD_DOORS;
   SetLineDefList();
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//  Set LineDef set list to ...
void TLineDefEditDialog::CeilingsClicked ()
{
   LineDefSet = LD_CEILINGS;
   SetLineDefList();
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//  Set LineDef set list to ...
void TLineDefEditDialog::RaiseFloorsClicked ()
{
   LineDefSet = LD_RAISEFLOORS;
   SetLineDefList();
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//  Set LineDef set list to ...
void TLineDefEditDialog::LowerFloorsClicked ()
{
   LineDefSet = LD_LOWERFLOORS;
   SetLineDefList();
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//  Set LineDef set list to ...
void TLineDefEditDialog::MovingThingsClicked ()
{
   LineDefSet = LD_MOVINGTHINGS;
   SetLineDefList();
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//  Set LineDef set list to ...
void TLineDefEditDialog::SpecialClicked ()
{
   LineDefSet = LD_SPECIAL;
   SetLineDefList();
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
// 	Set the LineDef type to the selected type
void TLineDefEditDialog::LineDefSelChange ()
{
   if ( LineDefSet < 0  ||  LineDefSet >= NB_LINEDEFSET )
      return;
   
   // Get selected item index
   int SelIndex = pLineDefList->GetSelIndex ();
   if ( SelIndex < 0 )
      return;
   
   CurLineDef.type = GetLineDefTab(LineDefSet)[SelIndex];
   ConfirmData.pTypeCheck = TRUE;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD1ClearAboveClicked ()
{
   pSDAboveStatic[0]->SetText("-");
   ConfirmData.pSDCheck[0] = TRUE;
   ConfirmData.pSDAboveCheck[0] = TRUE;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD1ClearBelowClicked ()
{
   pSDBelowStatic[0]->SetText("-");
   ConfirmData.pSDCheck[0] = TRUE;
   ConfirmData.pSDBelowCheck[0] = TRUE;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD1ClearNormalClicked ()
{
   pSDNormalStatic[0]->SetText("-");
   ConfirmData.pSDCheck[0] = TRUE;
   ConfirmData.pSDNormalCheck[0] = TRUE;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD1ToAboveClicked ()
{
   if ( TextureName[0] == '\0' )
      return;

   pSDAboveStatic[0]->SetText (TextureName);
   ConfirmData.pSDCheck[0] = TRUE;
   ConfirmData.pSDAboveCheck[0] = TRUE;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD1ToBelowClicked ()
{
   if ( TextureName[0] == '\0' )
      return;
   
   pSDBelowStatic[0]->SetText (TextureName);
   ConfirmData.pSDCheck[0] = TRUE;
   ConfirmData.pSDBelowCheck[0] = TRUE;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD1ToNormalClicked ()
{
   if ( TextureName[0] == '\0' )
      return;
   
   pSDNormalStatic[0]->SetText (TextureName);
   ConfirmData.pSDCheck[0] = TRUE;
   ConfirmData.pSDNormalCheck[0] = TRUE;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD2ClearAboveClicked ()
{
   pSDAboveStatic[1]->SetText("-");
   ConfirmData.pSDCheck[1] = TRUE;
   ConfirmData.pSDAboveCheck[1] = TRUE;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD2ClearBelowClicked ()
{
   pSDBelowStatic[1]->SetText("-");
   ConfirmData.pSDCheck[1] = TRUE;
   ConfirmData.pSDBelowCheck[1] = TRUE;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD2ClearNormalClicked ()
{
   pSDNormalStatic[1]->SetText("-");
   ConfirmData.pSDCheck[1] = TRUE;
   ConfirmData.pSDNormalCheck[1] = TRUE;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD2ToAboveClicked ()
{
   if ( TextureName[0] == '\0' )
      return;
   
   pSDAboveStatic[1]->SetText (TextureName);
   ConfirmData.pSDCheck[1] = TRUE;
   ConfirmData.pSDAboveCheck[1] = TRUE;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD2ToBelowClicked ()
{
   if ( TextureName[0] == '\0' )
      return;
   
   pSDBelowStatic[1]->SetText (TextureName);
   ConfirmData.pSDCheck[1] = TRUE;
   ConfirmData.pSDBelowCheck[1] = TRUE;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD2ToNormalClicked ()
{
   if ( TextureName[0] == '\0' )
      return;
   
   pSDNormalStatic[1]->SetText (TextureName);
   ConfirmData.pSDCheck[1] = TRUE;
   ConfirmData.pSDNormalCheck[1] = TRUE;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD1Killfocus ()
{
   SDKillFocus(0);
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD2Killfocus ()
{
   SDKillFocus(1);
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SDKillFocus (int sdnum)
{
   assert (sdnum == 0 || sdnum == 1);
   char num[8];
   SHORT sd;
   
#if 0
   // TODO: Why this doesn't work ?
   if ( ! pSDEdit[sdnum]->IsModified() )
      return;
#endif
   
   if ( ! pSDEdit[sdnum]->IsValid(FALSE) )
      return;
   
   pSDEdit[sdnum]->GetText (num, 8);
   sd = (SHORT)atoi (num);
   
   // Ignore if sidedef number was not changed
   if ( ( sdnum == 0 && sd == CurLineDef.sidedef1 ) ||
       ( sdnum == 1 && sd == CurLineDef.sidedef2 ) )
   {
      return;
   }
   
   // Clear confirmation data, except sidedef number
   ConfirmData.pSDCheck[sdnum] = TRUE;
   ConfirmData.pSDAboveCheck[sdnum]  = FALSE;
   ConfirmData.pSDNormalCheck[sdnum] = FALSE;
   ConfirmData.pSDBelowCheck[sdnum]  = FALSE;
   ConfirmData.pSDSectorCheck[sdnum] = FALSE;
   ConfirmData.pSDXOfsCheck[sdnum]   = FALSE;
   ConfirmData.pSDYOfsCheck[sdnum]   = FALSE;
   ConfirmData.pIDNumCheck[sdnum]    = FALSE;
   ConfirmData.pSpeedCheck[sdnum]    = FALSE;
   
   // Check SideDef number is valid
   if ( sd != -1 )
   {
      // Check SideDef numbers are different
      if ( (sdnum == 0  &&  sd == CurLineDef.sidedef2) ||
	  (sdnum == 1  &&  sd == CurLineDef.sidedef1) )
      {
	 MessageBox ("The two SideDef numbers MUST be different !",
		     "Error",
		     MB_OK | MB_ICONEXCLAMATION);
	 sd = -1;
	 pSDEdit[sdnum]->SetText ("-1");
      }
      else
	 CurSD[sdnum] = SideDefs [sd];
   }
   
   // Copy SideDef number and setup controls
   if ( sdnum == 0 )		CurLineDef.sidedef1 = sd;
   else					CurLineDef.sidedef2 = sd;
   
   SetSideDef (sdnum);
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD1NewClicked ()
{
   SDNew (0);
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::SD2NewClicked ()
{
   SDNew (1);
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//  A new SideDef is created
void TLineDefEditDialog::SDNew (int sdnum)
{
   assert (sdnum == 0 || sdnum == 1);
   char str[10];
   SHORT sd;
   
   // Create new SideDef
   InsertObject (OBJ_SIDEDEFS, -1, 0, 0);
   
   // Fill confirmation data
   ConfirmData.pSDCheck[sdnum] = TRUE;
   ConfirmData.pSDAboveCheck[sdnum]  = FALSE;
   ConfirmData.pSDNormalCheck[sdnum] = FALSE;
   ConfirmData.pSDBelowCheck[sdnum]  = FALSE;
   ConfirmData.pSDSectorCheck[sdnum] = FALSE;
   ConfirmData.pSDXOfsCheck[sdnum]   = FALSE;
   ConfirmData.pSDYOfsCheck[sdnum]   = FALSE;
   ConfirmData.pIDNumCheck[sdnum]    = FALSE;
   ConfirmData.pSpeedCheck[sdnum]    = FALSE;
   
   // Copy SideDef number
   sd = NumSideDefs-1;
   if ( sdnum == 0 ) 	CurLineDef.sidedef1 = sd;
   else			  	CurLineDef.sidedef2 = sd;
   
   // Setup sector number
   SHORT OppSector = GetOppositeSector (SelLineDefs->objnum, sdnum == 0);
   
   // If no opposite sector, choose the last one
   if ( OppSector > 0 )
      OppSector = NumSectors-1;
   SideDefs[sd].sector = OppSector;
   
   // Setup SideDef number control
   wsprintf (str, "%d", sd);
   pSDEdit[sdnum]->SetText (str);
   
   // Copy SideDef to temporary SideDef struct.
   CurSD[sdnum] = SideDefs[sd];
   
   // Replace validators for SideDef number
   pSDEdit[0]->SetValidator (new TRangeValidator (-1, NumSideDefs-1));
   pSDEdit[1]->SetValidator (new TRangeValidator (-1, NumSideDefs-1));
   
   // Setup SideDef controls
   SetSideDef (sdnum);
}

//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
// 	Set TextureName to the new selected texture.
void TLineDefEditDialog::TextureSelchange ()
{
   char texname[100];
   
   // Get selected texture name
   if ( pTextureList->GetSelString (texname, 99) <= 1 )
   {
      TextureName[0] = '\0';
      return;
   }
   
   // Don't go further than the '\t' char
   for (char *pstr = texname ; *pstr ; pstr++)
   {
      if ( *pstr == '\t' )
      {
	 *pstr = '\0';
	 break;
      }
   }
   
   // Don't do anything if no real change
   if ( strcmp (texname, TextureName) == 0 )
      return;
   
   strcpy (TextureName, texname);
   
   // If texture view dialog box opened, change selection
   if ( pWTextureDialog != NULL && pWTextureDialog->IsWindow() )
   {
      TextureListDBLClick();
   }
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::TextureListDBLClick ()
{
   // Don't select empty texture !
   if ( TextureName[0] == '\0' || strcmp (TextureName, "-") == 0 )
      return;
   
   // Create modeless dialog box
   if ( pWTextureDialog == NULL || pWTextureDialog->IsWindow() == FALSE )
   {
      delete pWTextureDialog;
      pWTextureDialog = new TDisplayWallTextureDialog (Parent);
      pWTextureDialog->Create();
   }
   
   if ( pWTextureDialog->IsWindow() )
   {
      TextureInfo *info = FindTextureByName(TextureName);

      if ( pWTextureDialog->SelectBitmap2 (info->filename) < 0 )
	 Notify ("Error: Cannot select the texture name \"%s\" in the "
		 "dialog box of Wall Texture view ! (BUG)", TextureName);
   }
   else
      Notify ("Error: Cannot create dialog box of Wall Texture view !");
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::EvLButtonDown (UINT modKeys, const TPoint& point)
{
   TDialog::EvLButtonDown(modKeys, point);
   
   // Retreive object for handle
   TStatic *pStatic = GetPointedStatic ((TPoint&)point);
   if ( pStatic == NULL )
      return;
   
   // If floor or ceiling static, get texture name
   char texname[MAX_BITMAPNAME + 2];
   pStatic->GetText (texname, MAX_BITMAPNAME + 1);
   if ( texname[0] != '\0' && strcmp (texname, "-") != 0 )
   {
      strcat(texname,"\t");
      pTextureList->SetSelString(texname, -1);
      texname[strlen(texname)] = '\0';
      TextureSelchange();  	// Be sure to change selection
   }
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
void TLineDefEditDialog::EvLButtonDblClk (UINT modKeys, const TPoint& point)
{
   TDialog::EvLButtonDblClk(modKeys, point);
   
   // Retreive object for handle
   TStatic *pStatic = GetPointedStatic ((TPoint&)point);
   if ( pStatic == NULL )
      return;
   
   // If floor or ceiling static, get texture name
   char texname[MAX_BITMAPNAME + 2];
   pStatic->GetText (texname, MAX_BITMAPNAME + 1);
   if ( texname[0] != '\0' && strcmp (texname, "-") != 0 )
   {
      strcat(texname,"\t");
      pTextureList->SetSelString(texname, -1);
      texname[strlen(texname)] = '\0';
      TextureSelchange();  		// Be sure to change selection
      TextureListDBLClick();		// Force to open Dialog Box
   }
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//
TStatic *TLineDefEditDialog::GetPointedStatic (TPoint& point)
{
   // Retreive object for handle
   TStatic *pStatic = NULL;
   
   if ( IsPointInDlgItem (IDC_SD1_ABOVE, point)  ||
       IsPointInDlgItem (IDC_SD1_ABOVE_FRAME, point) )
   {
      pStatic = pSDAboveStatic[0];
   }
   
   else if ( IsPointInDlgItem (IDC_SD1_NORMAL, point)  ||
	    IsPointInDlgItem (IDC_SD1_NORMAL_FRAME, point) )
   {
      pStatic = pSDNormalStatic[0];
   }
   
   else if ( IsPointInDlgItem (IDC_SD1_BELOW, point)  ||
	    IsPointInDlgItem (IDC_SD1_BELOW_FRAME, point) )
   {
      pStatic = pSDBelowStatic[0];
   }
   
   else if ( IsPointInDlgItem (IDC_SD2_ABOVE, point)  ||
	    IsPointInDlgItem (IDC_SD2_ABOVE_FRAME, point) )
   {
      pStatic = pSDAboveStatic[1];
   }
   
   else if ( IsPointInDlgItem (IDC_SD2_NORMAL, point)  ||
	    IsPointInDlgItem (IDC_SD2_NORMAL_FRAME, point) )
   {
		pStatic = pSDNormalStatic[1];
   }
   
   else if ( IsPointInDlgItem (IDC_SD2_BELOW, point)  ||
	    IsPointInDlgItem (IDC_SD2_BELOW_FRAME, point) )
   {
      pStatic = pSDBelowStatic[1];
   }
   
   return pStatic;
}


//////////////////////////////////////////////////////////////////////
// TLineDefEditDialog
// ------------------
//   Look if the dialog control 'resid' window contains the point
//   'clientPoint', which is a dialog client coord. of the point
BOOL TLineDefEditDialog::IsPointInDlgItem (int itemId, TPoint &clientPoint)
{
   HWND hWnd = GetDlgItem (itemId);
   if ( hWnd == (HWND)NULL )
      return FALSE;
   
   TWindow wnd(hWnd);
   TRect wRect;
   wnd.GetWindowRect (wRect);
   TPoint TopLeft (wRect.left, wRect.top);
   TPoint BotRight(wRect.right, wRect.bottom);
   ScreenToClient (TopLeft);
   ScreenToClient (BotRight);
   TRect cRect (TopLeft, BotRight);
   
   return cRect.Contains (clientPoint);
}


void TLineDefEditDialog::PassPosClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pPassPosCheck = TRUE;
}


void TLineDefEditDialog::PassNegClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pPassNegCheck = TRUE;
}


void TLineDefEditDialog::TransPosClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pTransPosCheck = TRUE;
}


void TLineDefEditDialog::TransNegClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pTransNegCheck = TRUE;
}


void TLineDefEditDialog::FlipPosClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pFlipPosCheck = TRUE;
}


void TLineDefEditDialog::FlipNegClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pFlipNegCheck = TRUE;
}


void TLineDefEditDialog::NoMapClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pNoMapCheck = TRUE;
}


void TLineDefEditDialog::MapStartClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pMapStartCheck = TRUE;
}

void TLineDefEditDialog::NoLookThroughPosClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pNoLookThroughPosCheck = TRUE;
}

void TLineDefEditDialog::NoLookThroughNegClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pNoLookThroughNegCheck = TRUE;
}

void TLineDefEditDialog::AbovePosClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pAbovePosCheck = TRUE;
}

void TLineDefEditDialog::AboveNegClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pAboveNegCheck = TRUE;
}

void TLineDefEditDialog::NormalPosClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pNormalPosCheck = TRUE;
}

void TLineDefEditDialog::NormalNegClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pNormalNegCheck = TRUE;
}

void TLineDefEditDialog::BelowPosClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pBelowPosCheck = TRUE;
}

void TLineDefEditDialog::BelowNegClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pBelowNegCheck = TRUE;
}

void TLineDefEditDialog::NoVTilePosClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pNoVTilePosCheck = TRUE;
}

void TLineDefEditDialog::NoVTileNegClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pNoVTileNegCheck = TRUE;
}

void TLineDefEditDialog::ScrollPosClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pScrollCheck[0] = TRUE;
}

void TLineDefEditDialog::ScrollNegClicked ()
{
   ConfirmData.pFlagsCheck = TRUE;
   ConfirmData.pScrollCheck[1] = TRUE;
}
