//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TEditFile, a text edit which can find/replace and
/// read/write from/to a file.
//----------------------------------------------------------------------------

#if !defined(OWL_EDITFILE_H)
#define OWL_EDITFILE_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/editsear.h>
#include <owl/opensave.h>
#include <owl/editfile.rh>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup ctrl
/// @{
/// \class TEditFile
// ~~~~~ ~~~~~~~~~
/// TEditFile is a file-editing window. TEditFile's data members and member
/// functions manage the file dialog box and automatic responses for file commands
/// such as Open, Read, Write, Save, and SaveAs. TEditFile is streamable.
//
class _OWLCLASS TEditFile : public TEditSearch {
  public:
    TEditFile(TWindow*        parent = 0,
              int             id = 0,
              LPCTSTR         text = 0,
              int x = 0, int y = 0, int w = 0, int h = 0,
              LPCTSTR         fileName = 0,
              TModule*        module = 0);

    TEditFile(
      TWindow* parent,
      int id,
      const tstring& text,
      int x = 0, int y = 0, int w = 0, int h = 0,
      const tstring& fileName = tstring(),
      TModule* module = 0);

    TEditFile(TWindow*   parent,
              int        resourceId,
              TModule&   module);
              
   ~TEditFile();

    virtual bool  CanClear();
    virtual bool  CanClose();

    virtual void  NewFile();
    virtual void  Open();
    virtual bool  Read(LPCTSTR fileName = 0);
    bool Read(const tstring& fileName) {return Read(fileName.c_str());}
    virtual void  ReplaceWith(LPCTSTR fileName);
    void ReplaceWith(const tstring& fileName) {ReplaceWith(fileName.c_str());}
    virtual bool  Save();
    virtual bool  SaveAs();
    virtual bool  Write(LPCTSTR fileName=0);
    bool Write(const tstring& fileName) {return Write(fileName.c_str());}

    LPCTSTR       GetFileName() const;
    void          SetFileName(LPCTSTR fileName);
    void SetFileName(const tstring& fileName) {SetFileName(fileName.c_str());}

    TOpenSaveDialog::TData& GetFileData();
    void   SetFileData(const TOpenSaveDialog::TData& fd);

  protected:  // !CQ these command handlers were public...

    // Command response functions
    //
    void          CmFileNew();     ///< CM_FILENEW
    void          CmFileOpen();    ///< CM_FILEOPEN
    void          CmFileSave();    ///< CM_FILESAVE
    void          CmFileSaveAs();  ///< CM_FILESAVEAS

    // Command enabler functions
    //
    void          CmSaveEnable(TCommandEnabler& commandHandler);

  protected:
    // Override virtual functions defined by TWindow
    //
    void          SetupWindow();

  public_data:
/// Contains information about the user's file open or save selection.
    TOpenSaveDialog::TData FileData;
    	
/// Contains the name of the file being edited.
    LPTSTR FileName;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TEditFile(const TEditFile&);
    TEditFile& operator =(const TEditFile&);

  DECLARE_RESPONSE_TABLE(TEditFile);
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TEditFile, 1);
  DECLARE_STREAMABLE_OWL(TEditFile, 1);
};
/// @}

// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TEditFile );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// This constructor, which aliases an Edit control created from a
/// dialog resource is 'unconventional' in that it expects a TModule reference
/// instead of the traditional 'TModule* = 0'. This is, however, to avoid
/// ambiguities between the two forms of constructor. Since it is traditionally
/// created as a child of a TDialog-derived class, you can simply use the module of
/// the parent object.
///      For example:
/// \code
/// TMyDialog::TMyDialog(....)
/// { 
///   edit = new TEditFile(this, ID_EDIT1, *GetModule());
/// }
/// \endcode
//
inline
TEditFile::TEditFile(TWindow* parent, int resourceId, TModule& module)
          :TEditSearch(parent, resourceId, module),
           FileName(0)
{
}

/// Return the filename for this buffer.
//
inline LPCTSTR TEditFile::GetFileName() const {
  return FileName;
}

//
/// Return the FileData data member used for the common dialogs.
//
inline TOpenSaveDialog::TData& TEditFile::GetFileData() {
  return FileData;
}

//
inline void TEditFile::SetFileData(const TOpenSaveDialog::TData& fd) {
  FileData = fd;
}

//
/// Calls NewFile() in response to an incoming File New command with a CM_FILENEW
/// command identifier.
//
inline void TEditFile::CmFileNew() {
  NewFile();
}

//
/// Calls Open() in response to an incoming File Open command with a CM_FILEOPEN
/// command identifier.
//
inline void TEditFile::CmFileOpen() {
  Open();
}

//
/// Calls Save() in response to an incoming File Save command with a CM_FILESAVE
/// command identifier.
//
inline void TEditFile::CmFileSave() {
  Save();
}

//
/// Calls SaveAs() in response to an incoming File SaveAs command with a CM_FILESAVEAS
/// command identifier.
//
inline void TEditFile::CmFileSaveAs() {
  SaveAs();
}


} // OWL namespace


#endif  // OWL_EDITFILE_H
