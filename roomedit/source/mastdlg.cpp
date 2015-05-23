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

	FILE:         mastdlg.cpp

	OVERVIEW
	========
	Source file for implementation of TMasterDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#include "mastdlg.h"

#ifndef OWL_LISTBOX_H
	#include <owl\listbox.h>
#endif

#ifndef __wads_h
	#include "wads.h"	// MasterDir
#endif


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TMasterDialog, TDialog)
//{{TMasterDialogRSP_TBL_BEGIN}}
    EV_WM_DRAWITEM,
//{{TMasterDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TMasterDialog Implementation}}


/////////////////////////////////////////////////////////////////////
// TMasterDialog
// -------------
//
TMasterDialog::TMasterDialog (TWindow* parent, TResId resId, TModule* module):
	TDialog(parent, resId, module)
{
	pMasterList = new TListBox(this, IDC_MASTER_LIST);
}


/////////////////////////////////////////////////////////////////////
// ~TMasterDialog
// --------------
//
TMasterDialog::~TMasterDialog ()
{
	Destroy();
}


/////////////////////////////////////////////////////////////////////
// SetupWindow
// -----------
//
void TMasterDialog::SetupWindow ()
{
	TDialog::SetupWindow();
	CenterWindow(this);

	// Set tab stops for list box
	const int Tab1 = 12*4;
	const int Tab2 = Tab1 + 36 * 4;
	const int Tab3 = Tab2 + 8 * 4;
	int TabStops[] = { Tab1, Tab2, Tab3 };

	pMasterList->SetTabStops (3, TabStops);

	// Set list box title (first line)
	pMasterList->AddString("0");
	pMasterList->SetItemData(0, NULL);

    /////////////////////////////////////////////////////////////////
    // Note on how to partialy break the 64Kb barrier of List boxes
    // -------------------------------------------------------------
	// The item string and data are:
	// String   ItemData
	// ------   ----------------------
	// "0"		MasterDir
	// "1"		MasterDir->next
	// "2"		MasterDir->next->next
	// ....
	//
	// This format is used to:
	//  1. have the good MasterDir order though the ListBox has
	//     the 'Sorted' style
	//  2. Keep the ListBox items well below < 64kb. If we used
	//     the real strings, it would take more than 64Kb.
	//  3. Retrieve easyly the MDirPtr for each string, when the
	//     EvDrawItem function is called for each string.
    /////////////////////////////////////////////////////////////////
	MDirPtr Dir = MasterDir;
	for (int i = 1 ; Dir != NULL ; i++)
	{
		char Buf[128];

		wsprintf(Buf, "%d", i);
		pMasterList->AddString(Buf);
		pMasterList->SetItemData(i, (DWORD)Dir);
		Dir = Dir->next;
	}
}


/////////////////////////////////////////////////////////////////////
// EvDrawItem
// ----------
//
void TMasterDialog::EvDrawItem (UINT ctrlId, const DRAWITEMSTRUCT& drawInfo)
{
	char Buf[128];

    // To be sure
	if ( ctrlId != IDC_MASTER_LIST )
		return;

	// If there are no list box items, skip this message.
	if (drawInfo.itemID == (UINT)-1)
		return;

	switch (drawInfo.itemAction)
	{
		case ODA_SELECT:
		case ODA_DRAWENTIRE:
		{
	// Build string to draw
	strcpy (Buf, "");
	if ( drawInfo.itemID == 0 )
	{
		// Set list box title (first line)
		wsprintf(Buf, "%s\t%s\t%s\t%s", "NAME_____",
										"FILE______________________________",
										"SIZE__",
										"START____");
	}
	else
	{
		MDirPtr Dir = MasterDir;

        // Retreive the Dir ptr of the item to be draw
		Dir = (MDirPtr)pMasterList->GetItemData(drawInfo.itemID);

		if ( Dir != NULL )
		{
			char DataName[9];
			strncpy(DataName, Dir->dir.name, 8);
			DataName[8] = '\0';

			wsprintf(Buf, "%s\t%s\t%06ld\t0x%08lx",
						  DataName, Dir->wadfile->filename,
						  Dir->dir.size, Dir->dir.start);
		}
	}

			// Retreive the average character width
			TEXTMETRIC tm;
			GetTextMetrics(drawInfo.hDC, &tm);

			int Tab1 =        14 * tm.tmAveCharWidth;
			int Tab2 = Tab1 + 41 * tm.tmAveCharWidth;
			int Tab3 = Tab2 +  9 * tm.tmAveCharWidth;
			int TabStops[] = { Tab1, Tab2, Tab3 };

			TabbedTextOut(drawInfo.hDC,
						  drawInfo.rcItem.left, drawInfo.rcItem.top,
						  Buf, strlen(Buf),
						  3, TabStops, 0);
#if 0
			/* Is the item selected? */
			if (drawInfo.itemState & ODS_SELECTED)
			{
				/* Draw a rectangle around bitmap to indicate the selection. */
				DrawFocusRect(drawInfo.hDC, &drawInfo.rcItem);
			}
#endif
		}
		break;

		case ODA_FOCUS:
			/*
			 * Do not process focus changes. The focus caret (outline
			 * rectangle) indicates the selection. The Which one? (IDOK)
			 * button indicates the final selection.
			 */
		  break;
	}
}

