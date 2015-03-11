//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of classes TDocument, TView, TWindowView, TStream, TInStream,
/// TOutStream
//----------------------------------------------------------------------------

#if !defined(OWL_DOCVIEW_H)
#define OWL_DOCVIEW_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

# include <owl/applicat.h>
# include <owl/dialog.h>
# include <owl/framewin.h>

#if !defined(_INC_COMMDLG)
# include <commdlg.h>
#endif

#include <owl/private/strmdefs.h>

namespace owl {

class _OWLCLASS TDocManager;
class _OWLCLASS TDocTemplate;
class _OWLCLASS TMenuDescr;
class _OWLCLASS TBarDescr;

/// \addtogroup docview
/// @{

_OWLFUNC(int)  FormatFileTime(FILETIME* pft, void * dest, int textlen);

//
/// Document open and sharing modes - used in storage and stream constructors
//
/// Defines the document and open sharing modes used for constructing streams and
/// storing data. Any constants that have the same functionality as those used by
/// OLE 2.0 doc files are indicated in the following table; for example,
/// STGM_TRANSACTED, STGM_CONVERT, STGM_PRIORITY, and STGM_DELETEONRELEASE.
/// Although files are typically used for data storage, databases or spreadsheets
/// can also be used. I/O streams rather than DOS use these bit values. Documents
/// open the object used for storage in one of the following modes:
//
/// \note the bits values are those of file streams, not same as RTL or OLE
//
enum TDocMode {
//  ofParent    = (IOS_OMODE_LAST << 1),      // use open mode of parent storage
  ofParent    = 0,        ///< use open mode of parent storage  
  ofRead      = std::ios::in,  ///< ios::in,  open for reading
  ofWrite     = std::ios::out, ///< ios::out, open for writing
  ofReadWrite = (ofRead|ofWrite),
  ofAtEnd     = std::ios::ate, ///< ios::ate, seek to eof upon original open
  ofAppend    = std::ios::app, ///< ios::app, append mode: all additions at eof
  ofTruncate  = std::ios::trunc, ///< ios::trunc, truncate file if already exists
//  ofNoCreate  = (IOS_OMODE_LAST << 2), // ios::nocreate,  open fails if file doesn't exist
//  ofNoReplace = (IOS_OMODE_LAST << 3), // ios::noreplace, open fails if file already exists
#if defined(_IOS_Nocreate)
  ofNoCreate = _IOS_Nocreate, ///< ios::nocreate,  open fails if file doesn't exist
#else
  ofNoCreate  = 0x40, ///< ios::nocreate,  open fails if file doesn't exist
#endif
#if defined(_IOS_Noreplace)
  ofNoReplace = _IOS_Noreplace, ///< ios::noreplace, open fails if file already exists
#else
  ofNoReplace = 0x80, ///< ios::noreplace, open fails if file already exists
#endif
  ofBinary    = std::ios::binary, // ios::binary, binary (not text) file, no CR stripping
  ofIosMask   = 0x00FF, // all of the above bits as used by class ios

  ofTransacted= 0x1000, ///< STGM_TRANSACTED, supports commit and revert
  ofPreserve  = 0x2000, ///< STGM_CONVERT, backup old data of same name
  ofPriority  = 0x4000, ///< STGM_PRIORITY, temporary efficient peeking
  ofTemporary = 0x8000, ///< STGM_DELETEONRELEASE, delete when destructed

