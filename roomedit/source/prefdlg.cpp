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

	FILE:         prefdlg.cpp

	OVERVIEW
	========
	Source file for implementation of TPreferencesDialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#ifndef __prefdlg_h
	#include "prefdlg.h"
#endif

#ifndef OWL_LISTBOX_H
	#include <owl\listbox.h>
#endif

#ifndef OWL_STATIC_H
	#include <owl\static.h>
#endif

#ifndef OWL_EDIT_H
	#include <owl\edit.h>
#endif

#ifndef OWL_VALIDATE_H
	#include <owl\validate.h>
#endif

#ifndef __viewbmp_h
	#include "viewbmp.h"
#endif

#ifndef __levels_h
	#include "levels.h"
#endif

#ifndef __undo_h
	#include "undo.h"
#endif

#ifndef __newers_h
	#include "newers.h"
#endif

#ifndef __mainfram_h
	#include "mainfram.h"
#endif

#ifndef __windeapp_h
	#include "windeapp.h"
#endif

//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TPreferencesDialog, TDialog)
//{{TPreferencesDialogRSP_TBL_BEGIN}}
	EV_BN_CLICKED(IDOK, CmOk),
	EV_BN_CLICKED(IDC_TO_NORMAL, ToNormalClicked),
	EV_BN_CLICKED(IDC_TO_FLOOR, ToFloorClicked),
	EV_BN_CLICKED(IDC_TO_CEILING, ToCeilingClicked),
	EV_BN_CLICKED(IDC_TO_BELOW, ToBelowClicked),
	EV_BN_CLICKED(IDC_TO_ABOVE, ToAboveClicked),
	EV_BN_CLICKED(IDC_NORMAL_CLEAR, ClearNormalClicked),
	EV_LBN_SELCHANGE(IDC_FTEXTURE_LIST, FTextureSelChange),
	EV_LBN_DBLCLK(IDC_FTEXTURE_LIST, FTextureDblClk),
	EV_BN_CLICKED(IDC_FLOOR_CLEAR, ClearFloorClicked),
	EV_BN_CLICKED(IDC_CEILING_CLEAR, ClearCeilingClicked),
	EV_BN_CLICKED(IDC_BELOW_CLEAR, ClearBelowClicked),
	EV_BN_CLICKED(IDC_ABOVE_CLEAR, ClearAboveClicked),
	EV_LBN_SELCHANGE(IDC_WTEXTURE_LIST, WTextureSelChange),
	EV_LBN_DBLCLK(IDC_WTEXTURE_LIST, WTextureDblClk),
	EV_WM_LBUTTONDOWN,
	EV_WM_LBUTTONDBLCLK,
