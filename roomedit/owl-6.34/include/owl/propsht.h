//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of classes encapsulating PropertySheets and PropertyPages
//----------------------------------------------------------------------------

#if !defined(OWL_PROPSHT_H)
#define OWL_PROPSHT_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>
#include <owl/dialog.h>
#include <owl/commctrl.h>


namespace owl {

// Documented but missing from system headers!
//
#if !defined(PSH_MULTILINETABS)
# define PSH_MULTILINETABS 0x0010
#endif

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

class _OWLCLASS TIcon;
class _OWLCLASS TPropertySheet;
class _OWLCLASS TPropertyPage;
class _OWLCLASS TTabControl;
class _OWLCLASS TTabKeyDown;

/// \addtogroup commctrl
/// @{
//
// Prototypes of callbacks used to iterate through PropertyPages
//
typedef void (*TActionPageFunc)(TPropertyPage* pPage, void* param);
typedef bool (*TCondPageFunc)(TPropertyPage* pPage, void* param);



//----------------------------------------------------------------------------
// TPropertyPage
//----------------------------------------------------------------------------

//
/// \class TPropertyPage
// ~~~~~ ~~~~~~~~~~~~~
/// The 'TPropertyPage' object represents a dialog box found within a property
/// sheet. Each 'page' contains controls for setting a group of related
/// properties. Each page has a tab that the user can select to bring the page
/// to the foreground of the property sheet.
//
class _OWLCLASS TPropertyPage: public TDialog {
  public:
    TPropertyPage(TPropertySheet* parent, TResId resId,
                  LPCTSTR title = 0, TResId iconRes = 0,
                  TModule* module = 0);
    TPropertyPage(TPropertySheet* parent, TResId resId,
                  const tstring& title, TResId iconRes = 0,
                  TModule* module = 0);
    TPropertyPage(TPropertySheet* parent, const PROPSHEETPAGE& pgInfo,
                  TModule* module = 0);
   ~TPropertyPage();

    // Set attributes of the page
    //
    void            SetIcon(const TIcon&);
    void            SetIcon(TResId iconResId);
    void            SetTitle(LPCTSTR);
    void            SetTitle(const tstring& s) {SetTitle(s.c_str());}
    void            SetTitle(int txtResId);
    void            SetFlags(uint32 flags);

    // Retrieve Sheet object [i.e. Parent of page]
    //
    TPropertySheet* GetSheet() const;

    // Operator to return HPROSHEETPAGE handle
    //
    operator        HPROPSHEETPAGE() const;

    // Retrieve information about this page
    //
    void            CopyPageInfo(PROPSHEETPAGE& pgInfo) const;
    PROPSHEETPAGE&  GetPropSheetPage();

    // Create/Destroy the HPROPSHEETPAGE handle that uniquely identifies this
    // page.
    //
    HPROPSHEETPAGE  CreatePropertyPage();
    bool            DestroyPropertyPage();

    // Override this to process messages within the dialog function
    // Return true if message handled, false if not.
    //
    virtual TDialogProcReturnType DialogFunction(TMsgId, TParam1, TParam2);

    // Overriden to patch PSN_FIRST-PSN_LAST notifications
    //
    virtual TResult EvNotify(uint id, TNotify & notifyInfo);

    // Overriden virtuals of TWindow/TDialog
    //
    bool            Create();

    // This is the callback invoked when the a property sheet page is created
    // and destroyed
    //
    static UINT CALLBACK
    PropCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);

    // This is the callback invoked when notifications are sent to the pages
    //
    static INT_PTR CALLBACK
    PropDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

  protected:

    // 
    // Handle Escape from Multiline Edit control
    void            EvClose();

    // Virtual methods to handle the Sheet notifications. Some of these
    // methods will most likely be overriden by TPropertyPage-derived classes
    //
    virtual   int   SetActive(TNotify&);    ///< PSN_SETACTIVE
    virtual   bool  KillActive(TNotify&);   ///< PSN_KILLACTIVE
    virtual   int   Apply(TNotify&);        ///< PSN_APPLY
    virtual   void  Reset(TNotify&);        ///< PSN_RESET
    virtual   void  Help(TNotify&);         ///< PSN_HELP
    virtual   int   WizBack(TNotify&);      ///< PSN_WIZBACK
    virtual   int   WizNext(TNotify&);      ///< PSN_WIZNEXT
    virtual   bool  WizFinish(TNotify&);    ///< PSN_WIZFINISH
    virtual   bool  QueryCancel(TNotify&);  ///< PSN_QUERYCANCEL

