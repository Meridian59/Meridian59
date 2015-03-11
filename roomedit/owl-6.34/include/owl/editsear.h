//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TEditSearch, an edit control that responds to Find,
/// Replace and FindNext menu commands.
//----------------------------------------------------------------------------

#if !defined(OWL_EDITSEAR_H)
#define OWL_EDITSEAR_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/edit.h>
#include <owl/findrepl.h>
#include <owl/editsear.rh>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup ctrl
/// @{

/// \class TEditSearch
// ~~~~~ ~~~~~~~~~~~
/// TEditSearch is an edit control that responds to Find, Replace, and FindNext menu
/// commands. This class is streamable.
//
class _OWLCLASS TEditSearch : public TEdit {
  public:
    TEditSearch(TWindow*        parent = 0,
                int             id = 0,
                LPCTSTR         text = 0,
                int x = 0, int y = 0, int w = 0, int h = 0,
                TModule*        module = 0);

    TEditSearch(
      TWindow* parent,
      int id,
      const tstring& text,
      int x = 0, int y = 0, int w = 0, int h = 0,
      TModule* = 0);

    TEditSearch(TWindow*   parent,
                int        resourceId,
                TModule&   module);

   ~TEditSearch();

    void          SetupWindow();
    void          DoSearch();

    TFindReplaceDialog::TData& GetSearchData();
    void          SetSearchData(const TFindReplaceDialog::TData& searchdata);

    TFindReplaceDialog* GetSearchDialog();
    void          SetSearchDialog(TFindReplaceDialog* searchdialog);

    uint          GetSearchCmd();
    void          SetSearchCmd(uint searchcmd);

  protected: // !CQ these command handlers were public

    // Menu command handlers
    //
    void          CmEditFind();               ///< CM_EDITFIND
    void          CmEditReplace();            ///< CM_EDITREPLACE
    void          CeEditFindReplace(TCommandEnabler& ce);
    void          CmEditFindNext();           ///< CM_EDITFINDNEXT
    void          CeEditFindNext(TCommandEnabler& ce);

    TResult       EvFindMsg(TParam1, TParam2);  ///< Registered commdlg message

  public_data:
/// The SearchData structure defines the search text string, the replacement text
/// string, and the size of the text buffer.
    TFindReplaceDialog::TData SearchData;
    	
/// Contains find or replace dialog-box information (such as the text to find and
/// replace) and check box settings.
    TFindReplaceDialog*       SearchDialog;  
    
/// Contains the search command identifier that opened the dialog box if one is open.
    uint                      SearchCmd;     

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TEditSearch(const TEditSearch&);
    TEditSearch& operator=(const TEditSearch&);

  DECLARE_RESPONSE_TABLE(TEditSearch);
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TEditSearch, 1);
  DECLARE_STREAMABLE_OWL(TEditSearch, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TEditSearch );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

/// This constructor, which aliases an Edit control created from a
/// dialog resource is 'unconventional' in that it expects a TModule reference
/// instead of the traditional 'TModule* = 0'. This is, however, to avoid
/// ambiguities between the two forms of constructor. Since it is traditionally
/// created as a child of a TDialog-derived class, you can simply use the module of
/// the      parent object.
///     For example,
/// \code
/// TMyDialog::TMyDialog(....)
/// { 
///   edit = new TEditSearch(this, ID_EDIT1, *GetModule());
/// }
/// \endcode
//
inline
TEditSearch::TEditSearch(TWindow* parent, int resourceId, TModule& module)
            :TEdit(parent, resourceId, 0, &module),SearchData(FR_DOWN)
{
  SearchDialog = 0;
  SearchCmd = 0;
}

//
/// Returns the search data used for the common dialog.
//
inline TFindReplaceDialog::TData& TEditSearch::GetSearchData() {
  return SearchData;
}

//
/// Uses new search data.
//
inline void TEditSearch::SetSearchData(const TFindReplaceDialog::TData& searchdata) {
  SearchData = searchdata;
}

//
/// Returns the common dialog pointer.
//
inline TFindReplaceDialog* TEditSearch::GetSearchDialog() {
  return SearchDialog;
}

//
/// Uses new common dialog pointer.
//
inline void TEditSearch::SetSearchDialog(TFindReplaceDialog* searchdialog) {
  SearchDialog = searchdialog;
}

//
/// Returns the user selected command that brought up the search dialog.
//
inline uint TEditSearch::GetSearchCmd() {
  return SearchCmd;
}

//
/// Remembers the command the user selected to bring up the search dialog.
//
inline void TEditSearch::SetSearchCmd(uint searchcmd) {
  SearchCmd = searchcmd;
}

} // OWL namespace


#endif  // OWL_EDITSEAR_H

