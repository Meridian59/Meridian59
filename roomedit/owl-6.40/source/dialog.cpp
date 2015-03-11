//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TDialog. TDialogs are the base for any type of
/// modal or modeless window that loads itself from resource.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/dialog.h>
#include <owl/applicat.h>
#include <owl/commctrl.h>
#include <owl/appdict.h>
#include <string.h>

#if defined(OWL_SUPPORT_BWCC)
#  include <owl/private/bwcc.h>
#endif

#include <owl/winres.rh> // Defines RT_DLGINIT etc.

namespace owl {

/// \class TDialog
/// Typically used to obtain information from a user, a dialog box is a window
/// inside of which other controls such as buttons and scroll bars can appear.
/// Unlike actual child windows which can only be displayed in the parent window's
/// client area, dialog boxes can be moved anywhere on the screen. TDialog objects
/// represent both modal and modeless dialog box interface elements. (A modal dialog
/// box disables operations in its parent window while it is open, and, thus, lets
/// you function in only one window "mode.")
/// 
/// A TDialog object has a corresponding resource definition that describes the
/// placement and appearance of its controls. The identifier of this resource
/// definition is supplied to the constructor of the TDialog object. A TDialog
/// object is associated with a modal interface element by calling its Execute
/// member function and with a modeless interface element by calling its Create
/// member function.
/// 
/// You can use TDialog to build an application that uses a dialog as its main
/// window by constructing your dialog as a TDialog and passing it as the client of
/// a TFrameWindow. Your code might look something like this:
/// \code
/// SetMainWindow(new TFrameWindow(0, "title" new TDialog(0, IDD_MYDIALOG)));
/// \endcode
/// TDialog is a streamable class.
/// 
/// ObjectWindows also encapsulates common dialog boxes that let the user select
/// font, file name, color, print options, and so on. TCommonDialog is the parent
/// class for this group of common dialog box classes.


OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlMsg);  // diagnostic group for message tracing
DIAG_DECLARE_GROUP(OwlWin);  // diagnostic group for windows

#define dfTmplMask    0x0f0000 // template mask
#define dfTmplDelMask 0x0a0000 // delete mask

//
// Global message id for GetWindowPtr call defined in owl.cpp
//
extern _OWLDATA(uint) GetWindowPtrMsgId;

//----------------------------------------------------------------------------

//
// Public so derived classes can create // !CQ make a static, protected member?
//

//JJH
#if defined(WINELIB)
static
#endif
_OWLDATA(TDialog*) DlgCreationWindow = 0;

DEFINE_RESPONSE_TABLE1(TDialog, TWindow)
  EV_WM_CLOSE,
  EV_WM_PAINT,
  EV_WM_CTLCOLOR,
  EV_WM_SETFONT,
  EV_DM_GETDEFID,
  EV_DM_SETDEFID,
  EV_DM_REPOSITION,
  EV_COMMAND(IDOK, CmOk),
  EV_COMMAND(IDCANCEL, CmCancel),
END_RESPONSE_TABLE;


//
/// Constructor for a TDialog object
//
/// Takes an TResId for its template name or numeric Id
//
/// Invokes a TWindow constructor, passing parent and module, and calls
/// DisableAutoCreate to prevent TDialog from being automatically created and
/// displayed along with its parent. TDialog then initializes the caption to 0 and sets
/// TDialogAttr.Name using the dialog box's integer or string resource
/// identifier which must correspond to a dialog resource definition in the resource
/// file. Note that resId may be 0, e.g. when TDialog encapsulates a common dialog.
///
/// Finally, it initializes TDialogAttr.Param to 0 and clear the flag dfIsModal, and
/// if OWL5_COMPAT is defined IsModal is set to false.
//
TDialog::TDialog(TWindow* parent, TResId resId, TModule* module)
{
  // Initialize virtual base, in case the derived-most used default ctor
  //
  Init(parent, 0, module);

  DialogAttr.Name = resId.IsString() ? strnewdup(resId) : const_cast<LPTSTR>(static_cast<LPCTSTR>(resId));
}


//
/// Takes an pointer to dialog template
//
/// Invokes a TWindow constructor, passing parent and module, and calls
/// DisableAutoCreate to prevent TDialog from being automatically created and
/// displayed along with its parent. TDialog then initializes the caption to 0 and sets
/// TDialogAttr.Name using DLGTEMPLATE*.
///
/// Finally, it initializes TDialogAttr.Param to 0 and clear the flag dfIsModal, and
/// if OWL5_COMPAT is defined IsModal is set to false.
//
TDialog::TDialog(TWindow* parent, const DLGTEMPLATE& dlgTemplate, TAutoDelete del, TModule* module)
{
  Init(parent, 0, module);

  DialogAttr.Name = (LPTSTR)&dlgTemplate;
  SetFlag(del == AutoDelete ? dfDeleteTemplate : dfNoDeleteTemplate);
}

