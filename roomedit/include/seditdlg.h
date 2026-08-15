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

	FILE:         seditdlg.h

	OVERVIEW
	========
	Class definition for TSectorEditDialog (TDialog).
*/
#if !defined(__seditdlg_h)              // Sentry, use file only if it's not already included.
#define __seditdlg_h

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

#ifndef OWL_EDIT_H
	class _OWLCLASS TEdit;
#endif

#ifndef OWL_COMBOBOX_H
	class _OWLCLASS TComboBox;
#endif

#ifndef __wstructs_h
	#include "wstructs.h"	// Sector
#endif

#ifndef __objects_h
	#include "objects.h"	// SelPtr
#endif

#ifndef __cnfsedlg_h
	#include "cnfsedlg.h"
#endif

#include "seditdlg.rh"            // Definition of all resources.
#include "dibutil.h"

//{{TDialog = TSectorEditDialog}}


class TSectorEditDialog : public TDialog
{
private:
	Sector CurSector;
	char TextureName[MAX_BITMAPNAME + 1];
	SelPtr SelSectors;
	TConfirmSectorDialogXfer ConfirmData;
	SHORT *SlopeVertexList;	// Vertices on this sector's boundary
	int NumSlopeVertexes;

protected:
	void SetTextureList();
	void SetSectorList();
	void SetVertexLists();
	int SlopeVertexIndex (SHORT vertex);
	void SetSector();
        BOOL GetSector();
	BOOL IsPointInDlgItem (int itemId, TPoint &clientPoint);
	TStatic *GetPointedStatic (TPoint& point);

public:
	TSectorEditDialog (TWindow* parent, SelPtr sel, TResId resId = IDD_SECTOR_EDIT, TModule* module = 0);
	virtual ~TSectorEditDialog ();

	// Preset the floor and ceiling slopes to the given vertices, with
	// all heights at the sector's flat floor/ceiling heights.  Call
	// before Execute().
	void SetSlopePreset (SHORT v1, SHORT v2, SHORT v3);

//{{TSectorEditDialogVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
//{{TSectorEditDialogVIRTUAL_END}}

//{{TSectorEditDialogRSP_TBL_BEGIN}}
protected:
	void CmOk ();
	void ToFloorClicked ();
	void FloorClearClicked ();
	void ToCeilingClicked ();
	void CeilingClearClicked ();
	void FloorSlopeClearClicked ();
	void CeilingSlopeClearClicked ();
	void SectorSelChange ();
	void TextureSelChange ();
	void TextureDblclick ();
	void NewTagClicked ();
	void DepthClicked ();
	void ScrollClicked ();
	void EvLButtonDown (UINT modKeys, const TPoint& point);
	void EvLButtonDblClk (UINT modKeys, const TPoint& point);
//{{TSectorEditDialogRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TSectorEditDialog);

protected:
	TCheckBox *pNoAmbientCheck;
	TListBox *pSectorList;
	TEdit *pLightEdit;
	TCheckBox *pFlickerCheck;
	TEdit *pTextureXEdit;
	TEdit *pTextureYEdit;
	TListBox *pTextureList;
	TStatic *pFloorStatic;
	TEdit *pFloorHeightEdit;
	TStatic *pFloorStyle;
	TStatic *pCeilingStatic;
	TEdit *pCeilingHeightEdit;
	TStatic *pCeilingStyle;
	TEdit *pIDNumEdit;
	TEdit *pSpeedEdit;
	TRadioButton *pDepth0Radio;
	TRadioButton *pDepth1Radio;
	TRadioButton *pDepth2Radio;
	TRadioButton *pDepth3Radio;
	TRadioButton *pScrollNRadio;
	TRadioButton *pScrollNERadio;
	TRadioButton *pScrollERadio;
	TRadioButton *pScrollSERadio;
	TRadioButton *pScrollSRadio;
	TRadioButton *pScrollSWRadio;
	TRadioButton *pScrollWRadio;
	TRadioButton *pScrollNWRadio;
	TCheckBox    *pScrollFloorCheck;
	TCheckBox    *pScrollCeilingCheck;
	TRadioButton *pScrollNoneRadio;
	TRadioButton *pScrollSlowRadio;
	TRadioButton *pScrollMediumRadio;
	TRadioButton *pScrollFastRadio;
	TComboBox    *pSlopeFloorVertex[3];
	TEdit        *pSlopeFloorHeight[3];
	TComboBox    *pSlopeCeilingVertex[3];
	TEdit        *pSlopeCeilingHeight[3];
	TEdit        *pFloorAngle;
	TEdit        *pCeilingAngle;
};    //{{TSectorEditDialog}}


#endif                                      // __seditdlg_h sentry.

