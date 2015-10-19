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

	FILE:         editcli.cpp

	OVERVIEW
	========
	Source file for implementation of TEditorClient (TLayoutWindow).
*/

#include "common.h"
#pragma hdrstop

#ifndef __editcli_h
	#include "editcli.h"
#endif

#ifndef OWL_LAYOUTWI_H
	#include <owl\layoutwi.h>
#endif

#ifndef OWL_STATUSBA_H
	#include <owl\statusba.h>
#endif

#ifndef _editscro_h
	#include <editscro.h>
#endif

#ifndef OWL_OPENSAVE_H
	#include <owl\opensave.h>
#endif

#ifndef OWL_INPUTDIA_H
	#include <owl\inputdia.h>
#endif

#ifndef OWL_VALIDATE_H
	#include <owl\validate.h>
#endif

#ifndef __mainfram_h
	#include "mainfram.h"
#endif

#ifndef __mapdc_h
	#include "mapdc.h"
#endif

#ifndef __infocont_h
	#include "infocont.h"
#endif

#ifndef __episdlg_h
	#include "episdlg.h"
#endif

#ifndef __statdlg_h
	#include "statdlg.h"
#endif

#ifndef __thingdlg_h
	#include "thingdlg.h"
#endif

#ifndef __lineedit_h
	#include "lineedit.h"
#endif

#ifndef __seditdlg_h
	#include "seditdlg.h"
#endif

#ifndef __vertdlg_h
	#include "vertdlg.h"
#endif

#ifndef __inpt1dlg_h
	#include "inpt1dlg.h"
#endif

#ifndef __inpt2dlg_h
	#include "inpt2dlg.h"
#endif

#ifndef __prefdlg_h
	#include "prefdlg.h"
#endif

#ifndef __levels_h
	#include "levels.h"
#endif

#ifndef __names_h
	#include "names.h"	// GetObjectTypeName, ...
#endif

#ifndef __windeapp_h
	#include "windeapp.h"	// AppHelp function, ...
#endif

#ifndef __undo_h
	#include "undo.h"		// Undo/Redo
#endif

#include "bsp.h"

#include "editmenu.rh"

// HelpContext items
#include "windeuhl.h"

// Hightlight color
#define HL_COLOR	YELLOW

//
// FastScroll on/off global var (to keep between calls)
//
static BOOL SlowScroll = FALSE;

static const char* _apszDirection[] =
{
	"N", "NE", "E", "SE", "S", "SW", "W", "NW"
};

static const char* _apszSpeed[] =
{
	"none", "slow", "med", "fast"
};

//
// Information windows position
//
#define INFOWIN_HORIZ_DOWNLEFT	0
#define INFOWIN_HORIZ_DOWNRIGHT	1
#define INFOWIN_HORIZ_UPLEFT	2
#define INFOWIN_HORIZ_UPRIGHT	3

#define INFOWIN_VERT_DOWNLEFT	4
#define INFOWIN_VERT_DOWNRIGHT	5
#define INFOWIN_VERT_UPLEFT		6
#define INFOWIN_VERT_UPRIGHT	7
static int InfoWinPos = 0;

//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TEditorClient, TWindow)
//{{TEditorClientRSP_TBL_BEGIN}}
	EV_WM_SIZE,
	EV_WM_CHAR,
	EV_WM_KEYDOWN,
	EV_WM_MOUSEMOVE,
	EV_WM_LBUTTONDOWN,
	EV_WM_LBUTTONUP,
	EV_WM_LBUTTONDBLCLK,
	EV_WM_RBUTTONDOWN,
	EV_COMMAND(CM_FILE_SAVEAS, CmFileSaveAs),
	EV_COMMAND(CM_FILE_SAVE, CmFileSave),
	EV_COMMAND(CM_FILE_QUITEDIT, CmFileQuit),
	EV_COMMAND(CM_SEARCH_PREVOBJ, CmSearchPrev),
	EV_COMMAND(CM_SEARCH_NEXTOBJ, CmSearchNext),
	EV_COMMAND(CM_SEARCH_JUMPOBJ, CmSearchJump),
	EV_COMMAND(CM_OBJECTS_TORCH, CmObjectsTorch),
	EV_COMMAND(CM_OBJECTS_CIRCLE, CmObjectsCircle),
	EV_COMMAND(CM_OBJECTS_RECTANGLE, CmObjectsRectangle),
	EV_COMMAND(CM_OBJECTS_POLYGON, CmObjectsPolygon),
	EV_COMMAND(CM_EDIT_DELETEOBJ, CmEditDelete),
	EV_COMMAND(CM_EDIT_COPYOBJ, CmEditCopy),
	EV_COMMAND(CM_EDIT_ADDOBJ, CmEditAdd),
	EV_COMMAND(CM_EDIT_OBJECT, CmEditObject),
	EV_COMMAND(CM_EDIT_PREFERENCES, CmEditPreferences),
	EV_COMMAND(CM_MODE_THINGS, CmModeThings),
	EV_COMMAND(CM_MODE_LINEDEFS, CmModeLinedefs),
	EV_COMMAND(CM_MODE_VERTEXES, CmModeVertexes),
	EV_COMMAND(CM_MODE_SECTORS, CmModeSectors),
	EV_COMMAND(CM_MODE_NEXT, CmModeNext),
	EV_COMMAND(CM_MODE_PREV, CmModePrev),
	EV_COMMAND(CM_MISCT_FINDFREETAG, CmMiscFindTag),
	EV_COMMAND(CM_MISCV_FINDFREETAG, CmMiscFindTag),
	EV_COMMAND(CM_MISCL_FINDFREETAG, CmMiscFindTag),
	EV_COMMAND(CM_MISCS_FINDFREETAG, CmMiscFindTag),
	EV_COMMAND(CM_MISCT_ROTATESCALE, CmMiscRotateScale),
	EV_COMMAND(CM_MISCL_ROTATESCALE, CmMiscRotateScale),
	EV_COMMAND(CM_MISCV_ROTATESCALE, CmMiscRotateScale),
	EV_COMMAND(CM_MISCS_ROTATESCALE, CmMiscRotateScale),
	EV_COMMAND(CM_MISCL_SPLITADD, CmMiscLDSplitAdd),
	EV_COMMAND(CM_MISCL_SPLITSECT, CmMiscLDSplitSector),
	EV_COMMAND(CM_MISCL_DELETE, CmMiscLDDelete),
	EV_COMMAND(CM_MISCL_FLIP, CmMiscLDFlip),
	EV_COMMAND(CM_MISCL_SWAP, CmMiscLDSwap),
	EV_COMMAND(CM_MISCL_ALIGNY, CmMiscLDAlignY),
	EV_COMMAND(CM_MISCL_AX_SD1_NORMAL, CmAlignXSD1Normal),
	EV_COMMAND(CM_MISCL_AX_SD1_UPPER, CmAlignXSD1Upper),
	EV_COMMAND(CM_MISCL_AX_SD1_LOWER, CmAlignXSD1Lower),
	EV_COMMAND(CM_MISCL_AX_SD2_NORMAL, CmAlignXSD2Normal),
	EV_COMMAND(CM_MISCL_AX_SD2_UPPER, CmAlignXSD2Upper),
	EV_COMMAND(CM_MISCL_AX_SD2_LOWER, CmAlignXSD2Lower),
   EV_COMMAND(CM_MISCL_AX_SD1_CIRCLE_UPPER, CmAlignXCircleSD1Upper),
   EV_COMMAND(CM_MISCL_AX_SD1_CIRCLE_LOWER, CmAlignXCircleSD1Lower),
   EV_COMMAND(CM_MISCL_AX_SD1_CIRCLE_NORMAL, CmAlignXCircleSD1Normal),
   EV_COMMAND(CM_MISCL_AX_SD2_CIRCLE_UPPER, CmAlignXCircleSD2Upper),
   EV_COMMAND(CM_MISCL_AX_SD2_CIRCLE_LOWER, CmAlignXCircleSD2Lower),
   EV_COMMAND(CM_MISCL_AX_SD2_CIRCLE_NORMAL, CmAlignXCircleSD2Normal),
	EV_COMMAND(CM_MISCV_DELETE, CmMiscVDelete),
	EV_COMMAND(CM_MISCV_MERGE, CmMiscVMerge),
	EV_COMMAND(CM_MISCV_ADD, CmMiscVAddLineDef),
	EV_COMMAND(CM_MISCS_MAKEDOOR, CmMiscSMakeDoor),
	EV_COMMAND(CM_MISCS_MAKELIFT, CmMiscSMakeLift),
	EV_COMMAND(CM_MISCS_DISTRIBFLOOR, CmMiscSDitribFloor),
	EV_COMMAND(CM_MISCS_DISTRIBCEILING, CmMiscSDitribCeiling),
	EV_COMMAND(CM_CHECK_TEXTURES, CmCheckTextures),
	EV_COMMAND(CM_CHECK_SECTORS, CmCheckSectors),
	EV_COMMAND(CM_CHECK_NUMBER, CmCheckNumbers),
	EV_COMMAND(CM_CHECK_TEXTURES_NAMES, CmCheckNames),
	EV_COMMAND(CM_CHECK_CROSSREF, CmCheckCrossRefs),
	EV_COMMAND(CM_WINDOW_ZOOMIN, CmZoomIn),
	EV_COMMAND(CM_WINDOW_ZOOMOUT, CmZoomOut),
	EV_COMMAND(CM_WINDOW_AUTOLAYOUT, CmAutoLayout),
	EV_COMMAND(CM_WINDOW_LAYOUT, CmLayout),
	EV_COMMAND(CM_WINDOW_SNAPTOGRID, CmSnapToGrid),
	EV_COMMAND(CM_WINDOW_GRIDNEXT, CmWindowGridNext),
	EV_COMMAND(CM_WINDOW_GRIDPREV, CmWindowGridPrev),
	EV_COMMAND(CM_WINDOW_SHOWGRID, CmShowGrid),
	EV_COMMAND(CM_WINDOW_INFOWIN, CmInfoWin),
	EV_COMMAND(CM_WINDOW_INFOBAR, CmWindowInfoBar),
	EV_COMMAND(CM_WINDOW_TOOLBAR, CmWindowToolBar),
	EV_COMMAND(CM_WINDOW_CENTER_MAP, CmCenterMap),
	EV_COMMAND(CM_HELP_LEVEL_EDITOR, CmHelpLevelEditor),
	EV_COMMAND(CM_HELP_KEYBOARD, CmHelpKeyboard),
	EV_COMMAND(CM_HELP_MOUSE, CmHelpMouse),
	EV_COMMAND(CM_WINDOW_SLOW_SCROLL, CmSlowScroll),
	EV_COMMAND(CM_EDIT_UNDO, CmUndo),
	EV_COMMAND(CM_EDIT_REDO, CmRedo),
	EV_COMMAND(CM_INFOWIN_HORIZ_DOWNLEFT, CmHorizDownLeft),
	EV_COMMAND(CM_INFOWIN_HORIZ_DOWNRIGHT, CmHorizDownRight),
	EV_COMMAND(CM_INFOWIN_HORIZ_UPLEFT, CmHorizUpLeft),
	EV_COMMAND(CM_INFOWIN_HORIZ_UPRIGHT, CmHorizUpRight),
	EV_COMMAND(CM_INFOWIN_VERT_DOWNLEFT, CmVertDownLeft),
	EV_COMMAND(CM_INFOWIN_VERT_DOWNRIGHT, CmVertDownRight),
	EV_COMMAND(CM_INFOWIN_VERT_UPLEFT, CmVertUpLeft),
	EV_COMMAND(CM_INFOWIN_VERT_UPRIGHT, CmVertUpRight),
	EV_COMMAND_ENABLE(CM_EDIT_DELETEOBJ, CmDeleteEnable),
	EV_COMMAND_ENABLE(CM_EDIT_COPYOBJ, CmCopyEnable),
	EV_COMMAND_ENABLE(CM_EDIT_UNDO, CmUndoEnable),
	EV_COMMAND_ENABLE(CM_EDIT_REDO, CmRedoEnable),
	EV_COMMAND_ENABLE(CM_WINDOW_LAYOUT, CmLayoutEnable),
	EV_COMMAND_ENABLE(CM_WINDOW_AUTOLAYOUT, CmLayoutEnable),
	EV_COMMAND_ENABLE(CM_INFOWIN_HORIZ_DOWNLEFT, CmLayoutEnable),
	EV_COMMAND_ENABLE(CM_INFOWIN_HORIZ_DOWNRIGHT, CmLayoutEnable),
	EV_COMMAND_ENABLE(CM_INFOWIN_HORIZ_UPLEFT, CmLayoutEnable),
	EV_COMMAND_ENABLE(CM_INFOWIN_HORIZ_UPRIGHT, CmLayoutEnable),
	EV_COMMAND_ENABLE(CM_INFOWIN_VERT_DOWNLEFT, CmLayoutEnable),
	EV_COMMAND_ENABLE(CM_INFOWIN_VERT_DOWNRIGHT, CmLayoutEnable),
	EV_COMMAND_ENABLE(CM_INFOWIN_VERT_UPLEFT, CmLayoutEnable),
	EV_COMMAND_ENABLE(CM_INFOWIN_VERT_UPRIGHT, CmLayoutEnable),
//{{TEditorClientRSP_TBL_END}}
	EV_COMMAND(CM_MISC_MENU, CmMiscMenu),
	EV_COMMAND(CM_INSERT_MENU, CmInsertMenu),
	EV_COMMAND(CM_CHECK_MENU, CmCheckMenu),
END_RESPONSE_TABLE;


