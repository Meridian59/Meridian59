//------------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TDocManager
//------------------------------------------------------------------------------

#if !defined(OWL_DOCMANAG_H)
#define OWL_DOCMANAG_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/docview.h>
#include <owl/applicat.h>
#include <owl/doctpl.h>
#include <owl/opensave.h>


namespace owl {

/// \addtogroup docview
/// @{


//
// Class modifier that can be used for docview classes that may be in a dll
//
#define _DOCVIEWCLASS _USERCLASS

//
/// \name Definitions of dmXxx document manager operational mode flags
/// @{
const int dmSDI        = 0x0001;  ///< Does not support multiple open documents
const int dmMDI        = 0x0002;  ///< Supports multiple open documents
const int dmMenu       = 0x0004;  ///< Set IDs for file menu
const int dmSaveEnable = 0x0010;  ///< Enable FileSave even if doc is unmodified
const int dmNoRevert   = 0x0020;  ///< Disable FileRevert menu item
/// @}

//
/// Flags which are stripped off when invoking Open/Save dialogs [See
/// SelectDocPath() method for more information ]
//
const long dtProhibited = OFN_ENABLEHOOK | 
                          OFN_ENABLETEMPLATE | 
                          OFN_ALLOWMULTISELECT |
                          OFN_ENABLETEMPLATEHANDLE;

//
/// Definitions of dnXxx document/view message notifications
//
enum {
  dnCreate,                 ///< New document or view has been created
  dnClose,                  ///< Document or view has been closed
  dnRename                  ///< Document has been renamed
};

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TDocManager
// ~~~~~ ~~~~~~~~~~~
/// TDocManager creates a document manager object that manages the list of current
/// documents and registered templates, handles standard file menu commands, and
/// displays the user-interface for file and view selection boxes. To provide
/// support for documents and views, an instance of TDocManager must be created by
/// the application and attached to the application.
/// 
/// The document manager normally handles events on behalf of the documents by using
/// a response table to process the standard CM_FILENEW, CM_FILEOPEN, CM_FILECLOSE,
/// CM_FILESAVE, CM_FILESAVEAS, CM_FILEREVERT, CM_FILEPRINT, CM_FILEPRINTERSETUP,
/// and CM_VIEWCREATE File menu commands. In response to a CM_FILENEW or a
/// CM_FILEOPEN command, the document manager creates the appropriate document based
/// on the user's selections. In response to the other commands, the document
/// manager determines which of the open documents contains the view associated with
/// the window that has focus. The menu commands are first sent to the window that
/// is in focus and then through the parent window chain to the main window and
/// finally to the application, which forwards the commands to the document manager.
/// 
/// When you create a TDocManager or a derived class, you must specify that it has
/// either a multi-document (dmMDI) or single-document (dmSDI) interface. In
/// addition, if you want the document manager to handle the standard file commands,
/// you must OR dmMDI or dmSDI with dmMenu.
/// 
/// You can also enable or disable the document manager menu options by passing
/// dmSaveEnable or dmNoRevert in the constructor. If you want to enable the
/// File|Save menu option if the document is unmodified, pass the dmSaveEnable flag
/// in the constructor. To disable the "Revert to Saved" menu option, pass
/// dmNoRevert in the constructor.
/// 
/// When the application directly creates a new document and view, it can attach the
/// view to its frame window, create MDI children, float the window, or create a
/// splitter. However, when the document manager creates a new document and view
/// from the File|Open or File|New menu selection, the application doesn't control
/// the process. To give the application control, the document manager sends
/// messages after the new document and view are successfully created. Then, the
/// application can use the information contained in the template to determine how
/// to install the new document or view object.
//
class _OWLCLASS TDocManager : virtual public TEventHandler,
                              virtual public TStreamableBase {
  public:
    TDocManager(int mode, TApplication* app,
                TDocTemplate*& templateHead = DocTemplateStaticHead);
    virtual ~TDocManager();

    // Retrieve template info: count, list or descriptions
    //
    int                   GetTemplateCount(TDocument* ofDocType = 0,
                                           TView* ofViewType = 0);
    virtual int           GetNewTemplates(TDocTemplate** tplList, int size,
                                          bool newDoc);
    virtual int           GetSaveTemplates(TDocTemplate** tplList, int size,
                                           TDocument& doc, bool sameDoc);
    virtual int           GetViewTemplates(TDocTemplate** tplList, int size,
                                           TDocument& doc);
    virtual int           GetTemplateDescription(TDocTemplate** tpllist,
                                                 int tplcount, tchar* buff = 0,
                                                 int size = 0);

    // Create (construct & initialize) document and views
    //
    virtual TDocument*    CreateAnyDoc(LPCTSTR path, long flags = 0);
    //Sirma (Krasi)
    virtual TDocument*    CreateDoc(TDocTemplate* tpl, LPCTSTR path,
                                    TDocument* parent=0, long flags=0);
    TDocument*            InitDoc(TDocument* doc, LPCTSTR path,
                                  long flags);
    // Sirma (Kalin)
    TDocument*            CreateDummyDoc();

    virtual TView*        CreateAnyView(TDocument& doc,long flags = 0);
    TView*                CreateView(TDocument& doc, TDocTemplate* tpl=0);

    // Save document
    //
    bool                  SelectSave(TDocument& doc);
    virtual TDocTemplate* SelectAnySave(TDocument& doc, bool samedoc = true);
    virtual bool          FlushDoc(TDocument& doc); // Attempt to update changes

    // Find related template or document
    //
    virtual TDocTemplate* MatchTemplate(LPCTSTR path);
    virtual TDocument*    GetCurrentDoc();  ///< Return doc with focus, else 0
    TDocument*            FindDocument(LPCTSTR path); ///< 0 if not found
    static  bool          IsAMatch(LPCTSTR path, LPCTSTR fltr);

    // Accessors
    //
    TApplication*         GetApplication() const;
    int&                  GetUntitledIndex();
    TDocTemplate*         GetTemplateList() const;
    TDocument::List&      GetDocList();

    // Check Document Manager's state
    //
    bool                  IsFlagSet(int flag) const;
    int                   GetModeFlags() const;

    // Get or set additional OPENFILENAME flags used for Open and Save dialogs.
    // See SelectDocPath for details.
    //
    long                  GetAdditionalFileDialogFlags() const;
    void                  SetAdditionalFileDialogFlags(long);

    // Manipulate template List [add, remove, etc]
    //
    TDocTemplate*         GetNextTemplate(TDocTemplate* tpl) const;
    void                  RefTemplate(TDocTemplate&);    ///< add template ref
    void                  UnRefTemplate(TDocTemplate&);  ///< drop template ref
    void                  DeleteTemplate(TDocTemplate&); ///< remove from list
    void                  AttachTemplate(TDocTemplate&); ///< append to list

    // Primary event handlers, public to allow direct invocation from app
    // For backward compatibility call virtual member instead
    void                  CmFileOpen();
    void                  CmFileNew();
    void                  CmFileClose();
    void                  CmFileSave();
    void                  CmFileSaveAs();
    void                  CmFileRevert();
    void                  CmViewCreate();

    // Overridable handlers
    virtual void          FileOpen();
    virtual void          FileNew();
    virtual void          FileClose();
    virtual void          FileSave();
    virtual void          FileSaveAs();
    virtual void          FileRevert();
    virtual void          ViewCreate();

    // Overrideable document manager UI functions
    //
    virtual uint          PostDocError(TDocument& doc, uint sid,
                                       uint choice = MB_OK);
    virtual void          PostEvent(int id, TDocument& doc); ///< changed doc status
    virtual void          PostEvent(int id, TView& view);    ///< changed view status
    virtual void          UpdateFileMenu(TMenu& menubar);

    // Delegated methods from TApplication
    //
    void                  EvPreProcessMenu(HMENU hMenu);
    bool                  EvCanClose();
    void                  EvWakeUp();

  protected:

    /// Index of last untitled document
    //
    int                   UntitledIndex;

    /// Next ID to be assigned to a view
    //
    uint                  NextViewId;  

    /// Helper routine used by constructor(s)
    //
    void                  Init(int mode, TApplication* app, 
                               TDocTemplate*& templateHead);

    // Overrideable document manager UI functions
    //
    virtual int           SelectDocType(TDocTemplate** tpllist,
                                        int tplcount);
    virtual int           SelectDocPath(TDocTemplate** tpllist, int tplcount,
                                        tchar * path, int buflen, 
                                        long flags, bool save=false, TDocument* doc = NULL);
    virtual int           SelectViewType(TDocTemplate** tpllist, 
                                         int tplcount);

    // Overrideable method invoked just before DocManager creates a document
    //
    virtual bool          CreatingDoc(TDocTemplate* tpl);

  public_data:

/// Holds the list of attached documents, or 0 if no documents exist.
    TDocument::List       DocList;

  private:
    int                   Mode;         ///< Mode flags: dmxxx
    TDocTemplate*         TemplateList; ///< Chained list of doc templates
    TApplication*         Application;  ///< Current application
    TDocTemplate**        TemplateHead; ///< Saved pointer Template Head
    long                  AdditionalFileDialogFlags; ///< OPENFILENAME flags

    // Command enabler handlers
    //
    void  CeFileNew(TCommandEnabler& ce);
    void  CeFileOpen(TCommandEnabler& ce);
    void  CeFileSave(TCommandEnabler& ce);
    void  CeFileSaveAs(TCommandEnabler& ce);
    void  CeFileRevert(TCommandEnabler& ce);
    void  CeFileClose(TCommandEnabler& ce);
    void  CeViewCreate(TCommandEnabler& ce);

  DECLARE_RESPONSE_TABLE(TDocManager);

  friend class TDocTemplate;  // access to template list, PostEvent()
  friend class TDocument;     // access to Application
  
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TDocManager, 1);
  DECLARE_STREAMABLE_OWL(TDocManager, 1);
};

// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TDocManager );

//
/// \class TDvOpenSaveData
// ~~~~~ ~~~~~~~~~~~~~~~
/// TDvOpenSaveData encapsulates the information used to initialized
/// the Open or Save As dialog box by ObjectWindows' DocView manager.
/// Besides the typically 'OPENFILENAME' related information, it also
/// provides methods to retrieve the template list and template count
/// of the current Document Manager.
//
class _OWLCLASS TDvOpenSaveData : public TOpenSaveDialog::TData {
  public:
    TDvOpenSaveData(uint32 flags, tchar* filter,
                    tchar* initialDir, tchar* defExt,
                    int filterIndex, TDocTemplate **tmplList, int tmplCount);

    TDocTemplate**  GetTmplList() const;
    int             GetTmplCount() const;

  protected:
    int             TmplCount; ///< Number of Doc/View templates in 'TmplList'
    int             TmplIndex; ///< Index of the default template.
    TDocTemplate**  TmplList;  ///< List of Doc/View templates displayed in
                               ///< Common Dialog during a File|Open or
                               ///< File|SaveAs operation.
};

//
/// \class TDvOpenSaveDataOwner
// ~~~~~ ~~~~~~~~~~~~~~~~~~~~
/// TDvOpenSaveDataOwner is a mixin class used by the DocView encapsulation
/// of the FileOpen and FileSave dialogs. It provides accessors to the 
/// template list and template count used by these dialogs. It purpose is to 
/// provide an  encapsulation of code shared by the TDvFileOpenDialog an 
/// TDvFileSaveDialog class.
//
class _OWLCLASS TDvOpenSaveDataOwner {
  public:
    TDocTemplate**    GetTmplList() const;
    int               GetTmplCount() const;

