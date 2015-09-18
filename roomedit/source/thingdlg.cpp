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

	FILE:         thingdlg.cpp

	OVERVIEW
	========
	Source file for implementation of TThingEditDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#include "thingdlg.h"
//#include "Entrance.h"

#ifndef OWL_LISTBOX_H
	#include <owl\listbox.h>
#endif

#ifndef OWL_COMBOBOX_H
	#include <owl\combobox.h>
#endif

#ifndef OWL_CHECKBOX_H
	#include <owl\checkbox.h>
#endif

#ifndef OWL_RADIOBUT_H
	#include <owl\radiobut.h>
#endif

#ifndef __viewbmp_h
	#include "viewbmp.h"
#endif

#ifndef __levels_h
	#include "levels.h"
#endif

#ifndef __things_h
	#include "things.h"
#endif

#ifndef __newers_h
	#include "newers.h"
#endif

#ifndef __cnfthdlg_h
	#include "cnfthdlg.h"
#endif

// HELP IDs
#include "windeuhl.h"

//
// Find a thing set from a thing number
// (-1 if thing is not in a set)
//
int FindThingSet(SHORT tnum)
{
	if (tnum < kodPlayerBlocking)
		return -1;
	if (tnum < kodObject)
		return tnum;
	else if (tnum >= GetNumKodObjects())
		return -1;
	else
		return kodObject;
}

//
// Find a thing type from a thing set and an index in the set
// (This function must skip DOOM2 entries if DOOM1 version)
//
SHORT FindThingType(int tset, int tindex)
{
	return tset + tindex;
}


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TThingEditDialog, TDialog)
//{{TThingEditDialogRSP_TBL_BEGIN}}
	EV_BN_CLICKED(IDC_PLAYER, PlayerClicked),
	EV_BN_CLICKED(IDC_EXIT, ExitClicked),
	EV_BN_CLICKED(IDC_OBJECTS, ObjectsClicked),
	EV_BN_CLICKED(IDC_MONSTERGEN, MonsterGenClicked),
	EV_BN_CLICKED(IDC_ENTRANCE, EntranceClicked),

	EV_LBN_SELCHANGE(IDC_THING_LIST, ThingSelchange),
	EV_CBN_SELCHANGE(IDC_EXIT_ROOM_ID, RoomIDSelchange),
	EV_CBN_SELCHANGE(IDC_SUBTYPE, SubTypeSelchange),

	EV_BN_CLICKED(IDC_ANGLE_E, AngleClicked),
	EV_BN_CLICKED(IDC_ANGLE_N, AngleClicked),
	EV_BN_CLICKED(IDC_ANGLE_NE, AngleClicked),
	EV_BN_CLICKED(IDC_ANGLE_NW, AngleClicked),
	EV_BN_CLICKED(IDC_ANGLE_S, AngleClicked),
	EV_BN_CLICKED(IDC_ANGLE_SE, AngleClicked),
	EV_BN_CLICKED(IDC_ANGLE_SW, AngleClicked),
	EV_BN_CLICKED(IDC_ANGLE_W, AngleClicked),

	EV_BN_CLICKED(IDOK, CmOk),
	EV_BN_CLICKED(IDC_SELECT_FROM_ENTRANCES, SelectFromEntrancesClicked),

//{{TThingEditDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TThingEditDialog Implementation}}


