//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of classes TView & TWindowView
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/docmanag.h>
#include <owl/appdict.h>
#include <owl/bardescr.h>
#include <owl/docview.rh>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlDocView);        // General Doc/View diagnostic group



const uint MinUniqueViewId = 0x8000;
uint TView::NextViewId = MinUniqueViewId;

//
/// Constructs a TView object of the document associated with the view. Sets the
/// private data member ViewId to NextViewId. Calls TDocument's private member
/// function AttachView to attach the view to the associated document.
//
TView::TView(TDocument& doc)
:
  Tag(nullptr),
  ViewMenu(nullptr),
  ViewBar(0)
{
  ViewId = NextViewId;
  doc.AttachView(*this);
}

//
/// Frees a TView object and calls TDocument's private member function DetachView to
/// detach the view from the associated document.
//
TView::~TView()
{
  delete ViewMenu;
  delete ViewBar;

  const auto ok = Doc != nullptr;
  WARN(!ok, _T("TView::~TView: Terminating due to failed precondition."));
  if (!ok) std::terminate();

  if (Doc->DetachView(*this)) {
    delete Doc;
  }
}

//
// Detach the view from the current document and attach it to another.
// (Added by Vidar Hasfjord, 2007-08-27.)
//
void
TView::SetDocument(TDocument& new_doc)
{
  if (&new_doc == Doc) return; // Guard against reassignment.

  // Detach and register whether the document should be deleted.
  // The actual deletion is postponed to later to avoid any side-effects
  // from the deletion that may affect the validity of the new document.

  TDocument* delete_doc = 0;
  if (Doc && Doc->DetachView(*this))
    delete_doc = Doc;

  // Update the new document and existing views as if this was a new view.

  new_doc.AttachView(*this);
  CHECK(Doc == &new_doc);
  Doc->ReindexFrames();

  // Deletion of the old doc should be safe now.

  if (delete_doc)
    delete delete_doc;
}

//
//
//
void
TView::SetViewMenu(TMenuDescr* menu)
{
  delete ViewMenu;
  ViewMenu = menu;
  TDocTemplate* tpl = Doc->GetTemplate();

  //if (tpl && ViewMenu && *ViewMenu->GetModule() == *tpl->GetModule())
  // must check also if template is static becouse as module = 0;
  if (tpl && ViewMenu && !tpl->IsStatic() && *ViewMenu->GetModule() == *tpl->GetModule())
    ViewMenu->SetModule(tpl->GetModule());// force same module alias as template
}
//
/// Sets the menu descriptor for this view. This can be any existing TMenuDescr
/// object. If no descriptor exists, ViewMenu is 0.
//
void
TView::SetViewBar(TBarDescr* bar)
{
  delete ViewBar;
  ViewBar = bar;
}

static const tchar* const TView_PropertyNames[] = {
  _T("View Class"),      // ViewClass
  _T("View Name"),       // ViewName
};

static const int TView_PropertyFlags[] = {
  pfGetText|pfConstant,  // ViewClass
  pfGetText|pfConstant,  // ViewName
};

//
/// Returns the text name of the property given the index value.
//
const tchar*
TView::PropertyName(int index)
{
  if (index <= PrevProperty) {
    TRACEX(OwlDocView, 0, _T("PropertyName(): index <= PrevProperty!"));
    return nullptr;
  }
  else if (index < NextProperty)
    return TView_PropertyNames[index-PrevProperty-1];
  else {
    TRACEX(OwlDocView, 0, _T("PropertyName(): index >= NextProperty!"));
    return nullptr;
  }
}

//
/// Returns the attributes of a specified property given the index of the property
/// whose attributes you want to retrieve.
//
int
TView::PropertyFlags(int index)
{
  if (index <= PrevProperty) {
    TRACEX(OwlDocView, 0, _T("PropertyFlags(): index <= PrevProperty!"));
    return 0;
  }
  else if (index < NextProperty)
    return TView_PropertyFlags[index-PrevProperty-1];
  else {
    TRACEX(OwlDocView, 0, _T("PropertyFlags(): index >= NextProperty!"));
    return 0;
  }
}

//
/// Gets the property index, given the property name (name). Returns 0 if the name
/// is not found.
//
int
TView::FindProperty(LPCTSTR name)
{
  PRECONDITION(name != nullptr);
  int i;
  for (i=0; i < NextProperty-PrevProperty-1; i++)
    if (_tcscmp(TView_PropertyNames[i], name) == 0)
      return i+PrevProperty+1;

  TRACEX(OwlDocView, 0, _T("FindProperty: Index of [") << name << _T("] not found") );
  return 0;
}

