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

	FILE:         windeapp.h

	OVERVIEW
	========
	Class definition for WinDEUApp (TApplication).
*/
#if !defined(__windeapp_h)              // Sentry, use file only if it's not already included.
#define __windeapp_h

#ifndef __common_h
	#include "common.h"
#endif

//{{TApplication = WinDEUApp}}
class WinDEUApp : public TApplication
{
private:
	int argc;
	char **argv;
	BOOL            HelpState;      // Has the help engine been used.
	BOOL            ContextHelp;	// SHIFT-F1 state (context sensitive HELP)
	HCURSOR         HelpCursor;     // Context sensitive help cursor
	int roomID;

public:
	WinDEUApp (int _argc, char **_argv);
	virtual ~WinDEUApp ();
	void AppHelp(UINT command, DWORD data);
	int GetRoomID(void) {return roomID;}
   void SetRoomID(int id) {roomID = id;}

//{{WinDEUAppVIRTUAL_BEGIN}}
public:
	virtual void InitInstance ();
	virtual void InitMainWindow();
	virtual bool CanClose ();
	virtual bool ProcessAppMsg (MSG& msg);
//{{WinDEUAppVIRTUAL_END}}

//{{WinDEUAppRSP_TBL_BEGIN}}
protected:
	void CmFileSetRoomID ();
	void CmHelpContents ();
	void CmHelpSearch ();
	void CmHelpUsingHelp ();
	void CmHelpAbout ();
	 void CmHelpTutorial ();
//{{WinDEUAppRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(WinDEUApp);
};    //{{WinDEUApp}}


#endif                                      // __windeapp_h sentry.
