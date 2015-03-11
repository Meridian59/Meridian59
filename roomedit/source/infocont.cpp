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

	FILE:         infocont.cpp

	OVERVIEW
	========
	Source file for implementation of TInfoControl (TControl).
*/

#include "common.h"
#pragma hdrstop

#ifndef __infocont_h
	#include "infocont.h"
#endif

#ifndef __mapdc_h
	#include "mapdc.h"
#endif

#include "cursor.rh"	// for IDC_HAND

//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TInfoControl, TControl)
//{{TInfoControlRSP_TBL_BEGIN}}
	EV_WM_NCHITTEST,
	EV_WM_SETCURSOR,
//{{TInfoControlRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TInfoControl Implementation}}

// Intialize class static vars.
TFont *TInfoControl::StaticFont = NULL ;
int TInfoControl::FontHeight = 0 ;
int TInfoControl::NbObjects = 0 ;


/////////////////////////////////////////////////////////////////////
// TInfoControl
// ------------
//
TInfoControl::TInfoControl (TWindow* parent, int id, unsigned _NumLines,
							const char* title, int x, int y, int w, int h,
							TModule* module):
	TControl(parent, id, title, x, y, w, h, module)
{
	//
	// Update Class static members
	//

	// Load a small font, common to most installed Windows in the world
	// This var is a static class var !
	if ( StaticFont == 0 )
		// staticFont = new TFont( "Courier", 12);
		StaticFont = new TFont( "MS Sans serif", 12);

	// Number of objects of this class ++
	NbObjects++ ;

	//
	//  Init object data members
	//

	// Override the default window style for TControl.
	Attr.Style = WS_BORDER | WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE;

	Attr.X = x;
	Attr.Y = y;
	Attr.W = w;
	Attr.H = h;

	TextLineTab = NULL ;
	NumLines = _NumLines;
	LinePos = 0;
	Inserting = FALSE;
	curColor = DEF_COLOR;
	curAlign = DEF_ALIGN;

	SetCursor (GetApplication(), IDC_HAND);
}


/////////////////////////////////////////////////////////////////////
// TInfoControl
// ------------
//
TInfoControl::~TInfoControl ()
{
	Destroy();
	Cleanup();

	// One object less
	NbObjects--;
	assert (NbObjects>=0);

	// If no more objects, destroy font
	if ( NbObjects == 0 )
	{
		delete StaticFont;
		StaticFont = 0;
		FontHeight = 0;
	}
}


/////////////////////////////////////////////////////////////////////
// TInfoControl
// ------------
//
void TInfoControl::Cleanup()
{
	if ( TextLineTab != NULL )
	{
		// Free TextLines
		for (unsigned int i = 0 ; i < NumLines ; i++)
		{
			if ( TextLineTab[i] != NULL )
			{
				if ( TextLineTab[i]->Text != NULL )
					FreeMemory (TextLineTab[i]->Text);
				FreeMemory (TextLineTab[i]);
			}
		}

		FreeMemory (TextLineTab);
		TextLineTab = NULL;
	}

	if ( IsWindow() )
		Invalidate();

	LinePos = 0;
}


/////////////////////////////////////////////////////////////////////
// TInfoControl
// ------------
//
void TInfoControl::SetupWindow ()
{
	TControl::SetupWindow();

	TRect wRect ;
	GetWindowRect (wRect);

	// Calculate font height on first time
	if ( FontHeight == 0 )
	{
		TClientDC dc(Handle);
		dc.SelectObject (*StaticFont);

		TEXTMETRIC textMetric;
		dc.GetTextMetrics( textMetric);
		FontHeight = textMetric.tmHeight;
		dc.RestoreFont();
	}

	// Calc no client area height
	int NCHeight = 2;
	NCHeight += ::GetSystemMetrics (SM_CYBORDER);

	MoveWindow (wRect.left, wRect.top,
				wRect.Width(), NumLines * FontHeight + NCHeight,
				FALSE);
}


