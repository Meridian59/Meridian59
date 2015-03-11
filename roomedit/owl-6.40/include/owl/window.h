//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Base window class TWindow definition, including HWND encapsulation.
//----------------------------------------------------------------------------

#if !defined(OWL_WINDOW_H)
#define OWL_WINDOW_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>
#include <owl/wsyscls.h>
#include <owl/color.h>
#include <owl/objstrm.h>
#include <owl/clipboar.h>
#include <owl/windowev.h>


//FMM
#include <owl/applicat.h>
#include <owl/menu.h>
#include <owl/window.rh>
#include <stdarg.h>
#include <utility>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

class _OWLCLASS TApplication;
class _OWLCLASS TModule;
class _OWLCLASS TScroller;
class _OWLCLASS TRegion;
class _OWLCLASS TWindow;
class _OWLCLASS TDC;
class _OWLCLASS TNotify;
class _OWLCLASS TXWindow;
class _OWLCLASS TTooltip;

/// \addtogroup window
/// @{

//
/// \enum TWindowFlag
// ~~~~ ~~~~~~~~~~~
/// Define bit masks for the internally used flag attributes of TWindow. 
enum TWindowFlag {
  wfAlias           = 0x0001, ///< TWindow is an alias to a preexisting HWND
  wfAutoCreate      = 0x0002, ///< Create the HWND when our parent is created
  wfFromResource    = 0x0004, ///< Handle comes from HWND created from resource
  wfShrinkToClient  = 0x0008, ///< Shrink a frame window to its client's size
  wfMainWindow      = 0x0010, ///< This frame window is the main window
  wfFullyCreated    = 0x0020, ///< Window is fully created & not being destroyed
  wfStreamTop       = 0x0040, ///< This window is the topmost one to stream
  wfPredefinedClass = 0x0080, ///< Window class used belongs to a predefined Windows class and was not defined by OWL
  wfTransfer        = 0x0100, ///< Participates in the Transfer mechanism.
  wfUnHidden        = 0x0200, ///< Used temporarily when destroying MDI child
  wfUnDisabled      = 0x0400, ///< Used temporarily when destroying MDI child
  wfDeleteOnClose   = 0x0800, ///< Window is condemned on EvClose.
															///< Indicates that a modeless dialog's pointer is to be deleted when
															///< it receives a WM_CLOSE message, causing its class's destructor to be called
															///< immediately. Default behavior is to leave the dialog pointer alone. Setting this
															///< flag has no effect on modal dialogs. To set this flag for a modeless dialog
															///< object, add the following statement to the constructor of your TDialog-derived
															///< class:
															///< \code
															///< 	SetFlag(wfDeleteOnClose);  
															///< \endcode
  wfPropagateIdle   = 0x1000, ///< Pass down IdleAction
  wfModalWindow     = 0x2000, ///< Was created via TWindow::Execute
  wfDetached        = 0x4000, ///< Window handle has been detached
  wfInsertAtEdge    = 0x8000  ///< (Decoration) Window to be inserted against frame's edge
};

//
/// \enum TTransferDirection
// ~~~~ ~~~~~~~~~~~~~~~~~~
/// The TTransferDirection enum describes the constants that the transfer
/// function uses to determine how to transfer data to and from the transfer buffer
//
enum TTransferDirection {
  tdGetData,    ///< Get data from the window into the buffer
  tdSetData,    ///< Set data from the buffer into the window
  tdSizeData    ///< Return the size of data transferred by the class.
};

//
/// \enum TEventStatus
// ~~~~ ~~~~~~~~~~~~
/// Mixin window event implementation return status
//
/// Event status constants indicate the status of a mix-in window event
/// implementation, for example, a keyboard event. The TEventStatus constants
/// indicate whether or not additional handlers are needed.
//
enum TEventStatus {
  esPartial,    ///< Additional handlers may be invoked
  esComplete    ///< No additional handlers are needed
};

//
/// \name Special background color flags for EvEraseBkgnd processing
/// @{
#define NoColor TColor::None        ///< let DefWindowProc erase
#define NoErase TColor::Transparent ///< don't erase, wait for Paint
/// @}

//
// Windows 3.1 windowsx.h name confict with TWindows::GetFirstChild()
//
#if defined(GetFirstChild)
# undef GetFirstChild(hwnd)
#endif

//----------------------------------------------------------------------------

//
// Member and non-member action and conditional function types used with
// ForEach and FirstThat.
//

/// Defines the signature of the callback function passed to the ForEach method of
/// TWindow.
typedef void (*TActionFunc)(TWindow* win, void* param);

/// Defines the signature of the callback function used by the FirstThat function of
/// TWindow.
typedef bool (*TCondFunc)(TWindow* win, void* param);

#if defined(BI_COMP_MSC)
# pragma pointers_to_members(full_generality, virtual_inheritance)
#endif

/// Defines the signature of the callback function passed to the ForEach method of
/// TWindow.
typedef void (TWindow::*TActionMemFunc)(TWindow* win, void* param);

/// Defines the signature of the member function used by the FirstThat function of
/// TWindow.
typedef bool (TWindow::*TCondMemFunc)(TWindow* win, void* param);

/// @}
//
/// \addtogroup newctrl
/// @{

//----------------------------------------------------------------------------
//
/// \class TDrawItem
// ~~~~~ ~~~~~~~~~
/// Class will be base class for owner draw items: ListBox,ComboBox,Menu atc.
/// lParam member of DRAWITEMSTRUCT will point to instance of derived from this class
/// Note: OwnerDraw Buttons isn't support lParam !!!!!
//
// trick for dynamic_cast<>(), don't derive from it!!!!!!!!!!!!!
class TDrawItemBase {
  public:
//FMM
DECLARE_CASTABLE;
    enum TDrawItemId { drawId = 0x69777264, /*'drwi'*/ };
    TDrawItemBase():__itemId(drawId){}
    virtual ~TDrawItemBase(){}
    uint32 __itemId; // don't use __itemId
};

class TDrawItem: public TDrawItemBase {
  public:
//FMM 2 lines below
  typedef const char* Type_id;    
  DECLARE_CASTABLE;  
    //virtual ~TDrawItem(){} // use base
    virtual void Draw(DRAWITEMSTRUCT &) {}
    virtual void Measure(MEASUREITEMSTRUCT &) {}
    virtual int  Compare(COMPAREITEMSTRUCT &) { return 0;}
};

/// @}
//
/// \addtogroup enabler
/// @{
//
/// \class TCommandEnabler
// ~~~~~ ~~~~~~~~~~~~~~~
/// Base class for an extensible interface for auto enabling/disabling of
/// commands (menu items, tool bar buttons, ...)
//
class _OWLCLASS TCommandEnabler {
  public:
    TCommandEnabler(uint id, HWND hWndReceiver = 0);

    uint GetId() const;

    virtual void  Enable(bool enable = true);        // Also sets Handled
    
/// Changes the text associated with a command sender; for example, text associated
/// with a menu item or text on a button.
    virtual void  SetText(LPCTSTR text) = 0;
    void          SetText(const tstring& s) {SetText(s.c_str());}

		/// Enumeration describing the 3-states or availability of a command. This state
		/// applies to buttons, such as those used for tool bars, or to control bar gadgets.
    //
    enum TCommandState {
      Unchecked,          ///< Command is not enabled
      Checked,            ///< Command is enabled
      Indeterminate       ///< Command is unavaiable
    };

/// Changes the check state of the command sender to either unchecked, checked, or
/// indeterminate. This state applies to buttons, such as those used for tool bars,
/// or to control bar gadgets. Pass a TCommandState enum
    virtual void  SetCheck(int check) = 0;           

    //
    /// Overload; allows you to pass a bool safely.
    /// Sets the state to Checked if true is passed, and to Unchecked otherwise.
    //
    void SetCheck(bool isChecked) 
    {SetCheck(isChecked ? Checked : Unchecked);}

    bool        SendsCommand() const;

    bool        GetHandled();

    bool        IsReceiver(HWND hReceiver);
    HWND        GetReceiver() const;
    void        SetReceiver(HWND hReceiver);

  protected:
    void        SetHandled();

  public_data:
/// Command ID for the enabled command.
    const uint  Id;

  protected_data:
/// The message responder (the window) that receives the command.
    HWND  HWndReceiver;

	protected:
    /// Enumeration describing whether the associated command has been
    /// enabled or disabled, and whether the command generates WM_COMMAND
    /// messages.
    //
    enum TCommandStatus {
      WasHandled = 1,       ///< Command was enabled or disabled
      NonSender = 2         ///< Command does not generate WM_COMMAND messages
    };

    /// TCommandStatus flags
    /// Is TCommandStatus::WasHandled if the command enabler has been handled.
    //
    uint Flags;

  DECLARE_CASTABLE;
};

/// @}

/// \addtogroup window
/// @{

//
/// \struct TWindowAttr
// ~~~~~~ ~~~~~~~~~~~~
/// Holds TWindow attributes set during construction of a window. Your program
/// controls a window's creation by passing these values to one of TWindow's
/// creation routines. If the window is streamed, these attributes are also used for
/// re-creation.
/// \note Don't rely on these to track once Handle
/// has been created, use member functions to access information.
//
struct TWindowAttr 
{
/// Contains the values that define the style, shape, and size of your window.
/// Although TWindow sets Attr.Style to WS_CHILD and WS_VISIBLE, you can also use
/// other combinations of the following style constants:
/// - \c \b WS_BORDER	Creates a window with a thin lined border
/// - \c \b WS_CAPTION	Creates a window with a title bar.
/// - \c \b WS_CHILD	Creates a child windows. Cannot be used with popup styles.
/// - \c \b WS_CHILDWINDOW	Creates a child window.
/// - \c \b WS_CLIPCHILDREN	Used when creating a parent window. Excludes the area occupied
/// by child windows when drawing takes place within the parent window.
/// - \c \b WS_CLIPSIBLINGS	Clips child windows relative to the child window that receives a
/// paint message.
/// - \c \b WS_DISABLED	Creates a window that cannot receive user input.
/// - \c \b WS_DLGFRAME	Creates a window having a typical dialog box style (without a title
/// bar).
/// - \c \b WS_GROUP	Indicates the first control in a group of controls, which the user can
/// change by pressing the direction keys.
/// - \c \b WS_HSCROLL	Window has a horizontal scroll bar.
/// - \c \b WS_MAXIMIZE	Window is initially maximized.
/// - \c \b WS_MAXIMIZEBOX	Window has a maximize button.
/// - \c \b WS_MINIMIZE	Window is initially minimized.
/// - \c \b WS_MINIMIZEBOX	Window has a minimize button.
/// - \c \b WS_OVERLAPPED	Creates an overlapped window with a title bar and a border.
/// - \c \b WS_OVERLAPPEDWINDOW	Overlapped window has the WS_OVERLAPPED, WS_CAPTION,
/// WS_SYSMENU, WS_THICKFRAME, WS_MINIMIZEBOX, and WS_MAXIMIZEBOX styles.
/// - \c \b WS_POPUP	Creates a popup window. Cannot be used with child window styles.
/// - \c \b WS_POPUPWINDOW	Creates a popup window with WS_BORDER, WS_POPUP, and WS_SYSMENU
/// styles. The WS_CAPTION and WS_POPUPWINDOW styles combine to create a system
/// menu.
/// - \c \b WS_SYSMENU	Window has a system menu box in its title bar. Must also indicate
/// the WS_CAPTION style.
/// - \c \b WS_TABSTOP	Control can receive the keyboard focus when TAB key is pressed.
/// - \c \b WS_THICKFRAME	Window has a border that lets you change the window size.
/// - \c \b WS_VISIBLE	Window is initially visible.
/// - \c \b WS_VSCROLL	Window has a vertical scroll bar.
    uint32     Style;
    
/// Contains the extended style values of your window. These can be any one of the
/// extended style constants (WS_EX_DLGMODALFRAME, WS_EX_NOPARENTNOTIFY,
/// WS_EX_TOPMOST, WS_EX_SHADOW). See  TWindow::AdjustWindowRectEx  for a
/// description of these constants.
    uint32     ExStyle;
    
    int        X; ///< horizontal position of top left corenr of the window
    int        Y; ///< vertical position of top left corenr of the window
    int        W; ///< width of the window
    int        H; ///< height of the window
    
/// Contains the resource ID for the menu associated with this window. If no menu
/// exists, Menu is 0.
    TResId     Menu;        
    
/// Contains the identifier of the child window. For a dialog box control, Id is its
/// resource identifier. If Win32 is defined, Id is set to GetWindowLong ; otherwise
/// Id is set to GetWindowWord.
    int        Id;          
    
/// Contains a value that is passed to Windows when the window is created. This
/// value identifies a data block that is then available in the message response
/// functions associated with WM_CREATE. Param is used by TMDIClient and can be
/// useful when converting non-ObjectWindows code.
    LPVOID     Param;
    
/// Holds the resource ID for the window's accelerator table.
    TResId     AccelTable;  
};

