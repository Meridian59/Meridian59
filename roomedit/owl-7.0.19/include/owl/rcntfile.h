//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of TRecentFiles class
//----------------------------------------------------------------------------

#if !defined(OWL_RECENTFILE_H)
#define OWL_RECENTFILE_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/eventhan.h>
#include <owl/filename.h>


namespace owl {

//
/// Derived classes should catch the following registered message to know
/// when an item in the recent list has been selected.
//
#define MruFileMessage _T("MRUFILEMESSAGE") ///< String that OWL uses to register the message used by the MRU classes.

#include <owl/preclass.h>

//
/// \class TRecentFiles
// ~~~~~ ~~~~~~~~~~~~
/// TRecentFiles implements a most-recent files list, designed to be mixed in with
/// TApplication. The list is appended to the menu with CM_FILEOPEN and CM_FILECLOSE
/// options.
//
class _OWLCLASS TRecentFiles : virtual public TEventHandler
{
  public:
    enum { MaxMenuItems = 10 };

    TRecentFiles(const tstring& iniOrKeyName, int numSavedFiles = MaxMenuItems,
                 int namelen = 30, bool useRegistry=false, bool keepFullNameInMenu = true);
    virtual ~TRecentFiles();

    void          SaveMenuChoice(LPCTSTR text);
    void SaveMenuChoice(const tstring& text) {SaveMenuChoice(text.c_str());}
    void          RemoveMenuChoice(LPCTSTR text);
    void RemoveMenuChoice(const tstring& text) {RemoveMenuChoice(text.c_str());}
    bool          GetMenuText(int id, LPTSTR text, int maxTextLen);
    tstring    GetMenuText(int id);
    void          SetMaxMruItems(int maxValue);
    void           SetDispLength(const int);
    void           EnableRegistry(bool enable = true);
    int           GetMruCount();

  protected:
    void          CeExit(TCommandEnabler& ce);
    void          CmFile(uint id);

    // Do the MRU items already exists in the menu?
    //
    bool          MruItemsInsertedIntoMenu(HMENU hMenu);
    void          RemoveMruItemsFromMenu(HMENU hMenu);
    void          InsertMruItemsToMenu(HMENU hMenu);
    int           GetMenuPos(HMENU hMenu, uint id);
    int           GetExitMenuPos(HMENU hMenu);

    // Profile methods
    //
    //int           GetMruCount();
    void          RemoveMruIndex(int index);
    void          AddMruItem(LPCTSTR text);
    void AddMruItem(const tstring& text) {AddMruItem(text.c_str());}
    bool          ExistMruItem(LPCTSTR text);
    bool ExistMruItem(const tstring& text) {return ExistMruItem(text.c_str());}
    int           GetMruItemIndex(LPCTSTR text);
    int GetMruItemIndex(const tstring& text) {return GetMruItemIndex(text.c_str());}

  private:
    // Read information from the .INI file or Registry
    //
    void          Read();
    // Write information from the .INI file or Registry
    //
    void          Write();

    static uint           MruMessage;
    int                   MaxFilesToSave;
    int                    MaxDispLen;
    bool                  AddedSeparator;
    bool                  UseRegistry;
    tstring             MruName;
    HMENU                 LastHMenu;
    TAPointer<TFileName>   MruNames;
    int                   MruCount;
    bool                  keepFullNameInMenu;  //when true (default), the name of the file is kept full. 
                                                //when false, the name is condensed, if it is too big

  DECLARE_RESPONSE_TABLE(TRecentFiles);
  DECLARE_CASTABLE;
};
#include <owl/posclass.h>


//-------------------
// inlines
inline void  TRecentFiles::SetDispLength(const int newLen){
  MaxDispLen = newLen;
}

inline void TRecentFiles::EnableRegistry(bool enable){
  UseRegistry = enable;
}

/// Returns the number of files that are currently in the MRU list.
inline int TRecentFiles::GetMruCount(){
  return MruCount;
}

} // OWL namespace


#endif  // OWL_RECENTFILE_H
