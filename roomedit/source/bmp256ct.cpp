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

	FILE:         bmp256ct.cpp

	OVERVIEW
	========
	Source file for implementation of TBitmap256Control (TControl).
*/

#include "common.h"
#pragma hdrstop

#include "bmp256ct.h"

#ifndef __wads_h
	#include "wads.h"		// FindMasterDir(), BasicWadRead(), ...
#endif

#ifndef OWL_DC_H
	#include <owl\dc.h>
#endif

#ifndef OWL_CONTROL_H
	#include <owl\control.h>
#endif

#ifndef OWL_SCROLLER_H
	#include <owl\scroller.h>
#endif

#include "dibutil.h"
#include "pal.h"

extern Color base_palette[];


#ifndef max
#define max(a,b)    ((a) > (b) ? (a) : (b))
#define min(a,b)    ((a) < (b) ? (a) : (b))
#endif

// Global Zoom and gamma correction vers. to keep
// between dialog boxes
static int GlobalZoom  = 2;
static int GlobalGamma = 20;

//
// size of scan in bytes =
//   Pixel Width * bits per pixel rounded up to a DWORD boundary
//
inline
long
ScanBytes(int pixWidth, int bitsPixel)
{
  return (((long)pixWidth*bitsPixel+31) / 32) * 4;
}


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TBitmap256Control, TControl)
//{{TBitmap256ControlRSP_TBL_BEGIN}}
	EV_WM_ERASEBKGND,