//
/// Invokes a TWindow constructor, passing parent and module, and calls
/// DisableAutoCreate to prevent TDialog from being automatically created and
/// displayed along with its parent. TDialog then initializes the caption to 0 and sets
/// TDialogAttr.Name using DLGTEMPLATE*.
///
/// Finally, it initializes TDialogAttr.Param to 0 and clear the flag dfIsModal, and
/// if OWL5_COMPAT is defined IsModal is set to false.
//
TDialog::TDialog(HGLOBAL hTemplate, TWindow* parent, TAutoDelete del, TModule* module)
{
  PRECONDITION(hTemplate);

  Init(parent, 0, module);

  DialogAttr.Name = (LPTSTR)GlobalLock(hTemplate);
  SetFlag(del == AutoDelete ? dfDeleteHandle : dfNoDeleteHandle);
}


//
/// Common initialization in constructoe
//
void            
TDialog::Init(TWindow* parent, LPCTSTR /*title*/, TModule* module)
{
#if defined(OWL5_COMPAT)
  IsModal = false;
#endif
  // Initialize virtual base, in case the derived-most used default ctor
  //
  TWindow::Init(parent, 0, module);

  DisableAutoCreate();

  // NB! Previously (<= 6.30) TWindow::Title was here set to 0x0000FFFF as a flag.
  // This "don't-change" flag was then detected in SetCaption to disable title updates.
  // It seems the only reason for this was to avoid that the SetCaption call in SetupWindow
  // overwrote the title of the window loaded from the resource. 
  //
  // Now, the caption is simply set to 0 here. We use the flag wfFromResource to detect that
  // we need to sync the caption from the loaded resource later (see SetupWindow).

  SetFlag(wfFromResource);
  ClearFlag(wfDeleteOnClose);
  DialogAttr.Param = 0;
  DialogAttr.Name  = 0;
}

//
/// Destructor for a TDialog
//
/// If DialogAttr.Name is a string and not an integer resource identifier, this destructor
/// frees memory allocated to DialogAttr.Name, which holds the name of the dialog box.
//
TDialog::~TDialog()
{
  if(!IsFlagSet(dfTmplMask)){
    if (TResId(DialogAttr.Name).IsString())
      delete[] DialogAttr.Name;
  }
  else if(IsFlagSet(dfTmplDelMask))
  {
    if(IsFlagSet(dfDeleteHandle)) // Is DialogAttr.Name a handle to memory that should be deleted?
      ::GlobalFree(::GlobalHandle(DialogAttr.Name)); // Free it.
    else if(IsFlagSet(dfNoDeleteHandle)) // Is DialogAttr.Name a handle to memory that should not be deleted?
      ::GlobalUnlock(::GlobalHandle(DialogAttr.Name)); // Just unlock it then.
  }
  
  // If DlgCreationWindow still points to this object, zero it, as the object is no longer valid
  if (DlgCreationWindow == this)
  {
  	DlgCreationWindow = 0;
  }
}

//
/// Preprocess posted messages to provide various accelerator translations.
//
/// Overrides the virtual function defined by TWindow in order to perform
/// preprocessing of window messages. If the child window has requested keyboard
/// navigation, PreProcessMsg handles any accelerator key messages and then
/// processes any other keyboard messages.
//
bool
TDialog::PreProcessMsg(MSG& msg)
{
  // Processed any accelerators associated with this window
  //
  if (TWindow::PreProcessMsg(msg))
    return true;

  // See if we are in an mdi child, & try mdi accelerator translation if so
  //
  THandle child;
  THandle client;
  if ((child = GetParentH()) != 0 &&
      (client = ::GetParent(child)) != 0 &&
      child == (THandle)::SendMessage(client, WM_MDIGETACTIVE, 0, 0) &&
      ::TranslateMDISysAccel(client, &msg))
    return true;

  // Do dialog accelerator translation last, since it tends to eat other
  // accelerators
  //
  return ::IsDialogMessage(GetHandle(), &msg);
}

//
/// Overrides the virtual function defined in TWindow and returns the name of the
/// dialog box's default Windows class, which must be used for a modal dialog box.
/// For a modeless dialog box, GetClassName returns the name of the default TWindow.
//
TWindow::TGetClassNameReturnType
TDialog::GetClassName()
{
#if defined(OWL_SUPPORT_BWCC)
  if (GetApplication()->BWCCEnabled())
    return BORDLGCLASS;
  else
#endif
    return (TGetClassNameReturnType)WC_DIALOG;
}

