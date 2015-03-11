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

	FILE:         viewbmp.cpp

	OVERVIEW
	========
	Source file for implementation of TViewBitmapDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#ifndef __viewbmp_h
	#include "viewbmp.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_LISTBOX_H
	#include <owl\listbox.h>
#endif

#ifndef OWL_STATIC_H
	#include <owl\static.h>
#endif

#ifndef OWL_SLIDER_H
	#include <owl\slider.h>
#endif

#ifndef __bmp256ct_h
	#include "bmp256ct.h"
#endif

#ifndef __wads_h
	#include "wads.h"
#endif

#ifndef __levels_h
	#include "levels.h"		// Floor/Ceiling texture names
#endif


// System palette elements
/*
int DspElements[21] =
{
	COLOR_ACTIVEBORDER,
	COLOR_ACTIVECAPTION,
	COLOR_APPWORKSPACE,
	COLOR_BACKGROUND,
	COLOR_BTNFACE,
	COLOR_BTNHIGHLIGHT,
	COLOR_BTNSHADOW,
	COLOR_BTNTEXT,
	COLOR_CAPTIONTEXT,
	COLOR_GRAYTEXT,
	COLOR_HIGHLIGHT,
	COLOR_HIGHLIGHTTEXT,
	COLOR_INACTIVEBORDER,
	COLOR_INACTIVECAPTION,
	COLOR_INACTIVECAPTIONTEXT,
	COLOR_MENU,
	COLOR_MENUTEXT,
	COLOR_SCROLLBAR,
	COLOR_WINDOW,
	COLOR_WINDOWFRAME,
	COLOR_WINDOWTEXT,
};
*/

//////////////////////////////////////////////////////////////////////////
//
//
//  TViewBitmapDialog implementation
//
//
//////////////////////////////////////////////////////////////////////////


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TViewBitmapDialog, TDialog)
//{{TViewBitmapDialogRSP_TBL_BEGIN}}
	EV_WM_SETFOCUS,
	EV_CHILD_NOTIFY_ALL_CODES(IDC_ZOOM_SLIDER, UpdateZoom),
	EV_CHILD_NOTIFY_ALL_CODES(IDC_GAMMA_SLIDER, UpdateGamma),
//{{TViewBitmapDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TViewBitmapDialog Implementation}}

////////////////////////////////////////////////////////////
// TViewBitmapDialog
// -----------------
//
TViewBitmapDialog::TViewBitmapDialog (TWindow* parent, TResId resId,
										TModule* module):
	TDialog(parent, resId, module)
{
	pSizeStatic = new TStatic (this, IDC_BITMAP_SIZE);

	pZoomFrame = new TStatic (this, IDC_ZOOM_FRAME);
	pZoomStatic = new TStatic (this, IDC_ZOOM_FACTOR);

	pGammaFrame = new TStatic (this, IDC_GAMMA_FRAME);
	pGammaStatic = new TStatic (this, IDC_GAMMA_LEVEL);

	// UseSystemPalette = FALSE;
}


////////////////////////////////////////////////////////////
// TViewBitmapDialog
// -----------------
//
TViewBitmapDialog::~TViewBitmapDialog ()
{
	TRACE ("TViewBitmapDialog: destructor called");
	Destroy();
	delete pGammaSlider;
	delete pZoomSlider;
	delete pBitmapControl;
}