//////////////////////////////////////////////////////////////////////
// TThingEditDialog
// ----------
//
TThingEditDialog::TThingEditDialog (TWindow* parent, SelPtr SelectedThings, TResId resId, TModule* module):
	TDialog(parent, resId, module)
{
	SelThings = SelectedThings;
	CurThing  = Things[SelectedThings->objnum];
	memset (&ConfirmData, 0, sizeof(ConfirmData));

	pXPosEdit = newTEdit (this, IDC_THING_XPOS, 7);
	//pXPosEdit->SetValidator(new TRangeValidator(MapMinX, MapMaxX));
	pYPosEdit = newTEdit (this, IDC_THING_YPOS, 7);
	//pYPosEdit->SetValidator(new TRangeValidator(MapMinY, MapMaxY));

	pXExit = newTEdit (this, IDC_XEXIT, 7);
	pYExit = newTEdit (this, IDC_YEXIT, 7);
	pEntrancesButton = newTButton (this, IDC_SELECT_FROM_ENTRANCES);

	pPlayerRadio = newTRadioButton (this, IDC_PLAYER);
	pExitRadio = newTRadioButton (this, IDC_EXIT);
	pObjectsRadio = newTRadioButton (this, IDC_OBJECTS);
	pMonsterGenRadio = newTRadioButton (this, IDC_MONSTERGEN);
	pEntranceRadio = newTRadioButton (this, IDC_ENTRANCE);

	pAngleERadio = newTRadioButton(this, IDC_ANGLE_E, 0);
	pAngleNRadio = newTRadioButton(this, IDC_ANGLE_N, 0);
	pAngleNERadio = newTRadioButton(this, IDC_ANGLE_NE, 0);
	pAngleNWRadio = newTRadioButton(this, IDC_ANGLE_NW, 0);
	pAngleSRadio = newTRadioButton(this, IDC_ANGLE_S, 0);
	pAngleSERadio = newTRadioButton(this, IDC_ANGLE_SE, 0);
	pAngleSWRadio = newTRadioButton(this, IDC_ANGLE_SW, 0);
	pAngleWRadio = newTRadioButton(this, IDC_ANGLE_W, 0);

	pThingList = newTListBox (this, IDC_THING_LIST);

	pExitRoomID = newTComboBox(this, IDC_EXIT_ROOM_ID);
	pSubType = newTComboBox(this, IDC_SUBTYPE);

	pDontGenerate = newTCheckBox(this, IDC_DONT_GENERATE);
	pComment = newTEdit(this, IDC_COMMENT);

}


//////////////////////////////////////////////////////////////////////
// TThingEditDialog
// ----------
//
TThingEditDialog::~TThingEditDialog ()
{
	Destroy();
}


//////////////////////////////////////////////////////////////////////
// TThingEditDialog
// ----------
//  Retrieve Thing set from the first selected thing type.
//  and update the thing set check boxes.
void
TThingEditDialog::InitThingSet ()
{
	ThingSet = FindThingSet(CurThing.type);

	pEntranceRadio->SetCheck (ThingSet == kodEntrance ? BF_CHECKED : BF_UNCHECKED);
	pPlayerRadio->SetCheck (ThingSet == kodPlayerBlocking ? BF_CHECKED : BF_UNCHECKED);
	pExitRadio->SetCheck (ThingSet == kodExit ? BF_CHECKED : BF_UNCHECKED);
	pMonsterGenRadio->SetCheck (ThingSet == kodMonsterGenerator ? BF_CHECKED : BF_UNCHECKED);
	pObjectsRadio->SetCheck (ThingSet == kodObject ? BF_CHECKED : BF_UNCHECKED);
	pDontGenerate->SetCheck ((CurThing.flags & THING_FLAG_DONTGENERATE) ? BF_CHECKED : BF_UNCHECKED);

	pXExit->EnableWindow(ThingSet == kodExit);
	pYExit->EnableWindow(ThingSet == kodExit);
	pExitRoomID->EnableWindow(ThingSet == kodExit);
	pSubType->EnableWindow((ThingSet == kodObject) && (GetNumKodTypes(CurThing.type) > 0));
	pEntrancesButton->EnableWindow(kodExit == ThingSet);
}


//////////////////////////////////////////////////////////////////////
// TThingEditDialog
// ----------
//
void
TThingEditDialog::SetupWindow ()
{
	TDialog::SetupWindow();
	::CenterWindow (this);

	// Retreive thing set and setup check boxes
	InitThingSet ();

	// Add thing types of thing set in list box
	SetupListBox();

	// Set angle and appearence boxes
	SetAngle();

	// Insert thing position in edit boxes
	char tmp[20];
	sprintf (tmp, "%d", CurThing.xpos);
	pXPosEdit->SetText(tmp);

	sprintf (tmp, "%d", CurThing.ypos);
	pYPosEdit->SetText(tmp);

	sprintf (tmp, "%d", CurThing.xExitPos);
	pXExit->SetText(tmp);

	sprintf (tmp, "%d", CurThing.yExitPos);
	pYExit->SetText(tmp);

	pComment->SetText(CurThing.comment);
}


