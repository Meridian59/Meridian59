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

	FILE:         mapdc.h

	OVERVIEW
	========
	Class definition for TMapDC (TDC).
*/
#ifndef __mapdc_h
#define __mapdc_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef __gfx_h
	#include "gfx.h"		// Colors, ScrCenterX, OrigX, ...
#endif

#ifndef __objects_h
	#include "objects.h"	// SelPtr
#endif

#ifndef OWL_DC_H
	#include <owl\dc.h>
#endif

#ifndef OWL_GDIOBJEC_H
	#include <owl\gdiobjec.h>
#endif

#ifndef __editcli_h
	class TEditorClient;
#endif

//
// Class for pen (retains color, width and style in members)
//
class TPenColor16 : public TPen
{
public:	// data members
	int color;
	int width;
	UINT style;
	unsigned long HitCount;

public:	// function members
	 TPenColor16 (int _color, int _width = 1, UINT _style = PS_SOLID);
	~TPenColor16 ();
} ;


//
// Size of cache for 16 color pens
//
#define COLOR16_CACHE_SIZE	12


//
// DC class for drawing on the level map in editor
//
class TMapDC : public TDC
{
//
// Class members
//
private:
	static TPenColor16 **LastPens;

public:
	static void InitCacheData();
	static void _USERENTRY CleanupCacheData();
	static TPenColor16 *GetPenColor16 (int color, int width, UINT style);
	static TColor GetColor16 (int color);

//
// Object members
//
private:
	BOOL DoDelete;			// use black pen for every drawing
	int CurPenColor;
	int CurPenWidth;
	UINT CurPenStyle;
	HPEN hOldPen;
	BOOL FromWindow;		// DC created by GetDC(hWnd) ?
	TEditorClient *pEditor;

protected:
	void Init(TEditorClient *_pEditor);

	// Clipping and Origin selection
	void SetupOrg ();
	void SetupClipping ();
	void ExcludeClipWindow (TWindow *pWnd);
	void SetNewPenColor16 (int color, int width = 1, UINT style = PS_SOLID);
	void DrawLineDefLen (SHORT, SHORT, SHORT, SHORT, int color);

public:
	TMapDC (TEditorClient *_pEditor, TDC &dc);	// from other DC
	TMapDC (TEditorClient *_pEditor);			// client DC from window
	~TMapDC ();

	// Primitive of color selection
	void SetPenColor16 (int color, int width = 1, UINT style = PS_SOLID)
	{
		// Doesn't allow color selection if Delete mode
		if (DoDelete)	return;

		// Choose new pen only if different than current pen
		if ( CurPenColor != color ||
			 CurPenWidth != width ||
			 CurPenStyle != style )
		{
			SetNewPenColor16 (color, width, style);
		}
	}
	void SetTextColor16 (int color);
	void SetBkColor16 (int color);
	void SetDelete(BOOL doDelete)
	{
		if ( doDelete )	SetPenColor16 (BLACK, CurPenWidth, CurPenStyle);
		DoDelete = doDelete ;
	}

	// Primitive to draw on map
	void DrawMapLine (SHORT, SHORT, SHORT, SHORT);
	void DrawMapRect (SHORT, SHORT, SHORT, SHORT);
	void DrawMapCircle (SHORT, SHORT, SHORT);
	void DrawMapVector (SHORT, SHORT, SHORT, SHORT);
	void DrawMapArrow (SHORT, SHORT, USHORT, USHORT);

	// Draw an object, use editmode to select color and shape
	void DrawMapThing   (int editmode, SHORT tnum);
	void DrawMapLineDef (int editmode, SHORT ldnum, BOOL DrawLen = FALSE);
	void DrawMapVertex  (int editmode, SHORT vnum);
	void DrawMapSector  (int editmode, SHORT snum);

    int GetLineDefColor (int editmode, SHORT ldnum);
	void DrawMapLineDefLen (int editmode, SHORT ldnum);

	// Draw a list of objects of a type
	void DrawMapThingsList   (int editmode, SelPtr list);
	void DrawMapLineDefsList (int editmode, SelPtr list, BOOL DrawLen = FALSE);
	void DrawMapVertexesList (int editmode, SelPtr list);
	void DrawMapSectorsList  (int editmode, SelPtr list);

	// Draw all objects of a type
	void DrawMapThings   (int editmode);
	void DrawMapLineDefs (int editmode, BOOL DrawLen = FALSE);
	void DrawMapVertexes (int editmode);
	void DrawMapSectors  (int editmode);

	// Draw the grid on the map
	void DrawMapGrid (SHORT grid);

	// Draw all map
	void DrawMap (int editmode, SHORT grid, BOOL drawgrid);
} ;


#endif		// __mapdc_h