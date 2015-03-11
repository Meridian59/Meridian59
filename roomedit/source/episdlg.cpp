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

	FILE:         episdlg.cpp

	OVERVIEW
	========
	Source file for implementation of TEpisodeMissionDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#ifndef __wads_h
	#include "wads.h"
#endif

#ifndef __episdlg_h
	#include "episdlg.h"
#endif

#ifndef OWL_RADIOBUT_H
	#include <owl\radiobut.h>
#endif

#ifndef OWL_GROUPBOX_H
	#include <owl\groupbox.h>
#endif

#ifndef OWL_CHECKBOX_H
	#include <owl\checkbox.h>
#endif

#ifndef OWL_STATIC_H
	#include <owl\static.h>
#endif

#ifndef OWL_VALIDATE_H
	#include <owl\validate.h>
#endif


// Last DOOM2 check button state
static int LastDoom2Check = -1;
// Last episode/mission selected
static int LastEpisode = 1;
static int LastMission = 1;
// Last map selected
static int LastMap     = 1;

//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TEpisodeMissionDialog, TDialog)
//{{TEpisodeMissionDialogRSP_TBL_BEGIN}}
	EV_BN_CLICKED(IDOK, CmOk),
	EV_BN_CLICKED(IDC_EM_DOOM2_CHECK, Doom2CheckClicked),
//{{TEpisodeMissionDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TEpisodeMissionDialog Implementation}}


///////////////////////////////////////////////////////////
// TEpisodeMissionDialog
// ---------------------
//
TEpisodeMissionDialog::TEpisodeMissionDialog (TWindow* parent, char *levelName, BOOL mustExist,
											  int resId, TModule* module):
	TDialog(parent, resId, module)
{
	int i;

	pLevelName = levelName;
	MustExist = mustExist;

	if ( LastEpisode < 1 )	LastEpisode = 1;
	if ( LastEpisode > 3 )	LastEpisode = 3;
	if ( LastMission < 1 )	LastMission = 1;
	if ( LastMission > 9 )	LastMission = 9;
	if ( LastMap < 1 )		LastMap = 1;
	if ( LastMap > 32 )		LastMap = 32;

	pDoom2Check = new TCheckBox(this, IDC_EM_DOOM2_CHECK, 0);

	pDoom1Group = new TGroupBox(this, IDC_EM_DOOM1_GROUP);
	for (i = 0 ; i < 3 * 9 ; i++)
		pDoom1Radio[i] = new TRadioButton(this, IDC_EM_FIRST + i);

	for (i = 0 ; i < 3 ; i++)
		pEpisodeStatic[i] = new TStatic(this, IDC_EM_EPISODE_FIRST + i);

	for (i = 0 ; i < 2 ; i++)
		pSeparatorStatic[i] = new TStatic(this, IDC_EM_SEPARATOR_FIRST + i);

	pDoom2Group = new TGroupBox(this, IDC_EM_DOOM2_GROUP);
	for (i = 0 ; i < 32 ; i++)
		pDoom2Radio[i] = new TRadioButton(this, IDC_EM_MAP_FIRST + i);

}


///////////////////////////////////////////////////////////
// TEpisodeMissionDialog
// ---------------------
//
TEpisodeMissionDialog::~TEpisodeMissionDialog ()
{
	Destroy();
}