//////////////////////////////////////////////////////////////////////
// TThingEditDialog
// ----------------
// Fills the thing type list box with the thing set types.
//
void
TThingEditDialog::SetupListBox ()
{
	int i;

	pThingList->ClearList();
	pExitRoomID->ClearList();
	pSubType->ClearList();
	pSubType->EnableWindow(FALSE);
	switch (ThingSet)
	{
	case kodPlayerBlocking:
		pThingList->SetSelIndex(pThingList->AddString ("Player Boundary"));
		break;
	case kodEntrance:
		pThingList->SetSelIndex(pThingList->AddString ("Room Entrance"));
		break;
	case kodExit:
		pThingList->SetSelIndex(pThingList->AddString ("Room Exit"));
		for(i=0; i < GetNumKodRooms(); i++)
		{
			int index = pExitRoomID->AddString(GetKodRoomDecorativeName(i));
			pExitRoomID->SetItemData(index,GetKodRoomID(i));
			if (GetKodRoomID(i) == CurThing.when)
			{
				pExitRoomID->SetSelIndex(index);
			}
		}
		break;
	case kodMonsterGenerator:
		pThingList->SetSelIndex(pThingList->AddString ("Monster Generator"));
		break;
	case kodExtraPreObject2:
	case kodExtraPreObject3:
	case kodExtraPreObject4:
		break;
	case kodObject:
		for (i=kodObject; i < GetNumKodObjects(); i++)
		{
			int index = pThingList->AddString(GetKodObjectDecorativeName(i));
			pThingList->SetItemData(index,i);
			if (CurThing.type == i)
			{
				pThingList->SetSelIndex(index);
				if (GetNumKodTypes(CurThing.type) > 0)
					pSubType->EnableWindow(TRUE);
				for(int t=0; t < GetNumKodTypes(CurThing.type); t++)
				{
					index = pSubType->AddString(GetKodTypeDecorativeName(CurThing.type,t));
					pSubType->SetItemData(index,i);
					if (GetKodTypeID(CurThing.type,t) == CurThing.when)
					{
						pSubType->SetSelIndex(index);
					}
				}
			}
		}
		break;
	}
	pXExit->EnableWindow(kodExit == ThingSet);
	pYExit->EnableWindow(kodExit == ThingSet);
	pExitRoomID->EnableWindow(kodExit == ThingSet);
	pEntrancesButton->EnableWindow(kodExit == ThingSet);
	SetAngle();
}

//////////////////////////////////////////////////////////////////////
// TThingEditDialog
// ----------
//
void TThingEditDialog::PlayerClicked ()
{
	ThingSet = kodPlayerBlocking;
	CurThing.type = kodPlayerBlocking;
	SetupListBox ();
}


//////////////////////////////////////////////////////////////////////
// TThingEditDialog
// ----------
//
void TThingEditDialog::ExitClicked ()
{
	ThingSet = kodExit;
	CurThing.type = kodExit;
	SetupListBox ();
}

void TThingEditDialog::EntranceClicked()
{
	ThingSet = kodEntrance;
	CurThing.type = kodEntrance;
	SetupListBox ();
}

