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

	FILE:         inpt2dlg.cpp

	OVERVIEW
	========
	Source file for implementation of TInput2Dialog (TDialog).
*/

#include "common.h"
#pragma hdrstop

#include "inpt2dlg.h"

#ifndef OWL_EDIT_H
	#include <owl\edit.h>
#endif

#ifndef OLW_STATIC_H
	#include <owl\static.h>
#endif


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TInput2Dialog, TDialog)
//{{TInput2DialogRSP_TBL_BEGIN}}
	EV_BN_CLICKED(IDOK, CmOk),
//{{TInput2DialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TInput2Dialog Implementation}}


TInput2Dialog::TInput2Dialog (TWindow *parent, char *title, char *prompt,
							  char *buffer1, int buffer1Size,
							  char *buffer2, int buffer2Size,
							  TValidator *valid1,
							  TValidator *valid2,
							  TResId resId, TModule *module):
	TDialog(parent, resId, module)
{
	Title = new char[strlen(title)+1];
	strcpy (Title, title);
	Prompt = new char[strlen(prompt)+1];
	strcpy (Prompt, prompt);
	Buffer1     = buffer1;
	Buffer1Size = buffer1Size;
	Buffer2     = buffer2;
	Buffer2Size = buffer2Size;

	pInput1Edit = new TEdit(this, IDC_INPUT1_EDIT, buffer1Size);
	pInput2Edit = new TEdit(this, IDC_INPUT2_EDIT, buffer2Size);
	pPromptStatic = new TStatic(this, IDC_PROMPT, 60);

	pInput1Edit->SetValidator (valid1);
	pInput2Edit->SetValidator (valid2);
}


TInput2Dialog::~TInput2Dialog ()
{
	Destroy();
	delete Title;
	delete Prompt;
}


void TInput2Dialog::CmOk ()
{
	pInput1Edit->GetText (Buffer1, Buffer1Size-1);
	pInput2Edit->GetText (Buffer2, Buffer2Size-1);

	TDialog::CmOk();
}


void TInput2Dialog::SetupWindow ()
{
	TDialog::SetupWindow();
	::CenterWindow (this);

	SetCaption (Title);
	pPromptStatic->SetText (Prompt);
	pInput1Edit->SetText (Buffer1);
	pInput2Edit->SetText (Buffer2);
}

