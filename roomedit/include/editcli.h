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

	FILE:         editcli.h

	OVERVIEW
	========
	Class definition for TEditorClient (TLayoutWindow).
*/
#if !defined(__editcli_h)              // Sentry, use file only if it's not already included.
#define __editcli_h


#ifndef __common_h
	#include "common.h"
#endif

#ifndef __objects_h
	#include "objects.h"	// SelPtr
#endif

#ifndef OWL_LAYOUTWI_H
	#include <owl\layoutwi.h>
#endif

#ifndef OWL_STATUSBA_H
	#include <owl/statusba.h>
#endif

#ifndef OWL_OPENSAVE_H
	#include <owl\opensave.h>
#endif

#ifndef __mapdc_h
	class TMapDC;
#endif

#ifndef __infocont_h
	class TInfoControl;
#endif

#ifndef __editscro_h
	class TEditScroller;
#endif


//{{TLayoutWindow = TEditorClient}}
class TEditorClient : public TLayoutWindow
{
	friend TMapDC;			// Class for level map DC
	friend TEditScroller;   // Class for Editor window scroller
private:
	// Data to control open/saveas standard dialog.
	TOpenSaveDialog::TData FileData;

	// Level number
	char LevelName[500];

	// Editing mode
	int    EditMode;

	// Current selected object
	SHORT  CurObject;

	// Drag info.
	BOOL   DragObject;
	BOOL   DragMoved;		// Did the user moved when dragging?
	SelPtr MovingLineDefs;	// Moving LineDefs when in drag mode

	// Insert object mode flag
	BOOL InsertingObject;

	// Information windows
	BOOL InfoWinShown;
	BOOL AutoLayout;		// Automatic Layout of info windows ?
	TInfoControl *pThingInfo;
	TInfoControl *pVertexInfo;
	TInfoControl *pLineDefInfo;
	TInfoControl *pSideDef1Info;
	TInfoControl *pSideDef2Info;
	TInfoControl *pSectorInfo;

	// Selection info.
	SelPtr Selected;

	// Selection box info.
	BOOL   StretchSelBox;
	BOOL   StretchMoved;	// Did the user moved when stretching?
	SHORT  SelBoxX, SelBoxY;
	SHORT  OldSelBoxX, OldSelBoxY;

	// Displaying Rulers ?
	BOOL   ShowRulers;

	// Mainframe status bar
	TStatusBar *pStatusBar ;

private:
	void RefreshWindows ();
	void SetupMode ();
	void SetupMenu ();
	void SetupInfoWindows ();
	void LayoutInfoWindows ();
	void ChangeMode(int NewMode);
	void AdjustScroller ();
	void DrawMouseCoord (TDC &dc);
	void SetupSelection (BOOL SaveSel);
	BOOL CheckSelection (SHORT min, SHORT max);
   void AlignX (SHORT sdType, SHORT texType);
   void AlignXCircle(SHORT sdType, SHORT texType);

	void EditCurObject();
	void CenterMapAroundCoords (SHORT xpos, SHORT ypos);
	void DrawStatusBar ();
	void DrawStretchSelBox (TMapDC &dc);
	void DrawMovingObjects (TMapDC &dc, BOOL doDelete = FALSE);
	void SelectMovingLineDefs ();
	void DisplayObjectInfo (int objtype, SHORT objnum);
	BOOL CheckStartingPos ();
	BOOL SaveLevel ();
	BOOL GetWadSaveFileName (char *filename, char *levelname);
	void DoPopupMenu (HMENU hMenu, UINT popupNumb);