//
/// Overrides the virtual function defined in TWindow. Fills WndClass with TDialog
/// registration attributes obtained from an existing TDialog window
/// 
/// If the class style is registered with CS_GLOBALCLASS, you must unregister the
/// class style. You can do this by turning off the style bit. For example:
/// \code
/// { 
///  baseclass::GetWindowClass(wndClass);
///  ....
///  WndClass.style &= ~CS_GLOBALCLASS:
///  ...
/// }
/// \endcode
//
void
TDialog::GetWindowClass(WNDCLASS& wndClass)
{
  TResId dlgClass;

#if defined(OWL_SUPPORT_BWCC)
  if (GetApplication()->BWCCEnabled()) {
    GetApplication()->GetBWCCModule()->Register(*GetModule());
    dlgClass = BORDLGCLASS;
  }
  else
#endif
    dlgClass = WC_DIALOG;

  if (!TUser::GetClassInfo(0, dlgClass, &wndClass))
    GetModule()->GetClassInfo(dlgClass, &wndClass);

  wndClass.lpszClassName   = GetClassName();
  wndClass.hInstance       = *GetModule();  // !CQ Win32 only? (B50.26561)
}

//
/// Handler for WM_SETFONT, is dispatched from DialogFunction() once during dialog
/// creation, subsequently as normal. Responds to a request to change a dialog's
/// font.
//
void
TDialog::EvSetFont(HFONT /*hFont*/, bool /*redraw*/)
{
  if (IsFlagSet(wfFullyCreated))
    DefaultProcessing();
}

//
/// Handler for WM_PAINT. Rerout this message back to the underlying dialog to
/// keep TWindow::EvPaint from eating it.
//
/// EvPaint calls TWindow's general-purpose default processing function,
/// DefaultProcessing, for Windows-supplied painting.
//
void
TDialog::EvPaint()
{
  DefaultProcessing();
}

//
/// Passes the handle to the display context for the child window, the handle to the
/// child window, and the default system colors to the parent window. The parent
/// window then uses the display-context handle given in hDC to set the text and
/// background colors of the child window.
/// 
HBRUSH
TDialog::EvCtlColor(HDC hDC, THandle hChild, uint ctlType)
{
#if defined(OWL_SUPPORT_CTL3D)
  if (GetApplication()->Ctl3dEnabled()) {
    HBRUSH hBr = GetApplication()->GetCtl3dModule()->CtlColorEx(
                 WM_CTLCOLORMSGBOX+ctlType, TParam1(hDC), TParam2(hChild));
    if (hBr)
      return hBr;
  }
#endif
  return TWindow::EvCtlColor(hDC, hChild, ctlType);
}

//
/// Return the default Id.
/// High word must be DC_HASDEFID.
//
/// Calls the DefaultProcessing() function. Returns the result.
//
uint32
TDialog::EvGetDefId()
{
  return DefaultProcessing();
}

//
/// Set the pushbutton with identifier id the default button for the dialog.
///
/// Calls the DefaultProcessing function. Returns the result (type cast to boolean).
//
bool
TDialog::EvSetDefId(int /*id*/)
{
  return (bool)DefaultProcessing();
}

//
/// Reposition the dialog so that it fits within the desktop area
//
void
TDialog::EvReposition()
{
  DefaultProcessing();
}

//
/// The default dialog function. Handles the two mesages, WM_INITDIALOG and
/// WM_SETFONT that may be passed to us without sending, or sent before we get
/// a chance to thunk DIALOG's window proc.
//
/// EvInitDialog() is called as a virtual function, while EvSetFont() is dispatched
/// to.
//
/// To process messages within the dialog function, your application must override
/// this function. DialogFunction returns true if the message is handled and false
/// if the message is not handled.
//
TDialog::TDialogProcReturnType
TDialog::DialogFunction(TMsgId msg, TParam1 param1, TParam2 param2)
{
  TRACEX(OwlMsg, 2, TMsgName(msg) << _T("(DlgFcn)=> ") << *this);

  switch (msg) {

    // 'Dispatch' WM_INITDIALOG by making a virtual call--no response table
    // used. This is not generally overriden, and only arrives once.
    //
    case WM_INITDIALOG:
      return EvInitDialog((THandle)param1);

    // Dispatch WM_SETFONT only for the first, non-sent occurance. Subsequent
    // WM_SETFONTs will be dispatched normally in TWindow.
    //
    case WM_SETFONT:
      if (!IsFlagSet(wfFullyCreated)) {
        TEventInfo  eventInfo(msg);
        if (Find(eventInfo)) {
          return Dispatch(eventInfo, param1, param2) == 0;
        }
      }
      return true;

#if defined(OWL_SUPPORT_CTL3D)
    // Catch these few messages here & forward to the ctrl3d dll if it is
    // loaded and enabled.
    //
    case WM_SETTEXT:
    case WM_NCPAINT:
    case WM_NCACTIVATE:
      if (GetApplication()->Ctl3dEnabled()) {
        SetMsgResult(GetApplication()->GetCtl3dModule()->
          DlgFramePaint(GetHandle(), msg, param1, param2));
      }
      break;
#endif
  }
  return false;  // Not handled
}

