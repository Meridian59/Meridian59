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

	FILE:         inpt2dlg.h

	OVERVIEW
	========
	Class definition for TInput2Dialog (TDialog).
*/
#if !defined(__inpt2dlg_h)              // Sentry, use file only if it's not already included.
#define __inpt2dlg_h

#ifndef __common_h
	#include "common.h"
#endif

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_EDIT_H
	class _OWLCLASS TEdit;
#endif

#ifndef OWL_STATIC_H
	class _OWLCLASS TStatic;
#endif

#ifndef OWL_VALIDATE_H
	#include <owl\validate.h>
#endif

#include "inpt2dlg.rh"            // Definition of all resources.


class TInput2Dialog : public TDialog
{
protected:
	char *Title, *Prompt;
	char *Buffer1, *Buffer2;
	int Buffer1Size, Buffer2Size;

	TEdit *pInput1Edit;
	TEdit *pInput2Edit;
	TStatic *pPromptStatic;

public:
	TInput2Dialog (TWindow *parent, char *title, char *prompt,
				   char *buffer1, int buffer1Size,
				   char *buffer2, int buffer2Size,
				   TValidator *valid1 = NULL,
				   TValidator *valid2 = NULL,
				   TResId resId = IDD_INPUT_2, TModule *module = 0);
	virtual ~TInput2Dialog ();

//{{TInput2DialogVIRTUAL_BEGIN}}
public:
	virtual void SetupWindow ();
//{{TInput2DialogVIRTUAL_END}}

//{{TInput2DialogRSP_TBL_BEGIN}}
protected:
	void CmOk ();
//{{TInput2DialogRSP_TBL_END}}
DECLARE_RESPONSE_TABLE(TInput2Dialog);
};    //{{TInput2Dialog}}

#endif                                      // __inpt2dlg_h sentry.

