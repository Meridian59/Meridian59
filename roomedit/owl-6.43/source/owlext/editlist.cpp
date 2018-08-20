//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
//  Description:  An editable listbox. Double clicking an item lets you edit
//                it. An EN_CHANGE notification is sent to the parent when
//                the user finishes editing.
//
// Original code by Christopher Kohloff; used with permission.
//-------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop


#include <owlext/editlist.h>

#include <owl/edit.h>
#include <owl/system.h>
#include <stdio.h>

using namespace owl;

namespace OwlExt {

const int EditId = 1234;

DEFINE_RESPONSE_TABLE1(TEditList, TListBox)
EV_WM_LBUTTONDBLCLK,
EV_EN_KILLFOCUS(EditId, EditKillFocus),
END_RESPONSE_TABLE;


//
// Dynamic creation.
//
TEditList::TEditList(TWindow* parent, int id, int x, int y, int w, int h, TModule* module)
:
TListBox(parent, id, x, y, w, h, module)
{
  Edit = new TEdit(this, EditId, 0, 0, 0, 0, 0);
  Edit->ModifyStyle(WS_VISIBLE,0);
  Edit->ModifyExStyle(0,WS_EX_CLIENTEDGE);

}


//
// Dialog control creation.
//
TEditList::TEditList(TWindow* parent, int id, TModule* module)
:
TListBox(parent, id, module)
{
  Edit = new TEdit(this, EditId, 0, 0, 0, 0, 0);
  Edit->ModifyStyle(WS_VISIBLE,0);
  Edit->ModifyExStyle(0,WS_EX_CLIENTEDGE);
}


TEditList::~TEditList()
{
  Destroy(IDCANCEL);
}


void TEditList::SetupWindow()
{
  TListBox::SetupWindow();

  // Give the edit box the same font as the list box.
  //
  Edit->SetWindowFont(GetWindowFont(), false);
}


//
// Display the edit box to allow user editing.
//
void TEditList::BeginEdit()
{
  int sel = GetSelIndex();
  if (sel >= 0){
    // Set the text of the edit box from the list item.
    //
    TAPointer<_TCHAR> text(new _TCHAR[GetStringLen(sel) + 1]);
    GetString(text, sel);
    Edit->SetText(text);

    // Update the position of the edit box.
    //
    TRect rect;
    GetItemRect(sel, rect);
    if(GetExStyle() & WS_EX_CLIENTEDGE)
      rect.Inflate(0,3);
    Edit->MoveWindow(rect, true);

    // Activate the edit box.
    //
    Edit->ShowWindow(SW_SHOW);
    Edit->SetSelection(0, -1);
    Edit->SetFocus();
  }
}


//
// Hide the edit box and make changes to list.
//
void TEditList::EndEdit()
{
  int sel = GetSelIndex();
  if (sel >= 0){
    // Get the text from the edit box.
    //
    int length = Edit->GetWindowTextLength() + 1;
    TAPointer<_TCHAR> text(new _TCHAR[length]);
    Edit->GetText(text, length);

    // Replace the line in the list.
    //
    InsertString(text, sel);
    DeleteString(sel + 1);
    SetSelIndex(sel);

    // Hide the edit box.
    //
    Edit->ShowWindow(SW_HIDE);
    Edit->MoveWindow(0, 0, 0, 0, true);

    // Inform the parent of the change.
    //
    if (Parent)
      Parent->SendMessage(WM_COMMAND, MAKEWPARAM(Attr.Id, EN_CHANGE), (LPARAM)GetHandle());
  }
}


//
// Double click lets user edit an item.
//
void TEditList::EvLButtonDblClk(uint modKeys, const TPoint& point)
{
  TListBox::EvLButtonDblClk(modKeys, point);
  BeginEdit();
}


//
// Whenever the edit box loses focus, stop editing.
//
void TEditList::EditKillFocus()
{
  EndEdit();
}

} // OwlExt namespace

