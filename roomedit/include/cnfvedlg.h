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

	FILE:         cnfvedlg.h

	OVERVIEW
	========
	Class definition for TConfirmVertexDialog (TDialog).
*/
#if !defined(__cnfvedlg_h)              // Sentry, use file only if it's not already included.
#define __cnfvedlg_h


#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_CHECKBOX_H
	class _OWLCLASS TCheckBox;
#endif

#include "cnfvedlg.rh"            // Definition of all resources.


//{{TDialog = TConfirmVertexDialog}}
struct TConfirmVertexDialogXfer
{
//{{TConfirmVertexDialogXFER_DATA}}
	BOOL    pXPosCheck;
	BOOL    pYPosCheck;
//{{TConfirmVertexDialogXFER_DATA_END}}
};


class TConfirmVertexDialog : public TDialog
{
public:
	TConfirmVertexDialog (TWindow* parent, TConfirmVertexDialogXfer &Xfer, TResId resId = IDD_CONFIRM_VERTEX, TModule* module = 0);
	virtual ~TConfirmVertexDialog ();

//{{TConfirmVertexDialogXFER_DEF}}
protected:
	TCheckBox *pXPosCheck;
	TCheckBox *pYPosCheck;
//{{TConfirmVertexDialogXFER_DEF_END}}

//{{TConfirmVertexDialogVIRTUAL_BEGIN}}
public:
    virtual void SetupWindow ();
//{{TConfirmVertexDialogVIRTUAL_END}}
};    //{{TConfirmVertexDialog}}


#endif                                      // __cnfvedlg_h sentry.

