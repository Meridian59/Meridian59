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

	FILE:         episdlg.h

	OVERVIEW
	========
	Class definition for TEpisodeMissionDialog (TDialog).
*/

#if !defined(__episdlg_h)              // Sentry, use file only if it's not already included.
#define __episdlg_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_RADIOBUT_H
	class _OWLCLASS TRadioButton;
#endif

#ifndef OWL_CHECKBOX_H
	class _OWLCLASS TCheckBox;
#endif

#ifndef OWL_STATIC_H
	class _OWLCLASS TStatic;
#endif

#ifndef OWL_GROUPBOX_H
	#include <owl\groupbox.h>
#endif

#include "episdlg.rh"


//{{TDialog = TEpisodeMissionDialog}}

class TEpisodeMissionDialog : public TDialog
{
private:
	char *pLevelName;
	BOOL MustExist;

protected:
	TCheckBox *pDoom2Check;
	TGroupBox *pDoom1Group;
	TGroupBox *pDoom2Group;
	TStatic      *pEpisodeStatic[3];
	TStatic      *pSeparatorStatic[2];
	TRadioButton *pDoom1Radio[3*9];
	TRadioButton *pDoom2Radio[32];

public:
	TEpisodeMissionDialog (TWindow* parent, char *levelName, BOOL mustExist = TRUE, int resId = IDD_EPISODE_MISSION, TModule* module = 0);
	virtual ~TEpisodeMissionDialog ();

//{{TEpisodeMissionDialogVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
//{{TEpisodeMissionDialogVIRTUAL_END}}

//{{TEpisodeMissionDialogRSP_TBL_BEGIN}}
protected:
	void CmOk ();
	void Doom2CheckClicked ();
//{{TEpisodeMissionDialogRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TEpisodeMissionDialog);
};    //{{TEpisodeMissionDialog}}


#endif                                      // __episdlg_h sentry.