////////////////////////////////////////////////////////////
// TViewBitmapDialog
// -----------------
//  Load the DOOM palette and add the sprite names in list box
void TViewBitmapDialog::SetupWindow ()
{
	TDialog::SetupWindow();
	::CenterWindow (this);

	// Setup control window size
	TRect cRect = pSizeStatic->GetClientRect ();
	BitmapControlW = cRect.right;
	BitmapControlH = cRect.bottom;
	TPoint UpLeft(cRect.left, cRect.top);

	pSizeStatic->ClientToScreen (UpLeft);
	ScreenToClient (UpLeft);
	BitmapControlX = UpLeft.x;
	BitmapControlY = UpLeft.y;

	//
	// Create zoom slider with same size as the its frame
	//
	TRect wRect;
	TPoint TopLeft;

	pZoomFrame->GetWindowRect (wRect);
	TopLeft = TPoint (wRect.left, wRect.top);
	ScreenToClient (TopLeft);
	pZoomSlider = new TVSlider (this, IDC_ZOOM_SLIDER,
								TopLeft.x+1, TopLeft.y+1,
								wRect.Width()-2, wRect.Height()-2);
	// Hide the slider frame
	pZoomFrame->ShowWindow (SW_HIDE);
	// Create the slider
	pZoomSlider->Create();
	pZoomSlider->SetRange (1, 16);
	pZoomSlider->SetRuler (3, FALSE);
	pZoomSlider->SetPosition (1);

	//
	// Create Gamma Level slider with same size as the its frame
	//
	pGammaFrame->GetWindowRect (wRect);
	TopLeft = TPoint (wRect.left, wRect.top);
	ScreenToClient (TopLeft);
	pGammaSlider = new TVSlider (this, IDC_GAMMA_SLIDER,
								TopLeft.x+1, TopLeft.y+1,
								wRect.Width()-2, wRect.Height()-2);
	// Hide the slider frame
	pGammaFrame->ShowWindow (SW_HIDE);
	// Create the slider
	pGammaSlider->Create();
	pGammaSlider->SetRange (0, 120);
	pGammaSlider->SetRuler (10, TRUE);
	pGammaSlider->SetPosition (0);

	// Creates bitmap control (virtual function of derived classes)
	InitBitmapControl ();
	CHECK (pBitmapControl != NULL);

	// Create the control window
	pSizeStatic->ShowWindow (SW_HIDE);
	pBitmapControl->Create();

	// Draw and set position of zoom and gamma slider
	char zoomtext[5];
	sprintf (zoomtext, "%u", pBitmapControl->GetZoomFactor());
	pZoomStatic->SetText (zoomtext);
	pZoomSlider->SetPosition (pBitmapControl->GetZoomFactor());

	char gtext[5];
	sprintf (gtext, "%u", pBitmapControl->GetGammaLevel());
	pGammaStatic->SetText (gtext);
	pGammaSlider->SetPosition (pBitmapControl->GetGammaLevel());

	// Realize palette in window
	/*
	if ( UseSystemPalette )
	{
		TScreenDC dc;
		dc.SetSystemPaletteUse (SYSPAL_NOSTATIC);
		dc.SelectObject (*pDoomPalette);
		dc.RealizePalette ();
		SaveSystemPalette();
	}
	*/
}


////////////////////////////////////////////////////////////
// TViewBitmapDialog
// -----------------
//
void TViewBitmapDialog::CloseWindow (int retVal)
{
	// Realize palette in window
	/*
	if ( UseSystemPalette )
	{
		TScreenDC dc;
		dc.SetSystemPaletteUse (SYSPAL_STATIC);
		dc.SelectObject (*pDoomPalette);
		dc.RealizePalette ();
		RestoreSystemPalette();
	}
	*/

	TDialog::CloseWindow(retVal);
}


////////////////////////////////////////////////////////////
// TViewBitmapDialog
// -----------------
//
void TViewBitmapDialog::UpdateZoom (UINT)
{
	PRECONDITION (pBitmapControl != NULL);
	PRECONDITION (pZoomSlider != NULL);
	PRECONDITION (pZoomStatic != NULL);

	pBitmapControl->SetZoomFactor(pZoomSlider->GetPosition());

	char text[5];
	sprintf (text, "%u", pBitmapControl->GetZoomFactor());
	pZoomStatic->SetText (text);
}


////////////////////////////////////////////////////////////
// TViewBitmapDialog
// -----------------
//
void TViewBitmapDialog::UpdateGamma (UINT)
{
	PRECONDITION (pBitmapControl != NULL);
	PRECONDITION (pGammaSlider != NULL);
	PRECONDITION (pGammaStatic != NULL);

	pBitmapControl->SetGammaLevel (pGammaSlider->GetPosition());

	char text[5];
	sprintf (text, "%u", pBitmapControl->GetGammaLevel());
	pGammaStatic->SetText (text);
}


