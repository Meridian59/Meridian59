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
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/docmanag.h>
#include <owl/richedit.h>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup newctrl
/// @{
/// \class TRichEditView
// ~~~~~ ~~~~~~~~~~~~~
class _OWLCLASS TRichEditView : public TRichEdit, public TView {
  public:
    TRichEditView(TDocument& doc, TWindow* parent = 0);
   ~TRichEditView();

    static LPCTSTR StaticName();  // put in resource

    // Overridden virtuals from TView
    //
    LPCTSTR    GetViewName();
    TWindow*  GetWindow();
    bool       SetDocTitle(LPCTSTR docname, int index);
    using TView::SetDocTitle; ///< String-aware overload

    // Overridden virtuals from TWindow
    //
    bool   Create();
    bool   CanClose();

  protected:
    virtual bool LoadData();
    virtual void SetupWindow();

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
    bool   CanClear();

    // Override TEditFile open/save functions to let doc manager handle
    // all that stuff...
    //
    //void   CmFileOpen()   { GetApplication()->GetDocManager()->CmFileOpen();  }
    void   CmFileSave()   { GetApplication()->GetDocManager()->CmFileSave();  }
    void   CmFileSaveAs() { GetApplication()->GetDocManager()->CmFileSaveAs();}
    void   CmFileClose()   { GetApplication()->GetDocManager()->CmFileClose(); }
    //void   CmFileNew()     { GetApplication()->GetDocManager()->CmFileNew();   }


  DECLARE_RESPONSE_TABLE(TRichEditView);
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TRichEditView,1);
  DECLARE_STREAMABLE_OWL(TRichEditView,1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TRichEditView );

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
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
  return (TWindow*)this;
}

//
inline bool TRichEditView::SetDocTitle(LPCTSTR docname, int index) {
  return TRichEdit::SetDocTitle(docname, index);
}

//
inline bool TRichEditView::CanClose() {
  return TRichEdit::CanClose() &&
         (Doc->NextView(this) ||
          Doc->NextView(0) != this ||
          Doc->CanClose());
}


//
inline bool TRichEditView::VnIsDirty() {
  return TRichEdit::IsModified();
}


} // OWL namespace

#endif  // OWL_EDITVIEW_H
