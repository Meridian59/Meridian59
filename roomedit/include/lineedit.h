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

	FILE:         lineedit.h

	OVERVIEW
	========
	Class definition for TLineDefEditDialog (TDialog).
*/
#if !defined(__lineedit_h)              // Sentry, use file only if it's not already included.
#define __lineedit_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_LISTBOX_H
	class _OWLCLASS TListBox;
#endif

#ifndef OWL_STATIC_H
	class _OWLCLASS TStatic;
#endif

#ifndef OWL_EDIT_H
	class _OWLCLASS TEdit;
#endif

#ifndef OWL_RADIOBUT_H
	class _OWLCLASS TRadioButton;
#endif

#ifndef OWL_BUTTON_H
	class _OWLCLASS TButton;
#endif

#ifndef OWL_CHECKBOX_H
	class _OWLCLASS TCheckBox;
#endif

#ifndef __objects_h
	#include "objects.h"	// SelPtr
#endif

#ifndef __cnflddlg_h
	#include "cnflddlg.h"
#endif

#ifndef __levels_h
	#include "levels.h"
#endif

#ifndef __cnflddlg_h
	#include "cnflddlg.h"
#endif

#ifndef __viewbmp_h
	class TDisplayWallTextureDialog;
#endif

#include "lineedit.rh"


//{{TDialog = TLineDefEditDialog}}

class TLineDefEditDialog : public TDialog
{
protected:
	SelPtr SelLineDefs;
	int LineDefSet;
	LineDef CurLineDef;
	SideDef CurSD[2];
	char TextureName[MAX_BITMAPNAME + 1];
	int  TextureNum;
	TDisplayWallTextureDialog *pWTextureDialog;
	TConfirmLineDefDialogXfer ConfirmData;

protected:
	void InitLineDefSet ();
	void SetLineDefList ();
	void SetTextureList ();
	void SetLineDef ();
	void GetLineDef ();
	void SetSideDef (int sdnum);
	void GetSideDef (int sdnum);
	void SDKillFocus (int sdnum);
	void SDNew (int sdnum);
	void CopySDData (int sdnum, SHORT DestLD, TConfirmLineDefDialogXfer &xfer);
	TStatic *GetPointedStatic (TPoint& point);
	BOOL IsPointInDlgItem (int itemId, TPoint &clientPoint);

public:
	TLineDefEditDialog (TWindow* parent, SelPtr Selected, TResId resId = IDD_LINEDEF_EDIT, TModule* module = 0);
	virtual ~TLineDefEditDialog ();

//{{TLineDefEditDialogVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
//{{TLineDefEditDialogVIRTUAL_END}}

//{{TLineDefEditDialogRSP_TBL_BEGIN}}
protected:
	void CmOk ();
	void CeilingsClicked ();
	void DoorsClicked ();
	void LowerFloorsClicked ();
	void MovingThingsClicked ();
	void RaiseFloorsClicked ();
	void SpecialClicked ();

	void SD1Changed ();
	void SD1ClearClicked ();
	void SD1ClearAboveClicked ();
	void SD1ClearBelowClicked ();
	void SD1ClearNormalClicked ();
	void SD1ToAboveClicked ();
	void SD1ToBelowClicked ();
	void SD1ToNormalClicked ();
	void SD1Killfocus ();
	void SD1NewClicked ();

	void SD2Change ();
	void SD2ClearAboveClicked ();
	void SD2ClearBelowClicked ();
	void SD2ClearNormalClicked ();
	void SD2ToAboveClicked ();
	void SD2ToBelowClicked ();
	void SD2ToNormalClicked ();
	void SD2Killfocus ();
	void SD2NewClicked ();

	void LineDefSelChange ();
	void TextureListDBLClick ();
	void TextureSelchange ();
	void SectorTagNewClicked ();
	void EvSetFocus (HWND hWndLostFocus );
	void EvActivate (UINT active, BOOL minimized, HWND hWndOther );

#if OWLVersion > OWLVERBC502
	void EvLButtonDown (UINT modKeys, const TPoint& point);
	void EvLButtonDblClk (UINT modKeys, const TPoint& point);
#else
	void EvLButtonDown(UINT modKeys, TPoint& point);
	void EvLButtonDblClk(UINT modKeys, TPoint& point);
#endif

	void PassPosClicked();
	void PassNegClicked();
	void TransPosClicked();
	void TransNegClicked();
	void FlipPosClicked();
	void FlipNegClicked();
	void NoMapClicked();
	void MapStartClicked();
	void NoLookThroughPosClicked();
	void NoLookThroughNegClicked();
	void AbovePosClicked();
	void AboveNegClicked();
	void NormalPosClicked();
	void NormalNegClicked();
	void BelowPosClicked();
	void BelowNegClicked();
	void NoVTilePosClicked();
	void NoVTileNegClicked();
	void ScrollPosClicked();
	void ScrollNegClicked();

//{{TLineDefEditDialogRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TLineDefEditDialog);

protected:
	TCheckBox *pPassPosCheck;
	TCheckBox *pPassNegCheck;
	TCheckBox *pTransPosCheck;
	TCheckBox *pTransNegCheck;
	TCheckBox *pFlipPosCheck;
	TCheckBox *pFlipNegCheck;
	TCheckBox *pNoMapCheck;
	TCheckBox *pMapStartCheck;
	TCheckBox *pPosNoLookThroughCheck;
	TCheckBox *pNegNoLookThroughCheck;
	TCheckBox *pPosAboveCheck;
	TCheckBox *pNegAboveCheck;
	TCheckBox *pPosNormalCheck;
	TCheckBox *pNegNormalCheck;
	TCheckBox *pPosBelowCheck;
	TCheckBox *pNegBelowCheck;
	TCheckBox *pPosNoVTile;
	TCheckBox *pNegNoVTile;

	TEdit *pSectorTagEdit;
	TEdit *pVertex1Edit;
	TEdit *pVertex2Edit;
	TEdit *pSDEdit[2];
	TEdit *pIDNumEdit[2];
	TEdit *pSpeedEdit[2];

	TEdit *pSDXPosEdit[2];
	TEdit *pSDYPosEdit[2];
	TEdit *pSDSectorEdit[2];
	TStatic *pSDAboveStatic[2];
	TStatic *pSDBelowStatic[2];
	TStatic *pSDNormalStatic[2];

	TListBox *pTextureList;
	TListBox *pLineDefList;

	TButton *pSDToAbove[2];
	TButton *pSDToBelow[2];
	TButton *pSDToNormal[2];
	TButton *pSDClearAbove[2];
	TButton *pSDClearBelow[2];
	TButton *pSDClearNormal[2];

        TRadioButton *pScrollNRadio[2];
        TRadioButton *pScrollNERadio[2];
        TRadioButton *pScrollERadio[2];
        TRadioButton *pScrollSERadio[2];
        TRadioButton *pScrollSRadio[2];
        TRadioButton *pScrollSWRadio[2];
        TRadioButton *pScrollWRadio[2];
        TRadioButton *pScrollNWRadio[2];
	TRadioButton *pScrollNoneRadio[2];
	TRadioButton *pScrollSlowRadio[2];
	TRadioButton *pScrollMediumRadio[2];
	TRadioButton *pScrollFastRadio[2];
};    //{{TLineDefEditDialog}}


#endif                                      // __lineedit_h sentry.

