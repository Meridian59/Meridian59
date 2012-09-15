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

	FILE:         vertdlg.cpp

	OVERVIEW
	========
	Source file for implementation of TVertexDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop


#include "vertdlg.h"

#ifndef __cnfvedlg_h
	#include "cnfvedlg.h"
#endif

#ifndef __newers_h
	#include "newers.h"
#endif

#ifndef __gfx_h
	#include "gfx.h"
#endif

// HELP IDs
#include "windeuhl.h"

//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TVertexDialog, TDialog)
//{{TVertexDialogRSP_TBL_BEGIN}}
	EV_BN_CLICKED(IDOK, CmOk),
//{{TVertexDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TVertexDialog Implementation}}


TVertexDialog::TVertexDialog (TWindow* parent, SelPtr SelectedVertices, TResId resId, TModule* module):
	TDialog(parent, resId, module)
{
	SelVertices = SelectedVertices;
	CurVertex = Vertexes[SelVertices->objnum];
	memset(&ConfirmData, 0, sizeof(ConfirmData));

	pXPosEdit = newTEdit(this, IDC_VERTEX_XPOS, 7);
	pXPosEdit->SetValidator(new TRangeValidator(min(MapMinX, MAP_MIN_X),
												max(MapMaxX, MAP_MAX_X)));
	pYPosEdit = newTEdit(this, IDC_VERTEX_YPOS, 7);
	pYPosEdit->SetValidator(new TRangeValidator(min(MapMinY, MAP_MIN_Y),
												max(MapMaxY, MAP_MAX_Y)));
}


TVertexDialog::~TVertexDialog ()
{
	Destroy();
}


void TVertexDialog::SetupWindow ()
{
	TDialog::SetupWindow();
	::CenterWindow(this);

	char tmp[20];
	wsprintf(tmp, "%d", CurVertex.x);
	pXPosEdit->SetText(tmp);

	wsprintf(tmp, "%d", CurVertex.y);
	pYPosEdit->SetText(tmp);
}


void TVertexDialog::CmOk ()
{
	// Cannot close if edit controls not valid
	if ( !CanClose() )
		return;

	// Get vertex XPos and YPos
	char tmp[10];

	pXPosEdit->GetText(tmp, 9);
	ConfirmData.pXPosCheck = (CurVertex.x != (SHORT)atoi(tmp));
	CurVertex.x = atoi(tmp);

	pYPosEdit->GetText(tmp, 9);
	ConfirmData.pYPosCheck = (CurVertex.y != (SHORT)atoi(tmp));
	CurVertex.y = atoi(tmp);

	// Update map max and min coordinates
	if (CurVertex.x < MapMinX ) 	MapMinX = CurVertex.x;
	if (CurVertex.x > MapMaxX ) 	MapMaxX = CurVertex.x;
	if (CurVertex.y < MapMinY ) 	MapMinY = CurVertex.y;
	if (CurVertex.y > MapMaxY ) 	MapMaxY = CurVertex.y;

	// Do we made changes ?
	VPtr pVertex = &Vertexes[SelVertices->objnum];

	if ( pVertex->x  != CurVertex.x ||
		 pVertex->y  != CurVertex.y )
	{
		MadeChanges = TRUE;
		MadeMapChanges = TRUE;
	}

	// Update first thing in list
	*pVertex = CurVertex;

	// If more than one thing, copy selected attributes to them
	if ( SelVertices->next != NULL )
	{
		SET_HELP_CONTEXT(Confirming_copy_of_Vertex_attributes);
		if ( TConfirmVertexDialog (this, ConfirmData).Execute() == IDOK )
		{
			// Copy the selected attributes of CurVertex to the selection list
			for (SelPtr cur = SelVertices->next ; cur != NULL ; cur = cur->next )
			{
				VPtr pVertex = &Vertexes[cur->objnum];
				Vertex VertexBefore = *pVertex;	// Copy before changes

				if ( ConfirmData.pXPosCheck )		pVertex->x  = CurVertex.x;
				if ( ConfirmData.pYPosCheck )		pVertex->y  = CurVertex.y;

				// Did we made changes?
				if ( memcmp(pVertex, &VertexBefore, sizeof(Vertex)) != 0 )
				{
					MadeChanges;
					MadeMapChanges;
				}
			}
		}
		RESTORE_HELP_CONTEXT();
	}

	// Close dialog box
	TDialog::CmOk();
}

