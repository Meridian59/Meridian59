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

	FILE:         windeapp.cpp

	OVERVIEW
	========
	Source file for implementation of WinDEUApp (TApplication).
*/


#include "common.h"
#pragma hdrstop
#include <time.h>

#if 0
	#include <ctl3d.h>
#endif

#ifndef __windeapp_h
	#include "windeapp.h"		    // Definition of Application
#endif

#ifndef __windeapp_rh
	#include "windeapp.rh"
#endif

#ifndef __wads_h
	#include "wads.h"
#endif

#ifndef __mainfram_h
	#include "mainfram.h"           // Definition of TMainFrame
#endif

#ifndef __aboutdlg_h
	#include "aboutdlg.h"
#endif

//#include "roomid.h"

//
// Help constants
//
#include "windeuhl.h"

//
// Main window title
//
#define TITLE "BlakSton Room Editor version 2.2 (October 19, 2015)"

//
// Generated help file.
//
const char HelpName[] = "windeu.hlp";
static char HelpFileName[MAX_PATH];

//
// Context sensitive help item ID
//
DWORD HelpContext = Contents;

//
// WinHelp opened?
//
BOOL HelpOpened = FALSE;

//
// Hook Handle returned by SetWindowsHookEx(WH_KEYBOARD, ...)
//
HHOOK hKeyboardHook = NULL;

// Initial level name to load from command line
static char init_level[500];

//
// Hook Callback function called each time a key is pressed or released
// We only check for 'F1' key to do contextual help.
//
// int nCode;		// hook code
// WPARAM wParam;	// virtual-key code
// LPARAM lParam;	// keystroke-message information
//
// We must define it extern "C" because it's a Windows Callback Function
// Note: CALLBACK is defined as _far _pascal in WINDOWS.H (Win16)
//                is defined as __stdcall    in WINDEF.H  (Win32)
//       It means the Win32 hook function is case sensitive, so we
//       must change the .DEF file.
//
extern "C"
{
extern LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	// Call next hook in chain and return if nCode less than 0
	if ( nCode >= 0 )
	{
		// Call WinHelp if F1 key is pressed
		if ( ( wParam                == VK_F1 ) && /* F1          */
			 ((lParam & 0x80000000L) == 0     )    /* Key pressed */ )
		{
			WinDEUApp *app = TYPESAFE_DOWNCAST(::Module, WinDEUApp);
			app->AppHelp(HELP_CONTEXT, HelpContext);

			// Return 1 to avoid passing message to the active window
			// (The message is not pushed in the queue)
			return 1;
		}
	}

	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

//
// Called at exit
//
void _USERENTRY CleanupApp()
{
	if ( hKeyboardHook != NULL )
		UnhookWindowsHookEx (hKeyboardHook);
	CleanupWindeu();
}


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(WinDEUApp, TApplication)
//{{WinDEUAppRSP_TBL_BEGIN}}
	EV_COMMAND(CM_HELP_ABOUT, CmHelpAbout),
	EV_COMMAND(CM_HELP_CONTENTS, CmHelpContents),
	EV_COMMAND(CM_HELP_SEARCH, CmHelpSearch),
	EV_COMMAND(CM_HELP_USING_HELP, CmHelpUsingHelp),
	EV_COMMAND(CM_HELP_TUTORIAL, CmHelpTutorial),
	EV_COMMAND(CM_FILESET_ROOM_ID, CmFileSetRoomID),
//{{WinDEUAppRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{WinDEUApp Implementation}}


//////////////////////////////////////////////////////////
// WinDEUApp
// ---------
//
WinDEUApp::WinDEUApp (int _argc, char **_argv):
	TApplication(TITLE)
{
	// Copy command options to local members
	argc = _argc;
	argv = _argv;

	atexit(CleanupApp);
}


//////////////////////////////////////////////////////////
// WinDEUApp
// ---------
//
WinDEUApp::~WinDEUApp ()
{
}


//////////////////////////////////////////////////////////
// WinDEUApp
// ---------
// MainWindow intialization.
void WinDEUApp::InitMainWindow ()
{
	MainWindow = new TMainFrame (GetName());
}


//////////////////////////////////////////////////////////
// WinDEUApp
// ---------
// Application instance intialisation. Read the INI file
// for default application options.
void WinDEUApp::InitInstance ()
{
	// Initialize and create MainWindow
	TApplication::InitInstance();

	// Retrieve module directory to construct help file full path
	if ( GetModuleFileName (HelpFileName, MAX_PATH) )
	{
		int i;
		for (i = strlen(HelpFileName) - 1;
			 (i >= 0) && (HelpFileName[i] != '\\');
			 i--)
			;

		HelpFileName[i+1] = '\0';

		strcat (HelpFileName, HelpName);
	}

	// Keyboard hook for F1 help
	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD,
									 KeyboardProc,
									 *this,
									 0);
	// Parse command line and INI file.
	InitWindeu(argc, argv, init_level);

// removed annoyances ARK
#if 0
	// Create and display credits for WinDEU
	CmHelpAbout();

	//RP moved from InitWinDEU
	if (Quieter == TRUE)
		Quiet = TRUE;
	if (Reminder == TRUE)
		FunnyMessage();
#endif

	/* load the wad files */
	// Don't load Doom WAD   ARK
#if 0
	OpenMainWad(MainWad);
	if (PatchWads)
		while (PatchWads[ 0])
		{
			OpenPatchWad( strupr( PatchWads[ 0]));
			PatchWads++;
		}
#endif

	/* sanity check */
	CloseUnusedWadFiles();

	// Load initial level if specified on command line
	if (init_level[0] != 0)
	{
		// Sets a new client window (the editor) and destroy
		// the old one (the main client)
		TMainFrame *MainFrame =
			TYPESAFE_DOWNCAST(MainWindow->GetApplication()->GetMainWindow(), TMainFrame);
		MainFrame->EditLevel (init_level, FALSE) ;
	}
}


