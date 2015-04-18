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

	FILE:         statdlg.cpp

	OVERVIEW
	========
	Source file for implementation of TStatisticsDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#ifndef __statdlg_h
	#include "statdlg.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef __levels_h
	#include "levels.h"		// Things, Vertices, ...
#endif


//{{TStatisticsDialog Implementation}}


TStatisticsDialog::TStatisticsDialog (TWindow* parent, TResId resId, TModule* module):
	TDialog(parent, resId, module)
{
}


TStatisticsDialog::~TStatisticsDialog ()
{
	Destroy();
}


void TStatisticsDialog::SetupWindow ()
{
	TDialog::SetupWindow();
	::CenterWindow(this);

	SendDlgItemMsg(IDC_TH_NUM, WM_SETTEXT, 0, (LPARAM)FormatNumber(NumThings));
	SendDlgItemMsg(IDC_VE_NUM, WM_SETTEXT, 0, (LPARAM)FormatNumber(NumVertexes));
	SendDlgItemMsg(IDC_LD_NUM, WM_SETTEXT, 0, (LPARAM)FormatNumber(NumLineDefs));
	SendDlgItemMsg(IDC_SD_NUM, WM_SETTEXT, 0, (LPARAM)FormatNumber(NumSideDefs));
	SendDlgItemMsg(IDC_SE_NUM, WM_SETTEXT, 0, (LPARAM)FormatNumber(NumSectors));

	SendDlgItemMsg(IDC_TH_SIZE, WM_SETTEXT, 0, (LPARAM)FormatNumber(((ULONG)NumThings   * sizeof(Thing)   + 512L) / 1024L));
	SendDlgItemMsg(IDC_VE_SIZE, WM_SETTEXT, 0, (LPARAM)FormatNumber(((ULONG)NumVertexes * sizeof(Vertex)  + 512L) / 1024L));
	SendDlgItemMsg(IDC_LD_SIZE, WM_SETTEXT, 0, (LPARAM)FormatNumber(((ULONG)NumLineDefs * sizeof(LineDef) + 512L) / 1024L));
	SendDlgItemMsg(IDC_SD_SIZE, WM_SETTEXT, 0, (LPARAM)FormatNumber(((ULONG)NumSideDefs * sizeof(SideDef) + 512L) / 1024L));
	SendDlgItemMsg(IDC_SE_SIZE, WM_SETTEXT, 0, (LPARAM)FormatNumber(((ULONG)NumSectors  * sizeof(Sector)  + 512L) / 1024L));

	SendDlgItemMsg(IDC_TH_BYTES, WM_SETTEXT, 0, (LPARAM)FormatNumber((ULONG)NumThings   * sizeof(Thing)));
	SendDlgItemMsg(IDC_VE_BYTES, WM_SETTEXT, 0, (LPARAM)FormatNumber((ULONG)NumVertexes * sizeof(Vertex)));
	SendDlgItemMsg(IDC_LD_BYTES, WM_SETTEXT, 0, (LPARAM)FormatNumber((ULONG)NumLineDefs * sizeof(LineDef)));
	SendDlgItemMsg(IDC_SD_BYTES, WM_SETTEXT, 0, (LPARAM)FormatNumber((ULONG)NumSideDefs * sizeof(SideDef)));
	SendDlgItemMsg(IDC_SE_BYTES, WM_SETTEXT, 0, (LPARAM)FormatNumber((ULONG)NumSectors  * sizeof(Sector)));
}

