//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of TDialog class and TDialogAttr struct
//----------------------------------------------------------------------------

#if !defined(OWL_DIALOG_H)
#define OWL_DIALOG_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/window.h>
#include <owl/resource.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup dlg
/// @{
//

/// \struct TDialogAttr
// ~~~~~~ ~~~~~~~~~~~
/// Holds a TDialog object's creation attributes, which include the style,
/// appearance, size, and types of controls associated with the dialog box.
/// TDialogAttr contains two data members: Name (the resource id) and Param. These
/// members contain user-defined data used for dialog box creation.
//
struct TDialogAttr {
/// Name holds the identifier, which can be either a string or an integer resource
/// identifier, of the dialog resource.
  LPTSTR Name;
  
/// Param is used to pass initialization data to the dialog box when it is
/// constructed. You can assign a value to this field in a derived class's
/// constructor. Although any Param-type information passed to the dialog box can be
/// saved as a data member, Param is especially useful if you want to create a
/// dialog box that's implemented by non-ObjectWindows code.
/// 
/// After Param is accepted it is then available in the message response functions
/// (for example, EvInitDialog()) associated with WM_INITDIALOG.
  LPARAM Param;
};
/// @}
//
/// \addtogroup enabler
/// @{

/// \class TControlEnabler
// ~~~~~ ~~~~~~~~~~~~~~~
/// A TCommandEnabler for child controls in a dialog
//
class _OWLCLASS TControlEnabler : public TCommandEnabler {
  public:
    TControlEnabler(uint id, HWND hWndReceiver = 0);

    // Override virtuals
    //
    void Enable(bool enable = true);
    void SetText(LPCTSTR text);

    using TCommandEnabler::SetText; // string-aware override

    // Use this method with only buttons!
    //
    void SetCheck(int check);
};

/// @}

/// \addtogroup dlg
/// @{


//
/// \class TDialog
// ~~~~~ ~~~~~~~
class _OWLCLASS TDialog : virtual public TWindow {
  public:
    // standard constructor
    TDialog(TWindow* parent, TResId resId, TModule* module = 0);
    // construct from pointer to template 
    explicit TDialog(TWindow* parent, const DLGTEMPLATE& dlgTemplate, TAutoDelete = AutoDelete, TModule* module = 0);
    // Not usual,to avoid ambiquaity, construct from handle to template
    explicit TDialog(HGLOBAL hTemplate, TWindow* parent, TAutoDelete = AutoDelete, TModule* module=0);

   ~TDialog();

#if OWL_STRICT
    typedef INT_PTR TDialogProcReturnType; // Windows API conformant (DialogProc signature)
#else
    typedef bool TDialogProcReturnType; // OWL 5 conformant
#endif

    //
    /// Override this to process messages within the dialog function.
    /// For most messages, 1 should be returned if the message is handled, and 0 if it is not.
    /// Some special messages require a message-specific return value other than this:
    /// WM_CHARTOITEM, WM_COMPAREITEM, WM_CTL*, WM_INITDIALOG, WM_QUERYDRAGICON and WM_VKEYTOITEM. 
    /// See the Windows API documentation for details.
    ///
    /// Note that in OWL5_COMPAT mode the return type is not compatible with the special messages.
    /// If you need to handle these messages, then compile in normal mode (non-OWL5_COMPAT).
    //
    virtual TDialogProcReturnType DialogFunction(TMsgId, TParam1, TParam2);

    /// Virtual handler for WM_INITDIALOG message, called from DialogFunction()
    //
    virtual bool    EvInitDialog(HWND hWndFocus);

    /// Handler for WM_SETFONT, is dispatched from DialogFunction() once
    /// during dialog creation, subsequently as normal.
    //
    void            EvSetFont(HFONT hFont, bool redraw);