///////////////////////////////////////////////////////////
// TEpisodeMissionDialog
// ---------------------
//
void TEpisodeMissionDialog::SetupWindow ()
{
	TDialog::SetupWindow();
	::CenterWindow (this);

	int i;

	// Check previous selection
	pDoom1Radio[(LastEpisode - 1) * 9 + (LastMission - 1)]->SetCheck (BF_CHECKED);
	pDoom2Radio[(LastMap - 1)]->SetCheck (BF_CHECKED);

	if ( MustExist )
	{
		for (i = 0 ; i < 3 * 9 ; i++ )
		{
			char name[9];

			wsprintf(name, "E%dM%d", (i / 9) + 1, (i % 9) + 1);
			if ( FindMasterDir (MasterDir, name) == NULL )
				pDoom1Radio[i]->EnableWindow(FALSE);
		}

		for (i = 0 ; i < 32 ; i++ )
		{
			char name[9];

			wsprintf(name, "MAP%02d", i + 1);
			if ( FindMasterDir (MasterDir, name) == NULL )
				pDoom2Radio[i]->EnableWindow(FALSE);
		}
	}

	// Select state of DOOM2 check box.
	switch (LastDoom2Check)
	{
		case -1:
			if ( DoomVersion == 2 )
				pDoom2Check->SetCheck(BF_CHECKED);
			else
				pDoom2Check->SetCheck(BF_UNCHECKED);
			break;
		case TRUE:
			pDoom2Check->SetCheck(BF_CHECKED);
			break;
		case FALSE:
			pDoom2Check->SetCheck(BF_UNCHECKED);
			break;
	}

	Doom2CheckClicked();
}


///////////////////////////////////////////////////////////
// TEpisodeMissionDialog
// ---------------------
//
void TEpisodeMissionDialog::CmOk ()
{
	if ( pDoom2Check->GetCheck() == BF_CHECKED )
	{
		for (SHORT i = 0 ; i < 32 ; i++)
		{
			// If radio button checked
			if ( pDoom2Radio[i]->GetCheck() == BF_CHECKED )
			{
				LastMap = i + 1;
				break ;
			}
		}
		sprintf (pLevelName, "MAP%02d", LastMap);
		LastDoom2Check = TRUE;	// The DOOM2 check button was checked
	}
	else
	{
		for (SHORT i = 0 ; i < 3 * 9 ; i++)
		{
			// If radio button checked
			if ( pDoom1Radio[i]->GetCheck() == BF_CHECKED )
			{
				LastEpisode = (i / 9) + 1;
				LastMission = (i % 9) + 1;
				break ;
			}
		}
		sprintf (pLevelName, "E%dM%d", LastEpisode, LastMission);
		LastDoom2Check = FALSE;	// The DOOM2 check button was unchecked
	}

	if ( (MustExist) && (FindMasterDir (MasterDir, pLevelName) == NULL) )
	{
		Notify ("Invalid level number: %s.\nThe level is not present "
				"in the Master directory", pLevelName);
		return;
	}

	TDialog::CmOk();
}


///////////////////////////////////////////////////////////
// TEpisodeMissionDialog
// ---------------------
//
void TEpisodeMissionDialog::Doom2CheckClicked ()
{
	int i;

	// Enable DOOM2 map number
	if ( pDoom2Check->GetCheck() == BF_CHECKED )
	{
		pDoom1Group->ShowWindow(SW_HIDE);
		for (i = 0 ; i < 2 ; i++)
			pSeparatorStatic[i]->ShowWindow(SW_HIDE);

		for (i = 0 ; i < 3 ; i++)
			pEpisodeStatic[i]->ShowWindow(SW_HIDE);

		for (i = 0 ; i < 3 * 9 ; i++)
			pDoom1Radio[i]->ShowWindow(SW_HIDE);

		pDoom2Group->ShowWindow(SW_SHOW);
		for (i = 0 ; i < 32 ; i++)
			pDoom2Radio[i]->ShowWindow(SW_SHOW);
	}
	// Enable DOOM1/HERETIC level number
	else
	{
		pDoom2Group->ShowWindow(SW_HIDE);
		for (i = 0 ; i < 32 ; i++)
			pDoom2Radio[i]->ShowWindow(SW_HIDE);

		pDoom1Group->ShowWindow(SW_SHOW);
		for (i = 0 ; i < 2 ; i++)
			pSeparatorStatic[i]->ShowWindow(SW_SHOW);

		for (i = 0 ; i < 3 ; i++)
			pEpisodeStatic[i]->ShowWindow(SW_SHOW);

		for (i = 0 ; i < 3 * 9 ; i++)
			pDoom1Radio[i]->ShowWindow(SW_SHOW);
	}
}