/////////////////////////////////////////////////////////////////////
// TInfoControl
// ------------
//
void TInfoControl::Paint (TDC& dc, bool erase, TRect& rect)
{
	TControl::Paint(dc, erase, rect);

	TRect cRect = GetClientRect();

	// Select font
	dc.SelectObject (*StaticFont);
	dc.SetBkMode (TRANSPARENT);

	// Draw each text line
	if ( TextLineTab != NULL )
	{
		for (unsigned int i = 0 ; i < NumLines ; i++)
		{
			if (TextLineTab[i] != NULL  && TextLineTab[i]->Text != NULL)
			{
				int x;

				dc.SetTextColor ( TMapDC::GetColor16(TextLineTab[i]->Color));
				dc.SetTextAlign( TextLineTab[i]->Align);

				switch (TextLineTab[i]->Align & (TA_CENTER |TA_LEFT |TA_RIGHT))
				{
				case TA_LEFT:
					x = 0;
					break;
				case TA_CENTER:
					x = cRect.Width() / 2;
					break;
				case TA_RIGHT:
					x = cRect.Width() - 1;
					break;
				}

				dc.TextOut( x, FontHeight * i, TextLineTab[i]->Text);
			}
		}
	}
	dc.RestoreFont();
}


/////////////////////////////////////////////////////////////////////
// TInfoControl
// ------------
//
#if OWLVersion > OWLVERBC502
UINT TInfoControl::EvNCHitTest (const TPoint& /*point*/)
#else
UINT TInfoControl::EvNCHitTest (TPoint& /*point*/)
#endif
{
	return HTCAPTION;
}


/////////////////////////////////////////////////////////////////////
// TInfoControl
// ------------
//
bool TInfoControl::EvSetCursor (HWND hWndCursor, UINT hitTest,
								UINT /* mouseMsg */)
{
	if (hWndCursor == Handle && hitTest == HTCAPTION && HCursor)
	{
		::SetCursor(HCursor);
		return TRUE;
	}
	return (BOOL)DefaultProcessing();
}


/////////////////////////////////////////////////////////////////////
// TInfoControl
// ------------
//
void TInfoControl::BeginInsert ()
{
	assert (Inserting == FALSE);

	Inserting = TRUE ;
	LinePos = 0;
}


/////////////////////////////////////////////////////////////////////
// TInfoControl
// ------------
//
void TInfoControl::EndInsert ()
{
	assert (Inserting == TRUE);

	if ( IsWindow() )
		Invalidate();
	Inserting = FALSE ;
}


/////////////////////////////////////////////////////////////////////
// TInfoControl
// ------------
//
void TInfoControl::Insert (const char *format, ...)
{
	va_list args;
	char text[128];

	assert (LinePos < NumLines);

	va_start (args, format);
	vsprintf (text, format, args);
	va_end (args);

	// Allocates space for text lines tab.
	if ( TextLineTab == NULL )
	{
		TextLineTab = (TextLine **)GetMemory( NumLines * sizeof (TextLine *));
		for (unsigned int i = 0 ; i < NumLines ; i++)
			TextLineTab[i] = NULL;
	}

	// Allocate space for this line
	if ( TextLineTab[LinePos] == NULL)
	{
		TextLineTab[LinePos] = (TextLine *)GetMemory (sizeof(TextLine));
		TextLineTab[LinePos]->Text = NULL;
	}

	// Allocate space for the text in the line
	if ( TextLineTab[LinePos]->Text != NULL)
		FreeMemory (TextLineTab[LinePos]->Text);
	TextLineTab[LinePos]->Text = (char *)GetMemory (strlen(text)+1);

	// Initialize TextLine struct
	TextLineTab[LinePos]->Color = curColor;
	TextLineTab[LinePos]->Align = curAlign;
	strcpy (TextLineTab[LinePos]->Text, text);

	// If we are not in insert mode, redraw window
	if ( IsWindow() && Inserting == FALSE)
		Invalidate();

	LinePos ++;
}


/////////////////////////////////////////////////////////////////////
// TInfoControl
// ------------
//
void TInfoControl::InsertAt (int linePos, const char *format, ...)
{
	va_list args;
	char text[128];

	va_start (args, format);
	vsprintf (text, format, args);
	va_end (args);

	LinePos = linePos;
	Insert (text);
}


char* TInfoControl::GetClassName ()
{
	return "WinDEU_InfoControl";
}


void TInfoControl::GetWindowClass (WNDCLASS& wndClass)
{
    TControl::GetWindowClass(wndClass);

	wndClass.hbrBackground = (HBRUSH)::GetStockObject (WHITE_BRUSH);
}