//
/// Retrieves the property identified by the given index.
/// If the requested property is text, then `dest` should point to a text buffer, and `textlen`
/// should specify the maximum number of characters the buffer can hold, excluding the terminating
/// null-character, i.e. the buffer must have room for (`textlen` + 1) characters.
///
/// If the requested property is numerical, then it may be requested either as text or in its
/// binary form. To request the property as text, pass a text buffer as described above. To request
/// the property in binary form, `dest` should point to storage of sufficent size, and `textlen`
/// should be zero.
///
/// Non-text properties without textual representation, e.g. file handles, may only be requested
/// in binary form, i.e. `dest` must point to sufficient storage, and `textlen` must be zero.
///
/// \return If the parameter `textlen` is non-zero, which means that the property is requested in
/// string form, the function returns the length of the string, i.e. the character count excluding
/// the terminating null-character. If the parameter `textlen` is zero, which means that property
/// is requested in binary form, the return value is the size of the data in bytes.
///
/// If the property is text, and `textlen` is zero, the function fails and returns 0. The function
/// also fails and returns 0 if `textlen` is non-zero and the property requested can not be
/// expressed as text. It also returns 0 if the property is not defined.
///
/// \sa TView::TViewProp
//
int
TView::GetProperty(int index, void * dest, int textlen)
{
  LPCTSTR src;
  switch (index) {

    case ViewClass:{
        _USES_CONVERSION;
        src = _A2W(_OBJ_FULLTYPENAME(this));
      }
      break;

    case ViewName:
      src = GetViewName();
      break;

    default:
      TRACEX(OwlDocView, 0, _T("GetProperty(): ") \
            _T("invalid property [") << index << _T("] specified!") );
      return 0;
  }

  if (!textlen) {
    TRACEX(OwlDocView, 0, _T("GetProperty(): 0-Length buffer specified!"));
    return 0;
  }

  int srclen = src ? static_cast<int>(::_tcslen(src)) : 0;
  if (textlen > srclen)
    textlen = srclen;
  if (textlen)
    memcpy(dest, src, textlen*sizeof(tchar));
  *((tchar *)dest + textlen) = 0;
  return srclen;
}

//
/// Increments an internal count used by the Doc/View subsystem to identify each
/// view.
//
void
TView::BumpNextViewId()
{
  if (++NextViewId < MinUniqueViewId)
    NextViewId = MinUniqueViewId;
}

IMPLEMENT_ABSTRACT_STREAMABLE(TView);

#if OWL_PERSISTENT_STREAMS

//
//
//
void*
TView::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  TView* o = GetObject();
  bool hasViewMenu = is.readByte();
  if (hasViewMenu) {
    o->ViewMenu = new TMenuDescr;
    is >> *o->ViewMenu;
  }
  else
    o->ViewMenu = 0;
  o->ViewBar = 0;
  is >> o->ViewId;
  is >> o->Doc;
  is >> o->NextView;
  return o;
}

//
//
//
void
TView::Streamer::Write(opstream& os) const
{
  TView* o = GetObject();
  os.writeByte(uint8(o->ViewMenu ? 1 : 0));
  if (o->ViewMenu)
    os << *o->ViewMenu;
  os << o->ViewId;
  os << o->Doc;
  os << o->NextView;
}
#endif
//----------------------------------------------------------------------------
// TWindowView Implementation
//

DEFINE_RESPONSE_TABLE1(TWindowView, TWindow)
  EV_VN_ISWINDOW,
END_RESPONSE_TABLE;

//
/// Constructs a TWindowView interface object associated with the window view. Sets
/// ViewId to NextViewId. Calls the associated document's AttachView() function (a
/// private TDocument function) to attach the view to the document.
//
TWindowView::TWindowView(TDocument& doc, TWindow* parent)
:
  TWindow(parent, nullptr, doc.GetDocManager().GetApplication()),
  TView(doc)
{
}

IMPLEMENT_STREAMABLE2(TWindowView, TWindow, TView);

#if OWL_PERSISTENT_STREAMS
//
//
//
void*
TWindowView::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TWindow*)GetObject(), is);
  ReadBaseObject((TView*)GetObject(), is);
  return GetObject();
}

//
//
//
void
TWindowView::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TWindow*)GetObject(), os);
  WriteBaseObject((TView*)GetObject(), os);
}
#endif
//----------------------------------------------------------------------------
// TDialogView Implementation
//

DEFINE_RESPONSE_TABLE1(TDialogView, TDialog)
  EV_VN_ISWINDOW,
END_RESPONSE_TABLE;

//
//
//
TDialogView::TDialogView(TDocument& doc, TWindow* parent, TResId resId, TModule* module)
:
  TDialog(parent, resId, module),
  TView(doc)
{
}
//
TDialogView::TDialogView(TDocument& doc, TWindow* parent, const DLGTEMPLATE& dlgTemplate,
                         TAutoDelete del, TModule* module)
:
  TDialog(parent, dlgTemplate, del, module),
  TView(doc)
{
}
//
TDialogView::TDialogView(TDocument& doc, TWindow* parent, TModule* module,
                         HGLOBAL hTemplate, TAutoDelete del)
:
  TDialog(hTemplate, parent, del, module),
  TView(doc)
{
}

IMPLEMENT_STREAMABLE2(TDialogView, TDialog, TView);

#if OWL_PERSISTENT_STREAMS
//
//
//
void*
TDialogView::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TDialog*)GetObject(), is);
  ReadBaseObject((TView*)GetObject(), is);
  return GetObject();
}

//
//
//
void
TDialogView::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TDialog*)GetObject(), os);
  WriteBaseObject((TView*)GetObject(), os);
}

#endif

//----------------------------------------------------------------------------

} // OWL namespace
/* ========================================================================== */

