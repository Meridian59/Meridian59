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

	FILE:         mainfram.cpp

	OVERVIEW
	========
	Source file for implementation of TMainFrame (TDecoratedFrame).
*/

#include "common.h"
#pragma hdrstop

#ifndef OWL_STATUSBA_H
	#include <owl\statusba.h>
#endif

#ifndef OWL_TEXTGADG_H
	#include <owl\textgadg.h>
#endif

#ifndef __mainfram_h
	#include "mainfram.h"
#endif

#ifndef __maincli_h
	#include "maincli.h"
#endif

#ifndef __editcli_h
	#include "editcli.h"
#endif

#ifndef __tooltip_h
	#include "tooltip.h"
#endif

#include "owldef.rh"		// for IDI-SDIAPPLICATION
#include "mainmenu.rh"
#include "editmenu.rh"

#define ID_TIMER	900

//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TMainFrame, TDecoratedFrame)
//{{TMainFrameRSP_TBL_BEGIN}}
	EV_WM_TIMER,
	EV_WM_ACTIVATE,
//{{TMainFrameRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TMainFrame Implementation}}


/////////////////////////////////////////////////////////////////
// TMainFrame
// ----------
//
TMainFrame::TMainFrame (const char* title, TModule* module):
	TDecoratedFrame(0, title, new TMainClient (0,""), TRUE, module),
	tooltip((Tip::Style)(Tip::RoundedBorder | Tip::Shadow), (TFont*)0)
{
	inEditor = FALSE;
	activated = FALSE;

	// Override the default window style for TDecoratedFrame.
	Attr.Style |= WS_THICKFRAME | WS_CAPTION | WS_CLIPCHILDREN |
				  WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_VISIBLE;

	// Assign ICON w/ this application.
	SetIcon (GetApplication(), IDI_SDIAPPLICATION);

	// Menu associated with window and accelerator table associated with table.
	SetMenuAndAccel (MENU_MAIN);

	//
	// Create default toolbar New and associate toolbar buttons with commands.
	//
	controlBar = new TTipControlBar(tooltip, this);
	SetupMainControlBar();
	Insert(*controlBar, TDecoratedFrame::Top);
	controlBarOn = TRUE;

//	cb->SetDocking (FALSE);

	//
	// Create status bar
	//
	// sb = new TStatusBar(this, TGadget::Recessed, TStatusBar::NumLock);
	statusBar= new TTipStatusBar(tooltip, this, TGadget::Recessed,
								 TStatusBar::ScrollLock);

	// Free memory display
	TTextGadget *tg1 = new TTextGadget(1, TGadget::Recessed,
									   TTextGadget::Left, 12) ;
	statusBar->Insert (*tg1) ;

	// Scale and grid display
	TTextGadget *tg2 = new TTextGadget(2, TGadget::Recessed,
									   TTextGadget::Left, 13) ;
	statusBar->Insert (*tg2) ;

	Insert(*statusBar, TDecoratedFrame::Bottom);
	statusBarOn = TRUE;

}


/////////////////////////////////////////////////////////////////
// TMainFrame
// ----------
//
TMainFrame::~TMainFrame ()
{
	if ( timer != 0 )
		KillTimer (timer);
	Destroy();
}


/////////////////////////////////////////////////////////////////
// TMainFrame
// ----------
//  Create timer
void TMainFrame::SetupWindow ()
{
	TDecoratedFrame::SetupWindow();

	// Timer for memory refresh
	timer = SetTimer (ID_TIMER, 2500);
}


/////////////////////////////////////////////////////////////////
// TMainFrame
// ----------
//
void TMainFrame::SetMenuAndAccel (TResId id)
{
	AssignMenu (id);
	Attr.AccelTable = id;
	LoadAcceleratorTable();
	
	if (GetHandle()) 
		DrawMenuBar();
}


