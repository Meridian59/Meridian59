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

	FILE:         mastdlg.h

	OVERVIEW
	========
	Class definition for TMasterDialog (TDialog).
*/
#if !defined(__mastdlg_h)              // Sentry, use file only if it's not already included.
#define __mastdlg_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_LISTBOX_H
	_OWLCLASS class TListBox;
#endif

#ifndef __mastdlg_rh
	#include "mastdlg.rh"
#endif


//{{TDialog = TMasterDialog}}

class TMasterDialog : public TDialog
{
public:
	TMasterDialog (TWindow* parent, TResId resId = IDD_MASTER_DIR, TModule* module = 0);
	virtual ~TMasterDialog ();

protected:
	TListBox *pMasterList;

//{{TMasterDialogVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
//{{TMasterDialogVIRTUAL_END}}

//{{TMasterDialogRSP_TBL_BEGIN}}
protected:

#if OWLVersion > OWLVERBC502
	void EvDrawItem (UINT ctrlId, const DRAWITEMSTRUCT& drawInfo);
#else
	void EvDrawItem (UINT ctrlId, DRAWITEMSTRUCT& drawInfo);
#endif

//{{TMasterDialogRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TMasterDialog);
};    //{{TMasterDialog}}


#endif                                      // __mastdlg_h sentry.