  shCompat    = 0x0600, ///< for non-compliant applications, avoid if possible
  shNone      = 0x0800, ///< EXCLUSIVE functionality
  shRead      = 0x0A00, ///< DENY_WRITE functionality
  shWrite     = 0x0C00, ///< DENY_READ functionality
  shReadWrite = 0x0E00, ///< DENY_NONE functionality
  shDefault   = 0,      ///< use stream implementation default value
  shMask      = (shCompat|shNone|shRead|shWrite)
};
#define PREV_OPEN           (ofNoCreate|ofNoReplace)
#define IS_PREV_OPEN(omode) ((omode & PREV_OPEN)==PREV_OPEN)

//
/// \name Definitions of vnXxxx view notification event IDs
/// @{
/// event ID's up to vnCustomBase reserved for general doc-view notifications
//
const int vnViewOpened  = 1;   ///< a new view has just been constructed
const int vnViewClosed  = 2;   ///< another view is about to be destructed
const int vnDocOpened   = 3;   ///< document has just been opened
const int vnDocClosed   = 4;   ///< document has just been closed
const int vnCommit      = 5;   ///< document is committing, flush cached changes
const int vnRevert      = 6;   ///< document has reverted, reload data from doc
const int vnIsDirty     = 7;   ///< respond true if uncommitted changes present
const int vnIsWindow    = 8;   ///< respond true if passed HWND is that of view
const int vnCustomBase = 100;  ///< base of document class specific notifications
/// @}

//
/// \name Document and view property access flags
/// @{
/// Define document and view property attributes. Documents, views, and applications
/// use these attributes to determine how to process a document or view.
//
const int pfGetText   =  1;   ///< property accessible as text format
const int pfGetBinary =  2;   ///< property accessible as native non-text format
const int pfConstant  =  4;   ///< property is invariant for object instance
const int pfSettable  =  8;   ///< property settable as native format
const int pfUnknown   = 16;   ///< property defined but unavailable in object
const int pfHidden    = 32;   ///< property should be hidden from normal browse
const int pfUserDef   =128;   ///< property has been user-defined at run time
/// @}

//
// Classes defined later in this file
//
class _OWLCLASS TStream;

//
// This causes
// fatal error C1001: INTERNAL COMPILER ERROR  (compiler file 'f:\vs70builds\3077\vc\Compiler\Utc\src\P2\p2symtab.c', line 4533)
//class _OWLCLASS TInStream;
//class _OWLCLASS TOutStream;
class TInStream;
class TOutStream;

class _OWLCLASS TDocument;
class _OWLCLASS TView;

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TDocument
// ~~~~~ ~~~~~~~~~
/// An abstract base class, TDocument is the base class for all document objects and
/// serves as an interface between the document, its views, and the document manager
/// (TDocManager class). TDocument creates, destroys, and sends messages about the
/// view. For example, if the user changes a document, TDocument tells the view that
/// the document has been updated. The DEFINE_DOC_TEMPLATE_CLASS macro associates a
/// document with its views.
/// 
/// In order to send messages to its associated views, the document maintains a list
/// of all the views existing for that document and communicates with the views
/// using ObjectWindows event-handling mechanism. Rather than using the function
/// SendMessage(), the document accesses the view's event table. The views can update
/// the document's data by calling the member functions of the particular document.
/// Views can also request streams, which are constructed by the document.
/// 
/// Both documents and views have lists of properties for their applications to use.
/// When documents and views are created or destroyed, messages are sent to the
/// application, which can then query the properties to determine how to process the
/// document or view. It is the document manager's responsibility to determine if a
/// particular view is appropriate for the given document.
/// 
/// Because the property attribute functions are virtual, a derived class (which is
/// called first) might override the properties defined in a base class. Each
/// derived class must implement its own property attribute types of either string
/// or binary data. If the derived class duplicates the property names of the parent
/// class, it should provide the same behavior and data type as the parent.
/// 
/// In order to add persistence to documents, TDocument contains several virtual
/// functions (for example, InStream and OutStream) that support streaming. Your
/// derived classes need to override these functions in order to read and write
/// data.
/// 
/// Although documents are usually associated with files, they do not necessarily
/// have to be files; they can also consist of database tables, mail systems, fax or
/// modem transmissions, disk directories, and so on.
//
class _OWLCLASS TDocument : public TStreamableBase {
  public:

		/// These property values, which describe the basic properties of a document, are
		/// available in classes derived from TDocument. They can be used to update and
		/// query the attributes of a document. PrevProperty and NextProperty are delimiters
		/// for every document's property list.
		/// See GetProperty() and SetProperty() for more information.
		//
    enum TDocProp {      
      PrevProperty = 0,  ///< Index of last property in base class (none in this case)
      DocumentClass,     ///< text Property: Name of C++ class encapsulating document
      TemplateName,      ///< text property: Name of template attached to document
      ViewCount,         ///< int  property: Number of views displaying this document
      StoragePath,       ///< text property: Identifies object holding data of this document
      DocTitle,          ///< text property: Caption of this document
      NextProperty,      ///< Next index to be used by derived class
    };

    /// Document list class
    //
		/// The TDocument::TList nested class encapsulates the chain of documents. It allows
		/// addition, removal, and destruction of documents from the document list.
    class _OWLCLASS TList {
      public:
/// Constructs a TDocument::TList object.
        TList() : DocList(0) {}
        	
/// Destroys a TDocument::TList object.
       ~TList() {Destroy();}
       
        bool Insert(TDocument* doc);  ///< Inserts a new document into the document list. Fails if the document already exists.
        bool Remove(TDocument* doc);  ///< Removes a document from the document list, fails if not there
        TDocument* Next(const TDocument* doc); ///< If the doc  parameter is 0, Next  returns the first document in the list of documents.
        void Destroy();               ///< Deletes all documents

      private:
        TDocument* DocList;
    };
    typedef TList List;  // for compatibility

    // Document constructor / destructor
    //
    TDocument(TDocument* parent = 0);
    virtual ~TDocument();

    virtual TInStream*  InStream(int mode, LPCTSTR strmId=0);
    TInStream* InStream(int mode, const tstring& streamId) {return InStream(mode, streamId.c_str());}
    virtual TOutStream* OutStream(int mode, LPCTSTR strmId=0);
    TOutStream* OutStream(int mode, const tstring& streamId) {return OutStream(mode, streamId.c_str());}

