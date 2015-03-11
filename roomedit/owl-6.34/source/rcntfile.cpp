//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TRecentFiles
//----------------------------------------------------------------------------
#include <owl/pch.h>

//#if !defined(__GNUC__) //JJH added removal of pragma hdrstop for gcc
#pragma hdrstop
//#endif

#include <owl/framewin.h>
#include <owl/rcntfile.h>
#include <owl/pointer.h>
#include <owl/profile.h>
#include <owl/registry.h>
#include <owl/window.rh>
#include <owl/editfile.rh>
#include <stdio.h>

#include <algorithm>

#include <owl/rcntfile.rh>  //for CM_MRU_FIRST

namespace owl {

OWL_DIAGINFO;

const int CM_MRU_LAST       = CM_MRU_FIRST + TRecentFiles::MaxMenuItems;
const uint FullPathName      = TFileName::Server|TFileName::Device|TFileName::Path;
const uint FullFileName     = TFileName::Server|TFileName::Device|TFileName::Path|TFileName::File|TFileName::Ext;

#if defined(WINELIB) //JJH's work
static const tchar* CountKey        = _T("Count");
static const tchar* MruPrefix      = _T("File");
static const tchar* MenuItemDefault= _T("Default");
static const tchar* Section        = _T("Recent File List");
static const tchar* MaxCountKey    = _T("MaxCount");
#else
const tchar* CountKey        = _T("Count");
const tchar* MruPrefix      = _T("File");
const tchar* MenuItemDefault= _T("Default");
const tchar* Section        = _T("Recent File List");
const tchar* MaxCountKey    = _T("MaxCount");
#endif
const int    MaxMenuItemLen  = 255;
const int   MaxRegValueLen  = MaxMenuItemLen;
uint TRecentFiles::MruMessage = 0;

//
// Response Table to catch the items selected from the menu.
//
DEFINE_RESPONSE_TABLE1(TRecentFiles, TEventHandler)
  EV_COMMAND_AND_ID(CM_MRU_FIRST + 0, CmFile),
  EV_COMMAND_AND_ID(CM_MRU_FIRST + 1, CmFile),
  EV_COMMAND_AND_ID(CM_MRU_FIRST + 2, CmFile),
  EV_COMMAND_AND_ID(CM_MRU_FIRST + 3, CmFile),
  EV_COMMAND_AND_ID(CM_MRU_FIRST + 4, CmFile),
  EV_COMMAND_AND_ID(CM_MRU_FIRST + 5, CmFile),
  EV_COMMAND_AND_ID(CM_MRU_FIRST + 6, CmFile),
  EV_COMMAND_AND_ID(CM_MRU_FIRST + 7, CmFile),
  EV_COMMAND_AND_ID(CM_MRU_FIRST + 8, CmFile),
  EV_COMMAND_AND_ID(CM_MRU_FIRST + 9, CmFile),
  EV_COMMAND_ENABLE(CM_EXIT, CeExit),
END_RESPONSE_TABLE;


static int 
TRecentFiles__ReadMaxNum(bool useRegistry, const tstring& mruName, 
                         int defaultCount)
{
  TAPointer<tchar> srcKeyBuffer(new tchar[80]);
  TAPointer<tchar> menuText(new tchar[MaxRegValueLen]);
  uint32 maxCount = abs(defaultCount);
  if(useRegistry){
      TRegKey regKey(TRegKey::GetCurrentUser(), mruName.c_str());
      uint32 size = sizeof(uint32);
      if (regKey.QueryValue((tchar*)MaxCountKey, 0, (uint8*)&maxCount, &size) != ERROR_SUCCESS)
          maxCount = abs(defaultCount);
  }
  else{
      TProfile profile(Section, mruName.c_str());
      maxCount = profile.GetInt(MaxCountKey, maxCount);
  }

  // Force it to be a reasonable number
  //
  if (maxCount < 0) 
      maxCount = abs(defaultCount);
  if (maxCount < 0 || maxCount > TRecentFiles::MaxMenuItems) 
      maxCount = TRecentFiles::MaxMenuItems;
  return maxCount;
}



//
/// Constructor to initialize the external storage and the maximum number of items
/// to save in the most-recently-used (MRU) list.
//
TRecentFiles::TRecentFiles(const tstring& iniName, int numSavedFiles,
                           int namelen, bool useRegistry)
:
  MaxFilesToSave(0),
  MaxDispLen(namelen),
  AddedSeparator(false),
  UseRegistry(useRegistry),
  LastHMenu(0),
  MruCount(0)
{
  int filestosave = std::min(numSavedFiles, (int)MaxMenuItems);
  MruName = iniName;
  if(UseRegistry){
    MruName += _T("\\");
    MruName += Section;
  }
  else
    MruName = TFileName(iniName).Canonical();

  if(filestosave < 0)
    filestosave = TRecentFiles__ReadMaxNum(UseRegistry, MruName, filestosave);

  SetMaxMruItems(filestosave);
  MruMessage = ::RegisterWindowMessage(MruFileMessage);

  Read();
}

//
/// Deletes the allocated profile.
//
TRecentFiles::~TRecentFiles()
{
  Write();
}

//
/// Sets the maximum number of items that can be saved with this MRU.
//
void
TRecentFiles::SetMaxMruItems(int numItems)
{
  if(numItems > MaxFilesToSave){
    TFileName* items = new TFileName[numItems];
    for(int i = 0; i < MaxFilesToSave && i < numItems; i++)
      items[i] = MruNames[i];
    MruNames = items;
  }
  MaxFilesToSave = numItems;
}

//
/// Responds to a menu item selection.
//
void
TRecentFiles::CmFile(uint id)
{
  TApplication* app    = TYPESAFE_DOWNCAST(this, TApplication);
  TFrameWindow* window = app ? app->GetMainWindow() : 0;

  if (window) {
    // Foward menu selection command to specified target
    //
    window->SendMessage(MruMessage, id, 0);
  }
}

//
/// Searches the menu to find the position of a menu item.
//
int
TRecentFiles::GetMenuPos(HMENU menu, uint id)
{
  for (int i = ::GetMenuItemCount(menu) - 1; i >= 0; i--)
    if (::GetMenuItemID(menu, i) == id)
      return i;
  return -1;
}

//
/// Retrieves the menu position of the CM_EXIT menu item. Returns -1 if not found.
//
int
TRecentFiles::GetExitMenuPos(HMENU hMenu)
{
  int exitPos = GetMenuPos(hMenu, CM_EXIT);
  return exitPos;
}

//
/// Returns true if the menu has any MRU items in it.
//
bool
TRecentFiles::MruItemsInsertedIntoMenu(HMENU hMenu)
{
  int menuPos = GetMenuPos(hMenu, CM_MRU_FIRST + 0);
  return menuPos != -1;
}

//
/// Removes the MRU items from the menu.
//
void
TRecentFiles::RemoveMruItemsFromMenu(HMENU hMenu)
{
  int exitPos = GetExitMenuPos(hMenu);
  if (exitPos > 1)
    if (::GetMenuItemID(hMenu, exitPos - 1) == 0 && AddedSeparator) {
      AddedSeparator = false;
      if (LastHMenu == hMenu)
        ::RemoveMenu(hMenu, exitPos - 1, MF_BYPOSITION);
    }

  // remove MRU items
  //
  if (MruItemsInsertedIntoMenu(hMenu)) {
    for (int i = CM_MRU_FIRST; i < CM_MRU_LAST; i++) {
      int menuPos = GetMenuPos(hMenu, i);
      if (menuPos != -1) {
        ::RemoveMenu(hMenu, i, MF_BYCOMMAND);
      }
    }
  }
}

//
/// Reads external information and adds the MRU items into the menu. Adds a
/// separator between the MRU items and the exit menu item.
//
void
TRecentFiles::InsertMruItemsToMenu(HMENU hMenu)
{
  TAPointer<tchar> menuItemBuffer(new tchar[MaxMenuItemLen+4]);

  TMenu menu(hMenu);
  int exitPos       = GetExitMenuPos(hMenu);
  //int count         = GetMruCount();
  int i;
  int numItemsAdded = 0;

  tstring curdir = TFileName(TFileName::CurrentDir).GetParts(FullPathName);

  for (i = 0; i < MruCount; i++) {
    //if(!MruNames[i].IsValid()) //?????????????
    //  break;

    numItemsAdded++;

    tstring curname = MruNames[i].GetParts(FullFileName);
    if(curdir == MruNames[i].GetParts(FullPathName))
      curname = MruNames[i].GetParts(TFileName::File|TFileName::Ext);
    curname = TFileName(curname).Squeezed(MaxDispLen, true);
    // insert mnemonic + the file name
    if((i+1) >= 10)
      wsprintf(menuItemBuffer, _T("%d&%d %s"), (i+1)/10, (i+1)%10, curname.c_str());
    else
      wsprintf(menuItemBuffer, _T("&%d %s"), (i+1), curname.c_str());

    menu.InsertMenu(exitPos + i, MF_BYPOSITION | MF_STRING, CM_MRU_FIRST + i, menuItemBuffer);
  }

  if (numItemsAdded > 0) {
    if (AddedSeparator == false) {
      LastHMenu = hMenu;
      AddedSeparator = true;
      menuItemBuffer[0] = 0;
      ::InsertMenu(hMenu, exitPos + numItemsAdded, MF_BYPOSITION|MF_SEPARATOR, 0, menuItemBuffer);
    }
  }
}

//
/// Reads information in the TProfile to display the menu choices.
//
void
TRecentFiles::CeExit(TCommandEnabler& ce)
{
  ce.Enable(true);

  TMenuItemEnabler* me = TYPESAFE_DOWNCAST(&ce, TMenuItemEnabler);
  if (me == 0)
    return;

  HMENU hMenu = me->GetMenu();
  RemoveMruItemsFromMenu(hMenu);
  InsertMruItemsToMenu(hMenu);
}

/// Retrieves the text of the choice based on the ID.
tstring
TRecentFiles::GetMenuText(int id)
{
  id -= CM_MRU_FIRST;
  if (id < 0 || MruCount <= id)
    return tstring(_T(""));
  return MruNames[id].GetParts(FullFileName);
}

//
/// Retrieves the text of the choice based on the ID.
//
bool
TRecentFiles::GetMenuText(int id, LPTSTR text, int maxTextLen)
{
  id -= CM_MRU_FIRST;
  //int count = GetMruCount();
  if (id < 0 || MruCount <= id)
    return false;
  tstring menuText = MruNames[id].GetParts(FullFileName);
  if(::_tcslen(menuText.c_str()) > (size_t)maxTextLen){
    _tcsncpy(text, menuText.c_str(), maxTextLen-1);
    text[maxTextLen] = _T('\0');
  }
  else
    ::_tcscpy(text, menuText.c_str());
  return true;
}

//
/// Saves the menu choice into the profile.
//
void
TRecentFiles::SaveMenuChoice(LPCTSTR text)
{
  int index = GetMruItemIndex(text);
  RemoveMruIndex(index);
  AddMruItem(text);
}

//
// Remove the menu choice.
//
void
TRecentFiles::RemoveMenuChoice(LPCTSTR text)
{
  RemoveMruIndex(GetMruItemIndex(text));
}

//
/// Removes the MRU item at index. Shuffles the items below index up.
//
void
TRecentFiles::RemoveMruIndex(int index)
{
  //int count = GetMruCount();
  if (index < 0 || MruCount <= index)
    return;

  if (index != MruCount - 1) {
    // shuffle so the item to be deleted is at index count-1
    //
    for(int i = index; i < MruCount - 1; i++)
      MruNames[i] = MruNames[i+1];
  }

  // delete the last item
  //
  MruNames[MruCount-1] = _T("");
  MruCount--; // decrease MRUCount
}

//
/// Adds an item to the top of the MRU list. If there is a duplicate, the item is
/// moved from its current position to the top of the list.
//
void
TRecentFiles::AddMruItem(LPCTSTR text)
{
  if (MaxFilesToSave <= 0) 
    return;
  if(MruCount < MaxFilesToSave)
    MruCount++;
  if (MruCount > 1){
    // Shuffle items down to make room at index 0
    //
    for (int i = MruCount - 2; i >= 0; i--)
       MruNames[i+1] = MruNames[i];
  }

  // Add item to the top of the list
  //
   MruNames[0] = TFileName(text).Canonical();
}

//
/// Returns true if there are any items in the MRU list that match the text.
//
bool
TRecentFiles::ExistMruItem(LPCTSTR text)
{
  return GetMruItemIndex(text) != -1;
}

//
/// Returns the index of the MRU item containing text. Returns -1 if not found.
//
int
TRecentFiles::GetMruItemIndex(LPCTSTR text)
{
  TFileName filename(TFileName(text).Canonical());
  for (int i = 0; i < MruCount; i++) {
    if(MruNames[i] == filename)
      return i;
  }
  return -1;
}
//
// Read Data from registry or file
//
void
TRecentFiles::Read()
{
  TAPointer<tchar> srcKeyBuffer(new tchar[80]);
  TAPointer<tchar> menuText(new tchar[MaxRegValueLen]);
  if(UseRegistry){
    TRegKey regKey(TRegKey::GetCurrentUser(), MruName.c_str());
    uint32 size = sizeof(uint32);
    if(regKey.QueryValue((tchar*)CountKey, 0, (uint8*)&MruCount, &size) != ERROR_SUCCESS)
      return;
    if (MruCount > MaxFilesToSave)
      MruCount = MaxFilesToSave;
    for (int i = 0; i < MruCount; i++){
      wsprintf(srcKeyBuffer, _T("%s%d"), MruPrefix, i);
      uint32 size = MaxRegValueLen;
      if(regKey.QueryValue((tchar*)srcKeyBuffer, 0, (uint8*)(tchar*)menuText, &size) != ERROR_SUCCESS)
        break;
      MruNames[i] = (tchar*)menuText;
    }
  }
  else{
    TProfile profile(Section, MruName.c_str());
    MruCount = profile.GetInt(CountKey, 0);
    if (MruCount > MaxFilesToSave)
      MruCount = MaxFilesToSave;
    for (int i = 0; i < MruCount; i++){
      wsprintf(srcKeyBuffer, _T("%s%d"), MruPrefix, i);
      profile.GetString(srcKeyBuffer, menuText, MaxMenuItemLen, MenuItemDefault);
      MruNames[i] = (tchar*)menuText;
    }
  }
}
//
// Write data to registry or file
//
void
TRecentFiles::Write()
{
  TAPointer<tchar> dstKeyBuffer(new tchar[80]);
  if(UseRegistry){
    //1. delete all old MRU
    TRegKey::GetCurrentUser().DeleteKey(MruName.c_str());
    // 2. create new mru
    TRegKey regKey(TRegKey::GetCurrentUser(), MruName.c_str());
    regKey.SetValue(CountKey, MruCount);
    for (int i = 0; i < MruCount; i++){
      wsprintf(dstKeyBuffer, _T("%s%d"), MruPrefix, i);
      tstring fname = MruNames[i].GetParts(FullFileName);
      regKey.SetValue((tchar*)dstKeyBuffer, REG_SZ,  (const uint8*)fname.c_str(), fname.length() * sizeof(tchar));
    }
  }
  else{
    TProfile profile(Section, MruName.c_str());
    profile.WriteInt(CountKey, MruCount);
    for (int i = 0; i < MruCount; i++){
      wsprintf(dstKeyBuffer, _T("%s%d"), MruPrefix, i);
      profile.WriteString(dstKeyBuffer, MruNames[i].GetParts(FullFileName));
    }
  }
}

} // OWL namespace

/* ========================================================================== */