//{{TPreferencesDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TPreferencesDialog Implementation}}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
TPreferencesDialog::TPreferencesDialog (TWindow* parent, TResId resId, TModule* module):
	TDialog(parent, resId, module)
{
	memset (WTextureName, 0, MAX_BITMAPNAME + 1);
	memset (FTextureName, 0, MAX_BITMAPNAME + 1);

	pWTextureDialog = NULL;
	pFTextureDialog = NULL;

	pWTextureList  = newTListBox(this, IDC_WTEXTURE_LIST);
	pFTextureList  = newTListBox(this, IDC_FTEXTURE_LIST);
	pFloorHeight   = newTEdit(this, IDC_FLOOR_HEIGHT, 6);
	pNormalText    = newTStatic(this, IDC_NORMAL_TEXT, MAX_BITMAPNAME + 1);
	pFloorText     = newTStatic(this, IDC_FLOOR_TEXT, MAX_BITMAPNAME + 1);
	pCeilingText   = newTStatic(this, IDC_CEILING_TEXT, MAX_BITMAPNAME + 1);
	pCeilingHeight = newTEdit(this, IDC_CEILING_HEIGHT, 6);
	pBelowText     = newTStatic(this, IDC_BELOW_TEXT, MAX_BITMAPNAME + 1);
	pAboveText     = newTStatic(this, IDC_ABOVE_TEXT, MAX_BITMAPNAME + 1);

	pAddSelBoxCheck  = newTCheckBox(this, IDC_PREF_ADDSELBOX, 0);
	pDebugCheck      = newTCheckBox(this, IDC_PREF_DEBUG, 0);
	pDrawLengthCheck = newTCheckBox(this, IDC_PREF_DRAWLENGTH, 0);
	pExpertCheck     = newTCheckBox(this, IDC_PREF_EXPERT, 0);
	pInfoBarCheck    = newTCheckBox(this, IDC_PREF_INFOBAR, 0);
	pQuietCheck      = newTCheckBox(this, IDC_PREF_QUIET, 0);
	pQuiterCheck     = newTCheckBox(this, IDC_PREF_QUIETER, 0);
	pSelect0Check    = newTCheckBox(this, IDC_PREF_SELECT0, 0);
	pAutoScrollCheck = newTCheckBox(this, IDC_PREF_AUTOSCROLL, 0);

	pMaxUndoEdit = newTEdit(this, IDC_PREF_MAXUNDO, 6);

	pCeilingHeight->SetValidator (new TRangeValidator (-16384, 16383));
	pFloorHeight->SetValidator (new TRangeValidator (-16384, 16383));

	pMaxUndoEdit->SetValidator(new TRangeValidator(1, 100));
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
TPreferencesDialog::~TPreferencesDialog ()
{
	delete pWTextureDialog;
	delete pFTextureDialog;
	Destroy();
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::SetupWindow ()
{
	SHORT i;

	TDialog::SetupWindow();
	CenterWindow(this);

	// Init Wall Texture list
	assert (WTexture != NULL);
	for (i = 0 ; i < NumWTexture ; i++)
	{
		assert (WTexture[i] != NULL);
		pWTextureList->AddString (WTexture[i]->Name);
	}

	// Init Floor/Ceiling Texture list
	assert (FTexture != NULL);
	for (i = 0 ; i < NumFTexture ; i++)
	{
		assert (FTexture[i] != NULL);
		pFTextureList->AddString (FTexture[i]->Name);
	}

	pAboveText->SetText (DefaultUpperTexture);
	pNormalText->SetText (DefaultWallTexture);
	pBelowText->SetText (DefaultLowerTexture);
	pFloorText->SetText (DefaultFloorTexture);
	pCeilingText->SetText (DefaultCeilingTexture);

	// Floor/ceiling height
	char str[MAX_BITMAPNAME];
	wsprintf (str, "%d", DefaultFloorHeight);
	pFloorHeight->SetText (str);
	wsprintf (str, "%d", DefaultCeilingHeight);
	pCeilingHeight->SetText (str);

	wsprintf (str, "%d", MaxUndo);
	pMaxUndoEdit->SetText (str);

	// Misc BOOLEAN options
	pAddSelBoxCheck->SetCheck(AdditiveSelBox ? BF_CHECKED : BF_UNCHECKED);
	pDebugCheck->SetCheck(Debug ? BF_CHECKED : BF_UNCHECKED);
	pDrawLengthCheck->SetCheck(DrawLineDefsLen ? BF_CHECKED : BF_UNCHECKED);
	pExpertCheck->SetCheck(Expert ? BF_CHECKED : BF_UNCHECKED);
	pInfoBarCheck->SetCheck(InfoShown ? BF_CHECKED : BF_UNCHECKED);
	pQuietCheck->SetCheck(Quiet ? BF_CHECKED : BF_UNCHECKED);
	pQuiterCheck->SetCheck(Quieter ? BF_CHECKED : BF_UNCHECKED);
	pSelect0Check->SetCheck(Select0 ? BF_CHECKED : BF_UNCHECKED);
	pAutoScrollCheck->SetCheck(AutoScroll ? BF_CHECKED : BF_UNCHECKED);
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::CmOk ()
{
	char str[MAX_BITMAPNAME + 1];

	if ( !CanClose() )
		return;

	// Get MAX UNDO
	int max_undo;
	pMaxUndoEdit->GetText(str, MAX_BITMAPNAME + 1);
	max_undo = atoi(str);
	if ( max_undo != MaxUndo )
	{
		if ( (GetUndoName() == NULL && GetRedoName() == NULL) ||
			 Confirm("WARNING: Changing the maximum number of UNDO/REDO "
					 "will free up current UNDO/REDO structure(s).\n"
					 "Are you sure you want to do this ?") )
		{
			CleanupUndo();
			MaxUndo = max_undo;
			InitUndo();
		}
		else
		{
			wsprintf(str, "%d", MaxUndo);
			pMaxUndoEdit->SetText(str);
			return;
		}
	}


	// Floor/Ceiling Texture
	pAboveText->GetText (str, MAX_BITMAPNAME + 1);
	FreeMemory (DefaultUpperTexture);
	DefaultUpperTexture = (char *)GetMemory (strlen(str)+1);
	strcpy (DefaultUpperTexture, str);

	pNormalText->GetText (str, MAX_BITMAPNAME + 1);
	FreeMemory (DefaultWallTexture);
	DefaultWallTexture = (char *)GetMemory (strlen(str)+1);
	strcpy (DefaultWallTexture, str);

	pBelowText->GetText (str, MAX_BITMAPNAME + 1);
	FreeMemory (DefaultLowerTexture);
	DefaultLowerTexture = (char *)GetMemory (strlen(str)+1);
	strcpy (DefaultLowerTexture, str);

	pFloorText->GetText (str, MAX_BITMAPNAME + 1);
	FreeMemory (DefaultFloorTexture);
	DefaultFloorTexture = (char *)GetMemory (strlen(str)+1);
	strcpy (DefaultFloorTexture, str);

	pCeilingText->GetText (str, MAX_BITMAPNAME + 1);
	FreeMemory (DefaultCeilingTexture);
	DefaultCeilingTexture = (char *)GetMemory (strlen(str)+1);
	strcpy (DefaultCeilingTexture, str);

	// Floor/Ceiling height
	pFloorHeight->GetText (str, 6);
	DefaultFloorHeight = (SHORT)atoi(str);

	pCeilingHeight->GetText (str, 6);
	DefaultCeilingHeight = (SHORT)atoi(str);

#if (COOPERATION_VERSION == 1)
	BuildCoopExecTab();
#endif	// COOPERATION_VERSION


	// Check grid size value
	/*
	if ( GridScale >= 256 ) 		GridScale = 256;
	else if ( GridScale >= 128 ) 	GridScale = 128;
	else if ( GridScale >= 64 ) 	GridScale = 64;
	else if ( GridScale >= 32 ) 	GridScale = 32;
	else if ( GridScale >= 16 ) 	GridScale = 16;
	else if ( GridScale >= 8 ) 		GridScale = 8;
	else                        	GridScale = 0;
	*/

	// Misc boolean options
	AdditiveSelBox  = (pAddSelBoxCheck->GetCheck()  == BF_CHECKED);
	Debug           = (pDebugCheck->GetCheck()      == BF_CHECKED);
	DrawLineDefsLen = (pDrawLengthCheck->GetCheck() == BF_CHECKED);
	Expert          = (pExpertCheck->GetCheck()     == BF_CHECKED);
	Quiet           = (pQuietCheck->GetCheck()      == BF_CHECKED);
	Quieter         = (pQuiterCheck->GetCheck()     == BF_CHECKED);
	Select0         = (pSelect0Check->GetCheck()    == BF_CHECKED);
	AutoScroll      = (pAutoScrollCheck->GetCheck() == BF_CHECKED);

	// Toggle status bar if
	if ( InfoShown != (pInfoBarCheck->GetCheck() == BF_CHECKED) )
		TYPESAFE_DOWNCAST(GetApplication()->GetMainWindow(),
						  TMainFrame)->ToggleStatusBar();
	InfoShown       = (pInfoBarCheck->GetCheck()    == BF_CHECKED);

	// Close Dialog box
	TDialog::CmOk();
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::ToAboveClicked ()
{
	if ( WTextureName[0] != '\0' )
		pAboveText->SetText (WTextureName);
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::ToNormalClicked ()
{
	if ( WTextureName[0] != '\0' )
		pNormalText->SetText (WTextureName);
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::ToBelowClicked ()
{
	if ( WTextureName[0] != '\0' )
		pBelowText->SetText (WTextureName);
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::ToFloorClicked ()
{
	if ( FTextureName[0] != '\0' )
		pFloorText->SetText (FTextureName);
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::ToCeilingClicked ()
{
	if ( FTextureName[0] != '\0' )
		pCeilingText->SetText (FTextureName);
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::ClearAboveClicked ()
{
	pAboveText->SetText ("-");
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::ClearNormalClicked ()
{
	pNormalText->SetText ("-");
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::ClearBelowClicked ()
{
	pBelowText->SetText ("-");
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::ClearFloorClicked ()
{
	pFloorText->SetText ("-");
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::ClearCeilingClicked ()
{
	pCeilingText->SetText ("-");
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::FTextureSelChange ()
{
	char texname[MAX_BITMAPNAME+1];

	// Get selected texture name
	if ( pFTextureList->GetSelString (texname, MAX_BITMAPNAME) <= 1 )
	{
		FTextureName[0] = '\0';
		return;
	}

	// Don't do anything if no real change
	if ( strcmp (texname, FTextureName) == 0 )
		return;

	strcpy (FTextureName, texname);

	// If texture view dialog box opened, display texture
	if ( pFTextureDialog != NULL && pFTextureDialog->IsWindow() )
	{
		FTextureDblClk();
	}
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::FTextureDblClk ()
{
   // Don't select empty texture !
   if ( FTextureName[0] == '\0' || strcmp (FTextureName, "-") == 0 )
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
      TextureInfo *info = FindTextureByName(FTextureName);
      
      if ( pFTextureDialog->SelectBitmap2 (info->filename) < 0 )
	 Notify ("Error: Cannot select the texture name \"%s\" in the "
		 "dialog box of Floor/Ceiling Texture view ! (BUG)",
		 FTextureName);
   }
   else
      Notify ("Error: Cannot create dialog box of Floor/Ceiling "
	      "Texture view !");
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::WTextureSelChange ()
{
	char texname[MAX_BITMAPNAME+1];

	// Get selected texture name
	if ( pWTextureList->GetSelString (texname, MAX_BITMAPNAME) <= 1 )
	{
		WTextureName[0] = '\0';
		return;
	}

	// Don't do anything if no real change
	if ( strcmp (texname, WTextureName) == 0 )
		return;

	strcpy (WTextureName, texname);

	// If texture view dialog box opened, display texture
	if ( pWTextureDialog != NULL && pWTextureDialog->IsWindow() )
	{
		WTextureDblClk();
	}
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::WTextureDblClk ()
{
   // Don't select empty texture !
   if ( WTextureName[0] == '\0' || strcmp (WTextureName, "-") == 0 )
      return;
   
   // Create modeless dialog box
   if ( pWTextureDialog == NULL || pWTextureDialog->IsWindow() == FALSE )
   {
      delete pWTextureDialog;
      pWTextureDialog = new TDisplayWallTextureDialog (Parent);
      pWTextureDialog->Create();
   }
   
   if ( pWTextureDialog->IsWindow() )
   {
      TextureInfo *info = FindTextureByName(WTextureName);
      
      if ( pWTextureDialog->SelectBitmap2 (info->filename) < 0 )
	 Notify ("Error: Cannot select the texture name \"%s\" in the "
		 "dialog box of Wall Texture view ! (BUG)",
		 WTextureName);
   }
   else
      Notify ("Error: Cannot create dialog box of Wall "
	      "Texture view !");
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::EvLButtonDown (UINT modKeys, const TPoint& point)
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
		if ( pStatic == pFloorText   ||
			 pStatic == pCeilingText )
		{
			pFTextureList->SetSelString (texname, -1);
			FTextureSelChange();  	// Be sure to change selection
		}
		else if ( pStatic == pAboveText  ||
				  pStatic == pNormalText ||
				  pStatic == pBelowText  )
		{
		    //pWTextureList->SetSelString (texname, -1);
		    List_SelectStringExact(pWTextureList, -1, texname);
			WTextureSelChange();  	// Be sure to change selection
		}
	}
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
void TPreferencesDialog::EvLButtonDblClk (UINT modKeys, const TPoint& point)
{
	TDialog::EvLButtonDblClk(modKeys, point);

	// Retreive object for handle
	TStatic *pStatic = GetPointedStatic ((TPoint&)point);
	if ( pStatic == NULL )
		return;

	// If floor or ceiling static, get texture name
	char texname[MAX_BITMAPNAME + 1];
	pStatic->GetText (texname, MAX_BITMAPNAME + 1);
	if ( texname[0] != '\0' && strcmp (texname, "-") != 0 )
	{
		if ( pStatic == pFloorText   ||
			 pStatic == pCeilingText )
		{
			pFTextureList->SetSelString (texname, -1);
			FTextureSelChange();  	// Be sure to change selection
			FTextureDblClk();		// Force to open Dialog Box
		}
		else if ( pStatic == pAboveText  ||
				  pStatic == pNormalText ||
				  pStatic == pBelowText  )
		{
		    //pWTextureList->SetSelString (texname, -1);
		    List_SelectStringExact(pWTextureList, -1, texname);
			WTextureSelChange();  	// Be sure to change selection
			WTextureDblClk();		// Force to open Dialog Box
		}
	}
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//
TStatic *TPreferencesDialog::GetPointedStatic (TPoint& point)
{
	// Retreive object for handle
	TStatic *pStatic = NULL;

	if ( IsPointInDlgItem (IDC_CEILING_TEXT, point)  ||
		 IsPointInDlgItem (IDC_CEILING_FRAME, point) )
	{
		pStatic = pCeilingText;
	}

	else if ( IsPointInDlgItem (IDC_FLOOR_TEXT, point)  ||
			  IsPointInDlgItem (IDC_FLOOR_FRAME, point) )
	{
		pStatic = pFloorText;
	}

	else if ( IsPointInDlgItem (IDC_ABOVE_TEXT, point)  ||
			  IsPointInDlgItem (IDC_ABOVE_FRAME, point) )
	{
		pStatic = pAboveText;
	}

	else if ( IsPointInDlgItem (IDC_NORMAL_TEXT, point)  ||
			  IsPointInDlgItem (IDC_NORMAL_FRAME, point) )
	{
		pStatic = pNormalText;
	}

	else if ( IsPointInDlgItem (IDC_BELOW_TEXT, point)  ||
			  IsPointInDlgItem (IDC_BELOW_FRAME, point) )
	{
		pStatic = pBelowText;
	}

	return pStatic;
}


/////////////////////////////////////////////////////////////////////
// TPreferencesDialog
// ------------------
//   Look if the dialog control 'resid' window contains the point
//   'clientPoint', which is a dialog client coord. of the point
BOOL TPreferencesDialog::IsPointInDlgItem (int itemId, TPoint &clientPoint)
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

