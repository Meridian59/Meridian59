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

	FILE:         aboutdlg.h

	OVERVIEW
	========
	Class definition for WindeuAboutDlg (TDialog).
*/
#if !defined(__aboutdlg_h)              // Sentry, use file only if it's not already included.
#define __aboutdlg_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_GDIOBJEC_H
	#include "owl\gdiobjec.h"
#endif

#include "aboutdlg.rh"


//{{TDialog = WindeuAboutDlg}}
class WindeuAboutDlg : public TDialog
{
public:
	WindeuAboutDlg (TWindow *parent, TResId resId = IDD_ABOUT, TModule *module = 0);
	virtual ~WindeuAboutDlg ();

//{{WindeuAboutDlgVIRTUAL_BEGIN}}
public:
    void SetupWindow ();
//{{WindeuAboutDlgVIRTUAL_END}}

//{{WindeuAboutDlgRSP_TBL_BEGIN}}
//{{WindeuAboutDlgRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(WindeuAboutDlg);
};    //{{WindeuAboutDlg}}


#endif                                      // __aboutdlg_h sentry.
