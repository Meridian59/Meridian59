//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of classes TPickListPopup & TPickListDialog.
///
/// Both of these modal windows allow the selction of a string from a list.
/// TPickListPopup uses a lean popup menu for a quick selection, while
/// TPickListDialog uses a listbox in a dialog with OK and Cancel buttons.
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/picklist.h>
#include <owl/picklist.rh>

using namespace std;

namespace owl {

OWL_DIAGINFO;

//
/// Constructs a popup menu.
/// Adds the title at the top of the menu.
//
TPickListPopup::TPickListPopup(TWindow* parent, LPCTSTR title)
:
  TWindow(parent, 0, 0),
  Count(0)
{
  PRECONDITION(Popup.GetHandle());

  if (title) {
    Popup.AppendMenu(MF_GRAYED, 0, title);
    Popup.AppendMenu(MF_SEPARATOR);
  }
}

//
/// String-aware overload
//
TPickListPopup::TPickListPopup(TWindow* parent, const tstring& title)
  : TWindow(parent, 0, 0), Count(0)
{
  PRECONDITION(Popup.GetHandle());
  if (!title.empty()) 
  {
    Popup.AppendMenu(MF_GRAYED, 0, title);
    Popup.AppendMenu(MF_SEPARATOR);
  }
}

//
/// Constructs a popu menu.
/// Adds the title from resource at the top of the menu.
//
TPickListPopup::TPickListPopup(TWindow* parent, uint titleId)
:
  TWindow(parent, 0, 0),
  Count(0)
{
  PRECONDITION(Popup.GetHandle());

  tchar buf[_MAX_PATH];
  if (!titleId || !GetApplication()->LoadString(titleId, buf, _MAX_PATH))
    return;

  Popup.AppendMenu(MF_GRAYED, 0, buf);
  Popup.AppendMenu(MF_SEPARATOR);
}

//
/// Clears all strings from the pop-up.
//
void
TPickListPopup::ClearStrings()
{
  while (Count)
    Popup.RemoveMenu(--Count, MF_BYCOMMAND);
}

//
/// Adds a string to the pop-up.
//
int
TPickListPopup::AddString(LPCTSTR str)
{
  PRECONDITION(Popup.GetHandle());
  Popup.AppendMenu(MF_ENABLED, Count++, str);
  return Count;
}

//
/// Displays the pop-up menu and returns the command ID of the menu item the user
/// selected.
//
int
TPickListPopup::Execute()
{
  // Create the hidden window
  //
  Create();

  // Reset the index result
  //
  Result = -2;

  // Grab current mouse location
  //
  TPoint cursorLoc;
  GetCursorPos(cursorLoc);

  // Display section & cleanup
  //
  Popup.TrackPopupMenu(0, cursorLoc, 0, GetHandle());
  if(!IsWindow())
    return -2;
//  while (Popup.GetMenuItemCount())     // !CQ Menus clean up their own items
//    Popup.DeleteMenu(0, MF_BYPOSITION);

  // Dispatch any pending WM_COMMAND stuck in the message queue.
  //
  CHECK(GetApplication());
  GetApplication()->PumpWaitingMessages();

  return Result;
}

//
/// Generic handler for WM_COMMAND messages.
/// Result is set to the id of the menu item clicked.
//
TResult
TPickListPopup::EvCommand(uint id, THandle /*hWndCtl*/, uint /*notifyCode*/)
{
//  WARNX(OwlDocView, id > Count, 0, "TPickListPopup index invalid");
  Result = id;
  return 0;
}

//----------------------------------------------------------------------------

//
//
//
DEFINE_RESPONSE_TABLE1(TPickListDialog, TDialog)
  EV_COMMAND(IDOK,     CmOK),
  EV_COMMAND(IDCANCEL, CmCancel),
  EV_LBN_DBLCLK(IDC_LIST, CmOK),
END_RESPONSE_TABLE;

//
/// Initialize the dialog.
/// Sets the initial selection of the listbox.
/// Allocate strings if necessary.
//
TPickListDialog::TPickListDialog(TWindow*      parent,
                                 TStringArray* strings,
                                 int           initialSelection,
                                 TResId        templateId,
                                 LPCTSTR       title,
                                 TModule*      module)
:
  TDialog(parent, templateId ? templateId : TResId(IDD_PICKLISTDIALOG), module),
  List(this, IDC_LIST, module),
  Result(initialSelection),
  Strings(strings ? strings : new TStringArray),
  NewedStrings(!strings)
{
  if (title)
    SetCaption(title);
}

//
/// String-aware overload
//
TPickListDialog::TPickListDialog(
  TWindow* parent,
  TStringArray* strings,
  int initialSelection,
  TResId templateId,
  const tstring& title,
  TModule* module
  )
  : TDialog(parent, templateId ? templateId : TResId(IDD_PICKLISTDIALOG), module),
  List(this, IDC_LIST, module),
  Result(initialSelection),
  Strings(strings ? strings : new TStringArray),
  NewedStrings(!strings)
{
  if (!title.empty())
    SetCaption(title);
}

//
/// Deletes any allocated string.
//
TPickListDialog::~TPickListDialog()
{
  if (NewedStrings)
    delete Strings;
}

//
/// Clears all strings from the list.
//
void
TPickListDialog::ClearStrings()
{
  Strings->Flush();
  if (List.GetHandle())
    List.ClearList();
}

//
/// Adds a string to the Strings list and to the List box if it has already been
/// created.
//
int
TPickListDialog::AddString(LPCTSTR str)
{
  Strings->Add(str);
  if (List.GetHandle())
    List.AddString(str);
  return Strings->Size()-1;
}

//
// Add a string to a listbox, passed via the args from ForEach.

static void
AddToList(tstring& str, void* param)
{
  TListBox* list = REINTERPRET_CAST(TListBox*,param);
  if (list && list->GetHandle()) // just in case
    list->AddString(str);
}
//
/// Override from TDialog.
/// Adds each string into the listbox.
//
void
TPickListDialog::SetupWindow()
{
  TDialog::SetupWindow();

  // Add each string in Strings to the List box
  //
  Strings->ForEach(owl::AddToList, &List);

  // Set inital state of listbox
  //
  if (List.GetCount() > Result) {
    List.SetSelIndex(Result);
    List.SetCaretIndex(Result, false);
  }
}

//
/// User-selected OK. Gets selection from the listbox and returns it.
//
void
TPickListDialog::CmOK()
{
  int index = List.GetSelIndex();
  if (index >= 0) {
    // Save the selection index.
    Result = index;
  }
  CloseWindow(Result);
}

//
/// User-selected Cancel. Returns a value less than zero. Negative One (-1) cannot
/// be used since it signals a dialog failure.
//
void
TPickListDialog::CmCancel()
{
  CloseWindow(-2);
}


} // OWL namespace
/* ========================================================================== */

