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

	FILE:         cnflddlg.h

	OVERVIEW
	========
	Class definition for TConfirmLineDefDialog (TDialog).
*/
#if !defined(__cnflddlg_h)              // Sentry, use file only if it's not already included.
#define __cnflddlg_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_CHECKBOX_H
	class _OWLCLASS TCheckBox;
#endif

#include "cnflddlg.rh"


//{{TDialog = TConfirmLineDefDialog}}
struct TConfirmLineDefDialogXfer {
	uint16    pTypeCheck;
	uint16    pFlagsCheck;
	uint16    pSectorCheck;
	uint16    pVertex1Check;
	uint16    pVertex2Check;
	uint16    pSDCheck[2];

	uint16    pPassPosCheck;
	uint16    pPassNegCheck;
	uint16    pTransPosCheck;
	uint16    pTransNegCheck;
	uint16    pFlipPosCheck;
	uint16    pFlipNegCheck;
	uint16    pNoMapCheck;
	uint16    pMapStartCheck;
	uint16    pNoLookThroughPosCheck;
	uint16    pNoLookThroughNegCheck;
	uint16    pAbovePosCheck;
	uint16    pAboveNegCheck;
	uint16    pNormalPosCheck;
	uint16    pNormalNegCheck;
	uint16    pBelowPosCheck;
	uint16    pBelowNegCheck;
	uint16    pNoVTilePosCheck;
	uint16    pNoVTileNegCheck;
	uint16    pNoHTilePosCheck;
	uint16    pNoHTileNegCheck;

	uint16    pSDAboveCheck[2];
	uint16    pSDBelowCheck[2];
	uint16    pSDNormalCheck[2];
	uint16    pSDSectorCheck[2];
	uint16    pSDXOfsCheck[2];
	uint16    pSDYOfsCheck[2];
	uint16    pIDNumCheck[2];
	uint16    pSpeedCheck[2];

   uint16    pScrollCheck[2];
};


class TConfirmLineDefDialog : public TDialog
{
public:
	TConfirmLineDefDialog (TWindow* parent, TConfirmLineDefDialogXfer *pXFer, TResId resId = IDD_CONFIRM_LINEDEF, TModule* module = 0);
	virtual ~TConfirmLineDefDialog ();

protected:
	void EnableFlags ();
	void EnableSD (int sdnum);

protected:
	TCheckBox *pTypeCheck;
	TCheckBox *pFlagsCheck;
	TCheckBox *pSectorCheck;
	TCheckBox *pVertex1Check;
	TCheckBox *pVertex2Check;
	TCheckBox *pSDCheck[2];

	TCheckBox *pPassPosCheck;
	TCheckBox *pPassNegCheck;
	TCheckBox *pTransPosCheck;
	TCheckBox *pTransNegCheck;
	TCheckBox *pFlipPosCheck;
	TCheckBox *pFlipNegCheck;
	TCheckBox *pNoMapCheck;
	TCheckBox *pMapStartCheck;
	TCheckBox *pNoLookThroughPosCheck;
	TCheckBox *pNoLookThroughNegCheck;
	TCheckBox *pAbovePosCheck;
	TCheckBox *pAboveNegCheck;
	TCheckBox *pNormalPosCheck;
	TCheckBox *pNormalNegCheck;
	TCheckBox *pBelowPosCheck;
	TCheckBox *pBelowNegCheck;
	TCheckBox *pNoVTilePosCheck;
	TCheckBox *pNoVTileNegCheck;
	TCheckBox *pNoHTilePosCheck;
	TCheckBox *pNoHTileNegCheck;

	TCheckBox *pSDAboveCheck[2];
	TCheckBox *pSDBelowCheck[2];
	TCheckBox *pSDNormalCheck[2];
	TCheckBox *pSDSectorCheck[2];
	TCheckBox *pSDXOfsCheck[2];
	TCheckBox *pSDYOfsCheck[2];
	TCheckBox *pIDNumCheck[2];
	TCheckBox *pSpeedCheck[2];

	TCheckBox *pScrollCheck[2];

//{{TConfirmLineDefDialogVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
//{{TConfirmLineDefDialogVIRTUAL_END}}

//{{TConfirmLineDefDialogRSP_TBL_BEGIN}}
protected:
	void CmOk ();
	void FlagsClicked ();
	void SD1CheckClicked ();
	void SD2CheckClicked ();
//{{TConfirmLineDefDialogRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TConfirmLineDefDialog);
};    //{{TConfirmLineDefDialog}}

#endif                                      // __cnflddlg_h sentry.

