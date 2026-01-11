//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Bidus Yura, All Rights Reserved
//
/// \file
/// Definition of class TRichEditView
/// Based on Russell Morris class.
//----------------------------------------------------------------------------

#if !defined(OWL_RICHEDV_H)
#define OWL_RICHEDV_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/docmanag.h>
#include <owl/richedit.h>

namespace owl {

#include <owl/preclass.h>

/// \addtogroup newctrl
/// @{
/// \class TRichEditView
// ~~~~~ ~~~~~~~~~~~~~
class _OWLCLASS TRichEditView : public TRichEdit, public TView {
  public:
    TRichEditView(TDocument& doc, TWindow* parent = 0);
   ~TRichEditView() override;

    static LPCTSTR StaticName();  // put in resource

    // Overridden virtuals from TView
    //
    auto GetViewName() -> LPCTSTR override;
    auto GetWindow() -> TWindow* override;
    auto SetDocTitle(LPCTSTR docname, int index) -> bool override;
    using TView::SetDocTitle; ///< String-aware overload

    // Overridden virtuals from TWindow
    //
    auto Create() -> bool override;
    auto CanClose() -> bool override;

  protected:
    virtual bool LoadData();
    void SetupWindow() override;

    // Doc/View functions
    //
    bool   VnCommit(bool force);
    bool   VnRevert(bool clear);
    bool   VnIsWindow(HWND hWnd);
    bool   VnIsDirty() ;
    bool   VnDocClosed(int omode);


    // Menu response functions
    //
    void  CmFormatFont();
    void  CmFormatColor();
    void  CmFormatBkColor();

    void  CmParagraphLeft();
    void  CmParagraphCenter();
    void  CmParagraphRight();
    void  CmParagraphBullet();

    void  CmFormatBold();
    void  CmFormatItalic();
    void  CmFormatUnderline();
    void  CmFormatStrikeout();

    void  CeFormatFont(TCommandEnabler&);
    void  CeFormatColor(TCommandEnabler&);
    void  CeFormatBkColor(TCommandEnabler&);
    void  CeFormatBold(TCommandEnabler&);
    void  CeFormatItalic(TCommandEnabler&);
    void  CeFormatUnderline(TCommandEnabler&);
    void  CeFormatStrikeout(TCommandEnabler&);

    void  CeParagraphLeft(TCommandEnabler&);
    void  CeParagraphCenter(TCommandEnabler&);
    void  CeParagraphRight(TCommandEnabler&);
    void  CeParagraphBullet(TCommandEnabler&);

    // Overridden TEditFile functions
    //
    auto CanClear() -> bool override;

    // Override TEditFile open/save functions to let doc manager handle
    // all that stuff...
    //
    //void   CmFileOpen()   { GetApplication()->GetDocManager()->CmFileOpen();  }
    void   CmFileSave()   { GetApplication()->GetDocManager()->CmFileSave();  }
    void   CmFileSaveAs() { GetApplication()->GetDocManager()->CmFileSaveAs();}
    void   CmFileClose()   { GetApplication()->GetDocManager()->CmFileClose(); }
    //void   CmFileNew()     { GetApplication()->GetDocManager()->CmFileNew();   }


  DECLARE_RESPONSE_TABLE(TRichEditView);
  DECLARE_STREAMABLE_OWL(TRichEditView,1);
};

DECLARE_STREAMABLE_INLINES(owl::TRichEditView);

/// @}

#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementation
//

//
inline LPCTSTR TRichEditView::StaticName() {
  return _T("RichEdit View");
}  // put in resource

//
inline LPCTSTR TRichEditView::GetViewName() {
  return StaticName();
}

//
inline TWindow* TRichEditView::GetWindow() {
  return static_cast<TWindow*>(this);
}

//
inline bool TRichEditView::SetDocTitle(LPCTSTR docname, int index) {
  return TRichEdit::SetDocTitle(docname, index);
}

//
inline bool TRichEditView::CanClose() {
  return TRichEdit::CanClose() &&
         (Doc->NextView(this) ||
          Doc->NextView(nullptr) != this ||
          Doc->CanClose());
}


//
inline bool TRichEditView::VnIsDirty() {
  return TRichEdit::IsModified();
}


} // OWL namespace

#endif  // OWL_EDITVIEW_H
