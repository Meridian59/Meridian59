//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TFileDocument
//----------------------------------------------------------------------------

#if !defined(OWL_FILEDOC_H)
#define OWL_FILEDOC_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/docview.h>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup docview
/// @{

/// \class TFileDocument
// ~~~~~ ~~~~~~~~~~~~~
/// Derived from TDocument, TFileDocument opens and closes views and provides stream
/// support for views. TFileDocument has member functions that continue to process
/// FileNew and FileOpen messages after a view is constructed. You can add support
/// for specialized file types by deriving classes from TFileDocument. TFileDocument
/// makes this process easy by hiding the actual process of storing file types.
//
class _OWLCLASS TFileDocument : public TDocument {
  public:
		/// Contains constants that define the following properties of the document:
    enum TFileDocProp {
      PrevProperty = TDocument::NextProperty-1,
      CreateTime,        ///< The time the view or document was created (FILETIME)
      ModifyTime,        ///< The time the view or document was modified (FILETIME)
      AccessTime,        ///< The time the view or document was last accessed (FILETIME)
      StorageSize,       ///< An unsigned long containing the storage size (ulong)
      FileHandle,        ///< The platform file handle (HFILE if Windows)
      NextProperty,
    };

    TFileDocument(TDocument* parent = 0);
   ~TFileDocument();

    // Implement virtual methods of TDocument
    //
    bool        Open(int mode, LPCTSTR path=0);
    using TDocument::Open; ///< String-aware overload
    bool        Close();
    TInStream*  InStream(int mode, LPCTSTR strmId=0);
    using TDocument::InStream; ///< String-aware overload
    TOutStream* OutStream(int mode, LPCTSTR strmId=0);
    using TDocument::OutStream; ///< String-aware overload
    bool        Commit(bool force = false);
    bool        Revert(bool clear = false);
    bool        IsOpen();

    int         FindProperty(LPCTSTR name);  ///< return index
    using TDocument::FindProperty; ///< String-aware overload
    int         PropertyFlags(int index);
    LPCTSTR      PropertyName(int index);
    int         PropertyCount();
    int         GetProperty(int index, void * dest, int textlen=0);
    bool        SetProperty(int index, const void * src);

    // Additional methods for file document
    //
    bool        Open(HFILE fhdl);     ///< open on existing file handle

  protected:
    HFILE OpenThisFile(int omode, const tstring& fileName, std::streampos* pseekpos);
    void  CloseThisFile(HFILE fhdl, int omode);

  protected_data:
    HFILE    FHdl;         ///< file handle if held open at the document level

  private:                 // cached info for property access
    bool     InfoPresent;
    ulong    FileLength;

    FILETIME FileCreateTime;
    FILETIME FileAccessTime;
    FILETIME FileUpdateTime;

  friend class _OWLCLASS TFileInStream;
  friend class _OWLCLASS TFileOutStream;
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TFileDocument,1);
  DECLARE_STREAMABLE_OWL(TFileDocument,1);
};
/// @{
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TFileDocument );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Constructs a TFileDocument object with the optional parent document.
inline TFileDocument::TFileDocument(TDocument* parent)
:
  TDocument(parent), FHdl(HFILE_ERROR), InfoPresent(false)
{
}

//
/// Destroys a TFileDocument object.
inline TFileDocument::~TFileDocument()
{
}

//
/// Is nonzero if the document or any streams are open.
//
inline bool TFileDocument::IsOpen()
{
  return FHdl != HFILE_ERROR || TDocument::IsOpen();
}

//
/// Return the number of property support by this document.
/// \note The property count includes properties inherited from base document
///       classes.
//
inline int TFileDocument::PropertyCount()
{
  return NextProperty - 1;
}


} // OWL namespace


#endif  // OWL_FILEDOC_H