  protected:
    TDvOpenSaveDataOwner(TDvOpenSaveData& data);
    TDvOpenSaveData&  DvData;
};


//
/// \class TDvFileOpenDialog
// ~~~~~ ~~~~~~~~~~~~~~~~~
/// TDvFileOpenDialog encapsulates the FileOpen dialog used by the
/// DocView manager. It enhances the standard ObjectWindows TFileOpenDialog 
/// by providing logic to update the dialog as the user switches amoung
/// file types.
//
class _OWLCLASS TDvFileOpenDialog : public TFileOpenDialog, public TDvOpenSaveDataOwner {
  public:
    TDvFileOpenDialog(TWindow* parent, TDvOpenSaveData& data,
                      LPCTSTR title = 0);
  protected:
    virtual TDialogProcReturnType DialogFunction(TMsgId, TParam1, TParam2);
};

//
/// \class TDvFileSaveDialog
// ~~~~~ ~~~~~~~~~~~~~~~~~
/// TDvFileSaveDialog encapsulates the FileSave dialog used by the
/// DocView manager. It enhances the standard ObjectWindows TFileSaveDialog
/// by providing logic to update the dialog as the user switches amoung
/// file types.
//
class _OWLCLASS TDvFileSaveDialog : public TFileSaveDialog, public TDvOpenSaveDataOwner {
  public:
    TDvFileSaveDialog(TWindow* parent, TDvOpenSaveData& data,
                      LPCTSTR title = 0);
  protected:
    virtual TDialogProcReturnType DialogFunction(TMsgId, TParam1, TParam2);
};

/// \name Custom OWL Window  Messages
/// @{
/// These macros handle OWL-defined window messages used internally by the
/// framework:
#define EV_WM_PREPROCMENU\
  {{WM_OWLPREPROCMENU}, 0, (::owl::TAnyDispatcher) ::owl::v_WPARAM_Dispatch,\
   (TMyPMF)::owl::v_HMENU_Sig(&TMyClass::EvPreProcessMenu)}

#define EV_WM_CANCLOSE\
  {{WM_OWLCANCLOSE}, 0, (::owl::TAnyDispatcher) ::owl::B_Dispatch,\
   (TMyPMF)::owl::B_Sig(&TMyClass::EvCanClose)}

#define EV_WM_WAKEUP\
  {{WM_OWLWAKEUP}, 0, (::owl::TAnyDispatcher) ::owl::v_Dispatch,\
   (TMyPMF)::owl::v_Sig(&TMyClass::EvWakeUp)}
