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

	FILE:         editscro.h

	OVERVIEW
	========
	Class definition for TEditScroller (TScroller).
*/
#if !defined(__editscro_h)              // Sentry, use file only if it's not already included.
#define __editscro_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_SCROLLER_H
	#include "owl\scroller.h"
#endif

class TEditScroller : public TScroller
{
public:
	TEditScroller(TWindow* window, int xUnit, int yUnit, long xRange, long yRange):
		TScroller(window, xUnit, yUnit, xRange, yRange)
	{
		// We don't want the scroller to set the window origin before calling
		// Paint. The TEditorWindow::Paint member sets himself the window origin
		// before redrawing the level map.
		AutoOrg = FALSE ;
	}
	virtual ~TEditScroller()
	{
	}

	virtual void HScroll(UINT scrollEvent, int thumbPos)
	{
#if 0
		if ( scrollEvent != SB_THUMBTRACK )
#endif
			TScroller::HScroll (scrollEvent, thumbPos);
	}

	virtual void VScroll(UINT scrollEvent, int thumbPos)
	{
#if 0
		if ( scrollEvent != SB_THUMBTRACK )
#endif
			TScroller::VScroll (scrollEvent, thumbPos);
	}

	virtual void ScrollTo(long x, long y);
	virtual void AutoScroll();
};

#endif                                      // __editscro_h sentry.

