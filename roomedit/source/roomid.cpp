
#include "common.h"
#pragma hdrstop

#include "thingdlg.rh"

#include "roomid.h"
#include "things.h"

#ifndef OWL_LISTBOX_H
	#include <owl\listbox.h>
#endif

#ifndef __newers_h
	#include "newers.h"
#endif

// HELP IDs
#include "windeuhl.h"


//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TSetRoomIDDialog, TDialog)
//{{TSetRoomIDDialogRSP_TBL_BEGIN}}
	EV_BN_CLICKED(IDOK, CmOk),
//{{TSetRoomIDDialogRSP_TBL_END}}
END_RESPONSE_TABLE;


//{{TSetRoomIDDialog Implementation}}


//////////////////////////////////////////////////////////////////////
// TSetRoomIDDialog
// ----------
//
TSetRoomIDDialog::TSetRoomIDDialog (TWindow* parent, TResId resId, TModule* module):
	TDialog(parent, resId, module)
{
	pRoomIDList = newTListBox(this, IDC_ROOMID_LIST);
}


//////////////////////////////////////////////////////////////////////
// TSetRoomIDDialog
// ----------
//
TSetRoomIDDialog::~TSetRoomIDDialog ()
{
	Destroy();
}


//////////////////////////////////////////////////////////////////////
// TSetRoomIDDialog
// ----------
//
void
TSetRoomIDDialog::SetupWindow ()
{
	TDialog::SetupWindow();
	::CenterWindow (this);

	if (pRoomIDList)
	{
		pRoomIDList->ClearList();
		for(int i=0; i < GetNumKodRooms(); i++)
		{
			int index = pRoomIDList->AddString(GetKodRoomDecorativeName(i));
			if (RoomID == GetKodRoomID(i))
			{
				pRoomIDList->SetSelIndex(index);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////
// TSetRoomIDDialog
// ----------------
//
void TSetRoomIDDialog::CmOk ()
{
	char buffer[256];
	// Cannot close if edit controls not valid
	if ( !CanClose() )
		return;

	int index = pRoomIDList->GetSelIndex ();
	if (index < 0)
		return;

	pRoomIDList->GetString(buffer,index);
	RoomID = FindKodRoomID(buffer);

	// Close dialog box
	TDialog::CmOk();
}


