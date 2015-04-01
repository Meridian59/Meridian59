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

	FILE:         vertdlg.h

	OVERVIEW
	========
	Class definition for TVertexDialog (TDialog).
*/
#if !defined(__workdlg_h)              // Sentry, use file only if it's not already included.
#define __workdlg_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_STATIC_H
	class _OWLCLASS TStatic;
#endif

#ifndef OWL_GAUGE_H
	#include <owl\gauge.h>
#endif

#include "workdlg.rh"            // Definition of all resources.

//{{TDialog = TWorkDialog}}

class TWorkDialog : public TDialog
{
public:
	BOOL Cancel;
	int vmin, vmax;

protected:
	TStatic *pGaugeStatic;
	TStatic *pWorkStatic;
	TGauge  *pGauge;

public:
	TWorkDialog (TWindow* parent = ((TApplication *)::Module)->GetMainWindow(), TResId resId = IDD_WORK, TModule* module = 0);
	virtual ~TWorkDialog ();
	void SetWorkText(const char *txt)
	{
		pWorkStatic->SetText(txt);
	}
	void SetRange(int min, int max)
	{
		vmin = min;
		vmax = max;
	}
	void SetValue(int value);

//{{TWorkDialogVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
	virtual void Destroy (int retVal = 0);
//{{TWorkDialogVIRTUAL_END}}

//{{TWorkDialogRSP_TBL_BEGIN}}
protected:
	void CmCancel ();
//{{TWorkDialogRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TWorkDialog);
};    //{{TWorkDialog}}


#endif                                      // __workdlg_h sentry.

