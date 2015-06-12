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

	FILE:         tooltip.cpp
	Author:  	  Steve Saxon (Compuserve: 100321,2355)
	Written:      26th June 1994

	OVERVIEW
	========
	Source file for implementation of TToolTip, TTipControlBar, TTipStatusBar.
*/
#include "common.h"
#pragma hdrstop

#ifndef __tooltip_h
	#include "tooltip.h"
#endif

#define TIP_DELAY_FIRST	800
#define TIP_DELAY_AFTER	150

#define ID_TIMER	1000

static BOOL 	bEnableHints = FALSE;	// prevents menus displaying hints!!

static TToolTip      *ptTooltip;		// pointer used by KbdProc
static TTipStatusBar *ptStatusBar;		// pointer used to hide tip when LMB click
static HHOOK	hookKbd;				// hookchain used by KbdProc

LRESULT CALLBACK KbdProc (int code, WPARAM wParam, LPARAM lParam);


/////////////////////////////////////////////////////////////////////
//                                                                 //
//                                                                 //
//  TTipControlBar Window class                                    //
//                                                                 //
//                                                                 //
/////////////////////////////////////////////////////////////////////


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1 (TTipControlBar, TControlBar)
//{{TTipControlBarRSP_TBL_BEGIN}}
  EV_WM_MOUSEMOVE,
  EV_WM_LBUTTONDOWN,
  EV_WM_LBUTTONUP,
  // EV_WM_NCHITTEST,
//{{TTipControlBarRSP_TBL_END}}
END_RESPONSE_TABLE;

//{{TTipControlBar Implementation}}

TTipControlBar::TTipControlBar (TToolTip& tip, TWindow* parent, TTileDirection direction, TFont* font, TModule* module)
	 : 	TControlBar (parent, direction, font, module),
		tooltip (tip)
{
	Attr.Id = IDW_CONTROLBAR;

	SetHintMode(TGadgetWindow::EnterHints);
}

void TTipControlBar::EvMouseMove (UINT modKeys, const TPoint& point)
{
	if (!Capture && !GadgetFromPoint (point))
	{
		// hide the tip window if not over a gadget
		tooltip.HideTip ();
	}

	bEnableHints = TRUE;

	TControlBar::EvMouseMove (modKeys, point);

	bEnableHints = FALSE;
}

void TTipControlBar::EvLButtonDown (UINT modKeys, const TPoint& point)
{
	// hide the tip window if mouse-button pressed
	tooltip.HideTip ();

	TControlBar::EvLButtonDown (modKeys, point);
}

void TTipControlBar::EvLButtonUp (UINT modKeys, const TPoint& point)
{
	// Hide hint text
	ptStatusBar->SetHintText(NULL);

	TControlBar::EvLButtonUp (modKeys, point);
}

#if 0
UINT TTipControlBar::EvNCHitTest (TPoint& screenPt)
{
	// WM_NCHITTEST
	TPoint point = screenPt;

	ScreenToClient(point);
	if (!Capture && !GadgetFromPoint (point))
	{
		// hide the tip window if mouse-button pressed
		tooltip.HideTip ();

		// hide the tip window if not over a gadget
		return HTCAPTION;
	}

	return TControlBar::EvNCHitTest (screenPt);
}
#endif


/////////////////////////////////////////////////////////////////////
//                                                                 //
//                                                                 //
//  TTipStatusBar Window class                                     //
//                                                                 //
//                                                                 //
/////////////////////////////////////////////////////////////////////


DEFINE_RESPONSE_TABLE1 (TTipStatusBar, TStatusBar)
  EV_WM_MOUSEMOVE,
  EV_WM_LBUTTONDOWN,
END_RESPONSE_TABLE;

TTipStatusBar::TTipStatusBar (TToolTip& tip, TWindow* parent, TGadget::TBorderStyle borderStyle, UINT modeIndicators, TFont *font, TModule* module)
	: 	TStatusBar (parent, borderStyle, modeIndicators, font, module),
		tooltip (tip)
{
	bShowTips = FALSE;
	ptStatusBar = this;
}

void TTipStatusBar::SetHintText (const char *lpszText)
{
	// when hint message displayed on the status bar,
	// pick out the tooltip text and display it (with delay)

	if (lpszText != NULL)
	{
		static char	buf[128];
		int n;

		lstrcpy (buf, lpszText);

		// locate the tooltip text
		for (n = 0; buf[n] && buf[n] != '\n'; n++) ;

		if (buf[n])
		{
			buf[n++] = '\0';
		}

		// only display hints from gadgets (not menus!)
		if (bEnableHints)
			tooltip.SetCaption (buf + n);

		if (bShowTips)
			SetTextEx (buf);
		else
			DrawHintText(buf);
	}
	else
	{
		tooltip.SetCaption (NULL);

		if (bShowTips)
			SetTextEx (NULL);
		else
			DrawHintText(NULL);
	}
}


