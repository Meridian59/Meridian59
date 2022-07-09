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

	FILE:         editscro.cpp

	OVERVIEW
	========
	Source file for implementation of TEditScroller (TScroller).
*/

#include "common.h"
#pragma hdrstop

#ifndef __editscro_h
	#include "editscro.h"
#endif

#ifndef __editcli_h
	#include "editcli.h"
#endif

#ifndef __mapdc_h
	#include "mapdc.h"	// InfoShown
#endif

#ifndef OWL_SCROLLER_H
	#include <owl\scroller.h>
#endif


/////////////////////////////////////////////////////////////////////
// TEditScroller::AutoScroll
// -------------------------
//
// This function is a copy of the original OWL function, but the
// auto-scrolling is done when the mouse is wihtin the client
// rect (less than 5 pixels from border).
//
// Original OWL comment:
// performs "auto-scrolling" (dragging the mouse from within the client
// client area of the Window to without results in auto-scrolling when
// the AutoMode data member of the Scroller is TRUE)
//
void TEditScroller::AutoScroll()
{
	// If ScrollLock enabled, don't do anything
	if (::GetKeyState(VK_SCROLL) & 0x0001)
		return;

	if (AutoMode && Window && ::AutoScroll)
	{
		TRect  clientRect;
		TPoint cursorPos;
		long  dx = 0, dy = 0;

		GetCursorPos(&cursorPos);
		Window->ScreenToClient(cursorPos);
		Window->GetClientRect(clientRect);

		if (cursorPos.y < 5)
			dy = min(-YLine, max(-YPage, -abs(cursorPos.y) / 10 * YLine));

		else if (cursorPos.y > clientRect.bottom - 5)
			dy = max(YLine, min(YPage, (-abs(cursorPos.y)-clientRect.bottom)/10 * YLine));

		if (cursorPos.x < 5)
			dx = min(-XLine, max(-XPage, -abs(cursorPos.x) / 10 * XLine));

		else if (cursorPos.x > clientRect.right - 5)
			dx = max(XLine, min(XPage, (-abs(cursorPos.x)-clientRect.right)/10 * XLine));

		ScrollBy(dx, dy);
	}
}


/////////////////////////////////////////////////////////////////////
// TEditScroller::ScrollTo
// -----------------------
//  Scrolls the map
//
void TEditScroller::ScrollTo(long x, long y)
{
	// Don't do anything when no window
	if ( ! Window )
		return;

	//
	// NOTE: x and y are given respectively in XUnit and YUnit units.
	//
	long  newXPos = max(0, min(x, XRange));
	long  newYPos = max(0, min(y, YRange));

	// If new position is same as old, don't do anything
	if ( newXPos == XPos  &&  newYPos == YPos )
		return;

	// If (x,y) outside Scroller range, don't do anything
	/*
	if (newXPos < 0 || x >= XRange || y < 0 || y >= YRange)
		return;
	*/

	// Delete selection box if any (XOR)
	TEditorClient *pEditor = TYPESAFE_DOWNCAST(Window, TEditorClient);
	if ( pEditor->StretchSelBox )
	{
		TMapDC dc(pEditor);
		pEditor->DrawStretchSelBox(dc);
	}
	// Delete dragging objects (XOR)
	else if ( pEditor->DragObject )
	{
		TMapDC dc(pEditor);
		pEditor->DrawMovingObjects(dc);
	}

	// Calculate the max values for OrigX and OrigY
	SHORT XMapMin = MAP_MIN_X + (SHORT)(ScrCenterX * DIV_SCALE);
	SHORT YMapMax = MAP_MAX_Y - (SHORT)(ScrCenterY * DIV_SCALE);

	// Setup map origin according to scroller pos
	// Note: We are sure the positions x and y are legal since we used
	//       AdjustScroller to set a proper Scroller range.
	if ( newXPos != XPos )
		OrigX = (SHORT)(newXPos * XUnit) + XMapMin;
	if ( newYPos != YPos )
		OrigY = YMapMax - (SHORT)(newYPos * YUnit); // Vertical axe is reversed!

	// Set scroller pos
	XPos = newXPos;
	YPos = newYPos;

	// Invalidate and repaint immediately entire window
	Window->Invalidate();
	Window->UpdateWindow();

	// Draw selection box of selected objects (XOR)
	if ( pEditor->StretchSelBox )
	{
		// Draw selection box
		TMapDC dc (pEditor);
		pEditor->DrawStretchSelBox(dc);
	}
	// Draw the dragging objects (using XOR mode)
	else if ( pEditor->DragObject )
	{
		TMapDC dc (pEditor);
		// Delete selected objects (COPY black)
		pEditor->DrawMovingObjects (dc, TRUE);
		// Draw selected objects (XOR)
		pEditor->DrawMovingObjects (dc);
	}
}

