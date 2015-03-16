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

	FILE:         mainwnd.cpp

	OVERVIEW
	========
	Source file for implementation of TMainClient (TWindow).
	TMainClient is the default client window for the
	MainWindow frame.

*/

#include "common.h"
#pragma hdrstop

#include "maincli.h"

#ifndef OWL_INPUTDIA
	#include <owl\inputdia.h>
#endif

#ifndef __mainfram_h
	#include "mainfram.h"
#endif

#ifndef __mastdlg_h
	#include "mastdlg.h"
#endif

#ifndef __episdlg_h
	#include "episdlg.h"
#endif

#ifndef __entrydlg_h
	#include "entrydlg.h"
#endif

#ifndef __wadlidlg_h
	#include "wadlidlg.h"
#endif

#ifndef __viewspr_h
	#include "viewbmp.h"
#endif

#ifndef __wads_h
	#include "wads.h"
#endif

#ifndef __windeapp_h
	#include "windeapp.h"
#endif

#include "mainmenu.rh"

// Help context IDs
#include "windeuhl.h"


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TMainClient, TWindow)
//{{TMainClientRSP_TBL_BEGIN}}
	EV_COMMAND(CM_FILE_READPATCH, CmFileOpenWad),
//	EV_COMMAND(CM_FILE_SAVEOBJECT, CmFileSaveObject),
//	EV_COMMAND(CM_FILE_BUILDMAIN, CmFileBuildMain),
//	EV_COMMAND(CM_FILE_GROUPPATCH, CmFileGroup),
//	EV_COMMAND(CM_FILE_INSERTRAW, CmFileInsertRaw),
//	EV_COMMAND(CM_FILE_EXTRACTOBJ, CmFileExtractRaw),
//	EV_COMMAND(CM_FILE_DISPLAYFILES, CmFileListWads),
//	EV_COMMAND(CM_FILE_LISTMASTER, CmFileListMaster),
	EV_COMMAND(CM_FILE_EXIT, CmFileExit),
	EV_COMMAND(CM_EDITOR_CREATELEVEL, CmEditorCreate),
//	EV_COMMAND(CM_EDITOR_DUMPENTRY, CmEditorDump),
//	EV_COMMAND(CM_EDITOR_EDITLEVEL, CmEditorEdit),
//	EV_COMMAND(CM_EDITOR_VIEWSPRITE, CmEditorViewSprites),
	EV_COMMAND(CM_EDITOR_VIEWWTEXTURE, CmEditorViewWTextures),
//	EV_COMMAND(CM_EDITOR_VIEWFTEXTURE, CmEditorViewFTextures),
	EV_COMMAND(CM_HELP_MAIN_MENU, CmHelpMainMenu),
