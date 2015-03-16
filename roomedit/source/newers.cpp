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

	FILE:         newers.cpp

	OVERVIEW
	========
	Source file for implementation of newers.
*/

#include "common.h"
#pragma hdrstop

#ifndef OWL_EDIT_H
	#include <owl\edit.h>
#endif

TEdit *newTEdit(TWindow* parent, int resourceID, UINT textLen, TModule* module)
{
	return new TEdit(parent, resourceID, textLen, module);
}

#ifndef OWL_STATIC_H
	#include <owl\static.h>
#endif


TStatic *newTStatic(TWindow* parent, int resourceId, UINT textLen, TModule* module)
{
	return new TStatic(parent, resourceId, textLen, module);
}


#ifndef OWL_LISTBOX_H
	#include <owl\listbox.h>
#endif

TListBox *newTListBox(TWindow* Parent, int resourceId, TModule* module)
{
	return new TListBox(Parent, resourceId, module);
}


#ifndef OWL_COMBOBOX_H
	#include <owl\combobox.h>
#endif

TComboBox *newTComboBox(TWindow* parent, int ResourceId, UINT textLen, TModule*module)
{
	return new TComboBox(parent, ResourceId, textLen, module);
}


#ifndef OWL_CHECKBOX_H
	#include <owl\checkbox.h>
#endif

TCheckBox *newTCheckBox(TWindow* parent, int resourceId, TGroupBox* group, TModule*module)
{
	return new TCheckBox(parent, resourceId, group, module);
}


#ifndef OWL_RADIOBUT_H
	#include <owl\radiobut.h>
#endif

TRadioButton *newTRadioButton(TWindow* parent,int resourceId,TGroupBox *group, TModule*module)
{
	return new TRadioButton(parent, resourceId, group, module);
}


#ifndef OWL_BUTTON_H
	#include <owl\button.h>
#endif

TButton *newTButton(TWindow* parent, int resourceId, TModule*module)
{
	return new TButton(parent, resourceId, module);
}

