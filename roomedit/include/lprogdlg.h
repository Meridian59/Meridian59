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

	FILE:         lprogdlg.h

	OVERVIEW
	========
	Class definition for TLevelProgressDialog (TDialog).
*/
#if !defined(__lprogdlg_h)              // Sentry, use file only if it's not already included.
#define __lprogdlg_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_STATIC_H
	#include <owl\static.h>
#endif

#ifndef OWL_GAUGE_H
	#include <owl\gauge.h>
#endif

#include "lprogdlg.rh"            // Definition of all resources.


//{{TDialog = TLevelProgressDialog}}
class TLevelProgressDialog : public TDialog {
private:
	// Dialog is minimized ?
	BOOL Minimized;

	// Interface objects for NODES building
	TStatic *pNodesText;
	TGauge *pNodesGauge;
	TStatic *pVertexesStatic;
	TStatic *pSideDefsStatic;
	TStatic *pSegsStatic;
	TStatic *pSSectorsStatic;

	// Interface objects for REJECT building
	TStatic *pRejectFrame;
	TStatic *pRejectText;
	TGauge *pRejectGauge;

	// Interface objects for BLOCKMAP building
	TStatic *pBlockmapFrame;
	TStatic *pBlockmapText;
	TGauge *pBlockmapGauge;

public:
	TLevelProgressDialog (TWindow* parent, TResId resId = IDD_LEVEL_PROGRESS, TModule* module = 0);
	virtual ~TLevelProgressDialog ();

	void ShowRejectControls ();
	void ShowRejectProgress (int value);

	void ShowBlockmapControls ();
	void ShowBlockmapProgress (int value);

	void ShowNodesProgress (int objtype);


//{{TLevelProgressDialogVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
//{{TLevelProgressDialogVIRTUAL_END}}

//{{TLevelProgressDialogRSP_TBL_BEGIN}}
protected:
	void EvPaint();
	void EvSize (UINT sizeType, const TSize& size);

//{{TLevelProgressDialogRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TLevelProgressDialog);
};    //{{TLevelProgressDialog}}


#endif                                      // __lprogdlg_h sentry.