    /// Following structure holds information about this dialog when it is
    /// inserted into a PropertySheet
    //
    PROPSHEETPAGE   PageInfo;

    /// Handle of this property page
    //
    HPROPSHEETPAGE  HPropPage;


  private:
    // Hidden to prevent accidental copying or assignment
    //
    TPropertyPage(const TPropertyPage&);
    TPropertyPage& operator =(const TPropertyPage&);

    // Initialization shared by constructors (not yet implemented)
    //
    //void Init(TResId resId, LPCTSTR title, TResId iconRes, uint32 flags);

    /// Method that ties the C++ object presenting the page with the underlying
    /// 'HWND' handle.
    //
    static void     InitHandle(THandle pageHandle, LPPROPSHEETPAGE pageInfo);

    /// Iterator callback allowing each page to register its children
    //
    static void     RegisterPageChildObjects(TPropertyPage* page, void*);

//copied from old unix owl (JAM 4-16-01)
// CC 5.1  added def(UNIX) on the next line for CmOk and CmCancel to be defined
#if (defined(UNIX) && !defined(WINELIB))

    // The following methods are only valid when ObjectWindows is providing
    // the underlying implementation of PropertySheet/Page. These methods are
    // protected to support derived classes. However, your derived classes
    // should be implemented to rely on these methods only when the Common
    // Control library is unavailable and ObjectWindows is providing the
    // underlying implementation of PropertySheet/PropertyPages.
    //

    // Caption of dialog as stored in resource
    //
    TAPointer<tchar> ResCaption;

  public:
    void            SetModified(bool modified);
    bool            IsModified() const;

  protected:
    void            SetupWindow();

    // Child notifications
    //
    void            CmOk();      // IDOK
    void            CmCancel();  // IDCANCEL

// !BB    bool            PreProcessMsg(MSG& msg);

  private:
    bool            Modified;
#endif

    friend class _OWLCLASS TPropertySheet;
    DECLARE_RESPONSE_TABLE(TPropertyPage);
/// DECLARE_STREAMABLE(_OWLCLASS, owl::TPropertyPage, 1);
};

//----------------------------------------------------------------------------
// TPropertySheet
//----------------------------------------------------------------------------
class _OWLCLASS TPshNotify : public PSHNOTIFY {
  public:
    TPshNotify(HWND hwnd, uint id, uint code, LPARAM lp);
    operator  NMHDR&() { return hdr; }
};

//
/// \class TPropertySheet
// ~~~~~ ~~~~~~~~~~~~~~
/// TPropertySheet encapsulates a window which contains one or more overlapping
/// child windows knowns as property pages. The sheet provides a container
/// which display pages allowing the user to view and edit related properties
/// grouped in individual pages.
//
class _OWLCLASS TPropertySheet : public TWindow {
  public:
    TPropertySheet(TWindow* parent,
                   LPCTSTR   title,
                   uint     startPage = 0,
                   bool     isWizard = false,
                   uint32   flags = PSH_DEFAULT,
                   TModule* module = 0);
    TPropertySheet(TWindow* parent,
                   const tstring& title,
                   uint startPage = 0,
                   bool isWizard = false,
                   uint32 flags = PSH_DEFAULT,
                   TModule* module = 0);
// !BB  // !BB Nice, if you can pull it
// !BB  TPropertySheet(TWindow* parent, const PROPSHEETHEADER& propHdr,
// !BB                 TModule* module = 0);
   ~TPropertySheet();

    // Override virtual functions defined by class TWindow
    //
    void            SetupWindow();
    void            CleanupWindow();

    void            SetCaption(LPCTSTR title);
    void            SetCaption(const tstring& title) {SetCaption(title.c_str());}
    int             Execute();
    bool            Create();
    int             Run(bool modal);

    // Manipulate (add, remove, select) pages of the property sheet
    //
    void            AddPage(TPropertyPage&);
    bool            SelectPage(TPropertyPage&);
    bool            SelectPage(int pgIndex);
    bool            SelectPage(TResId pgRes);
    void            RemovePage(TPropertyPage&);
    void            RemovePage(int pgIndex);
    int             GetPageCount() const;

    // Inform sheet of changes in state of page
    //
    void            PageChanged(const TPropertyPage&);
    void            PageUnchanged(TPropertyPage&);