//
/// Handles messages that come from the window procedure of dialogs (WC_DIALOG's
/// wndProc). If DlgCreationWindow is not 0, then msg is destined for
/// DlgCreationWindow. When StdDlgProc first receives a message, it checks to see if
/// it needs to associate the global C++ variable DlgCreationWindow with the one
/// passed in HWND. If it needs to do that, it will then subclass the window
/// procedure. It calls the virtual DialogFunction() to handle specific messages,
/// mostly for the startup sequence.
/// Any exception within this function is suspended by TApplication::SuspendThrow.
//
INT_PTR CALLBACK
TDialog::StdDlgProc(HWND hDlg, UINT msg, WPARAM param1, LPARAM param2) throw()
{
  try
  {
    // Retrieve the TDialog pointer.
    //
    if (msg == GetWindowPtrMsgId) return false;
    TDialog* dlg = dynamic_cast<TDialog*>(owl::GetWindowPtr(hDlg));
    if (DlgCreationWindow && !dlg && msg != WM_CANCELMODE && msg != WM_ENABLE)
    {
      dlg = DlgCreationWindow;
      dlg->SetHandle(hDlg);
      dlg->SubclassWindowFunction();
      DlgCreationWindow = 0;
    }
    if (!dlg) return false;

    // Dispatch the message to the DialogFunction virtual function.
    //
    return dlg->DialogFunction(msg, param1, param2);
  }
  catch (const TXBase& x) 
  {
    TRACEX(OwlWin, 0, _T("TDialog::StdDlgProc: Suspending unhandled TXBase exception for message: ") << msg);
    OWLGetAppDictionary().GetApplication()->SuspendThrow(x);
  }
  catch (const TXEndSession& x)
  {
    TRACEX(OwlWin, 0, _T("TDialog::StdDlgProc: Suspending unhandled TXEndSession for message: ") << msg);
    OWLGetAppDictionary().GetApplication()->SuspendThrow(x);
  }
  catch (const std::exception& x) 
  {
    TRACEX(OwlWin, 0, _T("TDialog::StdDlgProc: Suspending unhandled std::exception for message: ") << msg);
    OWLGetAppDictionary().GetApplication()->SuspendThrow(x);
  }
  catch (...) 
  {
    TRACEX(OwlWin, 0, _T("TDialog::StdDlgProc: Suspending unhandled unknown exception for message: ") << msg);
    OWLGetAppDictionary().GetApplication()->SuspendThrow();
  }
  
  return false; // If we get here, an error occurred.
}

//
//
//
static bool
RegisterFails(TWindow* win, void*)
{
  return !win->Register();
}

/// Register all the dialog's child objects (for custom control support)
//
void
TDialog::RegisterChildObjects()
{
  if (FirstThat(RegisterFails))
    TXWindow::Raise(this, IDS_CHILDREGISTERFAIL);
}