/////////////////////////////////////////////////////////////////
// TMainFrame
// ----------
//
void TMainFrame::EditLevel (char *levelName, BOOL newLevel)
{
	TRACE ("TMainFrame::StartEditLevel: start");
	assert (inEditor == FALSE) ;
	if ( inEditor )
		return;

	// set new menu and accel.
	SetMenuAndAccel (MENU_EDITOR);

	// Hide client window
	TWindow *pClient = GetClientWindow () ;
	pClient->ShowWindow (SW_HIDE);

	// Hide status bar
	if ( ! InfoShown )
		ToggleStatusBar ();

	// Setup gadgets for editor window
	SetupEditorControlBar();

	// Set editor client
	SetClientWindow (new TEditorClient (0, levelName, newLevel));
	statusBar->SetWindowPos(*GetClientWindow(),
									0, 0, 0, 0,
									SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
	controlBar->SetWindowPos(*GetClientWindow(),
							 0, 0, 0, 0,
							 SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
	// delete old client window
	// pClient->CloseWindow();
	delete pClient ;

	inEditor = TRUE ;
}


/////////////////////////////////////////////////////////////////
// TMainFrame
// ----------
//
BOOL TMainFrame::StopEditLevel ()
{
	TRACE ("TMainFrame::StopEditLevel: start");
	assert (inEditor == TRUE) ;
	if ( !inEditor )
		return TRUE;

	TEditorClient *pClient =
		TYPESAFE_DOWNCAST(GetClientWindow (), TEditorClient);

	// Try to save editor changes
	if ( pClient->SaveChanges() == FALSE )
		return FALSE;

	// Set new menu and accel.
	SetMenuAndAccel (MENU_MAIN);

	// Hide editor client window
	pClient->ShowWindow (SW_HIDE);

	// Show status and control bar
	if ( ! controlBarOn )		ToggleControlBar ();
	if ( ! statusBarOn ) 		ToggleStatusBar ();

	// Set new Main client window
	SetClientWindow (new TMainClient (0, ""));

	// Delete old client
	// pClient->CloseWindow();
	delete pClient ;

	if (GetStatusBar() && GetStatusBar()->GadgetCount() >= 3)
	{
		// Clear status bar (1+3, not 2)
		((TTextGadget *)GetStatusBar()->FirstGadget())->SetText("");
		((TTextGadget *)GetStatusBar()->FirstGadget()->NextGadget()->NextGadget())->SetText("");
	}

	// Setup gadgets for main window
	SetupMainControlBar();

	inEditor = FALSE ;

	return TRUE;
}


/////////////////////////////////////////////////////////////////
// TMainFrame
// ----------
//
void TMainFrame::ToggleStatusBar ()
{
	EvCommand (IDW_STATUSBAR, 0, 0);
	statusBarOn = !statusBarOn;
}


/////////////////////////////////////////////////////////////////
// TMainFrame
// ----------
//
void TMainFrame::ToggleControlBar ()
{
	EvCommand (IDW_CONTROLBAR, 0, 0);
	controlBarOn = !controlBarOn;
}


/////////////////////////////////////////////////////////////////
// TMainFrame
// ----------
//
void TMainFrame::SetupMainControlBar()
{
	if ( controlBar == NULL )
		return;

	// Empty the control bar
	TGadget *gadget;
	while ( (gadget = controlBar->FirstGadget()) != NULL )
	{
		controlBar->Remove(*gadget);
		delete gadget;
	}

	// Inserts main window gadgets
	// controlBar->Insert(*new TButtonGadget(CM_FILENEW, CM_FILENEW));
	controlBar->Insert(*new TButtonGadget(CM_FILE_READPATCH, CM_FILE_READPATCH));
//	controlBar->Insert(*new TButtonGadget(CM_FILE_SAVEOBJECT, CM_FILE_SAVEOBJECT));
	controlBar->Insert(*new TSeparatorGadget(6));
//	controlBar->Insert(*new TButtonGadget(CM_EDITOR_EDITLEVEL, CM_EDITOR_EDITLEVEL));
	controlBar->Insert(*new TButtonGadget(CM_EDITOR_CREATELEVEL, CM_EDITOR_CREATELEVEL));
//	controlBar->Insert(*new TSeparatorGadget(6));
//	controlBar->Insert(*new TButtonGadget(CM_FILE_INSERTRAW, CM_FILE_INSERTRAW));
//	controlBar->Insert(*new TButtonGadget(CM_FILE_EXTRACTOBJ, CM_FILE_EXTRACTOBJ));
//	controlBar->Insert(*new TSeparatorGadget(6));
//	controlBar->Insert(*new TButtonGadget(CM_FILE_GROUPPATCH, CM_FILE_GROUPPATCH));
//	controlBar->Insert(*new TButtonGadget(CM_FILE_BUILDMAIN, CM_FILE_BUILDMAIN));
	controlBar->Insert(*new TSeparatorGadget(6));
	controlBar->Insert(*new TButtonGadget(CM_HELP_TUTORIAL, CM_HELP_TUTORIAL));
	controlBar->Insert(*new TButtonGadget(CM_HELP_CONTENTS, CM_HELP_CONTENTS));

	// Layout new gadgets
	if ( controlBar->IsWindow() )
		controlBar->LayoutSession();
}


/////////////////////////////////////////////////////////////////
// TMainFrame
// ----------
//
void TMainFrame::SetupEditorControlBar()
{
	if ( controlBar == NULL )
		return;
	// Empty the control bar
	TGadget *gadget;
	while ( ( gadget = controlBar->FirstGadget()) != NULL )
	{
		controlBar->Remove(*gadget);
		delete gadget;
	}

	// Inserts editor window gadgets
	controlBar->Insert(*new TButtonGadget(CM_FILE_SAVE, CM_FILE_SAVE));
	controlBar->Insert(*new TSeparatorGadget(6));
	controlBar->Insert(*new TButtonGadget(CM_MODE_THINGS, CM_MODE_THINGS));
	controlBar->Insert(*new TButtonGadget(CM_MODE_VERTEXES, CM_MODE_VERTEXES));
	controlBar->Insert(*new TButtonGadget(CM_MODE_LINEDEFS, CM_MODE_LINEDEFS));
	controlBar->Insert(*new TButtonGadget(CM_MODE_SECTORS, CM_MODE_SECTORS));
	controlBar->Insert(*new TSeparatorGadget(6));
	controlBar->Insert(*new TButtonGadget(CM_OBJECTS_RECTANGLE, CM_OBJECTS_RECTANGLE));
	controlBar->Insert(*new TButtonGadget(CM_OBJECTS_POLYGON, CM_OBJECTS_POLYGON));
	controlBar->Insert(*new TButtonGadget(CM_OBJECTS_CIRCLE, CM_OBJECTS_CIRCLE));
	controlBar->Insert(*new TButtonGadget(CM_OBJECTS_TORCH, CM_OBJECTS_TORCH));
	controlBar->Insert(*new TSeparatorGadget(6));
	controlBar->Insert(*new TButtonGadget(CM_EDIT_UNDO, CM_EDIT_UNDO));
	controlBar->Insert(*new TButtonGadget(CM_EDIT_REDO, CM_EDIT_REDO));
	controlBar->Insert(*new TButtonGadget(CM_EDIT_DELETEOBJ, CM_EDIT_DELETEOBJ));
	controlBar->Insert(*new TButtonGadget(CM_EDIT_COPYOBJ, CM_EDIT_COPYOBJ));
	controlBar->Insert(*new TButtonGadget(CM_EDIT_ADDOBJ, CM_EDIT_ADDOBJ));
	controlBar->Insert(*new TSeparatorGadget(6));
	controlBar->Insert(*new TButtonGadget(CM_WINDOW_ZOOMIN, CM_WINDOW_ZOOMIN));
	controlBar->Insert(*new TButtonGadget(CM_WINDOW_ZOOMOUT, CM_WINDOW_ZOOMOUT));
	controlBar->Insert(*new TSeparatorGadget(6));
	controlBar->Insert(*new TButtonGadget(CM_WINDOW_INFOWIN, CM_WINDOW_INFOWIN));
	controlBar->Insert(*new TButtonGadget(CM_WINDOW_TOOLBAR, CM_WINDOW_TOOLBAR));
	controlBar->Insert(*new TButtonGadget(CM_WINDOW_INFOBAR, CM_WINDOW_INFOBAR));
	controlBar->Insert(*new TSeparatorGadget(6));
	controlBar->Insert(*new TButtonGadget(CM_WINDOW_SHOWGRID, CM_WINDOW_SHOWGRID));
	controlBar->Insert(*new TButtonGadget(CM_WINDOW_SNAPTOGRID, CM_WINDOW_SNAPTOGRID));
	controlBar->Insert(*new TButtonGadget(CM_WINDOW_GRIDNEXT, CM_WINDOW_GRIDNEXT));
	controlBar->Insert(*new TButtonGadget(CM_WINDOW_GRIDPREV, CM_WINDOW_GRIDPREV));
	controlBar->Insert(*new TSeparatorGadget(6));
	controlBar->Insert(*new TButtonGadget(CM_HELP_TUTORIAL, CM_HELP_TUTORIAL));
	controlBar->Insert(*new TButtonGadget(CM_HELP_CONTENTS, CM_HELP_CONTENTS));

	// Layout new gadgets
	if ( controlBar->IsWindow() )
		controlBar->LayoutSession();
}



/////////////////////////////////////////////////////////////////
// TMainFrame
// ----------
//
void TMainFrame::EvTimer (UINT timerId)
{
	DrawFreeMemory();
	TDecoratedFrame::EvTimer(timerId);
}


/////////////////////////////////////////////////////////////////
// TMainFrame
// ----------
//
void TMainFrame::DrawFreeMemory()
{
	char msg[40];

	if (!GetStatusBar() || GetStatusBar()->GadgetCount() <= 1)
		return;

	// Draw the memory info in the third text gadget of status bar
	wsprintf (msg, "Free mem: %sKb",
				   FormatNumber (::GetAvailMemory() / 1024L));
	((TTextGadget *)GetStatusBar()->FirstGadget()->NextGadget())->SetText(msg);
	GetStatusBar()->UpdateWindow();
}


/////////////////////////////////////////////////////////////////////
// PreProcessMsg
// -------------
//  Ignore mouse movements when window is deactivated
//
bool TMainFrame::PreProcessMsg (MSG& msg)
{
	switch (msg.message)
	{
		case WM_MOUSEMOVE:
			// Return non-zero if not activated
			if ( ! activated )
				return TRUE;
		break;
	}

	return TDecoratedFrame::PreProcessMsg(msg);
}


/////////////////////////////////////////////////////////////////////
// EvActivate
// ----------
//  Keep 'activated' member up to date.
//
void TMainFrame::EvActivate (UINT active, bool minimized, HWND hWndOther )
{
	if (active == WA_ACTIVE ||
		active == WA_CLICKACTIVE )
	{
		activated = TRUE;
	}
	else if (active == WA_INACTIVE )
	{
		activated = FALSE;
	}

	TDecoratedFrame::EvActivate(active, minimized, hWndOther );
}

