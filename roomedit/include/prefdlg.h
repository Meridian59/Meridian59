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

	FILE:         prefdlg.h

	OVERVIEW
	========
	Class definition for TPreferencesDialog (TDialog).
*/
#if !defined(__prefdlg_h)              // Sentry, use file only if it's not already included.
#define __prefdlg_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_STATIC_H
	class _OWLCLASS TStatic;
#endif

#ifndef OWL_EDIT_H
	class _OWLCLASS TEdit;
#endif

#ifndef OWL_LISTBOX_H
	class _OWLCLASS TListBox;
#endif

#ifndef __viewbmp_h
	class TDisplayWallTextureDialog;
	class TDisplayFloorTextureDialog;
#endif

#ifndef __prefdlg_rh
	#include "prefdlg.rh"            // Definition of resource
#endif

#include "dibutil.h"


//{{TDialog = TPreferencesDialog}}


class TPreferencesDialog : public TDialog
{
protected:
	TListBox *pWTextureList;
	TListBox *pFTextureList;
	TEdit    *pFloorHeight;
	TStatic  *pNormalText;
	TStatic  *pFloorText;
	TStatic  *pCeilingText;
	TEdit    *pCeilingHeight;
	TStatic  *pBelowText;
	TStatic  *pAboveText;
	TCheckBox *pAddSelBoxCheck;
	TCheckBox *pDebugCheck;
	TCheckBox *pDrawLengthCheck;
	TCheckBox *pExpertCheck;
	TCheckBox *pInfoBarCheck;
	TCheckBox *pQuietCheck;
	TCheckBox *pQuiterCheck;
	TCheckBox *pSelect0Check;
	TCheckBox *pAutoScrollCheck;

	TEdit *pMaxUndoEdit;

	char WTextureName[MAX_BITMAPNAME + 1];
	char FTextureName[MAX_BITMAPNAME + 1];

	TDisplayWallTextureDialog  *pWTextureDialog;
	TDisplayFloorTextureDialog *pFTextureDialog;

protected:
	BOOL IsPointInDlgItem (int itemId, TPoint &clientPoint);
	TStatic *GetPointedStatic (TPoint& point);

public:
	TPreferencesDialog (TWindow* parent, TResId resId = IDD_PREFERENCES, TModule* module = 0);
	virtual ~TPreferencesDialog ();

//{{TPreferencesDialogVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
//{{TPreferencesDialogVIRTUAL_END}}

//{{TPreferencesDialogRSP_TBL_BEGIN}}
protected:
	void CmOk ();
	void ToNormalClicked ();
	void ToFloorClicked ();
	void ToCeilingClicked ();
	void ToBelowClicked ();
	void ToAboveClicked ();
	void ClearNormalClicked ();
	void FTextureSelChange ();
	void FTextureDblClk ();
	void ClearFloorClicked ();
	void ClearCeilingClicked ();
	void ClearBelowClicked ();
	void ClearAboveClicked ();
	void WTextureSelChange ();
	void WTextureDblClk ();

	void EvLButtonDown (UINT modKeys, const TPoint& point);
	void EvLButtonDblClk (UINT modKeys, const TPoint& point);

//{{TPreferencesDialogRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TPreferencesDialog);
};    //{{TPreferencesDialog}}


#endif                                      // __prefdlg_h sentry.