//
/// Creates a modeless dialog box interface element associated with the TDialog
/// object. Registers all the dialog's child windows for custom control support.
/// Calls PerformCreate to perform the actual creation of the dialog box. Create
/// returns true if successful. If unsuccessful, Create throws a TXInvalidWindow
/// exception.
//
bool
TDialog::Create()
{
  PRECONDITION(GetHandle() == 0);

#if defined(OWL5_COMPAT)
  IsModal = false;
#endif
  ClearFlag(dfIsModal);

  if (!Register())
    TXWindow::Raise(this, IDS_CLASSREGISTERFAIL);

  DlgCreationWindow = this;
  LoadAcceleratorTable();

  // Register all the dialog's child objects (for custom control support)
  //
  RegisterChildObjects();

  CHECK(!GetHandle()); // Ensure handle is NULL in case of exceptions.

#if OWL_STRICT

  // In strict API mode, the argument til PerformCreate is no longer used.
  // Also PerformCreate now returns the handle instead of calling SetHandle, hence
  // we must do the latter here. 
  //
  SetHandle(PerformCreate());

#else

  // In non-strict API mode (compatibility mode) we simulate the old behaviour and 
  // construct the argument to PerformCreate as before. In non-strict mode 
  // PerformCreate has the responsibility of calling SetHandle.
  //
  PerformCreate(reinterpret_cast<int>(DialogAttr.Name));

#endif

  GetApplication()->ResumeThrow();
  WARNX(OwlWin, !GetHandle(), 0, _T("CreateDialog[Indirect]Param failed: ") <<
    _T("Class: \"") << tstring(GetClassName()).c_str() << _T("\", ") <<
    _T("Title: \"") << tstring(Title).c_str() << _T("\", ") <<
    _T("Parent: ") << (Parent ? uint(Parent->GetHandle()) : 0) << _T(", ") <<
    _T("Module: \"") << GetModule()->GetModuleFileName());
  if (!GetHandle())
    TXWindow::Raise(this, IDS_WINDOWCREATEFAIL);

  GetHWndState();
  return true;
}

//
/// Called from Create to perform the final step in creating an Windows interface
/// element to be associated with a TWindow. Creates the dialgo using either a
/// resource id or dialog template.
//
// TODO: Store the DLGTEMPLATE in a separate parameter instead of multiplexing it
// into DialogAttr.Name and make DialogAttr.Name a TResId.
//
TWindow::TPerformCreateReturnType
TDialog::PerformCreate(int arg)
{
  PRECONDITIONX(!(OWL_STRICT && arg), _T("The deprecated argument to PerformCreate is disallowed.")); InUse(arg);
  PRECONDITION(GetModule());
  typedef const DLGTEMPLATE* TTemplate;
  THandle h = !IsFlagSet(dfTmplMask) ? 
    ::CreateDialogParam (
      *GetModule(), 
      OWL_STRICT ? DialogAttr.Name : reinterpret_cast<LPCTSTR>(arg),
      GetParentO() ? GetParentO()->GetHandle() : 0,
      reinterpret_cast<DLGPROC>(StdDlgProc),
      DialogAttr.Param
    ) :
    ::CreateDialogIndirectParam(
      *GetModule(), 
      OWL_STRICT ? reinterpret_cast<TTemplate>(DialogAttr.Name) : reinterpret_cast<TTemplate>(arg),
      GetParentO() ? GetParentO()->GetHandle() : 0,
      reinterpret_cast<DLGPROC>(StdDlgProc),
      DialogAttr.Param
    );
  OWL_SET_OR_RETURN_HANDLE(h);
}


//
/// Creates and executes a modal dialog box interface element associated with the
/// TDialog object. If the element is successfully associated, Execute does not
/// return until the TDialog is closed.
/// 
/// Execute performs the following tasks:
/// - 1.	Registers this dialog's window class and all of the dialog's child windows.
/// - 2.	Calls DoExecute() to execute the dialog box.
/// - 3.	Checks for exceptions and throws a TXWindow exception if an error occurs.
/// 
/// Execute returns an integer value that indicates how the user closed the modal
/// dialog box. If the dialog box cannot be created, Execute returns -1.
//
int
TDialog::Execute()
{
  PRECONDITION(GetHandle() == 0);

#if defined(OWL5_COMPAT)
  IsModal = true;
#endif
  SetFlag(dfIsModal);

  if (!Register())
    TXWindow::Raise(this, IDS_CLASSREGISTERFAIL);

  DlgCreationWindow = this;

  // Register all the dialog's child objects (for custom control support)
  //
  RegisterChildObjects();

  int retValue = DoExecute();
  GetApplication()->ResumeThrow();

  // DoExecute returns -1 if it could not create the dialog box
  //
  if (retValue == -1)
    TXWindow::Raise(this, IDS_WINDOWEXECUTEFAIL);

  return retValue;
}

//
//
/// Overriden TWindow virtual to modaly execute this dialog & return the result.
///
/// DoExecute is called by Execute() to perform the actual execution of a modal dialog
/// box. If enabled Emulation of ModalDialog it calls TWindow::DoExecute().
//
int
TDialog::DoExecute()
{
  if (IsFlagSet(dfModalWindow))
    return TWindow::DoExecute();

  if (!IsFlagSet(dfTmplMask))
    return ::DialogBoxParam(
      *GetModule(), 
      DialogAttr.Name,
      GetParentO() ? GetParentO()->GetHandle() : 0,
      StdDlgProc,
      DialogAttr.Param);
  else
    return ::DialogBoxIndirectParam(
      *GetModule(), 
      reinterpret_cast<LPCDLGTEMPLATE>(DialogAttr.Name),
      GetParentO() ? GetParentO()->GetHandle() : 0,
      StdDlgProc,
      DialogAttr.Param);
}