//{{TMainClientRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TMainClient Implementation}}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
TMainClient::TMainClient (TWindow* parent, const char* title,
						  TModule* module):
	TWindow(parent, title, module)
{
	// Common file file flags and filters for Open/Save As dialogs.
	// Filename and directory are computed in the member functions CmFileOpen,
	// and CmFileSaveAs.
	FileData.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	FileData.SetFilter("Room Files (*.ROO)|*.ROO|"
					   "All Files (*.*)|*.*|");

}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
TMainClient::~TMainClient ()
{
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::SetupWindow ()
{
	TWindow::SetupWindow();
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::CmFileOpenWad ()
{
	// save current working directory (of windeu32.exe)
	// the TFileOpenDialog below is going to set
	// it to the folder of the file being opened...
	char workdir[256];
	GetCurrentDirectory(256, workdir);

	SET_HELP_CONTEXT(Open_WAD_file);
	//
	// Display standard Open dialog box to select a file name.
	//
	*FileData.FileName = 0;
	if (TFileOpenDialog(this, FileData).Execute() == IDOK)
	{
		// restore workingdirectory to folder of windeu32.exe
		SetCurrentDirectory(workdir);
#if 0
		OpenPatchWad(FileData.FileName);
		CloseUnusedWadFiles();
#endif
		// Sets a new client window (the editor) and destroy
		// the old one (the main client)
		TMainFrame *MainFrame =
			TYPESAFE_DOWNCAST (GetApplication()->GetMainWindow(), TMainFrame);
		MainFrame->EditLevel (FileData.FileName, FALSE) ;
		
	}
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
// Saves an object to a .WAD file
void TMainClient::CmFileSaveObject ()
{
	static char ObjectName[12];
	WadPtr wad;
	MDirPtr entry;
	FILE *File;

	SET_HELP_CONTEXT(Save_object);
	// Get objet name
	// Stupid InputDialog box, would be better with a list box
	if ( TInputDialog (this, "Enter object name", "Object :",
					   ObjectName, 12).Execute() != IDOK )
		goto End;

	// Check the name of the object exists
	for (entry = MasterDir ; entry != NULL ; entry = entry->next)
		if ( strnicmp (entry->dir.name, ObjectName, 8) == 0 )
			break;

	if ( entry == NULL )
	{
		Notify ("The object \"%s\" doesn't exist", ObjectName);
		goto End;
	}


	//
	// Display standard Open dialog box to select a file name.
	//
	*FileData.FileName = 0;
	if (TFileSaveDialog(this, FileData).Execute() != IDOK)
		goto End;

	// Check WAD file isn't opened
	for (wad = WadFileList; wad; wad = wad->next)
		if ( !stricmp (FileData.FileName, wad->filename) )
			break;

	if (wad)
	{
		Notify ("The Wad file \"%s\" is already in use. You may not "
				"overwrite it.", FileData.FileName);
		goto End;
	}

	// Save object to file
	WorkMessage ("Saving directory entry data to \"%s\"",
				 FileData.FileName);

	File = fopen(FileData.FileName, "wb");
	if (File == NULL)
	{
		Notify ("Error opening output file \"%s\"", FileData.FileName);
		goto End;
	}
	SaveDirectoryEntry(File, ObjectName);
	fclose(File);

End:
	RESTORE_HELP_CONTEXT();
}



/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::CmFileBuildMain ()
{
	WadPtr wad ;

	SET_HELP_CONTEXT(Build_new_main);
	//
	// Display standard Open dialog box to select a file name.
	//
	*FileData.FileName = 0;
	if (TFileSaveDialog(this, FileData).Execute() == IDOK)
	{
		// Check that WAD file isn't opened in our list
		for (wad = WadFileList; wad != NULL ; wad = wad->next)
			if (stricmp( FileData.FileName, wad->filename) == 0)
				break;

		if (wad)
		{
			Notify ("File \"%s\" is opened and cannot be overwritten.",
					FileData.FileName);
			goto End;
		}

		// Everything is ok, we can build the new file
		BuildNewMainWad(FileData.FileName, FALSE);
	}

End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::CmFileGroup ()
{
	WadPtr wad ;

	SET_HELP_CONTEXT(Group_WAD_files);

	if (WadFileList->next == NULL || WadFileList->next->next == NULL)
	{
		Notify ("You need at least two open wad files "
				"if you want to group them.");
		goto End;
	}

	//
	// Display standard Open dialog box to select a file name.
	//
	*FileData.FileName = 0;
	if (TFileSaveDialog(this, FileData).Execute() == IDOK)
	{
		// Check that WAD file isn't opened in our list
		for (wad = WadFileList ; wad != NULL ; wad = wad->next)
			if (stricmp( FileData.FileName, wad->filename) == 0)
				break;

		if (wad)
		{
			Notify ("File \"%s\" is opened and cannot be overwritten.",
					FileData.FileName);
			goto End;
		}

		// Everithing is ok, wa can build the new file
		BuildNewMainWad(FileData.FileName, TRUE);
	}
End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::CmFileInsertRaw ()
{
	static char ObjectName[12];
	char input[MAX_PATH];
	MDirPtr entry;
	FILE *raw;
	FILE *file;
	WadPtr wad;

	SET_HELP_CONTEXT(Insert_RAW_file);
	//
	// Display standard Open dialog box to select a file name.
	//
	*FileData.FileName = 0;
	if (TFileOpenDialog(this, FileData).Execute() != IDOK)
		goto End;

	// Get objet name
	//TODO: Stupid InputDialog box, would be better with a list box
	if ( TInputDialog (this, "Enter object name", "Object :",
					   ObjectName, 12).Execute() != IDOK )
		goto End;

	// Check the name of the object exists
	for (entry = MasterDir ; entry != NULL ; entry = entry->next)
		if ( strnicmp (entry->dir.name, ObjectName, 8) == 0 )
			break;

	if ( entry == NULL )
	{
		Notify ("The object \"%s\" doesn't exist", ObjectName);
		goto End;
	}

	// Check WAD file isn't opened
	raw = fopen(FileData.FileName, "rb");
	if (raw == NULL)
	{
		Notify ("Error opening input file \"%s\"", FileData.FileName);
		goto End;
	}

	// kluge: Check WAD isn't opened
	strcpy (input, ObjectName);
	strcat (input, ".WAD");

	for (wad = WadFileList; wad; wad = wad->next)
		if (!stricmp( input, wad->filename))
		   break;

	if (wad)
	{
		Notify ("The Wad file \"%s\" is already in use. You may not "
				"overwrite it.", input);
		goto End;
	}

	// Opening .RAW file in .WAD entry
	WorkMessage ("Including new object %s in \"%s\".", ObjectName, input);

	file = fopen (input, "wb");
	if (file == NULL)
		Notify ("Error opening output file \"%s\"", input);

	SaveEntryFromRawFile (file, raw, ObjectName);

	fclose(raw);
	fclose(file);
End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::CmFileExtractRaw ()
{
	static char ObjectName[12];
	WadPtr wad;
	MDirPtr entry;
	FILE *file;

	SET_HELP_CONTEXT(Extract_object);
	// Get objet name
	// Stupid InputDialog box, would be better with a list box
	if ( TInputDialog (this, "Enter object name", "Object :",
					   ObjectName, 12).Execute() != IDOK )
		goto End;

	// Check the name of the object exists
	for (entry = MasterDir ; entry != NULL ; entry = entry->next)
		if ( strnicmp (entry->dir.name, ObjectName, 8) == 0 )
			break;

	if ( entry == NULL )
	{
		Notify ("The object \"%s\" doesn't exist", ObjectName);
		goto End;
	}

	//
	// Display standard Open dialog box to select a file name.
	//
	*FileData.FileName = 0;
	if (TFileSaveDialog(this, FileData).Execute() != IDOK)
		goto End;

	// Check WAD isn't opened
	for (wad = WadFileList; wad; wad = wad->next)
		if (!stricmp(FileData.FileName, wad->filename))
		   break;

	if (wad)
	{
		Notify ("You may not overwrite an opened Wad file with raw data");
		goto End;
	}

	// Opening .RAW file in .WAD entry
	WorkMessage ("Saving directory entry data to \"%s\".", FileData.FileName);

	file = fopen(FileData.FileName, "wb");
	if ( file == NULL)
	{
		Notify ("Error opening output file \"%s\"", FileData.FileName);
	}
	SaveEntryToRawFile (file, ObjectName);
	fclose (file);

End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::CmFileListWads ()
{
	SET_HELP_CONTEXT(List_opened_WAD_files);
	// Display the dialog box with the list of wad files
	TWadlistDialog (this).Execute();
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::CmFileListMaster ()
{
	SET_HELP_CONTEXT(List_master_directory);
	TMasterDialog(this).Execute();
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::CmFileExit ()
{
	// Close MainFrame
	GetApplication()->GetMainWindow()->CloseWindow();
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::CmEditorEdit ()
{
   return; // We don't like this option ARK
#if 0
	char LevelName[9];

	SET_HELP_CONTEXT(Edit_level);

	// Executes the dialog box to get level number
	if (TEpisodeMissionDialog (this, LevelName).Execute() == IDOK)
	{
		// Sets a new client window (the editor) and destroy
		// the old one (the main client)
		TMainFrame *MainFrame =
			TYPESAFE_DOWNCAST (GetApplication()->GetMainWindow(), TMainFrame);
		MainFrame->EditLevel (LevelName, FALSE) ;
	}
	RESTORE_HELP_CONTEXT();
	return;		// 'this' is not valid anymore
#endif
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::CmEditorCreate ()
{
	char LevelName[9];

	SET_HELP_CONTEXT(Create_level);
	// Executes the dialog box to get level number
	
#if 0
	sprintf(LevelName, "temp.roo");
	if (1)
#endif
	{
		// Sets a new client window (the editor) and destroy
		// the old one (the main client)
		TMainFrame *MainFrame =
			TYPESAFE_DOWNCAST(GetApplication()->GetMainWindow(), TMainFrame);
		MainFrame->EditLevel (LevelName, TRUE) ;
	}
	RESTORE_HELP_CONTEXT();
	return;		// 'this' is not valid anymore
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::CmEditorDump ()
{
	SET_HELP_CONTEXT(Dump_entry);
	TViewEntryDialog(this).Execute();
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::CmEditorViewSprites ()
{
	SET_HELP_CONTEXT(View_Sprites);
	TViewSpriteDialog(this).Execute();
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::CmEditorViewWTextures ()
{
	SET_HELP_CONTEXT(View_Wall_textures);
	TViewWallTextureDialog(this).Execute();
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::CmEditorViewFTextures ()
{
	SET_HELP_CONTEXT(View_Floor_Ceiling_textures);
	TViewFloorTextureDialog(this).Execute();
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////
// TMainClient
// -----------
//
void TMainClient::CmHelpMainMenu ()
{
	WinDEUApp *app = TYPESAFE_DOWNCAST(GetApplication(), WinDEUApp);
	app->AppHelp(HELP_CONTEXT, Main_menu);
}

