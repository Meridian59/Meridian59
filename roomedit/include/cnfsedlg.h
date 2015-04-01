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

	FILE:         cnfsedlg.h

	OVERVIEW
	========
	Class definition for TConfirmSectorDialog (TDialog).
*/
#if !defined(__cnfsedlg_h)              // Sentry, use file only if it's not already included.
#define __cnfsedlg_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_CHECKBOX_H
	class _OWLCLASS TCheckBox;
#endif


#include "cnfsedlg.rh"            // Definition of all resources.
#include "seditdlg.rh"


//{{TDialog = TConfirmSectorDialog}}
struct TConfirmSectorDialogXfer
{
//{{TConfirmSectorDialogXFER_DATA}}
	BOOL    pSpecialCheck;
	BOOL    pTagCheck;
	BOOL    pLightCheck;
	BOOL    pTextureOffsetCheck;
	BOOL    pFloorTextureCheck;
	BOOL    pFloorHeightCheck;
	BOOL    pCeilingTextureCheck;
	BOOL    pCeilingHeightCheck;
	BOOL    pIDNumCheck;
	BOOL    pSpeedCheck;
	BOOL    pDepthCheck;
        BOOL    pScrollCheck;
        BOOL    pSlopeCheck;
//{{TConfirmSectorDialogXFER_DATA_END}}
};


class TConfirmSectorDialog : public TDialog
{
public:
	TConfirmSectorDialog (TWindow* parent, TConfirmSectorDialogXfer &Xfer, TResId resId = IDD_CONFIRM_SECTOR, TModule* module = 0);
	virtual ~TConfirmSectorDialog ();

//{{TConfirmSectorDialogXFER_DEF}}
protected:
	TCheckBox *pSpecialCheck;
	TCheckBox *pTagCheck;
	TCheckBox *pLightCheck;
	TCheckBox *pTextureOffsetCheck;
	TCheckBox *pFloorTextureCheck;
	TCheckBox *pFloorHeightCheck;
	TCheckBox *pCeilingTextureCheck;
	TCheckBox *pCeilingHeightCheck;
	TCheckBox *pIDNumCheck;
	TCheckBox *pSpeedCheck;
	TCheckBox *pDepthCheck;
	TCheckBox *pScrollCheck;
	TCheckBox *pSlopeCheck;
//{{TConfirmSectorDialogXFER_DEF_END}}

//{{TConfirmSectorDialogVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
//{{TConfirmSectorDialogVIRTUAL_END}}

//{{TConfirmSectorDialogRSP_TBL_BEGIN}}
protected:
    void CmOk ();
//{{TConfirmSectorDialogRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TConfirmSectorDialog);
};    //{{TConfirmSectorDialog}}


#endif                                      // __cnfsedlg_h sentry.