/////////////////////////////////////////////////////////////////////
// TTipStatusBar::SetHintTitle
// ---------------------------
//  Set main window caption to hint text
//
void TTipStatusBar::DrawHintText (const char *lpszText)
{
	TFrameWindow &mainWindow = *((TApplication *)::Module)->GetMainWindow();

	// Redraw window title if not hint text
	if ( lpszText == NULL || lpszText[0] == '\0' )
	{
		char title[128];

		strcpy (title, mainWindow.Title);
		mainWindow.SetCaption(title);
		return;
	}

	TRect rc2;
	mainWindow.GetWindowRect(rc2);

	// Compute the caption bar's origin. This window has a system
	// box, a minimize box, a maximize box, and has a resizeable
	// frame.
	TRect rc1;
	rc1.left = GetSystemMetrics(SM_CXFRAME) +
			   GetSystemMetrics(SM_CXSIZE) +
			   GetSystemMetrics(SM_CXBORDER);
	rc1.top = GetSystemMetrics(SM_CYFRAME);

	//
	// If we are running Win95 or greater, then 3rd box on right of title bar
	//
	// Note: This line display 70005F03 got GetVerion() under the final
	//       beta release of Win95. So, the Win95 version is 395 (0x03 + 0x5F)
	// Notify ("GetVersion() : %08lx", ::GetVersion());
	if ( LOBYTE(LOWORD(::GetVersion())) * 100 +
		 HIBYTE(LOWORD(::GetVersion())) >= 390)
	{
		rc1.right  = rc2.Width()
					 - GetSystemMetrics(SM_CXSIZE)		// minimize bitmap
					 - GetSystemMetrics(SM_CXSIZE)		// maximize bitmap
					 - GetSystemMetrics(SM_CXBORDER)	// border separator
					 - GetSystemMetrics(SM_CXSIZE) 		// close bitmap
					 - GetSystemMetrics(SM_CXBORDER);	// border separator
//					 - GetSystemMetrics(SM_CXFRAME);    // right frame
	}
	else
	{
		rc1.right  = rc2.Width()
					 - GetSystemMetrics(SM_CXBORDER)	// border separator
					 - GetSystemMetrics(SM_CXSIZE)		// minimize bitmap
					 - GetSystemMetrics(SM_CXSIZE)		// maximize bitmap
					 - GetSystemMetrics(SM_CXBORDER)	// right border
					 - GetSystemMetrics(SM_CXFRAME);    // right frame
	}

	rc1.bottom = rc1.top + GetSystemMetrics(SM_CYSIZE);

	// render the caption
	// Use the active caption color as the text background.
	TWindowDC dc(mainWindow);
	dc.FillRect(rc1, TBrush(GetSysColor(COLOR_ACTIVECAPTION)));
	dc.SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));
	dc.SetBkColor(GetSysColor(COLOR_ACTIVECAPTION));
	rc1.left += 3 ;	// Offset text by 3 pixels
	dc.DrawText(lpszText, -1, rc1, DT_LEFT);
}

void TTipStatusBar::EvMouseMove (UINT modKeys, const TPoint& point)
{
	if (bShowTips)
	{
		if (!Capture && !GadgetFromPoint (point))
		{
			// hide the tip window if not over a gadget
			tooltip.HideTip ();
		}

		bEnableHints = TRUE;

		TStatusBar::EvMouseMove (modKeys, point);

		bEnableHints = FALSE;
	}
	else
	{
		TStatusBar::EvMouseMove (modKeys, point);
	}
}

void TTipStatusBar::EvLButtonDown (UINT modKeys, const TPoint& point)
{
	if (bShowTips)
	{
		// hide the tip window if mouse-button pressed
		tooltip.HideTip ();
	}

	TStatusBar::EvLButtonDown (modKeys, point);
}

void TTipStatusBar::UseTips ()
{
	bShowTips = TRUE;

	Margins.Units = TMargins::BorderUnits;

	if (Direction == Horizontal)
	{
		Margins.Left = Margins.Right = 6;
		Margins.Top = Margins.Bottom = 2;
	}
}


/////////////////////////////////////////////////////////////////////
//                                                                 //
//                                                                 //
//  TToolTip Window class                                          //
//                                                                 //
//                                                                 //
/////////////////////////////////////////////////////////////////////


DEFINE_RESPONSE_TABLE1 (TToolTip, TWindow)
  EV_WM_TIMER,
END_RESPONSE_TABLE;

// ================================================================ //

TToolTipFont::TToolTipFont () : TFont ("MS Sans Serif", -9)
{
}

// ================================================================ //

TToolTip::TToolTip (Tip::Style _style, TFont* _font) : TWindow (NULL, "")
{
	style			= _style;
	font			= _font;

	Attr.Style 		= WS_POPUP;
	Attr.ExStyle 	= WS_EX_TOPMOST;

	uiTimer			= NULL;
	bEnabled		= TRUE;

	::hookKbd		= SetWindowsHookEx (WH_KEYBOARD, KbdProc, NULL, GetCurrentThreadId ());
	::ptTooltip		= this;

	Create ();
}

TToolTip::~TToolTip ()
{
	KillTipTimer ();

	UnhookWindowsHookEx (hookKbd);

	delete font;
}

void TToolTip::GetWindowClass (WNDCLASS &wc)
{
	TWindow::GetWindowClass (wc);

	// no background brush
	wc.hbrBackground = NULL;

	wc.style |= CS_SAVEBITS;
}

