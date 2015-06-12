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

	FILE:         seditdlg.cpp

	OVERVIEW
	========
	Source file for implementation of TSectorEditDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#ifndef __seditdlg_h
	#include "seditdlg.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_LISTBOX_H
	#include <owl\listbox.h>
#endif

#ifndef OWL_EDIT_H
	#include <owl\edit.h>
#endif

#ifndef OWL_STATIC_H
	#include <owl\static.h>
#endif

#ifndef __cnfsedlg_h
	#include "cnfsedlg.h"
#endif

#ifndef __levels_h
	#include "levels.h"
#endif

#ifndef __names_h
	#include "names.h"
#endif

#ifndef __newers_h
	#include "newers.h"
#endif

#ifndef __viewbmp_h
	#include "viewbmp.h"
#endif

#include "bsp.h"

// HELP Ids
#include "windeuhl.h"


const SHORT D_SectorTypeTab[] =
{
	0, 1, 2, 3, 5, 4, 16, 7,
	8, 9, 10, 11, 12, 13, 14,
	-1
};
#define NB_D_SECTOR	(sizeof(D_SectorTypeTab) / sizeof(SHORT))

const SHORT D2_SectorTypeTab[] =
{
	0, 1, 2, 3, 5, 4, 16, 7,
	8, 9, 10, 11, 12, 13, 14, 17,
	-1
};
#define NB_D2_SECTOR	(sizeof(D2_SectorTypeTab) / sizeof(SHORT))

const SHORT H_SectorTypeTab[] =
{
	0, 1, 2, 3, 5, 4, 16, 6, 7,
	8, 9, 10, 12, 13, 14,
	21, 22, 23, 26, 27, 28, 31,
	32, 33, 36, 37, 38, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51,
	-1
};

#define NB_H_SECTOR	(sizeof(H_SectorTypeTab) / sizeof(SHORT))

SHORT GetSectorType(int i)
{
	if ( DoomVersion == 16 )
	{
		assert (i >= 0 && i < NB_H_SECTOR);
		return H_SectorTypeTab[i];
	}
	else if ( DoomVersion == 2 )
	{
		assert (i >= 0 && i < NB_D2_SECTOR);
		return D2_SectorTypeTab[i];
	}
	else
	{
		assert (i >= 0 && i < NB_D_SECTOR);
		return D_SectorTypeTab[i];
	}
}


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TSectorEditDialog, TDialog)
//{{TSectorEditDialogRSP_TBL_BEGIN}}
	EV_BN_CLICKED(IDC_TO_FLOOR, ToFloorClicked),
	EV_BN_CLICKED(IDC_TO_CEILING, ToCeilingClicked),
	EV_BN_CLICKED(IDC_FLOOR_CLEAR, FloorClearClicked),
	EV_BN_CLICKED(IDC_CEILING_CLEAR, CeilingClearClicked),
	EV_BN_CLICKED(IDC_DEPTHNONE, DepthClicked),
	EV_BN_CLICKED(IDC_DEPTHSHALLOW, DepthClicked),
	EV_BN_CLICKED(IDC_DEPTHDEEP, DepthClicked),
	EV_BN_CLICKED(IDC_DEPTHVERYDEEP, DepthClicked),
	EV_BN_CLICKED(IDC_SCROLLN, ScrollClicked),
	EV_BN_CLICKED(IDC_SCROLLNE, ScrollClicked),
	EV_BN_CLICKED(IDC_SCROLLE, ScrollClicked),
	EV_BN_CLICKED(IDC_SCROLLSE, ScrollClicked),
	EV_BN_CLICKED(IDC_SCROLLS, ScrollClicked),
	EV_BN_CLICKED(IDC_SCROLLSW, ScrollClicked),
	EV_BN_CLICKED(IDC_SCROLLW, ScrollClicked),
	EV_BN_CLICKED(IDC_SCROLLNW, ScrollClicked),
	EV_BN_CLICKED(IDC_SCROLLFLOOR, ScrollClicked),
	EV_BN_CLICKED(IDC_SCROLLCEILING, ScrollClicked),
	EV_BN_CLICKED(IDC_SCROLLNONE, ScrollClicked),
	EV_BN_CLICKED(IDC_SCROLLSLOW, ScrollClicked),
	EV_BN_CLICKED(IDC_SCROLLMEDIUM, ScrollClicked),
	EV_BN_CLICKED(IDC_SCROLLFAST, ScrollClicked),
	EV_BN_CLICKED(IDC_FLICKER, ScrollClicked),