    virtual bool   Open(int mode, LPCTSTR path=0);
    bool Open(int mode, const tstring& path) {return Open(mode, path.c_str());}

    virtual bool   Close();         ///< close document, does not delete or detach
    virtual bool   Commit(bool force=false); ///< save current data, force write
    virtual bool   Revert(bool clear=false); ///< abort changes, no reload if true
    virtual TDocument& RootDocument();

    TDocManager&   GetDocManager();
    void           SetDocManager(TDocManager& dm);
    TDocument*     GetParentDoc();

    TDocTemplate*  GetTemplate();
    bool           SetTemplate(TDocTemplate* tpl);

    LPCTSTR GetDocPath() const;
    virtual bool    SetDocPath(LPCTSTR path);
    bool SetDocPath(const tstring& path) {return SetDocPath(path.c_str());}

    LPCTSTR GetTitle() const;
    virtual void   SetTitle(LPCTSTR title);
    void SetTitle(const tstring& title) {SetTitle(title.c_str());}

    virtual bool   IsDirty();           ///< Also queries doc and view hierarchy
    void           SetDirty(bool dirty = true);

    virtual bool   IsOpen();
    virtual bool   CanClose();           ///< Returns false if unable to close
    virtual bool   HasFocus(HWND hwnd);  ///< Document (or child doc) has Focus
    virtual TDocument* DocWithFocus(HWND hwnd); // Doc/ChildDoc with focus

    bool           NotifyViews(int event, long item=0, TView* exclude=0);
    TView*         QueryViews(int event, long item=0, TView* exclude=0);
    virtual uint   PostError(uint sid, uint choice = MB_OK);

    // Property access and info
    //
    virtual int    PropertyCount();
    virtual int    FindProperty(LPCTSTR name);    ///< return property index
    int FindProperty(const tstring& name) {return FindProperty(name.c_str());}
    virtual int    PropertyFlags(int index);      ///< pfXxxxx bit array
    virtual LPCTSTR PropertyName(int index);  ///< locale invariant name
    virtual int    GetProperty(int index, void * dest, int textlen=0);
    virtual bool   SetProperty(int index, const void * src); ///< native type

    TList&         GetChildren();

    TView*         GetViewList() const;
    TView*         NextView(const TView* view);

    TStream*       GetStreamList() const;
    TStream*       NextStream(const TStream* strm);

    void *      GetTag() const;
    void           SetTag(void* * tag);

    int            GetOpenMode() const;
    void           SetOpenMode(int mode);

    TView*         InitView(TView* view);    ///< called from template InitView

    bool           IsEmbedded() const;
    void           SetEmbedded(bool embed);

    virtual bool   InitDoc();

  public:
    TDocument(TDocManager* docMan);  ///< create a dummy document to hold docmgr

  protected:
    virtual void  AttachStream(TStream& strm);///< called from TStream constructor
    virtual void  DetachStream(TStream& strm);///< called from TStream destructor

  public_data:
		/// Holds a pointer to the application-defined data. Typically, you can use Tag to
		/// install a pointer to your own application's associated data structure. Tag,
		/// which is initialized to 0 at the time a TDocument object is constructed, is not
		/// otherwise used by the document view classes.
    void *     Tag;          
    
    TList         ChildDoc;     ///< The list of child documents associated with this document.

  protected_data:
  	
/// Indicates that unsaved changes have been made to the document. Views can also
/// independently maintain their local disk status.
    bool          DirtyFlag;    
    
    bool          Embedded;     ///< Indicates whether the document is embedded.
    
//  static int    UntitledIndex;// last used index for Untitled document

  private:
    TDocManager*  DocManager;   ///< pointer back to document manager
    TDocument*    ParentDoc;    ///< parent document, 0 if this is root document
    TDocument*    NextDoc;      ///< next in linked chain of active documents
    int           OpenMode;     ///< mode and protection flags
    tchar *   Title;        ///< current document title, 0 if untitled
    TDocTemplate* Template;     ///< template associated with this document
    TView*        ViewList;     ///< head of linked view chain, 0 if no views
    TStream*      StreamList;   ///< head of linked stream chain, 0 if no streams
    tchar *   DocPath;      ///< path used to open/save document

    void   ReindexFrames();          ///< force view title and index update
    void   AttachView(TView& view);  ///< called from TView constructor
    bool   DetachView(TView& view);  ///< called from TView destructor


  friend class _OWLCLASS TDocTemplate;  ///< access to InitView()
  friend class _OWLCLASS TView;         ///< access to Attach/DetatchView()
  friend class _OWLCLASS TStream;   ///< access to Attach/DetachStream()
  friend class _OWLCLASS TDocManager;
  friend class _OWLCLASS TList;         ///< access to NextDoc