//////////////////////////////////////////////////////////
// WinDEUApp
// ---------
// Ask for confirmation before exiting application
bool WinDEUApp::CanClose ()
{
	BOOL result;

	// Default member function
	if ( TApplication::CanClose() == FALSE )
		return FALSE;

	TMainFrame *mainFrame = TYPESAFE_DOWNCAST(GetMainWindow(), TMainFrame);
	// Close level editor and return
	if ( mainFrame->inEditor )
	{
		mainFrame->StopEditLevel();
		return FALSE;
	}

	// Last chance message
/*
	result = mainFrame->MessageBox (
				"Do you really want to quit?",
				"Quit",
				MB_YESNO | MB_ICONQUESTION) == IDYES ? TRUE : FALSE ;
*/
	result = TRUE;

	// Close WinHelp
	if ( result && HelpOpened )
	{
		mainFrame->WinHelp(HelpFileName, HELP_QUIT, 0L);
		HelpOpened = FALSE;
	}

	return result;
}


/////////////////////////////////////////////////////////////////
// WinDEUApp
// ---------
//
bool WinDEUApp::ProcessAppMsg (MSG& msg)
{
#if 0

	BOOL result;

	if (msg.message == WM_COMMAND)
	{
		if (ContextHelp || (GetKeyState(VK_F1) < 0))
		{
			ContextHelp = FALSE;
			GetMainWindow()->WinHelp(HelpFileName, HELP_CONTEXT, msg.wParam);
			return TRUE;
		}
	}
	else
	{
		switch (msg.message)
		{
		case WM_KEYDOWN:
			if (msg.wParam == VK_F1)
			{
				// If the Shift/F1 then set the help cursor and turn on the modal help state.
				if (::GetKeyState(VK_SHIFT) < 0)
				{
					ContextHelp = TRUE;
					HelpCursor = ::LoadCursor(GetMainWindow()->GetModule()->GetInstance(), MAKEINTRESOURCE(IDC_HELPCURSOR));
					::SetCursor(HelpCursor);
					return TRUE;        // Gobble up the message.
				}
				else
				{
					// If F1 w/o the Shift key then bring up help's main index.
					// GetMainWindow()->WinHelp(HelpFileName, HELP_INDEX, 0L);
					GetMainWindow()->WinHelp(HelpFileName, HelpContext, 0L);
					return TRUE;        // Gobble up the message.
				}
			}
			else
			{
				if (ContextHelp && (msg.wParam == VK_ESCAPE))
				{
					if (HelpCursor)
						::DestroyCursor(HelpCursor);
					ContextHelp = FALSE;
					HelpCursor = 0;
					GetMainWindow()->SetCursor(0, IDC_ARROW);
					return TRUE;    // Gobble up the message.
				}
			}
			break;

		case WM_MOUSEMOVE:
		case WM_NCMOUSEMOVE:
			if (ContextHelp)
			{
				::SetCursor(HelpCursor);
				return TRUE;        // Gobble up the message.
			}
			break;

		case WM_INITMENU:
			if (ContextHelp)
			{
				::SetCursor(HelpCursor);
				return TRUE;        // Gobble up the message.
			}
			break;

		case WM_ENTERIDLE:
			if (msg.wParam == MSGF_MENU)
			{
				if (GetKeyState(VK_F1) < 0)
				{
					ContextHelp = TRUE;
					GetMainWindow()->PostMessage(WM_KEYDOWN, VK_RETURN, 0L);
					return TRUE;       // Gobble up the message.
				}
			}
			break;

		default:
			;
		}  // End of switch
	}

#endif
	return TApplication::ProcessAppMsg(msg);
}


/////////////////////////////////////////////////////////////////
// WinDEUApp
// ---------
//
void WinDEUApp::AppHelp(UINT command, DWORD data)
{
	HelpOpened = GetMainWindow()->WinHelp(HelpFileName, command, data);
}

void WinDEUApp::CmFileSetRoomID()
{
//	TSetRoomIDDialog(GetMainWindow()).Execute();
}

/////////////////////////////////////////////////////////////////
// WinDEUApp
// ---------
//
void WinDEUApp::CmHelpAbout ()
{
	//
	// Show the modal dialog.
	//
	SET_HELP_CONTEXT(About_WinDEU);
	WindeuAboutDlg(GetMainWindow()).Execute();
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////
// WinDEUApp
// ---------
//
void WinDEUApp::CmHelpContents ()
{
	AppHelp(HELP_CONTENTS, 0L);
}


/////////////////////////////////////////////////////////////////
// WinDEUApp
// ---------
//
void WinDEUApp::CmHelpSearch ()
{
	AppHelp(HELP_PARTIALKEY, (DWORD)"");
}




/////////////////////////////////////////////////////////////////
// WinDEUApp
// ---------
//
void WinDEUApp::CmHelpTutorial ()
{
	AppHelp (HELP_CONTEXT, Tutorial);
}


/////////////////////////////////////////////////////////////////
// WinDEUApp
// ---------
//
void WinDEUApp::CmHelpUsingHelp ()
{
	AppHelp(HELP_HELPONHELP, 0L);
}


//////////////////////////////////////////////////////////
// OwlMain
// -------
//
int OwlMain (int argc, char* argv[])
{
	WinDEUApp App(argc, (char **)argv);

	return App.Run();
}