	const char *AddObjectType(const char *);

public:
	TEditorClient (TWindow* parent, char *_LevelName, BOOL newLevel,
				   const char* title = 0, TModule* module = 0);
	virtual ~TEditorClient ();
	BOOL SaveChanges ();


//{{TEditorClientVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
	virtual void Paint (TDC& dc, bool erase, TRect& rect);
	virtual char* GetClassName ();
	virtual void GetWindowClass (WNDCLASS& wndClass);
	virtual void CloseWindow (int retVal = 0);
	virtual void Destroy (int retVal = 0);
//{{TEditorClientVIRTUAL_END}}

//{{TEditorClientRSP_TBL_BEGIN}}
protected:
	void EvChar (UINT key, UINT repeatCount, UINT flags);
	void EvKeyDown (UINT key, UINT repeatCount, UINT flags);	
	void EvSize(UINT sizeType, const TSize& size);
	void EvMouseMove(UINT modKeys, const TPoint& point);
	void EvLButtonDown (UINT modKeys, const TPoint& point);
	void EvLButtonUp (UINT modKeys, const TPoint& point);
	void EvLButtonDblClk (UINT modKeys, const TPoint& point);
	void EvRButtonDown (UINT modKeys, const TPoint& point);
	void CmFileQuit ();
	void CmSearchPrev ();
	void CmSearchNext ();
	void CmSearchJump ();
	void CmObjectsRectangle ();
	void CmObjectsPolygon ();
   void CmObjectsCircle();
   void CmObjectsTorch();
	void CmModeThings ();
	void CmModeVertexes ();
	void CmModeLinedefs ();
	void CmModeSectors ();
	void CmModeNext ();
	void CmModePrev ();
	void CmMiscFindTag ();
	void CmMiscRotateScale ();
	void CmMiscLDSplitAdd ();
	void CmMiscLDSplitSector ();
	void CmMiscLDDelete ();
	void CmMiscLDFlip ();
	void CmMiscLDSwap ();
	void CmMiscLDAlignY ();

	void CmAlignXSD1Normal ();
	void CmAlignXSD1Upper ();
	void CmAlignXSD1Lower ();
	void CmAlignXSD2Normal ();
	void CmAlignXSD2Upper ();
	void CmAlignXSD2Lower ();

   void CmAlignXCircleSD1Normal();
   void CmAlignXCircleSD1Upper();
   void CmAlignXCircleSD1Lower();
   void CmAlignXCircleSD2Normal();
   void CmAlignXCircleSD2Upper();
   void CmAlignXCircleSD2Lower();

	void CmMiscVDelete ();
	void CmMiscVMerge ();
	void CmMiscVAddLineDef ();
	void CmMiscSMakeDoor ();
	void CmMiscSMakeLift ();
	void CmMiscSDitribFloor ();
	void CmMiscSDitribCeiling ();
	void CmFileSaveAs ();
	void CmFileSave ();
	void CmEditPreferences ();
	void CmEditDelete ();
	void CmEditCopy ();
	void CmEditAdd ();
	void CmCheckTextures ();
	void CmCheckSectors ();
	void CmCheckNumbers ();
	void CmCheckNames ();
	void CmCheckCrossRefs ();
	void CmMiscMenu ();
	void CmInsertMenu ();
	void CmCheckMenu ();
	void CmZoomIn ();
	void CmZoomOut ();
	void CmAutoLayout ();
	void CmLayout ();
	void CmWindowGridNext ();
	void CmWindowGridPrev ();
	void CmSnapToGrid ();
	void CmShowGrid ();
	void CmInfoWin ();
	void CmCenterMap ();
	void CmEditObject ();
	void CmHelpLevelEditor ();
	void CmHelpKeyboard ();
	void CmHelpMouse ();
	void CmSlowScroll ();
	void CmWindowInfoBar ();
	void CmWindowToolBar ();
	void CmUndo ();
	void CmRedo ();
	void CmHorizDownLeft ();
	void CmHorizDownRight ();
	void CmHorizUpLeft ();
	void CmHorizUpRight ();
	void CmVertDownLeft ();
	void CmVertDownRight ();
	void CmVertUpLeft ();
	void CmVertUpRight ();
	void CmDeleteEnable (TCommandEnabler &tce);
	void CmCopyEnable (TCommandEnabler &tce);
	void CmUndoEnable (TCommandEnabler &tce);
	void CmRedoEnable (TCommandEnabler &tce);
	void CmLayoutEnable (TCommandEnabler &tce);
	//{{TEditorClientRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TEditorClient);
};    //{{TEditorClient}}


#endif                                      // __editcli_h sentry.