    // Have sheet simulate user actions
    //
    void            PressButton(int button);
    bool            Apply();

    // Query the pages of the sheet
    //
    int             QuerySiblings(TParam1, TParam2);

    // Inform sheet of special requirements
    //
    void            RebootSystem();
    void            RestartWindows();

    // Update sheet attributes: title, buttons, ...
    //
    void            SetTitle(LPCTSTR text, uint32 style = PSH_PROPTITLE);
    void            SetTitle(const tstring& text, uint32 style = PSH_PROPTITLE) {SetTitle(text.c_str(), style);}
    void            SetFinishText(LPCTSTR text);
    void            SetFinishText(const tstring& text) {SetFinishText(text.c_str());}
    void            SetWizButtons(uint32 flags);
    void            CancelToClose();

    // Retrieve handle to underlying tab control or current page
    //
    HWND            GetTabControl() const;
    HWND            GetCurrentPage() const;

    // Allow processing of dialog messages
    //
    bool            IsDialogMessage(MSG & msg);

    // Retrieve the structure describing the sheet
    //
    PROPSHEETHEADER& GetPropSheetHeader();

    // Helper routines to query/iterate about/through pages
    //
    void            ForEachPage(TActionPageFunc action, void* paramList = 0);
    TPropertyPage*  FirstPageThat(TCondPageFunc cond, void* paramList = 0);

    // Tell OWL to subclass the sheet after it's created
    //
    void            EnableSubclass(bool subclass);

    // Set timer - for IdleAction() processing
    //
    bool            EnableTimer();

  protected:
    bool            SubClassSheet;  ///< Should we subclass the sheet (OS only)
    bool            WantTimer;      ///< Flags whether to start a timer
    uint            TimerID;        ///< Timer identifier

    // Holds information necessary to create the property sheet
    //
    PROPSHEETHEADER HeaderInfo;

    // Flags if sheet is a wizard
    //
    bool            IsWizard;

    bool            PreProcessMsg(MSG& msg);

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TPropertySheet(const TPropertySheet&);
    TPropertySheet& operator =(const TPropertySheet&);

    // Helper routine to retrieve handle of pages
    //
    bool            GetPageHandles();

    // Method that ties the C++ object presenting the sheet
    // with the underlying 'HWND' handle.
    //
    void            InitHandle(THandle sheetHandle);

    void             EvTimer (uint timerId);

    DECLARE_RESPONSE_TABLE(TPropertySheet);

    friend class _OWLCLASS TPropertyPage;
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//
inline
TPshNotify::TPshNotify(HWND hwnd, uint id, uint code, LPARAM lp)
{
  hdr.hwndFrom = hwnd;
  hdr.idFrom = id;
  hdr.code = code;
  lParam = lp;
}

//
/// Returns the HPROPSHEETPAGE handle representing an unlying PropertyPage.
/// \note This method is only functional when the Common Control library
///       provides the underlying implementation of PropertyPages.
///
/// \note The 'CreatePropertyPage' method also returns the HPROPSHEETPAGE.
///       However, it will attempt to create the page if the latter had not been
///       created beforehand. The HPROPSHEETPAGE operator simply returns the
///       page handle without attempting to create the page.
//
inline TPropertyPage::operator HPROPSHEETPAGE() const
{
  PRECONDITION(HPropPage);
  return HPropPage;
}

//
/// Return structure which describes the characteristics/attributes of page.
//
inline PROPSHEETPAGE&  
TPropertyPage::GetPropSheetPage()
{
  return PageInfo;  
}

//
/// Return pointer to parent/sheet object
//
inline TPropertySheet*
TPropertyPage::GetSheet() const
{
  CHECK(GetParentO());
  CHECK(TYPESAFE_DOWNCAST(GetParentO(), TPropertySheet));
  return TYPESAFE_DOWNCAST(GetParentO(), TPropertySheet);
}


/// Return structure which describes the characteristics/attributes of sheet.
//
inline PROPSHEETHEADER&
TPropertySheet::GetPropSheetHeader()
{
  return HeaderInfo;  
}

//
//
inline void
TPropertySheet::EnableSubclass(bool subclass)
{
  // Method must be invoked before the sheet is created
  //
  PRECONDITION(GetHandle() == 0);
  SubClassSheet = subclass;
}



} // OWL namespace



#endif  // OWL_PROPSHT_H