void TThingEditDialog::SelectFromEntrancesClicked ()
{
  // Disabled 7/3/04 ARK.  See comment in windeu.cpp about entrances
#if 0  
	int startExitRoomID = CurThing.when;
	int exitRoomID,exitRow,exitCol,exitAngle;
	int id = SelectEntrance(this,startExitRoomID,CurThing.id,&exitRoomID,&exitRow,&exitCol,&exitAngle);
	if (id >= 0)
	{
		int roomIndex = GetKodIndexFromID(exitRoomID);
		if (roomIndex >= 0)
		{
			char tmp[256];
			sprintf (tmp, "%d", exitCol);
			pXExit->SetText(tmp);
			sprintf (tmp, "%d", exitRow);
			pYExit->SetText(tmp);
			pExitRoomID->SetSelIndex(pExitRoomID->FindString(GetKodRoomDecorativeName(roomIndex),-1));
			CurThing.xExitPos = exitCol;
			CurThing.yExitPos = exitRow;
			CurThing.id = id;
			CurThing.angle = exitAngle;
			CurThing.when = exitRoomID;
			SetAngle();
			SetupListBox();
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////
// TThingEditDialog
// ----------
//
void TThingEditDialog::ObjectsClicked ()
{
	ThingSet = kodObject;
	if (CurThing.type < kodObject)
		CurThing.type = kodObject;
	SetupListBox ();
}

void TThingEditDialog::MonsterGenClicked ()
{
	ThingSet = kodMonsterGenerator;
	CurThing.type = kodMonsterGenerator;
	SetupListBox ();
}

//////////////////////////////////////////////////////////////////////
// TThingEditDialog
// ----------------
//
void TThingEditDialog::CmOk ()
{
	// Cannot close if edit controls not valid
	if ( !CanClose() )
		return;

	if (pDontGenerate->GetCheck() == BF_CHECKED)
		CurThing.flags |= THING_FLAG_DONTGENERATE;
	else
		CurThing.flags &= ~THING_FLAG_DONTGENERATE;

	// Get thing position
	char tmp[20];
	pXPosEdit->GetText(tmp, 9);
	if ( CurThing.xpos != atoi(tmp) )	ConfirmData.pXPosCheck = TRUE;
	CurThing.xpos = atoi(tmp);

	pYPosEdit->GetText(tmp, 9);
	if ( CurThing.ypos != atoi(tmp) )	ConfirmData.pYPosCheck = TRUE;
	CurThing.ypos = atoi(tmp);

	pXExit->GetText(tmp, 9);
	if ( CurThing.xExitPos != atoi(tmp) )	ConfirmData.pExitCheck = TRUE;
	CurThing.xExitPos = atoi(tmp);

	pYExit->GetText(tmp, 9);
	if ( CurThing.yExitPos != atoi(tmp) )	ConfirmData.pExitCheck = TRUE;
	CurThing.yExitPos = atoi(tmp);

	GetAngle();

	pComment->GetWindowText(CurThing.comment,sizeof(CurThing.comment));

	if (CurThing.type == kodEntrance)
	{
//		CurThing.id = SetEntrance(RoomID, CurThing.id, CurThing.xpos, CurThing.ypos, CurThing.angle, CurThing.comment);
	}

	// Update confirm data for X and Y pos
	// DEBUG: This doesn't work !!! (always FALSE)
	// ConfirmData.pXPosCheck = pXPosEdit->IsModified();
	// ConfirmData.pYPosCheck = pYPosEdit->IsModified();

	// Do we made changes ?
	TPtr pThing = &Things[SelThings->objnum];

	if ( memcmp (pThing, &CurThing, sizeof(Thing)) != 0 )
	{
		MadeChanges = TRUE;
	}

	// Update first thing in list
	*pThing = CurThing;

	// If more than one thing, copy selected attributes to them
	if ( SelThings->next != NULL )
	{
		SET_HELP_CONTEXT(Confirming_copy_of_Thing_attributes);

		if ( TConfirmThingDialog (this, ConfirmData).Execute() == IDOK )
		{
			// Copy the selected attributes of CurThing to the selection list
			for (SelPtr cur = SelThings->next ; cur != NULL ; cur = cur->next )
			{
				Thing *pThing = &Things[cur->objnum];
				Thing ThingBefore = *pThing;	// Copy before changes

				if ( ConfirmData.pXPosCheck )	pThing->xpos  = CurThing.xpos;
				if ( ConfirmData.pYPosCheck )	pThing->ypos  = CurThing.ypos;
				if ( ConfirmData.pAngleCheck )pThing->angle = CurThing.angle;
				if ( ConfirmData.pTypeCheck ) pThing->type  = CurThing.type;
				if ( ConfirmData.pExitCheck )
				{
					pThing->when  = CurThing.when;
					pThing->xExitPos = CurThing.xExitPos;
					pThing->yExitPos = CurThing.yExitPos;
				}
				if ( ConfirmData.pSubTypeCheck) pThing->when = CurThing.when;

				strcpy(pThing->comment,CurThing.comment);
				pThing->flags = CurThing.flags;

				if (pThing->type == kodEntrance)
				{
//					pThing->id = SetEntrance(RoomID, pThing->id, pThing->xpos, pThing->ypos, pThing->angle, pThing->comment);
				}
				// Did we made changes?
				if ( memcmp(pThing, &ThingBefore, sizeof(Thing)) != 0 )
					MadeChanges;
			}
		}
		RESTORE_HELP_CONTEXT();
	}

	// Close dialog box
	TDialog::CmOk();
}


//////////////////////////////////////////////////////////////////////
// TThingEditDialog
// ----------------
//
void TThingEditDialog::GetAngle ()
{
	if ( pAngleERadio->GetCheck() == BF_CHECKED )
		CurThing.angle = 0;
	else if ( pAngleNERadio->GetCheck() == BF_CHECKED )
		CurThing.angle = 45;
	else if ( pAngleNRadio->GetCheck() == BF_CHECKED )
		CurThing.angle = 90;
	else if ( pAngleNWRadio->GetCheck() == BF_CHECKED )
		CurThing.angle = 135;
	else if ( pAngleWRadio->GetCheck() == BF_CHECKED )
		CurThing.angle = 180;
	else if ( pAngleSWRadio->GetCheck() == BF_CHECKED )
		CurThing.angle = 225;
	else if ( pAngleSRadio->GetCheck() == BF_CHECKED )
		CurThing.angle = 270;
	else if ( pAngleSERadio->GetCheck() == BF_CHECKED )
		CurThing.angle = 315;
}


//////////////////////////////////////////////////////////////////////
// TThingEditDialog
// ----------------
//
void TThingEditDialog::SetAngle ()
{
	pAngleERadio->SetCheck(CurThing.angle == 0 ? BF_CHECKED : BF_UNCHECKED);
	pAngleNERadio->SetCheck(CurThing.angle == 45 ? BF_CHECKED : BF_UNCHECKED);
	pAngleNRadio->SetCheck(CurThing.angle == 90 ? BF_CHECKED : BF_UNCHECKED);
	pAngleNWRadio->SetCheck(CurThing.angle == 135 ? BF_CHECKED : BF_UNCHECKED);
	pAngleWRadio->SetCheck(CurThing.angle == 180 ? BF_CHECKED : BF_UNCHECKED);
	pAngleSWRadio->SetCheck(CurThing.angle == 225 ? BF_CHECKED : BF_UNCHECKED);
	pAngleSRadio->SetCheck(CurThing.angle == 270 ? BF_CHECKED : BF_UNCHECKED);
	pAngleSERadio->SetCheck(CurThing.angle == 315 ? BF_CHECKED : BF_UNCHECKED);
}

//////////////////////////////////////////////////////////////////////
// TThingEditDialog
// ----------------
//  The user changed the selected thing type in the list box,
//  we update the CurThing struct
void TThingEditDialog::ThingSelchange ()
{
	char name[256];
	// Get selected string
	int index = pThingList->GetSelIndex ();
	if (index < 0)
		return;
	pThingList->GetString(name,index);
	CurThing.type = FindKodObjectByName(name);
	ConfirmData.pTypeCheck = TRUE;	// Remember in confirm data
	pSubType->ClearList();
	pSubType->EnableWindow(FALSE);
	for(int t=0; t < GetNumKodTypes(CurThing.type); t++)
	{
		pSubType->EnableWindow(TRUE);
		index = pSubType->AddString(GetKodTypeDecorativeName(CurThing.type,t));
		if (GetKodTypeID(CurThing.type,t) == CurThing.when)
		{
			pSubType->SetSelIndex(index);
		}
	}
}

void TThingEditDialog::RoomIDSelchange ()
{
	char buffer[256];

	if ( ThingSet != kodExit )
		return;

	// Get selected string
	int SelIndex = pExitRoomID->GetSelIndex ();
	if ( SelIndex < 0)
		return;

	pExitRoomID->GetString(buffer,SelIndex);
	CurThing.when = FindKodRoomID(buffer);
	ConfirmData.pExitCheck = TRUE;	// Remember in confirm data
}

void TThingEditDialog::SubTypeSelchange()
{
	char buffer[256];

	if ( ThingSet != kodObject )
		return;

	// Get selected string
	int SelIndex = pSubType->GetSelIndex ();
	if ( SelIndex < 0)
		return;

	pSubType->GetString(buffer,SelIndex);
	CurThing.when = FindKodTypeID(CurThing.type,buffer);
	ConfirmData.pSubTypeCheck = TRUE;	// Remember in confirm data
}

/////////////////////////////////////////////////////////////////////
//
// Functions to setup the ConfirmData fields when the user changes
// something in the dialog box.
//
/////////////////////////////////////////////////////////////////////

void TThingEditDialog::AngleClicked ()
{
	ConfirmData.pAngleCheck = TRUE;
}