//	EV_LBN_SELCHANGE(IDC_SECTOR_LIST, SectorSelChange),
	EV_LBN_SELCHANGE(IDC_FTEXTURE_LIST, TextureSelChange),
	EV_LBN_DBLCLK(IDC_FTEXTURE_LIST, TextureDblclick),
//	EV_BN_CLICKED(IDC_NEW_TAG, NewTagClicked),
	EV_BN_CLICKED(IDOK, CmOk),
	EV_WM_LBUTTONDOWN,
	EV_WM_LBUTTONDBLCLK,
//{{TSectorEditDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TSectorEditDialog Implementation}}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
TSectorEditDialog::TSectorEditDialog (TWindow* parent, SelPtr sel,
									  TResId resId, TModule* module):
	TDialog(parent, resId, module)
{
	assert (sel != NULL);
	assert (sel->objnum >= 0 && sel->objnum < NumSectors);

	SelSectors = sel;
	CurSector =Sectors[sel->objnum];
	TextureName[0] = '\0';
	memset(&ConfirmData, 0, sizeof(ConfirmData));

	// Crete object for texture view, but do not create dialog box.
	pFTextureDialog = NULL;

	// Create objects for controls
	pNoAmbientCheck    = newTCheckBox(this, IDC_NOAMBIENT, 0);
//	pTagEdit           = newTEdit(this, IDC_TAG_EDIT, 6);
//	pSectorList        = newTListBox(this, IDC_SECTOR_LIST);
	pLightEdit         = newTEdit(this, IDC_LIGHT_EDIT, 4);
	pTextureXEdit      = newTEdit(this, IDC_TEXTURE_XOFFSET, 4);
	pTextureYEdit      = newTEdit(this, IDC_TEXTURE_YOFFSET, 4);
	pTextureList       = newTListBox(this, IDC_FTEXTURE_LIST);
	pFloorStatic       = newTStatic(this, IDC_FLOOR_TEXT, MAX_BITMAPNAME + 1);
	pFloorHeightEdit   = newTEdit(this, IDC_FLOOR_HEIGHT, 6);
	pFloorStyle        = newTStatic(this, IDC_FLOORSTYLE);
	pCeilingStatic     = newTStatic(this, IDC_CEILING_TEXT, MAX_BITMAPNAME + 1);
	pCeilingHeightEdit = newTEdit(this, IDC_CEILING_HEIGHT, 6);
	pCeilingStyle      = newTStatic(this, IDC_CEILINGSTYLE);
	pIDNumEdit         = newTEdit(this, IDC_ID, 6);
	pSpeedEdit         = newTEdit(this, IDC_SPEED, 6);
	pDepth0Radio       = newTRadioButton(this, IDC_DEPTHNONE, 0);
	pDepth1Radio       = newTRadioButton(this, IDC_DEPTHSHALLOW, 0);
	pDepth2Radio       = newTRadioButton(this, IDC_DEPTHDEEP, 0);
	pDepth3Radio       = newTRadioButton(this, IDC_DEPTHVERYDEEP, 0);
	pScrollNRadio      = newTRadioButton(this, IDC_SCROLLN, 0);
	pScrollNERadio     = newTRadioButton(this, IDC_SCROLLNE, 0);
	pScrollERadio      = newTRadioButton(this, IDC_SCROLLE, 0);
	pScrollSERadio     = newTRadioButton(this, IDC_SCROLLSE, 0);
	pScrollSRadio      = newTRadioButton(this, IDC_SCROLLS, 0);
	pScrollSWRadio     = newTRadioButton(this, IDC_SCROLLSW, 0);
	pScrollWRadio      = newTRadioButton(this, IDC_SCROLLW, 0);
	pScrollNWRadio     = newTRadioButton(this, IDC_SCROLLNW, 0);
	pScrollFloorCheck  = newTCheckBox(this, IDC_SCROLLFLOOR, 0);
	pScrollCeilingCheck= newTCheckBox(this, IDC_SCROLLCEILING, 0);
	pScrollNoneRadio   = newTRadioButton(this, IDC_SCROLLNONE, 0);
	pScrollSlowRadio   = newTRadioButton(this, IDC_SCROLLSLOW, 0);
	pScrollMediumRadio = newTRadioButton(this, IDC_SCROLLMEDIUM, 0);
	pScrollFastRadio   = newTRadioButton(this, IDC_SCROLLFAST, 0);
	pFlickerCheck      = newTCheckBox(this, IDC_FLICKER, 0);
	pSlopeFloorVertex[0]  = newTEdit(this, IDC_FLOORV1, 6);
	pSlopeFloorVertex[1]  = newTEdit(this, IDC_FLOORV2, 6);
	pSlopeFloorVertex[2]  = newTEdit(this, IDC_FLOORV3, 6);
	pSlopeFloorHeight[0]  = newTEdit(this, IDC_FLOORH1, 6);
	pSlopeFloorHeight[1]  = newTEdit(this, IDC_FLOORH2, 6);
	pSlopeFloorHeight[2]  = newTEdit(this, IDC_FLOORH3, 6);
	pSlopeCeilingVertex[0]  = newTEdit(this, IDC_CEILINGV1, 6);
	pSlopeCeilingVertex[1]  = newTEdit(this, IDC_CEILINGV2, 6);
	pSlopeCeilingVertex[2]  = newTEdit(this, IDC_CEILINGV3, 6);
	pSlopeCeilingHeight[0]  = newTEdit(this, IDC_CEILINGH1, 6);
	pSlopeCeilingHeight[1]  = newTEdit(this, IDC_CEILINGH2, 6);
	pSlopeCeilingHeight[2]  = newTEdit(this, IDC_CEILINGH3, 6);
	pFloorAngle    = newTEdit(this, IDC_FLOOR_ANGLE, 6);
	pCeilingAngle  = newTEdit(this, IDC_CEILING_ANGLE, 6);
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
TSectorEditDialog::~TSectorEditDialog ()
{
	Destroy();
	delete pFTextureDialog;
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::SetupWindow ()
{
	TDialog::SetupWindow();
	::CenterWindow (this);

	SetTextureList();
//	SetSectorList();
	SetSector();

	// Setup validators
	pLightEdit->SetValidator (new TRangeValidator (0, 255));
	pTextureXEdit->SetValidator (new TRangeValidator (0, FINENESS));
	pTextureYEdit->SetValidator (new TRangeValidator (0, FINENESS));
//	pTagEdit->SetValidator (new TRangeValidator (0, SHRT_MAX));
	pFloorHeightEdit->SetValidator (new TRangeValidator (-16384, 16383));
	pCeilingHeightEdit->SetValidator (new TRangeValidator (-16384, 16383));
	pIDNumEdit->SetValidator (new TRangeValidator (0, 65535));
	pSpeedEdit->SetValidator (new TRangeValidator (0, 255));
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::SetSectorList ()
{
	assert (pSectorList->IsWindow());
	int Special;
	int TabStops[] = { 5*4 };

	pSectorList->ClearList();
	pSectorList->SetTabStops (1, TabStops);

	for ( int i = 0 ;
		  (Special = GetSectorType(i)) != -1;
		  i++)
	{
		char str[128];
		char sname[128];

		strcpy (sname, GetSectorTypeLongName (Special));
		wsprintf (str, "[%d]\t%s", Special, sname);
		pSectorList->AddString (str);

		if ( CurSector.special == Special )
			pSectorList->SetSelString (str, -1);
	}
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::SetTextureList ()
{
	assert (pTextureList->IsWindow());

	pTextureList->ClearList();

	// Add texture1 and texture 2 names
	for (SHORT i = 0 ; i < NumFTexture ; i++)
	{
		assert (FTexture[i] != NULL);
		pTextureList->AddString (FTexture[i]->Name);
	}
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::SetSector()
{
   char str[MAX_BITMAPNAME+2];
   int i;
   
   // Show Sector info
//   wsprintf (str, "%d", CurSector.tag);
//   pTagEdit->SetText (str);

   if (CurSector.light < 128)
   {
      wsprintf (str, "%d", CurSector.light * 2);
      pLightEdit->SetText (str);
      pNoAmbientCheck->SetCheck(BF_CHECKED);
   }
   else
   {
      wsprintf (str, "%d", (CurSector.light - 128) * 2);
      pLightEdit->SetText (str);
      pNoAmbientCheck->SetCheck(BF_UNCHECKED);
   }

   if (CurSector.blak_flags & SF_FLICKER)
      pFlickerCheck->SetCheck(BF_CHECKED);
   
   wsprintf (str, "%d", CurSector.xoffset);
   pTextureXEdit->SetText (str);

   wsprintf (str, "%d", CurSector.yoffset);
   pTextureYEdit->SetText (str);
   
   wsprintf (str, "%d", CurSector.floorh);
   pFloorHeightEdit->SetText (str);
   
   wsprintf (str, "%d", CurSector.ceilh);
   pCeilingHeightEdit->SetText (str);

   wsprintf (str, "%d", CurSector.user_id);
   pIDNumEdit->SetText (str);

   wsprintf (str, "%d", CurSector.animate_speed);
   pSpeedEdit->SetText (str);
   
   str[MAX_BITMAPNAME] = '\0';
   memcpy (str, CurSector.floort, MAX_BITMAPNAME);
   pFloorStatic->SetText (str);
   
   memcpy (str, CurSector.ceilt, MAX_BITMAPNAME);
   pCeilingStatic->SetText (str);

   // Set depth button
   switch (SectorDepth(CurSector.blak_flags))
   {
   case SF_DEPTH0:
      pDepth0Radio->SetCheck(BF_CHECKED);
      break;
   case SF_DEPTH1:
      pDepth1Radio->SetCheck(BF_CHECKED);
      break;
   case SF_DEPTH2:
      pDepth2Radio->SetCheck(BF_CHECKED);
      break;
   case SF_DEPTH3:
      pDepth3Radio->SetCheck(BF_CHECKED);
      break;
   }

   switch (SectorScrollSpeed(CurSector.blak_flags))
   {
   case SCROLL_NONE:
      pScrollNoneRadio->SetCheck(BF_CHECKED);
      break;
   case SCROLL_SLOW:
      pScrollSlowRadio->SetCheck(BF_CHECKED);
      break;
   case SCROLL_MEDIUM:
      pScrollMediumRadio->SetCheck(BF_CHECKED);
      break;
   case SCROLL_FAST:
      pScrollFastRadio->SetCheck(BF_CHECKED);
      break;
   }

   if (CurSector.blak_flags & SF_SCROLL_FLOOR)
      pScrollFloorCheck->SetCheck(BF_CHECKED);
   if (CurSector.blak_flags & SF_SCROLL_CEILING)
      pScrollCeilingCheck->SetCheck(BF_CHECKED);

   switch (SectorScrollDirection(CurSector.blak_flags))
   {
   case SCROLL_N:
      pScrollNRadio->SetCheck(BF_CHECKED);
      break;
   case SCROLL_NE:
      pScrollNERadio->SetCheck(BF_CHECKED);
      break;
   case SCROLL_E:
      pScrollERadio->SetCheck(BF_CHECKED);
      break;
   case SCROLL_SE:
      pScrollSERadio->SetCheck(BF_CHECKED);
      break;
   case SCROLL_S:
      pScrollSRadio->SetCheck(BF_CHECKED);
      break;
   case SCROLL_SW:
      pScrollSWRadio->SetCheck(BF_CHECKED);
      break;
   case SCROLL_W:
      pScrollWRadio->SetCheck(BF_CHECKED);
      break;
   case SCROLL_NW:
      pScrollNWRadio->SetCheck(BF_CHECKED);
      break;
   }

   // Show slope information
   if (CurSector.blak_flags & SF_SLOPED_FLOOR)
   {
      for (i=0; i < 3; i++)
      {
	 wsprintf (str, "%d", CurSector.floor_slope.points[i].vertex);
	 pSlopeFloorVertex[i]->SetText (str);
	 wsprintf (str, "%d", CurSector.floor_slope.points[i].z);
	 pSlopeFloorHeight[i]->SetText (str);
      }
      wsprintf (str, "%d", CurSector.floor_slope.angle);
      pFloorAngle->SetText (str);
      //pFloorHeightEdit->SetText("");
      pFloorStyle->SetText("(sloped)");
   }
   if (CurSector.blak_flags & SF_SLOPED_CEILING)
   {
      for (i=0; i < 3; i++)
      {
	 wsprintf (str, "%d", CurSector.ceiling_slope.points[i].vertex);
	 pSlopeCeilingVertex[i]->SetText (str);
	 wsprintf (str, "%d", CurSector.ceiling_slope.points[i].z);
	 pSlopeCeilingHeight[i]->SetText (str);
      }
      wsprintf (str, "%d", CurSector.ceiling_slope.angle);
      pCeilingAngle->SetText (str);
      //pCeilingHeightEdit->SetText("");
      pCeilingStyle->SetText("(sloped)");
   }
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
// Return TRUE iff values in dialog are legal.
//
BOOL TSectorEditDialog::GetSector()
{
   char str[MAX_BITMAPNAME + 1];
   SHORT light;
   int flags = 0, i, vertex, z;
   
   // Get Sector info
   
   pLightEdit->GetText (str, 4);
   light = (SHORT) atoi(str) / 2;
   if (pNoAmbientCheck->GetCheck() == BF_UNCHECKED)
      light += 128;

   if ( CurSector.light != light )	ConfirmData.pLightCheck = TRUE;
   CurSector.light = light;

   if (pFlickerCheck->GetCheck() == BF_CHECKED)
      flags |= SF_FLICKER;

   pTextureXEdit->GetText (str, 4);
   if ( CurSector.xoffset != atoi(str) )	ConfirmData.pTextureOffsetCheck = TRUE;
   CurSector.xoffset = atoi (str);

   pTextureYEdit->GetText (str, 4);
   if ( CurSector.yoffset != atoi(str) )	ConfirmData.pTextureOffsetCheck = TRUE;
   CurSector.yoffset = atoi (str);
   
   pFloorHeightEdit->GetText (str, 6);
   if ( CurSector.floorh != (SHORT)atoi(str) )	ConfirmData.pFloorHeightCheck = TRUE;
   CurSector.floorh = (SHORT)atoi (str);
   
   pCeilingHeightEdit->GetText (str, 6);
   if ( CurSector.ceilh != (SHORT)atoi(str) )	ConfirmData.pCeilingHeightCheck = TRUE;
   CurSector.ceilh = (SHORT)atoi (str);
   
   pFloorStatic->GetText (str, MAX_BITMAPNAME+1);
   memset (CurSector.floort, 0, MAX_BITMAPNAME);
   strncpy (CurSector.floort, str, MAX_BITMAPNAME);
   CurSector.floor_type = TextureToNumber(str);
   
   pCeilingStatic->GetText (str, MAX_BITMAPNAME+1);
   memset (CurSector.ceilt, 0, MAX_BITMAPNAME);
   strncpy (CurSector.ceilt, str, MAX_BITMAPNAME);
   CurSector.ceiling_type = TextureToNumber(str);

   pIDNumEdit->GetText (str, 6);
   if ( CurSector.user_id != (SHORT)atoi(str) )	ConfirmData.pIDNumCheck = TRUE;
   CurSector.user_id = (SHORT)atoi (str);

   pSpeedEdit->GetText (str, 6);
   if ( CurSector.animate_speed != (SHORT)atoi(str) )	ConfirmData.pSpeedCheck = TRUE;
   CurSector.animate_speed = (BYTE)atoi (str);

   if (pDepth0Radio->GetCheck() == BF_CHECKED)
      flags |= SF_DEPTH0;
   else if (pDepth1Radio->GetCheck() == BF_CHECKED)
      flags |= SF_DEPTH1;
   else if (pDepth2Radio->GetCheck() == BF_CHECKED)
      flags |= SF_DEPTH2;
   else if (pDepth3Radio->GetCheck() == BF_CHECKED)
      flags |= SF_DEPTH3;

   if (pScrollSlowRadio->GetCheck() == BF_CHECKED)
      flags |= SCROLL_SLOW << 2;
   else if (pScrollMediumRadio->GetCheck() == BF_CHECKED)
      flags |= SCROLL_MEDIUM << 2;
   else if (pScrollFastRadio->GetCheck() == BF_CHECKED)
      flags |= SCROLL_FAST << 2;

   if (pScrollFloorCheck->GetCheck() == BF_CHECKED)
      flags |= SF_SCROLL_FLOOR;
   if (pScrollCeilingCheck->GetCheck() == BF_CHECKED)
      flags |= SF_SCROLL_CEILING;

   if (pScrollNRadio->GetCheck() == BF_CHECKED)
      flags |= SCROLL_N <<4;
   else if (pScrollNERadio->GetCheck() == BF_CHECKED)
      flags |= SCROLL_NE <<4;
   else if (pScrollERadio->GetCheck() == BF_CHECKED)
      flags |= SCROLL_E <<4;
   else if (pScrollSERadio->GetCheck() == BF_CHECKED)
      flags |= SCROLL_SE <<4;
   else if (pScrollSRadio->GetCheck() == BF_CHECKED)
      flags |= SCROLL_S <<4;
   else if (pScrollSWRadio->GetCheck() == BF_CHECKED)
      flags |= SCROLL_SW <<4;
   else if (pScrollWRadio->GetCheck() == BF_CHECKED)
      flags |= SCROLL_W <<4;
   else if (pScrollNWRadio->GetCheck() == BF_CHECKED)
      flags |= SCROLL_NW <<4;

   // Get slope info
   for (i=0; i < 3; i++)
   {
      pSlopeFloorVertex[i]->GetText (str, 6);
      if (str[0] == 0)
	 vertex = -1;
      else vertex = atoi(str);
      if (vertex != CurSector.floor_slope.points[i].vertex)
	 ConfirmData.pSlopeCheck = TRUE;
      CurSector.floor_slope.points[i].vertex = vertex;
      
      pSlopeFloorHeight[i]->GetText (str, 6);
      if (str[0] == 0)
	 z = -1;
      else z = atoi(str);
      if (z != CurSector.floor_slope.points[i].z)
	 ConfirmData.pSlopeCheck = TRUE;
      CurSector.floor_slope.points[i].z = z;
   }   
   for (i=0; i < 3; i++)
   {
      pSlopeCeilingVertex[i]->GetText (str, 6);
      if (str[0] == 0)
	 vertex = -1;
      else vertex = atoi(str);
      if (vertex != CurSector.ceiling_slope.points[i].vertex)
	 ConfirmData.pSlopeCheck = TRUE;
      CurSector.ceiling_slope.points[i].vertex = vertex;

      pSlopeCeilingHeight[i]->GetText (str, 6);
      if (str[0] == 0)
	 z = -1;
      else z = atoi(str);
      if (z != CurSector.ceiling_slope.points[i].z)
	 ConfirmData.pSlopeCheck = TRUE;
      CurSector.ceiling_slope.points[i].z = z;
   }   
   if (CurSector.floor_slope.points[0].vertex == -1)
      flags &= ~SF_SLOPED_FLOOR;
   else flags |= SF_SLOPED_FLOOR;

   if (CurSector.ceiling_slope.points[0].vertex == -1)
      flags &= ~SF_SLOPED_CEILING;
   else flags |= SF_SLOPED_CEILING;

   pFloorAngle->GetText (str, 6);
   if (atoi(str) != CurSector.floor_slope.angle)
      ConfirmData.pSlopeCheck = TRUE;
   CurSector.floor_slope.angle = atoi(str);
   pCeilingAngle->GetText (str, 6);
   if (atoi(str) != CurSector.ceiling_slope.angle)
      ConfirmData.pSlopeCheck = TRUE;
   CurSector.ceiling_slope.angle = atoi(str);

   // Check legality of vertex #s; check for duplicates
   if (flags & SF_SLOPED_FLOOR)
      for (i=0; i < 3; i++)
      {
	 if (CurSector.floor_slope.points[i].vertex < 0 ||
	     CurSector.floor_slope.points[i].vertex >= NumVertexes)
	 {
	    MessageBox("The floor slope contains an illegal vertex number!  This won't work!");
	    return FALSE;
	 }
	 if (CurSector.floor_slope.points[i].vertex == CurSector.floor_slope.points[(i + 1) % 3].vertex)
	 {
	    MessageBox("The floor slope contains duplicate vertex numbers!  This won't work!");
	    return FALSE;
	 }
      }
   
   if (flags & SF_SLOPED_CEILING)
      for (i=0; i < 3; i++)
      {
	 if (CurSector.ceiling_slope.points[i].vertex < 0 ||
	     CurSector.ceiling_slope.points[i].vertex >= NumVertexes)
	 {
	    MessageBox("The ceiling slope contains an illegal vertex number!  This won't work!");
	    return FALSE;
	 }
	 if (CurSector.ceiling_slope.points[i].vertex == CurSector.ceiling_slope.points[(i + 1) % 3].vertex)
	 {
	    MessageBox("The ceiling slope contains duplicate vertex numbers!  This won't work!");
	    return FALSE;
	 }
      }

   CurSector.blak_flags = flags;
   return TRUE;
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::CmOk ()
{
   // Cannot close if edit controls not valid
   if ( ! CanClose() )
      return;
   
   // Save first selected sector info
   if (!GetSector())
      return;
   
   SPtr pSector = &Sectors[SelSectors->objnum];
   
   // Did the user made changes?
   if ( memcmp(pSector, &CurSector, sizeof(Sector)) != 0 )
   {
      MadeChanges = TRUE;
   }
   *pSector = CurSector;
   
   // Copy to the others selected sectors
   if ( SelSectors->next != NULL )
   {
      SET_HELP_CONTEXT(Confirming_copy_of_Sector_attributes);
      if ( TConfirmSectorDialog (this, ConfirmData).Execute() == IDOK )
      {
	 for (SelPtr cur = SelSectors->next ; cur != NULL ; cur = cur->next)
	 {
	    Sector *pSector = &Sectors[cur->objnum];
	    Sector SectorBefore = *pSector;	// Copy before changing
	    
	    if ( ConfirmData.pSpecialCheck )
	       pSector->special = CurSector.special;
	    
	    if ( ConfirmData.pTagCheck )
	       pSector->tag = CurSector.tag;
	    
	    if ( ConfirmData.pLightCheck )
	       pSector->light = CurSector.light;

	    if ( ConfirmData.pTextureOffsetCheck)
	    {
	       pSector->xoffset = CurSector.xoffset;
	       pSector->yoffset = CurSector.yoffset;
	    }
	    
	    if ( ConfirmData.pFloorTextureCheck )
	    {
	       strncpy (pSector->floort, CurSector.floort, MAX_BITMAPNAME);
	       pSector->floor_type = TextureToNumber(CurSector.floort);
	    }
	    
	    if ( ConfirmData.pFloorHeightCheck )
	       pSector->floorh = CurSector.floorh;
	    
	    if ( ConfirmData.pCeilingTextureCheck )
	    {
	       strncpy (pSector->ceilt, CurSector.ceilt, MAX_BITMAPNAME);
	       pSector->ceiling_type = TextureToNumber(CurSector.ceilt);
	    }
	    
	    if ( ConfirmData.pCeilingHeightCheck )
	       pSector->ceilh = CurSector.ceilh;

	    if ( ConfirmData.pIDNumCheck )
	       pSector->user_id = CurSector.user_id;

	    if ( ConfirmData.pSpeedCheck )
	       pSector->animate_speed = CurSector.animate_speed;

	    if ( ConfirmData.pDepthCheck )
	    {
	       pSector->blak_flags &= ~SectorDepth(pSector->blak_flags);
	       pSector->blak_flags |= SectorDepth(CurSector.blak_flags);
	    }

	    if ( ConfirmData.pScrollCheck )
	    {
	       pSector->blak_flags &= ~0x000001FC;
	       pSector->blak_flags |= (SectorScrollSpeed(CurSector.blak_flags) << 2);
	       pSector->blak_flags |= (SectorScrollDirection(CurSector.blak_flags) << 4);
	       pSector->blak_flags |= (CurSector.blak_flags & SF_SCROLL_FLOOR);
	       pSector->blak_flags |= (CurSector.blak_flags & SF_SCROLL_CEILING);
	    }

	    if (ConfirmData.pSlopeCheck)
	    {
	       memcpy(&pSector->floor_slope, &CurSector.floor_slope, sizeof(SlopeInfo));
	       memcpy(&pSector->ceiling_slope, &CurSector.ceiling_slope, sizeof(SlopeInfo));
	       pSector->blak_flags &= ~SF_SLOPED_FLOOR;
	       pSector->blak_flags |= (CurSector.blak_flags & SF_SLOPED_FLOOR);
	       pSector->blak_flags &= ~SF_SLOPED_CEILING;
	       pSector->blak_flags |= (CurSector.blak_flags & SF_SLOPED_CEILING);
	    }
	    
	    // Did we made changes ?
	    if ( memcmp(pSector, &SectorBefore, sizeof(Sector)) != 0 )
	       MadeChanges = TRUE;
	 }
      }
      RESTORE_HELP_CONTEXT();
   }
   
   // Close Dialog box
   TDialog::CmOk();
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::ToFloorClicked ()
{
   if ( TextureName[0] != '\0' )
   {
      pFloorStatic->SetText (TextureName);
      ConfirmData.pFloorTextureCheck = TRUE;
   }
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::ToCeilingClicked ()
{
   if ( TextureName[0] != '\0' )
   {
      pCeilingStatic->SetText (TextureName);
      ConfirmData.pCeilingTextureCheck = TRUE;
   }
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::FloorClearClicked ()
{
	pFloorStatic->SetText ("-");
	ConfirmData.pFloorTextureCheck = TRUE;
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::CeilingClearClicked ()
{
	pCeilingStatic->SetText ("-");
	ConfirmData.pCeilingTextureCheck = TRUE;
}

//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::DepthClicked ()
{
   ConfirmData.pDepthCheck = TRUE;
}

//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::ScrollClicked ()
{
   ConfirmData.pScrollCheck = TRUE;
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::SectorSelChange ()
{
	// Get selected item index
	int SelIndex = pSectorList->GetSelIndex ();
	if ( SelIndex < 0 )
		return;

	CurSector.special = GetSectorType(SelIndex);
	ConfirmData.pSpecialCheck = TRUE;
}



//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::NewTagClicked ()
{
//	char str[10];
//	wsprintf (str, "%d", FindFreeTag());
//	pTagEdit->SetText (str);
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
TStatic *TSectorEditDialog::GetPointedStatic (TPoint& point)
{
	// Retreive object for handle
	TStatic *pStatic = NULL;

	if ( IsPointInDlgItem (IDC_CEILING_TEXT, point)  ||
		 IsPointInDlgItem (IDC_CEILING_FRAME, point) )
	{
		pStatic = pCeilingStatic;
	}

	else if ( IsPointInDlgItem (IDC_FLOOR_TEXT, point)  ||
			  IsPointInDlgItem (IDC_FLOOR_FRAME, point) )
	{
		pStatic = pFloorStatic;
	}

	return pStatic;
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//   Look if the dialog control 'resid' window contains the point
//   'clientPoint', which is a dialog client coord. of the point
BOOL TSectorEditDialog::IsPointInDlgItem (int itemId, TPoint &clientPoint)
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


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::TextureSelChange ()
{
	char texname[MAX_BITMAPNAME+1];
	// Get selected texture name
	if ( pTextureList->GetSelString (texname, MAX_BITMAPNAME) <= 1 )
	{
		TextureName[0] = '\0';
		return;
	}
	// Don't do anything if no real change
	if ( strcmp (texname, TextureName) == 0 )
		return;

	strcpy (TextureName, texname);

	// If texture view dialog box opened, display texture
	if ( pFTextureDialog != NULL && pFTextureDialog->IsWindow() )
	{
		TextureDblclick();
	}
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::TextureDblclick ()
{
	// Don't select empty texture !
	if ( TextureName[0] == '\0' || strcmp (TextureName, "-") == 0 )
		return;

	// Create modeless dialog box
	if ( pFTextureDialog == NULL || pFTextureDialog->IsWindow() == FALSE )
	{
		delete pFTextureDialog;
		pFTextureDialog = new TDisplayFloorTextureDialog (Parent);
		pFTextureDialog->Create();
	}

	if ( pFTextureDialog->IsWindow() )
	{
	   TextureInfo *info = FindTextureByName(TextureName);
	   
	   if ( pFTextureDialog->SelectBitmap2 (info->filename) < 0 )
	      Notify ("Error: Cannot select the texture name \"%s\" in the "
		      "dialog box of Floor/Ceiling Texture view ! (BUG)",
		      TextureName);
	}
	else
		Notify ("Error: Cannot create dialog box of Floor/Ceiling "
				"Texture view !");
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::EvLButtonDown (UINT modKeys, const TPoint& point)
{
	TDialog::EvLButtonDown(modKeys, point);

	// Retreive object for handle
	TStatic *pStatic = GetPointedStatic ((TPoint&)point);
	if ( pStatic == NULL )
		return;

	// If floor or ceiling static, get texture name
	char texname[MAX_BITMAPNAME + 1];
	pStatic->GetText (texname, MAX_BITMAPNAME + 1);
	if ( texname[0] != '\0' && strcmp (texname, "-") != 0 )
	{
		//pTextureList->SetSelString (texname, -1);
                List_SelectStringExact(pTextureList, -1, texname);
		TextureSelChange();  	// Be sure to change selection
	}
}


//////////////////////////////////////////////////////////////////////
// TSectorEditDialog
// -----------------
//
void TSectorEditDialog::EvLButtonDblClk (UINT modKeys, const TPoint& point)
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
		//pTextureList->SetSelString (texname, -1);
                List_SelectStringExact(pTextureList, -1, texname);
		TextureSelChange();  	// Be sure to change selection
		TextureDblclick();		// Force to open Dialog Box
	}
}