//
/// \class TWindow
// ~~~~~ ~~~~~~~
/// TWindow, derived from TEventHandler and TStreamableBase, provides
/// window-specific behavior and encapsulates many functions that control window
/// behavior and specify window creation and registration attributes.
/// 
/// TWindow is a generic window that can be resized and moved. You can construct an
/// instance of TWindow, though normally you use TWindow as a base for your
/// specialized window classes. In general, to associate and disassociate a TWindow
/// object with a window element, you need to follow these steps:
/// - 1.	Construct an instance of a TWindow.
/// - 2.	Call Create or Execute, which creates the interface element (HWND) and then
/// calls SetupWindow, which calls the base SetupWindow for normal processing, which
/// in turn involves
/// - Creating the HWindow and any child HWindows.
/// - Calling TransferData to setup the transfer of data between the
/// parent and child windows.
/// - 3.	To destroy the interface element, choose one of the following actions,
/// depending on your application:
/// - Call Destroy to destroy the interface element unconditionally.
/// - Call CloseWindow, which calls CanClose to test if it is OK to
/// destroy the interface element.
/// - 4.	There are two ways to destroy the interface object:
/// - If the object has been new'd, use delete.
/// - If the object has not been new'd, the compiler automatically
/// destructs the object.
/// 
/// The ObjectWindows destroy process consists of two parts: (1) call Destroy to
/// destroy the interface element and (2) then delete the C++ object. However, it is
/// perfectly valid to call Destroy on the interface element without deleting the
/// C++ object and then to call Create at a later time to re-create the window.
/// Because it is also valid to construct a C++ window object on the stack or as an
/// aggregated member, the Destroy function cannot assume it should delete the C++
/// object.
/// 
/// The user-generated WM_CLOSE event handler, EvClose, also causes a C++ object to
/// be deleted by passing the this pointer to the application. The C++ object is
/// deleted automatically because the EvClose event frequently occurs in response to
/// a user action, and this is the most convenient place for the deletion to take
/// place. Later, when it's safe to do so, the application then deletes the window
/// pointer. Because the stack often contains selectors that refer to the addresses
/// of objects that may become invalid during the delete process, it is not safe to
/// delete the this pointer while events are still being processed. If the addresses
/// become invalid, they could cause trouble when they are reloaded from the stack.
/// 
/// TWindow is the base class for all window classes, including TFrameWindow,
/// TControl, TDialog, and TMDIChild. The ObjectWindows hierarchy diagram shows the
/// many classes that are derived from TWindow.
//
class _OWLCLASS TWindow : virtual public TEventHandler,
                          virtual public TStreamableBase {
  public:
    // Class scoped types
    //
    typedef HWND THandle;  ///< TWindow encapsulates an HWND

		/// \todo Make protected or private
    void TraceWindowPlacement(); //DLN debug
    
    // Constructors and destructor for TWindow
    //
    TWindow(TWindow* parent, LPCTSTR title = 0, TModule* module = 0);
    TWindow(TWindow* parent, const tstring& title, TModule* module = 0);
    TWindow(HWND handle, TModule* module = 0);

    virtual ~TWindow();

    /// \name Two iterators that take function pointers
    /// @{
    TWindow*          FirstThat(TCondFunc test, void* paramList = 0) const;
    void              ForEach(TActionFunc action, void* paramList = 0);
    /// @}

    /// \name Two iterators that take pointers to member functions
    /// @{
    TWindow*          FirstThat(TCondMemFunc test, void* paramList = 0);
    void              ForEach(TActionMemFunc action, void* paramList = 0);
    /// @}

    /// \name Other functions for iteration
    /// @{
    TWindow*          Next();
    void              SetNext(TWindow* next);
    TWindow*          GetFirstChild();
    TWindow*          GetLastChild();
    TWindow*          Previous();
    uint              NumChildren() const;   // Number of child windows
    /// @}

    /// \name Query and set the Owl window flags. 
    /// Accepts TWindowFlag args, possibly or'd together.
    /// @{
    void              SetFlag(uint mask);
    void              ClearFlag(uint mask);
    bool              IsFlagSet(uint mask);
    /// @}

    /// \name Sets/clears auto-create flag 
    /// It indicates that the TWindow should be
    /// created if a create is sent while in the parent's child list
    /// @{
    void              EnableAutoCreate();
    void              DisableAutoCreate();
    /// @}

    /// \name Retrieves/assigns tooltip of/to window
    /// @{
    virtual TTooltip* GetTooltip() const;
    virtual void       EnableTooltip(bool enable=true);
    void              SetTooltip(TTooltip* tooltip);
    /// @}

    /// \name Sets/clears transfer flag  
    /// It indicates that the TWindow can/will transfer data via the transfer mechanism
    /// @{
    void              EnableTransfer();
    void              DisableTransfer();
    /// @}

    /// \name Window's default module access functions
    // !CQ SetModule() should be protected!
    /// @{
    TModule*          GetModule() const;
    void              SetModule(TModule* module);
    /// @}

    /// Convenience functions - forwards the call to the module associated with this window.
    /// @{
    tstring LoadString(uint id) const {PRECONDITION(Module); return Module->LoadString(id);}
    HBITMAP LoadBitmap(TResId id) const {PRECONDITION(Module); return Module->LoadBitmap(id);}
    HACCEL LoadAccelerators(TResId id) const {PRECONDITION(Module); return Module->LoadAccelerators(id);}
    HMENU LoadMenu(TResId id) const {PRECONDITION(Module); return Module->LoadMenu(id);}
    HCURSOR LoadCursor(TResId id) const {PRECONDITION(Module); return Module->LoadCursor(id);}
    HICON LoadIcon(TResId id) const {PRECONDITION(Module); return Module->LoadIcon(id);}
    std::string LoadHtml(TResId id) const {PRECONDITION(Module); return Module->LoadHtml(id);}
    /// @}

    TApplication*     GetApplication() const;
    virtual bool      Register();

    TScroller*        GetScroller();
    void              SetScroller(TScroller* scroller);

    /// \name Create/destroy an native window to be associated with this window
    /// @{
    virtual bool      Create();
    bool              CreateChildren();
    virtual void      Destroy(int retVal = 0);
    /// @}

    /// \name Create a modal window, and perform actual modal execute call
    /// @{
    virtual int       Execute();
    virtual int       DoExecute();
    /// @}

    // Request this window to close itself
    //
    virtual void      CloseWindow(int retVal = 0);

    /// \name Unconditionally shut down a given window. 
    /// Destroy() is called to
    /// destroy the Handle, & then the window is deleted. Non-static version
    /// is safe as long as it is inline and not called on itself
    /// @{
    static void       ShutDownWindow(TWindow* win, int retVal = 0);
    void              ShutDownWindow(int retVal = 0);
    /// @}

    /// \name Attach or detach a window handle to a TWindow object.  
    /// Used when a child re-creates itself.
    /// @{
    void              AttachHandle(HWND handle);
    void              DetachHandle();
    /// @}

#if defined(BI_MULTI_THREAD_RTL)
    //Override TEventHandler::Dispatch() to handle multi-thread
    //synchronization
    //
    virtual TResult  Dispatch(TEventInfo&, TParam1, TParam2 = 0);
#endif

    /// \name Message preprocessing
    /// Called from TApplication::ProcessAppMsg() to give the window an
    /// opportunity to perform preprocessing of the Windows message
    /// @{
    virtual bool      PreProcessMsg(MSG& msg);
    virtual bool      IdleAction(long idleCount);
    virtual bool      HoldFocusHWnd(HWND hLose, HWND hGain);
    /// @}

    TWindowAttr&      GetWindowAttr();
    const TWindowAttr& GetWindowAttr() const;

    LPCTSTR           GetCaption() const;


    /// \name Child and parenting
    /// @{
    int               GetId() const;
    TWindow*          ChildWithId(int id) const;
    /// @}

    /// \name Get this window's parent. 
    /// Either the handle of native window, or a
    /// pointer to the OWL object. May return different objects in some cases.
    /// Use H & O varieties to avoid change across version
    /// @{
    HWND               GetParentH() const; // Native handle version
    TWindow*          GetParentO() const; // OWL object version
    TWindow*          GetParent() const;        // New version -- returns object

    /// @}

    virtual void      SetParent(TWindow* newParent);

    /// \name Other attributes
    /// @{
    virtual bool      SetDocTitle(LPCTSTR docname, int index);
    bool              SetDocTitle(const tstring& docname, int index) {return SetDocTitle(docname.c_str(), index);}
    void              SetCaption(LPCTSTR title);
    void              SetCaption(const tstring& title) {SetCaption(title.c_str());}
    void              SetCaption(uint resourceStringId);
    bool              SetCursor(TModule* module, TResId resId);
    void              SetBkgndColor(const TColor& color);
    void              SetAcceleratorTable(TResId resId);
    /// @}

    //Can close virtual tests whether all children and this window are ready
    //and able to close
    //
    virtual bool      CanClose();

    /// \name Message forwarding
    /// Forward the current event to "handle" using either PostMessage() or
    /// SendMessage(). Owl window version calls directly to window proc on send.
    /// @{
    TResult           ForwardMessage(HWND handle, bool send = true);
    TResult           ForwardMessage(bool send = true);
    /// @}

    // Send message to all children
    //
    void              ChildBroadcastMessage(TMsgId, TParam1 = 0, TParam2 = 0);

    /// \name Notify a window (parent usually) of a child action.
    /// @{
    void SendNotification(int id, int notifyCode, HWND hCtl, TMsgId = WM_COMMAND);
    void SendNotification(HWND receiver, int id, int notifyCode, HWND hCtl, TMsgId = WM_COMMAND);
    TResult SendNotification(int id, NMHDR&, TMsgId = WM_NOTIFY);
    TResult SendNotification(HWND receiver, uint id, NMHDR&, TMsgId = WM_NOTIFY);
    /// @}

#if defined(OWL5_COMPAT)

    // Obsolete. Forwards the call to message handler.
    //
    TResult ReceiveMessage(TMsgId msg, TParam1 p1 = 0, TParam2 p2 = 0)
    {return ReceiveMessage(Handle, msg, p1, p2);}

#endif

    TResult HandleMessage(TMsgId, TParam1 = 0, TParam2 = 0);

    /// \name Windows message handlers
    /// Virtual functions called to handle a message, and to deal with an
    /// unhandled message in a default way.
    /// @{
    virtual TResult   WindowProc(TMsgId, TParam1, TParam2);
    virtual TResult   DefWindowProc(TMsgId, TParam1, TParam2);
    /// @}

    // Called by WindowProc() to handle WM_COMMANDs
    //
    virtual TResult   EvCommand(uint id, HWND hWndCtl, uint notifyCode);

    // Called by WindowProc() to handle WM_NOTIFYs
    //
    virtual TResult   EvNotify(uint id, TNotify & notifyInfo);

    /// \name Called by WindowProc() to handle WM_COMMAND_ENABLE, & helper function
    /// @{
    virtual void      EvCommandEnable(TCommandEnabler& ce);
    void              RouteCommandEnable(HWND hInitCmdTarget, TCommandEnabler& ce);
    /// @}

    // Default processing, deals with special cases or calls DefWindowProc
    //
    TResult           DefaultProcessing();

    // Paint function called by base classes when responding to WM_PAINT
    //
    virtual void      Paint(TDC& dc, bool erase, TRect& rect);

    /// \name Transfer buffer functionality
    /// @{
#if defined(OWL5_COMPAT)
    void              SetTransferBuffer(void* transferBuffer);
#endif

    void              SetTransferBuffer(void* transferBuffer, uint size);
    
    template <class TBuffer>
    void              SetTransferBuffer(TBuffer* transferBuffer);

    template <class TElement, uint Count>
    void              SetTransferBuffer(TElement (&transferBuffer)[Count]);

    void*             GetTransferBuffer() const {return TransferBuffer;}
    uint              GetTransferBufferSize() const {return TransferBufferSize;}

    virtual uint      Transfer(void* buffer, TTransferDirection direction);
    virtual void      TransferData(TTransferDirection direction);
    /// @}

    // Installs the instance window procedure and saves the previous window
    // procedure in "DefaultProc"
    //
    void              SubclassWindowFunction();

    //-----------------------------------
    // Encapsulated native HWND functions inline
    //

    /// \name Allow a TWindow& to be used as an HWND in Windows API calls
    /// @{
    HWND               GetHandle() const;
    operator          HWND() const;
    bool              IsWindow() const;
    /// @}

    /// \name Messages
    /// @{
    TResult SendMessage(TMsgId, TParam1 = 0, TParam2 = 0) const;
    TResult SendDlgItemMessage(int childId, TMsgId, TParam1 = 0, TParam2 = 0);
    bool PostMessage(TMsgId, TParam1 = 0, TParam2 = 0);
    static HWND        GetCapture();
    HWND               SetCapture();
    static void       ReleaseCapture();
    static HWND        GetFocus();
    HWND               SetFocus();
    bool              IsWindowEnabled() const;
    virtual bool      EnableWindow(bool enable);
    void              SetRedraw(bool redraw);

    bool TrackMouseEvent(uint flags = TME_HOVER | TME_LEAVE, int hoverTime = HOVER_DEFAULT);
    bool CancelMouseEvent(uint flags = TME_HOVER | TME_LEAVE);
    TRACKMOUSEEVENT QueryMouseEventTracking() const;
    /// @}

    /// \name Window coordinates, dimensions...
    /// @{

    //
    //
    //
    void ClientToScreen(TPoint& point) const;

    //
    /// Function-style version of ClientToScreen
    //
    TPoint MapClientToScreen(const TPoint& p) const
    {TPoint q = p; ClientToScreen(q); return q;}

    //
    /// Maps the given points, specified in client coordinates in this window, to screen coordinates.
    //
    void MapClientToScreen(TPoint* p, int count) const 
    {::MapWindowPoints(GetHandle(), HWND_DESKTOP, p, count);}

    //
    /// Overload for array
    /// See MapClientToScreen(TPoint*, int).
    //
    template <size_t Count>
    void MapClientToScreen(TPoint (&p)[Count]) const 
    {MapClientToScreen(&p[0], Count);}

    //
    /// Functional-style overload for TRect; converts the given rectangle coordinates and returns the result.
    /// See MapClientToScreen(TPoint*, int).
    //
    TRect MapClientToScreen(const TRect& r) const 
    {TRect q = r; MapClientToScreen(q, 2); return q;}

    //
    //
    //
    void ScreenToClient(TPoint& point) const;

    //
    /// Functional-style version of ScreenToClient
    //
    TPoint MapScreenToClient(const TPoint& p) const 
    {TPoint q = p; ScreenToClient(q); return q;}

    //
    /// Maps the given points, specified in screen coordinates, to client coordinates in this window.
    //
    void MapScreenToClient(TPoint* p, int count) const 
    {::MapWindowPoints(HWND_DESKTOP, GetHandle(), p, count);}

    //
    /// Overload for array
    /// See MapScreenToClient(TPoint*, int).
    //
    template <size_t Count>
    void MapScreenToClient(TPoint (&p)[Count]) const 
    {MapScreenToClient(&p[0], Count);}

    //
    /// Functional-style overload for TRect; converts the given rectangle coordinates and returns the result.
    /// See MapScreenToClient(TPoint*, int).
    //
    TRect MapScreenToClient(const TRect& r) const 
    {TRect q = r; MapScreenToClient(q, 2); return q;}

    //
    //
    //
    void MapWindowPoints(HWND hWndTo, TPoint* pts, int count) const;

    //
    /// Overload for TWindow
    /// See MapWindowPoints(HWND, TPoint*, int).
    //
    void MapWindowPoints(const TWindow& wndTo, TPoint* p, int n) const 
    {MapWindowPoints(wndTo.GetHandle(), p, n);}

    //
    /// Overload for array
    /// See MapWindowPoints(HWND, TPoint*, int).
    //
    template <size_t Count>
    void MapWindowPoints(HWND hWndTo, TPoint (&p)[Count]) const 
    {MapWindowPoints(hWndTo, &p[0], Count);}

    //
    /// Overload for TWindow and array
    /// See MapWindowPoints(HWND, TPoint*, int).
    //
    template <size_t Count>
    void MapWindowPoints(const TWindow& wndTo, TPoint (&p)[Count]) const 
    {MapWindowPoints(wndTo, &p[0], Count);}

    //
    /// Functional-style overload for TPoint; converts the given point coordinates and returns the result.
    /// See MapWindowPoints(HWND, TPoint*, int).
    //
    TPoint MapWindowPoints(HWND hWndTo, const TPoint& p) const 
    {TPoint q = p; MapWindowPoints(hWndTo, &q, 1); return q;}

    //
    /// Overload for TWindow and TPoint
    /// See MapWindowPoints(HWND, const TPoint&).
    //
    TPoint MapWindowPoints(const TWindow& wndTo, const TPoint& p) const
    {return MapWindowPoints(wndTo.GetHandle(), p);}

    //
    /// Functional-style overload for TRect; converts the given rectangle coordinates and returns the result.
    /// See MapWindowPoints(HWND, TPoint*, int).
    //
    TRect MapWindowPoints(HWND hWndTo, const TRect& r) const 
    {TRect q = r; MapWindowPoints(hWndTo, q, 2); return q;}

    //
    /// Overload for TWindow and TRect
    /// See MapWindowPoints(HWND, const TRect&).
    //
    TRect MapWindowPoints(const TWindow& wndTo, const TRect& r) const
    {return MapWindowPoints(wndTo.GetHandle(), r);}

    //
    /// Maps the window's rectangle (GetWindowRect) to client coordinates in the given destination window.
    //
    TRect MapWindowRect(HWND hWndTo) const 
    {TRect r = GetWindowRect(); ::MapWindowPoints(HWND_DESKTOP, hWndTo, r, 2); return r;}

    //
    /// Overload for TWindow
    /// See MapWindowRect(HWND).
    //
    TRect MapWindowRect(const TWindow& wndTo) const
    {return MapWindowRect(wndTo.GetHandle());}

    //
    /// Maps the window's client rectangle (GetClientRect) to client coordinates in the given destination window.
    //
    TRect MapClientRect(HWND hWndTo) const
    {TRect r = GetClientRect(); MapWindowPoints(hWndTo, r, 2); return r;}

    //
    /// Overload for TWindow
    /// See MapClientRect(HWND).
    //
    TRect MapClientRect(const TWindow& wndTo) const
    {return MapClientRect(wndTo.GetHandle());}

    void              GetClientRect(TRect& rect) const;
    TRect             GetClientRect() const;
    static HWND       WindowFromPoint(const TPoint& point);
    HWND              ChildWindowFromPoint(const TPoint& point) const;
    void              GetWindowRect(TRect& rect) const;
    TRect             GetWindowRect() const;
    static void       AdjustWindowRect(TRect& rect, uint32 style, bool menu);
    static void       AdjustWindowRectEx(TRect& rect, uint32 style, bool menu, uint32 exStyle);

    //
    /// Returns the window rectangle of the given window, specified in this window's client coordinates.
    //
    TRect GetChildRect(HWND hWnd) const
    {
      TRect r;
      ::GetWindowRect(hWnd, &r);
      ::MapWindowPoints(HWND_DESKTOP, GetHandle(), r, 2);
      return r;
    }

    //
    /// Overload for TWindow
    /// See GetChildRect(HWND).
    //
    TRect GetChildRect(const TWindow& wnd) const
    {return GetChildRect(wnd.GetHandle());}

    /// @}

    /// \name Window and class Words and Longs, window properties
    /// @{
    long              GetClassName(LPTSTR className, int maxCount) const;
    long              GetClassLong(int index) const;
    long              SetClassLong(int index, long newLong);
    uint16            GetClassWord(int index) const;
    uint16            SetClassWord(int index, uint16 newWord);
    LONG_PTR          GetWindowLongPtr(int index) const;
    LONG_PTR          SetWindowLongPtr(int index, LONG_PTR newLong);
    long              GetWindowLong(int index) const;
    long              SetWindowLong(int index, long newLong);
    uint16            GetWindowWord(int index) const;
    uint16            SetWindowWord(int index, uint16 newWord);
    WNDPROC           GetWindowProc() const;
    WNDPROC           SetWindowProc(WNDPROC wndProc);
    int               EnumProps(PROPENUMPROC proc);
    HANDLE            GetProp(uint16 atom) const;
    HANDLE            RemoveProp(uint16 atom) const;
    bool              SetProp(uint16 atom, HANDLE data) const;
    HANDLE            GetProp(LPCTSTR str) const;
    HANDLE            GetProp(const tstring& str) const {return GetProp(str.c_str());}
    HANDLE            RemoveProp(LPCTSTR str) const;
    HANDLE            RemoveProp(const tstring& str) const {return RemoveProp(str.c_str());}
    bool              SetProp(LPCTSTR str, HANDLE data) const;
    bool              SetProp(const tstring& str, HANDLE data) const {return SetProp(str.c_str(), data);}
    /// @}

    /// \name Dual mode accessors. 
    /// Work with Attr and other members as well as the underlying window information
    /// @{
    uint32            GetStyle() const;
    uint32            SetStyle(uint32 style);
    uint32            GetExStyle() const;
    uint32            SetExStyle(uint32 style);
    bool              ModifyStyle(uint32 offBits, uint32 onBits,
                                  uint swpFlags = 0);
    bool              ModifyExStyle(uint32 offBits, uint32 onBits,
                                    uint swpFlags = 0);
    /// @}

    /// \name Window placement(X,Y) and display
    /// @{
    bool              MoveWindow(int x, int y, int w, int h, bool repaint = false);
    bool              MoveWindow(const TRect& rect, bool repaint = false);
    virtual bool      ShowWindow(int cmdShow);
    void              ShowOwnedPopups(bool show);
    bool              IsWindowVisible() const;
    bool              IsZoomed() const;
    bool              IsIconic() const;
    int               GetWindowTextLength() const;
    int               GetWindowText(LPTSTR str, int maxCount) const;
    tstring        GetWindowText() const;
    void              SetWindowText(LPCTSTR str);
    void              SetWindowText(const tstring& str) {SetWindowText(str.c_str());}
    void              SetWindowText(uint resourceStringId);
    bool              GetWindowPlacement(WINDOWPLACEMENT* place) const;
    bool              SetWindowPlacement(const WINDOWPLACEMENT* place);
    /// @}

    /// \name Window positioning(Z), sibling relationships
    /// @{
    void              BringWindowToTop();
    static HWND        GetActiveWindow();
    HWND               SetActiveWindow();
    static HWND        GetDesktopWindow();
    HWND               GetLastActivePopup() const;
    HWND               GetNextWindow(uint dirFlag) const;
    HWND              GetTopWindow() const;
    HWND               GetWindow(uint cmd) const;
    bool              SetWindowPos(HWND      hWndInsertAfter,
                                   const TRect& rect,
                                   uint         flags);
    bool              SetWindowPos(HWND      hWndInsertAfter,
                                   int x, int y, int w, int h,
                                   uint         flags);
    /// @}

    /// \name Window painting: invalidating, validating & updating
    /// @{
    virtual void      Invalidate(bool erase = true);
    virtual void      InvalidateRect(const TRect& rect, bool erase = true);
    void              InvalidateRgn(HRGN hRgn, bool erase = true);
    void              Validate();
    void              ValidateRect(const TRect& rect);
    void              ValidateRgn(HRGN hRgn);
    void              UpdateWindow();
    bool              FlashWindow(bool invert);
    bool              GetUpdateRect(TRect& rect, bool erase = true) const;
    int               GetUpdateRgn(TRegion& rgn, bool erase = true) const;
    bool              LockWindowUpdate(bool lock=true);
    bool              RedrawWindow(TRect* update,
                                   HRGN   hUpdateRgn,
                                   uint   redrawFlags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
    /// @}
    /// \name Scrolling and scrollbars
    /// @{

    bool GetScrollInfo(int bar, SCROLLINFO* scrollInfo) const;
    SCROLLINFO GetScrollInfo(int bar, uint mask = SIF_ALL) const;
    int SetScrollInfo(int bar, SCROLLINFO* scrollInfo, bool redraw = true);
    int GetScrollPos(int bar) const;
    int SetScrollPos(int bar, int pos, bool redraw = true);
    int GetScrollTrackPos(int bar) const;
    void GetScrollRange(int bar, int& minPos, int& maxPos) const;
    typedef std::pair<int, int> TScrollRange;
    TScrollRange GetScrollRange(int bar) const;
    void SetScrollRange(int bar, int minPos, int maxPos, bool redraw = true);
    void SetScrollRange(int bar, const TScrollRange&, bool redraw = true);
    int GetScrollPage(int bar) const;
    void SetScrollPage(int bar, int page, bool redraw = true);

    bool EnableScrollBar(uint sbFlags = SB_BOTH, uint arrowFlags = ESB_ENABLE_BOTH);
    void ShowScrollBar(int bar, bool show = true);
    void ScrollWindow(int dx, int dy, const TRect* scroll = 0, const TRect* clip = 0);
    void ScrollWindowEx(
      int dx,
      int dy,
      const TRect* scroll = 0,
      const TRect* clip = 0,
      HRGN hUpdateRgn = 0,
      TRect* update = 0,
      uint flags = 0);

    /// @}
    /// \name Parent/child with Ids
    /// @{
    int               GetDlgCtrlID() const;
    HWND               GetDlgItem(int childId) const;
    uint              GetDlgItemInt(int   childId,
                                    bool* translated = 0,
                                    bool  isSigned = true) const;
    void              SetDlgItemInt(int  childId,
                                    uint value,
                                    bool isSigned = true) const;
    int               GetDlgItemText(int childId, LPTSTR text, int maxValue) const;
    tstring        GetDlgItemText(int childId) const;
    void              SetDlgItemText(int childId, LPCTSTR text) const;
    void              SetDlgItemText(int childId, const tstring& text) const {SetDlgItemText(childId, text.c_str());}
    void              SetDlgItemText(int childId, uint resourceStringId) const {SetDlgItemText(childId, LoadString(resourceStringId));}
    uint              IsDlgButtonChecked(int buttonId) const;
    bool              IsChild(HWND hWnd) const;
    HWND              GetNextDlgGroupItem(HWND hWndCtrl,
                                          bool previous = false) const;
    HWND              GetNextDlgTabItem(HWND hWndCtrl,
                                        bool previous = false) const;
    void              CheckDlgButton(int buttonId, uint check);
    void              CheckRadioButton(int firstButtonId,
                                       int lastButtonId,
                                       int checkButtonId);

    void SetDlgItemFont(int childId, HFONT f, bool redraw = false) const
    {TWindow(GetDlgItem(childId)).SetWindowFont(f, redraw);}

    HFONT GetDlgItemFont(int childId) const
    {return TWindow(GetDlgItem(childId)).GetWindowFont();}

    //
    /// Returns the window rectangle of the given child, specified in this window's client coordinates.
    //
    TRect GetDlgItemRect(int childId) const
    {return GetChildRect(GetDlgItem(childId));}

    /// @}

    /// \name Menus and menubar
    /// @{
    HMENU             GetMenu() const;
    HMENU             GetSystemMenu(bool revert = false) const;
    bool              SetMenu(HMENU hMenu);
    bool              HiliteMenuItem(HMENU hMenu, uint idItem, uint hilite);
    void              DrawMenuBar();
    /// @}

    /// \name Timer
    /// @{
    bool              KillTimer(UINT_PTR timerId);
    UINT_PTR          SetTimer(UINT_PTR timerId, uint timeout, TIMERPROC proc = 0);
    /// @}

    /// \name Caret, cursor, font
    /// @{
    void              CreateCaret(HBITMAP hBitmap);
    void              CreateCaret(bool isGray, int width, int height);
    static uint       GetCaretBlinkTime();
    static void       GetCaretPos(TPoint& point);
    static TPoint GetCaretPos() {TPoint p; GetCaretPos(p); return p;}
    void              HideCaret();
    static void       SetCaretBlinkTime(uint16 milliSecs);
    static void       SetCaretPos(int x, int y);
    static void       SetCaretPos(const TPoint& pos);
    void              ShowCaret();
    static void       DestroyCaret();
    static void       GetCursorPos(TPoint& pos);
    static TPoint GetCursorPos() {TPoint p; GetCursorPos(p); return p;}
    static TPoint GetMessagePos() {return TPoint(::GetMessagePos());}
    void              SetWindowFont(HFONT font, bool redraw);
    HFONT             GetWindowFont();
    /// @}
    

    /// \name Hot keys
    /// @{
    bool              RegisterHotKey(int idHotKey, uint modifiers, uint virtKey);
    bool              UnregisterHotKey(int idHotKey);
    /// @}

    /// \name Misc
    /// @{
    bool              WinHelp(LPCTSTR helpFile, uint command, uint32 data);
    bool              WinHelp(const tstring& helpFile, uint command, uint32 data) {return WinHelp(helpFile.c_str(), command, data);}
    void              AssignContextMenu(TPopupMenu* menu);
    TPopupMenu*       GetContextMenu() const;
    int               MessageBox(LPCTSTR text, LPCTSTR caption = 0, uint flags = MB_OK);
    int               MessageBox(const tstring& text, const tstring& caption = tstring(), uint flags = MB_OK);
    int               MessageBox(uint resId, LPCTSTR caption = 0, uint flags = MB_OK);
    int               MessageBox(uint resId, const tstring& caption, uint flags = MB_OK);
    int               FormatMessageBox(LPCTSTR text, LPCTSTR caption, uint flags, va_list argp);
    int               FormatMessageBox(LPCTSTR text, LPCTSTR caption, uint flags, ...);
    int               FormatMessageBox(const tstring& text, const tstring& caption, uint flags, va_list argp);
    int               FormatMessageBox(const tstring& text, const tstring& caption, uint flags, ...);
    int               FormatMessageBox(uint resId, LPCTSTR caption, uint flags, va_list argp);
    int               FormatMessageBox(uint resId, LPCTSTR caption = 0, uint flags = MB_OK, ...);
    int               FormatMessageBox(uint resId, const tstring& caption, uint flags, va_list argp);
    int               FormatMessageBox(uint resId, const tstring& caption, uint flags = MB_OK, ...);
    HTASK             GetWindowTask() const;
    void              DragAcceptFiles(bool accept);

    TCurrentEvent&    GetCurrentEvent();
    void              SethAccel(HACCEL);
    /// @}

  protected:
    /// \name These events are processed by TWindow
    /// @{
    void              EvClose();
    int               EvCreate(CREATESTRUCT & createStruct);
    void              EvDestroy();
    int               EvCompareItem(uint ctrlId, const COMPAREITEMSTRUCT& compareInfo);
    void              EvDeleteItem(uint ctrlId, const DELETEITEMSTRUCT& deleteInfo);
    void              EvDrawItem(uint ctrlId, const DRAWITEMSTRUCT& drawInfo);
    void              EvMeasureItem(uint ctrlId, MEASUREITEMSTRUCT & measureInfo);
    void              EvHScroll(uint scrollCode, uint thumbPos, HWND hWndCtl);
    void              EvVScroll(uint scrollCode, uint thumbPos, HWND hWndCtl);
    void              EvMove(const TPoint& clientOrigin);
    void              EvNCDestroy();
    bool              EvQueryEndSession(uint flags);
    void              EvSize(uint sizeType, const TSize& size);
    void              EvLButtonDown(uint modKeys, const TPoint& point);
    bool              EvEraseBkgnd(HDC);
    void              EvPaint();
    void              EvSysColorChange();
    TResult           EvWin32CtlColor(TParam1, TParam2);
    /// @}

    /// \name Input validation message handler
    /// @{
    void              EvChildInvalid(HWND hWnd);
    /// @}

    /// \name System messages
    /// @{
    void              EvCommNotify(uint commId, uint status);
    void              EvCompacting(uint compactRatio);
    void              EvDevModeChange(LPCTSTR devName);
    void              EvEnable(bool enabled);
    void              EvEndSession(bool endSession, uint flags);
    void              EvFontChange();
    int               EvPower(uint powerEvent);
    void              EvSysCommand(uint cmdType, const TPoint& point);
    void              EvSystemError(uint error);
    void              EvTimeChange();
    void              EvTimer(uint timerId);
    void              EvWinIniChange(LPCTSTR section);
    /// @}

    /// \name Window manager messages
    /// @{
    void              EvActivate(uint active,
                                 bool minimized,
                                 HWND hWndOther /* may be 0 */);
    void              EvActivateApp(bool active, HTASK hTask);
    void              EvCancelMode();
    void              EvGetMinMaxInfo(MINMAXINFO & minmaxinfo);
    void              EvGetText(uint buffSize, LPTSTR buff);
    uint              EvGetTextLength();
    void              EvIconEraseBkgnd(HDC hDC);
    void              EvKillFocus(HWND hWndGetFocus /* may be 0 */);
    uint              EvMouseActivate(HWND hTopLevel, uint hitCode, TMsgId);
    /// @}

    /// \name The following are called under Win32 only
    /// @{
    void              EvInputFocus(bool gainingFocus);
    void              EvOtherWindowCreated(HWND hWndOther);
    void              EvOtherWindowDestroyed(HWND hWndOther);
    void              EvPaintIcon();
    void              EvHotKey(int idHotKey);
    bool              EvCopyData(HWND hwnd, const COPYDATASTRUCT& dataStruct);

    void              EvNextDlgCtl(TParam1 hctlOrDir, uint isHCtl);
    void              EvParentNotify(uint event, TParam1, TParam2);
    HANDLE            EvQueryDragIcon();
    bool              EvQueryOpen();
    void              EvQueueSync();
    bool              EvSetCursor(HWND hWndCursor, uint hitTest, TMsgId mouseMsg);
    void              EvSetFocus(HWND hWndLostFocus /* may be 0 */);
    HFONT             EvGetFont();
    void              EvSetFont(HFONT hFont, bool redraw);
    void              EvSetRedraw(bool redraw);
    void              EvSetText(LPCTSTR text);
    void              EvShowWindow(bool show, uint status);
    void              EvWindowPosChanged(const WINDOWPOS& windowPos);
    void              EvWindowPosChanging(WINDOWPOS & windowPos);
    /// @}

    /// \name Controls
    /// @{
    HBRUSH            EvCtlColor(HDC hDC, HWND hWndChild, uint ctlType);
    /// @}

    /// \name Keyboard input
    /// @{
    void              EvChar(uint key, uint repeatCount, uint flags);
    void              EvDeadChar(uint deadKey, uint repeatCount, uint flags);
    void              EvKeyDown(uint key, uint repeatCount, uint flags);
    void              EvKeyUp(uint key, uint repeatCount, uint flags);
    void              EvSysChar(uint key, uint repeatCount, uint flags);
    void              EvSysDeadChar(uint key, uint repeatCount, uint flags);
    void              EvSysKeyDown(uint key, uint repeatCount, uint flags);
    void              EvSysKeyUp(uint key, uint repeatCount, uint flags);
    /// @}

    /// \name Mouse input
    /// @{
    void              EvLButtonDblClk(uint modKeys, const TPoint& point);
    void              EvLButtonUp(uint modKeys, const TPoint& point);
    void              EvMButtonDblClk(uint modKeys, const TPoint& point);
    void              EvMButtonDown(uint modKeys, const TPoint& point);
    void              EvMButtonUp(uint modKeys, const TPoint& point);
    void              EvMouseMove(uint modKeys, const TPoint& point);
    bool              EvMouseWheel(uint modKeys, int zDelta, const TPoint& point);
    void              EvRButtonDblClk(uint modKeys, const TPoint& point);
    void              EvRButtonDown(uint modKeys, const TPoint& point);
    void              EvRButtonUp(uint modKeys, const TPoint& point);
    TResult            EvRegisteredMouseWheel(TParam1, TParam2);
    /// @}


    /// \name Menu related messages
    /// @{
    void              EvInitMenu(HMENU hMenu);
    void              EvInitMenuPopup(HMENU hPopupMenu,
                                      uint  index,
                                      bool  sysMenu);
    int32             EvMenuChar(uint nChar, uint menuType, HMENU hMenu);
    void              EvMenuSelect(uint menuItemId, uint flags, HMENU hMenu);
    void              EvContextMenu(HWND childHwnd, int x, int y);
    /// @}

    /// \name Dialog [Menu] messages
    /// @{
    uint              EvGetDlgCode(const MSG* msg);
    void              EvEnterIdle(uint source, HWND hWndDlg);
    /// @}

    /// \name Print manager messages
    /// @{
    void              EvSpoolerStatus(uint jobStatus, uint jobsLeft);
    /// @}

    /// \name Clipboard messages
    /// @{
    void              EvAskCBFormatName(uint bufLen, LPTSTR buffer);
    void              EvChangeCBChain(HWND hWndRemoved, HWND hWndNext);
    void              EvDrawClipboard();
    void              EvDestroyClipboard();
    void              EvHScrollClipboard(HWND hCBViewer, uint scrollCode, uint pos);
    void              EvPaintClipboard(HWND hWnd, HANDLE hPaintStruct);
    void              EvRenderAllFormats();
    void              EvRenderFormat(uint dataFormat);
    void              EvSizeClipboard(HWND hWndViewer, HANDLE hRect);
    void              EvVScrollClipboard(HWND hCBViewer, uint scrollCode, uint pos);
    /// @}

    /// \name Palette manager messages
    /// @{
    void              EvPaletteChanged(HWND hWndPalChg);
    void              EvPaletteIsChanging(HWND hWndPalChg);
    bool              EvQueryNewPalette();
    /// @}

    /// \name Drag-n-drop messages
    /// @{
    void              EvDropFiles(TDropInfo dropInfo);
    /// @}

    /// \name List box messages
    /// @{
    int               EvCharToItem(uint key, HWND hWndListBox, uint caretPos);
    int               EvVKeyToItem(uint key, HWND hWndListBox, uint caretPos);
    /// @}

    /// \name Non-client messages
    /// @{
    bool              EvNCActivate(bool active);
    uint              EvNCCalcSize(bool calcValidRects, NCCALCSIZE_PARAMS & params);
    bool              EvNCCreate(CREATESTRUCT & createStruct);
    uint              EvNCHitTest(const TPoint& point);
    void              EvNCLButtonDblClk(uint hitTest, const TPoint& point);
    void              EvNCLButtonDown(uint hitTest, const TPoint& point);
    void              EvNCLButtonUp(uint hitTest, const TPoint& point);
    void              EvNCMButtonDblClk(uint hitTest, const TPoint& point);
    void              EvNCMButtonDown(uint hitTest, const TPoint& point);
    void              EvNCMButtonUp(uint hitTest, const TPoint& point);
    void              EvNCMouseMove(uint hitTest, const TPoint& point);

    // WM_NCPAINT now passes an HRGN under Win32.
    //
    void              EvNCPaint(HRGN);

    void              EvNCRButtonDblClk(uint hitTest, const TPoint& point);
    void              EvNCRButtonDown(uint hitTest, const TPoint& point);
    void              EvNCRButtonUp(uint hitTest, const TPoint& point);
    /// @}

    /// \name Icon messages
    /// \todo There is no implementation for these functions
    /// @{
    HICON             EvGetIcon(bool largeIcon);
    HICON             EvSetIcon(bool largeIcon, HICON icon);
    /// @}

    /// \name Callback procs for hooking TWindow to native window
    /// @{
    static LRESULT CALLBACK InitWndProc(HWND, UINT msg, WPARAM, LPARAM);
    /// @}

  protected:
    // Constructor & subsequent initializer for use with virtual derivations
    // Immediate derivitives must call Init() before constructions are done.
    //
    TWindow();
    void              Init(TWindow* parent, LPCTSTR title, TModule* module);
    void              Init(TWindow* parent, const tstring& title, TModule* module) {Init(parent, title.c_str(), module);}
    void              Init(HWND hWnd, TModule* module);

    // 
    /// For code safety and 64-bit compatibility the old parameter to PerformCreate
    /// has been deprecated. In the strict build mode, the parameter is no longer
    /// used, and the signature of PerformCreate has been changed to force a 
    /// compilation error for legacy code. This is achieved by changing the return 
    /// type but leaving the old parameter in place, although now unused.
    //
#if OWL_STRICT
    typedef THandle TPerformCreateReturnType;
#else
    typedef void TPerformCreateReturnType;
#endif

    virtual TPerformCreateReturnType PerformCreate(int deprecated_argument = 0);
    
    void SetHandle(THandle);
    TPerformCreateReturnType SetOrReturnHandle(THandle);

    // Resynching state
    //
    void              GetHWndState(bool forceStyleSync = false);
    void              GetWindowTextTitle();

    virtual void      GetWindowClass(WNDCLASS& wndClass);

#if defined(OWL5_COMPAT)
    typedef LPTSTR TGetClassNameReturnType;
#else
    typedef LPCTSTR TGetClassNameReturnType;
#endif
    virtual TGetClassNameReturnType GetClassName();

    void              PerformSetupAndTransfer();
    virtual void      SetupWindow();
    virtual void      CleanupWindow();

    void              DispatchScroll(uint scrollCode, uint thumbPos, HWND hWndCtrl);

    void              LoadAcceleratorTable();
    virtual void      RemoveChild(TWindow* child);

    TWindow*          GetWindowPtr(HWND hWnd) const;

    // Member data  // !CQ need to add accessors for many of these
    //
  public_data:

    /// Holds the handle to the associated MS-Windows window, which you'll need to
    /// access if you make calls directly to Windows API functions.
    //
    HWND         Handle;   

/// Points to the window's caption. When there is a valid HWindow, Title will yield
/// the same information as ::GetWindowText if you use TWindow::SetCaption to set
/// it.
    LPTSTR             Title;    // Logical title. Usually the same as window text
    
/// Points to the interface object that serves as the parent window for this interface object.
    TWindow*          Parent;   // Owl parent, use GetParentO(), SetParent()

/// Holds a TWindowAttr structure, which contains the window's creation attributes.
/// These attributes, which  include the window's style, extended style, position,
/// size, menu ID, child window ID, and menu accelerator table ID, are passed to the
/// function that creates the window.
    TWindowAttr       Attr;     

/// Holds the address of the default window procedure. DefWindowProc calls
/// DefaultProc to process Windows messages that are not handled by the window.
    WNDPROC           DefaultProc; 

/// Points to the scroller object that supports either the horizontal or vertical
/// scrolling for this window.
    TScroller*        Scroller; // Scrolling helper object

  protected_data:

    /// Holds the handle to the current Windows accelerator table associated with this window.
    //
    HACCEL          HAccel;
    
/// Holds the module ID for the specified cursor. A value of 0 indicates a standard system cursor.
    TModule*          CursorModule;
    
/// Holds the cursor resource ID for the window's cursor. If the data member
/// CursorModule is 0, CursorResId can be one of the following IDC_Xxxx constants
/// that represent different kinds of cursors:
/// - \c \b IDC_ARROW	Customary arrow cursor
/// - \c \b IDC_CROSS	Crosshair cursor
/// - \c \b IDC_IBEAM	I-beam cursor
/// - \c \b IDC_ICON	Unfilled icon cursor
/// - \c \b IDC_SIZE	A smaller square in the right inside corner of a larger square
/// - \c \b IDC_SIZENESW	Dual-pointing cursor with arrows pointing southwest and northeast
/// - \c \b IDC_SIZENS	Dual-pointing cursor with arrows pointing south and north
/// - \c \b IDC_SIZENWSE	Dual-pointing cursor with arrows pointing southeast and northwest
/// - \c \b IDC_SIZEWE	Dual-pointing cursor with arrows pointing east and west
/// - \c \b IDC_UPARROW	Vertical arrow cursor
/// - \c \b IDC_WAIT	Hourglass cursor
    TResId            CursorResId;
    
/// Holds a handle to the window's cursor. The cursor is retrieved using
/// CursorModule and CursorResId and set using SetCursor.
    HCURSOR           HCursor;

/// Stores the current background color set by TWindow::SetBkgndColor.
    TColor            BkgndColor;

    TPopupMenu*       ContextPopupMenu;   ///< Popup menu used for right click
    TTooltip*          Tooltip;            // Tooltip

#if defined(OWL5_COMPAT)
  protected_data: // TransferBuffer is proteced or maybe even public.
#else
  private: // TransferBuffer is always private.
#endif

/// Points to a buffer to be used in transferring data in and out of the TWindow
/// object. A TWindow object assumes that the buffer contains data used by the
/// windows in its child list. If TransferBuffer is 0, no data is to be transferred.
    void*             TransferBuffer;

/// Specifies the size of the transfer buffer pointed to by TransferBuffer.
    uint              TransferBufferSize;
    
  private:
    WNDPROC           InstanceProc; ///< The window proc for this window instance
    TApplication*     Application;  ///< Application that this window belongs to
    TModule*          Module;       ///< Default module used for getting resources
    uint32            Flags;
    uint16            ZOrder;
    TWindow*          ChildList;
    TWindow*          SiblingList;
    uint32            UniqueId;

    static uint32     LastUniqueId;

    // Instance window proc interface
    //
    WNDPROC           CreateInstanceProc();
    WNDPROC           GetInstanceProc() const;
    void              InitInstanceProc();
    void              FreeInstanceProc();

    // Internal functions
    //
    HMENU MakeMenuOrId();
    void              PerformInit(TWindow* parent, TModule* module);
    bool              OrderIsI(TWindow* win, void* position);
    void              AssignZOrder();
    void              AddChild(TWindow* child);
    int               IndexOf(TWindow* child) const;
    TWindow*          At(int position);
    void              SetUniqueId();

    TResult ReceiveMessage(HWND, TMsgId, TParam1 = 0, TParam2 = 0) throw();
    friend void* GetMessageReceiverMemberFunctionAddress();
    friend TResult CALLBACK DispatchWindowMessage(TWindow* w, TMsgId msg, TParam1 param1, TParam2 param2);

    // Hidden to prevent accidental copying or assignment
    //
    TWindow(const TWindow&);
    TWindow& operator =(const TWindow&);

  DECLARE_RESPONSE_TABLE(TWindow);
//  DECLARE_STREAMABLE(_OWLCLASS, owl::TWindow, 3);
  DECLARE_STREAMABLE_OWL(TWindow, 3);
};  // class TWindow

// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TWindow );

//
/// \class TXWindow
// ~~~~~ ~~~~~~~~
/// A nested class, TXWindow describes an exception that results from trying to
/// create an invalid window.
//
class _OWLCLASS TXWindow : public TXOwl {
  public:
    TXWindow(TWindow* win = 0, uint resourceId = IDS_INVALIDWINDOW);

    TXWindow(const TXWindow& src);
    int Unhandled(TModule* app, uint promptResId);

    virtual TXWindow* Clone() const; // override
    void Throw();

    static void Raise(TWindow* win = 0, uint resourceId = IDS_INVALIDWINDOW);

    TWindow* GetWindow() const;

  public_data:
/// Points to the window object that is associated with the exception.
    TWindow*      Window;
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------

//
/// A TActionFunc defined in window.cpp
//
void DoEnableAutoCreate(TWindow* win, void*);

//
/// Raw function to retrieve a TWindow pointer given an HWND from the a given
/// app, or any app (app==0). Use the TWindow or TApplication member versions
/// in new code for more protection.
//
_OWLFUNC(TWindow*) GetWindowPtr(HWND hWnd, const TApplication* app = 0);

#if defined(__TRACE) || defined(__WARN)
_OWLCFUNC(std::ostream&) operator <<(std::ostream& os, const TWindow& w);
#endif

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Retrieves the id of the command.
//
inline uint TCommandEnabler::GetId() const
{
  return Id;
}

//
/// Returns true if this command enabler sends a command message.
//
inline bool TCommandEnabler::SendsCommand() const
{
  return !(Flags & NonSender);
}

//
/// Return true if the command enabler has been handled.
//
inline bool TCommandEnabler::GetHandled()
{
  return Flags & WasHandled;
}

//
/// Returns true if receiver is the same as the message responder originally set up
/// in the constructor.
//
inline bool TCommandEnabler::IsReceiver(HWND hReceiver)
{
  return hReceiver == HWndReceiver;
}

//
/// Returns the handle of the window that this enabler was destined for.
//
inline HWND TCommandEnabler::GetReceiver() const
{
  return HWndReceiver;
}

//
/// Sets the receiver for the enabler.
//
inline void TCommandEnabler::SetReceiver(HWND hReceiver)
{
  HWndReceiver = hReceiver;
}

//
/// Marks that the enabler has been handled.
//
inline void TCommandEnabler::SetHandled()
{
  Flags |= WasHandled;
}

//----------------------------------------------------------------------------
// TWindow inlines
//

//
/// Returns a pointer to the next sibling window in the window's sibling list.
inline TWindow* TWindow::Next()
{
  return SiblingList;
}

//
/// Sets the next window in the sibling list.
inline void TWindow::SetNext(TWindow* next)
{
  SiblingList = next;
}

//
/// Returns a pointer to the first child window, which is the first window created
/// in the interface object's child list.
inline TWindow* TWindow::GetFirstChild(){
  return ChildList ? ChildList->SiblingList : 0;
}

//
/// Returns a pointer to the last child window in the interface object's child list.
inline TWindow* TWindow::GetLastChild(){
  return ChildList;
}

//
/// Sets the specified TWindow wfXxxx constant flags (for example wfAlias,
/// wfTransfer, and so on) in the Flags member.
inline void TWindow::SetFlag(uint mask){
  Flags |= uint32(mask);
}

//
/// Clears the specified TWindow wfXxxx constant flags (for example wfAlias,
/// wfTransfer, and so on) in the Flags member.
inline void TWindow::ClearFlag(uint mask){
  Flags &= uint32(~mask);
}

//
/// Returns the state of the bit flag in Attr.Flags whose mask is supplied. Returns
/// true if the bit flag is set, and false if not set.
inline bool TWindow::IsFlagSet(uint mask){
  return (Flags & mask) ? true : false;
}

//
/// Ensures that an associated child window interface element is created and
/// displayed along with its parent window. By default, this feature is enabled for
/// windows and controls, but disabled for dialog boxes.
inline void TWindow::EnableAutoCreate(){
  SetFlag(wfAutoCreate);
}

//
/// Disables the feature that allows an associated child window interface element to
/// be created and displayed along with its parent window. Call DisableAutoCreate
/// for pop-up windows and controls if you want to create and display them at a time
/// later than their parent windows.
inline void TWindow::DisableAutoCreate(){
  ClearFlag(wfAutoCreate);
}

//
// get Tooltip
//
inline TTooltip* TWindow::GetTooltip() const{
  return Tooltip;
}

//
/// Enables the transfer mechanism, which allows state data to be transferred
/// between the window and a transfer buffer.
inline void TWindow::EnableTransfer(){
  SetFlag(wfTransfer);
}

//
/// Disables (for the interface object) the transfer mechanism, which allows state
/// data to be transferred to and from a transfer buffer.
inline void TWindow::DisableTransfer(){
  ClearFlag(wfTransfer);
}

//
/// Returns a pointer to the module object.
inline TModule* TWindow::GetModule() const{
  return Module;
}

//
/// Sets the default module for this window.
inline void TWindow::SetModule(TModule* module){
  Module = module;
}

//
/// Gets a pointer to the TApplication object associated with this. Use
/// GetApplication to obtain access to data and functions in the TApplication
/// object.
inline TApplication* TWindow::GetApplication() const{
  return Application;
}

//
inline WNDPROC TWindow::GetInstanceProc() const{
  return InstanceProc;
}

//
/// Returns the associated scroller object for this window.
//
inline TScroller* TWindow::GetScroller(){
  return Scroller;
}

//
/// This inline version of ShutDownWindow calls the static version of
/// ShutDownWindow.
inline void TWindow::ShutDownWindow(int retVal){
  ShutDownWindow(this, retVal);
}

//
/// Returns Attr.Id, the ID used to find the window in a specified parent's child
/// list.
inline int TWindow::GetId() const{
  return Attr.Id;
}

/// Returns the TWindowAttr structure, which contains the window's creation attributes.
inline TWindowAttr& TWindow::GetWindowAttr()
{
	return Attr;
}

/// Returns the TWindowAttr structure, which contains the window's creation attributes.
inline const TWindowAttr& TWindow::GetWindowAttr() const
{
	return Attr;
}

/// Returns the Title member of TWindow. Returns 0 if the caption has not yet been set.
/// \note It may differ from the actual window caption, if not set via TWindow::SetCaption().
/// TWindow::GetWindowTextTitle() can be used to keep Title synchronized.
inline LPCTSTR TWindow::GetCaption() const
{
	return Title;
}


//
/// Sets the background color for the window. You can also get the current color of
/// an element displayed on the screen. For example,
/// layout -> SetBkgndColor(GetSysColor(COLOR_APPWORKSPACE));
/// 
/// Uses one of the Windows COLOR values (in this case, the color of multiple
/// document interface (MDI) applications).
inline void TWindow::SetBkgndColor(const TColor& color){
  BkgndColor = color;
}

#if defined(OWL5_COMPAT)
//
/// Sets TransferBuffer only - for backward compatibility
inline void TWindow::SetTransferBuffer(void* transferBuffer){
  TransferBuffer = transferBuffer;
  TransferBufferSize = 0;
}
#endif

//
/// Sets TransferBuffer and TransferBufferSize.
inline void TWindow::SetTransferBuffer(void* transferBuffer, uint size){
  TransferBuffer = transferBuffer;
  TransferBufferSize = size;
}

//
/// Sets TransferBuffer and TransferBufferSize.
/// The size is inferred from the type of the passed pointer.
template <class TBuffer>
inline void TWindow::SetTransferBuffer(TBuffer* transferBuffer){
  SetTransferBuffer(transferBuffer, sizeof *transferBuffer);
}

//
/// Sets TransferBuffer and TransferBufferSize.
/// The size is inferred from the type of the passed array.
template <class TElement, uint Count>
inline void TWindow::SetTransferBuffer(TElement (&transferBuffer)[Count]){
  SetTransferBuffer(&transferBuffer[0], sizeof transferBuffer);
}

//----------------------------------------------------------------------------
// HWND wrappers
//

//
/// Return the handle of the parent.
//
inline HWND TWindow::GetParentH() const{
  return GetHandle() ? ::GetParent(GetHandle()) : 0;
}

//
/// Return the OWL's parent for this window.
//
inline TWindow* TWindow::GetParentO() const{
  return Parent;
}

//
/// Retrieves the OWL object of the parent window. If none exists, returns 0.
//
inline TWindow* TWindow::GetParent() const{
  return GetParentO();
}

//
/// Returns the handle of the window.
//
inline HWND TWindow::GetHandle() const{
  return Handle;
}

//
/// Allows a TWindow& to be used as an HWND in Windows API calls by providing an
/// implicit conversion from TWindow to HWND.
inline TWindow::operator HWND() const{
  return GetHandle();
}

//
/// Sets the window handle in a derived class. Used by derived classes that create
/// their window handle in a class-specific way.
inline void TWindow::SetHandle(THandle handle){
  Handle = handle;
}

//
/// Returns true if an HWND is being used.
inline bool TWindow::IsWindow() const{
  return ::IsWindow(GetHandle());
}

//
/// Returns the current event to be processed in the message queue.
//
inline TCurrentEvent& TWindow::GetCurrentEvent(){
  return GetApplication()->GetCurrentEvent();
}

//
/// Repacks a command message (msg) so that a child window (hCtl) can send a message
/// to its parent regardless of whether this is a WIN16 or WIN32 application.
inline void
TWindow::SendNotification(int id, int notifyCode, HWND hCtl, TMsgId msg){
  SendMessage(msg, MkParam1(id, notifyCode), TParam2(hCtl));
}

//
/// Repacks a command message (msg) so that a child window (hCtl) can send a message
/// to its parent regardless of whether this is a WIN16 or WIN32 application.
inline void
TWindow::SendNotification(HWND receiver, int id, int notifyCode,
                          HWND hCtl, TMsgId msg)
{
  PRECONDITION(receiver && ::IsWindow(receiver));
  ::SendMessage(receiver, msg, MkParam1(id, notifyCode), TParam2(hCtl));
}

//
/// Repacks a command message (msg) so that a child window (hCtl) can send a message
/// to its parent regardless of whether this is a WIN16 or WIN32 application.
inline TResult
TWindow::SendNotification(int id, NMHDR& nmhdr, TMsgId msg){
  return SendMessage(msg, TParam1(id), TParam2(&nmhdr));
}

//
/// Repacks a command message (msg) so that a child window (hCtl) can send a message
/// to its parent regardless of whether this is a WIN16 or WIN32 application.
//
inline TResult
TWindow::SendNotification(HWND receiver, uint id, NMHDR& nmhdr, TMsgId msg){
  return ::SendMessage(receiver, msg, TParam1(id), TParam2(&nmhdr));
}

//
/// Sends a message (msg) to the control specified in childId.
/// \note Wrapper for Windows API.
//
inline TResult
TWindow::SendDlgItemMessage(int childId, TMsgId msg, TParam1 p1, TParam2 p2){
  PRECONDITION(GetHandle());
  return ::SendDlgItemMessage(GetHandle(), childId, msg, p1, p2);
}

//
/// Posts a message (msg) to the window in the application's message queue.
/// PostMessage returns without waiting for the corresponding window to process the
/// message.
/// \note Wrapper for Windows API.
//
inline bool TWindow::PostMessage(TMsgId msg, TParam1 p1, TParam2 p2){
  PRECONDITION(GetHandle());
  return ::PostMessage(GetHandle(), msg, p1, p2);
}

//
/// Returns the handle of the window that has captured the mouse.
/// \note Wrapper for Windows API.
//
inline HWND TWindow::GetCapture(){
  return ::GetCapture();
}

//
/// Sets the mouse capture to the current window. All mouse input is directed to
/// this window. 
/// \note Wrapper for Windows API.
//
inline HWND TWindow::SetCapture(){
  PRECONDITION(GetHandle());
  return ::SetCapture(GetHandle());
}

//
/// Releases the mouse capture from this window.
/// \note Wrapper for Windows API.
//
inline void TWindow::ReleaseCapture(){
  ::ReleaseCapture();
}

//
/// Gets a handle to the window that has the focus. Use the function SetFocus to set
/// the keyboard focus to this window.
/// \note Wrapper for Windows API.
//
inline HWND TWindow::GetFocus(){
  return ::GetFocus();
}

//
/// Sets the keyboard focus to current window and activates the window that receives
/// the focus by sending a WM_SETFOCUS message to the window. All future keyboard
/// input is directed to this window, and any previous window that had the input
/// focus loses it. If successful, SetFocus returns a handle to the window that has
/// the focus; otherwise, it returns NULL.
/// \note Wrapper for Windows API.
//
inline HWND TWindow::SetFocus(){
  PRECONDITION(GetHandle());
  return ::SetFocus(GetHandle());
}

//
/// Returns true if the window is enabled. Use the function EnableWindow to enable
/// or disable a window.
/// \note Wrapper for Windows API.
//
inline bool TWindow::IsWindowEnabled() const{
  PRECONDITION(GetHandle());
  return ::IsWindowEnabled(GetHandle());
}

//
/// Allows the given window to receive input from the keyboard of mouse. If enable
/// is true, the window can receive input. Use the function IsWindowEnabled to
/// determine if the window has been enabled.
/// \note Wrapper for Windows API.
//
inline bool TWindow::EnableWindow(bool enable){
  PRECONDITION(GetHandle());
  return ::EnableWindow(GetHandle(), enable);
}

//
/// Sends a WM_SETREDRAW message to a window so that changes can be redrawn (redraw
/// = true) or to prevent changes from being redrawn (redraw = false).
/// \note Wrapper for Windows API.
//
inline void TWindow::SetRedraw(bool redraw){
  PRECONDITION(GetHandle());
  SendMessage(WM_SETREDRAW, redraw);
}

//
/// Uses the screen coordinates specified in point to calculate the client window's
/// coordinates and then places the new coordinates into point.
/// \note Wrapper for Windows API.
//
inline void TWindow::ScreenToClient(TPoint& point) const{
  PRECONDITION(GetHandle());
  ::ScreenToClient(GetHandle(), &point);
}

//
/// Maps a set of points in one window to a relative set of points in another
/// window. hWndTo specifies the window to which the points are converted. points
/// points to the array containing the points. If hWndTo is 0, the points are
/// converted to screen coordinates. count specifies the number of points structures
/// in the array.
/// \note Wrapper for Windows API.
//
inline void
TWindow::MapWindowPoints(HWND hWndTo, TPoint* points, int count) const
{
  PRECONDITION(GetHandle());
  ::MapWindowPoints(GetHandle(), hWndTo, points, count);
}

//
/// Gets the coordinates of the window's client area (the area in a window you can
/// use for drawing).
/// \note Wrapper for Windows API.
//
inline TRect TWindow::GetClientRect() const{
  TRect  rect;
  GetClientRect(rect);
  return rect;
}

//
/// Returns the handle of the window in which the specified point (point) lies.
/// \note Wrapper for Windows API.
//
inline HWND TWindow::WindowFromPoint(const TPoint& point){
  return ::WindowFromPoint(point);
}

//
/// Determines which of the child windows contains the point specified in TPoint.
/// Returns a handle to the window that contains the point, or 0 if the point lies
/// outside the parent window.
/// \note Wrapper for Windows API.
//
inline HWND
TWindow::ChildWindowFromPoint(const TPoint& point) const{
  PRECONDITION(GetHandle());
  return ::ChildWindowFromPoint(GetHandle(), point);
}

//
/// Converts the client coordinates specified in point to screen coordinates for the
/// new window.
/// \note Wrapper for Windows API.
//
inline void TWindow::ClientToScreen(TPoint& point) const{
  PRECONDITION(GetHandle());
  ::ClientToScreen(GetHandle(), &point);
}

//
/// Gets the screen coordinates of the window's rectangle.
/// \note Wrapper for Windows API.
//
inline TRect TWindow::GetWindowRect() const{
  TRect  rect;
  GetWindowRect(rect);
  return rect;
}

//
/// Calculates the size of the window rectangle according to the indicated
/// client-rectangle size. rect refers to the structure that contains the client
/// rectangle's coordinates. style specifies the style of the window. menu is true
/// if the window has a menu.
/// \note Wrapper for Windows API.
//
inline void
TWindow::AdjustWindowRect(TRect& rect, uint32 style, bool menu)
{
  ::AdjustWindowRect(&rect, style, menu);
}

//
/// Calculates the size of a window rectangle that has an extended style. TRect
/// refers to the class that contains the client rectangle's coordinates. style
/// specifies the window styles of the window to be adjusted. menu returns true if
/// the window has a menu. exStyle indicates the extended styles to be used for the
/// window. Extended styles include the following styles:
/// - \c \b WS_EX_ACCEPTFILES	The window can make use of drag and drop files.
/// - \c \b WS_EX_DLGMODALFRAME	The window has a double border that can be created with a
/// title bar if the WS_CAPTION style flag is specified.
/// - \c \b WS_EX_NOPARENTNOTIFY	The child window created from this style does not send
/// parent notify messages to the parent window when the child is created or
/// destroyed.
/// - \c \b WS_EX_TOPMOST	A window having this style is placed above windows that are not
/// topmost and remains above the non-topmost windows even when it's deactivated.
/// - \c \b WS_EX_TRANSPARENT	A window having this style is transparent, that is, any
/// windows beneath this window are not concealed by this window.
///
/// \note Wrapper for Windows API.
//
inline void
TWindow::AdjustWindowRectEx(TRect& rect, uint32 style, bool menu, uint32 exStyle)
{
  ::AdjustWindowRectEx(&rect, style, menu, exStyle);
}

//
// Window and class Words and Longs, window properties
//
/// Returns the class name for a generic OWL window.
///
/// Returns the Windows registration class name. The default class name is generated
/// using the module name plus Window.  If you are registering a new class or
/// changing the name of an existing window class, override this function in your
/// derived class.
//
inline long
TWindow::GetClassName(LPTSTR className, int maxCount) const
{
  PRECONDITION(GetHandle());
  return ::GetClassName(GetHandle(), className, maxCount);
}

//
/// Retrieves the 32-bit value containing information about the window class. If
/// unsuccessful, returns 0.  Depending on the value of index, GetClassLong can
/// retrieve the following information:
/// - \c \b GCL_CBCLSEXTRA	Size in bytes of memory associated with this class
/// - \c \b GCL_CBWINDEXTRA	Size of extra window memory associated with each window
/// - \c \b GCL_HBRBACKGROUND	Handle of the background brush associated with the class
/// - \c \b GCL_HCURSOR	Handle of the cursor
/// - \c \b GCL_HICON	Handle of the icon
/// - \c \b GCL_HMODULE	Handle of the module that registered the class
/// - \c \b GCL_MENUNAME	Address of the menu name string
/// - \c \b GCL_STYLE	The style bits associated with a window class
/// - \c \b GCL_WNDPROC	Address of the window procedure associated with this class
inline long TWindow::GetClassLong(int index) const{
  PRECONDITION(GetHandle());
  return ::GetClassLong(GetHandle(), index);
}

//
/// Sets the long value at the specified offset (index). Depending on the value of
/// index, SetClassLong sets a handle to a background brush, cursor, icon, module,
/// menu, window function, or extra class bytes.
inline long TWindow::SetClassLong(int index, long newLong){
  PRECONDITION(GetHandle());
  return ::SetClassLong(GetHandle(), index, newLong);
}

//
/// Gets a 16-bit value containing information about the class or style of the
/// window. If unsuccessful; returns 0. Depending on the value of index,
/// GetClassWord can retrieve the following information:
/// - \c \b GCW_CBCLSEXTRA	Number of additional class information
/// - \c \b GCW_CBWINDEXTRA	Number of bytes of additional window information
/// - \c \b GCW_HBRBACKGROUND	Handle of the background brush
/// - \c \b GCW_HCURSOR	Handle of the cursor
/// - \c \b GCW_HICON	Handle of the icon
/// - \c \b GCW_HMODULE	Handle of the module
/// - \c \b GCW_STYLE	The style bits associated with a window class
inline uint16 TWindow::GetClassWord(int index) const{
  PRECONDITION(GetHandle());
  return ::GetClassWord(GetHandle(), index);
}

//
/// Sets the word value at the specified offset (index). Depending on the value of
/// index, SetClassLong sets the number of bytes of class information, of additional
/// window information, or the style bits. Unlike SetClassLong, SetClassWord uses
/// one of the following GCW_xxxx Class Word constants:
/// - \c \b GCW_HBRBACKGROUND	Sets a handle for a background brush.
/// - \c \b GCW_HCURSOR	Sets a handle of a cursor.
/// - \c \b GCW_HICON	Sets a handle of an icon.
/// - \c \b GCW_STYLE	Sets a style bit for a window class.
inline uint16 TWindow::SetClassWord(int index, uint16 newWord){
  PRECONDITION(GetHandle());
  return ::SetClassWord(GetHandle(), index, newWord);
}

//
/// Retrieves information about the window depending on the value stored in index.
/// The values returned, which provide information about the window, include the
/// following GWL_Xxxx window style constants:
/// - \c \b GWL_EXSTYLE 	The extended window style
/// - \c \b GWL_STYLE	The window style (position, device context creation, size, and so on)
/// 
/// \note The constants GWL_WNDPROC, GWL_HINSTANCE, GWL_HWDPARENT, GWL_USERDATA should not be used anymore.
/// They are replaced with GWLP_WNDPROC, GWLP_HINSTANCE, GWLP_HWNDPARENT, GWLP_USERDATA which should be passed
/// to GetWindowLongPtr()/SetWindowLongPtr()
///
/// In the case of a dialog box, additional information can be retrieved, such as:
/// - \c \b DWL_DLGPROC 	The address of the procedure processed by the dialog box
/// - \c \b DWL_MSGRESULT 	The value that a message processed by the dialog box returns
/// - \c \b DWL_USER	Additional information that pertains to the application, such as
/// pointers or handles the application uses.
//
inline long
TWindow::GetWindowLong(int index) const
{
  PRECONDITION(GetHandle());
  return ::GetWindowLong(GetHandle(), index);
}

//
/// Changes information about the window. Depending on the value of index,
/// SetWindowLong sets a handle to a background brush, cursor, icon, module, menu,
/// or window function. The window style can be one of the GWL_xxxx values that
/// represent styles.
inline long
TWindow::SetWindowLong(int index, long newLong)
{
  PRECONDITION(GetHandle());
  return ::SetWindowLong(GetHandle(), index, newLong);
}

//
/// Retrieves information about the window depending on the value stored in index.
/// The values returned, which provide information about the window, include the
/// following GWLP_Xxxx window style constants:
/// - \c \b GWLP_WNDPROC  	The address of the window procedure being processed
/// - \c \b GWLP_HINSTANCE
/// - \c \b GWLP_HWNDPARENT
/// - \c \b GWLP_USERDATA
/// - \c \b GWLP_ID
/// 
//
inline LONG_PTR
TWindow::GetWindowLongPtr(int index) const
{
  PRECONDITION(GetHandle());
  return ::GetWindowLongPtr(GetHandle(), index);
}

//
/// Changes information about the window. For a list of constants, see GetWindowLongPtr()
//
inline LONG_PTR
TWindow::SetWindowLongPtr(int index, LONG_PTR newLong)
{
  PRECONDITION(GetHandle());
  return ::SetWindowLongPtr(GetHandle(), index, newLong);
}

//
/// Retrieves information about this window depending on the value of index.
/// GetWindowWord returns one of the following values that indicate information
/// about the window:
/// - \c \b GWW_HINSTANCE	The instance handle of the module owning the window
/// - \c \b GWW_HWNDPARENT	The handle of the parent window
/// - \c \b GWW_ID	The ID number of the child window
inline uint16
TWindow::GetWindowWord(int index) const
{
  PRECONDITION(GetHandle());
  return ::GetWindowWord(GetHandle(), index);
}

//
/// Changes information about the window. index specifies a byte offset of the word
/// to be changed to the new value (newWord).
inline uint16
TWindow::SetWindowWord(int index, uint16 newWord)
{
  PRECONDITION(GetHandle());
  return ::SetWindowWord(GetHandle(), index, newWord);
}

inline WNDPROC           
TWindow::GetWindowProc() const
{
  PRECONDITION(GetHandle());
  return WNDPROC(GetWindowLongPtr(GWLP_WNDPROC));
}

//
inline WNDPROC           
TWindow::SetWindowProc(WNDPROC wndProc)
{
  PRECONDITION(GetHandle());
  return WNDPROC(SetWindowLongPtr(GWLP_WNDPROC, LONG_PTR(wndProc)));
}

//
/// Enumerates all the items in the property list of the current window and passes
/// them one by one to the callback function indicated in proc. The process
/// continues until every item has been enumerated or until proc returns zero. proc
/// holds the address of the callback function.
/// \note Wrapper for Windows API.
//
inline int
TWindow::EnumProps(PROPENUMPROC proc)
{
  PRECONDITION(GetHandle());
  return ::EnumProps(GetHandle(), proc);
}

//
/// Returns a handle to the property list of the specified window. atom contains a
/// value that identifies the character string whose handle is to be retrieved. If
/// the specified string is not found in the property list for this window, returns
/// NULL.
/// \note Wrapper for Windows API.
//
inline HANDLE
TWindow::GetProp(uint16 atom) const
{
  PRECONDITION(GetHandle());
  return ::GetProp(GetHandle(), (LPCTSTR)(uint32)atom);
}

//
/// Removes the property specified by atom from the application's property list.
/// atom indicates the string to be removed. Returns the handle of the given string
/// or NULL if no string exists in the window's property list.
/// \note Wrapper for Windows API.
//
inline HANDLE
TWindow::RemoveProp(uint16 atom) const
{
  PRECONDITION(GetHandle());
  return ::RemoveProp(GetHandle(), (LPCTSTR)(uint32)atom);
}

//
/// Adds an item to the property list of the specified window. atom contains a value
/// that identifies the data entry to be added to the property list.
/// \note Wrapper for Windows API.
inline bool
TWindow::SetProp(uint16 atom, HANDLE data) const
{
  PRECONDITION(GetHandle());
  return ::SetProp(GetHandle(), (LPCTSTR)(uint32)atom, data);
}

//
/// Returns a handle to the property list of the specified window. Unlike the Syntax
/// 1 GetProp function, string points to the string whose handle is to be retrieved.
/// If the specified string is not found in the property list for this window,
/// returns NULL.
/// \note Wrapper for Windows API.
//
inline HANDLE
TWindow::GetProp(LPCTSTR str) const
{
  PRECONDITION(GetHandle());
  return ::GetProp(GetHandle(), str);
}

//
/// Removes the property specified by str, a null-terminated string, from the
/// application's property list. Returns the handle of the given string or NULL if
/// no string exists in the window's property list.
/// \note Wrapper for Windows API.
//
inline HANDLE
TWindow::RemoveProp(LPCTSTR str) const
{
  PRECONDITION(GetHandle());
  return ::RemoveProp(GetHandle(), str);
}

//
/// Adds an item to the property list of the specified window. str points to the
/// string used to identify the entry data to be added to the property list.
/// \note Wrapper for Windows API.
//
inline bool
TWindow::SetProp(LPCTSTR str, HANDLE data) const
{
  PRECONDITION(GetHandle());
  return ::SetProp(GetHandle(), str, data);
}

//
/// Repositions the specified window. x and y specify the new upper left coordinates
/// of the window; w and h specify the new width and height, respectively. If
/// repaint is false, the window is not repainted after it is moved.
/// \note MoveWindow calls thru TWindow::SetWindowPos to allow moving before creation
//
inline bool
TWindow::MoveWindow(int x, int y, int w, int h, bool repaint)
{
  return SetWindowPos(0, x, y, w, h,
                      SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE |
                      (repaint ? 0 : SWP_NOREDRAW));
}

//
/// Repositions the window. rect references the left and top coordinates and the
/// width and height of the new screen rectangle. If repaint is false, the window is
/// not repainted after it is moved.
inline bool
TWindow::MoveWindow(const TRect& rect, bool repaint)
{
  return MoveWindow(rect.left, rect.top, rect.Width(), rect.Height(), repaint);
}

//
/// Shows or hides all owned pop-up windows according to the value of show.
/// \note Wrapper for Windows API.
//
inline void
TWindow::ShowOwnedPopups(bool show)
{
  PRECONDITION(GetHandle());
  ::ShowOwnedPopups(GetHandle(), show);
}

//
/// Returns true if the window is visible. By default, TWindow's constructor sets
/// the window style attribute (WS_VISIBLE) so that the window is visible.
/// \note Wrapper for Windows API.
//
inline bool
TWindow::IsWindowVisible() const
{
  if(GetHandle())
    return ::IsWindowVisible(GetHandle());
  return Attr.Style & WS_VISIBLE;
}

//
/// Returns true if window is zoomed or maximized.
/// \note Wrapper for Windows API.
//
inline bool
TWindow::IsZoomed() const
{
  PRECONDITION(GetHandle());
  return ::IsZoomed(GetHandle());
}

//
/// Returns true if window is iconic or minimized.
/// \note Wrapper for Windows API.
//
inline bool
TWindow::IsIconic() const
{
  PRECONDITION(GetHandle());
  return ::IsIconic(GetHandle());
}

//
/// Returns the length, in characters, of the specified window's title. If the
/// window is a control, returns the length of the text within the control. If the
/// window does not contain any text, GetWindowTextLength returns 0.
/// \note Wrapper for Windows API.
//
inline int
TWindow::GetWindowTextLength() const
{
  PRECONDITION(GetHandle());
  return ::GetWindowTextLength(GetHandle());
}

//
/// Copies the window's title into a buffer pointed to by string. maxCount indicates
/// the maximum number of characters to copy into the buffer. A string of characters
/// longer than maxCount is truncated. GetWindowText returns the length of the
/// string or 0 if no title exists.
/// \note Wrapper for Windows API.
//
inline int
TWindow::GetWindowText(LPTSTR str, int maxCount) const
{
  PRECONDITION(GetHandle());
  return ::GetWindowText(GetHandle(), str, maxCount);
}


//
/// Sets the window title to a buffer pointed to by str. maxCount indicates the
/// number of characters to copy into the buffer. Note that this does not update
/// this window's Title member. Use SetCaption if the window's Title member needs to
/// be synchronized with the window's title.
/// \note Wrapper for Windows API.
//
inline void
TWindow::SetWindowText(LPCTSTR str)
{
  PRECONDITION(GetHandle());
  ::SetWindowText(GetHandle(), str);
}

//
/// Sets the window title to the resource string identified by the given id.
//
inline void
TWindow::SetWindowText(uint resourceStringId) 
{
  SetWindowText(LoadString(resourceStringId));
}

//
/// Retrieves display and placement information (normal, minimized, and maximized
/// positions) about the window and stores that information in the argument, place.
/// \note Wrapper for Windows API.
//
inline bool
TWindow::GetWindowPlacement(WINDOWPLACEMENT* place) const
{
  PRECONDITION(GetHandle());
  place->length = sizeof(WINDOWPLACEMENT);
  return ::GetWindowPlacement(GetHandle(), place);
}

//
/// Sets the window to a display mode and screen position. place points to a window
/// placement structure that specifies whether the window is to be hidden, minimized
/// or displayed as an icon, maximized, restored to a previous position, activated
/// in its current form, or activated and displayed in its normal position.
/// \note Wrapper for Windows API.
//
inline bool
TWindow::SetWindowPlacement(const WINDOWPLACEMENT* place)
{
  PRECONDITION(GetHandle());
  ((WINDOWPLACEMENT*)place)->length = sizeof(WINDOWPLACEMENT);
  return ::SetWindowPlacement(GetHandle(), place);
}

//
/// Brings a pop-up or child window to the top of the stack of overlapping windows
/// and activates it.
/// \note Wrapper for Windows API.
//
inline void
TWindow::BringWindowToTop()
{
  PRECONDITION(GetHandle());
  ::BringWindowToTop(GetHandle());
}

//
/// Retrieves the handle of the active window. Returns 0 if no window is associated
/// with the calling thread.
/// \note Wrapper for Windows API.
//
inline HWND
TWindow::GetActiveWindow()
{
  return ::GetActiveWindow();
}

//
/// Activates a top-level window. Returns a handle to the previously active window.
/// \note Wrapper for Windows API.
//
inline HWND
TWindow::SetActiveWindow()
{
  PRECONDITION(GetHandle());
  return ::SetActiveWindow(GetHandle());
}

//
/// Returns a handle to the desktop window.
/// \note Wrapper for Windows API.
//
inline HWND
TWindow::GetDesktopWindow()
{
  return ::GetDesktopWindow();
}


//
/// Finds the handle associated with either the next or previous window in the
/// window manager's list. dirFlag specifies the direction of the search. Under the
/// Win 32 API, GetNextWindow returns either the next or the previous window's
/// handle. If the application is not running under Win32, GetNextWindow returns the
/// next window's handle.
/// \note Wrapper for Windows API.
//
inline HWND
TWindow::GetNextWindow(uint flag) const
{
  PRECONDITION(GetHandle());
  return ::GetWindow(GetHandle(), flag);
}

//
/// Returns the last active pop-up window in the list.
/// \note Wrapper for Windows API.
//
inline HWND
TWindow::GetLastActivePopup() const
{
  PRECONDITION(GetHandle());
  return ::GetLastActivePopup(GetHandle());
}

//
/// Returns the handle of the window that has the indicated relationship to this
/// window.  cmd, which indicates the type of relationship to be obtained, can be
/// one of the following values:
/// - \c \b GW_CHILD	If the given window is a parent window, the return value indicates the
/// child window at the top of the Z order (the position of a window in a series of
/// overlapping windows arranged in a stack). Otherwise, the return value is 0. Only
/// child windows are examined.
/// - \c \b GW_HWNDFIRST	The return value indicates the window at the top of the Z order.
/// If this window is a topmost window, the return value identifies the topmost
/// window at the top of the Z order. If this window is a top-level window, the
/// return value identifies the top-level window at the bottom of the Z order. If
/// this window is a child window, the return value indicates the sibling window at
/// the bottom of the Z order.
/// - \c \b GW_HWNDNEXT	The return value identifies the window below the given window in the
/// Z order. If this window is a topmost window, the return value identifies the
/// topmost window below this window. If this window is a top-level window, the
/// return value indicates the top-level window below this window. If this window is
/// a child window, the return value indicates the sibling window below this window.
/// - \c \b GW_HWNDPREV	The return value identifies the window above the given window in the
/// Z order. If this window is a topmost window, the return value identifies the
/// topmost window above this window. If this window is a top-level window, the
/// return value indicates the top-level window above this window. If this window is
/// a child window, the return value indicates the sibling window above this window.
/// - \c \b GW_OWNER	The return value identifies this window's owner window, if one exists.
///
/// \note Wrapper for Windows API.
//
inline HWND
TWindow::GetWindow(uint flag) const
{
//  PRECONDITION(GetHandle());
  return ::GetWindow(GetHandle(), flag);
}

//
/// Returns a handle to the top window currently owned by this parent window. If no
/// children exist, GetTopWindow returns 0.
/// \note Wrapper for Windows API.
//
inline HWND
TWindow::GetTopWindow() const
{
  PRECONDITION(GetHandle());
  return ::GetTopWindow(GetHandle());
}

//
/// Changes the size of the window pointed to by rect. flags contains one of the
/// SWP_Xxxx Set Window Position constants (described below) that specify the size
/// and position of the window. If flags is set to SWP_NOZORDER, SetWindowPos
/// ignores the hWndInsertAfter parameter and retains the current ordering of the
/// child, pop-up, or top-level windows.
inline bool
TWindow::SetWindowPos(HWND hWndInsertAfter, const TRect& rect, uint flags)
{
  return SetWindowPos(hWndInsertAfter,
                      rect.left, rect.top, rect.Width(), rect.Height(), flags);
}

//
/// Invalidates (mark for painting) the entire client area of a window. The window
/// then receives a message to redraw the window. By default, the background of the
/// client area is marked for erasing.
/// \note Wrapper for Windows API.
//
inline void
TWindow::Invalidate(bool erase)
{
  PRECONDITION(GetHandle());
  ::InvalidateRect(GetHandle(), 0, erase);
}

//
/// Invalidates a specified client area. By default, the background of the client
/// area to be invalidated is marked for erasing.
/// \note Wrapper for Windows API.
//
inline void
TWindow::InvalidateRect(const TRect& rect, bool erase)
{
  PRECONDITION(GetHandle());
  ::InvalidateRect(GetHandle(), &rect, erase);
}

//
/// Invalidates a client area within a region specified by the hRgn  parameter when
/// the application receives a WM_PAINT message. The region to be invalidated is
/// assumed to have client coordinates. If hRgn is 0, the entire client area is
/// included in the region to be updated. The parameter erase specifies whether the
/// background with the update region needs to be erased when the region to be
/// updated is determined.  If erase is true, the background is erased; if erase  is
/// false, the background is not erased when the Paint function is called. By
/// default, the background within the region is marked for erasing.
/// \note Wrapper for Windows API.
//
inline void
TWindow::InvalidateRgn(HRGN hRgn, bool erase)
{
  PRECONDITION(GetHandle());
  ::InvalidateRgn(GetHandle(), hRgn, erase);
}

//
/// Calls the function ValidateRect to validate (that is, remove from the area to be
/// updated) the entire client area (the area you can use for drawing).
/// \note Wrapper for Windows API.
//
inline void
TWindow::Validate()
{
  PRECONDITION(GetHandle());
  ::ValidateRect(GetHandle(), 0);
}

//
/// Validates a portion of the client area indicated by rect.
/// \note Wrapper for Windows API.
//
inline void
TWindow::ValidateRect(const TRect& rect)
{
  PRECONDITION(GetHandle());
  ::ValidateRect(GetHandle(), &rect);
}

//
/// Validates the client area within a region of the current window. hRgn is a
/// handle to the client area that is to be removed from the region to be updated.
/// If hRgn is NULL, the entire client area is removed from the region to be
/// updated.
/// \note Wrapper for Windows API.
//
inline void
TWindow::ValidateRgn(HRGN hRgn)
{
  PRECONDITION(GetHandle());
  ::ValidateRgn(GetHandle(), hRgn);
}

//
/// Updates the client area of the specified window by immediately sending a
/// WM_PAINT message.
/// \note Wrapper for Windows API.
//
inline void
TWindow::UpdateWindow()
{
  PRECONDITION(GetHandle());
  ::UpdateWindow(GetHandle());
}

//
/// Changes the window from active to inactive or vice versa. If invert is nonzero,
/// the window is flashed. If invert is 0, the window is returned to its original
/// state–either active or inactive.
/// \note Wrapper for Windows API.
//
inline bool
TWindow::FlashWindow(bool invert)
{
  PRECONDITION(GetHandle());
  return ::FlashWindow(GetHandle(), invert);
}

//
/// Retrieves the screen coordinates of the rectangle that encloses the updated
/// region of the specified window. erase specifies whether GetUpdateRect should
/// erase the background of the updated region.
/// \note Wrapper for Windows API.
//
inline bool
TWindow::GetUpdateRect(TRect& rect, bool erase) const
{
  PRECONDITION(GetHandle());
  return ::GetUpdateRect(GetHandle(), &rect, erase);
}

//
/// Prevents or enables window drawing for one window at a time. If the window is
/// locked, returns true; otherwise, returns false, which indicates either that an
/// error occurred or that some other window is already locked.
/// If any drawing is attempted within a locked window or locked child windows, the
/// extent of the attempted operation is saved within a bounding rectangle. When the
/// window is then unlocked, the area within the rectangle is invalidated, causing a
/// paint message to be sent to this window. If any drawing occurred while the
/// window was locked for updates, the area is invalidated.
/// \note Wrapper for Windows API.
//
inline bool
TWindow::LockWindowUpdate(bool lock)
{
  PRECONDITION(GetHandle());
  return ::LockWindowUpdate(lock?GetHandle():0);
}

//
/// Redraws the rectangle specified by update and the region specified by
/// hUpdateRgn. redrawFlags can be a combination of one or more of the following
/// RDW_Xxxx Redraw Window constants used to invalidate or validate a window:
///
/// Flags that invalidate a window
/// - \c \b RDW_ERASE	When the window is repainted, it receives a WM_ERASEBKGND message. If
/// - \c \b RDW_INVALIDATE is not also specified, this flag has no effect.
/// - \c \b RDW_FRAME	Any part of the non-client area of the window receives a WM_NCPAINT
/// message if it intersects the region to be updated.
/// - \c \b RDW_INTERNALPAINT	A WM_PAINT message is posted to the window whether or not it
/// contains an invalid region.
/// - \c \b RDW_INVALIDATE	Invalidates either hUpdateRgn or update. In cases where both are
/// 0, the entire window becomes invalid.
/// 
/// Flags that validate a window
/// - \c \b RDW_NOERASE	The window is prevented from receiving any WM_ERASEBKGND messages.
/// - \c \b RDW_NOFRAME	The window is prevented from receiving any WM_NCPAINT messages. The
/// flag RDW_VALIDATE must also be used with this flag.
/// - \c \b RDW_NOINTERNALPAINT	The window is prevented from receiving internal WM_PAINT
/// messages, but does not prevent the window from receiving WM_PAINT messages from
/// invalid regions.
/// - \c \b RDW_VALIDATE	Validates update and hUpdateRgn. However, if both are 0, the
/// entire window area is validated. The flag does not have any effect on internal
/// WM_PAINT messages.
/// 
/// Flags that control when the window is repainted
/// - \c \b RDW_ERASENOW	Before the function returns, the specified windows will receive
/// WM_NCPAINT and WM_ERASEBKGND messages.
/// - \c \b RDW_UPDATENOW	Before the function returns, the specified windows will receive
/// WM_NCPAINT, WM_ERASEBKGND, as well as WM_PAINT messages.
///
/// \note Wrapper for Windows API.
//
inline bool
TWindow::RedrawWindow(TRect* update, HRGN hUpdateRgn, uint redraw)
{
  PRECONDITION(GetHandle());
  return ::RedrawWindow(GetHandle(), update, hUpdateRgn, redraw);
}

//
/// Disables or enables one or both of the scroll bar arrows on the scroll bars
/// associated with this window. sbFlags, which specifies the type of scroll bar,
/// can be one of the Scroll Bar constants (SB_CTL, SB_HORZ, SB_VERT, or SB_BOTH).
/// By default, the arrows on both the horizontal and vertical scroll bars are
/// either enabled or disabled. arrowFlags, which indicates whether the scroll bar
/// arrows are enabled or disabled, can be one of the Enable Scroll Bar constants
/// (ESB_ENABLE_BOTH, ESB_DISABLE_LTUP, ESB_DISABLE_RTDN, ESB_DISABLE_BOTH). By
/// default, the arrows on both the horizontal and vertical scroll bars are enabled.
/// \note Wrapper for Windows API.
//
inline bool
TWindow::EnableScrollBar(uint sbFlags, uint arrowFlags)
{
  PRECONDITION(GetHandle());
  return ::EnableScrollBar(GetHandle(), sbFlags, arrowFlags);
}

//
/// Displays or hides the scroll bar. bar specifies whether the bar is a control
/// itself or part of the window's nonclient area. If bar is part of the nonclient
/// area, it can be one of the SB_Xxxx scroll bar constants (specifically, SB_BOTH,
/// SB_HORZ, or SB_VERT). If it is a control, it should be SB_CTRL. If show is true,
/// the scroll bar is displayed; if false, it is hidden.
/// \note Wrapper for Windows API.
//
inline void TWindow::ShowScrollBar(int bar, bool show)
{
  PRECONDITION(GetHandle());
  ::ShowScrollBar(GetHandle(), bar, show);
}

//
/// Scrolls a window in the vertical (dx) and horizontal (dy) directions. scroll
/// indicates the area to be scrolled. If 0, the entire client area is scrolled.
/// clip specifies the clipping rectangle to be scrolled. Only the area within clip
/// is scrolled. If clip is 0, the entire window is scrolled.
/// \note Wrapper for Windows API.
//
inline void
TWindow::ScrollWindow(int dx, int dy, const TRect* scroll, const TRect* clip)
{
  PRECONDITION(GetHandle());
  ::ScrollWindow(GetHandle(), dx, dy, scroll, clip);
}

//
/// Scrolls a window in the vertical (dx) and horizontal (dy) directions. scroll
/// indicates the area to be scrolled. If 0, the entire client area is scrolled.
/// clip specifies the clipping rectangle to be scrolled. Only the area within clip
/// is scrolled. If clip is 0, the entire window is scrolled. update indicates the
/// region that will receive the boundaries of the area that becomes invalidated as
/// a result of scrolling. flags, which determines how the window's children are
/// scrolled, can be one of the following SW_Xxxx  Scroll Window constants:
/// - \c \b SW_ERASE	Erases the invalidated region after sending an erase background
/// message to the window indicated by the SW_INVALIDATE flag value.
/// - \c \b SW_INVALIDATE	Invalidates the region indicated by the hUpdate parameter.
/// - \c \b SW_SCROLLCHILDREN	Scrolls all the child window intersecting the rectangle
/// pointed to by the scroll parameter.
/// \note Wrapper for Windows API.
//
inline void
TWindow::ScrollWindowEx(
  int dx, 
  int dy,
  const TRect* scroll,
  const TRect* clip,
  HRGN hUpdateRgn,
  TRect* update,
  uint flags)
{
  PRECONDITION(GetHandle());
  ::ScrollWindowEx(GetHandle(), dx, dy, scroll, clip, hUpdateRgn, update, flags);
}

//
/// Returns the ID of the control.
/// \note Wrapper for Windows API.
//
inline int
TWindow::GetDlgCtrlID() const
{
  PRECONDITION(GetHandle());
  return ::GetDlgCtrlID(GetHandle());
}

//
/// Retrieves the handle of a control specified by childId.
/// \note Wrapper for Windows API.
//
inline HWND
TWindow::GetDlgItem(int childId) const
{
  PRECONDITION(GetHandle());
  return ::GetDlgItem(GetHandle(), childId);
}

//
/// Sets the child window with the Id (childId) in the window to the integer value
/// specified in value. If isSigned is true, the value is signed.
/// \note Wrapper for Windows API.
//
inline void
TWindow::SetDlgItemInt(int childId, uint value, bool isSigned) const
{
  PRECONDITION(GetHandle());
  ::SetDlgItemInt(GetHandle(), childId, value, isSigned);
}

//
/// Retrieves the text of a control specified by childId. text points to the text
/// buffer to receive the text. maxLen specifies the maximum length of the caption,
/// which is truncated if it exceeds this length.
/// \note Wrapper for Windows API.
//
inline int
TWindow::GetDlgItemText(int childId, LPTSTR text, int maxLen) const
{
  PRECONDITION(GetHandle());
  return ::GetDlgItemText(GetHandle(), childId, text, maxLen);
}

//
/// Sets the title or text of a control in a dialog box. childId identifes the
/// control. text points to the text buffer containing the text that is to be copied
/// into the control.
/// \note Wrapper for Windows API.
//
inline void
TWindow::SetDlgItemText(int childId, LPCTSTR text) const
{
  PRECONDITION(GetHandle());
  ::SetDlgItemText(GetHandle(), childId, text);
}

//
/// Indicates if the child button specified in the integer parameter, buttonId, is
/// checked, or if a button is grayed, checked, or neither. If the return value is
/// 0, the button is unchecked. If the return value is 1, the button is checked. If
/// the return value is 3, the button state is undetermined. This function sends a
/// BM_GETCHECK message to the specified button control.
/// \note Wrapper for Windows API.
//
inline uint
TWindow::IsDlgButtonChecked(int buttonId) const
{
  PRECONDITION(GetHandle());
  return ::IsDlgButtonChecked(GetHandle(), buttonId);
}

//
/// Returns true if the window is a child window or a descendant window of this
/// window. A window is considered a child window if it is the direct descendant of
/// a given parent window and the parent window is in a chain of windows leading
/// from the original overlapped or pop-up window down to the child window. hWnd
/// identifies the window to be tested.
/// \note Wrapper for Windows API.
//
inline bool
TWindow::IsChild(HWND hWnd) const
{
  PRECONDITION(GetHandle());
  return ::IsChild(GetHandle(), hWnd);
}

//
/// Returns either the next or the previous control in the dialog box. hWndCtrl
/// identifies the control in the dialog box where the search begins. If previous is
/// 0, GetNextDlgGroupItem searches for the next control. If previous is nonzero, it
/// searches for the previous control.
/// \note Wrapper for Windows API.
//
inline HWND
TWindow::GetNextDlgGroupItem(HWND hWndCtrl, bool previous) const
{
  PRECONDITION(GetHandle());
  return ::GetNextDlgGroupItem(GetHandle(), hWndCtrl, previous);
}

//
/// Returns the handle of the first control that lets the user press the Tab key to
/// move to the next control (that is, the first control with the WS_TABSTOP style
/// associated with it). hWndCtrl identifies the control in the dialog box where the
/// search begins. If previous is 0, GetNextDlgTabItem searches for the next
/// control. If previous is nonzero, it searches for the previous control.
/// \note Wrapper for Windows API.
//
inline HWND
TWindow::GetNextDlgTabItem(HWND hWndCtrl, bool previous) const
{
  PRECONDITION(GetHandle());
  return ::GetNextDlgTabItem(GetHandle(), hWndCtrl, previous);
}

//
/// Places a check mark in (or removes a check mark from) the button specified in
/// buttonId. If check is nonzero, the check mark is placed next to the button; if
/// 0, the check mark is removed. For buttons having three states, check can be 0
/// (clear), 1 (checked), or 2 (gray).
/// \note Wrapper for Windows API.
//
inline void
TWindow::CheckDlgButton(int buttonId, uint check)
{
  PRECONDITION(GetHandle());
  ::CheckDlgButton(GetHandle(), buttonId, check);
}

//
/// Checks the radio button specified by checkButtonId and removes the check mark
/// from the other radio buttons in the group. firstButtonId and lastButtonId
/// specify the first and last buttons, respectively, in the group.
/// \note Wrapper for Windows API.
//
inline void
TWindow::CheckRadioButton(int firstButtonId, int lastButtonId, int checkButtonId)
{
  PRECONDITION(GetHandle());
  ::CheckRadioButton(GetHandle(), firstButtonId, lastButtonId, checkButtonId);
}

//
///  Returns the handle to the menu of the indicated window. If the window has no
/// menu, the return value is 0.
/// \note Wrapper for Windows API.
//
inline HMENU
TWindow::GetMenu() const
{
  PRECONDITION(GetHandle());
  return ::GetMenu(GetHandle());
}

//
/// Returns a handle to the system menu so that an application can access the system
/// menu.
/// \note Wrapper for Windows API.
//
inline HMENU
TWindow::GetSystemMenu(bool revert) const
{
  PRECONDITION(GetHandle());
  return ::GetSystemMenu(GetHandle(), revert);
}

//
/// Returns the associated popup menu used by the window.
//
inline TPopupMenu*
TWindow::GetContextMenu() const
{
  return ContextPopupMenu;
}

//
/// Sets the specified window's menu to the menu indicated by hMenu. If hMenu is 0,
/// the window's current menu is removed. SetMenu returns 0 if the menu remains
/// unchanged; otherwise, it returns a nonzero value.
/// \note Wrapper for Windows API.
//
inline bool
TWindow::SetMenu(HMENU hMenu)
{
  PRECONDITION(GetHandle());
  return ::SetMenu(GetHandle(), hMenu);
}

//
/// Either highlights or removes highlighting from a top-level item in the menu.
/// idItem indicates the menu item to be processed. hilite (which contains a value
/// that indicates if the idItem is to be highlighted or is to have the highlight
/// removed) can be one or more of the following constants:
/// - \c \b MF_BYCOMMAND	The idItem  parameter contains the menu item's identifier.
/// - \c \b MF_BYPOSITION	The idItem  parameter contains the zero-based relative position of
/// the menu item.
/// - \c \b MF_HILITE	Highlights the menu item. If this value is not specified, highlighting
/// is removed from the item.
/// - \c \b MF_UNHILITE	Removes the menu item's highlighting.
/// If the menu is set to the specified condition, HiliteMenuItem returns true;
/// otherwise, it returns false.
///
/// \note Wrapper for Windows API.
//
inline bool
TWindow::HiliteMenuItem(HMENU hMenu, uint idItem, uint hilite)
{
  PRECONDITION(GetHandle());
  return ::HiliteMenuItem(GetHandle(), hMenu, idItem, hilite);
}

//
/// DrawMenuBar redraws the menu bar. This function should be called to redraw the
/// menu if the menu is changed after the window is created.
/// \note Wrapper for Windows API.
//
inline void
TWindow::DrawMenuBar()
{
  PRECONDITION(GetHandle());
  ::DrawMenuBar(GetHandle());
}

//
/// Gets rid of the timer and removes any WM_TIMER messages from the message queue.
/// timerId contains the ID number of the timer event to be killed.
/// \note Wrapper for Windows API.
//
inline bool
TWindow::KillTimer(UINT_PTR timerId)
{
  return ::KillTimer(GetHandle(), timerId);
}

//
/// Creates a timer object associated with this window. timerID contains the ID
/// number of the timer to be created, timeout specifies the length of time in
/// milliseconds, and proc identifies the address of the function that's to be
/// notified when the timed event occurs. If proc is 0, WM_TIMER messages are placed
/// in the queue of the application that called SetTimer for this window.
/// \note Wrapper for Windows API.
//
inline UINT_PTR
TWindow::SetTimer(UINT_PTR timerId, uint timeout, TIMERPROC proc)
{
  return ::SetTimer(GetHandle(), timerId, timeout, proc);
}

//
/// Creates a new caret for the system. HBITMAP  specifies the bitmapped caret
/// shape.
/// \note Wrapper for Windows API.
//
inline void
TWindow::CreateCaret(HBITMAP hBitmap)
{
  PRECONDITION(GetHandle());
  ::CreateCaret(GetHandle(), hBitmap, 0, 0);
}

//
/// Create a new caret for the system with the specified shape, bitmap shade, width,
/// and height. If width or height is 0, the corresponding system-defined border
/// size is used.
/// \note Wrapper for Windows API.
//
inline void
TWindow::CreateCaret(bool isGray, int width, int height)
{
  PRECONDITION(GetHandle());
  ::CreateCaret(GetHandle(), (HBITMAP)(isGray ? 1 : 0), width, height);
}

//
/// DestroyCaret first checks the ownership of the caret. If a window in the current
/// task owns the caret, DestroyCaret destroys the caret and removes it from the
/// screen.
/// \note Wrapper for Windows API.
//
inline void
TWindow::DestroyCaret()
{
  ::DestroyCaret();
}

//
/// Retrieves the caret blink rate in milliseconds.
/// \note Wrapper for Windows API.
//
inline uint
TWindow::GetCaretBlinkTime()
{
  return ::GetCaretBlinkTime();
}

//
/// Gets the position of the caret in the coordinates of the client window. point
/// refers to the structure that receives the client coordinates of the caret.
/// \note Wrapper for Windows API.
//
inline void
TWindow::GetCaretPos(TPoint& point)
{
  ::GetCaretPos(&point);
}

//
/// Removes the caret from the specified display screen. The caret is hidden only if
/// the current task's window owns the caret. Although the caret is not visible, it
/// can be displayed again using ShowCaret.
/// \note Wrapper for Windows API.
//
inline void
TWindow::HideCaret()
{
  PRECONDITION(GetHandle());
  ::HideCaret(GetHandle());
}

//
/// Sets the caret blink rate in milliseconds.
/// \note Wrapper for Windows API.
//
inline void
TWindow::SetCaretBlinkTime(uint16 milliSecs)
{
  ::SetCaretBlinkTime(milliSecs);
}

//
/// Sets the position of the caret in the coordinates of the client window. x and y
/// indicate the client coordinates of the caret.
/// \note Wrapper for Windows API.
//
inline void
TWindow::SetCaretPos(int x, int y)
{
  ::SetCaretPos(x, y);
}

//
/// Sets the position of the caret in the coordinates of the client window. pos
/// indicates the client coordinates of the caret.
/// \note Wrapper for Windows API.
//
inline void
TWindow::SetCaretPos(const TPoint& pos)
{
  ::SetCaretPos(pos.x, pos.y);
}

//
/// Displays the caret in the specified shape in the active window at the current
/// position.
/// \note Wrapper for Windows API.
//
inline void
TWindow::ShowCaret()
{
  PRECONDITION(GetHandle());
  ::ShowCaret(GetHandle());
}

//
/// Retrieves the cursor's current position (in window screen coordinates) and
/// copies the values into the structure pointed to by pos.
/// \note Wrapper for Windows API.
//
inline void
TWindow::GetCursorPos(TPoint& pos)
{
  ::GetCursorPos(&pos);
}

//
/// Sets the font that a control uses to draw text. font, which specifies the font
/// being used, is NULL if the default system font is used. If redraw is true, the
/// control redraws itself after the font is set; if false, the control does not
/// redraw itself. See the sample program, FILEBROW.CPP, for an example of how to
/// set the font for a file browser list box.
/// \note Wrapper for Windows API.
//
inline void
TWindow::SetWindowFont(HFONT font, bool redraw)
{
  PRECONDITION(GetHandle());
  SendMessage(WM_SETFONT, TParam1(font), redraw);
}

//
/// Gets the font the control uses to draw text. The return value is a handle of the
/// font the control uses. If a system default font is being used, GetWindowFont 
/// returns NULL.
/// \note Wrapper for Windows API.
//
inline HFONT
TWindow::GetWindowFont()
{
  PRECONDITION(GetHandle());
  return (HFONT)SendMessage(WM_GETFONT);
}

//
/// Registers a hotkey ID with the current application. modifiers can be a
/// combination of keys that must be pressed to activate the specified hotkey, such
/// as HOTKEYF_SHIFT, HOTKEYF_CONTROL, and HOTKEYF_ALT.
inline bool
TWindow::RegisterHotKey(int idHotKey, uint modifiers, uint virtKey)
{
  PRECONDITION(GetHandle());
  return ::RegisterHotKey(GetHandle(), idHotKey, modifiers, virtKey);
}

//
/// Unregisters a hotkey ID with the current application.
inline bool
TWindow::UnregisterHotKey(int idHotKey)
{
  PRECONDITION(GetHandle());
  return ::UnregisterHotKey(GetHandle(), idHotKey);
}

//
/// Invokes a specified help system. helpFile points to a string containing the
/// directory path and name of the help file. command, which indicates the type of
/// help requested, can be one of the Windows Help_xxxx constants such as
/// HELP_CONTEXT, HELP_HELPONHELP, HELP_INDEX, HELP_MULTIKEY, HELP_QUIT, or
/// HELP_SETINDEX. data contains keywords that indicate the help topic items.
/// For example, in the sample ObjectWindows file, HELP.CPP, WinHelp is called with
/// the arguments HELP_CONTEXT and HELP_MENUITEMA if the F1 key is pressed.
/// \code
/// void TOwlHelpWnd::CmMenuItemA()
/// { 
///    if (F1Pressed) { 
///      WinHelp(HelpFile, HELP_CONTEXT, HELP_MENUITEMA);
///      F1Pressed = false;
/// } else { 
///      MessageBox("In Menu Item A command", Title, MB_ICONINFORMATION);
///  }
/// }
/// \endcode
/// You can also include bitmaps in your Help file by referencing their file names
/// or by copying them from the Clipboard. For more information about how to create
/// Help files, see the online Help documentation.
/// \note Wrapper for Windows API.
//
inline bool
TWindow::WinHelp(LPCTSTR helpFile, uint command, uint32 data)
{
  PRECONDITION(GetHandle());
  return ::WinHelp(GetHandle(), helpFile, command, data);
}

inline int 
TWindow::MessageBox(const tstring& text, const tstring& caption, uint flags) 
{
  return MessageBox(text.c_str(), caption.length() > 0 ? caption.c_str() : 0, flags);
}

inline int 
TWindow::FormatMessageBox(const tstring& text, const tstring& caption, uint flags,  va_list argp) 
{
  return FormatMessageBox(text.c_str(), caption.length() > 0 ? caption.c_str() : 0, flags, argp);
}

//
/// Returns a handle to the task that created the specified window.
/// \note Wrapper for Windows API.
//
inline HTASK
TWindow::GetWindowTask() const
{
  PRECONDITION(GetHandle());
  return (HANDLE)::GetWindowThreadProcessId(GetHandle(), 0);
}

//
/// Calls TApplication:GetWindowPtr on the application associated with this window.
/// Then, given the handle to this window (hWnd), GetWindowPtr returns the TWindow
/// pointer associated with this window.
inline TWindow* TWindow::GetWindowPtr(HWND hWnd) const
{
  return owl::GetWindowPtr(hWnd, GetApplication());
}

//
// Inline member functions that call DefWindowProc()
//

//
/// Default message handler for WM_ACTIVATE.
inline void TWindow::EvActivate(uint /*active*/,
                                bool /*minimized*/,
                                HWND /*hWndOther*//*may be 0*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_ACTIVATEAPP.
inline void TWindow::EvActivateApp(bool /*active*/, HTASK /*hTask or threadId*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_ASKCBFORMATNAME.
inline void TWindow::EvAskCBFormatName(uint /*bufLen*/, LPTSTR /*buffer*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_CANCELMODE.
inline void TWindow::EvCancelMode()
{
  DefaultProcessing();
}

//
/// The default message handler for WM_CHANGECBCHAIN.
inline void TWindow::EvChangeCBChain(HWND /*hWndRemoved*/, HWND /*hWndNext*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_CHAR.
inline void TWindow::EvChar(uint /*key*/, uint /*repeatCount*/, uint /*flags*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_CHARTOITEM.
inline int TWindow::EvCharToItem(uint /*key*/, HWND /*hWndListBox*/, uint /*caretPos*/)
{
  return (int)DefaultProcessing();
}

//
/// The default message handler for WM_VKEYTOITEM.
inline int TWindow::EvVKeyToItem(uint /*key*/, HWND /*hWndListBox*/, uint /*caretPos*/)
{
  return (int)DefaultProcessing();
}

//
/// The default message handler for WM_COMMNOTIFY.
inline void TWindow::EvCommNotify(uint /*commId*/, uint /*status*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_COMPACTING.
inline void TWindow::EvCompacting(uint /*compactRatio*/)
{
  DefaultProcessing();
}

//
inline bool TWindow::EvCopyData(HWND /*hwnd*/, const COPYDATASTRUCT& /*dataStruct*/)
{
  return DefaultProcessing();
}


//
/// The default message handler for WM_DEADCHAR.
inline void TWindow::EvDeadChar(uint /*deadKey*/, uint /*repeatCount*/, uint /*flags*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_DESTROYCLIPBOARD.
inline void TWindow::EvDestroyClipboard()
{
  DefaultProcessing();
}

//
/// The default message handler for WM_DEVMODECHANGE.
inline void TWindow::EvDevModeChange(LPCTSTR /*device*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_DROPFILES.
inline void TWindow::EvDropFiles(TDropInfo)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_DRAWCLIPBOARD.
inline void TWindow::EvDrawClipboard()
{
  DefaultProcessing();
}

//
/// The default message handler for WM_ENABLE.
inline void TWindow::EvEnable(bool /*enabled*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_FONTCHANGE.
inline void TWindow::EvFontChange()
{
  DefaultProcessing();
}

//
/// The default message handler for WM_GETDLGCODE.
inline uint TWindow::EvGetDlgCode(const MSG*)
{
  return (uint)DefaultProcessing();
}

//
/// The default message handler for WM_GETFONT.
inline HFONT   TWindow::EvGetFont()
{
  return (HFONT)DefaultProcessing();
}

//
/// The default message handler for WM_GETMINMAXINFO.
inline void TWindow::EvGetMinMaxInfo(MINMAXINFO & /*info*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_GETTEXT.
inline void TWindow::EvGetText(uint /*buffSize*/, LPTSTR /*buff*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_GETTEXTLENGTH.
inline uint TWindow::EvGetTextLength()
{
  return (uint)DefaultProcessing();
}

//
/// The default message handler for WM_HOTKEY.
inline void TWindow::EvHotKey(int /*idHotKey*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_INPUTFOCUS.
inline void TWindow::EvInputFocus(bool /*gainingFocus*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_HSCROLLCLIPBOARD.
inline void TWindow::EvHScrollClipboard(HWND /*hWndCBViewer*/,
                                           uint /*scrollCode*/,
                                           uint /*pos*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_ICONERASEBKGND.
inline void TWindow::EvIconEraseBkgnd(HDC /*dc*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_INITMENU.
inline void TWindow::EvInitMenu(HMENU /*menu*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_KEYDOWN.
inline void TWindow::EvKeyDown(uint /*key*/, uint /*repeatCount*/, uint /*flags*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_KEYUP.
inline void TWindow::EvKeyUp(uint /*key*/, uint /*repeatCount*/, uint /*flags*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_LBUTTONDBLCLK.
inline void TWindow::EvLButtonDblClk(uint /*modKeys*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_LBUTTONUP.
inline void TWindow::EvLButtonUp(uint /*modKeys*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_MBUTTONDBLCLK.
inline void TWindow::EvMButtonDblClk(uint /*modKeys*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_MBUTTONDOWN.
inline void TWindow::EvMButtonDown(uint /*modKeys*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_MBUTTONUP.
inline void TWindow::EvMButtonUp(uint /*modKeys*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_MENUCHAR.
inline int32   TWindow::EvMenuChar(uint /*nChar*/, uint /*menuType*/, HMENU /*hMenu*/)
{
  return (uint)DefaultProcessing();
}

//
/// The default message handler for WM_MENUSELECT.
inline void TWindow::EvMenuSelect(uint /*menuItemId*/, uint /*flags*/, HMENU /*hMenu*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_MOUSEACTIVATE.
inline uint TWindow::EvMouseActivate(HWND /*hWndTopLevel*/, uint /*hitTestCode*/, TMsgId)
{
  return (uint)DefaultProcessing();
}

//
/// The default message handler for WM_MOUSEMOVE.
inline void TWindow::EvMouseMove(uint /*modKeys*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_NCACTIVATE.
inline bool TWindow::EvNCActivate(bool /*active*/)
{
  return (bool)DefaultProcessing();
}

//
/// The default message handler for WM_NCCALCSIZE.
inline uint TWindow::EvNCCalcSize(bool /*calcValidRects*/, NCCALCSIZE_PARAMS &)
{
  return (uint)DefaultProcessing();
}

//
/// The default message handler for WM_NCCREATE.
inline bool TWindow::EvNCCreate(CREATESTRUCT &)
{
  return (bool)DefaultProcessing();
}

//
/// The default message handler for WM_NCHITTEST.
inline uint TWindow::EvNCHitTest(const TPoint&)
{
  return (uint)DefaultProcessing();
}

//
/// The default message handler for WM_NCLBUTTONDBLCLK.
inline void TWindow::EvNCLButtonDblClk(uint /*hitTest*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_NCLBUTTONDOWN.
inline void TWindow::EvNCLButtonDown(uint /*hitTest*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_NCLBUTTONUP.
inline void TWindow::EvNCLButtonUp(uint /*hitTest*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_NCMBUTTONDBLCLK.
inline void TWindow::EvNCMButtonDblClk(uint /*hitTest*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_NCMBUTTONDOWN.
inline void TWindow::EvNCMButtonDown(uint /*hitTest*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_NCMBUTTONUP.
inline void TWindow::EvNCMButtonUp(uint /*hitTest*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_NCMOUSEMOVE.
inline void TWindow::EvNCMouseMove(uint /*hitTest*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_NCPAINT.
inline void TWindow::EvNCPaint(HRGN /*region*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_NCRBUTTONDBLCLK.
inline void TWindow::EvNCRButtonDblClk(uint /*hitTest*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_NCRBUTTONDOWN.
inline void TWindow::EvNCRButtonDown(uint /*hitTest*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_NCRBUTTONUP.
inline void TWindow::EvNCRButtonUp(uint /*hitTest*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_NEXTDLGCTL.
inline void TWindow::EvNextDlgCtl(TParam1 /*hctlOrDir*/, uint /*isHCtl*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_OTHERWINDOWCREATED.
inline void TWindow::EvOtherWindowCreated(HWND /*hWndOther*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_OTHERWINDOWDESTROYED.
inline void TWindow::EvOtherWindowDestroyed(HWND /*hWndOther*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_PAINTICON.
inline void TWindow::EvPaintIcon()
{
  DefaultProcessing();
}

//
/// The default message handler for WM_PAINTCLIPBOARD.
inline void TWindow::EvPaintClipboard(HWND, HANDLE /*hPaintStruct*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_PALETTECHANGED.
inline void TWindow::EvPaletteChanged(HWND /*hWndPalChg*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_PALETTEISCHANGING.
inline void TWindow::EvPaletteIsChanging(HWND /*hWndPalChg*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_PARENTNOTIFY.
inline void TWindow::EvParentNotify(uint /*event*/, TParam1, TParam2)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_POWER.
inline int  TWindow::EvPower(uint)
{
  return (int)DefaultProcessing();
}

//
/// Responds to a user-selected command from the System menu or when the user
/// selects the maximize or minimize box. Applications that modify the system menu
/// must process EvSysCommand messages. Any EvSysCommand messages not handled by the
/// application must be passed to DefaultProcessing. The parameter cmdType can be
/// one of the following system commands:
/// - \c \b SC_CLOSE	Close the window.
/// - \c \b SC_HOTKEY	Activate the specified window.
/// - \c \b SC_HSCROLL	Scroll horizontally.
/// - \c \b SC_KEYMENU	Retrieve a menu through a keystroke.
/// - \c \b SC_MAXIMIZE (or SC_ZOOM)	Maximize the window.
/// - \c \b SC_MINIMIZE (or SC_ICON)	Minimize the window.
/// - \c \b SC_MOUSEMENU	Retrieve a menu through a mouse click.
/// - \c \b SC_NEXTWINDOW	Move to the next window.
/// - \c \b SC_PREVWINDOW	Move to the previous window.
/// - \c \b SC_SCREENSAVE	Execute the specified screen saver.
/// - \c \b SC_SIZE	Size the window
/// - \c \b SC_TASKLIST	Activate the Windows Task Manager.
/// - \c \b SC_VSCROLL	Scroll vertically.
/// 
/// In the following example, EvSysCommand either processes system messages or calls
/// DefaultProcessing:
/// \code
/// void MyWindow::EvSysCommand(uint cmdType, TPoint&)
/// { 
///    switch (cmdType&  0xFFF0) { 
///       case SC_MOUSEMENU:
///       case SC_KEYMENU:
///          break;
///       default:
///          DefaultProcessing();
/// 	}
/// }
/// \endcode
//
inline void TWindow::EvSysCommand(uint /*cmdType*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_QUERYDRAGICON.
inline HANDLE  TWindow::EvQueryDragIcon()
{
  return (HANDLE)DefaultProcessing();
}

//
/// The default message handler for WM_QUERYNEWPALETTE.
inline bool TWindow::EvQueryNewPalette()
{
  return (bool)DefaultProcessing();
}

//
/// The default message handler for WM_QUERYOPEN.
inline bool TWindow::EvQueryOpen()
{
  return (bool)DefaultProcessing();
}

//
/// The default message handler for WM_QUEUESYNC.
inline void TWindow::EvQueueSync()
{
  DefaultProcessing();
}

//
/// The default message handler for WM_RENDERALLFORMATS.
inline void TWindow::EvRenderAllFormats()
{
  DefaultProcessing();
}

//
/// The default message handler for WM_RENDERFORMAT.
inline void TWindow::EvRenderFormat(uint /*dataFormat*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_RBUTTONDBLCLK.
inline void TWindow::EvRButtonDblClk(uint /*modKeys*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_RBUTTONDOWN.
inline void TWindow::EvRButtonDown(uint /*modKeys*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_RBUTTONUP.
inline void TWindow::EvRButtonUp(uint /*modKeys*/, const TPoint&)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_SETFOCUS.
inline void TWindow::EvSetFocus(HWND /*hWndLostFocus*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_SETFONT.
inline void TWindow::EvSetFont(HFONT, bool)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_SETREDRAW.
inline void TWindow::EvSetRedraw(bool)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_SETTEXT.
inline void TWindow::EvSetText(LPCTSTR)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_SHOWWINDOW.
inline void TWindow::EvShowWindow(bool /*show*/, uint /*status*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_SIZECLIPBOARD.
inline void TWindow::EvSizeClipboard(HWND /*hWndViewer*/, HANDLE /*hRect*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_SPOOLERSTATUS.
inline void TWindow::EvSpoolerStatus(uint /*jobStatus*/, uint /*jobsLeft*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_SYSCHAR.
inline void TWindow::EvSysChar(uint /*key*/, uint /*repeatCount*/, uint /*flags*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_SYSDEADCHAR.
inline void TWindow::EvSysDeadChar(uint /*key*/, uint /*repeatCount*/, uint /*flags*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_SYSKEYDOWN.
inline void TWindow::EvSysKeyDown(uint /*key*/, uint /*repeatCount*/, uint /*flags*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_SYSKEYUP.
inline void TWindow::EvSysKeyUp(uint /*key*/, uint /*repeatCount*/, uint /*flags*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_SYSTEMERROR.
inline void TWindow::EvSystemError(uint /*error*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_TIMECHANGE.
inline void TWindow::EvTimeChange()
{
  DefaultProcessing();
}

//
/// The default message handler for WM_TIMER.
inline void TWindow::EvTimer(uint /*timerId*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_WININICHANGE.
inline void TWindow::EvWinIniChange(LPCTSTR /*section*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_VSCROLLCLIPBOARD.
inline void TWindow::EvVScrollClipboard(HWND /*hWndCBViewer*/,
                                           uint /*scrollCode*/,
                                           uint /*pos*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_WINDOWPOSCHANGED.
inline void TWindow::EvWindowPosChanged(const WINDOWPOS& /*windowPos*/)
{
  DefaultProcessing();
}

//
/// The default message handler for WM_WINDOWPOSCHANGING.
inline void TWindow::EvWindowPosChanging(WINDOWPOS & /*windowPos*/)
{
  DefaultProcessing();
}

} // OWL namespace

#endif  // OWL_WINDOW_H