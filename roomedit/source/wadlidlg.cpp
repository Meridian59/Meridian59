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

	FILE:         wadlidlg.cpp

	OVERVIEW
	========
	Source file for implementation of TWadlistDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#ifndef __wadlidlg_h
	#include "wadlidlg.h"
#endif

#ifndef OWL_LISTBOX_H
	#include <owl\listbox.h>
#endif

#ifndef OWL_CHECKBOX_H
	#include <owl\checkbox.h>
#endif

#ifndef __wads_h
	#include "wads.h"
#endif

#ifndef __names_h
	#include "names.h"	// IsDoomLevelName
#endif


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TWadlistDialog, TDialog)
//{{TWadlistDialogRSP_TBL_BEGIN}}
	EV_LBN_SELCHANGE(IDC_WADLISTBOX, LBNSelChange),
    EV_BN_CLICKED(IDC_CHECK_ONLYEM, OnlyEMClicked),
//{{TWadlistDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TWadlistDialog Implementation}}


/////////////////////////////////////////////////////////////////
// TWadlistDialog
// --------------
//
TWadlistDialog::TWadlistDialog (TWindow* parent, TResId resId, TModule* module):
	TDialog(parent, resId, module)
{
	pWadlistBox = new TListBox (this, IDC_WADLISTBOX);
	pDirlistBox = new TListBox (this, IDC_DIRLISTBOX);
	pOnlyEMCheckBox = new TCheckBox (this, IDC_CHECK_ONLYEM);
}


/////////////////////////////////////////////////////////////////
// TWadlistDialog
// --------------
//
TWadlistDialog::~TWadlistDialog ()
{
	Destroy();
}


/////////////////////////////////////////////////////////////////
// TWadlistDialog
// --------------
//
void TWadlistDialog::SetupWindow ()
{
	TDialog::SetupWindow();
	::CenterWindow (this);

	// Build wad file list
	for (WadPtr wad = WadFileList; wad; wad = wad->next)
	{
		pWadlistBox->AddString (wad->filename) ;
	}
}


/////////////////////////////////////////////////////////////////
// TWadlistDialog
// --------------
//
void TWadlistDialog::LBNSelChange ()
{
	UpdateDirlistBox();
}


/////////////////////////////////////////////////////////////////
// TWadlistDialog
// --------------
//
void TWadlistDialog::OnlyEMClicked ()
{
	UpdateDirlistBox();
}


/////////////////////////////////////////////////////////////////
// TWadlistDialog
// --------------
//
void TWadlistDialog::UpdateDirlistBox ()
{
	WadPtr wad;
	int SelIndex = pWadlistBox->GetSelIndex();
	BOOL OnlyEMChecked = pOnlyEMCheckBox->GetCheck() == BF_CHECKED;

	// If no selection, return
	if ( SelIndex < 0 )
		return;

	// Search for wad file number 'SelIndex' in list
	wad = WadFileList;
	for (int i = 0 ; i < SelIndex ; i ++)
		wad = wad->next;

	// Clear list of directory entries
	pDirlistBox->ClearList();

	// Insert list of directory entries for this 'wad' file
	for (size_t d = 0 ; d < (size_t)wad->dirsize ; d++)
	{
		char entry[9];

		strncpy (entry, wad->directory[d].name, 8);
		entry[8] = '\0';

		if ( !OnlyEMChecked ||
			 IsDoomLevelName (entry, 1) || IsDoomLevelName (entry, 2))
		{
			pDirlistBox->AddString (entry);
		}
	}
}

