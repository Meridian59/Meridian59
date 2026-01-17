//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------

#if !defined(OCF_OLEDOC_H)
#define OCF_OLEDOC_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <ocf/stgdoc.h>
#include <ocf/ocfevent.h>
#include <ocf/ocdoc.h>



namespace ocf {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
#include <owl/preclass.h>

class _OCFCLASS TOleWindow;

//
/// \class TOleDocument
// ~~~~~ ~~~~~~~~~~~~
/// Derived from TStorageDocument, TOleDocument implements the document half of the
/// Doc/View pair. It manages the document's data while the corresponding TOleView
/// object determines how the data is displayed on the screen. Basically,
/// TOleDocument is a TStorageDocument with a knowledge of TOcDocument through its
/// pointer to TOcDocument.
///
/// TOleDocument is responsible for creating compound documents, closing documents,
/// reading documents from storage, and writing documents to storage. In the case of
/// a server, the document consists of a single object. In the case of a container,
/// the document can consist of one or more embedded objects (also referred to as
/// parts).
///
/// To accomplish these tasks, TOleDocument talks to the underlying ObjectComponents
/// classes through the use of functions such as GetOcApp, GetOcDoc, and SetOcDoc.
//
class _OCFCLASS TOleDocument : public TStorageDocument {
  public:
    TOleDocument(owl::TDocument* parent = 0);
   ~TOleDocument();

    /// \name Accessors
    /// @{
    void          SetOcDoc(TOcDocument* doc);
    TOcDocument*  GetOcDoc();
    TOcApp*       GetOcApp();
    /// @}

    /// \name Overridables:
    /// @{
    //   owl::TDocument:
    bool          Commit(bool force);
    bool          Open(int mode, LPCTSTR path = 0);
    bool          Close();
    bool          InitDoc();
    bool          CanClose();
    bool          Revert(bool clear);

    //   TStorageDocument:
    bool          SetStorage(IStorage* stg, bool remember = true);
    bool          RestoreStorage();
    bool          ReleaseDoc();

    //   TOleDocument:
    virtual void  PreOpen();
    virtual bool  Read();
    virtual bool  Write();
    virtual bool  CommitSelection(TOleWindow& oleWin, void* userData);
    /// @}

    /// \name Utilities
    /// @{
    bool PathChanged();
    void OleViewClose();
    /// @}

  private:
    TOcDocument*      OcDoc;    ///< OCF document partner
    bool              Closing;  ///< shutdown in progress
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Sets the ObjectComponents document associated with this TOleDocument object.
inline void TOleDocument::SetOcDoc(TOcDocument* doc)
{
  OcDoc = doc;
}

//
/// Returns the ObjectComponents document associated with this TOleDocument object.
inline TOcDocument* TOleDocument::GetOcDoc()
{
  return OcDoc;
}

//
/// Virtual function to be overriden in TOleDocument-derived class which serves or
/// supports linking to portions of a document's data.
//
inline bool TOleDocument::CommitSelection(TOleWindow& oleWin, void* userData)
{
  return true;
}


} // OCF namespace


#endif