LPSTR TToolTip::GetClassName ()
{
	return "WinDEUTooltip";
}

void TToolTip::Paint (TDC &dc, bool, TRect &)
{
	char	szText[50];
	TRect	client;

	// get the tooltip text
	GetWindowText (szText, sizeof (szText));
	GetClientRect (client);
	int rad = client.Height () * 3 / 4;		// radius for rounded tip

	if (style & Tip::Shadow)
	{
		client.left += 4;
		client.top  += 4;

		BYTE		byData[] = { 0x55, 0, 0xAA, 0, 0x55, 0, 0xAA, 0, 0x55, 0, 0xAA, 0, 0x55, 0, 0xAA, 0 };
		TBitmap		bm (8, 8, 1, 1, byData);
		TBrush		brush (bm);
		dc.SelectObject (brush);
		dc.SelectStockObject (NULL_PEN);

		if (style & Tip::RoundedBorder)
			dc.RoundRect (client, TPoint (rad, rad));
		else
			dc.Rectangle (client);

		client = client.OffsetBy(-4, -4);
		//client.right  -= 4;
		//client.bottom -= 4;
	}

	// TPen	pen (GetSysColor (COLOR_WINDOWFRAME));
	TPen	pen (TColor::Black);
	TBrush	brush (TColor::LtYellow);
	// TBrush	brush (RGB (255, 255, 128));

	dc.SelectObject (pen);
	dc.SelectObject (brush);

	if (style & Tip::RoundedBorder)
		dc.RoundRect (client, TPoint (rad, rad));
	else
		dc.Rectangle (client);

	// set up the device context
	dc.SetBkMode (TRANSPARENT);
	dc.SelectObject (*font);

	// draw the text
	dc.SetTextColor (TColor::Black);
	dc.DrawText (szText, -1, client, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
}

void TToolTip::SetCaption (const char* title)
{
	static DWORD dwTickCount = 0;

	TWindow::SetCaption (title);

	// if the caption is missing, hide the tip window
	if (title == NULL || !*title || !bEnabled)
	{
		KillTipTimer ();

		ShowWindow (SW_HIDE);

		dwTickCount = GetTickCount ();
	}
	else
	{
		DWORD dwTickNew = GetTickCount ();

		// work out the extent of the text
		{
			TClientDC	dc (Handle);

			dc.SelectObject (*font);
			sizeText = dc.GetTextExtent (title, lstrlen (title));

			sizeText.cx	+= 5;
			sizeText.cy	+= 4;

			ShowWindow (SW_HIDE);
		}

		// create the timer - this will send a WM_TIMER message
		// after 'TIP_DELAY_xxxx' milliseconds

		if (dwTickNew - dwTickCount > 0)
		{
			uiTimer = SetTimer (ID_TIMER, TIP_DELAY_FIRST);
		}
		else
		{
			uiTimer = SetTimer (ID_TIMER, TIP_DELAY_AFTER);
		}

		dwTickCount = 0;
	}
}

void TToolTip::PositionTip ()
{
	// position the tip relative to the mouse
	TPoint	ptMouse;
	TSize	scr (GetSystemMetrics (SM_CXSCREEN), GetSystemMetrics (SM_CYSCREEN));

	GetCursorPos (ptMouse);

	ptMouse.x	-= 2;
	// ptMouse.y	+= 22;
	ptMouse.y	+= 26;

	TRect	rc (ptMouse, sizeText);

	if (style & Tip::RoundedBorder)
	{
		rc = rc.InflatedBy (4, 2);
	}

	// check x screen position
	if (rc.left < 0)
	{
		rc.Offset (-rc.left + 2, 0);
	}
	else
	{
		if (rc.right > scr.cx)
		{
			rc.Offset (scr.cx - rc.right - 2, 0);
		}
	}

	// check y screen position
	if (rc.bottom > scr.cy)
	{
		rc.Offset (0, -42);
	}

	if (style & Tip::Shadow)
	{
		rc.right	+= 4;
		rc.bottom	+= 4;
	}

	SetWindowPos (NULL, rc, SWP_NOZORDER | SWP_NOACTIVATE);
}

void TToolTip::KillTipTimer ()
{
	// destroy the timer
	if (uiTimer)
	{
		KillTimer (ID_TIMER);
		uiTimer = NULL;
	}
}

void TToolTip::ShowNow ()
{
	// position the tip window
	PositionTip ();

	// show the tip window
	ShowWindow (SW_SHOWNA);
	UpdateWindow ();
}

void TToolTip::EvTimer (UINT)
{
	// timer message received - show the tip window!
	if (uiTimer)
	{
		KillTipTimer ();	// prevent further timer messages

		ShowNow ();
	}
}

/////////////////////////////////////////////////////////////////////
// Hides the tip window when a key is pressed/released
//
//
LRESULT CALLBACK KbdProc (int code, WPARAM wParam, LPARAM lParam)
{
	if (code >= 0)
	{
		ptTooltip->HideTip ();
	}

	return CallNextHookEx (hookKbd, code, wParam, lParam);
}