//
/// Performs resource-directed initialization of controls. Uses the DLGINIT
/// resource with an id matching this dialog's as structured message packets
/// to send to controls.
//
/// Initializes the dialog box controls with the contents of DLGINIT, the dialog
/// box resource identifier, which describes the appearance and location of the
/// controls (buttons, group boxes, and so on). Returns true if successful,
/// or no DLGINIT resource was found; otherwise returns false.
//
bool
TDialog::PerformDlgInit()
{
  if (!DialogAttr.Name) return true; // Move along; nothing to see here.

  // Try to retrieve a pointer to RT_DLGINIT resource data.
  //
  char* res = 0;
  if (IsFlagSet(dfTmplMask))
    res = reinterpret_cast<char*>(DialogAttr.Name); // Should point to DLGINIT resource data.
  else
  {
    HRSRC hRes = GetModule()->FindResourceEx(DialogAttr.Name, RT_DLGINIT, GetApplication()->GetLangId());
    if (!hRes) hRes = GetModule()->FindResource(DialogAttr.Name, RT_DLGINIT);
    if (hRes)
    {
      HGLOBAL hDat = GetModule()->LoadResource(hRes);
      if (!hDat) return false;
      res = static_cast<char*>(::LockResource(hDat));
      if (!res) return false;
    }
  }
  if (!res) return true; // No DLGINIT, but that's fine.

  // Now pick up message packets from the DLGINIT resource and send them.
  // A null-terminator indicates the end.
  //
  while (*res) 
  {
    // Decode the message packet header (idc, msg, len).
    //
    WORD idc = *reinterpret_cast<WORD*>(res); res += sizeof(idc);
    WORD msg = *reinterpret_cast<WORD*>(res); res += sizeof(msg);
    DWORD len = *reinterpret_cast<DWORD*>(res); res += sizeof(len);
    void* data = res;

    // Send the message.
    // Handle old MFC custom messages and the OWL DLGINIT messages specially.
    // These messages carry ANSI strings, so we convert to wide string as needed.
    // We check for the null-terminator to catch mal-formed strings. 
    //
    switch (msg)
    {
    case 0x0401: // LB_ADDSTRING alias (data is ANSI text)
      {
        _USES_CONVERSION;
        if (res[len - 1] != '\0') return false;
        LPCTSTR text = _A2W(static_cast<LPCSTR>(data));
        LRESULT r = SendDlgItemMessage(idc, LB_ADDSTRING, 0, TParam2(text));
        if (r == LB_ERR || r == LB_ERRSPACE) return false;
        break; 
      }
    case 0x0403: // CB_ADDSTRING alias (data is ANSI text)
      {
        _USES_CONVERSION;
        if (res[len - 1] != '\0') return false;
        LPCTSTR text = _A2W(static_cast<LPCSTR>(data));
        LRESULT r = SendDlgItemMessage(idc, CB_ADDSTRING, 0, TParam2(text));
        if (r == CB_ERR || r == CB_ERRSPACE) return false;
        break; 
      }
    case 0x1234: // CBEM_INSERTSTRING alias (data is ANSI text)
      {
        _USES_CONVERSION;
        if (res[len - 1] != '\0') return false;
        COMBOBOXEXITEM item;
        item.mask = CBEIF_TEXT;
        item.iItem = -1;
        item.pszText = _A2W(static_cast<LPSTR>(data));
        LRESULT r = SendDlgItemMessage(idc, CBEM_INSERTITEM, 0, TParam2(&item));
        if (r == -1) return false;
        break; 
      }
    case OWL_DLGINIT_COMMAND: // TODO: Move the code in this block to its own function.
      {
        // There should be no data with this message.
        //
        if (len != 0)
        {
          TRACE(_T("Corrupt OWL_DLGINIT_COMMAND in DLGINIT. Expected data size 0, but size was ") << len << _T(".\n"));
          return false;
        }

        // There should be an initialisation command in the static text of the given control.
        // We load the static text of the given control, which should be in the format "{<command>}".
        // We then execute "<command>". Currently, the only command supported is "LoadString <id>", 
        // which specifies that we should load the title of the control from resource string <id>.
        //
        using namespace std;
        IFDIAG(static const tchar error[] = _T("Either correct the command in the control title, or remove the OWL_DLGINIT_COMMAND message in DLGINIT.");)
        tstring title = GetDlgItemText(idc);
        tistringstream in(title);
        in.setf(ios::skipws);
        tchar openingBrace;
        tstring cmd;
        in >> openingBrace >> cmd; 
        if (!in.good() || openingBrace != _T('{'))
        {
          TRACE(_T("PerformDlgInit expected {<command>} in the title of control #") << idc 
            << _T(", but found \"") << title << _T("\".\n") << error);
          return false;
        }
        if (cmd == _T("LoadString"))
        {
          int id;
          tchar closingBrace;
          in >> id >> closingBrace;
          if (!in.good() || closingBrace != _T('}'))
          {
            TRACE(_T("PerformDlgInit found a syntax error in the title of control #") << idc 
              << _T(": \"") << title << _T("\".\n") << error);
            return false;
          }
          SetDlgItemText(idc, id);
        }
        else
        {
          TRACE(_T("PerformDlgInit didn't recognize the command in the title of control #") << idc
            << _T(": \"") << cmd << _T("\".\n") << error);
          return false;
        }
        break;
      }
    default: // Send any other message type and hope it goes well (there is no consistent way to detect failure).
      {
        WARNX(OwlMsg, true, 1, _T("Default DLGINIT item handler sent message #") << msg << _T(" to control #") << idc << _T(" with no error handling."));
        SendDlgItemMessage(idc, msg, 0, TParam2(data));
        break;
      }
    }

    // Move the pointer to the next packet.
    //
    res = static_cast<char*>(data) + len;
  }
  GetApplication()->ResumeThrow();
  return true;
}

