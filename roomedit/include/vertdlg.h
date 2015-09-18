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

	FILE:         vertdlg.h

	OVERVIEW
	========
	Class definition for TVertexDialog (TDialog).
*/
#if !defined(__vertdlg_h)              // Sentry, use file only if it's not already included.
#define __vertdlg_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_EDIT_H
	class _OWLCLASS TEdit;
#endif

#ifndef __objects_h
	#include "objects.h"	// SelPtr
#endif

#ifndef __levels_h
	#include "levels.h"	// Vertex
#endif

#ifndef __cnfvedlg_h
	#include "cnfvedlg.h"
#endif

#include "vertdlg.rh"            // Definition of all resources.


//{{TDialog = TVertexDialog}}

class TVertexDialog : public TDialog
{
protected:
	SelPtr SelVertices;
	Vertex CurVertex;
	TConfirmVertexDialogXfer ConfirmData;

public:
	TVertexDialog (TWindow* parent, SelPtr SelectedVertices, TResId resId = IDD_VERTEX, TModule* module = 0);
	virtual ~TVertexDialog ();

//{{TVertexDialogXFER_DEF}}
protected:
	TEdit *pXPosEdit;
	TEdit *pYPosEdit;
//{{TVertexDialogXFER_DEF_END}}

//{{TVertexDialogVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
//{{TVertexDialogVIRTUAL_END}}

//{{TVertexDialogRSP_TBL_BEGIN}}
protected:
	void CmOk ();
//{{TVertexDialogRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TVertexDialog);
};    //{{TVertexDialog}}


#endif                                      // __vertdlg_h sentry.

