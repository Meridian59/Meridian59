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

	FILE:         maincli.h

	OVERVIEW
	========
	Class definition for TMainClient (TWindow).
*/
#if !defined(__maincli_h)              // Sentry, use file only if it's not already included.
#define __maincli_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_WINDOW_H
	#include <owl\window.h>
#endif

#ifndef OWL_OPENSAVE_H
	#include <owl\opensave.h>
#endif


//{{TWindow = TMainClient}}
class TMainClient : public TWindow
{
private:
	// Data to control open/saveas standard dialog.
	TOpenSaveDialog::TData FileData;

public:
	TMainClient (TWindow* parent, const char* title = 0,
				 TModule* module = 0);
	virtual ~TMainClient ();

//{{TMainClientVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
//{{TMainClientVIRTUAL_END}}


//{{TMainClientRSP_TBL_BEGIN}}
protected:
	void CmFileOpenWad ();
	void CmFileSaveObject ();
	void CmFileBuildMain ();
	void CmFileGroup ();
	void CmFileInsertRaw ();
	void CmFileExtractRaw ();
	void CmFileListWads ();
	void CmFileListMaster ();
	void CmFileExit ();
	void CmEditorCreate ();
	void CmEditorDump ();
	void CmEditorEdit ();
	void CmEditorViewSprites ();
	void CmEditorViewWTextures ();
	void CmEditorViewFTextures ();
    void CmHelpMainMenu ();
//{{TMainClientRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TMainClient);
};    //{{TMainClient}}

#endif                                      // __maincli_h sentry.