  //DECLARE_ABSTRACT_STREAMABLE(_OWLCLASS,TDocument,1);
  DECLARE_ABSTRACT_STREAMABLE_OWL(TDocument,1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TDocument );

//
/// \class TView
// ~~~~~ ~~~~~
/// Abstract base class for view access from document
//
/// Derived virtually from both TEventHandler and TStreamableBase, TView is the
/// interface presented to a document so it can access its client views. Views then
/// call the document functions to request input and output streams. Views own the
/// streams and are responsible for attaching and deleting them.
/// 
/// Instead of creating an instance of TView, you create a derived class that can
/// implement TView's virtual functions. The derived class must have a way of
/// knowing the associated window (provided by GetWindow()) and of describing the view
/// (provided by GetViewName()). The view must also be able to display the document
/// title in its window (SetDocTitle()).
/// Classes derived from TView may need to handle several notification messages. For
/// example, if a view is associated with a window that can gain focus, then the
/// view should handle the vnIsWindow  notification message.
/// 
/// View classes can take various forms. For example, a view class can be a window
/// (through inheritance), can contain a window (an embedded object), can reference
/// a window, or can be contained within a window object. A view class might not
/// even have a window, as in the case of a voice mail or a format converter. Some
/// remote views (for example, those displayed by DDE servers) might not have local
/// windows.
/// 
/// Other viewer classes derived from TView include TEditView, TListBoxView, and
/// TWindowView. These classes display different types of data: TEditView displays
/// unformatted text files, TListBoxView displays text information in a list box, and
/// TWindowView is a basic viewer from which you can derive other types of viewers
/// such as hexadecimal file viewers.
/// 
/// For OLE-enabled applications, use TOleView, which supports views for embedded
/// objects and compound documents.
//
class _OWLCLASS TView : virtual public TEventHandler,
                        virtual public TStreamableBase {
  public:
/// These property values, which describe the basic properties of a view, are
/// available in classes derived from TView. They can be used to update and query
/// the attributes of a view. PrevProperty and NextProperty are delimiters for every
/// view's property list.
    enum {
      PrevProperty = 0,		///< Index of last property in base class.
      ViewClass,          ///< Name of the C++ class encapsulating the view. (text)
      ViewName,           ///< Name of the view. (text)
      NextProperty,				///< Next index to be used by derived class.
    };

    TView(TDocument& doc);
    virtual ~TView();

    TDocument&  GetDocument();
    void        SetDocument(TDocument&); // Added by Vidar Hasfjord, 2007-08-27.

    uint        GetViewId();

    virtual TMenuDescr* GetViewMenu();
    virtual TBarDescr*  GetViewBar();
    void        SetViewMenu(TMenuDescr* menu);
    void        SetViewBar(TBarDescr* bar);

    bool        IsOK();           ///< true if successfully created

    static uint GetNextViewId();  ///< Next global ID to assign
    static void BumpNextViewId();

    TView*      GetNextView();

		/// Pure virtual function that returns 0. Override this function in your derived
		/// class to return the name of the class.
    /// Must implement, used by template manager for selection
    /// \code
    /// static LPCTSTR StaticName() {return "name of view";}
    /// \endcode
    virtual LPCTSTR GetViewName() = 0;    

/// Returns the TWindow instance associated with the view, or 0 if no view exists.
    virtual TWindow* GetWindow();                 // if not derived from TWindow
    
    virtual bool   SetDocTitle(LPCTSTR docname, int index);
    bool SetDocTitle(const tstring& docname, int index) {return SetDocTitle(docname.c_str(), index);}

    // Property access and info
    //
    virtual int    PropertyCount();
    virtual int    FindProperty(LPCTSTR name);///< return property index
    int FindProperty(const tstring& name) {return FindProperty(name.c_str());}
    virtual int    PropertyFlags(int index);          ///< pfXxxxx bit array
    virtual LPCTSTR  PropertyName(int index);     ///< locale invariant name
    virtual int    GetProperty(int index, void * dest, int textlen=0);
    virtual bool   SetProperty(int index, const void * src);

    void *      GetTag() const;
    void           SetTag(void* * tag);

  protected:
    void         NotOK();       ///< To flag errors in creation

  protected_data:
    TDocument*   Doc;		///< Holds the current document.

  public_data:
  	/// Application hook, not used internally
		/// Holds a pointer to the application-defined data. Typically, you can use Tag to
		/// install a pointer to your own application's associated data structure. TView
		/// zeros Tag during construction and does not access it again.
    void *    Tag;           

  private:
    TView*       NextView;      ///< Linked view chain, 0 if no more views
    uint         ViewId;        ///< Unique ID for this view, used for controls
    TMenuDescr*  ViewMenu;      ///< Menu descriptor specific for this view or 0
    TBarDescr*   ViewBar;      ///< Bar descriptor specific for this view or 0
    static uint  NextViewId;    ///< Next view ID to be assigned to a view

  friend class _OWLCLASS TDocument;   ///< needs access to NextView
#if !defined(BI_NO_OBJ_STREAMING)
  friend class TDocument::Streamer;   ///< needs access to NextView
#endif
  //DECLARE_ABSTRACT_STREAMABLE(_OWLCLASS,TView,1);
  DECLARE_ABSTRACT_STREAMABLE_OWL(TView,1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TView );

//
/// \class TWindowView
// ~~~~~ ~~~~~~~~~~~
/// Derived from both TWindow and TView, TWindowView is a streamable base class that
/// can be used for deriving window-based views. TWindowView's functions override
/// TView's virtual function to provide their own implementation. By deriving a
/// window-view class from TWindow and TView, you add window functionality to the
/// view of your document.
class _OWLCLASS TWindowView : public TWindow, public TView {
  public:
    TWindowView(TDocument& doc, TWindow* parent = 0);
   ~TWindowView();

    static LPCTSTR StaticName();  ///< put in resource

    // Override virtuals from TWindow
    //
    bool     CanClose();

    // Override virtuals from TView
    //
    LPCTSTR  GetViewName();
    TWindow* GetWindow();
    bool     SetDocTitle(LPCTSTR docname, int index);
    using TView::SetDocTitle; ///< String-aware overload

  private:
    // Event handlers
    //
    bool     VnIsWindow(HWND hWnd);

  DECLARE_RESPONSE_TABLE(TWindowView);
  //DECLARE_STREAMABLE (_OWLCLASS, owl::TWindowView,1);
  DECLARE_STREAMABLE_OWL(TWindowView, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TWindowView );


//
/// \class TDialogView
// ~~~~~ ~~~~~~~~~~~
/// Derived from TDialog and TView.
/// Must be overriden, derived class have to define resId
//
class _OWLCLASS TDialogView : public TDialog, public TView {
  public:
    TDialogView(TDocument& doc, TWindow* parent, TResId resId, TModule* module = &GetGlobalModule());
    TDialogView(TDocument& doc, TWindow* parent, const DLGTEMPLATE& dlgTemplate, TAutoDelete = AutoDelete, TModule* module = &GetGlobalModule());
    TDialogView(TDocument& doc, TWindow* parent, TModule* module, HGLOBAL hTemplate, TAutoDelete = AutoDelete);
   ~TDialogView();

    static LPCTSTR StaticName();  // put in resource

    // Override virtuals from TWindow
    //
    bool     CanClose();

    // Override virtuals from TView
    //
    LPCTSTR  GetViewName();
    TWindow* GetWindow();
    bool     SetDocTitle(LPCTSTR docname, int index);
    using TView::SetDocTitle; ///< String-aware overload

  private:
    // Event handlers
    //
    bool     VnIsWindow(HWND hWnd);

  DECLARE_RESPONSE_TABLE(TDialogView);
  //DECLARE_STREAMABLE (_OWLCLASS, owl::TDialogView,1);
  DECLARE_STREAMABLE_OWL(TDialogView,1);

};

// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TDialogView );

//
/// \class TStream
// ~~~~~ ~~~~~~~
/// An abstract base class, TStream provides links between streams and documents,
/// views, and document files.
//
class _OWLCLASS TStream {
  public:
    TDocument& GetDocument();
   ~TStream();
    int  GetOpenMode();
    LPCTSTR GetStreamName();

  protected:
    TDocument& Doc;        ///< Stores the document that owns this stream.
    TStream* NextStream;   ///< Points to the next stream in the list of active streams.

    TStream(TDocument& doc, LPCTSTR name, int mode);

  private:
    int     OpenMode;
    LPCTSTR StreamName;

  friend class TDocument;
};

//
/// \class TInStream
// ~~~~~ ~~~~~~~~~
/// Derived from TStream and istream, TInStream is a base class used to define input
/// streams for documents.
//
class TInStream : public TStream, public tistream {
  public:
    TInStream(TDocument& doc, LPCTSTR name, int mode);
};

//
/// \class TOutStream
// ~~~~~ ~~~~~~~~~~
/// Derived from TStream and ostream, TOutStream is a base class used to create
/// output storage streams for a document.
//
class TOutStream : public TStream, public tostream {
  public:
    TOutStream(TDocument& doc, LPCTSTR name, int mode);
};

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// View Notification Handler Definitions
//

//
// DocView aliases to actual dispatchers
//
#define B_void_Dispatch     B_Dispatch            ///< No parameters
#define B_int_Dispatch      B_I2_Dispatch         ///< LPARAM as int
#define B_pointer_Dispatch  B_POINTER_Dispatch    ///< LPARAM as void*
#define B_long_Dispatch     B_LPARAM_Dispatch     ///< LPARAM as long

//
// Define a DocView notification signature
//   'id' is the vnXxxXxx name of the notification
//   'arg' is the type of the arg passed to the handler
//
#define NOTIFY_SIG(id, arg) \
  template <class T> \
  inline bool (T::*id##_Sig(bool (T::*pmf)(arg)))(arg) {return pmf;}

NOTIFY_SIG(vnViewOpened,  TView*)
NOTIFY_SIG(vnViewClosed,  TView*)
NOTIFY_SIG(vnDocOpened,   int)
NOTIFY_SIG(vnDocClosed,   int)
NOTIFY_SIG(vnCommit,      bool)
NOTIFY_SIG(vnRevert,      bool)
NOTIFY_SIG(vnIsDirty,     void)
NOTIFY_SIG(vnIsWindow,    HWND)

//
/// Define a DocView notification response entry
///   'id' is the id from NOTIFY_SIG above
///   'method' is the method name called by the notification
///   'disp' is the type of the lParam dispacher to use, and can be:
///     'void'    lParam not passed
///     'int'     integer size (16bit for win16, or 32bit on win32)
///     'pointer' pointer size (16bit for small & medium, or 32bit all other)
///     'long'    32 bit passed always
//
#define VN_DEFINE(id, method, disp) \
  {{WM_OWLNOTIFY}, id, \
  (::owl::TAnyDispatcher) ::owl::B_##disp##_Dispatch, \
  (TMyPMF)id##_Sig(&TMyClass::method)}

/// \name Document View  Messages
/// @{
/// These macros handle view-related messages generated by the document manager.
/// VnHandler is a generic term for the view notification handler function.

#define EV_VN_VIEWOPENED  VN_DEFINE(::owl::vnViewOpened,  VnViewOpened,  pointer)
#define EV_VN_VIEWCLOSED  VN_DEFINE(::owl::vnViewClosed,  VnViewClosed,  pointer)
#define EV_VN_DOCOPENED   VN_DEFINE(::owl::vnDocOpened,   VnDocOpened,   int)
#define EV_VN_DOCCLOSED   VN_DEFINE(::owl::vnDocClosed,   VnDocClosed,   int)
#define EV_VN_COMMIT      VN_DEFINE(::owl::vnCommit,      VnCommit,      int)
#define EV_VN_REVERT      VN_DEFINE(::owl::vnRevert,      VnRevert,      int)
#define EV_VN_ISDIRTY     VN_DEFINE(::owl::vnIsDirty,     VnIsDirty,     void)
#define EV_VN_ISWINDOW    VN_DEFINE(::owl::vnIsWindow,    VnIsWindow,    int)

/// @}

/// @}

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Generic input for the particular storage medium, InStream returns a pointer to a
/// TInStream. mode is a combination of the ios bits defined in iostream.h. See the
/// document open mode constants for a list of the open modes. Used for documents
/// that support named streams, strmId is a pointer to the name of a stream.
/// Override this function to provide streaming for your document class.
//
inline TInStream* TDocument::InStream(int /*mode*/, LPCTSTR /*strmId*/) {
  return 0;
}

//
/// Generic output for the particular storage medium, OutStream returns a pointer to
/// a TOutStream. mode is a combination of the ios bits defined in iostream.h. Used
/// for documents that support named streams, strmId points to the name of the
/// stream. TDocument::OutStream version always returns 0. Override this function to
/// provide streaming for your document class.
//
inline TOutStream* TDocument::OutStream(int /*mode*/, LPCTSTR /*strmId*/) {
  return 0;
}

//
/// Opens the document using the path specified by DocPath. Sets OpenMode to mode.
/// TDocument::Open always returns true and actually performs no actions. Other
/// classes override this function to open specified file documents and views.
//
inline bool TDocument::Open(int /*mode*/, LPCTSTR /*path*/) {
  return true;
}

//
/// Returns a pointer to the current document manager.
//
inline TDocManager& TDocument::GetDocManager() {
  return *DocManager;
}

//
/// Returns either the parent document of the current document or 0 if there is no
/// parent document.
//
inline TDocument* TDocument::GetParentDoc() {
  return ParentDoc;
}

//
/// Gets the template used for document creation. The template can be changed during
/// a SaveAs operation.
//
inline TDocTemplate* TDocument::GetTemplate() {
  return Template;
}

//
/// Returns the directory path for the document. This might change the SaveAs
/// operation.
//
inline LPCTSTR TDocument::GetDocPath() const {
  return DocPath;
}

//
/// Returns the title of the document.
//
inline LPCTSTR TDocument::GetTitle() const {
  return Title;
}

//
/// Updates the document's dirty flag using the specified parameter.
//
inline void TDocument::SetDirty(bool dirty) {
  DirtyFlag = dirty;
}

//
/// Checks to see if the document has any streams in its stream list. Returns false
/// if no streams are open; otherwise, returns true.
//
inline bool TDocument::IsOpen() {
  return StreamList != 0;
}

//
/// Gets the total number of properties for the TDocument object. Returns
/// NextProperty -1.
//
inline int TDocument::PropertyCount() {
  return NextProperty-1;
}

//
/// Returns true if the document is embedded in an OLE 2 container.
//
inline bool TDocument::IsEmbedded() const {
  return Embedded;
}

//
/// Marks the document as being embedded in an OLE 2 container. Typically, this
/// happens when the server is created and when the factory template class creates
/// the component.
//
inline void TDocument::SetEmbedded(bool embed) {
  Embedded = embed;
}

//
///  A virtual method that is overridden by TOleDocument::InitDoc. You can use this
/// function to prepare the document before the view is constructed and before the
/// dnCreate event, which indicates that the document has been created and is
/// posted.
//
inline bool TDocument::InitDoc() {
  return true;
}

//
/// Return reference to the children document list.
//
inline TDocument::TList& TDocument::GetChildren() {
  return ChildDoc;
}

//
/// Return pointer to the head of the link list of views associated with this
/// document.
/// \note To iterate through all the views, use the 'NextView' method with
///       the pointer obtained from this method.
//
inline TView* TDocument::GetViewList() const {
  return ViewList;
}

//
/// Returns head of the link list of streams associated with this document.
/// \note To iterate through all the streams, use the 'NextStream' method with the
/// pointer returned from this method.
//
inline TStream* TDocument::GetStreamList() const {
  return StreamList;
}

//
/// Returns pointer to user-defined data [i.e. tag] attached to this document.
//
inline void * TDocument::GetTag() const {
  return Tag;
}

//
/// Attach an arbitrary (user-defined) pointer with this document.
/// \note The 'Tag' is a place-holder. It is not used by the Doc/View subsystem.
//
inline void TDocument::SetTag(void* * tag) {
  Tag = tag;
}

//
/// Create a dummy document to hold docmgr.
//
inline TDocument::TDocument(TDocManager* docMan)
:
  DocManager(docMan), ParentDoc(this)
{}

//
/// Gets the mode and protection flag values for the current document.
//
inline int
TDocument::GetOpenMode() const
{
  return OpenMode;
}

//
/// Sets the mode and protection flag values for the current document.
//
inline void
TDocument::SetOpenMode(int mode)
{
  OpenMode = mode;
}

//
/// Returns a reference to the view's document.
//
inline TDocument& TView::GetDocument() {
  return *Doc;
}

//
/// Returns the unique ID for this view.
//
inline uint TView::GetViewId() {
  return ViewId;
}

//
/// Returns the menu descriptor for this view. This can be any existing TMenuDescr
/// object. If no descriptor exists, ViewMenu is 0.
//
inline TMenuDescr* TView::GetViewMenu() {
  return ViewMenu;
}

//
inline TBarDescr* TView::GetViewBar() {
  return ViewBar;
}

//
/// Returns a nonzero value if the view is successfully constructed.
//
inline bool TView::IsOK() {
  return ViewId != 0;
}
//
/// Returns the next view ID to be assigned.
//
inline uint TView::GetNextViewId() {
  return NextViewId;
}

//
/// Returns the next global view ID to be assigned.
//
inline TView* TView::GetNextView() {
  return NextView;
}

//
inline TWindow* TView::GetWindow() {
  return 0;
}

//
/// Retrieves the user-defined pointer attached to this view.
//
inline void * TView::GetTag() const {
  return Tag;
}

//
/// Associates an arbitrary (user-defined) pointer with this view.
/// \note The 'Tag' is not used by the Doc/View subsystem.
//
inline void TView::SetTag(void* * tag) {
  Tag = tag;
}

//
/// Stores the document title.
//
inline bool TView::SetDocTitle(LPCTSTR /*docname*/, int /*index*/) {
  return false;
}

//
/// Gets the total number of properties for the TDocument object.
//
inline int TView::PropertyCount() {
  return NextProperty - 1;
}

//
/// Sets the value of the property, given the index of the property, and src, the
/// data type (either binary or text) to which the property must be set.
//
inline bool TView::SetProperty(int /*index*/, const void * /*src*/) {
  return false;
}

//
/// Sets the view to an invalid state, causing IsOK to return 0.
//
inline void TView::NotOK() {
  ViewId = 0;
}

//
// class TWindowView
// ~~~~~ ~~~~~~~~~~~
/// Destroys a TWindowView object and calls the associated document's DetachView
/// function (a private TDocument function) to detach the view from the associated
/// document.
//
inline TWindowView::~TWindowView() {
}

//
/// Returns "Window View," the descriptive name of the view. This title is displayed
/// in the user-interface box.
//
inline LPCTSTR TWindowView::StaticName() {
  return _T("Window View");
}

//
/// Does a given HWND belong to this view? Yes if it is us, or a child of us
//
inline bool TWindowView::VnIsWindow(HWND hWnd){
  return hWnd == GetHandle() || IsChild(hWnd);
}
//
/// Overrides TWindow::CanClose and returns a nonzero value if the window can be
/// closed. CanClose checks all the associated document's CanClose functions. These
/// must return nonzero values before the window view can be closed.
//
/// Only query document if this is the last view open to it.
//
inline bool TWindowView::CanClose() {
  return TWindow::CanClose() && 
         (Doc->NextView(this) || 
          Doc->NextView(0) != this ||
          Doc->CanClose());
}

//
/// Overrides TView::GetViewName and returns StaticName, the name of the view.
//
inline LPCTSTR TWindowView::GetViewName() {
  return StaticName();
}

//
/// Overrides TView::GetWindow and returns the TWindowView object as a TWindow.
//
inline TWindow* TWindowView::GetWindow() {
  return (TWindow*)this;
}

//
/// Overrides TView::SetDocTitle and stores the document title. This name is
/// forwarded up the parent chain until a TFrameWindow object accepts the data and
/// displays it in its caption.
//
inline bool TWindowView::SetDocTitle(LPCTSTR docname, int index) {
  return TWindow::SetDocTitle(docname, index);
}

//
/// class TDialogView
// ~~~~~ ~~~~~~~~~~~
inline TDialogView::~TDialogView() {
}

//
inline LPCTSTR TDialogView::StaticName(){
  return _T("Dialog View");
}

//
/// Only query document if this is the last view open to it.
//
inline bool TDialogView::CanClose() {
  return TDialog::CanClose() && 
         (Doc->NextView(this) || 
          Doc->NextView(0) != this ||
          Doc->CanClose());
}

//
inline LPCTSTR TDialogView::GetViewName() {
  return StaticName();
}

//
inline TWindow* TDialogView::GetWindow() {
  return (TWindow*)this;
}

//
/// Does a given HWND belong to this view? Yes if it is us, or a child of us
//
inline bool TDialogView::VnIsWindow(HWND hWnd){
  return hWnd == GetHandle() || IsChild(hWnd);
}

//
inline bool TDialogView::SetDocTitle(LPCTSTR docname, int index) {
  return TDialog::SetDocTitle(docname, index);
}

//
// class TStream
// ~~~~~ ~~~~~~~
//
/// Returns the current document that is open for streaming.
//
inline TDocument& TStream::GetDocument() {
  return Doc;
}

//
/// Closes the stream. Derived classes generally close the document if it was opened
/// especially for this stream.
//
inline TStream::~TStream() {
  Doc.DetachStream(*this);
}

//
/// Constructs a TStream object. 
/// 'name' is the user-defined name of the stream and can be 0.
/// Note: If 'name' is not null, the referenced string must outlive the object. No copy is made.
//
inline TStream::TStream(TDocument& doc, LPCTSTR name, int mode)
:
  Doc(doc),
  OpenMode(mode),
  StreamName(name)
{
  Doc.AttachStream(*this);
}

//
/// Gets the name of the stream used for opening the document.
//
inline LPCTSTR TStream::GetStreamName() {
  return StreamName;
}

//
/// Gets mode flags used when opening document streams. For example, the stream can
/// be opened in ofRead mode to allow reading, but not changing (writing to) the
/// file.
//
inline int TStream::GetOpenMode() {
  return OpenMode;
}

//
// WORK-AROUND: Fix for a bug in the Dinkumware standard library; double-init memory leak.
// Using 0 as the stream initializer in the Dinkumware version of the library sets the stream buffer
// to 0 and also sets an error state of the stream. Also, research on the net indicate that this causes
// a double-init memory leak. 
//
// TODO: Research which versions are affected and how, and reliably detect only these.
//
#if defined(_CPPLIB_VER) // Defined by Dinkumware - assume this detects the library.
#define OWL_STREAM_NULL_INITIALIZER std::_Noinit
#else
#define OWL_STREAM_NULL_INITIALIZER 0
#endif

//
/// Constructs a TInStream object.
/// 'name' is the user-defined name of the stream and can be 0.
/// Note: If 'name' is not null, the referenced string must outlive the object. No copy is made.
//
inline TInStream::TInStream(TDocument& doc, LPCTSTR name, int mode)
:
  TStream(doc, name, mode),
  tistream(OWL_STREAM_NULL_INITIALIZER)  // init base to null, don't forget to call init.
{}

//
/// Constructs a TOutStream object. 
/// 'name' is the user-defined name of the stream and can be 0.
/// Note: If 'name' is not null, the referenced string must outlive the object. No copy is made.
//
inline TOutStream::TOutStream(TDocument& doc, LPCTSTR name, int mode)
:
  TStream(doc, name, mode),
  tostream(OWL_STREAM_NULL_INITIALIZER)  // init base to null, don't forget to call init.
{}


} // OWL namespace


#endif  // OWL_DOCVIEW_H
