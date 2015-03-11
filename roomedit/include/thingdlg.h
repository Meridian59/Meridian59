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

	FILE:         thingdlg.h

	OVERVIEW
	========
	Class definition for TThingEditDialog (TDialog).
*/
#if !defined(__thingdlg_h)              // Sentry, use file only if it's not already included.
#define __thingdlg_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef __viewbmp_h
	#include "viewbmp.h"
#endif

#ifndef OWL_LISTBOX_H
	class _OWLCLASS TListBox;
#endif

#ifndef OWL_EDIT_H
	class _OWLCLASS TEdit;
#endif

#ifndef OWL_CHECKBOX_H
	class _OWLCLASS TCheckBox;
#endif

#ifndef OWL_RADIOBUT_H
	class _OWLCLASS TRadioButton;
#endif

#ifndef OWL_COMBOBOX_H
	#include <owl\combobox.h>
#endif

#ifndef __objects_h
	#include "objects.h"	// SelPtr, Thing
#endif

#ifndef __levels_h
	#include "levels.h"
#endif

#ifndef __things_h
	#include "things.h"
#endif

#ifndef __cnfthdlg_h
	#include "cnfthdlg.h"
#endif


#include "thingdlg.rh"            // Definition of all resources.


//{{TDialog = TThingEditDialog}}

class TThingEditDialog : public TDialog
{
protected:
	SelPtr SelThings;
	Thing  CurThing;
	int    ThingSet;
	TConfirmThingDialogXfer ConfirmData;

protected:
	void SetupListBox ();
	void InitThingSet ();
	void GetAngle();
	void SetAngle();

public:
	TThingEditDialog (TWindow* parent, SelPtr SelectedThinges, TResId resId = IDD_THING_EDIT, TModule* module = 0);
	virtual ~TThingEditDialog ();

//{{TThingEditDialogVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
//{{TThingEditDialogVIRTUAL_END}}

protected:
	TButton		 *pEntrancesButton;
	TEdit        *pXPosEdit;
	TEdit        *pYPosEdit;
	TEdit        *pXExit;
	TEdit        *pYExit;
	TEdit			 *pComment;

	TRadioButton *pPlayerRadio;
	TRadioButton *pExitRadio;
	TRadioButton *pObjectsRadio;
	TRadioButton *pMonsterGenRadio;
	TRadioButton *pEntranceRadio;

	TRadioButton *pAngleERadio;
	TRadioButton *pAngleNRadio;
	TRadioButton *pAngleNERadio;
	TRadioButton *pAngleNWRadio;
	TRadioButton *pAngleSRadio;
	TRadioButton *pAngleSERadio;
	TRadioButton *pAngleSWRadio;
	TRadioButton *pAngleWRadio;

	TListBox		 *pThingList;

	TComboBox 	 *pExitRoomID;
	TComboBox 	 *pSubType;

	TCheckBox 	 *pDontGenerate;

//{{TThingEditDialogRSP_TBL_BEGIN}}
protected:
	void CmOk ();
	void PlayerClicked ();
	void ExitClicked ();
	void ObjectsClicked ();
	void MonsterGenClicked();
	void ThingSelchange ();
	void RoomIDSelchange ();
	void SubTypeSelchange ();
	void AngleClicked ();
	void EntranceClicked();
	void SelectFromEntrancesClicked ();
	void XPosChange ();
	void YPosChange ();
//{{TThingEditDialogRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TThingEditDialog);
};    //{{TThingEditDialogDialog}}


#endif                                      // __thingdlg_h sentry.