    // Create a modeless dialog box, and perform actual create call
    //
    virtual bool    Create();

#if defined(OWL5_COMPAT)
    // backward compatibility. not use
    virtual HWND    DoCreate();
#endif
    // Create a modal dialog box, and perform actual modal execute call
    //
    virtual int     Execute();
    virtual int     DoExecute();

    // Override virtual functions defined by class TWindow
    //
    bool            PreProcessMsg(MSG& msg);
    void            CloseWindow(int retValue = IDCANCEL);
    void            Destroy(int retValue = IDCANCEL);

    TDialogAttr&    GetDialogAttr();
    void            SetDialogAttr(TDialogAttr attr);

    bool            GetIsModal() const;
    void            SetIsModal(bool ismodal);
    
    // enable emulation of Modal Dialog by BeginModal/EndModal
    void            EnableModal(bool enable=true); 

    // Handy dialog functions
    //
    uint            GetDefaultId() const;
    void            SetDefaultId(uint id);
    void            MapDialogRect(TRect& rect);
    void            Reposition();

    /// Enumeration to be used when invoking the 'SetControlFocus' method.
    /// It specifies whether focus should be set to the next or previous
    /// sibling of the current control with focus.
    //
    enum THow {
      Next,         ///< Set focus to the next sibling
      Previous      ///< Set focus to the previous sibling
    };
    void            SetControlFocus(HWND hwnd);
    void            SetControlFocus(THow how);

    //
    /// Sets the dialog procedure message result (DWLP_MSGRESULT) and returns true.
    //
    bool SetMsgResult(TResult result);

    // Message response functions
    //
    void            EvClose();
    void            EvPaint();
    HBRUSH          EvCtlColor(HDC, HWND hWndChild, uint ctlType);
    uint32          EvGetDefId();
    bool            EvSetDefId(int id);
    void            EvReposition();

    // Child notifications
    //
    void            CmOk();      // IDOK
    void            CmCancel();  // IDCANCEL

#if defined(OWL5_COMPAT)
    /// Sends the passed message to the dialog's control which has id DlgItemId
    /// Obsolete- use TWindow::SendDlgItemMessage()
    //
    TResult SendDlgItemMsg(int childId, TMsgId, TParam1 = 0, TParam2 = 0);
#endif

    /// Callback procs for hooking TDialog to native window
    //
    static INT_PTR CALLBACK StdDlgProc(HWND, UINT, WPARAM, LPARAM) throw(); // nothrow

    /// Handle enabling and disabling of child controls
    //
    bool            IdleAction(long idleCount);

  protected:
    // Override virtual functions defined by class TWindow
    //
    virtual TPerformCreateReturnType PerformCreate(int deprecated_argument = 0);
    virtual void SetupWindow();
    virtual TGetClassNameReturnType GetClassName();
    virtual void GetWindowClass(WNDCLASS& wndClass);

    /// Registers WINDOW class(es) of Children
    //
    void            RegisterChildObjects();

    /// Common initialization in constructor
    void            Init(TWindow* parent, LPCTSTR title, TModule* module);

    /// Initialize dialog controls with contents of DLGINIT resources, if any.
    //
    bool            PerformDlgInit();

  public_data:
    union {
      TDialogAttr  Attr;        ///< Old name
      TDialogAttr  DialogAttr;  ///< New name
    };

#if defined(OWL5_COMPAT)
/// IsModal is true if the dialog box is modal and false if it is modeless.
    bool          IsModal;     ///< Don't use
#endif

  protected_data:
    enum{ 
      // using TWindows::Flags
      //wfInsertAtEdge  = 0x008000 // (Decoration) Window to be inserted against frame's edge
      dfResId           = 0x000000L, ///< Dialog created from ResId default
      dfNoDeleteHandle  = 0x010000L, ///< Dialog created from HGLOBAL no delete
      dfDeleteHandle    = 0x020000L, ///< Dialog created from HGLOBAL delete
      dfNoDeleteTemplate= 0x040000L, ///< Dialog created from LPDIALOGTEMPLATE no delete
      dfDeleteTemplate  = 0x080000L, ///< Dialog created from LPDIALOGTEMPLATE delete
      dfIsModal         = 0x100000L, ///< Dialog Is Modal
      dfModalWindow     = 0x200000L, ///< Use BeginModal() EndModal()  functionality
    };

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TDialog(const TDialog&);
    TDialog& operator =(const TDialog&);