//
/// Virtual handler for WM_INITDIALOG message, called from DialogFunction() just
/// before the dialog box is displayed. Performs any setup
/// required for the dialog box or its controls.
//
bool
TDialog::EvInitDialog(THandle /*hFocus*/)
{
  GetHWndState();
  if (DialogAttr.Name) // Resource to initialize?
  {
    bool r = PerformDlgInit();
    if (!r) TXWindow::Raise(this, IDS_DLGINITFAIL);
  }
  PerformSetupAndTransfer();
  return true;  // have Windows set focus to "hFocus"
}

//
/// Overrides the virtual function defined in TWindow. Sets up the dialog box by
/// calling SetCaption and TWindow::SetupWindow.
/// 
/// Calling SetCaption here allows us to override the dialog caption
/// (specified in the dialog resource) by calling SetCaption prior to this point.
/// If the caption is not yet initialised, and the dialog is created from a 
/// resource, we sync the caption with the window text title.
//
void
TDialog::SetupWindow()
{
#if defined(OWL_SUPPORT_CTL3D)
  // If this app is using Ctl3d, tell it to explicitly subclass this dialog's
  // controls (CTL3D_ALL). Its better to do this here than enable
  // autosubclassing since that requires a CBT hook set all the time which
  // slows the app considerably.
  //
  if (GetApplication()->Ctl3dEnabled())
    GetApplication()->GetCtl3dModule()->SubclassDlg(*this, 0xFFFF);
#endif

  TWindow::SetupWindow();
  if (!GetCaption() && IsFlagSet(wfFromResource))
    GetWindowTextTitle();
  else
    SetCaption(GetCaption());
}

//
/// Overrides the virtual function defined by TWindow and conditionally shuts down
/// the dialog box. If the dialog box is modeless, it calls TWindow::CloseWindow(), passing retValue. 
///
/// If the dialog box is modal, it calls CanClose(). If CanClose() returns true,
/// CloseWindow() calls TransferData() to transfer dialog box data, and shuts down, passing retValue.
/// \note The default value of retValue is IDCANCEL.
//
void
TDialog::CloseWindow(int retValue)
{
  if (IsFlagSet(dfIsModal)) 
  {
    try
    {
      if (CanClose())
      {
        TransferData(tdGetData);
        Destroy(retValue);
      }
    }
    catch (...)
    {
      // Jogy - an exception throw in either CacClose() or TransferData() can leave the dialog and it's message loop
      // hanging around, leaving the application in an unusable state. To avoid this, call unconditionally Destroy,
      // and then rethrow the exception to a higher-level handler
      Destroy(retValue);
      throw;
    }
  }
  else
    TWindow::CloseWindow(retValue);
}

