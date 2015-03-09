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
#if !defined(__newers_h)              // Sentry, use file only if it's not already included.
#define __newers_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_EDIT_H
	class _OWLCLASS TEdit;
#endif

TEdit        *newTEdit(TWindow* parent, int resourceID, UINT textLen = 0, TModule* module =0);

#ifndef OWL_STATIC_H
	class _OWLCLASS TStatic;
#endif

TStatic      *newTStatic(TWindow* parent, int resourceId, UINT textLen = 0, TModule* module= 0);

#ifndef OWL_LISTBOX_H
	class _OWLCLASS TListBox;
#endif

TListBox     *newTListBox(TWindow* Parent, int resourceId, TModule* module = 0);

#ifndef OWL_COMBOBOX_H
	class _OWLCLASS TComboBox;
#endif

TComboBox    *newTComboBox(TWindow* parent, int ResourceId, UINT textLen = 0, TModule*module = 0);

#ifndef OWL_CHECKBOX_H
	class _OWLCLASS TCheckBox;
#endif

TCheckBox    *newTCheckBox(TWindow* parent, int resourceId, TGroupBox* group = 0, TModule*module = 0);

#ifndef OWL_RADIOBUT_H
	class _OWLCLASS TRadioButton;
#endif

TRadioButton *newTRadioButton(TWindow* parent,int resourceId,TGroupBox *group = 0, TModule*module = 0);

#ifndef OWL_BUTTON_H
	class _OWLCLASS TButton;
#endif

TButton *newTButton(TWindow* parent, int resourceId, TModule*module = 0);


#endif
