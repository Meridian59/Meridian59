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

	FILE:         cnfthdlg.h

	OVERVIEW
	========
	Class definition for TConfirmThingDialog (TDialog).
*/
#if !defined(__cnfthdlg_h)              // Sentry, use file only if it's not already included.
#define __cnfthdlg_h

#ifndef __common_h
	#include "common.h"
#endif

#include <owl\checkbox.h>

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_CHECKBOX_H
	class _OWLCLASS TCheckBox;
#endif

#include "cnfthdlg.rh"


//{{TDialog = TConfirmThingDialog}}
struct TConfirmThingDialogXfer {
//{{TConfirmThingDialogXFER_DATA}}
   uint16 pAngleCheck;
   uint16 pTypeCheck;
   uint16 pXPosCheck;
   uint16 pYPosCheck;
   uint16 pExitCheck;
   uint16 pSubTypeCheck;
//{{TConfirmThingDialogXFER_DATA_END}}
};


class TConfirmThingDialog : public TDialog
{
public:
	TConfirmThingDialog (TWindow* parent, TConfirmThingDialogXfer &XFer, TResId resId = IDD_CONFIRM_THING, TModule* module = 0);
	virtual ~TConfirmThingDialog ();

//{{TConfirmThingDialogXFER_DEF}}
protected:
	TCheckBox *pAngleCheck;
	TCheckBox *pTypeCheck;
	TCheckBox *pXPosCheck;
    TCheckBox *pDeafCheck;
    TCheckBox *pLevel12Check;
    TCheckBox *pLevel3Check;
    TCheckBox *pLevel45Check;
    TCheckBox *pMultiCheck;
	TCheckBox *pYPosCheck;
//{{TConfirmThingDialogXFER_DEF_END}}

//{{TConfirmThingDialogVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
//{{TConfirmThingDialogVIRTUAL_END}}

//{{TConfirmThingDialogRSP_TBL_BEGIN}}
protected:
    void CmOk ();
//{{TConfirmThingDialogRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TConfirmThingDialog);
};    //{{TConfirmThingDialog}}


#endif                                      // __cnfthdlg_h sentry.

