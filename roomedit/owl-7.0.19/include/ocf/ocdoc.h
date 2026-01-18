//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// Definition of TOcDocument Class
//----------------------------------------------------------------------------

#if !defined(OCF_OCDOC_H)
#define OCF_OCDOC_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <ocf/oclink.h>

#include <ocf/ocpart.h>



//
// Interfaces & Classes referenced
//
interface _ICLASS IStorage;

namespace ocf {

//
// Interfaces & Classes referenced
//
class _ICLASS TOcStorage;
class _ICLASS TOcApp;
class _ICLASS TOcControl;
class _ICLASS TOcView;
class _ICLASS TOcRemVie;

//
/// \class TOcDocument
// ~~~~~ ~~~~~~~~~~~
/// OC Document class, holds parts & is a owner of views
//
class _ICLASS TOcDocument {
  public:
    TOcDocument(TOcApp& app, LPCTSTR fileName = 0);
    TOcDocument(TOcApp& app, LPCTSTR fileName, IStorage * storageI);
   ~TOcDocument();

    /// \name Collection management
    /// @{
    TOcPartCollection& GetParts() {return PartCollection;}
    TOcLinkCollection& GetLinks() {return LinkCollection;}
    TOcLinkCollection& GetViews() {return LinkCollection;} // Compatibility
    /// @}

    /// \name Storage & streaming related
    /// @{
    TOcStorage* GetStorage() {return Storage;}
    void        SetStorage(IStorage* storage, bool remember = true);
    void        SetStorage(LPCTSTR path);
    bool        SaveToFile(LPCTSTR newName);
    bool        RestoreStorage();
    /// @}

    /// \name Load/Save part information
    /// @{
    bool        LoadParts();
    bool        SaveParts(IStorage* storage = 0, bool sameAsLoaded = true,
                          bool remember = true);
    void        RenameParts(IBRootLinkable * bLDocumentI);
    void        Close();
    /// @}

    /// \name Get/Set active view
    /// @{
    TOcView*    GetActiveView() {return ActiveView;}
    void        SetActiveView(TOcView* view);
    /// @}

    /// \name Get/Set document name
    /// @{
    owl::tstring  GetName() const {return Name;}
    void        SetName(const owl::tstring& newName);
    /// @}

    int         AllocPartID() {return ++PartID;}
    TOcApp&     GetOcApp() {return OcApp;}

  private:
    TOcView*       ActiveView;   ///< Active TOcView object
    TOcApp&        OcApp;        ///< Our OC application object
    TOcStorage*    Storage;      ///< root storage for embedded objects
    TOcStorage*    OrgStorage;   ///< original root storage for embedded objects

    int            PartID;
    owl::tstring     Name;
    TOcPartCollection  PartCollection;
    TOcLinkCollection  LinkCollection;

  //friend class TOcPartCollectionIter;      // To allow iterator access to collection
  //friend class TOcLinkCollectionIter;      // To allow iterator access to collection
  friend class _ICLASS TOcControl;
  friend class _ICLASS TOcView;
  friend class _ICLASS TOcRemVie;
};

} // OCF namespace

#endif  // OCF_OCDOC_H

