//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TStatic.  This defines the basic behavior of static
/// text controls
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/static.h>
#include <owl/applicat.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

#if defined(OWL_SUPPORT_BWCC)
#  include <owl/private/bwcc.h>
#endif

namespace owl {

OWL_DIAGINFO;

DEFINE_RESPONSE_TABLE1(TStatic,TControl)
  EV_WM_SIZE,
END_RESPONSE_TABLE;

//
/// Constructs a static control object with the supplied parent window (parent),
/// control ID (Id), text (title), position (x, y) relative to the origin of the
/// parent window's client area, width (w), height (h), and default text length
/// (textLimit) of zero. By default, the static control is visible upon creation and
/// has left-justified text. (Set to WS_CHILD | WS_VISIBLE | WS_GROUP | SS_LEFT.)
/// Invokes a TControl constructor. You can change the default style of the static
/// control via the control object's constructor.
//
TStatic::TStatic(TWindow* parent, int id, LPCTSTR title, int x, int y, int w, int h,
  uint textLimit, TModule* module)
: TControl(parent, id, title, x, y, w, h, module),
  TextLimit(textLimit),
  TransferBufferFieldType(tbftCharArray)
{
  Attr.Style = (Attr.Style | SS_LEFT) & ~WS_TABSTOP;
}

//
/// String-aware overload
//
TStatic::TStatic(TWindow* parent, int id, const tstring& title, int x, int y, int w, int h,
  uint textLimit, TModule* module)
: TControl(parent, id, title, x, y, w, h, module),
  TextLimit(textLimit),
  TransferBufferFieldType(tbftCharArray)
{
  Attr.Style = (Attr.Style | SS_LEFT) & ~WS_TABSTOP;
}

//
/// Constructs a TStatic object to be associated with a static control interface
/// control of a TDialog object. Invokes the TControl constructor with similar
/// parameters, then sets  TextLimit to textLimit. Disables the data transfer
/// mechanism by calling DisableTransfer. The resourceId parameter must correspond
/// to a static control resource that you define.
//
TStatic::TStatic(TWindow* parent, int resourceId, uint textLimit, TModule* module)
: TControl(parent, resourceId, module),
  TextLimit(textLimit),
  TransferBufferFieldType(tbftCharArray)
{
  DisableTransfer();
}

//
/// Overload for static controls loaded from resource
/// Allows the title specified in the resource data to be overridden.
//
TStatic::TStatic(TWindow* parent, int resourceId, const tstring& title, uint textLimit, TModule* module)
: TControl(parent, resourceId, title, module),
  TextLimit(textLimit),
  TransferBufferFieldType(tbftCharArray)
{
  DisableTransfer();
}

//
/// Constructs a TEdit object to encapsulate (alias) an existing control.
//
TStatic::TStatic(THandle hWnd, TModule* module)
: TControl(hWnd, module),
  TextLimit(0),
  TransferBufferFieldType(tbftCharArray)
{
  DisableTransfer();
}
  
//
/// Overrides TWindow's virtual EvSize function. When the static control is resized,
/// EvSize ensures that it is repainted.
//
void
TStatic::EvSize(uint sizeType, const TSize& size)
{
  Invalidate();
  TControl::EvSize(sizeType, size);
}

//
/// Returns the name of TStatic's registration class (STATIC)
//
TWindow::TGetClassNameReturnType
TStatic::GetClassName()
{
#if defined(OWL_SUPPORT_BWCC)
  if (GetApplication()->BWCCEnabled())
    return STATIC_CLASS;
  else
#endif
    return _T("STATIC");
}

uint
TStatic::Transfer(void* buffer, TTransferDirection direction)
{
  uint n = 0;
  switch (TransferBufferFieldType)
  {
  case tbftCharArray: 
    n = TransferCharArray(buffer, direction);
    break;

  case tbftString:
    n = TransferString(buffer, direction);
    break;
  
  default:
    CHECKX(false, _T("TStatic::Transfer: Unknown field type."));
  }
  return n;
}

TStatic::TTransferBufferFieldType TStatic::GetTransferBufferFieldType() const
{
  return TransferBufferFieldType;
}

void TStatic::SetTransferBufferFieldType(TTransferBufferFieldType t)
{
  TransferBufferFieldType = t;
}

uint
TStatic::TransferCharArray(void* buffer, TTransferDirection direction)
{
  if (!buffer) return 0;
  tchar* s = static_cast<tchar*>(buffer);
  if (direction == tdGetData)
    GetText(s, TextLimit);
  else if (direction == tdSetData)
    SetText(s);
  return TextLimit * sizeof(tchar);
}

uint
TStatic::TransferString(void* buffer, TTransferDirection direction)
{
  if (!buffer) return 0;
  tstring& s = *static_cast<tstring*>(buffer);
  if (direction == tdGetData)
    s = GetText();
  else if (direction == tdSetData)
    SetText(s);
  return sizeof(tstring);
}

//
/// Clears the text of the associated static control.
//
void
TStatic::Clear()
{
  SetText(_T(""));
}

IMPLEMENT_STREAMABLE1(TStatic, TControl);

#if !defined(BI_NO_OBJ_STREAMING)

//
/// Reads an instance of TStatic from the passed ipstream
//
void*
TStatic::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TControl*)GetObject(), is);
  is >> GetObject()->TextLimit;
  return GetObject();
}


//
/// Writes the TStatic to the passed opstream
//
void
TStatic::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TControl*)GetObject(), os);
  os << GetObject()->TextLimit;
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */

