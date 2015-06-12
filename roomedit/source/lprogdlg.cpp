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

	FILE:         lprogdlg.cpp

	OVERVIEW
	========
	Source file for implementation of TLevelProgressDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#ifndef __lprogdlg_h
	#include "lprogdlg.h"
#endif

#ifndef __levels_h
	#include "levels.h"
#endif

#ifndef __objects_h
	#include "objects.h"
#endif



//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TLevelProgressDialog, TDialog)
//{{TLevelProgressDialogRSP_TBL_BEGIN}}
	EV_WM_SIZE,
	EV_WM_PAINT,
//{{TLevelProgressDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TLevelProgressDialog Implementation}}


///////////////////////////////////////////////////////////////
// TLevelProgressDialog
// --------------------
//
TLevelProgressDialog::TLevelProgressDialog (TWindow* parent, TResId resId,
											TModule* module) :
	TDialog(parent, resId, module)
{
	Minimized = FALSE;
	//
	// Create interface object for NODES building
	//
	pNodesText = new TStatic (this, IDC_BUILD_NODES);
	pNodesGauge = new TGauge (this, "%d%%", 200,
							  /*pNodesText->Attr.X,
							  pNodesText->Attr.Y+10,
							  pNodesText->Attr.W,
							  pNodesText->Attr.H*/
							  10, 20, 100, 10);
	pNodesGauge->SetRange (0, 100);
	pNodesGauge->SetValue (0);

	pVertexesStatic = new TStatic (this, IDC_VERTEXES);
	pSideDefsStatic = new TStatic (this, IDC_SIDEDEFS);
	pSegsStatic = new TStatic (this, IDC_SEGS);
	pSSectorsStatic = new TStatic (this, IDC_SSECTORS);

	//
	// Create interface object for REJECT building
	//
	pRejectFrame = new TStatic (this, IDC_FRAME_REJECT);
	// pRejectFrame->Attr.Style &= ~(WS_VISIBLE);

	pRejectText = new TStatic (this, IDC_BUILD_REJECT);
	// pRejectText->Attr.Style &= ~(WS_VISIBLE);

	pRejectGauge = new TGauge (this, "%d%%", 201,
								/* pRejectText->Attr.X,
								pRejectText->Attr.Y+10,
								pRejectText->Attr.W,
								pRejectText->Attr.H*/
								10, 50, 100, 10);
	// pRejectGauge->Attr.Style &= ~(WS_VISIBLE);
	pRejectGauge->SetRange (0, 100);
	pRejectGauge->SetValue (0);

	//
	// Create interface object for BLOCKMAP building
	//
	pBlockmapFrame = new TStatic (this, IDC_FRAME_BLOCKMAP);
	// pBlockmapFrame->Attr.Style &= ~(WS_VISIBLE);

	pBlockmapText = new TStatic (this, IDC_BUILD_BLOCKMAP);
	// pBlockmapText->Attr.Style &= ~(WS_VISIBLE);

	pBlockmapGauge = new TGauge (this, "%d%%", 202,
								 /* pBlockmapText->Attr.X,
								 pBlockmapText->Attr.Y+10,
								 pBlockmapText->Attr.W,
								 pBlockmapText->Attr.H*/
								10, 80, 100, 10);
	// pBlockmapGauge->Attr.Style &= ~(WS_VISIBLE);
	pBlockmapGauge->SetRange (0, 100);
	pBlockmapGauge->SetValue (0);
}


///////////////////////////////////////////////////////////////
// TLevelProgressDialog
// --------------------
//
TLevelProgressDialog::~TLevelProgressDialog ()
{
	Destroy();
}


///////////////////////////////////////////////////////////////
// TLevelProgressDialog
// --------------------
//
void TLevelProgressDialog::SetupWindow ()
{
	TRect wRect, cRect;
	TPoint TopLeft ;

	TDialog::SetupWindow();
	::CenterWindow (this);

	// Setup size and pos. of NODES gauge window
	pNodesText->GetClientRect(cRect);
	pNodesText->GetWindowRect(wRect);
	TopLeft = TPoint(wRect.left, wRect.top);
	ScreenToClient (TopLeft);

	pNodesGauge->MoveWindow (TopLeft.x, TopLeft.y + 16,
							 cRect.right, 20, TRUE);

	// Setup size and pos. of REJECT gauge window
	pRejectText->GetClientRect(cRect);
	pRejectText->GetWindowRect(wRect);
	TopLeft = TPoint(wRect.left, wRect.top);
	ScreenToClient (TopLeft);

	pRejectGauge->MoveWindow (TopLeft.x, TopLeft.y + 16,
							  cRect.right, 20, TRUE);

	// Setup size and pos. of BLOCKMAP gauge window
	pBlockmapText->GetClientRect(cRect);
	pBlockmapText->GetWindowRect(wRect);
	TopLeft = TPoint(wRect.left, wRect.top);
	ScreenToClient (TopLeft);

	pBlockmapGauge->MoveWindow (TopLeft.x, TopLeft.y + 16,
								cRect.right, 20, TRUE);

	/*
	pRejectText->ShowWindow (SW_HIDE);
	pRejectFrame->ShowWindow (SW_HIDE);
	pRejectGauge->ShowWindow (SW_HIDE);
	*/

	/*
	pBlockmapText->ShowWindow (SW_HIDE);
	pBlockmapFrame->ShowWindow (SW_HIDE);
	pBlockmapGauge->ShowWindow (SW_HIDE);
	*/

}


///////////////////////////////////////////////////////////////
// TLevelProgressDialog
// --------------------
//
void TLevelProgressDialog::ShowNodesProgress (int objtype)
{
	static int SavedNumVertexes = 0;
	char msg[10] ;

	switch (objtype)
	{
	case OBJ_VERTEXES:
		wsprintf (msg, "%d", NumVertexes);
		pVertexesStatic->SetText (msg);
		break;

	case OBJ_SIDEDEFS:
		wsprintf (msg, "%d", NumSideDefs);
		pSideDefsStatic->SetText (msg);
		SavedNumVertexes = NumVertexes;
		break;

	case OBJ_SSECTORS:
		wsprintf (msg, "%d", NumSegs);
		pSegsStatic->SetText (msg);

		wsprintf (msg, "%d", NumSectors);
		pSSectorsStatic->SetText (msg);

		//BUG: We must test the division, because of empty levels
		//
		if (NumSideDefs + NumVertexes - SavedNumVertexes > 0 )
		{
			pNodesGauge->SetValue ((int)(100.0f * ((float) NumSegs /
											(float) (NumSideDefs + NumVertexes -
													 SavedNumVertexes))));
		}
		else
			pNodesGauge->SetValue (100);

		// If minimized, we must paint the icon, else we paint the gauge
		if ( Minimized )
		{
			Invalidate(FALSE);	// Don't repaint background
			UpdateWindow();
		}
		else
			pNodesGauge->UpdateWindow();
		break;
   }
}


///////////////////////////////////////////////////////////////
// TLevelProgressDialog
// --------------------
//
void TLevelProgressDialog::ShowRejectControls ()
{
	/*
	pRejectFrame->ShowWindow (SW_SHOW);
	pRejectText->ShowWindow (SW_SHOW);
	pRejectGauge->ShowWindow (SW_SHOW);
	*/
}


///////////////////////////////////////////////////////////////
// TLevelProgressDialog
// --------------------
//
void TLevelProgressDialog::ShowRejectProgress (int value)
{
	pRejectGauge->SetValue (value);
	// If minimized, we must paint the icon, else we paint the gauge
	if ( Minimized )
	{
		Invalidate(FALSE);	// Don't repaint background
		UpdateWindow();
	}
	else
		pRejectGauge->UpdateWindow ();
}


///////////////////////////////////////////////////////////////
// TLevelProgressDialog
// --------------------
//
void TLevelProgressDialog::ShowBlockmapControls ()
{
	/*
	pBlockmapFrame->ShowWindow (SW_SHOW);
	pBlockmapText->ShowWindow (SW_SHOW);
	pBlockmapGauge->ShowWindow (SW_SHOW);
	*/
}


///////////////////////////////////////////////////////////////
// TLevelProgressDialog
// --------------------
//
void TLevelProgressDialog::ShowBlockmapProgress (int value)
{
	pBlockmapGauge->SetValue (value);
	// If minimized, we must paint the icon, else we paint the gauge
	if ( Minimized )
	{
		Invalidate(FALSE);	// Don't repaint background
		UpdateWindow();
	}
	else
		pBlockmapGauge->UpdateWindow ();
}


///////////////////////////////////////////////////////////////
// TLevelProgressDialog
// --------------------
//
void TLevelProgressDialog::EvSize (UINT sizeType, const TSize& size)
{
	if ( sizeType == SIZE_MINIMIZED )
	{
		Minimized = TRUE;
		Parent->ShowWindow(SW_HIDE);
	}
	else if ( sizeType == SIZE_RESTORED )
	{
		Parent->ShowWindow(SW_SHOW);
		Minimized = FALSE;
	}
	else
	{
		Minimized = FALSE;
		TDialog::EvSize(sizeType, size);
	}
}


///////////////////////////////////////////////////////////////
// TLevelProgressDialog
// --------------------
//
void DrawIconicGauge(TDC &dc, int yPosition, int Width, int Height, int Value)
{
	TBrush BleueBrush(TColor(0,0,255));
	TPen BleuePen (TColor(0,0,255));
	// TBrush GrayBrush (TColor(200,200,200));
	TPen WhitePen (TColor(255,255,255));
	TPen GrayPen (TColor(100,100,100));
	// TPen GrayPen (TColor(0,0,0));

	// Draw upper left white border
	dc.SelectObject(GrayPen);
	dc.MoveTo(0, yPosition + Height - 1);
	dc.LineTo(0, yPosition);
	dc.LineTo(Width - 1, yPosition);

	// Draw lower right border
	dc.SelectObject(WhitePen);
	dc.LineTo(Width - 1, yPosition + Height - 1);
	dc.LineTo(0, yPosition + Height - 1);

	// Draw gauge
	dc.SelectObject(BleueBrush);
	dc.SelectObject(BleuePen);
	dc.Rectangle(1, yPosition + 1, (Width - 1) * Value / 100, yPosition + Height - 1);

	dc.RestoreObjects();
}


///////////////////////////////////////////////////////////////
// TLevelProgressDialog
// --------------------
//
void TLevelProgressDialog::EvPaint ()
{
	if ( Minimized )
	{
		TPaintDC dc(*this);
		TRect ClientRect;
		// Get size of iconic window
		GetClientRect(ClientRect);
		int Width = ClientRect.Width();
		int Height = ClientRect.Height();

		DrawIconicGauge (dc, 0,
						 Width, Height / 3, pNodesGauge->GetValue());
		DrawIconicGauge (dc, Height / 3,
						 Width, Height / 3, pRejectGauge->GetValue());
		DrawIconicGauge (dc, 2 * Height / 3,
						 Width, Height / 3, pBlockmapGauge->GetValue());
	}
	else
		TDialog::EvPaint();
}