////////////////////////////////////////////////////////////
// TViewBitmapDialog
// -----------------
//
void TViewBitmapDialog::EvSetFocus (HWND hWndLostFocus )
{
	TDialog::EvSetFocus(hWndLostFocus );

	pBitmapControl->Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//  TViewBitmapListDialog implementation
//
//
//////////////////////////////////////////////////////////////////////////

//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TViewBitmapListDialog, TViewBitmapDialog)
	EV_LBN_DBLCLK(IDC_BITMAP_LIST, LBNDblclk),
	EV_LBN_SELCHANGE(IDC_BITMAP_LIST, LBNSelchange),
END_RESPONSE_TABLE;



////////////////////////////////////////////////////////////
// TViewBitmapListDialog
// -----------------
//
TViewBitmapListDialog::TViewBitmapListDialog (TWindow* parent, TResId resId,
											  TModule* module):
	TViewBitmapDialog(parent, resId, module)
{
	pBitmapList  = new TListBox (this, IDC_BITMAP_LIST);
	pChooseStatic = new TStatic (this, IDC_CHOOSE);
}


////////////////////////////////////////////////////////////
// TViewBitmapListDialog
// -----------------
//
TViewBitmapListDialog::~TViewBitmapListDialog ()
{
	TRACE ("TViewBitmapListDialog: destructor called");
	Destroy();
}


////////////////////////////////////////////////////////////
// TViewBitmapListDialog
// -----------------
//
void TViewBitmapListDialog::SetupWindow ()
{
	TViewBitmapDialog::SetupWindow();
	::CenterWindow (this);
}


////////////////////////////////////////////////////////////
// TViewBitmapListDialog
// -----------------
//
int TViewBitmapListDialog::SetSelection (const char *str)
{
#if OWLVersion > OWLVERBC502
	int SelIndex = pBitmapList->FindStringExact(str, -1);
#else
	int SelIndex = pBitmapList->FindExactString(str, -1);
#endif

	if ( SelIndex >= 0 )
	{
		pBitmapList->SetSelIndex (SelIndex);
		LBNSelchange();		// Display bitmap
		/*
		pBitmapList->SendMessage (LB_SETCARETINDEX,
								  (WPARAM)SelIndex, MAKELPARAM(FALSE, 0));
		*/
	}
	return SelIndex;
}


////////////////////////////////////////////////////////////
// TViewBitmapListDialog
// -----------------
//	Display sprite whose name was double clicked
void TViewBitmapListDialog::LBNDblclk ()
{
	char BitmapName[MAX_BITMAPNAME + 1];

	if ( pBitmapList->GetSelString (BitmapName, MAX_BITMAPNAME) > 1 )
	{
	   TextureInfo *info = FindTextureByName(BitmapName);
	   pBitmapControl->SelectBitmap2 (info->filename);
	}
}


////////////////////////////////////////////////////////////
// TViewBitmapListDialog
// -----------------
//	Display sprite whose name was clicked
void TViewBitmapListDialog::LBNSelchange ()
{
	char BitmapName[MAX_BITMAPNAME + 1];

	if ( pBitmapList->GetSelString (BitmapName, MAX_BITMAPNAME) > 1 )
	{
	   TextureInfo *info = FindTextureByName(BitmapName);
	   pBitmapControl->SelectBitmap2 (info->filename);
	   SetCaption(info->filename);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//  TViewSpriteDialog implementation
//
//
//////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////
// TViewSpriteDialog
// -----------------
//
TViewSpriteDialog::TViewSpriteDialog (TWindow* parent, TResId resId,
									  TModule* module):
	TViewBitmapListDialog(parent, resId, module)
{
}


////////////////////////////////////////////////////////////
// TViewSpriteDialog
// -----------------
//
TViewSpriteDialog::~TViewSpriteDialog ()
{
	TRACE ("TViewSpriteDialog: destructor called");
	Destroy();
}


////////////////////////////////////////////////////////////
// TViewSpriteDialog
// -----------------
//
void
TViewSpriteDialog::InitBitmapControl ()
{
	// Create the sprite control window
	pBitmapControl = new TSprite256Control (this, IDC_BITMAP_CONTROL, "",
											BitmapControlX, BitmapControlY,
											BitmapControlW, BitmapControlH);
}


////////////////////////////////////////////////////////////
// TViewSpriteDialog
// -----------------
//
void TViewSpriteDialog::SetupWindow ()
{
	TViewBitmapListDialog::SetupWindow();

	SetCaption ("Viewing Sprites");
	pChooseStatic->SetCaption ("Choose sprite name:");

	// Insert sprites names in list box
	MDirPtr dir = FindMasterDir( MasterDir, "S_START");
	for (dir = dir->next ;
		 dir != NULL && strcmp(dir->dir.name, "S_END") ;
		 dir = dir->next)
	{
		char str[MAX_BITMAPNAME + 1];

		strncpy (str, dir->dir.name, MAX_BITMAPNAME);
		str[MAX_BITMAPNAME] = '\0';

		pBitmapList->AddString (str);
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//  TViewWallTexture implementation
//
//
//////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// TViewWallTextureDialog
// -----------------
//
TViewWallTextureDialog::TViewWallTextureDialog (TWindow* parent, TResId resId,
									  TModule* module):
	TViewBitmapListDialog(parent, resId, module)
{
}


////////////////////////////////////////////////////////////
// TViewWallTextureDialog
// -----------------
//
TViewWallTextureDialog::~TViewWallTextureDialog ()
{
	TRACE ("TViewWallTextureDialog: destructor called");
	Destroy();
//	ForgetWTextureInfo();
}


////////////////////////////////////////////////////////////
// TViewWallTextureDialog
// -----------------
//
void
TViewWallTextureDialog::InitBitmapControl ()
{
	// Create the sprite control window
	pBitmapControl = new TWallTextureControl (this, IDC_BITMAP_CONTROL, "",
											  BitmapControlX, BitmapControlY,
											  BitmapControlW, BitmapControlH);
}


////////////////////////////////////////////////////////////
// TViewWallTextureDialog
// -----------------
//
void TViewWallTextureDialog::SetupWindow ()
{
	TViewBitmapListDialog::SetupWindow();

	SetCaption ("Viewing Wall textures");
	pChooseStatic->SetCaption ("Choose texture name:");

	// Insert wall textures names in list box
	assert (pBitmapList->IsWindow());

	//BOOL ForgetInfo = FALSE;
	if ( NumWTexture <= 0 )
	{
		ReadWTextureInfo();
		//ForgetInfo = TRUE;
	}
	assert (WTexture != NULL);

	pBitmapList->ClearList();

	// Add Wall texture names (except the first: "-")
	for (SHORT i = 1 ; i < NumWTexture ; i++)
	{
		assert (WTexture[i] != NULL);
		pBitmapList->AddString (WTexture[i]->Name);
	}

//	if ( ForgetInfo == TRUE )
//		ForgetWTextureInfo();
}


//////////////////////////////////////////////////////////////////////////
//
//
//  TViewFloorTexture implementation
//
//
//////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// TViewFloorTextureDialog
// -----------------
//
TViewFloorTextureDialog::TViewFloorTextureDialog (TWindow* parent, TResId resId,
									  TModule* module):
	TViewBitmapListDialog(parent, resId, module)
{
}


////////////////////////////////////////////////////////////
// TViewFloorTextureDialog
// -----------------
//
TViewFloorTextureDialog::~TViewFloorTextureDialog ()
{
	TRACE ("TViewFloorTextureDialog: destructor called");
	Destroy();
}


////////////////////////////////////////////////////////////
// TViewFloorTextureDialog
// -----------------
//
void
TViewFloorTextureDialog::InitBitmapControl ()
{
	// Create the floor texture control window
        // Wall texture control identical ARK
//	pBitmapControl = new TFloorTextureControl (this, IDC_BITMAP_CONTROL, "",
	pBitmapControl = new TWallTextureControl (this, IDC_BITMAP_CONTROL, "",
											   BitmapControlX, BitmapControlY,
											   BitmapControlW, BitmapControlH);
}


////////////////////////////////////////////////////////////
// TViewFloorTextureDialog
// -----------------
//
void TViewFloorTextureDialog::SetupWindow ()
{
	TViewBitmapListDialog::SetupWindow();

	SetCaption ("Viewing Floor/Ceiling textures");
	pChooseStatic->SetCaption ("Choose texture name:");

	// Insert wall textures names in list box

	BOOL ForgetInfo = FALSE;
	if ( NumFTexture <= 0 )
	{
		ReadFTextureInfo();
		ForgetInfo = TRUE;
	}
	assert (FTexture != NULL);

	assert (pBitmapList->IsWindow());
	pBitmapList->ClearList();

	// Add texture names
	for (SHORT i = 0 ; i < NumFTexture ; i++)
	{
		assert (FTexture[i] != NULL);
		pBitmapList->AddString (FTexture[i]->Name);
	}

//	if ( ForgetInfo == TRUE )
//		ForgetFTextureInfo();
}


//////////////////////////////////////////////////////////////////////////
//
//
//  TDisplayBitmapDialog implementation
//
//
//////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// TDisplayBitmapDialog
// --------------------
//
TDisplayBitmapDialog::TDisplayBitmapDialog (TWindow* parent, TResId resId,
										   TModule* module):
	TViewBitmapDialog(parent, resId, module)
{
}


////////////////////////////////////////////////////////////
// TDisplayBitmapDialog
// -------------------
//
TDisplayBitmapDialog::~TDisplayBitmapDialog ()
{
	Destroy();
}


////////////////////////////////////////////////////////////
// TDisplayBitmapDialog
// -------------------
//
void TDisplayBitmapDialog::SetupWindow ()
{
	TViewBitmapDialog::SetupWindow();

	// Move window to top left of screen
	SetWindowPos (NULL,
				  0, 0, 0, 0,
				  SWP_NOSIZE | SWP_NOZORDER);
}


////////////////////////////////////////////////////////////
// TDisplayBitmapDialog
// -------------------
//
int TDisplayBitmapDialog::SelectBitmap2 (const char *str)
{
	pBitmapControl->SelectBitmap2 (str);

	SetCaption(str);

	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//  TDisplaySpriteDialog implementation
//
//
//////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////
// TDisplaySpriteDialog
// --------------------
//
TDisplaySpriteDialog::TDisplaySpriteDialog (TWindow* parent, TResId resId,
											TModule* module):
	TDisplayBitmapDialog(parent, resId, module)
{
}


////////////////////////////////////////////////////////////
// TDisplaySpriteDialog
// --------------------
//
TDisplaySpriteDialog::~TDisplaySpriteDialog ()
{
	// TRACE ("TViewSpriteDialog: destructor called");
	Destroy();
}


////////////////////////////////////////////////////////////
// TDisplaySpriteDialog
// --------------------
//
void TDisplaySpriteDialog::InitBitmapControl ()
{
	// Create the sprite control window
	pBitmapControl = new TSprite256Control (this, IDC_BITMAP_CONTROL, "",
											BitmapControlX, BitmapControlY,
											BitmapControlW, BitmapControlH);
}


////////////////////////////////////////////////////////////
// TDisplaySpriteDialog
// --------------------
//
void TDisplaySpriteDialog::SetupWindow ()
{
	TDisplayBitmapDialog::SetupWindow();

	SetCaption ("Viewing Sprites");
}



//////////////////////////////////////////////////////////////////////////
//
//
//  TDisplayWallTextureDialog implementation
//
//
//////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// TDisplayWallTextureDialog
// -------------------------
//
TDisplayWallTextureDialog::TDisplayWallTextureDialog (TWindow* parent,
													  TResId resId,
													  TModule* module):
	TDisplayBitmapDialog(parent, resId, module)
{
}


////////////////////////////////////////////////////////////
// TDisplayWallTextureDialog
// -------------------------
//
TDisplayWallTextureDialog::~TDisplayWallTextureDialog ()
{
	// TRACE ("TDisplayWallTextureDialog: destructor called");
	Destroy();
}


////////////////////////////////////////////////////////////
// TDisplayWallTextureDialog
// -------------------------
//
void TDisplayWallTextureDialog::InitBitmapControl ()
{
	// Create the sprite control window
	pBitmapControl = new TWallTextureControl (this, IDC_BITMAP_CONTROL, "",
											  BitmapControlX, BitmapControlY,
											  BitmapControlW, BitmapControlH);
}


////////////////////////////////////////////////////////////
// TDisplayWallTextureDialog
// -------------------------
//
void TDisplayWallTextureDialog::SetupWindow ()
{
	TDisplayBitmapDialog::SetupWindow();

	SetCaption ("Viewing Wall textures");
}


//////////////////////////////////////////////////////////////////////////
//
//
//  TDisplayWallTextureDialog implementation
//
//
//////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// TDisplayFloorTextureDialog
// --------------------------
//
TDisplayFloorTextureDialog::TDisplayFloorTextureDialog (TWindow* parent,
														TResId resId,
														TModule* module):
	TDisplayBitmapDialog(parent, resId, module)
{
}


////////////////////////////////////////////////////////////
// TDisplayFloorTextureDialog
// --------------------------
//
TDisplayFloorTextureDialog::~TDisplayFloorTextureDialog ()
{
	// TRACE ("TDisplayFloorTextureDialog: destructor called");
	Destroy();
}


////////////////////////////////////////////////////////////
// TDisplayFloorTextureDialog
// --------------------------
//
void TDisplayFloorTextureDialog::InitBitmapControl ()
{
	// Create the floor texture control window
        // Wall texture control identical ARK
//	pBitmapControl = new TFloorTextureControl (this, IDC_BITMAP_CONTROL, "",
	pBitmapControl = new TWallTextureControl (this, IDC_BITMAP_CONTROL, "",
											   BitmapControlX, BitmapControlY,
											   BitmapControlW, BitmapControlH);
}


////////////////////////////////////////////////////////////
// TDisplayFloorTextureDialog
// --------------------------
//
void TDisplayFloorTextureDialog::SetupWindow ()
{
	TDisplayBitmapDialog::SetupWindow();

	SetCaption ("Viewing Floor/Ceiling textures");
}

