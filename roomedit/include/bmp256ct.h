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

	FILE:         bmp256ct.h

	OVERVIEW
	========
	Class definition for TBitmap256Control (TControl).
*/
#if !defined(__bmp256ct_h)              // Sentry, use file only if it's not already included.
#define __bmp256ct_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_CONTROL_H
	#include <owl\control.h>
#endif

#ifndef OWL_DC_H
	class _OWLCLASS TDC;
#endif


//{{TControl = TBitmap256Control}}
class TBitmap256Control : public TControl
{
private:
	TPalette     *pBitmapPalette;
	PALETTEENTRY *pDoomPaletteEntries;
	int           NumDoomPalette;
	int           MaxWidth ;
	int           MaxHeight;

protected:
	USHORT     BitmapXSize;
	USHORT     BitmapYSize;
	BYTE      *pBitmapData;
	BOOL      *pUsedColors;
	BYTE      *pEntryMapping;
	BYTE       NbEntries;
	UINT       ZoomFactor;
	UINT       GammaLevel;
	char       BitmapName[10];

	// DIB info
	LPBITMAPINFO pDIBInfo;
	BYTE        *pDIBits;

protected:
	// pure Virtual function redefined for things, walls, floors/celings
	virtual void BuildBitmapData (const char *name, SHORT remapPlayer = 0) = 0;

	void DisplayBitmap (TDC &dc, TRect &rect);
	void BuildBitmapPalette (BOOL CreateNewPal = TRUE);
	void BuildBitmap ();
	void ConvertBitmapToDib();
	void LoadDoomPalette (int playpalnum);
	void AdjustWindowSize ();
	void AdjustScroller ();
	ULONG DataOfs(short x, short y)
	{
		assert (x >= 0);
		assert (x < BitmapXSize);
		assert (y >= 0);
		assert (y < BitmapYSize);
		return (ULONG)y * BitmapXSize + x;
	}
	void GammaAdjustEntry (PALETTEENTRY &entry)
	{
		entry.peRed   = (BYTE)min (entry.peRed   + GammaLevel, 255);
		entry.peGreen = (BYTE)min (entry.peGreen + GammaLevel, 255);
		entry.peBlue  = (BYTE)min (entry.peBlue  + GammaLevel, 255);
	}

public:
	TBitmap256Control (TWindow* parent, int id, const char* title,
					   int x, int y, int w, int h, TModule* module = 0);
	virtual ~TBitmap256Control ();

	void SelectBitmap2 (const char *name, SHORT remap = 0, int palnum = 0);
	void SetZoomFactor (UINT factor);
	UINT GetZoomFactor() 	{ return ZoomFactor; }
	void SetGammaLevel (BYTE level);
	UINT GetGammaLevel ()	{ return GammaLevel; }

//{{TBitmap256ControlVIRTUAL_BEGIN}}
public:
	virtual void Paint (TDC& dc, bool erase, TRect& rect);
	virtual char* GetClassName ();
	virtual void GetWindowClass (WNDCLASS& wndClass);
	virtual void SetupWindow ();
//{{TBitmap256ControlVIRTUAL_END}}


//{{TBitmap256ControlRSP_TBL_BEGIN}}
protected:
	bool EvEraseBkgnd (HDC dc);
//{{TBitmap256ControlRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TBitmap256Control);
};    //{{TBitmap256Control}}


//
//
// Class to display a sprite
//
//
class TSprite256Control : public TBitmap256Control
{
protected:
	void LoadPictureData (const char *picname, BYTE **ppData,
						  USHORT *pxsize, USHORT *pysize, SHORT remapPlayer = 0);
	virtual void BuildBitmapData (const char *name, SHORT remapPlayer = 0);

public:
	TSprite256Control (TWindow* parent, int id, const char* title,
					   int x, int y, int w, int h, TModule* module = 0);
};


//
//
// Class to display a wall texture
//
//
class TWallTextureControl : public TSprite256Control
{
protected:
	virtual void BuildBitmapData (const char *name, SHORT remapPlayer = 0);

public:
	TWallTextureControl (TWindow* parent, int id, const char* title,
						 int x, int y, int w, int h, TModule* module = 0);
};

//
//
// Class to display a Floor/Ceiling texture
//
//
class TFloorTextureControl : public TSprite256Control
{
protected:
	virtual void BuildBitmapData (const char *name, SHORT remapPlayer = 0);

public:
	TFloorTextureControl (TWindow* parent, int id, const char* title,
						  int x, int y, int w, int h, TModule* module = 0);
};

#endif                                      // __bmp256ct_h sentry.