//{{TEditorClient Implementation}}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//  Constructor
//
TEditorClient::TEditorClient (TWindow* parent, char *_levelName,
							  BOOL newLevel, const char* title,
							  TModule* module):
	TLayoutWindow(parent, title, module)
{
	// Intialize level member
	strcpy (LevelName, _levelName);

	LogMessage (": Editing %s...\n", LevelName);

	// Common file file flags and filters for Open/Save As dialogs.
	// Filename and directory are computed in the member functions CmFileOpen,
	// and CmFileSaveAs.
	FileData.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	FileData.SetFilter("Room Files (*.ROO)|*.ROO|All Files (*.*)|*.*|");

	//
	// ObjectWindows specific initializations
	//
	AutoLayout = TRUE ;

	// Override the default window style for TWindow.
	Attr.Style |= WS_CLIPCHILDREN | WS_HSCROLL |
				  WS_VISIBLE | WS_VSCROLL;

	Scroller = new TEditScroller (this, 200, 200, MAP_X_SIZE, MAP_Y_SIZE) ;

	// Intialize info windows
	pThingInfo = 0;
	pVertexInfo = 0;
	pLineDefInfo = 0;
	pSideDef1Info = 0;
	pSideDef2Info = 0;
	pSectorInfo = 0;

	pThingInfo = new TInfoControl (this, 230, 5, "Thing");
	pLineDefInfo = new TInfoControl (this, 231, 7, "LineDef");
	pSideDef1Info = new TInfoControl (this, 232, 7, "First SideDef");
	pSideDef2Info = new TInfoControl (this, 233, 7, "Second SideDef");
	pVertexInfo = new TInfoControl (this, 234, 2, "Vertex");
	pSectorInfo = new TInfoControl (this, 235, 9, "Sector");


	// Retrieve pointer to status bar of MainFrame
	TMainFrame *mainFrame =
		TYPESAFE_DOWNCAST (GetApplication()->GetMainWindow(), TMainFrame);

	pStatusBar = mainFrame->GetStatusBar() ;

	//
	// Level data initializations
	//

	// Read editor needed data
//	ReadWTextureInfo();
//	ReadFTextureInfo();
	InitUndo();		// Init UNDO/REDO feature

	// If newLevel, delete this level data
	if ( newLevel )
	{
		ForgetLevelData();      // Cleanup level data
		MapMinX = -4000;
		MapMinY = -4000;
		MapMaxX = 4000;
		MapMaxY = 4000;
		// XXX set temporary name ARK
		strcpy(LevelName, "temp.roo");
	}
	else ReadLevelData (LevelName);

	//
	// Init. Editor data member
	//
	EditMode = OBJ_THINGS;

	CurObject = -1;

	DragObject = FALSE;
	DragMoved = FALSE;

	InsertingObject = FALSE;

	// These 3 vars are now configuration variables.
	// GridScale = 8;
	// GridShown = FALSE;
	// SnapToGrid = FALSE;

	Selected = NULL;
	MovingLineDefs = NULL;

	StretchSelBox = FALSE;
	StretchMoved = FALSE;
	SelBoxX = 0;
	SelBoxY = 0;

	ShowRulers = FALSE;

	InfoWinShown = TRUE;

	// Init global data
	ScrMaxX = Attr.W ;
	ScrMaxY = Attr.H ;
	ScrCenterX = ScrMaxX / 2 ;
	ScrCenterY = ScrMaxY / 2 ;

	MadeChanges = FALSE;
	MadeMapChanges = FALSE;
	ScaleMax = 32.0;
	ScaleMin = 1.0f / 20.0f;
	if (InitialScale < 1)
		InitialScale = 1;
	else if (InitialScale > 20)
		InitialScale = 20;
	SetScale ((float) (1.0 / InitialScale));
	CenterMapAroundCoords( (MapMinX + MapMaxX) / 2, (MapMinY + MapMaxY) / 2);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
TEditorClient::~TEditorClient ()
{
	// Destroy window
	Destroy();

	// Destroy editor data
	CleanupUndo();		// cleanup UNDO/REDO structures
	ForgetSelection (&Selected);
	ForgetSelection (&MovingLineDefs);
	ForgetLevelData ();
	Level = NULL;

//	ForgetWTextureInfo();
//	ForgetFTextureInfo();

	LogMessage (": Finished editing %s...\n", LevelName);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
char* TEditorClient::GetClassName ()
{
	return "WinDEUEditor";
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::GetWindowClass (WNDCLASS& wndClass)
{
	TLayoutWindow::GetWindowClass(wndClass);

	// Set background color to black
	wndClass.hbrBackground = (HBRUSH)::GetStockObject (BLACK_BRUSH);
	// wndClass.style |= CS_BYTEALIGNCLIENT;
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::SetupWindow ()
{
	TWindow::SetupWindow();

	// Setup menu and show info windows according to EditMode
	LayoutInfoWindows();
	SetupMode();
	DrawStatusBar();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
// Layout all information windows
//
void TEditorClient::LayoutInfoWindows ()
{
	TLayoutMetrics metrics;

	switch ( InfoWinPos )
	{
		case INFOWIN_HORIZ_DOWNLEFT:
			// pThingInfo
			metrics.X.Set(lmLeft, lmRightOf, lmParent, lmLeft, -1);
			metrics.Y.Set(lmBottom, lmAbove, lmParent, lmBottom, -1);
			SetChildLayoutMetrics(*pThingInfo, metrics);
			// pVertexInfo
			SetChildLayoutMetrics(*pVertexInfo, metrics);
			// pSectorInfo
			SetChildLayoutMetrics(*pSectorInfo, metrics);
			// pLineDefInfo
			SetChildLayoutMetrics(*pLineDefInfo, metrics);
			// pSideDef1Info
			metrics.X.Set(lmLeft, lmRightOf, pLineDefInfo, lmRight, -1);
			SetChildLayoutMetrics(*pSideDef1Info, metrics);
			// pSideDef2Info
			metrics.X.Set(lmLeft, lmRightOf, pSideDef1Info, lmRight, -1);
			SetChildLayoutMetrics(*pSideDef2Info, metrics);
			break;

		case INFOWIN_HORIZ_DOWNRIGHT:
			// pThingInfo
			metrics.X.Set(lmRight, lmLeftOf, lmParent, lmRight, -1);
			metrics.Y.Set(lmBottom, lmAbove, lmParent, lmBottom, -1);
			SetChildLayoutMetrics(*pThingInfo, metrics);
			// pVertexInfo
			SetChildLayoutMetrics(*pVertexInfo, metrics);
			// pSectorInfo
			SetChildLayoutMetrics(*pSectorInfo, metrics);
			// pSideDef2Info
			SetChildLayoutMetrics(*pSideDef2Info, metrics);
			// pSideDef1Info
			metrics.X.Set(lmRight, lmLeftOf, pSideDef2Info, lmLeft, -1);
			SetChildLayoutMetrics(*pSideDef1Info, metrics);
			// pLineDefInfo
			metrics.X.Set(lmRight, lmLeftOf, pSideDef1Info, lmLeft, -1);
			SetChildLayoutMetrics(*pLineDefInfo, metrics);
			break;

		case INFOWIN_HORIZ_UPLEFT:
			// pThingInfo
			metrics.X.Set(lmLeft, lmRightOf, lmParent, lmLeft, -1);
			metrics.Y.Set(lmTop, lmBelow, lmParent, lmTop, -1);
			SetChildLayoutMetrics(*pThingInfo, metrics);
			// pVertexInfo
			SetChildLayoutMetrics(*pVertexInfo, metrics);
			// pSectorInfo
			SetChildLayoutMetrics(*pSectorInfo, metrics);
			// pLineDefInfo
			SetChildLayoutMetrics(*pLineDefInfo, metrics);
			// pSideDef1Info
			metrics.X.Set(lmLeft, lmRightOf, pLineDefInfo, lmRight, -1);
			// metrics.Y.Set(lmBottom, lmAbove, lmParent, lmBottom, 0);
			SetChildLayoutMetrics(*pSideDef1Info, metrics);
			// pSideDef2Info
			metrics.X.Set(lmLeft, lmRightOf, pSideDef1Info, lmRight, -1);
			// metrics.Y.Set(lmBottom, lmAbove, lmParent, lmBottom, 0);
			SetChildLayoutMetrics(*pSideDef2Info, metrics);
			break;

		case INFOWIN_HORIZ_UPRIGHT:
			// pThingInfo
			metrics.X.Set(lmRight, lmLeftOf, lmParent, lmRight, -1);
			metrics.Y.Set(lmTop, lmBelow, lmParent, lmTop, -1);
			SetChildLayoutMetrics(*pThingInfo, metrics);
			// pVertexInfo
			SetChildLayoutMetrics(*pVertexInfo, metrics);
			// pSectorInfo
			SetChildLayoutMetrics(*pSectorInfo, metrics);
			// pSideDef2Info
			SetChildLayoutMetrics(*pSideDef2Info, metrics);
			// pSideDef1Info
			metrics.X.Set(lmRight, lmLeftOf, pSideDef2Info, lmLeft, -1);
			SetChildLayoutMetrics(*pSideDef1Info, metrics);
			// pLineDefInfo
			metrics.X.Set(lmRight, lmLeftOf, pSideDef1Info, lmLeft, -1);
			SetChildLayoutMetrics(*pLineDefInfo, metrics);
			break;

		case INFOWIN_VERT_DOWNLEFT:
			// pThingInfo
			metrics.X.Set(lmLeft, lmRightOf, lmParent, lmLeft, -1);
			metrics.Y.Set(lmBottom, lmAbove, lmParent, lmBottom, -1);
			SetChildLayoutMetrics(*pThingInfo, metrics);
			// pVertexInfo
			SetChildLayoutMetrics(*pVertexInfo, metrics);
			// pSectorInfo
			SetChildLayoutMetrics(*pSectorInfo, metrics);
			// pSideDef2Info
			SetChildLayoutMetrics(*pSideDef2Info, metrics);
			// pSideDef1Info
			metrics.Y.Set(lmBottom, lmAbove, pSideDef2Info, lmTop, -1);
			SetChildLayoutMetrics(*pSideDef1Info, metrics);
			// pLineDefInfo
			metrics.Y.Set(lmBottom, lmAbove, pSideDef1Info, lmTop, -1);
			SetChildLayoutMetrics(*pLineDefInfo, metrics);
			break;

		case INFOWIN_VERT_DOWNRIGHT:
			// pThingInfo
			metrics.X.Set(lmRight, lmLeftOf, lmParent, lmRight, -1);
			metrics.Y.Set(lmBottom, lmAbove, lmParent, lmBottom, -1);
			SetChildLayoutMetrics(*pThingInfo, metrics);
			// pVertexInfo
			SetChildLayoutMetrics(*pVertexInfo, metrics);
			// pSectorInfo
			SetChildLayoutMetrics(*pSectorInfo, metrics);
			// pSideDef2Info
			SetChildLayoutMetrics(*pSideDef2Info, metrics);
			// pSideDef1Info
			metrics.Y.Set(lmBottom, lmAbove, pSideDef2Info, lmTop, -1);
			SetChildLayoutMetrics(*pSideDef1Info, metrics);
			// pLineDefInfo
			metrics.Y.Set(lmBottom, lmAbove, pSideDef1Info, lmTop, -1);
			SetChildLayoutMetrics(*pLineDefInfo, metrics);
			break;

		case INFOWIN_VERT_UPLEFT:
			// pThingInfo
			metrics.X.Set(lmLeft, lmRightOf, lmParent, lmLeft, -1);
			metrics.Y.Set(lmTop, lmBelow, lmParent, lmTop, -1);
			SetChildLayoutMetrics(*pThingInfo, metrics);
			// pVertexInfo
			SetChildLayoutMetrics(*pVertexInfo, metrics);
			// pSectorInfo
			SetChildLayoutMetrics(*pSectorInfo, metrics);
			// pLineDefInfo
			SetChildLayoutMetrics(*pLineDefInfo, metrics);
			// pSideDef1Info
			metrics.Y.Set(lmTop, lmBelow, pLineDefInfo, lmBottom, -1);
			SetChildLayoutMetrics(*pSideDef1Info, metrics);
			// pSideDef2Info
			metrics.Y.Set(lmTop, lmBelow, pSideDef1Info, lmBottom, -1);
			SetChildLayoutMetrics(*pSideDef2Info, metrics);
			break;

		case INFOWIN_VERT_UPRIGHT:
			// pThingInfo
			metrics.X.Set(lmRight, lmLeftOf, lmParent, lmRight, -1);
			metrics.Y.Set(lmTop, lmBelow, lmParent, lmTop, -1);
			SetChildLayoutMetrics(*pThingInfo, metrics);
			// pVertexInfo
			SetChildLayoutMetrics(*pVertexInfo, metrics);
			// pSectorInfo
			SetChildLayoutMetrics(*pSectorInfo, metrics);
			// pLineDefInfo
			SetChildLayoutMetrics(*pLineDefInfo, metrics);
			// pSideDef1Info
			metrics.Y.Set(lmTop, lmBelow, pLineDefInfo, lmBottom, -1);
			SetChildLayoutMetrics(*pSideDef1Info, metrics);
			// pSideDef2Info
			metrics.Y.Set(lmTop, lmBelow, pSideDef1Info, lmBottom, -1);
			SetChildLayoutMetrics(*pSideDef2Info, metrics);
			break;

	}
	Layout();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CloseWindow (int retVal)
{
	TLayoutWindow::CloseWindow(retVal);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::Destroy (int retVal)
{
	TLayoutWindow::Destroy(retVal);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
// According to edit mode : Setup menu, info windows
//
void TEditorClient::SetupMode()
{
	SetupMenu();
	SetupInfoWindows();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::SetupMenu()
{
	TMainFrame *mainFrame =
		TYPESAFE_DOWNCAST (GetApplication()->GetMainWindow(), TMainFrame);
	TMenu menu (mainFrame->GetMenu());
	HMENU newMiscMenu;
	int newMiscMenuID;
	char *newMiscMenuName;

	// Uncheck and check appropriate mode menu item
	menu.CheckMenuItem (CM_MODE_THINGS, MF_BYCOMMAND |
		(EditMode == OBJ_THINGS ? MF_CHECKED : MF_UNCHECKED) );

	menu.CheckMenuItem (CM_MODE_LINEDEFS, MF_BYCOMMAND |
		(EditMode == OBJ_LINEDEFS ? MF_CHECKED : MF_UNCHECKED) );

	menu.CheckMenuItem (CM_MODE_VERTEXES, MF_BYCOMMAND |
		(EditMode == OBJ_VERTEXES ? MF_CHECKED : MF_UNCHECKED) );

	menu.CheckMenuItem (CM_MODE_SECTORS, MF_BYCOMMAND |
		(EditMode == OBJ_SECTORS ? MF_CHECKED : MF_UNCHECKED) );

	switch ( EditMode )
	{
		case OBJ_THINGS:
			newMiscMenuID = MENU_MISC_THINGS;
			newMiscMenuName = "&Things";
			break;
		case OBJ_LINEDEFS:
			newMiscMenuID = MENU_MISC_LINEDEFS;
			newMiscMenuName = "&LineDefs";
			break;
		case OBJ_VERTEXES:
			newMiscMenuID = MENU_MISC_VERTEXES;
			newMiscMenuName = "&Vertices";
			break;
		case OBJ_SECTORS:
			newMiscMenuID = MENU_MISC_SECTORS;
			newMiscMenuName = "Sec&tors";
			break;

		default:
			assert (FALSE);
	}
	newMiscMenu = GetApplication()->LoadMenu (newMiscMenuID);
	menu.ModifyMenu (3, MF_BYPOSITION | MF_POPUP,
							 (UINT)newMiscMenu, newMiscMenuName);

	// Draw the check for the Automatic Layout menu item
	menu.CheckMenuItem (CM_WINDOW_AUTOLAYOUT, MF_BYCOMMAND |
						(AutoLayout ? MF_CHECKED : MF_UNCHECKED) );

	// Draw the check for the 'Snap to Grid' menu item
	menu.CheckMenuItem (CM_WINDOW_SNAPTOGRID, MF_BYCOMMAND |
						(SnapToGrid ? MF_CHECKED : MF_UNCHECKED) );

	// Draw the check for the 'Show Grid' menu item
	menu.CheckMenuItem (CM_WINDOW_SHOWGRID, MF_BYCOMMAND |
						(GridShown ? MF_CHECKED : MF_UNCHECKED) );

	// Draw the check for the 'Slow scroll' menu item
	menu.CheckMenuItem (CM_WINDOW_SLOW_SCROLL, MF_BYCOMMAND |
						(SlowScroll ? MF_CHECKED : MF_UNCHECKED) );

	// Draw the check for the 'Show info bar' menu item
	menu.CheckMenuItem (CM_WINDOW_INFOBAR, MF_BYCOMMAND |
						(mainFrame->statusBarOn ? MF_CHECKED : MF_UNCHECKED) );

	// Draw the check for the 'Show Tool bar' menu item
	menu.CheckMenuItem (CM_WINDOW_TOOLBAR, MF_BYCOMMAND |
						(mainFrame->controlBarOn ? MF_CHECKED : MF_UNCHECKED));

	// Draw the check for the info win position menu items
	menu.CheckMenuItem (CM_INFOWIN_HORIZ_DOWNLEFT, MF_BYCOMMAND |
						(InfoWinPos == INFOWIN_HORIZ_DOWNLEFT ? MF_CHECKED : MF_UNCHECKED) );
	menu.CheckMenuItem (CM_INFOWIN_HORIZ_DOWNRIGHT, MF_BYCOMMAND |
						(InfoWinPos == INFOWIN_HORIZ_DOWNRIGHT ? MF_CHECKED : MF_UNCHECKED) );
	menu.CheckMenuItem (CM_INFOWIN_HORIZ_UPLEFT, MF_BYCOMMAND |
						(InfoWinPos == INFOWIN_HORIZ_UPLEFT ? MF_CHECKED : MF_UNCHECKED) );
	menu.CheckMenuItem (CM_INFOWIN_HORIZ_UPRIGHT, MF_BYCOMMAND |
						(InfoWinPos == INFOWIN_HORIZ_UPRIGHT ? MF_CHECKED : MF_UNCHECKED) );

	menu.CheckMenuItem (CM_INFOWIN_VERT_DOWNLEFT, MF_BYCOMMAND |
						(InfoWinPos == INFOWIN_VERT_DOWNLEFT ? MF_CHECKED : MF_UNCHECKED) );
	menu.CheckMenuItem (CM_INFOWIN_VERT_DOWNRIGHT, MF_BYCOMMAND |
						(InfoWinPos == INFOWIN_VERT_DOWNRIGHT ? MF_CHECKED : MF_UNCHECKED) );
	menu.CheckMenuItem (CM_INFOWIN_VERT_UPLEFT, MF_BYCOMMAND |
						(InfoWinPos == INFOWIN_VERT_UPLEFT ? MF_CHECKED : MF_UNCHECKED) );
	menu.CheckMenuItem (CM_INFOWIN_VERT_UPRIGHT, MF_BYCOMMAND |
						(InfoWinPos == INFOWIN_VERT_UPRIGHT ? MF_CHECKED : MF_UNCHECKED) );

	// Draw the check for the 'Info Windows' menu item
	menu.CheckMenuItem (CM_WINDOW_INFOWIN, MF_BYCOMMAND |
						(InfoWinShown ? MF_CHECKED : MF_UNCHECKED) );

	// Redraw new menu bar
	GetApplication()->GetMainWindow()->DrawMenuBar();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::SetupInfoWindows()
{
	assert (pThingInfo != 0);
	assert (pLineDefInfo != 0);
	assert (pSideDef1Info != 0);
	assert (pSideDef2Info != 0);
	assert (pVertexInfo != 0);
	assert (pSectorInfo != 0);

	// Hides the information windows, except the ones for the current mode
	pThingInfo->ShowWindow ( InfoWinShown && EditMode == OBJ_THINGS ?
							 SW_SHOW : SW_HIDE);
	pLineDefInfo->ShowWindow ( InfoWinShown && EditMode == OBJ_LINEDEFS ?
							   SW_SHOW : SW_HIDE);
	pSideDef1Info->ShowWindow ( InfoWinShown && EditMode == OBJ_LINEDEFS ?
								SW_SHOW : SW_HIDE);
	pSideDef2Info->ShowWindow ( InfoWinShown && EditMode == OBJ_LINEDEFS ?
								SW_SHOW : SW_HIDE);
	pVertexInfo->ShowWindow ( InfoWinShown && EditMode == OBJ_VERTEXES ?
							  SW_SHOW : SW_HIDE);
	pSectorInfo->ShowWindow ( InfoWinShown && EditMode == OBJ_SECTORS ?
							  SW_SHOW : SW_HIDE);

	// Display object info in window
	if (InfoWinShown)
		DisplayObjectInfo (EditMode, CurObject);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//  Setup selected list as below:
//      - if CurObject <= 0, return
//      - else select CurObject if selection list = NULL
//
void TEditorClient::SetupSelection (BOOL SaveSel)
{
	static int ForgetCurObject = FALSE;	// Did we select CurObject?
	static int OldCurObject    = -1;    // If Yes, CurOjbect at that time
	static int OldEditMode     = -1;    //         EditMode at that time

	// If not saving, remember selection
	if ( SaveSel == FALSE )
	{
		if ( ForgetCurObject == TRUE )
		{
			// Check the past is coherent
			assert (Selected != NULL);
			assert (OldCurObject != -1);
			assert (OldEditMode == EditMode);
			assert (Selected->objnum == OldCurObject);

			UnSelectObject (&Selected, OldCurObject);
		}
	}

	// Clear the past ...
	ForgetCurObject = FALSE;
	OldCurObject    = -1;
	OldEditMode     = -1;

	// Setup current selection
	if ( SaveSel )
	{
		// If no current object, don't anything to the selection
		if ( CurObject < 0 )
			return;

		if ( Selected == NULL )
		{
			SelectObject (&Selected, CurObject);
			ForgetCurObject = TRUE;
			OldCurObject    = CurObject;
			OldEditMode     = EditMode;
		}
		else if ( IsSelected (Selected, CurObject) )
		{
			// Move CurObject to beginning of selection list
			UnSelectObject (&Selected, CurObject);
			SelectObject (&Selected, CurObject);
		}
	}
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
// According to edit mode : Setup menu, info windows
//
void TEditorClient::ChangeMode (int NewMode)
{
	SelPtr NewSel;
	int PrevMode = EditMode;
	EditMode = NewMode;

	// If same mode, return directly
	if ( PrevMode == EditMode )
		return ;

	// special cases for the selection list...
	if (Selected)
	{
		// select all LineDefs bound to the selected Sectors
		if (PrevMode == OBJ_SECTORS && EditMode == OBJ_LINEDEFS)
		{
			SHORT l, sd;

			NewSel = NULL;
			for (l = 0; l < NumLineDefs; l++)
			{
				sd = LineDefs [l].sidedef1;
				assert (sd == -1  ||  (sd >= 0 && sd < NumSideDefs));
				if (sd >= 0  &&  IsSelected (Selected, SideDefs[sd].sector))
					SelectObject (&NewSel, l);
				else
				{
					sd = LineDefs [l].sidedef2;
					assert (sd == -1  ||  (sd >= 0 && sd < NumSideDefs));
					if (sd >= 0 && IsSelected (Selected, SideDefs[sd].sector))
						SelectObject (&NewSel, l);
				}
			}
			ForgetSelection (&Selected);
			Selected = NewSel;
		}

		// select all Vertices bound to the selected LineDefs
		else if (PrevMode == OBJ_LINEDEFS && EditMode == OBJ_VERTEXES)
		{
			NewSel = NULL;
			while (Selected)
			{
				SHORT n = Selected->objnum;
				assert (n >= 0 && n < NumLineDefs);
				SHORT start = LineDefs[n].start;
				SHORT end   = LineDefs[n].end;
				if ( ! IsSelected (NewSel, start) )
					SelectObject (&NewSel, start);

				if ( ! IsSelected (NewSel, end))
					SelectObject (&NewSel, end);

				UnSelectObject (&Selected, n);
			}
			Selected = NewSel;
		}

		// select all Sectors that have their LineDefs selected
		else if (PrevMode == OBJ_LINEDEFS && EditMode == OBJ_SECTORS)
		{
			SHORT l, sd;

			NewSel = NULL;
			/* select all Sectors... */
			for (l = 0; l < NumSectors; l++)
				SelectObject (&NewSel, l);

			/* ... then unselect those that should not be in the list */
			for (l = 0; l < NumLineDefs; l++)
			{
				if ( ! IsSelected (Selected, l) )
				{
					sd = LineDefs[ l].sidedef1;
					assert (sd == -1  ||  (sd >= 0 && sd < NumSideDefs));
					if (sd >= 0)
						UnSelectObject (&NewSel, SideDefs[sd].sector);

					sd = LineDefs[ l].sidedef2;
					assert (sd == -1  ||  (sd >= 0 && sd < NumSideDefs));
					if (sd >= 0)
						UnSelectObject (&NewSel, SideDefs[sd].sector);
				}
			}
			ForgetSelection (&Selected);
			Selected = NewSel;
		}

		// select all LineDefs that have both ends selected
		else if (PrevMode == OBJ_VERTEXES && EditMode == OBJ_LINEDEFS)
		{
			SHORT l;

			NewSel = NULL;
			for (l = 0; l < NumLineDefs; l++)
			{
				if (IsSelected (Selected, LineDefs[l].start) &&
					IsSelected (Selected, LineDefs[l].end)    )
				{
					SelectObject (&NewSel, l);
				}
			}
			ForgetSelection (&Selected);
			Selected = NewSel;
		}

		// unselect all
		else
			ForgetSelection (&Selected);
	}

	// If couldn't select something, select object #0
	if (GetMaxObjectNum (EditMode) >= 0 && Select0 == TRUE)
		CurObject = 0;
	else
		CurObject = -1;

	// Setup menu, info windows and map display
	SetupMode();
	RefreshWindows();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::Paint (TDC& dc, bool erase, TRect& rect)
{
	// Save GDI object and set WAIT cursor
	// SELECT_WAIT_CURSOR();

	TLayoutWindow::Paint(dc, erase, rect);

	// Draw mouse coord.
	DrawMouseCoord (dc);

	// Set new visible map limits
	VisMapMinX = MAPX(rect.left);
	VisMapMaxX = MAPX(rect.right);
	VisMapMinY = MAPY(rect.bottom);	// Vertical axe is reversed!
	VisMapMaxY = MAPY(rect.top);	// Vertical axe is reversed!

	// Draw map
	TMapDC ndc (this, dc);
	ndc.DrawMap (EditMode, GridScale, GridShown) ;

	if (NumThings >= 2)
	{
	int left = Things[0].xpos;
	int top = Things[0].ypos;
	int right = Things[1].xpos;
	int bottom = Things[1].ypos;
#ifndef WINDOWS_SCALING
	left = SCREENX(left);
	top = SCREENY(top);
	right = SCREENX(right);
	bottom = SCREENY(bottom);
#endif
	int oldROP2 = ndc.SetROP2 (R2_XORPEN);
	ndc.SetPenColor16 (GetKodObjectColor (Things[0].type));
	ndc.Rectangle(left, top, right+1, bottom+1);
	ndc.SetROP2 (oldROP2);
	}

	// Draw selected objects if user isn't dragging
	if ( ! DragObject )
	{
		// Draw selection
		if ( Selected != NULL)
			HighlightSelection (ndc, EditMode, Selected);

		// Draw current selected object
		if (CurObject >= 0)
			HighlightObject(ndc, EditMode, CurObject, HL_COLOR);
	}

	// Draw mouse coord.
	DrawMouseCoord (dc);

	// Restore visible map limits
	VisMapMinX = MAPX(0);
	VisMapMaxX = MAPX(ScrMaxX);
	VisMapMinY = MAPY(ScrMaxY);		// Vertical axe is reversed!
	VisMapMaxY = MAPY(0);			// Vertical axe is reversed!

	// Restore GDI object and cursor
	// UNSELECT_WAIT_CURSOR();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//	Invalidate map, refresh info windows and draw status bar and
void TEditorClient::RefreshWindows ()
{
	DisplayObjectInfo (EditMode, CurObject);
	DrawStatusBar ();
	Invalidate ();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::DrawMouseCoord (TDC &dc)
{
   char msg[80];
   SHORT x, y;
   int row, col, xoffset, yoffset;
   
   x = MAPX(PointerX);
   y = MAPY(PointerY);
   
   if ( SnapToGrid && GridScale != 0 )
   {
      x = (x + GridScale / 2) & ~(GridScale -1);
      y = (y + GridScale / 2) & ~(GridScale -1);
   }
   sprintf (msg, "%+06d, %+06d ", x, y);
   dc.SetBkColor (TColor::Black);
   dc.SetTextColor (TColor::LtGray);
   dc.TextOut (0, 0, msg, -1);
   
   // Display server coordinates
   col = x;
   row = y;
   GetServerCoords(&col, &row, &xoffset, &yoffset);
   sprintf(msg, "row = %03d, col = %03d (%02d %02d) ", row, col, yoffset, xoffset);
   dc.TextOut (0, 20, msg, -1);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::EvSize (UINT sizeType, const TSize& size)
{
	TRect clientRect = GetClientRect ();

	if (AutoLayout)
		// Set ClientSize and calls layout()
		TLayoutWindow::EvSize(sizeType, size);
	else
		ClientSize = TSize(clientRect.Width(), clientRect.Height());

	// Initialize glob. vars. for gfx.cpp
	ScrMaxX = clientRect.Width();
	ScrMaxY = clientRect.Height();
	ScrCenterX = ScrMaxX / 2;
	ScrCenterY = ScrMaxY / 2;

	// Set visible map limits
	VisMapMinX = MAPX(0) ;
	VisMapMaxX = MAPX(ScrMaxX) ;
	// Vertical axe is reversed !!
	VisMapMinY = MAPY(ScrMaxY) ;
	VisMapMaxY = MAPY(0) ;

	// Calculate minimum scale and adjust Scale if needed
	SetScaleMin();

	//DEBUG: Force to repaint window
	Invalidate();
	// The screen limits have changed, so scroll bars range must change too.
	AdjustScroller();

	DrawStatusBar();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//  Mouse MOVE:
//    - Draw mouse pointer coord.
//	  - Either
//	     - Draw dragging objects
//       OR Draw stretch selection box
//       OR Highlight pointed object (and set it to new current object), only
//          if CTRL key not pressed
//
void TEditorClient::EvMouseMove (UINT modKeys, const TPoint& point)
{
	TLayoutWindow::EvMouseMove(modKeys, point);

	if ( PointerX != point.x  ||  PointerY != point.y )
	{
		// Display mouse coord. in MAP coord.
		TClientDC dc (Handle);
		DrawMouseCoord (dc);
	}

   PointerX = (SHORT)point.x;
   PointerY = (SHORT)point.y;

	// If Inserting object, don't do anything
	if ( InsertingObject )
	{
		// Maybe we should draw the object(s) ?
	}
	// If stretching selection box
	else if (StretchSelBox)
	{
		SHORT x = MAPX(PointerX);
		SHORT y = MAPY(PointerY);
		TMapDC dc (this);

		if ( !StretchMoved )
			StretchMoved = TRUE;
		else
			// Delete old selection box
			DrawStretchSelBox (dc);

		// Draw new selection box
		OldSelBoxX = x;
		OldSelBoxY = y;
		DrawStretchSelBox (dc);
	}

	// If dragging object
	else if (DragObject)
	{
		TMapDC dc (this);

		// Delete selected objects
		if ( !DragMoved )
		{
			// The user begin to move objects, so we begin UNDO record.
			StartUndoRecording(AddObjectType("Move"));

			//
			// Delete hightlight objects
			//
			if ( CurObject >= 0 )
				HighlightObject(dc, EditMode, CurObject, HL_COLOR);
			HighlightSelection (dc, EditMode, Selected);

			//
			// Init. dragging starting position
			//
			SHORT x, y;
			SHORT TempGridScale = 0;

			if (EditMode == OBJ_THINGS && CurObject >= 0)
			{
				x = Things[CurObject].xpos;
				y = Things[CurObject].ypos;
			}
			else if (EditMode == OBJ_VERTEXES && CurObject >= 0)
			{
				x = Vertexes[CurObject].x;
				y = Vertexes[CurObject].y;
			}
			else
			{
				x = MAPX (PointerX);
				y = MAPY (PointerY);
				TempGridScale = (SnapToGrid ? GridScale : 0);
			}
			MoveObjectsToCoords (EditMode, NULL, x, y, TempGridScale);

			//
			//  Choose objects to move
			//

			//  Current object must be first in the list
			if ( IsSelected (Selected, CurObject) )
				UnSelectObject (&Selected, CurObject);
			SelectObject (&Selected, CurObject);

			// Select the moving LineDefs
			SelectMovingLineDefs ();

			// Delete objects from their old positions
			DrawMovingObjects (dc, TRUE);
			DragMoved = TRUE;
		}
		else
		{
			// Delete objects from their old positions (XOR)
			DrawMovingObjects (dc);
		}

		// Move selected object to pointer coord.
		BOOL ObjMoved;
		ObjMoved = MoveObjectsToCoords (EditMode, Selected,
										MAPX(PointerX), MAPY(PointerY),
										(SnapToGrid) ? GridScale : 0);
		if ( ObjMoved )
		{
			// Display coordinate of current thing or vertex
			if ( EditMode == OBJ_THINGS || EditMode == OBJ_VERTEXES )
				DisplayObjectInfo (EditMode, CurObject);
		}

		// Draw selected objects
		DrawMovingObjects (dc);
	}

	// Search for a object next to pointer to select
	else if ( !(modKeys & MK_CONTROL) )
	{
		SHORT OldObject = CurObject;

		// check if there is something near the pointer
		CurObject = GetCurObject (EditMode,
								  MAPX( PointerX - 4), MAPY( PointerY - 4),
								  MAPX( PointerX + 4), MAPY( PointerY + 4));
		if ( CurObject < 0 )
			CurObject = OldObject;

		// highlight the current object and display the information box
		if (CurObject >= 0  &&  CurObject != OldObject)
		{
			TMapDC dc (this);

			// Delete highlight of old selected object
			if (OldObject >= 0)
				HighlightObject (dc, EditMode, OldObject, HL_COLOR);

			PlaySound (50, 10);

			// Draw highlight of current selected object
			HighlightObject (dc, EditMode, CurObject, HL_COLOR);

			DisplayObjectInfo (EditMode, CurObject);
		}
	}
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//	Left mouse button (DOWN) is used for
//		- Selection of objects
//      - Drag mode
//      - Stretch selection box.
//
// How this function work ?
//   1. If there is a valid CurObject:
//        If SHIFT key is pressed,
//          selected or unselect CurObject
//        else
//          If CurObject is not currently selected,
//             Unselected everything and select CurObject
//          (else current selection is kept)
//
//   2. If Mouse pointer is EXACTLY on a object,
//         begin dragging mode and set window capture
//      Else
//         begin stretch selection box mode and set window capture
//
// PS: Note that dragging mode and stretch selection box mode
//     won't realy take effect until the mouse is moved, and will
//     stop when Left mouse button is released.
//
void TEditorClient::EvLButtonDown (UINT modKeys, const TPoint& point)
{
   PointerX = (SHORT)point.x;
   PointerY = (SHORT)point.y;

	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	// (Un)select current object if this object is valid
	if (CurObject >= 0)
	{
		TMapDC dc (this);
		if ( modKeys & MK_SHIFT )
		{
			// mark or unmark one object  of the selection
			if (IsSelected (Selected, CurObject) )
				UnSelectObject (&Selected, CurObject);
			else
				SelectObject (&Selected, CurObject);

			// (Un)HightLight (un)selected object
			HighlightObject(dc, EditMode, CurObject, GREEN);
		}
		else
		{
			if ( !IsSelected (Selected, CurObject) )
			{
				// Unselect all object and select the current
				HighlightSelection (dc, EditMode, Selected);
				ForgetSelection (&Selected);

				SelectObject (&Selected, CurObject);
				HighlightSelection (dc, EditMode, Selected);
			}
		}

		if (Selected)
			 PlaySound (440, 10);
	}

	// Check if click on a selected object
	SHORT PointerObject;
	PointerObject = GetCurObject (EditMode,
								  MAPX(PointerX - 4), MAPY(PointerY - 4),
								  MAPX(PointerX + 4), MAPY(PointerY + 4));

	// If the user click exactly on an object, begin 'Drag' mode
	if ( PointerObject >= 0 )
	{
		// Not realy useful, but to be sure
		if ( CurObject != PointerObject )
		{
			TMapDC dc(this);
			HighlightObject (dc, EditMode, CurObject, HL_COLOR);
			CurObject = PointerObject;
			HighlightObject (dc, EditMode, CurObject, HL_COLOR);
			DisplayObjectInfo (EditMode, CurObject);
		}
		assert (PointerObject == CurObject);

		// Set window capture for dragging objects
		DragObject = TRUE;
		DragMoved  = FALSE;
		TLayoutWindow::EvLButtonDown(modKeys, point);
	}
	else
	{
		// Begin 'stretch selection box'
		SelBoxX = MAPX(PointerX);
		SelBoxY = MAPY(PointerY);

		// If SHIFT, additive selection box ON
		StretchSelBox = TRUE;
		AdditiveSelBox = (modKeys & MK_SHIFT) ? TRUE : FALSE;
		StretchMoved = FALSE;

		// Set window capture for stretch selection box
		TLayoutWindow::EvLButtonDown(modKeys, point);
	}
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//	Left mouse button UP:
//		- Stop Drag mode or Stretch selection box mode.
//
void TEditorClient::EvLButtonUp (UINT modKeys, const TPoint& point)
{
   PointerX = (SHORT)point.x;
   PointerY = (SHORT)point.y;

	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	// Default function member
	TLayoutWindow::EvLButtonUp(modKeys, point);

	// Stop stretching selection box
	if ( StretchSelBox )
	{
		if ( StretchMoved )
		{
			SelPtr oldsel;
			TMapDC dc (this);

			// Delete selection box
			DrawStretchSelBox(dc);

			// Delete old selection
			if ( CurObject >= 0 )
				HighlightObject (dc, EditMode, CurObject, HL_COLOR);
			HighlightSelection (dc, EditMode, Selected);

			// Additive selection box or not? */
			if (AdditiveSelBox == FALSE)
				ForgetSelection (&Selected);
			else
				oldsel = Selected;

			//
			// select all objects in the selection box
			//
			Selected = SelectObjectsInBox (EditMode, SelBoxX, SelBoxY,
													 OldSelBoxX, OldSelBoxY);
			// Transfer old selection to new selection
			if (AdditiveSelBox)
			{
				while (oldsel != NULL)
				{
					if ( !IsSelected (Selected, oldsel->objnum) )
						SelectObject (&Selected, oldsel->objnum);
					UnSelectObject (&oldsel, oldsel->objnum);
				}
			}

			// If something seleted, CurObject is first in list
			if (Selected)
			{
				PlaySound (440, 10);
				CurObject = Selected->objnum;

				// Highlight new selection
				HighlightSelection (dc, EditMode, Selected);
				HighlightObject (dc, EditMode, CurObject, HL_COLOR);
			}
			else
				CurObject = -1;

			// New display info of new current object
			DisplayObjectInfo (EditMode, CurObject);
		}

		StretchSelBox = FALSE;
		StretchMoved = FALSE;
	}
	else if (DragObject)
	{
		if ( DragMoved )
		{
			// Automerge vertices
			if (EditMode == OBJ_VERTEXES)
			{
				AutoMergeVertices (&Selected);
			}
			else if (EditMode == OBJ_LINEDEFS)
			{
				SelPtr NewSel, cur;

				NewSel = NULL;
				if (Selected == NULL && CurObject >= 0)
				{
					LineDef *pLineDef = &LineDefs[CurObject];

					SelectObject (&NewSel, pLineDef->start);
					SelectObject (&NewSel, pLineDef->end);
				}
				else
				{
					for (cur = Selected; cur; cur = cur->next)
					{
						LineDef *pLineDef = &LineDefs[cur->objnum];

						if ( !IsSelected (NewSel, pLineDef->start) )
							SelectObject (&NewSel, pLineDef->start);

						if ( !IsSelected (NewSel, pLineDef->end) )
							SelectObject (&NewSel, pLineDef->end);
					}
				}
				AutoMergeVertices (&NewSel);
				ForgetSelection (&NewSel);
			}

			// Forget moving LineDefs
			ForgetSelection (&MovingLineDefs);

			// Redraw map
			RefreshWindows();

			// The user has moved objects, so we stop UNDO record.
			StopUndoRecording();
		}
		DragObject = FALSE;
		DragMoved = FALSE;
	}
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::EvLButtonDblClk (UINT modKeys, const TPoint& point)
{
	PointerX = (SHORT)point.x;
   PointerY = (SHORT)point.y;

	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	TLayoutWindow::EvLButtonDblClk(modKeys, point);

	// Check if double click on current object
	if ( CurObject == GetCurObject (EditMode,
									MAPX(PointerX - 4), MAPY(PointerY - 4),
									MAPX(PointerX + 4), MAPY(PointerY + 4)) )
	{
		EditCurObject ();
	}
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::EvRButtonDown (UINT modKeys, const TPoint& point)
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	TLayoutWindow::EvRButtonDown(modKeys, point);

	// If SHIFT + RIGHT mouse button: center map
	if ( modKeys & MK_SHIFT)
	{
		// Center map around the selected pointer map. coord
		CenterMapAroundCoords (MAPX(point.x), MAPY(point.y));

		Invalidate() ;     // Redraw map
		AdjustScroller();  // Adjust scroller units
		return;
	}

	// If Right mouse button, display popup menu
	SHORT PointerObject;
	PointerObject = GetCurObject (EditMode,
								  MAPX(point.x - 4), MAPY(point.y - 4),
								  MAPX(point.x + 4), MAPY(point.y + 4));

	TResId MenuResID;

	// If not on an object, select general popup menu
	if ( PointerObject < 0 )
	{
		MenuResID = MENU_OBJECTS;
	}
	// If on an object, select sensitive popup menu
	else
	{
		// Not realy useful, but to be sure
		if ( CurObject != PointerObject )
		{
			TRACE ("CurObject != PointerObject: strange happening");
			TMapDC dc(this);
			HighlightObject (dc, EditMode, CurObject, HL_COLOR);
			CurObject = PointerObject;
			HighlightObject (dc, EditMode, CurObject, HL_COLOR);
			DisplayObjectInfo (EditMode, CurObject);
		}
		assert (PointerObject == CurObject);

		switch ( EditMode )
		{
		case OBJ_THINGS:
			MenuResID = MENU_THINGS;
			break;
		case OBJ_LINEDEFS:
			MenuResID = MENU_LINEDEFS;
			break;
		case OBJ_VERTEXES:
			MenuResID = MENU_VERTEXES;
			break;
		case OBJ_SECTORS:
			MenuResID = MENU_SECTORS;
			break;
		default:
			assert (FALSE);
		}
	}


	// The menu was never Destroyed !!!!!! (Bad for User resources !!!)
	// Thanks to Drake O'Brien for signaling the bug.
	// HMENU hMenu = GetApplication()->LoadMenu (MenuResID);
	// TMenu TrackMenu (hMenu) ;
	TMenu TrackMenu (*GetApplication(), MenuResID) ;
	TPopupMenu PopupMenu (TrackMenu.GetSubMenu (0));
	if ( TrackMenu.IsOK() && PopupMenu.IsOK() )
	{
		ClientToScreen ((TPoint&)point);
		PopupMenu.TrackPopupMenu (TPM_LEFTALIGN | TPM_RIGHTBUTTON,
								  point, 0 /* reserved */, *Parent);
	}
	else
		Notify ("Error: Couldn't create popup menu");

	// Destroy menu
	/*
	if ( DestroyMenu (hMenu) == FALSE )
		Notify ("Error: Couldn't destroy popup menu "
				"(bad for Windows User resources");
	*/
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//   Move the map using cursor keys
//
void TEditorClient::EvKeyDown (UINT key, UINT repeatCount, UINT flags)
{
	switch (key)
	{
		case VK_LEFT:
			//                  repeatCount is UNSIGNED, be careful
			Scroller->ScrollBy(-(long)repeatCount, 0);
			break;
		case VK_RIGHT:
			//                  repeatCount is UNSIGNED, be careful
			Scroller->ScrollBy((long)repeatCount, 0);
			break;
		case VK_DOWN:
			//                  repeatCount is UNSIGNED, be careful
			Scroller->ScrollBy(0, (long)repeatCount);
			break;
		case VK_UP:
			//                  repeatCount is UNSIGNED, be careful
			Scroller->ScrollBy(0, -(long)repeatCount);
			break;
		case VK_ESCAPE:
			// Ignore if "insert object" mode
			if ( InsertingObject )
				break;

			// If there's map changes and user don't want to quit, return
			if ( (MadeChanges || MadeMapChanges) &&
				 MessageBox ("You have made changes.\n\n"
							 "Are you sure you want to quit the editor?",
							 "Quit editor",
							 MB_YESNO | MB_DEFBUTTON2) == IDNO )
			{
				break;
			}
			// We cancel all changes made to the map.
			// The SaveChanges function which will be called after
			// the WM_CLOSE msg is received by the MainWindow
			// won't ask the user to save the changes, since we mark
			// here there were no.
			MadeChanges = FALSE;
			MadeMapChanges = FALSE;
			// Send WM_CLOSE message to main frame
			GetApplication()->GetMainWindow()->PostMessage(WM_CLOSE);
			break;
	}

	TLayoutWindow::EvKeyDown(key, repeatCount, flags);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::EvChar (UINT key, UINT repeatCount, UINT flags)
{
	TLayoutWindow::EvChar(key, repeatCount, flags);

	// user wants to set the scale directly
	if ( key == '+' || key == '-' || (key >= '0' && key <= '9') )
	{
		float oldScale = Scale;

		OrigX += (SHORT) ((PointerX - ScrCenterX) * DIV_SCALE);
		OrigY += (SHORT) ((ScrCenterY - PointerY) * DIV_SCALE);
		if ( key == '+' )
			IncScale();
		else if (key == '-' )
			DecScale();
		else if (key == '0')
			SetScale (0.1f);
		else
			SetScale (1.0f / (float)(key - '0'));
		OrigX -= (SHORT) ((PointerX - ScrCenterX) * DIV_SCALE);
		OrigY -= (SHORT) ((ScrCenterY - PointerY) * DIV_SCALE);

		// If scale has really changed
		if ( Scale != oldScale )
		{
			Invalidate ();			// Redraw map
			AdjustScroller ();      // Adjust scroller units
			DrawStatusBar ();		// Display new scale
		}
	}
	// Unselect all objects
	else if ( key == 'c' )
	{
		// Ignore if "insert object" mode
		if ( InsertingObject )
			return;

		HighlightSelection (TMapDC(this), EditMode, Selected);
		ForgetSelection (&Selected);
	}
	// (Un)select current object
	else if ( key == 'm' && CurObject >= 0 )
	{
		// Ignore if "insert object" mode
		if ( InsertingObject )
			return;

		if ( IsSelected (Selected, CurObject) )
			UnSelectObject (&Selected, CurObject);
		else
			SelectObject (&Selected, CurObject);

		HighlightObject (TMapDC(this), EditMode, CurObject, GREEN);
	}
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::AdjustScroller ()
{
	SHORT step = SlowScroll ? 50 : 20;
	SHORT val = (20 * step * (SHORT)MUL_SCALE);

	if (val == 0)
		val = 1;

	SHORT XUnit = MAP_X_SIZE / val;
	SHORT YUnit = MAP_Y_SIZE / val;

	Scroller->SetUnits(XUnit, YUnit);

	//
	// Note: In this routine, we calc. the legal values of OrigX, OrigY,
	//       XRange and YRange. These legal values are calculated so that
	//       it is impossible for the user to go to map coordinates on the
	//       window which are out of the minimum (MAP_MIN_X, MAP_MIN_Y) and
	//       maximum (MAP_MAX_X, MAP_MAX_Y) legal doom coord.
	//       It's evident that these values are dependant from the window size
	//       and the Scale. So, this function must be called after each EvSize
	//       and/or Scale change.
	//		 Furthermore, OrigX and OrigY are bound so that the above condition
	//       is respected.
	//

	// Compute the max and min value for OrigX and OrigY
	SHORT OrigXMin = MAP_MIN_X + (SHORT)(ScrCenterX * DIV_SCALE);
	SHORT OrigXMax = MAP_MAX_X - (SHORT)(ScrCenterX * DIV_SCALE);
	if (OrigXMin > OrigXMax)
	{
		// If we go outside the valid map, center it back over
		// the middle at the starting scale. This used to be
		// checked using *assert*!
		SetScale((float)(1.0 / 20.0));
		CenterMapAroundCoords((MapMinX + MapMaxX) / 2, (MapMinY + MapMaxY) / 2);
	}

	SHORT OrigYMin = MAP_MIN_Y + (SHORT)(ScrCenterY * DIV_SCALE);
	SHORT OrigYMax = MAP_MAX_Y - (SHORT)(ScrCenterY * DIV_SCALE);
	if (OrigYMin > OrigYMax)
	{
		SetScale((float)(1.0 / 20.0));
		CenterMapAroundCoords((MapMinX + MapMaxX) / 2, (MapMinY + MapMaxY) / 2);
	}

	// Check that OrigX and OrigY are in their legal value
	if ( OrigX < OrigXMin )     OrigX = OrigXMin ;
	if ( OrigX > OrigXMax )     OrigX = OrigXMax ;

	if ( OrigY < OrigYMin )     OrigY = OrigYMin ;
	if ( OrigY > OrigYMax )     OrigY = OrigYMax ;

	// Calc scroller range (in X or Y Units)
	SHORT XRange = abs(OrigXMax - OrigXMin) / XUnit;
	SHORT YRange = abs(OrigYMax - OrigYMin) / YUnit;

	// Calc and Set the Scroller position (in XUnit and YUnit !)
	SHORT newXPos = max(0, min((OrigX - OrigXMin) / XUnit, XRange));
	SHORT newYPos = max(0, min((OrigYMax - OrigY) / YUnit, YRange));
	Scroller->XPos = newXPos;
	Scroller->YPos = newYPos; // Vertical axe is reversed !

	// Set the Scroller range (in XUnit and YUnit !)
	Scroller->XRange = XRange;
	Scroller->YRange = YRange;
	// Scroller->SetRange(XRange, YRange);

	// Update scroll bars position
	SetScrollRange(SB_HORZ, 0, XRange, FALSE);
	SetScrollRange(SB_VERT, 0, YRange, FALSE);
	SetScrollPos(SB_HORZ, newXPos, TRUE);
	SetScrollPos(SB_VERT, newYPos, TRUE);
	Scroller->ScrollTo(newXPos, newYPos);
}


//////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//   display the information about one object in the information windows
//
void TEditorClient::DisplayObjectInfo (int objtype, SHORT objnum)
{
	char  texname[MAX_BITMAPNAME + 1];
	SHORT tag, n;
	SHORT sd1, sd2, s1, s2; //, light;

	switch (objtype)
	{
	case OBJ_THINGS:
		assert (pThingInfo != 0);

		pThingInfo->Cleanup();
		pThingInfo->BeginInsert();
		if (objnum < 0)
		{
			pThingInfo->SetStyle(TA_CENTER, DARKGRAY);
			pThingInfo->InsertAt (1, "Use the cursor to");
			pThingInfo->Insert ("select a Thing");
		}
		else
		{
			assert (objnum >= 0 && objnum < NumThings);
			Thing *pThing = &Things[objnum];

			pThingInfo->SetStyle (TA_CENTER, BLACK);
			pThingInfo->Insert ("Selected Thing (#%d)", objnum);
			pThingInfo->SetStyle (TA_LEFT, LIGHTBLUE);
			pThingInfo->Insert ("Coordinates: (%d, %d)", pThing->xpos, pThing->ypos);
      // Commented out 7/25/04 ARK
//			pThingInfo->Insert ("Type:         %s", GetKodObjectName (pThing->type));
			pThingInfo->Insert ("Angle:        %s", GetAngleName (pThing->angle));
			if (pThing->type == kodExit)
				pThingInfo->Insert("Exit to:       %s", GetKodRoomNameByRoomID(pThing->when));
			else if (GetNumKodTypes(pThing->type) > 0)
				pThingInfo->Insert("Subtype:       %s", GetKodTypeName(pThing->type,pThing->when));

		}
		pThingInfo->EndInsert();
		break;

	case OBJ_LINEDEFS:
		assert (pLineDefInfo != 0);
		assert (pSideDef1Info != 0);
		assert (pSideDef2Info != 0);

		// display info for LineDef info window
		pLineDefInfo->Cleanup();
		pLineDefInfo->BeginInsert();

		if (objnum < 0)
		{
			pLineDefInfo->SetStyle (TA_CENTER, DARKGRAY);
			pLineDefInfo->InsertAt(3, "(No LineDef selected)");
		}
		else
		{
			assert_ldnum(objnum);

			LineDef *pLineDef = &LineDefs[objnum];

			pLineDefInfo->SetStyle (TA_CENTER, BLACK);
			pLineDefInfo->Insert ("Selected LineDef (#%d)", objnum);

			pLineDefInfo->SetStyle (TA_LEFT, LIGHTBLUE);
			pLineDefInfo->Insert ("Flags: %s", 
					      GetLineDefFlagsName (pLineDef->blak_flags));
			pLineDefInfo->Insert ("Type:  %3d %s", pLineDef->type,
								  GetLineDefTypeName (pLineDef->type));

			sd1 = pLineDef->sidedef1;
			sd2 = pLineDef->sidedef2;
			tag = pLineDef->tag;
			s1  = pLineDef->start;
			s2  = pLineDef->end;
			n   = ComputeDist (Vertexes[s2].x - Vertexes[s1].x,
							   Vertexes[s2].y - Vertexes[s1].y);
			pLineDefInfo->Insert ("Length: %d", n);
			if (tag > 0)
			{
				for (n = 0; n < NumSectors; n++)
					if (Sectors[n].tag == tag)
						break;
			}
			else
				n = NumSectors;

			if (n < NumSectors)
				pLineDefInfo->Insert ("Sector tag: %d (#%d)", tag, n);
			else
				pLineDefInfo->Insert ("Sector tag: %d (none)", tag);
			pLineDefInfo->Insert ("Vertexes: (#%d, #%d)", s1, s2);
			pLineDefInfo->Insert ("SideDefs: (#%d, #%d)", sd1, sd2);
			if (sd1 >= 0)
				s1 = SideDefs[sd1].sector;
			else
				s1 = -1;
			if (sd2 >= 0)
				s2 = SideDefs[sd2].sector;
			else
				s2 = -1;
		}
		pLineDefInfo->EndInsert();

		// Display info about first SideDef
		pSideDef1Info->Cleanup();
		pSideDef1Info->BeginInsert();

		if (objnum < 0 || sd1 < 0)
		{
			pSideDef1Info->SetStyle (TA_CENTER, DARKGRAY);
			pSideDef1Info->InsertAt (3, "(No first SideDef)");
		}
		else
		{
			SideDef *pSideDef1 = &SideDefs[sd1];
			pSideDef1Info->SetStyle (TA_CENTER, BLACK);
			pSideDef1Info->Insert ("First SideDef (#%d)", sd1);

			pSideDef1Info->SetStyle (TA_LEFT, LIGHTBLUE);
			// Normal texture
			texname[MAX_BITMAPNAME] = '\0';
			strncpy( texname, pSideDef1->tex3, MAX_BITMAPNAME);
			pSideDef1Info->Insert ("Normal texture: %s", texname);

			// Upper texture
			strncpy( texname, pSideDef1->tex1, MAX_BITMAPNAME);
			if (s1 >= 0 && s2 >= 0 && Sectors[s1].ceilh > Sectors[s2].ceilh)
			{
				if (texname[ 0] == '-' && texname[ 1] == '\0')
					pSideDef1Info->SetStyle (TA_LEFT, RED);
			}
			else
				pSideDef1Info->SetStyle (TA_LEFT, LIGHTBLUE);
			pSideDef1Info->Insert ("Upper texture:  %s", texname);

			// Lower texture
			pSideDef1Info->SetStyle (TA_LEFT, LIGHTBLUE);
			strncpy (texname, pSideDef1->tex2, MAX_BITMAPNAME);
			if (s1 >= 0 && s2 >= 0 && Sectors[s1].floorh < Sectors[s2].floorh)
			{
				if (texname[ 0] == '-' && texname[ 1] == '\0')
					pSideDef1Info->SetStyle (TA_LEFT, RED);
			}
			else
				pSideDef1Info->SetStyle (TA_LEFT, LIGHTBLUE);
			pSideDef1Info->Insert ("Lower texture:  %s", texname);

			pSideDef1Info->SetStyle (TA_LEFT, LIGHTBLUE);
			pSideDef1Info->Insert ("Tex. offset:  (%d, %d)", pSideDef1->xoff, pSideDef1->yoff);
			pSideDef1Info->Insert ("Sector:         #%d", s1);
			pSideDef1Info->Insert ("User id #:       %d", pSideDef1->user_id);
		}
		pSideDef1Info->EndInsert();

		// Display info about second SideDef
		pSideDef2Info->Cleanup();
		pSideDef2Info->BeginInsert();

		if (objnum < 0 || sd2 < 0)
		{
			pSideDef2Info->SetStyle (TA_CENTER, DARKGRAY);
			pSideDef2Info->InsertAt (3, "(No second SideDef)");
		}
		else
		{
			SideDef *pSideDef2 = &SideDefs[sd2];
			pSideDef2Info->SetStyle (TA_CENTER, BLACK);
			pSideDef2Info->Insert ("Second SideDef (#%d)", sd2);

			pSideDef2Info->SetStyle (TA_LEFT, LIGHTBLUE);
			// Normal texture
			texname[MAX_BITMAPNAME] = '\0';
			strncpy (texname, pSideDef2->tex3, MAX_BITMAPNAME);
			pSideDef2Info->Insert ("Normal texture: %s", texname);

			// Upper texture
			strncpy( texname, pSideDef2->tex1, MAX_BITMAPNAME);
			if (s1 >= 0 && s2 >= 0 && Sectors[s2].ceilh > Sectors[s1].ceilh)
			{
				if (texname[ 0] == '-' && texname[ 1] == '\0')
					pSideDef2Info->SetStyle (TA_LEFT, RED);
			}
			else
				pSideDef2Info->SetStyle (TA_LEFT, LIGHTBLUE);
			pSideDef2Info->Insert ("Upper texture:  %s", texname);

			// Lower texture
			pSideDef2Info->SetStyle (TA_LEFT, LIGHTBLUE);
			strncpy (texname, pSideDef2->tex2, MAX_BITMAPNAME);
			if (s1 >= 0 && s2 >= 0 && Sectors[s2].floorh < Sectors[s1].floorh)
			{
				if (texname[ 0] == '-' && texname[ 1] == '\0')
					pSideDef2Info->SetStyle (TA_LEFT, RED);
			}
			else
				pSideDef2Info->SetStyle (TA_LEFT, LIGHTBLUE);
			pSideDef2Info->Insert ("Lower texture:  %s", texname);

			pSideDef2Info->SetStyle (TA_LEFT, LIGHTBLUE);
			pSideDef2Info->Insert ("Tex. offset:  (%d %d)", pSideDef2->xoff, pSideDef2->yoff);
			pSideDef2Info->Insert ("Sector:         #%d", s2);
			pSideDef2Info->Insert ("User id #:       %d", pSideDef2->user_id);
		}
		pSideDef2Info->EndInsert();
		break;

	case OBJ_VERTEXES:
		assert (pVertexInfo != 0);

		pVertexInfo->Cleanup();
		pVertexInfo->BeginInsert();

		if (objnum < 0)
		{
			pVertexInfo->SetStyle(TA_CENTER, DARKGRAY);
			pVertexInfo->Insert ("(No Vertex selected)");
		}
		else
		{
		   // XXX Tends to go off after vertices merged ARK
#if 0
		   assert (objnum >= 0 && objnum < NumVertexes);
#endif
		   if (objnum >= 0 && objnum < NumVertexes)
		   {
		      pVertexInfo->SetStyle(TA_CENTER, BLACK);
		      pVertexInfo->Insert ("Selected Vertex (#%d)", objnum);
		      
		      pVertexInfo->SetStyle(TA_LEFT, LIGHTBLUE);
		      pVertexInfo->Insert ("Coordinates: (%d, %d)",
					   Vertexes[objnum].x, Vertexes[objnum].y);
		   }
		}
		pVertexInfo->EndInsert();
		break;

	case OBJ_SECTORS:
		assert (pSectorInfo != 0);

		// display info about sector
		pSectorInfo->Cleanup();
		pSectorInfo->BeginInsert();
		if (objnum < 0)
		{
			pSectorInfo->SetStyle (TA_CENTER, DARKGRAY);
			pSectorInfo->InsertAt (3, "(No Sector selected)");
		}
		else
		{
			assert (objnum >= 0 && objnum < NumSectors);
			Sector *pSector = &Sectors[objnum];

			pSectorInfo->SetStyle (TA_CENTER, BLACK);
			pSectorInfo->Insert ("Selected Sector (#%d)", objnum);

			pSectorInfo->SetStyle (TA_LEFT, LIGHTBLUE);
			if (!(pSector->blak_flags & SF_SLOPED_FLOOR))
			{
				pSectorInfo->Insert ("Floor height:    %d", pSector->floorh);
			}
			else
			{
				pSectorInfo->Insert ("Floor height:    sloped");
			}
			if (!(pSector->blak_flags & SF_SLOPED_CEILING))
			{
				pSectorInfo->Insert ("Ceiling height:  %d", pSector->ceilh);
			}
			else
			{
				pSectorInfo->Insert ("Ceiling height:  sloped");
			}

			// Display floor texture name
			strncpy (texname, pSector->floort, MAX_BITMAPNAME);
			texname[MAX_BITMAPNAME] = '\0';
			pSectorInfo->Insert ("Floor texture:   %s", texname);

			// Display Ceiling texture name
			strncpy (texname, pSector->ceilt, MAX_BITMAPNAME);
			texname[MAX_BITMAPNAME] = '\0';
			pSectorInfo->Insert ("Ceiling texture: %s", texname);

			if (pSector->light < 128)
			   pSectorInfo->Insert ("Light level:     %dx", pSector->light * 2);
			else pSectorInfo->Insert ("Light level:     %d", (pSector->light-128) * 2);


			{
				const char* pszDirection = _apszDirection[SectorScrollDirection(pSector->blak_flags)];
				const char* pszSpeed = _apszSpeed[SectorScrollSpeed(pSector->blak_flags)];
				switch (pSector->blak_flags & (SF_SCROLL_FLOOR | SF_SCROLL_CEILING))
				{
				case 0:
					pSectorInfo->Insert ("Texture offset:  (%d, %d)", 
								 pSector->xoffset, pSector->yoffset);
					break;
				case SF_SCROLL_FLOOR | SF_SCROLL_CEILING:
					pSectorInfo->Insert ("Scroll %s %s (Floor+Ceil)",
								 pszDirection, pszSpeed);
					break;
				case SF_SCROLL_FLOOR:
					pSectorInfo->Insert ("Scroll %s %s (Floor)",
								 pszDirection, pszSpeed);
					break;
				case SF_SCROLL_CEILING:
					pSectorInfo->Insert ("Scroll %s %s (Ceiling)",
								 pszDirection, pszSpeed);
					break;
				}
			}

			pSectorInfo->Insert ("User id #:     %d", pSector->user_id);

			switch (SectorDepth(pSector->blak_flags))
			{
			case SF_DEPTH0:
			   pSectorInfo->Insert ("Depth: None");
			   break;
			case SF_DEPTH1:
			   pSectorInfo->Insert ("Depth: Shallow");
			   break;
			case SF_DEPTH2:
			   pSectorInfo->Insert ("Depth: Deep");
			   break;
			case SF_DEPTH3:
			   pSectorInfo->Insert ("Depth: Very deep");
			   break;
			}
		}
		pSectorInfo->EndInsert();
		break;
	}
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::EditCurObject ()
{
	if ( CurObject >= 0 )
	{
		// Make sure current object is selected (and first in list)
		SetupSelection (TRUE);

		// Start UNDO recording
		StartUndoRecording (AddObjectType("Edit"));

		// Edit selected objects
		int rc = IDCANCEL;
		if ( EditMode == OBJ_THINGS )
		{
			SET_HELP_CONTEXT(Things_edit_mode);
			rc = TThingEditDialog (this, Selected).Execute();
			RESTORE_HELP_CONTEXT();
		}
		else if ( EditMode == OBJ_LINEDEFS )
		{
			SET_HELP_CONTEXT(LineDefs_edit_mode);
			rc = TLineDefEditDialog (this, Selected).Execute();
			RESTORE_HELP_CONTEXT();
		}
		else if ( EditMode == OBJ_SECTORS )
		{
			SET_HELP_CONTEXT(Sectors_edit_mode);
			rc = TSectorEditDialog (this, Selected).Execute();
			RESTORE_HELP_CONTEXT();
		}
		else if ( EditMode == OBJ_VERTEXES )
		{
			SET_HELP_CONTEXT(Vertices_edit_mode);
			rc = TVertexDialog (this, Selected).Execute();
			RESTORE_HELP_CONTEXT();
		}

		// Save UNDO data
		StopUndoRecording();

		// Restore old selection
		SetupSelection (FALSE);

		// Redraw map
		if ( rc == IDOK )
		{
			// Redraw map
			RefreshWindows();
		}
	}
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
// Draw the selection box using R2_XORPEN.
void TEditorClient::DrawStretchSelBox(TMapDC &dc)
{
	int oldROP2 = dc.SetROP2 (R2_XORPEN);
	dc.SetPenColor16 (CYAN);
	dc.DrawMapRect (SelBoxX, SelBoxY, OldSelBoxX, OldSelBoxY);
	dc.SetROP2 (oldROP2);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
// Draw the selected objects usign current ROP2
//
void TEditorClient::DrawMovingObjects(TMapDC &dc, BOOL doDelete)
{
	int OldROP2;

	if (doDelete)
	{
		OldROP2 = dc.SetROP2 (R2_COPYPEN);
		dc.SetDelete (TRUE);
	}
	else
		OldROP2 = dc.SetROP2 (R2_XORPEN);

	switch (EditMode)
	{
	case OBJ_THINGS:
		// Draw the selected things
		dc.DrawMapThingsList (EditMode, Selected);
		break;

	case OBJ_LINEDEFS:
		// Draw the moving LineDefs
		if ( doDelete  )
			dc.DrawMapLineDefsList (EditMode, MovingLineDefs, FALSE);
		else
			dc.DrawMapLineDefsList (EditMode, MovingLineDefs, DrawLineDefsLen);
		break;

	case OBJ_VERTEXES:
		// Draw the LineDefs
		if ( doDelete  )
			dc.DrawMapLineDefsList (EditMode, MovingLineDefs, FALSE);
		else
			dc.DrawMapLineDefsList (EditMode, MovingLineDefs, DrawLineDefsLen);

		// Draw the selected vertexes
		dc.DrawMapVertexesList (EditMode, Selected);
		break;

	case OBJ_SECTORS:
		// Draw the moving LineDefs
		dc.DrawMapLineDefsList (EditMode, MovingLineDefs);
		break;
	}

	// Restore drawing mode
	dc.SetROP2 (OldROP2);
	if ( doDelete )
		dc.SetDelete (FALSE);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
// Select the LineDefs attached to the selected objects
// (nothing to select if edit mode is OBJ_THINGS)
void TEditorClient::SelectMovingLineDefs ()
{
	assert (MovingLineDefs == NULL);
	SelPtr cur, SectorLineDefs;
	SHORT n;

	switch (EditMode)
	{
	case OBJ_THINGS:
		// No LineDefs to select
		break;

	case OBJ_LINEDEFS:
		// Select the LineDefs
		for (n = 0 ; n < NumLineDefs ; n++ )
		{
			LineDef *pLineDef = &LineDefs[n];
			SHORT nstart = pLineDef->start;
			SHORT nend   = pLineDef->end;

			for (cur = Selected ; cur != NULL ; cur = cur->next)
			{
				LineDef *pCLineDef = &LineDefs[cur->objnum];
				SHORT cstart = pCLineDef->start;
				SHORT cend   = pCLineDef->end;

				if ( cstart == nstart || cstart == nend ||
					 cend   == nstart || cend   == nend )
				{
					SelectObject (&MovingLineDefs, n);
					break;  // Select the LineDef only once
				}
			}
		}
		break;

	case OBJ_VERTEXES:
		// Select the LineDefs connected to the selected Vertexes
		for (n = 0 ; n < NumLineDefs ; n++ )
		{
			LineDef *pLineDef = &LineDefs[n];
			SHORT nstart = pLineDef->start;
			SHORT nend   = pLineDef->end;
			// For each selected Vertex, look if it's an extremity of a LineDef
			for (cur = Selected ; cur != NULL ; cur = cur->next)
			{
				if ( nstart == cur->objnum || nend == cur->objnum )
				{
					SelectObject (&MovingLineDefs, n);
					break;  // Select the LineDef only once
				}
			}
		}
		break;

	case OBJ_SECTORS:
		// Select the LineDefs of the sectors
		SectorLineDefs = NULL;
		for (n = 0 ; n < NumLineDefs ; n++ )
		{
			LineDef *pLineDef = &LineDefs[n];
			SHORT s1 = pLineDef->sidedef1;
			SHORT s2 = pLineDef->sidedef2;

			for (cur = Selected ; cur != NULL ; cur = cur->next)
			{
				// If the LineDef is owned by the sector, select it
				if ( (s1 >= 0 && SideDefs[s1].sector == cur->objnum) ||
					 (s2 >= 0 && SideDefs[s2].sector == cur->objnum) )
				{
					SelectObject (&SectorLineDefs, n);
					break;  // Select the LineDef only once
				}
			}
		}

		// Select the LineDefs connected to the LineDefs of the sectors
		for (n = 0 ; n < NumLineDefs ; n++ )
		{
			LineDef *pLineDef = &LineDefs[n];
			SHORT nstart = pLineDef->start;
			SHORT nend   = pLineDef->end;

			for (cur = SectorLineDefs ; cur != NULL ; cur = cur->next)
			{
				LineDef *pCLineDef = &LineDefs[cur->objnum];
				SHORT start  = pCLineDef->start;
				SHORT end    = pCLineDef->end;

				if ( start == nstart || start == nend ||
					 end   == nstart || end   == nend )
				{
					SelectObject (&MovingLineDefs, n);
					break;  // Select the LineDef only once
				}
			}
		}
		ForgetSelection (&SectorLineDefs);
		break;
	}
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//   check for players starting points
BOOL TEditorClient::CheckStartingPos()
{
	// we don't need starting points ARK
	return TRUE;

#if 0
	BOOL p1 = FALSE;
	BOOL p2 = FALSE;
	BOOL p3 = FALSE;
	BOOL p4 = FALSE;
	SHORT dm = 0;
	SHORT t;
	for (t = 0; t < NumThings; t++)
	{
		SHORT type = Things[t].type;

		if (type == THING_PLAYER1)			p1 = TRUE;
		if (type == THING_PLAYER2)          p2 = TRUE;
		if (type == THING_PLAYER3)          p3 = TRUE;
		if (type == THING_PLAYER4)          p4 = TRUE;
		if (type == THING_DEATHMATCH)       dm++;
	}

	if (Expert && p1 == TRUE)
		return TRUE;

	if (p1 == FALSE)
	{
		if ( Confirm ("There is no player 1 starting point. You will not be "
					  "able to use this level for single player games.\n"
					  "Do you want to return to the editor ?") == TRUE )
			return FALSE;
	}

	if (p2 == FALSE || p3 == FALSE || p4 == FALSE)
	{
		if (p4 == FALSE)			t = 4;
		if (p3 == FALSE)			t = 3;
		if (p2 == FALSE)			t = 2;
		if ( Confirm ("There is no player %d starting point. You will not be "
					  "able to use this level for multi-player games.\n"
					  "Do you want to return to the editor ?", t) == TRUE )
			return FALSE;
	}

	if (dm < 4)
	{
		char msg[256];

		if (dm == 0)
			wsprintf(msg, "There are no DeathMatch starting points.");
		else if (dm == 1)
			wsprintf(msg, "There is only one DeathMatch starting point.");
		else
			wsprintf(msg, "There are only %d DeathMatch starting points.", dm);

		if ( Confirm ("%s\n"
					  "You need at least four starting points "
					  "to play DeathMatch games.\n"
					  "Do you to want to return to the editor ?", msg) == TRUE )
			return FALSE;
	}

	return TRUE;
#endif
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//   center the map around the given map coords
void TEditorClient::CenterMapAroundCoords (SHORT xpos, SHORT ypos)
{
	OrigX = xpos;
	OrigY = ypos;
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::DrawStatusBar()
{
	if (InfoShown)
	{
		// 500 is from LevelName size, 20 from 'Editing %s..' below, 20 for EditMode
		char msg[500 + 20 + 20];
		int len;

		// OK to print stats without level now
#if 0
		// We must be editing a level
		assert (Level != NULL);
#endif
		if (pStatusBar && pStatusBar->GadgetCount() >= 1)
		{
			// Draw the mode info in the first text gadget of status bar
			len = wsprintf(msg, "Editing %s on %s",
				GetEditModeName(EditMode), LevelName);

			if (MadeMapChanges == TRUE)
				strcpy(&msg[len], " *+");

			else if (MadeChanges == TRUE)
				strcpy(&msg[len], " *");

			((TTextGadget *)pStatusBar->FirstGadget())->SetText(msg);
		}

		if (pStatusBar && pStatusBar->GadgetCount() >= 3)
		{
			// Draw the scale info in the third text gadget of status bar
			len = wsprintf(msg, "Scale: %d/%d  Grid: %d",
				ScaleNum, ScaleDen, GridScale);
			if (SnapToGrid)
				strcpy(&msg[len], "*");

			((TTextGadget *)pStatusBar->FirstGadget()->NextGadget()->NextGadget())->SetText(msg);
		}

		// Draw the memory info
		TMainFrame *mainFrame =
			TYPESAFE_DOWNCAST(GetApplication()->GetMainWindow(), TMainFrame);
		mainFrame->DrawFreeMemory();
	}
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
// Get the name of a new .WAD file. If the filename is OK, return
// TRUE.
//
BOOL TEditorClient::GetWadSaveFileName (char *filename, char * /*levelname*/)
{
	char *dotp;
	int rc;

	// Level variable meaningless now ARK
#if 0
	// We must be editing a level
	assert (Level != NULL);

	// get the default filename
	if ( stricmp(Level->wadfile->filename, MainWad) == 0 )
	{
		strcpy (filename, levelname);
		strcat (filename, ".roo");
	}
	else
		strcpy (filename, Level->wadfile->filename);
#endif

	strcpy (filename, LevelName);

	// Excecute an SaveDialog until valid wad filename
	strcpy (FileData.FileName, filename);
	while ( (rc = TFileSaveDialog(this, FileData).Execute()) == IDOK )
	{
		if ( stricmp(filename, MainWad) == 0 )
		{
			if ( MessageBox ("You cannot overwrite main wad file !\n"
							 "You can retry to choose another file name",
							 "Error",
							 MB_RETRYCANCEL) == IDCANCEL )
				return FALSE;
			else
				continue;
		}
		else
			break;
	}
	if ( rc == IDCANCEL )
		return FALSE;

	// Get the new filename
	strcpy (filename, FileData.FileName);

	// if the WAD file already exists, rename it to "*.BAK"
	for (WadPtr wad = WadFileList; wad != NULL; wad = wad->next)
	{
		if ( stricmp (filename, wad->filename) == 0 )
		{
			dotp = strrchr (wad->filename, '.');
			if (dotp == NULL)
				strcat (wad->filename, ".BAK");
			else
				strcpy (dotp, ".BAK");

			// need to close, then reopen: problems with SHARE.EXE
			fclose (wad->fileinfo);
			if (rename (filename, wad->filename) < 0)
			{
				if (unlink (wad->filename) < 0)
					ProgError ("Could not delete file \"%s\"", wad->filename);
				if (rename (filename, wad->filename) < 0)
					ProgError ("Could not rename \"%s\" to \"%s\"",
							   filename, wad->filename);
			}

			// Open new wad file
			wad->fileinfo = fopen (wad->filename, "rb");
			if (wad->fileinfo == NULL)
				ProgError ("Could not reopen file \"%s\"", wad->filename);
			break;
		}
	} // end of for

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
// Save the current level data if there are unsaved changes.
// Return FALSE if user asked to save changes, but couldn't choose a
// correct file name.
//
BOOL TEditorClient::SaveChanges ()
{
	BOOL rc = TRUE;

	// don't close if "insert object" mode
	if ( InsertingObject )
		return FALSE;

	SET_HELP_CONTEXT(Quit_editor);

	if (! Registered)
	{
		Notify ("Please register DOOM if you want to be able "
				"to save your changes.");
	}
	else if (Registered &&
			 (MadeChanges || MadeMapChanges) &&
			 Confirm ("You have unsaved changes. Do you want to save them ?"))
	{
		rc = SaveLevel();
	}

	RESTORE_HELP_CONTEXT();
	return rc;
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
// Save level data to .WAD file.
// Return FALSE if the level hasn't starting position, or if the user
// couldn't choose a correct file name for the level.
//
BOOL TEditorClient::SaveLevel ()
{
	char filename[MAX_PATH];

	if (Registered                               == FALSE ||
		CheckStartingPos()                       == FALSE ||
		GetWadSaveFileName (filename, LevelName) == FALSE )
			return FALSE;

	if ((0 == RoomID) && NumThings > 2)
	{
		MessageBox("Please set Room ID from File menu before saving","Error Saving",MB_OK);
		return FALSE;
	}
	SaveLevelData(filename);
	strcpy(LevelName, filename);

   // Don't delete this data on saving.
	// Forget all UNDO datas
	//CleanupUndo();
	//InitUndo();

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//  Call CloseWindow of the mainframe, which calls CanClose of the
//  application.
//
void TEditorClient::CmFileQuit ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	// Send WM_CLOSE message to main frame
	GetApplication()->GetMainWindow()->PostMessage(WM_CLOSE);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//  Hightlight the previous object (by number)
//
void TEditorClient::CmSearchPrev ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	TMapDC dc (this);

	// Unhighlight current object
	if (CurObject >= 0 )
		HighlightObject (dc, EditMode, CurObject, HL_COLOR);

	if (CurObject > 0)
		CurObject--;
	else
		CurObject = GetMaxObjectNum (EditMode);

	// Highlight new current object
	if (CurObject >= 0 )
	{
		HighlightObject (dc, EditMode, CurObject, HL_COLOR);
		DisplayObjectInfo (EditMode, CurObject);
	}
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//  Hightlight the next object (by number)
//
void TEditorClient::CmSearchNext ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	TMapDC dc (this);

	// Unhighlight current object
	if (CurObject >= 0 )
		HighlightObject (dc, EditMode, CurObject, HL_COLOR);

	if (CurObject < GetMaxObjectNum( EditMode))
		CurObject++;
	else if (GetMaxObjectNum( EditMode) >= 0)
		CurObject = 0;
	else
		CurObject = -1;

	// Highlight new current object
	if (CurObject >= 0 )
	{
		HighlightObject (dc, EditMode, CurObject, HL_COLOR);
		DisplayObjectInfo (EditMode, CurObject);
	}
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//  Ask for an object number, center the map around this object and
//  select it as CurObject.
//
//  NOTE: Calls Invalidate, because of GoToObject, which changes OrigX
//        and OrigY
//
void TEditorClient::CmSearchJump ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Jump_to_object);

	char Prompt[80];
	char Title[40];
	char InputText[7];

	// Init. input dialog parameters
	TRangeValidator *pvalid = new TRangeValidator (0, GetMaxObjectNum(EditMode));

	wsprintf (Prompt, "Enter an %s number between %d and %d",
					  GetObjectTypeName (EditMode),
					  0, GetMaxObjectNum(EditMode));
	wsprintf (Title, "Jump to a %s number",
					  GetObjectTypeName (EditMode));
	wsprintf (InputText, "%d", CurObject);

	// Execute input dialog
	if ( TInputDialog(this, Title, Prompt,
					  InputText, 6, 0, pvalid).Execute() == IDOK )
	{
		SHORT NewObject = (SHORT)atoi (InputText);

		// True because of TRangeValidator
		assert (NewObject >= 0  &&  NewObject <= GetMaxObjectNum (EditMode));

		CurObject = NewObject;
		GoToObject (EditMode, CurObject);
		RefreshWindows();
	}
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmObjectsRectangle ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	// Keep in memory between calls
	static SHORT Width  = 256;
	static SHORT Height = 128;
	char Title[80];
	char Prompt[80];
	TRangeValidator *pValid1 = new TRangeValidator(8, 14000);
	TRangeValidator *pValid2 = new TRangeValidator(8, 14000);
	char wBuf[7];
	char hBuf[7];

	SET_HELP_CONTEXT(Insert_Rectangle);
	wsprintf (Title, "New rectangle size");
	wsprintf (Prompt, "Enter Width and Height of new Rectangle:");
	wsprintf (wBuf, "%d", Width);
	wsprintf (hBuf, "%d", Height);

	if ( TInput2Dialog (this, Title, Prompt,
						wBuf, 7, hBuf, 7,
						pValid1, pValid2).Execute() == IDOK )
	{
		Width  = (SHORT)atoi(wBuf);
		Height = (SHORT)atoi(hBuf);

		// Hide information windows
		BOOL OldInfoWinShown = InfoWinShown;
		InfoWinShown = FALSE;
		if ( OldInfoWinShown != InfoWinShown )
		{
			SetupInfoWindows();
			UpdateWindow();
		}

		// Clip cursor movements to editor window and get mouse capture
		TRect editRect;
		GetWindowRect (editRect);
		editRect.right++;
		editRect.bottom++;
		ClipCursor(&editRect);
		SetCursor (GetApplication(), IDC_INSERT);
		SetCapture();

		// Begin insert mode
		WorkMessage("Click left mouse button to insert rectangle...");
		InsertingObject = TRUE;  // don't highlight when mouse move

		// Stops when left button down
		MSG  loopMsg;
		loopMsg.message = 0;
		while (loopMsg.message != WM_LBUTTONDOWN)
		{
			if (::PeekMessage(&loopMsg, 0, 0, 0, PM_REMOVE))
			{
				// Don't send the WM_LBUTTONDOWN, because we don't
				// want to select an object!
				if ( loopMsg.message != WM_LBUTTONDOWN )
				{
					::TranslateMessage(&loopMsg);
					::DispatchMessage(&loopMsg);
				}
			}

			Scroller->AutoScroll();
		}

		// Restore window and cursor
		InsertingObject = FALSE;
		SetCursor (NULL, IDC_ARROW);
		ReleaseCapture();
		ClipCursor(NULL);
		GetApplication()->ResumeThrow();

		// UNDO
		StartUndoRecording("Insert rectangle");

		// Insert rectangle a cursor position
		InsertRectangle (MAPX(PointerX), MAPY(PointerY), Width, Height);

		// UNDO
		StopUndoRecording();

		// Redraw map, status bar and info windows
		InfoWinShown = OldInfoWinShown;
		SetupInfoWindows();
		DrawStatusBar();
		Invalidate();
	}
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmObjectsPolygon ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	// Keep in memory between calls
	static SHORT nSides = 8;
	static SHORT Radius = 128;
	char Title[80];
	char Prompt[80];
	TRangeValidator *pValid1 = new TRangeValidator(3, 1024);
	TRangeValidator *pValid2 = new TRangeValidator(8, 14000);
	char nBuf[7];
	char rBuf[7];

	SET_HELP_CONTEXT(Insert_Polygon);
	wsprintf (Title, "New polygon size");
	wsprintf (Prompt, "Enter number of sides and radius of new polygon:");
	wsprintf (nBuf, "%d", nSides);
	wsprintf (rBuf, "%d", Radius);
	if ( TInput2Dialog (this, Title, Prompt,
						nBuf, 7, rBuf, 7,
						pValid1, pValid2).Execute() == IDOK )
	{
		nSides = (SHORT)atoi(nBuf);
		Radius = (SHORT)atoi(rBuf);

		// Hide information windows
		BOOL OldInfoWinShown = InfoWinShown;
		InfoWinShown = FALSE;
		if ( OldInfoWinShown != InfoWinShown )
		{
			SetupInfoWindows();
			UpdateWindow();
		}

		// Clip cursor movements to editor window and get mouse capture
		TRect editRect;
		GetWindowRect (editRect);
		editRect.right++;
		editRect.bottom++;
		ClipCursor(&editRect);
		SetCursor (GetApplication(), IDC_INSERT);
		SetCapture();

		// Begin insert mode
		WorkMessage("Click left mouse button to insert polygon...");
		InsertingObject = TRUE;  // don't highlight when mouse move

		// Stops when left button down
		MSG  loopMsg;
		loopMsg.message = 0;
		while (loopMsg.message != WM_LBUTTONDOWN)
		{
			if (::PeekMessage(&loopMsg, 0, 0, 0, PM_REMOVE))
			{
				// Don't send the WM_LBUTTONDOWN, because we don't
				// want to select an object!
				if ( loopMsg.message != WM_LBUTTONDOWN )
				{
					::TranslateMessage(&loopMsg);
					::DispatchMessage(&loopMsg);
				}
			}

			Scroller->AutoScroll();
		}

		// Restore window and cursor
		InsertingObject = FALSE;
		SetCursor (NULL, IDC_ARROW);
		ReleaseCapture();
		ClipCursor(NULL);
		GetApplication()->ResumeThrow();

		// UNDO
		StartUndoRecording("Insert polygon");

		// Insert rectangle a cursor position
		InsertPolygon (MAPX(PointerX), MAPY(PointerY), nSides, Radius);

		// UNDO
		StopUndoRecording();

		// Redraw map, status bar and info windows
		InfoWinShown = OldInfoWinShown;
		SetupInfoWindows();
		DrawStatusBar();
		Invalidate();
	}
	RESTORE_HELP_CONTEXT();
}

/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmObjectsCircle()
{
   // Ignore if "insert object" mode
   if (InsertingObject)
      return;

   // Keep in memory between calls
   static SHORT Radius = 256;
   char Title[80];
   char Prompt[80];
   TRangeValidator *pValid1 = new TRangeValidator(8, 14000);
   char wBuf[7];

   SET_HELP_CONTEXT(Insert_Circle);
   wsprintf(Title, "New circle size");
   wsprintf(Prompt, "Enter radius of circle:");
   wsprintf(wBuf, "%d", Radius);

   if (TInput1Dialog(this, Title, Prompt, wBuf, 7, pValid1).Execute() == IDOK)
   {
      Radius = (SHORT)atoi(wBuf);

      // Hide information windows
      BOOL OldInfoWinShown = InfoWinShown;
      InfoWinShown = FALSE;
      if (OldInfoWinShown != InfoWinShown)
      {
         SetupInfoWindows();
         UpdateWindow();
      }

      // Clip cursor movements to editor window and get mouse capture
      TRect editRect;
      GetWindowRect(editRect);
      editRect.right++;
      editRect.bottom++;
      ClipCursor(&editRect);
      SetCursor(GetApplication(), IDC_INSERT);
      SetCapture();

      // Begin insert mode
      WorkMessage("Click left mouse button to insert circle...");
      InsertingObject = TRUE;  // don't highlight when mouse move

      // Stops when left button down
      MSG  loopMsg;
      loopMsg.message = 0;
      while (loopMsg.message != WM_LBUTTONDOWN)
      {
         if (::PeekMessage(&loopMsg, 0, 0, 0, PM_REMOVE))
         {
            // Don't send the WM_LBUTTONDOWN, because we don't
            // want to select an object!
            if (loopMsg.message != WM_LBUTTONDOWN)
            {
               ::TranslateMessage(&loopMsg);
               ::DispatchMessage(&loopMsg);
            }
         }

         Scroller->AutoScroll();
      }

      // Restore window and cursor
      InsertingObject = FALSE;
      SetCursor(NULL, IDC_ARROW);
      ReleaseCapture();
      ClipCursor(NULL);
      GetApplication()->ResumeThrow();

      // UNDO
      StartUndoRecording("Insert circle");

      // Insert rectangle a cursor position
      InsertCircle(MAPX(PointerX), MAPY(PointerY), Radius);

      // UNDO
      StopUndoRecording();

      // Redraw map, status bar and info windows
      InfoWinShown = OldInfoWinShown;
      SetupInfoWindows();
      DrawStatusBar();
      Invalidate();
   }
   RESTORE_HELP_CONTEXT();
}

/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmObjectsTorch()
{
   // Ignore if "insert object" mode
   if (InsertingObject)
      return;
   // Keep in memory between calls.
   // 0 is north, 90 is east.
   static SHORT torchAngle = 0;
   char Title[80];
   char Prompt[80];
   char aBuf[7];
   TRangeValidator *pValid1 = new TRangeValidator(0, 360);
   
   SET_HELP_CONTEXT(Insert_Torch);
   wsprintf(Title, "New torch angle");
   wsprintf(Prompt, "Enter the angle for the torch (0 = north, 90 = east):");
   wsprintf(aBuf, "%d", torchAngle);

   if (TInputDialog(this, Title, Prompt, aBuf, 7, 0, pValid1).Execute() == IDOK)
   {
      torchAngle = (SHORT)atoi(aBuf);

      // Hide information windows
      BOOL OldInfoWinShown = InfoWinShown;
      InfoWinShown = FALSE;
      if (OldInfoWinShown != InfoWinShown)
      {
         SetupInfoWindows();
         UpdateWindow();
      }

      // Clip cursor movements to editor window and get mouse capture
      TRect editRect;
      GetWindowRect(editRect);
      editRect.right++;
      editRect.bottom++;
      ClipCursor(&editRect);
      SetCursor(GetApplication(), IDC_INSERT);
      SetCapture();

      // Begin insert mode
      WorkMessage("Click left mouse button to insert torch...");
      InsertingObject = TRUE;  // don't highlight when mouse move

      // Stops when left button down
      MSG  loopMsg;
      loopMsg.message = 0;
      while (loopMsg.message != WM_LBUTTONDOWN)
      {
         if (::PeekMessage(&loopMsg, 0, 0, 0, PM_REMOVE))
         {
            // Don't send the WM_LBUTTONDOWN, because we don't
            // want to select an object!
            if (loopMsg.message != WM_LBUTTONDOWN)
            {
               ::TranslateMessage(&loopMsg);
               ::DispatchMessage(&loopMsg);
            }
         }

         Scroller->AutoScroll();
      }

      // Restore window and cursor
      InsertingObject = FALSE;
      SetCursor(NULL, IDC_ARROW);
      ReleaseCapture();
      ClipCursor(NULL);
      GetApplication()->ResumeThrow();

      // UNDO
      StartUndoRecording("Insert torch");

      // Insert torch at cursor position
      InsertTorch(MAPX(PointerX), MAPY(PointerY), torchAngle);

      // UNDO
      StopUndoRecording();

      // Redraw map, status bar and info windows
      InfoWinShown = OldInfoWinShown;
      SetupInfoWindows();
      DrawStatusBar();
      Invalidate();
   }
   RESTORE_HELP_CONTEXT();
}

/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmModeThings ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	ForgetSelection(&Selected);
	ChangeMode (OBJ_THINGS);
}

/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmModeVertexes ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	ForgetSelection(&Selected);
	ChangeMode (OBJ_VERTEXES);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmModeLinedefs ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	ForgetSelection(&Selected);
	ChangeMode (OBJ_LINEDEFS);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmModeSectors ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	ForgetSelection(&Selected);
	ChangeMode (OBJ_SECTORS);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmModeNext ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	int NewMode;

	switch (EditMode)
	{
		case OBJ_THINGS:
			NewMode = OBJ_VERTEXES;
			break;
		case OBJ_VERTEXES:
			NewMode = OBJ_LINEDEFS;
			break;
		case OBJ_LINEDEFS:
			NewMode = OBJ_SECTORS;
			break;
		case OBJ_SECTORS:
			NewMode = OBJ_THINGS;
			break;
	}

	ChangeMode(NewMode);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmModePrev ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	int NewMode;

	switch (EditMode)
	{
		case OBJ_THINGS:
			NewMode = OBJ_SECTORS;
			break;
		case OBJ_VERTEXES:
			NewMode = OBJ_THINGS;
			break;
		case OBJ_LINEDEFS:
			NewMode = OBJ_VERTEXES;
			break;
		case OBJ_SECTORS:
			NewMode = OBJ_LINEDEFS;
			break;
	}

	ChangeMode(NewMode);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscFindTag ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Free_tag_number);
	Notify ("First free tag number: %d", FindFreeTag());
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//   Check that the number of selected objects is between
//   min and max.
//   If min==0, no minimum number
//   If max==0, no maximum number
BOOL TEditorClient::CheckSelection (SHORT min, SHORT max)
{
	BOOL rc = TRUE;
	SHORT count = 0;

	// Select CurObject is no selection
	SetupSelection (TRUE);

	// Count selected objects
	for (SelPtr cur = Selected ; cur != NULL ; cur = cur->next)
		count ++;

	// Must select an EXACT number of objects
	if ( min >= 1 && min == max && count != min )
	{
		Beep();
		Notify ("You must select exactly %d %s !",
				min,
				min == 1 ? GetObjectTypeName (EditMode) :
						   GetObjectsTypeName (EditMode));
		rc = FALSE;
	}
	// Must select a MINIMUM number of objects
	else if ( min >= 1 && count < min )
	{
		Beep();
		Notify ("You must select at least %d %s !",
				min,
				min == 1 ? GetObjectTypeName (EditMode) :
						   GetObjectsTypeName (EditMode));
		rc = FALSE;
	}
	// Must select less than a MAXIMUM number of objects
	else if ( max >= 1 && count > max )
	{
		Beep();
		Notify ("You must select LESS than %d %s !",
				min,
				min == 1 ? GetObjectTypeName (EditMode) :
						   GetObjectsTypeName (EditMode));
		rc = FALSE;
	}

	// Restore selection
	if ( rc == FALSE )
		SetupSelection (FALSE);

	return rc;
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscRotateScale ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	DWORD hc;
		 if (EditMode == OBJ_THINGS)	hc = Rotate_and_scale_Things;
	else if (EditMode == OBJ_VERTEXES)	hc = Rotate_and_scale_Vertices;
	else if (EditMode == OBJ_LINEDEFS)	hc = Rotate_and_scale_LineDefs;
	else if (EditMode == OBJ_SECTORS)	hc = Rotate_and_scale_Sectors;
	SET_HELP_CONTEXT(hc);

	char Title[80];
	char Prompt[80];
	char Buf1[7], Buf2[7];
	static SHORT Angle = 0;
	static SHORT Scale = 100;

	if ( EditMode == OBJ_THINGS || EditMode == OBJ_VERTEXES )
	{
		// Check at least two object selected
		if ( CheckSelection (2, -1) == FALSE )
			goto End;
	}
	else
	{
		// Check at least one object selected
		if ( CheckSelection (1, -1) == FALSE )
			goto End;
	}

	// Input Angle and Scale
	wsprintf (Title, "%s rotation and scaling", GetObjectTypeName(EditMode));
	wsprintf (Prompt, "Enter rotation angle (0-360) and scale factor (in %%) (1-1000)");
	wsprintf (Buf1, "%d", Angle);
	wsprintf (Buf2, "%d", Scale);

	if ( TInput2Dialog (this, Title, Prompt,
						Buf1, 6, Buf2, 6,
						new TRangeValidator (0, 360),
						new TRangeValidator (1, 1000)).Execute() == IDOK )
	{
		// Rotate and Scale objects
		Angle = (SHORT)atoi (Buf1);
		Scale = (SHORT)atoi (Buf2);

		// UNDO
		StartUndoRecording (AddObjectType ("Rotate"));

		RotateAndScaleObjects (EditMode, Selected,
							   (double) Angle * 0.0174533,
							   (double) Scale * 0.01);

		// UNDO
		StopUndoRecording();
	}

	// SELECTION IS STILL VALID
	// Restore selection
	SetupSelection (FALSE);

	// Redraw map, info windows and status bar
	RefreshWindows();

End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscVDelete ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Delete_Vertex_and_join_LineDefs);

	// Check at least one object selected
	if ( CheckSelection (1, -1) == FALSE )
		goto End;

	// UNDO
	StartUndoRecording (AddObjectType("Delete"));

	// Delete vertices
	DeleteVerticesJoinLineDefs (Selected);

	// UNDO
	StopUndoRecording();

	// SELECTION MAY BE NOT VALID
	// Forget selection to prevent invalid object numbers
	ForgetSelection (&Selected);
	CurObject = -1;

	// Redraw map, info windows and status bar
	RefreshWindows();

End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscVMerge ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Merge_several_Vertices_into_one);

	// Check at least one object selected
	if ( CheckSelection (1, -1) == FALSE )
		goto End;

	// UNDO
	StartUndoRecording (AddObjectType("Merge"));

	// Merge vertices
	MergeVertices (&Selected);

	// UNDO
	StopUndoRecording();

	// SELECTION MAY BE NOT VALID
	// Forget selection to prevent invalid object numbers
	ForgetSelection (&Selected);
	CurObject = -1;

	// Redraw map, info windows and status bar
	RefreshWindows();

End:
	RESTORE_HELP_CONTEXT();
}



/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscVAddLineDef ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Add_a_LineDef_and_split_Sector);

	// Check two object selected
	if ( CheckSelection (2, 2) == FALSE )
		goto End;

	// UNDO
	StartUndoRecording("Add LineDef");

	// Add LineDef and Split sector
	SplitSector (Selected->objnum, Selected->next->objnum);

	// UNDO
	StopUndoRecording();

	// SELECTION MAY BE NOT VALID
	// Forget selection to prevent invalid object numbers
	ForgetSelection (&Selected);
	CurObject = -1;

	// Redraw map, info windows and status bar
	RefreshWindows();

End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscLDSplitAdd ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Split_LineDef_and_Add_new_Vertex);

	// Check at least one object selected
	if ( CheckSelection (1, -1) == FALSE )
		goto End;

	// UNDO
	StartUndoRecording("Split LineDef");

	// Split LineDef and add a vertex
	SplitLineDefs (Selected);

	// UNDO
	StopUndoRecording();

	// SELECTION IS STILL VALID
	// Restore selection
	SetupSelection (FALSE);

	// Redraw map, info windows and status bar
	RefreshWindows();
End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscLDSplitSector ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Split_LineDefs_and_Sectors);

	// Check two object selected
	if ( CheckSelection (2, 2) == FALSE )
		goto End;

	// UNDO
	StartUndoRecording("Split Sector");

	// Split LineDef's and add a sector
	SplitLineDefsAndSector (Selected->objnum, Selected->next->objnum);

	// UNDO
	StopUndoRecording();

	// SELECTION IS STILL VALID
	// Restore selection
	SetupSelection (FALSE);

	// Redraw map, info windows and status bar
	RefreshWindows();

End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscLDDelete ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Delete_LineDefs_and_join_Sectors);

	// Check at least one object selected
	if ( CheckSelection (1, -1) == FALSE )
		goto End;

	// UNDO
	StartUndoRecording("Delete LineDef");

	// Delete LineDefs and join sectors
	DeleteLineDefsJoinSectors (&Selected);

	// UNDO
	StopUndoRecording();

	// SELECTION MAY BE NOT VALID
	// Forget selection to prevent invalid object numbers
	ForgetSelection (&Selected);
	CurObject = -1;

	// Redraw map, info windows and status bar
	RefreshWindows();

End:
	RESTORE_HELP_CONTEXT();
}

/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscLDFlip ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Flip_LineDef);

	// Check at least one object selected
	if ( CheckSelection (1, -1) == FALSE )
		goto End;

	// UNDO
	StartUndoRecording("Flip LineDef");

	// Flip LineDefs
	FlipLineDefs (Selected, TRUE);

	// UNDO
	StopUndoRecording();

	// SELECTION IS STILL VALID
	// Restore selection
	SetupSelection (FALSE);

	// Redraw map, info windows and status bar
	RefreshWindows();

End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscLDSwap ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Swap_SideDefs);

	// Check at least one object selected
	if ( CheckSelection (1, -1) == FALSE )
		goto End;

	// Swap LineDefs
	if (Expert ||
		Confirm ("Warning: The Sector references are also swapped.\nYou may "
				 "get strange results if you don't know what you are doing !"))
	{
		// UNDO
		StartUndoRecording("Swap SideDefs");

		// Swap LineDefs
		FlipLineDefs (Selected, FALSE);

		// UNDO
		StopUndoRecording();
	}

	// SELECTION IS STILL VALID
	// Restore selection
	SetupSelection (FALSE);

	// Redraw map, info windows and status bar
	RefreshWindows();

End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscLDAlignY ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Align_textures_Y_offset);

	// Check at least one object selected
	if ( CheckSelection (1, -1) == FALSE )
		goto End;

	SelPtr sdlist, cur;

	// UNDO
	StartUndoRecording("Align Y offset");

	// select all SideDefs
	sdlist = NULL;
	for (cur = Selected; cur; cur = cur->next)
	{
		LineDef *pLineDef = &LineDefs[cur->objnum];

		if (pLineDef->sidedef1 >= 0)
			SelectObject (&sdlist, pLineDef->sidedef1);

		if (pLineDef->sidedef2 >= 0)
			SelectObject (&sdlist, pLineDef->sidedef2);
	}

	// align the textures along the Y axis (height)
	AlignTexturesY (&sdlist);
	ForgetSelection (&sdlist);

	// UNDO
	StopUndoRecording();

	// SELECTION IS STILL VALID
	// Restore selection
	SetupSelection (FALSE);

	// Redraw map, info windows and status bar
	RefreshWindows();

End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscSMakeDoor ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Make_door_from_Sector);

	// Check at least one object selected
	if ( CheckSelection (1, 1) == FALSE )
		goto End;

	// UNDO
	StartUndoRecording("Make door");

	// Make door
	MakeDoorFromSector (Selected->objnum);

	// UNDO
	StopUndoRecording();

	// SELECTION IS STILL VALID
	// Restore selection
	SetupSelection (FALSE);

	// Redraw map, info windows and status bar
	RefreshWindows();

End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscSMakeLift ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Make_lift_from_Sector);

	// Check at least one object selected
	if ( CheckSelection (1, 1) == FALSE )
		goto End;

	// UNDO
	StartUndoRecording("Make lift");

	// Make lift
	MakeLiftFromSector (Selected->objnum);

	// UNDO
	StopUndoRecording();

	// SELECTION IS STILL VALID
	// Restore selection
	SetupSelection (FALSE);

	// Redraw map, info windows and status bar
	RefreshWindows();

End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscSDitribFloor ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Distribute_Sector_floor_heights);

	// Check at least 3 object selected
	if ( CheckSelection (3, -1) == FALSE )
		goto End;

	// UNDO
	StartUndoRecording("Distribute floor");

	// Distribute floors
	DistributeSectorFloors (Selected);

	// UNDO
	StopUndoRecording();

	// SELECTION IS STILL VALID
	// Restore selection
	SetupSelection (FALSE);

	// Redraw map, info windows and status bar
	RefreshWindows();

End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscSDitribCeiling ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Distributes_Sector_ceiling_heights);

	// Check at least 3 object selected
	if ( CheckSelection (3, -1) == FALSE )
		goto End;

	// UNDO
	StartUndoRecording("Distribute ceiling");

	// Distribute floors
	DistributeSectorCeilings (Selected);

	// UNDO
	StopUndoRecording();

	// SELECTION IS STILL VALID
	// Restore selection
	SetupSelection (FALSE);

	// Redraw map, info windows and status bar
	RefreshWindows();

End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmWindowInfoBar ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	InfoShown = !InfoShown;

	// Toggle the status bar visibility
	TMainFrame *mainFrame =
		TYPESAFE_DOWNCAST (GetApplication()->GetMainWindow(), TMainFrame);
	mainFrame->ToggleStatusBar();
	SetupMenu();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmWindowToolBar ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	// Toggle the status bar visibility
	TMainFrame *mainFrame =
		TYPESAFE_DOWNCAST (GetApplication()->GetMainWindow(), TMainFrame);
	mainFrame->ToggleControlBar();
	SetupMenu();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
// Change the current editing level number and save it
//
void TEditorClient::CmFileSaveAs ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(EDITOR_Save_as);
	SaveLevel();
	DrawStatusBar();
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
// Save the current editing level
//
void TEditorClient::CmFileSave ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	if ((0 == RoomID) && NumThings > 2)
	{
		MessageBox("Please set Room ID from File menu before saving","Error Saving",MB_OK);
		return;
	}
	SET_HELP_CONTEXT(EDITOR_Save);
	SaveLevelData(LevelName);
	
   // Don't delete this data on saving.
   // Forget all UNDO datas
	//CleanupUndo();
	//InitUndo();

	DrawStatusBar();
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
//
void TEditorClient::CmUndo ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	// Check there's an UNDO entry. Normaly, this should never
	// be true, because we use an command enabler to disable the
	// UNDO menu item, but it seems there are cases when the command
	// enabler is not called fast enough to disable the item.
	if ( GetUndoName() == NULL )
		return;

	Undo();

	// Forget selection and redraw all
	CurObject = -1;
	ForgetSelection(&Selected);
	RefreshWindows();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
//
void TEditorClient::CmRedo ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	// Check there's a REDO entry. Normaly, this should never
	// be true, because we use an command enabler to disable the
	// REDO menu item, but it seems there are cases when the command
	// enabler is not called fast enough to disable the item.
	if ( GetRedoName() == NULL )
		return;

	Redo();

	// Forget selection and redraw all
	CurObject = -1;
	ForgetSelection(&Selected);
	RefreshWindows();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
//
void TEditorClient::CmEditPreferences ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Preferences);
	TPreferencesDialog(this).Execute();
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
//
void TEditorClient::CmEditDelete ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	// Do nothing if nothing selected!
	if ( Selected == NULL && CurObject == -1 )
		return;

	SET_HELP_CONTEXT(Delete_objects);

	if ( EditMode == OBJ_THINGS ||
		 Expert ||
		 Confirm ("Do you really want to delete %s ?\nThis will also delete "
				  "the eventual objects bound to %s.",
				  Selected ? "these objects" : "this object",
				  Selected ? "them" : "it") )
	{
		// UNDO
		StartUndoRecording(AddObjectType ("Delete"));

		if (Selected)
			DeleteObjects (EditMode, &Selected);
		else
			DeleteObject (EditMode, CurObject);

		// UNDO
		StopUndoRecording();

		// No selection to prevent invalid object numbers
		assert (Selected == NULL);
		CurObject = -1;

		// Redraw map, info windows and status bar
		RefreshWindows();
	}

	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
//
void TEditorClient::CmEditCopy ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	// Do nothing if nothing selected!
	if ( Selected == NULL && CurObject == -1 )
		return;

	SET_HELP_CONTEXT(Copy_objects);

	// Copy the selected object(s)
	if (Selected == NULL)
	   SelectObject (&Selected, CurObject);

	// UNDO
	StartUndoRecording(AddObjectType("Copy"));

	// Copy objects: The Selection list is updated with new objects
	CopyObjects (EditMode, Selected);

	// UNDO
	StopUndoRecording();

	assert (Selected != NULL);
	CurObject = Selected->objnum;

	// Redraw map, info windows, status bar and highlight selected objects
	RefreshWindows();
	/*
	PasteMode = TRUE;
	SelectCursor (0, ::LoadCursor (IDC_PASTE));
	*/

	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//   Insert an object at pointer location
void TEditorClient::CmEditAdd ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Add_object);

	SelPtr cur;

	// first special case: if several Vertices are selected, add new LineDefs
	if (EditMode == OBJ_VERTEXES && Selected != NULL && Selected->next != NULL)
	{
		// UNDO
		StartUndoRecording("Add LineDefs");

		SHORT firstv;

		if (Selected->next->next != NULL)		firstv = Selected->objnum;
		else                             		firstv = -1;

		// create LineDefs between the Vertices
		for (cur = Selected; cur->next != NULL; cur = cur->next)
		{

			// check if there is already a LineDef between the two Vertices
			for (CurObject = 0; CurObject < NumLineDefs; CurObject++)
			{
				LineDef *pCurLineDef = &LineDefs[CurObject];
				SHORT start = pCurLineDef->start;
				SHORT end   = pCurLineDef->end;

				if ( (start == cur->next->objnum && end   == cur->objnum) ||
					 (end   == cur->next->objnum && start == cur->objnum) )
					break;
			}

			// If no LineDef, add one
			if (CurObject >= NumLineDefs)
			{
				InsertObject (OBJ_LINEDEFS, -1, 0, 0);
				CurObject = NumLineDefs - 1;
				LineDefs[CurObject].start = cur->next->objnum;
				LineDefs[CurObject].end   = cur->objnum;
			}
			cur->objnum = CurObject;
		}

		// If SHIFT key pressed and if there are more than 2 Vertices,
		// close the polygon
		if (firstv >= 0 && (GetAsyncKeyState(VK_SHIFT) & 0x8000))
		{
			// check if there is already a LineDef between the two Vertices
			for (CurObject = 0; CurObject < NumLineDefs; CurObject++)
			{
				LineDef *pCurLineDef = &LineDefs[CurObject];
				SHORT start = pCurLineDef->start;
				SHORT end   = pCurLineDef->end;

				if ( (start == firstv && end   == cur->objnum) ||
					 (end   == firstv && start == cur->objnum) )
					break;
			}

			// Close the polygon
			if (CurObject >= NumLineDefs)
			{
				InsertObject( OBJ_LINEDEFS, -1, 0, 0);
				CurObject = NumLineDefs - 1;
				LineDefs[CurObject].start = firstv;
				LineDefs[CurObject].end   = cur->objnum;
				cur->objnum = CurObject;
			}
			cur->objnum = CurObject;
		}
		else
			UnSelectObject (&Selected, cur->objnum);

		// Switch to LineDef mode
		EditMode = OBJ_LINEDEFS;
		SetupMode();
		SetupMenu();

		// UNDO
		StopUndoRecording();
	}

	// second special case: if several LineDefs are selected,
	// add new SideDefs and one Sector
	else if (EditMode == OBJ_LINEDEFS && Selected != NULL)
	{
		// UNDO
		StartUndoRecording("Add Sector");

		for (cur = Selected; cur; cur = cur->next)
		{
			LineDef *pLineDef = &LineDefs[cur->objnum];
			if (pLineDef->sidedef1 >= 0  &&  pLineDef->sidedef2 >= 0)
			{
				Beep();
				Notify ("Error: Cannot add the new Sector:\nLineDef #%d "
						"already has two SideDefs.", cur->objnum);
				break;
			}
		}

		// If LineDefs OK, add sector
		if (cur == NULL)
		{
			// Add sector
			InsertObject (OBJ_SECTORS, -1, 0, 0);
			CurObject = NumSectors - 1;

			// Add new SideDef to selected LineDefs
			for (cur = Selected; cur; cur = cur->next)
			{
				//BUG(NOT) can keep it here since no LineDefs inserted
				LineDef *pLineDef = &LineDefs[cur->objnum];

				InsertObject (OBJ_SIDEDEFS, -1, 0, 0);
				SideDefs[NumSideDefs - 1].sector = CurObject;

				// If first SideDef exists, Add second SideDef
				if (pLineDef->sidedef1 >= 0)
				{
					SHORT s;

					// Copy sector data
					s = SideDefs[pLineDef->sidedef1].sector;
					if (s >= 0)
						Sectors[CurObject] = Sectors[s];

					pLineDef->sidedef2 = NumSideDefs - 1;

					pLineDef->flags &= ~0x01;		// Not impassable
					pLineDef->flags |= 0x04;		// Two-sided

					// Remove normal textures, because the LineDef
					// is now Two-sided
					char texname[9];
					memset (texname, 0, 9);
					texname[0] = '-';
					memcpy (SideDefs[NumSideDefs - 1].tex3, texname, MAX_BITMAPNAME);
					memcpy (SideDefs[pLineDef->sidedef1].tex3,texname, MAX_BITMAPNAME);
				}
				else
					// Add first SideDef
					pLineDef->sidedef1 = NumSideDefs - 1;
			} // End For

			// Unselect LineDefs
			ForgetSelection (&Selected);
			// Select new sector
			SelectObject (&Selected, CurObject);

			// Siwtch to Sector mode
			EditMode = OBJ_SECTORS;
			SetupMode();
			SetupMenu();
		}

		// UNDO
		StopUndoRecording();
	}

	// Normal case: add a new object of the current type
	else
	{
		SHORT mapx = MAPX(PointerX);
		SHORT mapy = MAPY(PointerY);

		// Forget selection
		ForgetSelection (&Selected);

		// UNDO
		StartUndoRecording(AddObjectType("Add"));

		// Snap pointer position to grid
		if (GridScale > 0 && SnapToGrid)
		{
			mapx = (mapx + GridScale / 2) & ~(GridScale - 1);
			mapy = (mapy + GridScale / 2) & ~(GridScale - 1);
		}

		// Insert new object at pointer position
		InsertObject (EditMode, CurObject, mapx, mapy);
		CurObject = GetMaxObjectNum(EditMode);

		// If in LineDef mode, ask 2 vertex numbers
		if (EditMode == OBJ_LINEDEFS)
		{
			if ( NumVertexes < 2 )
			{
				Notify ("You need a least 2 vertices to create a LineDef");
				DeleteObject (EditMode, CurObject);
				CurObject = -1;
			}
			else
			{
				char Title[80];
				char Prompt[80];
				TRangeValidator *pValid1 = new TRangeValidator (0, NumVertexes - 1);
				TRangeValidator *pValid2 = new TRangeValidator (0, NumVertexes - 1);
				static char Input1[6], Input2[6];   // keep in memory between calls

				wsprintf (Title, "Choose the origin and destinatation of LineDef");
				wsprintf (Prompt, "Enter two vertices number (between 0 and %d):",
								  NumVertexes-1);

				if ( TInput2Dialog(this, Title, Prompt,
								   Input1, 6,
								   Input2, 6,
								   pValid1, pValid2).Execute() == IDOK )
				{
					LineDefs[CurObject].start = atoi(Input1);
					LineDefs[CurObject].end   = atoi(Input2);
				}
				else
				{
					DeleteObject (EditMode, CurObject);
					CurObject = -1;
				}
			}
		}

		// Auto merge vertices
		else if (EditMode == OBJ_VERTEXES)
		{
			SelectObject (&Selected, CurObject);
			AutoMergeVertices (&Selected);
			ForgetSelection (&Selected);
		}

		// UNDO
		StopUndoRecording();
	}

	// Redraw map, status bar and info windows
	RefreshWindows ();

	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmCheckNumbers ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Display_statistics);
	// Statistics();
	TStatisticsDialog(this).Execute();
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmCheckSectors ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Check_all_sectors_are_closed);

	// UNDO
	StartUndoRecording("Check Sectors");

	// Check all sectors are closed.
	// No changes are made to the level
	if ( CheckSectors() )
	{
		// Redraw map to go to eventual bad object
		RefreshWindows();
	}

	// UNDO
	StopUndoRecording();

	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmCheckCrossRefs ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Check_all_cross_references);

	// UNDO
	StartUndoRecording("Check cross ref.");

	// Check for invalid and unused objects
	// LEVEL CHANGES: Some Vertices, Sectors and/or LineDefs may be removed
	if ( CheckCrossReferences() )
	{
		// Remove selection to prevent invalid selected objects
		ForgetSelection (&Selected);
		CurObject = -1;

		// Redraw map
		RefreshWindows ();
	}

	// UNDO
	StopUndoRecording();

	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmCheckTextures ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Check_for_missing_textures);

	// UNDO
	StartUndoRecording("Check missing");

	// Look for missing textures names
	// LEVEL CHANGES: Some missing names may be set to default names
	if ( CheckTextures() )
	{
		// Redraw map to go to the invalid object
		RefreshWindows ();
	}

	// UNDO
	StopUndoRecording();

	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmCheckNames ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Check_textures_names);

	// UNDO
	StartUndoRecording("Check names");

	// Check that textures names are valid
	// LEVEL CHANGES: Some invalid textures names may be set to default names
	if ( CheckTextureNames() )
	{
		// Redraw map to go to the invalid object
		RefreshWindows ();
	}

	// UNDO
	StopUndoRecording();

	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmZoomIn ()
{
	float oldScale = Scale;

#ifdef DEU_FOLLOW_POINTER
	OrigX += (SHORT) ((PointerX - ScrCenterX) * DIV_SCALE);
	OrigY += (SHORT) ((ScrCenterY - PointerY) * DIV_SCALE);
#endif

	IncScale();

#ifdef DEU_FOLLOW_POINTER
	OrigX -= (SHORT) ((PointerX - ScrCenterX) * DIV_SCALE);
	OrigY -= (SHORT) ((ScrCenterY - PointerY) * DIV_SCALE);
#endif

	// If scale has really changed
	if ( Scale != oldScale )
	{
		AdjustScroller();       // Adjust scroller units
		RefreshWindows();     	// Redraw map
	}
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmZoomOut ()
{
	float oldScale = Scale;

#ifdef DEU_FOLLOW_POINTER
	OrigX += (SHORT) ((PointerX - ScrCenterX) * DIV_SCALE);
	OrigY += (SHORT) ((ScrCenterY - PointerY) * DIV_SCALE);
#endif

	DecScale();

#ifdef DEU_FOLLOW_POINTER
	OrigX -= (SHORT) ((PointerX - ScrCenterX) * DIV_SCALE);
	OrigY -= (SHORT) ((ScrCenterY - PointerY) * DIV_SCALE);
#endif

	// If scale has really changed
	if ( Scale != oldScale )
	{
		AdjustScroller();       // Adjust scroller units
		RefreshWindows () ;     // Redraw map
	}
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmSlowScroll ()
{
	TMenu menu (GetApplication()->GetMainWindow()->GetMenu());
	SlowScroll = !SlowScroll;

	menu.CheckMenuItem (CM_WINDOW_SLOW_SCROLL, MF_BYCOMMAND |
						(SlowScroll ? MF_CHECKED : MF_UNCHECKED));

	// Adjust scroller range (since scroll steps changes)
	AdjustScroller();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmAutoLayout ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	AutoLayout = ! AutoLayout;
	SetupMenu();

	// Layout info windows
	if ( AutoLayout )
		Layout();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmLayout ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	Layout();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmWindowGridNext ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	if (GridScale == 0)
		GridScale = 8;
	else if (GridScale < 256)
		GridScale *= 2;
	else
		GridScale = 0;

	if ( GridShown )
		Invalidate();
	DrawStatusBar();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmWindowGridPrev ()
{
	if (GridScale == 0)
		GridScale = 256;
	else if (GridScale > 8)
		GridScale /= 2;
	else
		GridScale = 0;

	if ( GridShown )
		Invalidate();
	DrawStatusBar();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmSnapToGrid ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SnapToGrid = !SnapToGrid;
	DrawStatusBar();	// Draw Snap to grid sign (*)
	SetupMenu();
}



/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmShowGrid ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	GridShown = !GridShown;
	SetupMenu();

	// Show or hide grid
	if ( GridScale != 0 )
		Invalidate();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmInfoWin ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	InfoWinShown = !InfoWinShown;

	SetupMode ();

	/*
	if ( !InfoWinShown )
		RefreshWindows();
	*/
}



/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmCenterMap ()
{
	// Center map
	CenterMapAroundCoords ((MapMinX + MapMaxX) / 2, (MapMinY + MapMaxY) / 2);

	// Zoom out until everything is visible
	SetScale (ScaleMax);
	while ( Scale > ScaleMin )
	{
		if ( MAPX(0) <= MapMinX && MAPX(ScrMaxX) >= MapMaxX &&
			 // Vertical axe is reversed !
			 MAPY(0) >= MapMaxY && MAPY(ScrMaxY) <= MapMinY )
		{
			break;
		}

		float OldScale = Scale;
		DecScale();

		// If scale didn't change, the Scale is minimal
		if ( OldScale == Scale )
			break;
	}

	// Refresh window
	Invalidate();			// Redraw map
	AdjustScroller();      	// Adjust scroller units
	DrawStatusBar();		// Draw new scale
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmEditObject ()
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Edit_objects);
	EditCurObject();
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::DoPopupMenu (HMENU hMenu, UINT popupNumb)
{
	TMenu menu (hMenu);
	TPopupMenu PopupMenu (menu.GetSubMenu (popupNumb));

	if ( menu.IsOK() && PopupMenu.IsOK() )
	{
		TPoint Center(ScrCenterX, ScrCenterY);
		ClientToScreen (Center);
		PopupMenu.TrackPopupMenu (TPM_CENTERALIGN,
								  Center,
								  0 /* reserved */,
								  *Parent);
	}
	else
	{
		Beep();
		Notify ("Error: Couldn't create popup menu");
	}
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmMiscMenu ()
{
	DoPopupMenu (Parent->GetMenu(), 3);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmInsertMenu ()
{
	TMenu Menu (Parent->GetMenu());
	TPopupMenu PopupMenu (Menu.GetSubMenu(1));
	DoPopupMenu (PopupMenu, 6);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmCheckMenu ()
{
	DoPopupMenu (Parent->GetMenu(), 5);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmAlignXSD1Normal ()
{
	AlignX (1, 3);
}
/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmAlignXSD1Upper ()
{
	AlignX (1, 1);
}
/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmAlignXSD1Lower ()
{
	AlignX (1, 2);
}
/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmAlignXSD2Normal ()
{
	AlignX (2, 3);
}
/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmAlignXSD2Upper ()
{
	AlignX (2, 1);
}
/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmAlignXSD2Lower ()
{
	AlignX (2, 2);
}

/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmAlignXCircleSD1Normal()
{
   AlignXCircle(1, 3);
}
/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmAlignXCircleSD1Upper()
{
   AlignXCircle(1, 1);
}
/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmAlignXCircleSD1Lower()
{
   AlignXCircle(1, 2);
}
/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmAlignXCircleSD2Normal()
{
   AlignXCircle(2, 3);
}
/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmAlignXCircleSD2Upper()
{
   AlignXCircle(2, 1);
}
/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmAlignXCircleSD2Lower()
{
   AlignXCircle(2, 2);
}

/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::AlignXCircle(SHORT sdType, SHORT texType)
{
   // Ignore if "insert object" mode
   if (InsertingObject)
      return;

   // Check only one object selected
   if (CheckSelection(1, 1) == FALSE)
   {
      Notify("Select only one linedef!");
      return;
   }

   SET_HELP_CONTEXT(Align_textures_X_circle);

   // UNDO
   StartUndoRecording("Align circle X offsets");

   // Step 1. Build up a selection of the circle linedefs, making sure
   // we actually have a circle. At some point the end of a linedef should
   // equal the start of the first linedef. If we can't find another linedef
   // starting from the end point of the previous one, we don't have a
   // valid circle and can exit.
   SelPtr ldlist = NULL, sdlist = NULL, cur;
   SHORT i;
   LineDef *pCheck = &LineDefs[Selected->objnum];
   SHORT start_vertex = pCheck->start;
   SHORT end_vertex = pCheck->end;
   BOOL bFound;
   int num_iterations = 0;
   // Add selected linedef to new list.
   SelectObject(&ldlist, Selected->objnum);

   while (end_vertex != start_vertex)
   {
      bFound = false;
      // Iterate through the room's linedefs, finding the one that
      // starts at the end of the previous one.
      for (i = 0; i < NumLineDefs; ++i)
      {
         pCheck = &LineDefs[i];
         if (pCheck->start == end_vertex)
         {
            SelectObject(&ldlist, i);
            end_vertex = pCheck->end;
            bFound = true;
            break;
         }
      }

      // If we couldn't find another linedef starting at the end of the
      // previous one, we likely haven't selected a circle.
      if (!bFound)
      {
         Notify("Selected linedef not part of a valid circle!");
         goto End;
      }
      if (++num_iterations > 1024)
      {
         Notify("Too many linedefs in circle!");
         goto End;
      }
   }

   // Step 2: Now have all the linedefs in the circle selected.
   // Select each sidedef based on which option we were given.
   for (cur = ldlist; cur; cur = cur->next)
   {
      LineDef *pLineDef = &LineDefs[cur->objnum];

      if (sdType == 1 && pLineDef->sidedef1 >= 0)
         SelectObject(&sdlist, pLineDef->sidedef1);
      else if (sdType == 2 && pLineDef->sidedef2 >= 0)
         SelectObject(&sdlist, pLineDef->sidedef2);
   }

   // If no sidedefs in list, exit.
   if (!sdlist)
   {
      Notify("No sidedefs found in circle!");
      goto End;
   }

   // Step 3: Call AlignTexturesX with the given side,
   // texture type and sidedef list.
   AlignTexturesX(&sdlist, sdType, 0, 1, texType);

   // Clean up sidedef list separately before end.
   ForgetSelection(&sdlist);

End:
   ForgetSelection(&ldlist);
   // UNDO
   StopUndoRecording();
   // SELECTION IS STILL VALID
   // Restore selection
   SetupSelection(FALSE);
   // Redraw map, info windows and status bar
   RefreshWindows();
   RESTORE_HELP_CONTEXT();
}

/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::AlignX (SHORT sdType, SHORT texType)
{
	// Ignore if "insert object" mode
	if ( InsertingObject )
		return;

	SET_HELP_CONTEXT(Align_textures_X_offset);

	// Check at least one object selected
	if ( CheckSelection (1, -1) == FALSE )
		goto End;

	// UNDO
	StartUndoRecording("Align X offset");

	SelPtr sdlist, cur;

	// select all SideDefs
	sdlist = NULL;
	for (cur = Selected; cur; cur = cur->next)
	{
		LineDef *pLineDef = &LineDefs[cur->objnum];

		if (pLineDef->sidedef1 >= 0)
		   SelectObject (&sdlist, pLineDef->sidedef1);

		if (pLineDef->sidedef2 >= 0)
		   SelectObject (&sdlist, pLineDef->sidedef2);
	}

	// align the textures along the X axis (height)
	// ARK:  Never check for same texture; always ask for initial offset
	AlignTexturesX (&sdlist, sdType, 0, 1, texType);
	ForgetSelection (&sdlist);

	// UNDO
	StopUndoRecording();

	// SELECTION IS STILL VALID
	// Restore selection
	SetupSelection (FALSE);

	// Redraw map, info windows and status bar
	RefreshWindows();

End:
	RESTORE_HELP_CONTEXT();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmHelpLevelEditor ()
{
	WinDEUApp *app = TYPESAFE_DOWNCAST(GetApplication(), WinDEUApp);
	app->AppHelp(HELP_CONTEXT, Level_editor);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmHelpKeyboard ()
{
	WinDEUApp *app = TYPESAFE_DOWNCAST(GetApplication(), WinDEUApp);
	app->AppHelp(HELP_CONTEXT, KEYBOARD_REF);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmHelpMouse ()
{
	WinDEUApp *app = TYPESAFE_DOWNCAST(GetApplication(), WinDEUApp);
	app->AppHelp(HELP_CONTEXT, Mouse_Commands);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//  Small function to add the object type name (depending on the
//  edit mode and number of selected objects) to a string
//	This function is used with StartUndoRecording() to get
//  a significant string corresponding to the UNDO/REDO function
//
const char *TEditorClient::AddObjectType(const char *str)
{
	static char msg[50];

	strcpy (msg, str);
	strcat (msg, " ");
	if ( Selected != NULL && Selected->next != NULL )
		strcat (msg, GetObjectsTypeName(EditMode));
	else
		strcat (msg, GetObjectTypeName(EditMode));

	return (msg);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmHorizDownLeft ()
{
	InfoWinPos = INFOWIN_HORIZ_DOWNLEFT;
	LayoutInfoWindows();
	SetupMenu();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmHorizDownRight ()
{
	InfoWinPos = INFOWIN_HORIZ_DOWNRIGHT;
	LayoutInfoWindows();
	SetupMenu();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmHorizUpLeft ()
{
	InfoWinPos = INFOWIN_HORIZ_UPLEFT;
	LayoutInfoWindows();
	SetupMenu();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmHorizUpRight ()
{
	InfoWinPos = INFOWIN_HORIZ_UPRIGHT;
	LayoutInfoWindows();
	SetupMenu();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmVertDownLeft ()
{
	InfoWinPos = INFOWIN_VERT_DOWNLEFT;
	LayoutInfoWindows();
	SetupMenu();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmVertDownRight ()
{
	InfoWinPos = INFOWIN_VERT_DOWNRIGHT;
	LayoutInfoWindows();
	SetupMenu();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmVertUpLeft ()
{
	InfoWinPos = INFOWIN_VERT_UPLEFT;
	LayoutInfoWindows();
	SetupMenu();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmVertUpRight ()
{
	InfoWinPos = INFOWIN_VERT_UPRIGHT;
	LayoutInfoWindows();
	SetupMenu();
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmLayoutEnable (TCommandEnabler &tce)
{
	tce.Enable (InfoWinShown);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmDeleteEnable (TCommandEnabler &tce)
{
	tce.Enable (Selected != NULL || CurObject >= 0);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
//
void TEditorClient::CmCopyEnable (TCommandEnabler &tce)
{
	tce.Enable (Selected != NULL || CurObject >= 0);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
// TODO: These two functions are almost identical!
void TEditorClient::CmUndoEnable (TCommandEnabler &tce)
{
	TMenu menu(GetApplication()->GetMainWindow()->GetMenu());

	// Get original menu name at first call
	static char OriginalName[50] ;
	if ( OriginalName[0] == '\0' )
	{
		char ItemString[50];
		unsigned int i;

		menu.GetMenuString (CM_EDIT_UNDO, ItemString,
							sizeof(ItemString), MF_BYCOMMAND);
		for(i = 0 ; i < strlen(ItemString) ; i++)
		{
			if ( ItemString[i] == '\t' )
				break;
		}

		strncpy (OriginalName, ItemString, i);
		OriginalName[i] = '\0';
		strcat (OriginalName, " [%s]");
		strcat (OriginalName, &ItemString[i]);
	}

	// Setup UNDO menu name
	char mname[80];
	char *uname = GetUndoName();

	if ( uname != NULL )
		wsprintf (mname, OriginalName, uname);
	else
		wsprintf (mname, OriginalName, "");

	// Change UNDO menu item name
	menu.ModifyMenu (CM_EDIT_UNDO, MF_BYCOMMAND | MF_STRING,
					 CM_EDIT_UNDO, mname);

	// We enable after modifying the menu item!!
	tce.Enable(uname != NULL);
}


/////////////////////////////////////////////////////////////////////
// TEditorClient
// -------------
// TODO: These two functions are almost identical!
void TEditorClient::CmRedoEnable (TCommandEnabler &tce)
{
	TMenu menu(GetApplication()->GetMainWindow()->GetMenu());

	// Get original menu name at first call
	static char OriginalName[50] ;
	if ( OriginalName[0] == '\0' )
	{
		char ItemString[50];
		unsigned int i;

		menu.GetMenuString (CM_EDIT_REDO, ItemString,
							sizeof(ItemString), MF_BYCOMMAND);
		for(i = 0 ; i < strlen(ItemString) ; i++)
		{
			if ( ItemString[i] == '\t' )
				break;
		}

		strncpy (OriginalName, ItemString, i);
		OriginalName[i] = '\0';
		strcat (OriginalName, " [%s]");
		strcat (OriginalName, &ItemString[i]);
	}

	// Setup UNDO menu name
	char mname[80];
	char *uname = GetRedoName();

	if ( uname != NULL )
		wsprintf (mname, OriginalName, uname);
	else
		wsprintf (mname, OriginalName, "");

	// Change UNDO menu item name
	menu.ModifyMenu (CM_EDIT_REDO, MF_BYCOMMAND | MF_STRING,
					 CM_EDIT_REDO, mname);

	// We enable after modifying the menu item!!
	tce.Enable(uname != NULL);
}