/// @}

/// @}


// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Increment the reference count of a DocView template to indicate that it's
/// currently in use by the DocManager.
//
inline void TDocManager::RefTemplate(TDocTemplate& tpl)
{
  ++tpl.RefCnt;
}

//
/// Decrement the reference count of a DocView template.
/// \note The template is deleted when the reference count reaches zero.
//
inline void TDocManager::UnRefTemplate(TDocTemplate& tpl)
{
  if (--tpl.RefCnt == 0)
    delete &tpl;
}

//
/// Returns the TApplication* object associated with this DocManager.
//
inline TApplication* TDocManager::GetApplication() const
{
  return Application;
}

//
/// Returns true if the specified flag is currently enabled by the
/// DocManager or false otherwise.
//
inline bool TDocManager::IsFlagSet(int flag) const
{
  return (Mode & flag) != 0;
}

//
/// Returns variable representing the dtXXXX flags [see DOCTPL.H for list]
/// of the DocManager.
//
inline int TDocManager::GetModeFlags() const
{
  return Mode;
}

//
/// Returns template following specified template - Pass in '0' as
/// parameter to get pointer to the template list.
//
inline TDocTemplate* TDocManager::GetNextTemplate(TDocTemplate* tpl) const
{
  return tpl ? (TDocTemplate*)tpl->GetNextTemplate() : TemplateList;
}

//
/// Returns the index to be used by an untitled document.
/// \note Untitled indices start at '0' and are incremented
///       after a new document is created.
//
inline int& TDocManager::GetUntitledIndex()
{
  return UntitledIndex;
}

//
/// Returns a pointer to the list of DocView templates currently
/// registered with the DocManager.
//
inline TDocTemplate* TDocManager::GetTemplateList() const
{
  return TemplateList;
}

//
/// Returns a List of document currently managed by the DocManager.
//
inline TDocument::List& TDocManager::GetDocList()
{
  return DocList;
}

//
/// See SelectDocPath for details.
//
inline long TDocManager::GetAdditionalFileDialogFlags() const
{
  return AdditionalFileDialogFlags;
}

//
/// See SelectDocPath for details.
//
inline void TDocManager::SetAdditionalFileDialogFlags(long flags)
{
  AdditionalFileDialogFlags = flags;
}

//
/// Creates dummy document
//
inline
TDocument* TDocManager::CreateDummyDoc()
{
  return new TDocument(this);
}

//
/// Public to allow direct invocation from app; for backward compatibility. Call
/// virtual member FileOpen() instead.
//
inline void TDocManager::CmFileOpen()
{
  FileOpen();
}
//
/// Public to allow direct invocation from app; for backward compatibility. Call
/// virtual member FileNew() instead.
//
inline void  TDocManager::CmFileNew()
{
  FileNew();
}
//
/// Public to allow direct invocation from app; for backward compatibility. Call
/// virtual member FileClose() instead.
//
inline void  TDocManager::CmFileClose()
{
  FileClose();
}
//
/// Public to allow direct invocation from app; for backward compatibility. Call
/// virtual member FileSave() instead.
//
inline void TDocManager::CmFileSave()
{
  FileSave();
}
//
/// Public to allow direct invocation from app; for backward compatibility. Call
/// virtual member FileSaveAs() instead.
//
inline void  TDocManager::CmFileSaveAs()
{
  FileSaveAs();
}
//
/// Public to allow direct invocation from app; for backward compatibility. Call
/// virtual member FileRevert() instead.
//
inline void  TDocManager::CmFileRevert()
{
  FileRevert();
}

//
/// Public to allow direct invocation from app; for backward compatibility. Call
/// virtual member ViewCreate() instead.
//
inline void  TDocManager::CmViewCreate()
{
  ViewCreate();
}

//
/// Returns the list of templates displayed in the Open or Save
/// dialogs used by the DocManager.
//
inline TDocTemplate** TDvOpenSaveData::GetTmplList() const
{
  return TmplList;
}

//
/// Returns the number of DocView templates currently displayed in the
/// Open or Save dialogs used by the DocManager.
//
inline int TDvOpenSaveData::GetTmplCount() const
{
  return TmplCount;
}

//
/// Returns the list of templates displayed in the Open or Save
/// dialogs used by the DocManager.
//
inline TDocTemplate** TDvOpenSaveDataOwner::GetTmplList() const
{
  return DvData.GetTmplList();
}

//
/// Returns the number of DocView templates currently displayed in the
/// Open or Save dialogs used by the DocManager.
//
inline int TDvOpenSaveDataOwner::GetTmplCount() const
{
  return DvData.GetTmplCount();
}


} // OWL namespace


#endif  // OWL_DOCMANAG_H
