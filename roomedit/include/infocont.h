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

	FILE:         infocont.h

	OVERVIEW
	========
	Class definition for TInfoControl (TControl).
*/
#if !defined(__infocont_h)              // Sentry, use file only if it's not already included.
#define __infocont_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef __gfx_h
	#include "gfx.h"	// Colors
#endif

#ifndef OWL_CONTROL_H
	#include <owl\control.h>
#endif


#define DEF_COLOR	LIGHTBLUE
#define DEF_ALIGN	TA_LEFT

//{{TControl = TInfoControl}}
class TInfoControl : public TControl
{
public:
	static TFont *StaticFont;
	static int FontHeight;
	static int NbObjects;

private:
	UINT curAlign;
	int curColor;
	BOOL Inserting;			// Inserting text in control ?
	unsigned NumLines;		// Maximum number of lines
	unsigned LinePos;       // Next line number
	struct TextLine
	{
		int Color;
		UINT Align;
		char *Text;
	};
	TextLine **TextLineTab;

public:
	TInfoControl (TWindow* parent, int id, unsigned _NumLines, const char* title,
				  int x=100, int y=100, int w=200, int h=50,
				  TModule* module = 0);
	virtual ~TInfoControl();

	void SetStyle (UINT align = DEF_ALIGN, int color = DEF_COLOR)
	{
		curAlign = align;
		curColor = color;
	}
	void BeginInsert();
	void Insert( const char *text, ...);
	void InsertAt (int linePos, const char *text, ...);
	void Cleanup();
	void EndInsert();

//{{TInfoControlVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow();
	virtual void Paint( TDC& dc, bool erase, TRect& rect);
    virtual char* GetClassName ();
    virtual void GetWindowClass (WNDCLASS& wndClass);
//{{TInfoControlVIRTUAL_END}}

//{{TInfoControlRSP_TBL_BEGIN}}
protected:
	UINT EvNCHitTest (const TPoint& point);
	bool EvSetCursor (HWND hWndCursor, UINT hitTest, UINT mouseMsg);
//{{TInfoControlRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TInfoControl);
};    //{{TInfoControl}}


#endif                                      // __infocont_h sentry.