  DECLARE_RESPONSE_TABLE(TDialog);
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TDialog, 1);
  DECLARE_STREAMABLE_OWL(TDialog, 1);
};
/// @}

// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TDialog );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//

#if defined(OWL5_COMPAT)

//
/// Do not use DoCreate. It is included if OWL5_COMPAT is defined for backward
/// compatibility. It calls PerformCreate() and returns Handle.
//
inline TWindow::THandle 
TDialog::DoCreate()
{
#if OWL_STRICT
  return PerformCreate();
#else
  PerformCreate(reinterpret_cast<int>(DialogAttr.Name));
  return GetHandle();
#endif
}

//
/// Sends a window control message, identified by Msg, to the dialog box's control
/// identified by its supplied ID, ChildID. WParam and LParam become parameters in
/// the control message. SendDlgItemMsg returns the value returned by the control,
/// or 0 if the control ID is invalid. This function is obsolete - use
/// TWindow::SendDlgItemMessage().
//
inline TResult
TDialog::SendDlgItemMsg(int ChildId, TMsgId msg, TParam1 p1, TParam2 p2) 
{
  return SendDlgItemMessage(ChildId, msg, p1, p2);
}

#endif

//
/// Set the result of the message for the dialog's WndProc.
//
inline bool
TDialog::SetMsgResult(TResult result)
{
  PRECONDITION(GetHandle());
  SetWindowLongPtr(DWLP_MSGRESULT, result);
  return true;
}

//
/// Return the ID of the default pushbutton.
//
inline uint
TDialog::GetDefaultId() const
{
  PRECONDITION(GetHandle());
  return LoUint16(CONST_CAST(TDialog*,this)->SendMessage(DM_GETDEFID));
}

//
/// Return the attributes data member of TDialog.
//
inline TDialogAttr&
TDialog::GetDialogAttr()
{
  return DialogAttr;
}

//
/// Set the attributes of the dialog.
//
inline void
TDialog::SetDialogAttr(TDialogAttr attr)
{
  DialogAttr = attr;
}

//
/// Return true if the dialog is modal.
//
inline bool
TDialog::GetIsModal() const
{
  return CONST_CAST(TDialog*,this)->IsFlagSet(dfIsModal);//IsModal -> old flag don't use;
}

//
/// Set the modal data member of TDialog.
//
inline void
TDialog::SetIsModal(bool ismodal)
{
#if defined(OWL5_COMPAT)
  IsModal = ismodal;// old flag
#endif
  ismodal ? SetFlag(dfIsModal) : ClearFlag(dfIsModal);
}

//
/// Enables emulation of executing ModalDialog box by calling
/// BeginModal()/EndModal()
//
inline void            
TDialog::EnableModal(bool enable)
{
  enable ? SetFlag(dfModalWindow) : ClearFlag(dfModalWindow);   
}

//
/// Set the id of the default push button.
//
inline void
TDialog::SetDefaultId(uint id)
{
  PRECONDITION(GetHandle());
  SendMessage(DM_SETDEFID, id, 0);
}

//
/// Convert the dialog units into screen coordinates.
//
inline void
TDialog::MapDialogRect(TRect& rect)
{
  PRECONDITION(GetHandle());
  ::MapDialogRect(GetHandle(), &rect);
}

//
/// Reposition the dialog to ensure it is fully visible.
//
inline void
TDialog::Reposition()
{
  PRECONDITION(GetHandle());
  SendMessage(DM_REPOSITION);
}


} // OWL namespace

#endif  // OWL_DIALOG_H
