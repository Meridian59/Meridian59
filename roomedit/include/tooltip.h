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

	FILE:         tooltip.h
	Original Author:  Steve Saxon (Compuserve: 100321,2355)

	OVERVIEW
	========
	Class definition for TToolTip, TTipControlBar, TTipStatusBar.
*/
#ifndef __tooltip_h
#define __tooltip_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_CONTROLB_H
	#include <owl\controlb.h>
#endif

#ifndef OWL_STATUSBA_H
	#include <owl\statusba.h>
#endif

#ifndef OWL_BUTTONGA_H
	#include <owl\buttonga.h>
#endif

#define IDW_CONTROLBAR		5500

class TToolTip;


//{{TControlBar = TTipControlBar}}

class TTipControlBar : public TControlBar
{
	TToolTip&	tooltip;

public:
	TTipControlBar (TToolTip& tip, TWindow* parent = 0, TTileDirection direction = Horizontal, TFont* font = new TGadgetWindowFont, TModule* module = 0);

protected:
//{{TTipControlBarRSP_TBL_BEGIN}}

	void 	EvMouseMove (UINT modKeys, const TPoint& point);
	void 	EvLButtonDown (UINT modKeys, const TPoint& point);
	void 	EvLButtonUp (UINT modKeys, const TPoint& point);
	UINT    EvNCHitTest (TPoint& screenPt);
//{{TTipControlBarRSP_TBL_END}}
	DECLARE_RESPONSE_TABLE (TTipControlBar);
};    //{{TTipControlBar}}



class TTipStatusBar : public TStatusBar
{
	TToolTip&	tooltip;
	BOOL		bShowTips;

public:
	TTipStatusBar (	TToolTip& 				tip,
					TWindow*				parent = 0,
					TGadget::TBorderStyle 	borderStyle = TGadget::Recessed,
					UINT                  	modeIndicators = 0,
					TFont*                	font = new TGadgetWindowFont,
					TModule*              	module = 0);

	void 			EvMouseMove (UINT modKeys, const TPoint& point);
	void 			EvLButtonDown (UINT modKeys, const TPoint& point);

	void 			SetHintText (const char *lpszText);
	void            DrawHintText (const char *lpszText);
	virtual void 	SetTextEx (const char *lpszText) { SetText (lpszText); }
	void			UseTips ();

	DECLARE_RESPONSE_TABLE (TTipStatusBar);
};



class TToolTipFont : public TFont
{
public:
	TToolTipFont ();
};


class Tip
{
public:
	enum Style
	{
		SquareBorder	= 0x00,
		RoundedBorder	= 0x01,
		Shadow			= 0x02,
	};
};


class TToolTip : private TWindow
{
	friend	TTipControlBar;
	friend	TTipStatusBar;

	UINT	uiTimer;
	BOOL	bEnabled;
	TSize	sizeText;

	Tip::Style	style;
	TFont* 	font;

	void	KillTipTimer ();
	void	ShowNow ();
	void	PositionTip ();

protected:
	void	GetWindowClass (WNDCLASS &wc);
	LPSTR	GetClassName ();

	void 	Paint (TDC &dc, bool erase, TRect &rc);
	void	EvTimer (UINT uiTimerId);

	void	SetCaption (const char* title);

public:
	TToolTip (Tip::Style style = Tip::SquareBorder, TFont* font = new TToolTipFont);
	~TToolTip ();

	BOOL	AreTipsEnabled () 					{ return bEnabled; }
	void	EnableTips (BOOL bEnable = TRUE) 	{ bEnabled = bEnable; }
	void	HideTip ()							{ SetCaption (NULL) ; }

	DECLARE_RESPONSE_TABLE (TToolTip);
};


#endif