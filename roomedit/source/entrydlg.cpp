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

	FILE:         entrydlg.cpp

	OVERVIEW
	========
	Source file for implementation of TViewEntryDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#include <ctype.h>

#ifndef __entrydlg_h
	#include "entrydlg.h"
#endif

#ifndef __wads_h
	#include "wads.h"
#endif


// Number of hex digits by line
#define NB_HEX_LINE	16


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TViewEntryDialog, TDialog)
//{{TViewEntryDialogRSP_TBL_BEGIN}}
	EV_WM_DRAWITEM,
	EV_CBN_SELCHANGE(IDC_VE_ENTRY_LIST, EntrySelChange),
//{{TViewEntryDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TViewEntryDialog Implementation}}

/////////////////////////////////////////////////////////////////////
// TViewEntryDialog
// ----------------
//
TViewEntryDialog::TViewEntryDialog (TWindow* parent, TResId resId,
									TModule* module):
	TDialog(parent, resId, module)
{
	CurrentEntry = NULL;
	pEntryCombo = new TComboBox(this, IDC_VE_ENTRY_LIST, 1);
	pHexDumpList = new TListBox(this, IDC_VE_DUMP_LIST);
}


/////////////////////////////////////////////////////////////////////
// ~TViewEntryDialog
// -----------------
//
TViewEntryDialog::~TViewEntryDialog ()
{
	Destroy();
}


/////////////////////////////////////////////////////////////////////
// SetupWindow
// -----------
//
void TViewEntryDialog::SetupWindow ()
{
	TDialog::SetupWindow();
	CenterWindow(this);

	// Setup the entries list box
	for (MDirPtr entry = MasterDir ; entry != NULL ; entry = entry->next)
	{
		char entryname[9];

		strncpy (entryname, entry->dir.name, 8);
		entryname[8] = '\0';
		pEntryCombo->AddString(entryname);
	}

	// Setup tab stops in HexDump list
	/*
	int TabStops[17];
	TabStops[0] = 6 * 4;
	for (int i = 1 ; i < 16 ; i++)
		TabStops[i] = TabStops[i-1] + 2 * 4 + 3;
	TabStops[i] = TabStops[i-1] + 3 * 4 + 2;

	pHexDumpList->SetTabStops(17, TabStops);
	*/
}


/////////////////////////////////////////////////////////////////////
// EntrySelChange
// --------------
//
void TViewEntryDialog::EntrySelChange ()
{
	SHORT   i;

	// Clear hex dump list box
	pHexDumpList->ClearList();
	CurrentEntry = NULL;

	// Just check index is valid
	int entryindex = pEntryCombo->GetSelIndex();
	if ( entryindex < 0 )
		return;

	// Get entry name
	char entryname[9];
	pEntryCombo->GetSelString(entryname, 9);

	// Search for entry index
	CurrentEntry = MasterDir;
	for (i = 0 ; (i < entryindex) && (CurrentEntry != NULL); i++)
		CurrentEntry = CurrentEntry->next;

	// This is a BUG (the entry should be in the combo box)
	if ( CurrentEntry == NULL )
	{
		Notify ("Entry \"%s\" not in master directory", entryname);
		return;
	}

	//
	// Setup fake list box
	//

	// Title
	pHexDumpList->AddString("0");

	// Each line
	ULONG last = (CurrentEntry->dir.size + NB_HEX_LINE - 1) / NB_HEX_LINE;
	for (ULONG off = 0 ; off < last ; off++)
	{
		char Buf[128];

		wsprintf (Buf, "%lu", off);
		//wsprintf (Buf, "");
		if ( pHexDumpList->AddString(Buf) < 0 )
		{
			Notify ("This entry is too big to be displayed entirely in "
					"the hexadecimal dump list box. The list box is apparently "
					"limited to %ld items (offset %06lX).",
					off, off * NB_HEX_LINE);
			break;
		}
	}
}


/////////////////////////////////////////////////////////////////////
// EvDrawItem
// ----------
//
void TViewEntryDialog::EvDrawItem (UINT ctrlId, const DRAWITEMSTRUCT& drawInfo)
{
	char Buf[128];

	// To be sure
	if ( CurrentEntry == NULL )
		return;

	// To be sure
	if ( ctrlId != IDC_VE_DUMP_LIST )
		return;

	// If there are no list box items, skip this message.
	if ( drawInfo.itemID == (UINT)-1 )
		return;

	// Build string to draw
	strcpy (Buf, "");
	if ( drawInfo.itemID == 0 )
	{
		char entryname[9];

		strncpy (entryname, CurrentEntry->dir.name, 8);
		entryname[8] = '\0';
		sprintf(Buf, "Contents of entry %s (size = %ld bytes):",
					 entryname, CurrentEntry->dir.size);
	}
	else
	{
		// Starting Offset in the entry
		ULONG EntryOffset = (ULONG)(drawInfo.itemID - 1) * NB_HEX_LINE;

		// Starting Offset in the wad file
		ULONG WadOffset   = CurrentEntry->dir.start + EntryOffset;

		// Number of chars for this line
		USHORT NbChars    = min(NB_HEX_LINE,
								(USHORT)(CurrentEntry->dir.size - EntryOffset));

		// Bytes of the line
		unsigned char HexBuf[NB_HEX_LINE];

		// Line len
		int len;
		int i;

		BasicWadSeek (CurrentEntry->wadfile, WadOffset);

		// Dump offset
		len = sprintf(Buf, "%06lX:\t", EntryOffset);

		// 16 hex values
		for (i = 0 ; i < NbChars ; i++)
		{
			BasicWadRead (CurrentEntry->wadfile, &(HexBuf[i]), 1);

			len += sprintf (&Buf[len], "%02X\t", HexBuf[i]);
		}

		// Add padding tabs
		for (; i < NB_HEX_LINE ; i++)
			len += sprintf(&Buf[len], "\t");
		// len += sprintf (&Buf[len], "\t");

		// 16 chars
		for (i = 0 ; i < NbChars ; i++)
		{
			char c = HexBuf[i];
			if ( ! isprint(c) )
				c = ' ';

			len += sprintf (&Buf[len], "%c", c);
		}
	}

	switch (drawInfo.itemAction)
	{
		case ODA_SELECT:
		case ODA_DRAWENTIRE:
		{
			// Retreive the average character width
			TEXTMETRIC tm;
			GetTextMetrics(drawInfo.hDC, &tm);

			// Setup tab stops in HexDump list
			int TabStops[NB_HEX_LINE+1];
			int i;

			TabStops[0] = 6 * tm.tmAveCharWidth;
			for (i = 1 ; i < NB_HEX_LINE ; i++)
				TabStops[i] = TabStops[i-1] + 3 * tm.tmAveCharWidth + 2;
			TabStops[i] = TabStops[i-1] + 4 * tm.tmAveCharWidth;

			TabbedTextOut(drawInfo.hDC,
						  drawInfo.rcItem.left, drawInfo.rcItem.top,
						  Buf, strlen(Buf),
						  NB_HEX_LINE+2, TabStops, 0);
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