//
/// Destroys the interface element associated with the TDialog object. If the
/// element is a modeless dialog box or enabled Emulation of ModlaDialog , Destroy
/// calls TWindow::Destroy. If the element is a modal dialog box, Destroy calls
/// EnableAutoCreate on all child windows. Then Destroy calls the Windows function
/// ::EndDialog, passing retValue as the value returned to indicate the result of
/// the dialog's execution. The default retValue is IDCANCEL.
//
void
TDialog::Destroy(int retValue)
{
  if (!IsFlagSet(dfModalWindow) && IsFlagSet(dfIsModal) && GetHandle()) {
    ForEach(DoEnableAutoCreate);
    ::EndDialog(GetHandle(), retValue);
  }
  else 
    TWindow::Destroy(retValue);
}


// Section is empty. It used to be occupied by a SetCaption override, that is no 
// longer needed.


//
/// Responds to a click on the dialog box's OK button with the identifier IDOK.
/// Calls CloseWindow, passing IDOK.
//
void
TDialog::CmOk()
{
  CloseWindow(IDOK);

  if (!IsFlagSet(dfIsModal)  && !GetHandle() && IsFlagSet(wfDeleteOnClose))
    GetApplication()->Condemn(this);
}

//
/// Responds to an incoming notification message from a button with
/// an Id equal to IDCANCEL. Unconditionally destroys the window.
//
void
TDialog::CmCancel()
{
  EvClose();
}

//
/// Message response function for WM_CLOSE by unconditionally closing
/// the window.
//
void
TDialog::EvClose()
{
  Destroy(IDCANCEL);

  if (!IsFlagSet(dfIsModal) && !GetHandle() && IsFlagSet(wfDeleteOnClose))
    GetApplication()->Condemn(this);
}


//
/// Constructor. Initializes the base class.
//
TControlEnabler::TControlEnabler(uint id, HWND hWndReceiver)
:
  TCommandEnabler(id, hWndReceiver)
{
}

//
/// Enables/disables the control.
//
void
TControlEnabler::Enable(bool enable)
{
  if(ToBool(::IsWindowEnabled(GetReceiver())) != enable)
    ::EnableWindow(GetReceiver(), enable);
  TCommandEnabler::Enable(enable);
}

//
/// Set the text of the control
//
void
TControlEnabler::SetText(LPCTSTR text)
{
  TTmpBuffer<tchar> buffer(MAX_PATH);
  ::GetWindowText(GetReceiver(),buffer,MAX_PATH);
  if(_tcscmp(buffer, text) != 0)
    ::SetWindowText(GetReceiver(), text);
}

//
/// Use with only buttons!
//
void
TControlEnabler::SetCheck(int check)
{
  if(::SendMessage(GetReceiver(), BM_GETCHECK,0,0) != check)
    ::SendMessage(GetReceiver(), BM_SETCHECK, check, 0);
}


//
/// If idleCount is 0, iterates over each child control and routes the command
/// enabler. Does not rely on an OWL interface object to work. Returns false.
//
bool
TDialog::IdleAction(long idleCount)
{
  TWindow::IdleAction(idleCount);

  if (idleCount == 0) {
    HWND child = GetWindow(GW_CHILD);
    while (::IsWindow(child)) {
      int id = ::GetWindowLong(child, GWL_ID);
      TControlEnabler ce(id, child);
      RouteCommandEnable(*this, ce);
      child = ::GetWindow(child, GW_HWNDNEXT);
    }
  }
  return false;
}


//
/// Sets focus within a dialog to a specific control.
//
void
TDialog::SetControlFocus(HWND hwnd)
{
  CHECK(hwnd);
  SendMessage(WM_NEXTDLGCTL, TParam1(hwnd), 1);
}


//
/// Sets the focus to either the previous control with WS_TABSTOP, or the next
/// control with WS_TABSTOP.
//
void
TDialog::SetControlFocus(THow how)
{
  SendMessage(WM_NEXTDLGCTL, (how == Next) ? 1 : 0, 0);
}



IMPLEMENT_STREAMABLE1(TDialog, TWindow);

#if !defined(BI_NO_OBJ_STREAMING)

//
// Reads an instance of TDialog from the passed ipstream
//
void*
TDialog::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TWindow*)GetObject(), is);

  is >> (TResId&)GetObject()->DialogAttr.Name;

  bool isModal;
  is >> isModal;
  isModal ? GetObject()->SetFlag(dfIsModal) : GetObject()->ClearFlag(dfIsModal);
  
  return GetObject();
}

//
// Writes the TDialog to the passed opstream
//
void
TDialog::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TWindow*)GetObject(), os);

  os << TResId(GetObject()->DialogAttr.Name);
  os << (bool)GetObject()->IsFlagSet(dfIsModal);
}
#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
