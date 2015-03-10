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

	FILE:         viewbmp.h

	OVERVIEW
	========
	Class definition for TViewBitmapDialog (TDialog).
*/
#if !defined(__viewbmp_h)              // Sentry, use file only if it's not already included.
#define __viewbmp_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_LISTBOX_H
	class _OWLCLASS TListBox;
#endif

#ifndef OWL_STATIC_H
	class _OWLCLASS TStatic;
#endif

#ifndef OWL_SLIDER_H
	#include <owl\slider.h>
#endif

#ifndef __bmp256ct_h
	#include "bmp256ct.h"
#endif

#include "viewbmp.rh"


/////////////////////////////////////////////////////////////////////
//
//
//  TViewBitmapDialog
//
//   Abstract class for a dialog box with a Bitmap Controler, a Zoom
//   Slider and a Gamma correction Slider.
//
/////////////////////////////////////////////////////////////////////

//{{TDialog = TViewBitmapDialog}}
class TViewBitmapDialog : public TDialog
{
protected:
	TStatic *pSizeStatic;
	TBitmap256Control *pBitmapControl;

	TStatic *pZoomFrame;
	TSlider *pZoomSlider;
	TStatic *pZoomStatic;

	TStatic *pGammaFrame;
	TSlider *pGammaSlider;
	TStatic *pGammaStatic;

	int BitmapControlX, BitmapControlY;
	int BitmapControlW, BitmapControlH;

protected:
	// This function must create the pBitmapControl object
	// virtual function, 0 for this abstract class
	virtual void InitBitmapControl () = 0;

public:
	TViewBitmapDialog (TWindow* parent, TResId resId = IDD_VIEW_BITMAP,
					   TModule* module = 0);
	virtual ~TViewBitmapDialog ();

//{{TViewBitmapDialogVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
	virtual void CloseWindow (int retVal = 0);
//{{TViewBitmapDialogVIRTUAL_END}}

//{{TViewBitmapDialogRSP_TBL_BEGIN}}
protected:
	void EvSetFocus (HWND hWndLostFocus );
	void UpdateZoom (UINT);
	void UpdateGamma (UINT);
//{{TViewBitmapDialogRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TViewBitmapDialog);
};    //{{TViewBitmapDialog}}



/////////////////////////////////////////////////////////////////////
//
//
//  TViewBitmapListDialog
//
//   Abstract class for a dialog box with a ListBox to choose the name
//   a of the bitmap to view.
//
/////////////////////////////////////////////////////////////////////
class TViewBitmapListDialog : public TViewBitmapDialog
{
protected:
	TListBox *pBitmapList;
	TStatic *pChooseStatic;

protected:
	virtual void InitBitmapControl () = 0;

public:
	TViewBitmapListDialog (TWindow* parent, TResId resId = IDD_VIEW_BITMAP, TModule* module = 0);
	virtual ~TViewBitmapListDialog ();
	int SetSelection (const char *str);

public:
	virtual void SetupWindow ();

protected:
	void LBNDblclk ();
	void LBNSelchange ();
DECLARE_RESPONSE_TABLE(TViewBitmapListDialog);
};


class TViewSpriteDialog : public TViewBitmapListDialog
{
protected:
	virtual void InitBitmapControl ();

public:
	TViewSpriteDialog (TWindow* parent, TResId resId = IDD_VIEW_BITMAP, TModule* module = 0);
	virtual ~TViewSpriteDialog ();

public:
	virtual void SetupWindow ();
};


class TViewWallTextureDialog : public TViewBitmapListDialog
{
protected:
	virtual void InitBitmapControl ();

public:
	TViewWallTextureDialog (TWindow* parent, TResId resId = IDD_VIEW_BITMAP, TModule* module = 0);
	virtual ~TViewWallTextureDialog ();

public:
	virtual void SetupWindow ();
};


class TViewFloorTextureDialog : public TViewBitmapListDialog
{
protected:
	virtual void InitBitmapControl ();

public:
	TViewFloorTextureDialog (TWindow* parent, TResId resId = IDD_VIEW_BITMAP, TModule* module = 0);
	virtual ~TViewFloorTextureDialog ();

public:
	virtual void SetupWindow ();
};


/////////////////////////////////////////////////////////////////////
//
//
//  TDisplayBitmapListDialog
//
//   Abstract class for a modeless dialog box to display a Bitmap.
//
/////////////////////////////////////////////////////////////////////

class TDisplayBitmapDialog : public TViewBitmapDialog
{
protected:
	virtual void InitBitmapControl () = 0;

public:
	TDisplayBitmapDialog (TWindow* parent, TResId resId = IDD_DISPLAY_BITMAP, TModule* module = 0);
	virtual ~TDisplayBitmapDialog ();
	int SelectBitmap2 (const char *str);

public:
	virtual void SetupWindow ();
};


class TDisplaySpriteDialog : public TDisplayBitmapDialog
{
protected:
	virtual void InitBitmapControl ();

public:
	TDisplaySpriteDialog (TWindow* parent, TResId resId = IDD_DISPLAY_BITMAP, TModule* module = 0);
	virtual ~TDisplaySpriteDialog ();

public:
	virtual void SetupWindow ();
};


class TDisplayWallTextureDialog : public TDisplayBitmapDialog
{
protected:
	virtual void InitBitmapControl ();

public:
	TDisplayWallTextureDialog (TWindow* parent, TResId resId = IDD_DISPLAY_BITMAP, TModule* module = 0);
	virtual ~TDisplayWallTextureDialog ();

public:
	virtual void SetupWindow ();
};


class TDisplayFloorTextureDialog : public TDisplayBitmapDialog
{
protected:
	virtual void InitBitmapControl ();

public:
	TDisplayFloorTextureDialog (TWindow* parent, TResId resId = IDD_DISPLAY_BITMAP, TModule* module = 0);
	virtual ~TDisplayFloorTextureDialog ();

public:
	virtual void SetupWindow ();
};


#endif                                      // __viewbmp_h sentry.

