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

	FILE:         entrydlg.h

	OVERVIEW
	========
	Class definition for TViewEntryDialog (TDialog).
*/

#if !defined(__entrydlg_h)              // Sentry, use file only if it's not already included.
#define __entrydlg_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef __wads_h
	#include "wads.h"		// MDirPtr
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_LISTBOX_H
	#include <owl\listbox.h>
#endif

#ifndef OWL_COMBOBOX_H
	#include <owl\combobox.h>
#endif

#include "entrydlg.rh"            // Definition of all resources.


//{{TDialog = TViewEntryDialog}}
class TViewEntryDialog : public TDialog
{
protected:
	TComboBox *pEntryCombo;
	TListBox  *pHexDumpList;
	MDirPtr CurrentEntry;

public:
	TViewEntryDialog (TWindow* parent, TResId resId = IDD_VIEW_ENTRY, TModule* module = 0);
	virtual ~TViewEntryDialog ();

//{{TViewEntryDialogVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
//{{TViewEntryDialogVIRTUAL_END}}

//{{TViewEntryDialogRSP_TBL_BEGIN}}
protected:
	void EntrySelChange ();

#if OWLVersion > OWLVERBC502
	void EvDrawItem (UINT ctrlId, const DRAWITEMSTRUCT& drawInfo);
#else
	void EvDrawItem (UINT ctrlId, DRAWITEMSTRUCT& drawInfo);
#endif

//{{TViewEntryDialogRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TViewEntryDialog);
};    //{{TViewEntryDialog}}


#endif                                      // __entrydlg_h sentry.