//{{TBitmap256ControlRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TBitmap256Control Implementation}}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//
TBitmap256Control::TBitmap256Control (TWindow* parent, int id,
									  const char* title, int x, int y,
									  int w, int h, TModule* module):
	TControl(parent, id, title, x, y, w, h, module)
{
	// No doom palette loaded
	NumDoomPalette = -1;

	// Override the default window style for TControl.
	Attr.Style = WS_BORDER | WS_CHILD | WS_HSCROLL  | WS_VISIBLE | WS_VSCROLL;

	// Window maximum size is construtor size
	MaxWidth  = w;
	MaxHeight = h;

	// Initialize GDI object to 0
	pBitmapData = NULL;
	strcpy(BitmapName, "");

	// BITMAP info memory handle and pointer
	pDIBInfo      = NULL;
	pDIBits       = NULL;

	// Log palette for displayed bitmap
	pDoomPaletteEntries = new PALETTEENTRY[256];
	pBitmapPalette = NULL;

	pUsedColors = new BOOL[256];
	pEntryMapping = new BYTE[256];

	// Initialize bitmap size and offset
	BitmapXSize = 0;
	BitmapYSize = 0;

	ZoomFactor = GlobalZoom;
	GammaLevel = GlobalGamma;

	Scroller = new TScroller (this, 1, 1, 1, 1);
	Scroller->AutoMode = TRUE;
	Scroller->AutoOrg  = TRUE;

	SetCursor (NULL, IDC_CROSS);
}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//
TBitmap256Control::~TBitmap256Control ()
{
	TRACE ("TBitmap256Control:: destructor called");
	Destroy();

	delete pUsedColors;
	delete pEntryMapping;
	delete pBitmapPalette;
	delete pDoomPaletteEntries;
	if ( pBitmapData != NULL )
		FreeMemory (pBitmapData);
	if ( pDIBInfo != NULL )
		FreeMemory (pDIBInfo);
}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//
char*
TBitmap256Control::GetClassName ()
{
	return "Bitmap256Control";
}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//  Select black background brush
void
TBitmap256Control::GetWindowClass (WNDCLASS& wndClass)
{
	TControl::GetWindowClass(wndClass);

	// Set background color to black
	wndClass.hbrBackground = (HBRUSH)::GetStockObject (BLACK_BRUSH);
}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//  Load a doom palette in DoomPaletteEntries
void
TBitmap256Control::LoadDoomPalette (int playpalnum)
{
   assert (pDoomPaletteEntries != NULL);
   assert (playpalnum >= 0 && playpalnum <= 13);
	int       n;
   
   // Palette is already loaded
   if ( playpalnum == NumDoomPalette )
      return;
   
   // Initialize palette entries
   // Just copy BlakSton palette ARK
   for (n = 0; n < 256; n++)
   {
      pDoomPaletteEntries[n].peRed   = base_palette[n].red;
      pDoomPaletteEntries[n].peGreen = base_palette[n].green;
      pDoomPaletteEntries[n].peBlue  = base_palette[n].blue;
      pDoomPaletteEntries[n].peFlags = PC_NOCOLLAPSE;
   }
   
   // New doom palette loaded
   NumDoomPalette = playpalnum;
}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//
void TBitmap256Control::SetupWindow ()
{
	TControl::SetupWindow();

	// TRect cRect = GetClientRect ();
	// MaxWidth = cRect.Width ();
	// MaxHeight = cRect.Height ();

	// Set window size to maximum (since no bitmap selected yet)
	AdjustWindowSize();
}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//  Adjust window size occording to zoom factor, bitmap size
//  and bitmap offset
void
TBitmap256Control::AdjustWindowSize ()
{
	// Removed 'static' for the 4 vars. below, because Win95 allow
	// the user to change system metrics when he wants...
	int SBWidth  = 0 ;
	int SBHeight = 0 ;
	int CXBorder = 0;
	int CYBorder = 0;
	int BitmapZoomXSize, BitmapZoomYSize;
	BOOL HasVScroll, HasHScroll;
	int BorderWidth, BorderHeight;
	int wWidth, wHeight;
	int xRange, yRange;

	// Get width of vertical and height of horizontal scroll bars
	if ( SBWidth == 0 )
	{
		SBWidth  = GetSystemMetrics (SM_CXVSCROLL);
		SBHeight = GetSystemMetrics (SM_CYHSCROLL);
		CXBorder = GetSystemMetrics (SM_CXBORDER);
		CYBorder = GetSystemMetrics (SM_CYBORDER);
	}

	// If no bitmap
	if ( pDIBInfo == NULL )
	{
		// Reset Window size to maximum orignal size
		Scroller->SetRange(0, 0);
		SetWindowPos (NULL,
					  0, 0,
					  MaxWidth + 2 * CXBorder, MaxHeight + 2 * CYBorder,
					  SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		return ;
	}

	// Calc. the maximum bitmap size
	BitmapZoomXSize = BitmapXSize * ZoomFactor;
	BitmapZoomYSize = BitmapYSize * ZoomFactor;

	// The window will have Scroll bars ?
	if ( BitmapZoomXSize > MaxWidth  )      HasHScroll = TRUE;
	else                                    HasHScroll = FALSE;
	if ( BitmapZoomYSize > MaxHeight )      HasVScroll = TRUE;
	else                                    HasVScroll = FALSE;

	// Calc. window width and height
	BorderWidth   = HasVScroll ? SBWidth  + CXBorder : CXBorder * 2;
	BorderHeight  = HasHScroll ? SBHeight + CYBorder : CYBorder * 2;

	// Calc. client area width
	wWidth  = BitmapZoomXSize + BorderWidth > MaxWidth  ?
			  ((MaxWidth - BorderWidth) / ZoomFactor) * ZoomFactor:
			  BitmapZoomXSize;

	xRange = max ((BitmapZoomXSize - wWidth) / ZoomFactor, 0);

	// Calc. client area height
	wHeight = BitmapZoomYSize + BorderHeight > MaxHeight ?
			  ((MaxHeight - BorderHeight) / ZoomFactor) * ZoomFactor:
			  BitmapZoomYSize;

	yRange = max ((BitmapZoomYSize - wHeight) / ZoomFactor, 0);

	// Set window size
	TRect cRect = GetWindowRect();
	if ( cRect.Width() != wWidth  ||  cRect.Height() != wHeight )
	{
		SetWindowPos (NULL,
					  0, 0,
					  wWidth + BorderWidth, wHeight + BorderHeight,
					  SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	//
	// Adjust scroller position and range
	//

	// Set scroller units
	Scroller->SetUnits (ZoomFactor, ZoomFactor);

	// Calc. scroller position
	Scroller->XPos = min (Scroller->XPos, xRange);
	Scroller->YPos = min (Scroller->YPos, yRange);

	Scroller->SetRange (xRange, yRange);
}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//  Display bitmap
void
TBitmap256Control::Paint (TDC& dc, bool erase, TRect& rect)
{
	TControl::Paint(dc, erase, rect);

	DisplayBitmap (dc, rect);
}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//  Display bitmap in DC
void TBitmap256Control::DisplayBitmap (TDC& dc, TRect &rect)
{
	// Display a cross if no bitmap
	if ( pDIBInfo == 0 )
	{
		dc.SelectObject(TPen(TColor::LtGray));
		dc.MoveTo (0, 0);
		dc.LineTo (MaxWidth, MaxHeight);
		dc.MoveTo (0, MaxHeight);
		dc.LineTo (MaxWidth, 0);
		dc.SetTextAlign(TA_CENTER);
		dc.SetTextColor(TColor::White);
		dc.SetBkColor(TColor::Black);
		char tmp[40];
		if ( BitmapName[0] != '\0' && BitmapName[0] != '-' )
			wsprintf (tmp, "No picture (%s)", BitmapName);
		else
			wsprintf (tmp, "No picture");
		dc.TextOut (MaxWidth / 2, MaxHeight / 2 - 6, tmp);
		return;
	}

	assert (pBitmapPalette != NULL);

	// pBitmapPalette->UnrealizeObject();
	dc.SelectObject (*pBitmapPalette);
	dc.RealizePalette();
	dc.SetStretchBltMode (COLORONCOLOR);

#if 1
	TRect ZoomRect;
	ZoomRect.left   = rect.left;
	ZoomRect.top    = rect.top;
	ZoomRect.right  = rect.right;
	ZoomRect.bottom = rect.bottom;

	// Convert the rect. size to a rect in the sprite
	rect.left   /= ZoomFactor;
	rect.top    /= ZoomFactor;
	rect.right  /= ZoomFactor;
	rect.bottom /= ZoomFactor;

	TRect DIBRect;
	DIBRect.left   = rect.left;
	DIBRect.top    = BitmapYSize - rect.bottom; 	// DIBs are Y inversed
	DIBRect.right  = DIBRect.left + rect.Width();
	DIBRect.bottom = DIBRect.top + rect.Height();

	dc.StretchDIBits (ZoomRect,
					  DIBRect,
					  pDIBits, *pDIBInfo,
					  DIB_PAL_COLORS, SRCCOPY);
#else
	// Create memory DC and display bitmap
	TMemoryDC mdc (dc);
	mdc.SelectObject (*pBitmapPalette);
	mdc.SelectObject (*pBitmap);
	dc.StretchBlt(0, 0, ZoomXSize,   ZoomYSize,
				  mdc,
				  0, 0, BitmapXSize, BitmapYSize,
				  SRCCOPY);

	// Restore GDI objects
	mdc.RestoreBitmap();
	mdc.RestorePalette();
#endif
	dc.RestorePalette();
}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//  Change the zoom factor
void
TBitmap256Control::SetZoomFactor (UINT factor)
{
	if (factor < 1 || factor > 16)
		return;

	if ( factor == ZoomFactor )
		return;

	ZoomFactor = factor;
	GlobalZoom = ZoomFactor;	// Global to save between calls

	if ( pDIBInfo != NULL )
	{
		AdjustWindowSize();
		Invalidate(FALSE);
		UpdateWindow();     // Repaint NOW!
	}
}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//  Change the gamma level correction
void
TBitmap256Control::SetGammaLevel (BYTE level)
{
	if ( (UINT)level == GammaLevel )
		return;

	GammaLevel  = level;
	GlobalGamma = GammaLevel;	// Global to save between calls

	if ( pDIBInfo != NULL )
	{
		BuildBitmapPalette (FALSE);
		// BuildBitmap();
		Invalidate(FALSE);
		UpdateWindow();     // Repaint NOW!
	}
}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//  Select a new bitmap in the control. First build the bitmap
//  data by calling the virtual function BuildBitmapData, then
//  calc. the bitmap palette and create the bitmap GDI object.
//  Then call AdjustWindowSize and DisplayBitmap.
void TBitmap256Control::SelectBitmap2 (const char *name, SHORT remapPlayer, int palnum)
{
   char buf[MAX_PATH];
   
   assert(pUsedColors != NULL);
   
   // Save GDI object and set WAIT cursor
   SELECT_WAIT_CURSOR();
   
   // Free all previous bitmap ressources
   if ( pBitmapData != NULL )	FreeMemory (pBitmapData);
   if ( pDIBInfo != NULL )		FreeMemory (pDIBInfo);
   delete pBitmapPalette;
   pBitmapPalette = NULL;
   
   // Copy name of the new bitmap
   strcpy(BitmapName, name);
   
   // Delete array of used color entry
   pUsedColors[0] = TRUE; // always use BLACK (for background)
   for (int i = 1; i < 256 ; i++)
      pUsedColors[i] = FALSE;
   
   // call virtual function of derived class to build bitmap data
   // TRACE ("TBitmap256Controll::SelectBitmap(): call BuildBitmapData");
   sprintf(buf, "%s\\%s", BitmapDir, name);
   BuildBitmapData (buf, remapPlayer);
   
   // Maybe there's no bitmap! (EX: sounds in heretic)
   if ( pBitmapData != NULL )
   {
      // Load DOOM palette asked
      LoadDoomPalette (palnum);
      
      // Create logical palette of used colors
      BuildBitmapPalette ();
      
      // Build bitmap GDI object
      BuildBitmap ();
   }
   
   // Adjust window size and scroller range, and display bitmap
   AdjustWindowSize ();
   Invalidate();
   UpdateWindow();     // Repaint NOW!
   
   // Restore cursor
   UNSELECT_WAIT_CURSOR();
}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//  Creates the GDI bitmap palette from UsedColors and from
//  the doom palette. The EntryMapping array is an array of 256
//  elements representing a bitmap palette index equivalent of
//  the doom palette.
void TBitmap256Control::BuildBitmapPalette (BOOL CreateNewPal)
{
	assert (pEntryMapping != NULL);
	assert (pUsedColors != NULL);

	// TRACE ("TBitmap256Controll::SelectBitmap(): Creating bitmap palette");
	PALETTEENTRY *Entries = new PALETTEENTRY[256];
	NbEntries = 0;

	// In this loop, we create palette entries and array of
	// palette mapping index from doom palette to bitmap palette
	for (int n = 0; n < 256; n++)
	{
		if ( pUsedColors[n] )
		{
			Entries[NbEntries] = pDoomPaletteEntries[n];
			// Don't adjust black background
			if ( n > 0 )
				GammaAdjustEntry (Entries[NbEntries]);
			pEntryMapping[n] = (BYTE)NbEntries;

			NbEntries++;
		}
		else
			pEntryMapping[n] = 0;
	}

	// Create new palette or update the existing one
	if ( CreateNewPal  ||  pBitmapPalette == NULL)
	{
		delete pBitmapPalette;
		pBitmapPalette = new TPalette (Entries, NbEntries);
	}
	else
	{
		pBitmapPalette->SetPaletteEntries ((WORD)0, (WORD)NbEntries, Entries);
	}
	delete Entries;
}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//  Creates the GDI bitmap from the BitmapData array and use
//  the EntryMapping array to select palette index colors.
void TBitmap256Control::BuildBitmap ()
{
	assert (pBitmapData    != NULL);
	assert (pBitmapPalette != NULL);
	assert (pEntryMapping  != NULL);

	// Create client DC
	ConvertBitmapToDib();

#if 0
	LogMessage ("\nBitmap bits from pDIBits() (inversed line order !)\n");
	for (y = BitmapYSize - 1 ; y >= 0 ; y--)
	{
		char msg[1024];

		msg[0] = '\0';
		for (x = 0 ; x < BitmapXSize ; x++)
		{
			wsprintf (&msg[strlen(msg)], "%02x ",
					  ((char *)pDIBits)[y * ((BitmapXSize+3)& ~3) + x]);
		}
		LogMessage ("Y = %03d : %s\n", y, msg);
	}
#endif
}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//  Convert the pBitmap and pBitmapPalette to a DIB.
void TBitmap256Control::ConvertBitmapToDib()
{
	// Just to be sure
	if ( BitmapXSize == 0 || BitmapYSize == 0 || pBitmapData == NULL )
		return;

	// DIB creation
	BITMAPINFOHEADER infoHeader;

	infoHeader.biSize          = sizeof(BITMAPINFOHEADER);
	infoHeader.biWidth         = BitmapXSize;
	infoHeader.biHeight        = BitmapYSize;
	infoHeader.biPlanes        = 1;		// 1 plane
	infoHeader.biBitCount      = 8;		// 8 bits/pixel
	infoHeader.biXPelsPerMeter = 0;
	infoHeader.biYPelsPerMeter = 0;
	infoHeader.biClrUsed       = NbEntries;	// # of entries in Palette
	infoHeader.biClrImportant  = 0;
	infoHeader.biCompression   = BI_RGB;
	infoHeader.biSizeImage     = ScanBytes(BitmapXSize, 8) * BitmapYSize;

	// size of color tables (we use INDEXES, not RGBQUAD !!!)
	int  colorAlloc = NbEntries * sizeof(WORD);
	long bitsAlloc = infoHeader.biSize + colorAlloc + infoHeader.biSizeImage;

	// Free memory of previous DIB
// XXX Not sure why this crashes now ARK
//	if ( pDIBInfo != NULL )
//		FreeMemory (pDIBInfo);

	pDIBInfo = (LPBITMAPINFO)GetMemory (bitsAlloc);
	pDIBInfo->bmiHeader = infoHeader;
	pDIBits = ((BYTE *)pDIBInfo) + ((int)infoHeader.biSize + colorAlloc);

	// Inverse the bits from the Bitmap to the DIB bits.
#if 0
	dc.GetDIBits (*pBitmap, 0, BitmapYSize, pDIBits, *pDIBInfo, DIB_PAL_COLORS);
#else
	// Set the DIB byte from pBitmapData with palette color mapping
	BYTE *ptrData = pBitmapData;
	SHORT xBytes  = (BitmapXSize + 3) & ~3;

	for (SHORT y = BitmapYSize - 1 ; y >= 0 ; y--)
	{
		BYTE *pDIBLineOffset = &pDIBits[y * xBytes];

		for (SHORT x = 0 ; x < BitmapXSize ; x++)
		{
			*pDIBLineOffset = pEntryMapping[*ptrData];
			ptrData++;
			pDIBLineOffset++;
		}
	}
#endif

	// Set the INDEX of the DIB palette
	WORD *lpTable;
	lpTable = (WORD*)((char*)pDIBInfo + (int)infoHeader.biSize);
	for (WORD i = 0; i < (WORD)infoHeader.biClrUsed; i++)
		*lpTable++ = i;
}


////////////////////////////////////////////////////////////
// TBitmap256Control
// -----------------
//
bool TBitmap256Control::EvEraseBkgnd (HDC dc)
{
	// Suppress flicker when there's a bitmap to be repainted.
	if ( pDIBInfo != NULL )
		return TRUE;
	else
		return TControl::EvEraseBkgnd(dc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//
// TSprite256Control implementation
//
//
//
//////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// TSprite256Control
// -----------------
//
TSprite256Control::TSprite256Control (TWindow* parent, int id, const char* title, int x, int y, int w, int h, TModule* module):
	TBitmap256Control(parent, id, title, x, y, w, h, module)
{
}


////////////////////////////////////////////////////////////
// TSprite256Control
// -----------------
//
void
TSprite256Control::BuildBitmapData (const char *name, SHORT remapPlayer)
{
	assert (pUsedColors != NULL);
	assert (pBitmapData == NULL);
	BitmapXSize = BitmapYSize = 0;

	// Initialize pBitmapData and Bitmap size
	LoadPictureData (name, &pBitmapData, &BitmapXSize, &BitmapYSize,
					 remapPlayer);
	// assert (pBitmapData != NULL);
}


////////////////////////////////////////////////////////////
// TSprite256Control
// -----------------
//
void TSprite256Control::LoadPictureData (const char *picname, BYTE **ppData,
										 USHORT *pxsize, USHORT *pysize,
										 SHORT remapPlayer)
{
  MDirPtr  dir;
	SHORT  x, y;
	SHORT  xofs, yofs;
	 BYTE *lpColumnData;
	 BYTE *lpColumn;
	 BYTE  color;
	 LONG *lpNeededOffsets;
	SHORT  nColumns, nCurrentColumn;
	 LONG  lCurrentOffset;
	SHORT  i, n;
	 BYTE  bRowStart, bColored;

	assert (pUsedColors != NULL);

	TRACE ("TSprite256Control::LoadPictureData(" << dec << picname);

	// Search for picture name
	dir = FindMasterDir (MasterDir, (char *)picname);
	if (dir == NULL)
	{
		*ppData = NULL;
		// Notify ("Couldn't find sprite or picture \"%s\" (BUG!)", picname);
		return;
	}

	// Read picture size and offset
	BasicWadSeek(dir->wadfile, dir->dir.start);
	BasicWadRead(dir->wadfile, pxsize, 2L);
	BasicWadRead(dir->wadfile, pysize, 2L);
	// We ingnore x and y offsets
	BasicWadRead(dir->wadfile, &xofs, 2L);
	BasicWadRead(dir->wadfile, &yofs, 2L);
	xofs = yofs = 0;

	USHORT xsize = *pxsize;
	USHORT ysize = *pysize;
	nColumns = xsize;

	// Allocate space for bitmap data
	ULONG dsize = (ULONG)xsize * ysize;

	TRACE ("TSprite256Control::LoadPictureData: " << dec
		   << "Datasize = " << dsize
		   << ", xsize = "  << xsize
		   << ", ysize = "  << ysize);

	*ppData = (BYTE *)GetMemory (dsize);

#define TEX_COLUMNBUFFERSIZE    (60L * 1024L)
#define TEX_COLUMNSIZE      512L

// dir->size - 4 shorts (size and offset) - Columns offsets * 4L
#define CD_BUFFER_SIZE		(dir->dir.size - 4 * 2L - nColumns * 4L)

	/* Note from CJS:
	   I tried to use far memory originally, but kept getting out-of-mem errors
	   that is really strange - I assume that the wad dir et al uses all
	   the far mem, and there is only near memory available. NEVER seen
	   this situation before..... I'll keep them huge pointers anyway,
	   in case something changes later
	*/
	lpColumnData = (BYTE*)GetMemory (CD_BUFFER_SIZE);

	// Initialize columns offsets
	lpNeededOffsets = (LONG *)GetMemory (nColumns * 4L);
	BasicWadRead (dir->wadfile, lpNeededOffsets, nColumns * 4);

	// read first column data, and subsequent column data
	BasicWadSeek(dir->wadfile, dir->dir.start + lpNeededOffsets[0]);
	BasicWadRead(dir->wadfile, lpColumnData, CD_BUFFER_SIZE);

	for (nCurrentColumn = 0; nCurrentColumn < nColumns; nCurrentColumn++)
	{
		lCurrentOffset  = lpNeededOffsets[nCurrentColumn];
		assert (lCurrentOffset - lpNeededOffsets[0] < CD_BUFFER_SIZE);
		lpColumn = (BYTE *)&lpColumnData[lCurrentOffset - lpNeededOffsets[0]];

		/* we now have the needed column data, one way or another, so write it */
		n = 1;
		bRowStart = lpColumn[0];
		x = nCurrentColumn;
		while (bRowStart != 255 && n < TEX_COLUMNSIZE)
		{
			bColored = lpColumn[n];
			n += 2;             // skip over 'null' pixel in data
			y = bRowStart;
			for (i = 0; i < bColored; i++)
			{
				assert (x >= 0 && x < xsize);
				assert (y >= 0 && y < ysize);

				color = lpColumn[i+n];
				// Remap player start position color (GREEN, RED, ...)
				if ( (remapPlayer) && (color >= 0x70) && (color < 0x80))
					color += remapPlayer;

				(*ppData)[(LONG)y * xsize + x] = color;
				pUsedColors[color] = TRUE;
				y++;
			}
			n += bColored + 1;  // skip over written pixels, and the 'null' one
			bRowStart = lpColumn[n++];
		}
		assert (bRowStart == 255);
	}
	FreeMemory (lpColumnData);
	FreeMemory (lpNeededOffsets);
}



//////////////////////////////////////////////////////////////////////////
//
//
//
// TWallTextureControl implementation
//
//
//
//////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// TWallTextureontrol
// -----------------
//
TWallTextureControl::TWallTextureControl (TWindow* parent, int id, const char* title, int x, int y, int w, int h, TModule* module):
	TSprite256Control(parent, id, title, x, y, w, h, module)
{
}


////////////////////////////////////////////////////////////
// TWallTextureontrol
// -----------------
//
void
TWallTextureControl::BuildBitmapData (const char *texname, SHORT /*remapPlayer*/)
{
   Bitmaps b;
   PDIB pdib;
   BYTE *bits;
   int size;
   
   if (DibOpenFile(texname, &b) == False)
   {
      Notify("Couldn't load texture %s", texname);
      return;
   }
   
   pdib = BitmapsGetPdib(b, 0, 0);
   if (pdib == NULL)
   {
      Notify("Missing bitmap 0 in texture %s", texname);
      return;
   }
	
   BitmapXSize = DibWidth(pdib);
   BitmapYSize = DibHeight(pdib);
   bits = DibPtr(pdib);
   
   size = BitmapXSize * BitmapYSize;
   pBitmapData = (BYTE *) GetMemory(size);
   memcpy(pBitmapData, bits, size);
   
   BitmapsFree(&b);

   // Mark used colors
   BYTE *ptrData = pBitmapData;
   for (int i = 0 ; i < size ; i++ )
   {
      pUsedColors[*ptrData] = TRUE;
      ptrData++;
   }

   return;
}



//////////////////////////////////////////////////////////////////////////
//
//
//
// TFloorextureControl implementation
//
//
//
//////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// TFloorTextureontrol
// -----------------
//
TFloorTextureControl::TFloorTextureControl (TWindow* parent, int id, const char* title, int x, int y, int w, int h, TModule* module):
	TSprite256Control(parent, id, title, x, y, w, h, module)
{
}


////////////////////////////////////////////////////////////
// TFloorTextureontrol
// -----------------
//
void TFloorTextureControl::BuildBitmapData (const char * /*texname*/, SHORT /*remapPlayer*/)
{
}

