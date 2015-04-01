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

	FILE:         mainfram.h

	OVERVIEW
	========
	Class definition for TMainFrame (TDecoratedFrame).
*/
#if !defined(__mainfram_h)              // Sentry, use file only if it's not already included.
#define __mainfram_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_OPENSAVE_H
	#include <owl\opensave.h>
#endif

#ifndef OWL_STATUSBA_H
	#include <owl\statusba.h>
#endif

#ifndef __tooltip_h
	#include "tooltip.h"
#endif


//{{TDecoratedFrame = TMainFrame}}
class TMainFrame : public TDecoratedFrame
{
private:
	TTipStatusBar  *statusBar;
	TTipControlBar *controlBar;
	TToolTip        tooltip;
	UINT            timer;

protected:
	void SetupMainControlBar();
	void SetupEditorControlBar();

public:
	BOOL inEditor;		// Currently editing a level ?
	BOOL activated;		// Window active ?
	BOOL controlBarOn;	// Control bar shown ?
	BOOL statusBarOn;	// Status bar shown ?

	TMainFrame (const char* title, TModule* module = 0);
	virtual ~TMainFrame ();

	void EditLevel (char *levelName, BOOL newLevel);
	BOOL StopEditLevel ();

	TStatusBar *GetStatusBar() { return statusBar; } ;
	void DrawFreeMemory();

	void SetMenuAndAccel (TResId id);
	void ToggleStatusBar ();
	void ToggleControlBar ();

//{{TMainFrameVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
	virtual bool PreProcessMsg (MSG& msg);
//{{TMainFrameVIRTUAL_END}}

//{{TMainFrameRSP_TBL_BEGIN}}
protected:
	void EvTimer (UINT timerId);
	void EvActivate (UINT active, bool minimized, HWND hWndOther);
//{{TMainFrameRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TMainFrame);
};    //{{TMainFrame}}

#endif                                      // __mainfram_h sentry.

