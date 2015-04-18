//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998, by Yura Bidus, All Rights Reserved
//
/// \file
/// Implementation of class TDrawItemProxy, TDrawItem,TButtonTextDrawItem,
/// TDrawMenuItem, TDrawMenuItemProxy. Owner draw family.
// 
// Note this classes stil under development!!!!!!!!!!!!!!!
//
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/uimetric.h>
#include <owl/template.h>
#include <owl/gadgetwi.h>
#include <owl/celarray.h>
#include <owl/uihelper.h>
#include <owl/glyphbtn.h>

#include <owl/drawitem.h>
#include <owl/resource.h>

#include <algorithm>

using namespace std;

namespace owl {

//
// class TDrawItemProxy
// ~~~~~ ~~~~~~~~~~~~~~
/// Constructs a TDrawItemProxy object using the specified font. Ownership of the
/// font transfers to the new object.
//
TDrawItemProxy::TDrawItemProxy(TFont* font)
:
  Font(font),CelArray(0),DeleteCel(false)
{
  ItemArray = new TDrawItemArray;
}
//
/// Deletes Font. If DeleteCel is true, delets CelArray.
//
TDrawItemProxy::~TDrawItemProxy() 
{ 
  delete Font;
  if(DeleteCel)
    delete CelArray; 
  delete ItemArray;
}
//
/// Deletes Font and sets it to font. Ownership of font transfers to the
/// TDrawItemProxy.
//
void TDrawItemProxy::SetFont(TFont* font)
{
  delete Font;
  Font = font;
}
//
/// If DeleteCel is true, deletes CelArray. CelArray is then set to celarray and
/// DeleteCel is set to del.
//
void TDrawItemProxy::SetCelArray(TCelArray* celarray, TAutoDelete del)
{
  if(DeleteCel)
    delete CelArray; 
  CelArray = celarray; 
  DeleteCel = del == AutoDelete;
}
//
/// Loads a TDib with resource id resId from module. It then uses this TDib and
/// count to create a new TCelArray. SetCelArray is then called with this TCelArray
/// and AutoDelete.
//
void TDrawItemProxy::SetCelArray(TResId resId, uint count, TModule* module)
{
  TBtnBitmap* bitmap = new TBtnBitmap(*module,resId);
  SetCelArray(new TCelArray(bitmap,count),AutoDelete);
}
//
/// Adds item to ItemArray.
//
void TDrawItemProxy::RegisterItem(TDrawItem* item)
{
  ItemArray->Add(item);
}
//
//---------------------------------------------------------------------------
//
// class TButtonTextDrawItem
// ~~~~~ ~~~~~~~~~~~~~~~~~~~
//
const int TextSpaceH = 2;
const int TextSpaceV = 2;
const int TextSpaceE = 15;
const int ButtonSpaceH = 0;
const int ButtonSpaceV = 0;
//---------------------------------------------------------------------------------------
//
/// Constructs a TButtonTextDrawItem with proxy as it's parent.
//
TButtonTextDrawItem::TButtonTextDrawItem(TDrawItemProxy* proxy)
:
  Text(0),
  Index(-1),
  Align(aLeft),
  Style(sNone),
  LayoutStyle(lTextRight),
  Flags(0),
  Font(0),
  CelArray(0),
  DeleteCel(false),
  Proxy(proxy)
{
}
//
/// Destroys a TButtonTextDrawItem object. If DeleteCel is true, deletes CelArray.
//
TButtonTextDrawItem::~TButtonTextDrawItem()
{
  if(DeleteCel)
    delete   CelArray;
  delete[] Text;
  delete   Font;
}
//
/// Deletes Text and clears the text style. If text is not NULL, a copy is assigned
/// to Text and the sText style is added to Style.
//
void 
TButtonTextDrawItem::SetText(LPCTSTR text)
{
  delete[] Text;
  Text = 0;
  uint style = (uint)Style; 
  style &= ~sText;
  if(text){
    Text    = strnewdup(text);
    // replace all '\a' to '\t'
    // '\a' in *.rc becomes '\b' in *.res
    tchar c = _T('\b');
    tchar* p = _tcschr(Text,c);
    while(p){
      *p = _T('\t');
      p = _tcschr(Text,_T('\b'));
    }
    style |= sText;
  }
  Style = (TStyle)style; 
}
//
/// Index is set to index and the sBitmap style is cleared from Style. If index >=
/// 0, the sBitmap style is added to Style.
//
void 
TButtonTextDrawItem::SetIndex(int index)
{
  Index = index;
  uint style = (uint)Style; 
  style &= ~sBitmap;
  if(Index >=0)
    style |= sBitmap;
  Style = (TStyle)style; 
}
//
/// Deletes Font and then assigns font to Font. Ownership of font is transferred to
/// this instance.
//
void 
TButtonTextDrawItem::SetFont(TFont* font)
{
  delete Font;
  Font = font;
}
//
/// If DeleteCel is true, CelArray is deleted. celarray is assigned to CelArray and
/// del is assigned to DeleteCel.
//
void 
TButtonTextDrawItem::SetCelArray(TCelArray* celarray, TAutoDelete del)
{
  if(DeleteCel)
    delete CelArray; 
  CelArray = celarray; 
  DeleteCel = del == AutoDelete;
}
//
/// A bitmap with resource id resId is loaded from module and used to create a
/// TCelArray with count cells. SetCelArray is then called with this TCelArray.
//
void 
TButtonTextDrawItem::SetCelArray(TResId resId, uint count, TModule* module)
{
  TBtnBitmap* bitmap = new TBtnBitmap(*module,resId);
  SetCelArray(new TCelArray(bitmap,count));
}
//
/// Extracts information from drawItem and calls Paint.
//
void 
TButtonTextDrawItem::Draw(DRAWITEMSTRUCT & drawItem)
{
  Flags = drawItem.itemState;

  TDC dc(drawItem.hDC);
  TRect rect(drawItem.rcItem);
  Paint(dc, rect);
}
//
/// Calculates the area required to display the button and text.
//
void 
TButtonTextDrawItem::Measure(MEASUREITEMSTRUCT & measureItem)
{
  TSize textSize;
  GetTextSize(textSize);

  TSize btnSize(0,0);
  if(GetStyle() & TButtonTextDrawItem::sBitmap)
    GetButtonSize(btnSize);
  
  measureItem.itemWidth    = textSize.cx + btnSize.cx;
  measureItem.itemHeight  = std::max(textSize.cy,btnSize.cy);
}
//
/// Does an lstrcmp on Text from the instances contained in drawItem.
//
int 
TButtonTextDrawItem::Compare(COMPAREITEMSTRUCT & compareItem)
{
  TButtonTextDrawItem* item1 = (TButtonTextDrawItem*)compareItem.itemData1;
  TButtonTextDrawItem* item2 = (TButtonTextDrawItem*)compareItem.itemData2;
  return lstrcmp(item1->GetText(), item2->GetText());
}
//
/// If selected returns TColor::SysActiveCaption; otherwise returns
/// TColor::SysWindow.
//
TColor 
TButtonTextDrawItem::GetBkColor()
{
  return IsSelected() ? TColor::SysActiveCaption : TColor::SysWindow;
}
//
/// If selected, returns TColor::Sys3dHilight if a 3d User interface is in use or
/// TColor::SysGrayText otherwise.
//
TColor 
TButtonTextDrawItem::GetTextColor()
{
  if (IsSelected() || !IsEnabled())
    return TColor::Sys3dHilight;
  else
  	return TColor::SysWindowText;
}
//
/// Calculates the size of the rectangle need to paint the text.
//
void
TButtonTextDrawItem::GetTextSize(TSize& size)
{
  TFont* font = Font;
  if(!font)
    font = Proxy->GetFont();

  TScreenDC dc;

  if(font)
   dc.SelectObject(*font);

  dc.GetTextExtent(GetText(), static_cast<int>(::_tcslen(GetText())), size);

  if(font)
    dc.RestoreFont();
}
//
/// Paint function.
//
void 
TButtonTextDrawItem::Paint(TDC& dc, TRect& rect)
{
  TRect textRect, btnRect;
  
  Layout(rect, textRect, btnRect);

  // 1. Erase all
  TColor oldBkColor = dc.SetBkColor(GetBkColor());

  dc.ExtTextOut(rect.TopLeft(), ETO_OPAQUE, &rect, _T(""), 0, 0);

   PaintText(dc, textRect);

  PaintBitmap(dc, btnRect);
  
  dc.SetBkColor(oldBkColor);

  PaintBorder(dc, rect);
}
//
/// Layout Text and Bitmap
///
/// Determines the bounding rectangles for the current style.
//
void
TButtonTextDrawItem::Layout(const TRect& faceRect, TRect& textRect, TRect& btnRect)
{
  TSize textSize;
  GetTextSize(textSize);

  TSize btnSize;
  GetButtonSize(btnSize);

  switch(LayoutStyle){

    case lTextLeft:
      textRect = TRect(faceRect.TopLeft(), TSize(textSize.cx,faceRect.Height()));
      btnRect  = TRect(faceRect.left+textSize.cx+ButtonSpaceH,faceRect.top+ButtonSpaceV,
                       faceRect.right-ButtonSpaceH,faceRect.bottom-ButtonSpaceV);
       break;

    case lTextRight:
      btnRect = TRect(faceRect.left+ButtonSpaceH, faceRect.top+ButtonSpaceV,
                      faceRect.left+btnSize.cx-ButtonSpaceH, faceRect.bottom-ButtonSpaceV);
      textRect = TRect(btnRect.right+TextSpaceH+ButtonSpaceH,faceRect.top,
                       faceRect.right-TextSpaceH, faceRect.bottom);

      break;

    case lTextTop:
       textRect = TRect(faceRect.TopLeft(), TSize(faceRect.Width()-TextSpaceH,textSize.cy));
      btnRect  = TRect(faceRect.left+ButtonSpaceH,faceRect.top+textSize.cy+TextSpaceV,
                       faceRect.right-ButtonSpaceH, faceRect.bottom-ButtonSpaceV);
       break;

    case lTextBottom:
       textRect = TRect(faceRect.left,faceRect.bottom-textSize.cy,
                      faceRect.right,faceRect.bottom);
       btnRect  = TRect(faceRect.left+ButtonSpaceH,faceRect.top+ButtonSpaceV,
                      faceRect.right-ButtonSpaceH, faceRect.bottom-textSize.cy-ButtonSpaceV);
      break;
  }
}
//
/// Paint the text.
//
void
TButtonTextDrawItem::PaintText(TDC& dc, TRect& rect)
{
  if((Style&sText)==0)
    return;

  TFont* font = Font;
  if (!font)
    font = Proxy->GetFont(); 
  if(font)
   dc.SelectObject(*font);

   TColor oldTxColor  = dc.SetTextColor(GetTextColor());

  uint format =  DT_SINGLELINE|DT_EXPANDTABS;
  switch(Align){
    case aLeft:
      format |= DT_LEFT;
      break;
    case aRight:
      format |= DT_RIGHT;
      break;
    case aCenter:
      format |= DT_CENTER;
      break;
  }
  switch(LayoutStyle){
    case lTextLeft:
    case lTextRight:
      format |= DT_VCENTER;
      break;
    case lTextTop:
      format |= DT_VCENTER;//DT_BOTTOM;
      break;
    case lTextBottom:
      format |= DT_VCENTER;//DT_TOP;
      break;
  }

  // Create a UI Face object for this button & let it paint the button face
  //
  TRect  tRect(rect);
  TPoint dstPt(tRect.TopLeft());

  TUIFace face(tRect, Text, GetBkColor(), format);
  if ((!IsEnabled()||IsGrayed()) && !IsSelected())
    face.Paint(dc, dstPt, TUIFace::Disabled, false, false);
  else
    face.Paint(dc, dstPt, TUIFace::Normal, false, false);

  dc.SetTextColor(oldTxColor);
  
  if(font)
    dc.RestoreFont();
}
//
/// Calculates the size of button.
//
void
TButtonTextDrawItem::GetButtonSize(TSize& btnSize)
{
  btnSize = TSize(0,0);
  if(Index == sepNoBitmap) // == -2
    return;
  TCelArray* celArray = CelArray ? CelArray : Proxy->GetCelArray();
  if(celArray)
    btnSize = celArray->CelSize();
}
//
/// Paint the border of the gadget based on the BorderStyle member
//
void
TButtonTextDrawItem::PaintBorder(TDC& dc, TRect& boundsRect)
{
  if(IsFocused())
    dc.DrawFocusRect(boundsRect);
}
//
/// Paint the bitmap.
//
void 
TButtonTextDrawItem::PaintBitmap(TDC& dc, TRect& rect)
{
  if((Style&sBitmap)==0 || Index < 0)
    return;

  // Determine which CelArray to paint from: ours or our Window's
  //
  TCelArray& celArray = CelArray ? *CelArray : *Proxy->GetCelArray();
  // Create a UI Face object for this button & let it paint the button face
  celArray.BitBlt(Index, dc, rect.left, rect.top);
}
//////////////////////////////////////////////////////////////////////////////////////
//
//

//
/// Called with the proxy parent, the command it, the index in the menu and the text
/// to display.
//
TDrawMenuItem::TDrawMenuItem(TDrawItemProxy* proxy, int cmdId, int index, LPCTSTR title)
:
  TButtonTextDrawItem(proxy),CmdId(cmdId),CheckPainter(0)
{
  SetText(title);
  SetIndex(index);
}

//
/// String-aware overload
//
TDrawMenuItem::TDrawMenuItem(TDrawItemProxy* proxy, int cmdId, int index, const tstring& title)
  : TButtonTextDrawItem(proxy), CmdId(cmdId), CheckPainter(0)
{
  SetText(title.c_str());
  SetIndex(index);
}

//
//
//
void
TDrawMenuItem::GetTextSize(TSize& size)
{
  TButtonTextDrawItem::GetTextSize(size);
  size.cx += TextSpaceE;
  if((GetStyle() & TButtonTextDrawItem::sBitmap)==0)
    size.cx += TextSpaceE;
  size.cy = std::max((int)size.cy, 16);
}
//
/// Get the size of the bitmap button.
//
void
TDrawMenuItem::GetButtonSize(TSize& btnSize)
{
  btnSize = TSize(0,0);
  if(Index == sepNoBitmap) // == -2
    return;
  TCelArray* celArray = CelArray ? CelArray : Proxy->GetCelArray();
  if(celArray){
    btnSize = celArray->CelSize();
    btnSize.cx  += 3;
    btnSize.cy  += 3;
  }
}
//
/// If the menu item is selected returns TColor::SysActiveCaption; otherwise returns
/// TColor::SysMenu.
//
TColor 
TDrawMenuItem::GetBkColor()
{
  return IsSelected() ? TColor::SysActiveCaption : TColor::SysMenu;
}
//
/// Returns the text color.
//
TColor 
TDrawMenuItem::GetTextColor()
{
  if (IsSelected() || !IsEnabled())
    return TColor::Sys3dHilight;
  else
  	return TColor::SysMenuText;
}
//
/// Paint the bitmap on the menu entry.
//
void 
TDrawMenuItem::PaintBitmap(TDC& dc, TRect& rect)
{
  if(((Style&sBitmap)==0 || Index < 0) && CheckPainter==0)
    return;

  TColor oldBkColor = dc.SetBkColor(TColor::SysMenu);
  dc.ExtTextOut(rect.TopLeft(), ETO_OPAQUE, &rect, _T(""), 0, 0);

  if(CheckPainter){
    
    if(IsChecked())
      (this->*CheckPainter)(dc, rect);
    else
      PaintCheck_None(dc,rect);
    
    dc.SetBkColor(oldBkColor);
    return;
  }

  // Determine which CelArray to paint from: ours or our Window's
  //
  TCelArray& celArray = CelArray ? *CelArray : *Proxy->GetCelArray();

  // Calc the source rect from the celarray. The dest point of the glyph is
  // relative to the face rect.
  //
  TRect  srcRect(celArray.CelRect(Index));

  // Paint border
  //
  TUIBorder::TEdge edge = TUIBorder::TEdge(0);
  if(IsChecked())
    edge = TUIBorder::SunkenInner;
  else if(IsSelected()&&IsEnabled())
    edge = TUIBorder::RaisedInner;
  if(edge)
    TUIBorder(rect, edge).Paint(dc);
  
  rect.Inflate(-1,-1);

  // Create a UI Face object for this button & let it paint the button face
  //
  TUIFace face(rect, celArray.operator TBitmap&()); 
  TPoint dstPt(1,1);

  if (!IsEnabled())
    face.Paint(dc, srcRect, dstPt, TUIFace::Disabled, IsChecked());
  else if(IsGrayed())
    face.Paint(dc, srcRect, dstPt, TUIFace::Indeterm, IsChecked());
  else
#if 0
    face.Paint(dc, srcRect, (IsSelected() && !IsChecked()) ? 
               dstPt.Offset(-1,-1) : dstPt, TUIFace::Normal, IsChecked());
#else
    face.Paint(dc, srcRect, dstPt, TUIFace::Normal, IsChecked());
#endif
  dc.SetBkColor(oldBkColor);
}
//
// Paint Text
//
void
TDrawMenuItem::PaintText(TDC& dc, TRect& rect)
{
  if((Style&sText)==0)
    return;

  TFont* font = Font;
  if (!font)
    font = Proxy->GetFont(); 
  if(font)
   dc.SelectObject(*font);

   TColor oldTxColor  = dc.SetTextColor(GetTextColor());

  uint format =  DT_SINGLELINE|DT_EXPANDTABS;
  switch(Align){
    case aLeft:
      format |= DT_LEFT;
      break;
    case aRight:
      format |= DT_RIGHT;
      break;
    case aCenter:
      format |= DT_CENTER;
      break;
  }
  switch(LayoutStyle){
    case lTextLeft:
    case lTextRight:
      format |= DT_VCENTER;
      break;
    case lTextTop:
      format |= DT_VCENTER;//DT_BOTTOM;
      break;
    case lTextBottom:
      format |= DT_VCENTER;//DT_TOP;
      break;
  }

  // DT_RIGHT for \t
  tstring cs1;
  tstring cs(Text);
  size_t t_pos = cs.find(_T("\t")); //JJH changed int -> size_t

  TRect   tRect(rect);
  tRect.right -= TextSpaceE;

  // Accelerator can be \b instead of \t (NOTE: \a in .rc make a \b in res)
  //
  if(t_pos == NPOS)
    t_pos = cs.find(_T("\b"));
 
  if (t_pos != NPOS){
    cs1 = cs.substr(t_pos+1);
    cs  = cs.substr(0, t_pos);
    // Create a UI Face object for this button & let it paint the button face
    //
    TPoint  dstPt(tRect.TopLeft());

    TUIFace face1(tRect, cs.c_str(), GetBkColor(), format);
    format &= ~DT_LEFT;
    format |= DT_RIGHT;
    TUIFace face2(tRect, cs1.c_str(), GetBkColor(), format);
    
    if ((!IsEnabled()||IsGrayed()) && !IsSelected()){
      face1.Paint(dc, dstPt, TUIFace::Disabled, false, false);
      face2.Paint(dc, dstPt, TUIFace::Disabled, false, false);
    }
    else{
      face1.Paint(dc, dstPt, TUIFace::Normal, false, false);
      face2.Paint(dc, dstPt, TUIFace::Normal, false, false);
    }
  }
  else{
    // Create a UI Face object for this button & let it paint the button face
    //
    TPoint  dstPt(tRect.TopLeft());

    TUIFace face(tRect, Text, GetBkColor(), format);
    if ((!IsEnabled()||IsGrayed()) && !IsSelected())
      face.Paint(dc, dstPt, TUIFace::Disabled, false, false);
    else
      face.Paint(dc, dstPt, TUIFace::Normal, false, false);
  }

  dc.SetTextColor(oldTxColor);
  
  if(font)
    dc.RestoreFont();
}
//
/// If checked, clears the area in preparation for painting the check mark.
//
void 
TDrawMenuItem::PaintCheckFace(TDC& dc, TRect& rect)
{
  if(IsChecked()){
    TColor oldBkColor = dc.SetBkColor(TColor::SysMenu);
    if(!IsSelected() || !IsEnabled())
      dc.FillRect(rect, TUIFace::GetDitherBrush());
    else
      dc.ExtTextOut(rect.TopLeft(), ETO_OPAQUE, &rect, _T(""), 0, 0);
    TUIBorder(rect, TUIBorder::Recessed).Paint(dc);
    dc.SetBkColor(oldBkColor);
  }
}
///////////////////////////////////////////////////////////////////////////////////////////
//
// Check Draw functions
//
static TColor SelectColor(bool enabled)
{
  return enabled ? TColor::SysMenuText : TColor::Sys3dHilight;
}
//
///   Draw this:
/// \code
///   01234567
/// 0 ---x----
/// 1 ---xx---
/// 2 ---xxx--
/// 3 ---xxxx-
/// 4 ---xxx--
/// 5 ---xx---
/// 6 ---x----
/// 7 --------
/// \endcode
//
void 
TDrawMenuItem::PaintCheck_3Angle(TDC& dc, TRect& rect)
{
  //rect.Inflate(-3,-3);
  //TPoint point(rect.left+3,rect.top+3);
  rect.Inflate(-2,-2);
  TPoint point(rect.left+3,rect.top+2);
  TColor color = SelectColor(IsEnabled());

  PaintCheckFace(dc,rect);

  //  Draw this:
  //
  //   01234567
  // 0 ---x----
  // 1 ---xx---
  // 2 ---xxx--
  // 3 ---xxxx-
  // 4 ---xxx--
  // 5 ---xx---
  // 6 ---x----
  // 7 --------
  //
  dc.SetPixel(point.x + 3, point.y + 0, color);
  dc.SetPixel(point.x + 3, point.y + 1, color);
  dc.SetPixel(point.x + 3, point.y + 2, color);
  dc.SetPixel(point.x + 3, point.y + 3, color);
  dc.SetPixel(point.x + 3, point.y + 4, color);
  dc.SetPixel(point.x + 3, point.y + 5, color);
  dc.SetPixel(point.x + 3, point.y + 6, color);
  dc.SetPixel(point.x + 4, point.y + 1, color);
  dc.SetPixel(point.x + 4, point.y + 2, color);
  dc.SetPixel(point.x + 4, point.y + 3, color);
  dc.SetPixel(point.x + 4, point.y + 4, color);
  dc.SetPixel(point.x + 4, point.y + 5, color);
  dc.SetPixel(point.x + 5, point.y + 2, color);
  dc.SetPixel(point.x + 5, point.y + 3, color);
  dc.SetPixel(point.x + 5, point.y + 4, color);
  dc.SetPixel(point.x + 6, point.y + 3, color);
  if(!IsEnabled()){
    //  Add this:
    //
    //   01234567
    // 0 --------
    // 1 --------
    // 2 --------
    // 3 --------
    // 4 ------x-
    // 5 -----x--
    // 6 ----x---
    // 7 ---x----
    //
    TColor clr = TColor::Sys3dHilight;
    dc.SetPixel(point.x + 3, point.y + 7, clr);
    dc.SetPixel(point.x + 4, point.y + 6, clr);
    dc.SetPixel(point.x + 5, point.y + 5, clr);
    dc.SetPixel(point.x + 6, point.y + 4, clr);
  }
}
//
///   Draw this:
/// \code
///    01234567
///  0 ---x----
///  1 ---xx---
///  2 ---xxx--
///  3 xxxxxxx-
///  4 ---xxx--
///  5 ---xx---
///  6 ---x----
///  7 --------
/// \endcode
void 
TDrawMenuItem::PaintCheck_Arrow(TDC& dc, TRect& rect)
{
  //rect.Inflate(-3,-3);
  //TPoint point(rect.left+4,rect.top+3);
  rect.Inflate(-2,-2);
  TPoint point(rect.left+4,rect.top+2);
  TColor color = SelectColor(IsEnabled());

  PaintCheckFace(dc,rect);

  //  Draw this:
  //
  //   01234567
  // 0 ---x----
  // 1 ---xx---
  // 2 ---xxx--
  // 3 xxxxxxx-
  // 4 ---xxx--
  // 5 ---xx---
  // 6 ---x----
  // 7 --------
  //

  dc.SetPixel(point.x + 0, point.y + 3, color);
  dc.SetPixel(point.x + 1, point.y + 3, color);
  dc.SetPixel(point.x + 2, point.y + 3, color);
  dc.SetPixel(point.x + 3, point.y + 0, color);
  dc.SetPixel(point.x + 3, point.y + 1, color);
  dc.SetPixel(point.x + 3, point.y + 2, color);
  dc.SetPixel(point.x + 3, point.y + 3, color);
  dc.SetPixel(point.x + 3, point.y + 4, color);
  dc.SetPixel(point.x + 3, point.y + 5, color);
  dc.SetPixel(point.x + 3, point.y + 6, color);
  dc.SetPixel(point.x + 4, point.y + 1, color);
  dc.SetPixel(point.x + 4, point.y + 2, color);
  dc.SetPixel(point.x + 4, point.y + 3, color);
  dc.SetPixel(point.x + 4, point.y + 4, color);
  dc.SetPixel(point.x + 4, point.y + 5, color);
  dc.SetPixel(point.x + 5, point.y + 2, color);
  dc.SetPixel(point.x + 5, point.y + 3, color);
  dc.SetPixel(point.x + 5, point.y + 4, color);
  dc.SetPixel(point.x + 6, point.y + 3, color);
  if(!IsEnabled()){
    //  Add this:
    //
    //   01234567
    // 0 --------
    // 1 --------
    // 2 --------
    // 3 --------
    // 4 ------x-
    // 5 -----x--
    // 6 ----x---
    // 7 ---x----
    //
    TColor clr = TColor::Sys3dHilight;
    dc.SetPixel(point.x + 3, point.y + 7, clr);
    dc.SetPixel(point.x + 4, point.y + 6, clr);
    dc.SetPixel(point.x + 5, point.y + 5, clr);
    dc.SetPixel(point.x + 6, point.y + 4, clr);
  }
}
//
///   Draw this:
/// \code
///    01234567
///  1 -xxxxxxx
///  2 -xxxxxxx
///  3 -xxxxxxx
///  4 -xxxxxxx
///  5 -xxxxxxx
///  6 -xxxxxxx
///  7 -xxxxxxx
/// \endcode
//
void 
TDrawMenuItem::PaintCheck_Box(TDC& dc, TRect& rect)
{
  //rect.Inflate(-3,-3);
  //TPoint point(rect.left+4,rect.top+3);
  rect.Inflate(-2,-2);
  TPoint point(rect.left+4,rect.top+2);
  TColor color = SelectColor(IsEnabled());

  PaintCheckFace(dc,rect);

  //  Draw this:
  //
  //   01234567
  // 0 --------
  // 1 -xxxxxxx
  // 2 -xxxxxxx
  // 3 -xxxxxxx
  // 4 -xxxxxxx
  // 5 -xxxxxxx
  // 6 -xxxxxxx
  // 7 -xxxxxxx
  //
  
  dc.TextRect(point.x + 1, point.y + 1, point.x + 7, point.y + 7, color);

  if(!IsEnabled()){
    //  Add this:
    //
    //   01234567
    // 0 --------
    // 1 --------
    // 2 --------x
    // 3 --------x
    // 4 --------x
    // 5 --------x
    // 6 --------x
    // 7 --------x
    //     xxxxxxx
    //
    TPen pen(TColor::Sys3dHilight, 0, PS_SOLID);
    dc.SelectObject(pen);
    dc.MoveTo(point.x + 2, point.y + 8);
    dc.LineTo(point.x + 8, point.y + 8);
    dc.MoveTo(point.x + 8, point.y + 2);
    dc.LineTo(point.x + 8, point.y + 9);
    dc.RestorePen();
  }
}
//
///   Draw this:
/// \code
///    01234567
///  0 --------
///  1 ----x---
///  2 ---xxx--
///  3 --xxxxx-
///  4 -xxxxxxx
///  5 --xxxxx-
///  6 ---xxx--
///  7 ----x---
/// \endcode
void 
TDrawMenuItem::PaintCheck_Diamond(TDC& dc, TRect& rect)
{
  //rect.Inflate(-3,-3);
  //TPoint point(rect.left+3,rect.top+2);
  rect.Inflate(-2,-2);
  TPoint point(rect.left+3,rect.top+1);
  TColor color = SelectColor(IsEnabled());

  PaintCheckFace(dc,rect);

  //  Draw this:
  //
  //   01234567
  // 0 --------
  // 1 ----x---
  // 2 ---xxx--
  // 3 --xxxxx-
  // 4 -xxxxxxx
  // 5 --xxxxx-
  // 6 ---xxx--
  // 7 ----x---
  //

  dc.SetPixel(point.x + 1, point.y + 4, color);
  dc.SetPixel(point.x + 2, point.y + 3, color);
  dc.SetPixel(point.x + 2, point.y + 4, color);
  dc.SetPixel(point.x + 2, point.y + 5, color);
  dc.SetPixel(point.x + 3, point.y + 2, color);
  dc.SetPixel(point.x + 3, point.y + 3, color);
  dc.SetPixel(point.x + 3, point.y + 4, color);
  dc.SetPixel(point.x + 3, point.y + 5, color);
  dc.SetPixel(point.x + 3, point.y + 6, color);
  dc.SetPixel(point.x + 4, point.y + 1, color);
  dc.SetPixel(point.x + 4, point.y + 2, color);
  dc.SetPixel(point.x + 4, point.y + 3, color);
  dc.SetPixel(point.x + 4, point.y + 4, color);
  dc.SetPixel(point.x + 4, point.y + 5, color);
  dc.SetPixel(point.x + 4, point.y + 6, color);
  dc.SetPixel(point.x + 4, point.y + 7, color);
  dc.SetPixel(point.x + 5, point.y + 2, color);
  dc.SetPixel(point.x + 5, point.y + 3, color);
  dc.SetPixel(point.x + 5, point.y + 4, color);
  dc.SetPixel(point.x + 5, point.y + 5, color);
  dc.SetPixel(point.x + 5, point.y + 6, color);
  dc.SetPixel(point.x + 6, point.y + 3, color);
  dc.SetPixel(point.x + 6, point.y + 4, color);
  dc.SetPixel(point.x + 6, point.y + 5, color);
  dc.SetPixel(point.x + 7, point.y + 4, color);
  if(!IsEnabled()){
    //  Add this:
    //
    //   01234567
    // 0 --------
    // 1 --------
    // 2 --------
    // 3 --------
    // 4 --------
    // 5 -------x
    // 6 ------x-
    // 7 -----x--
    //       x
    //
    TColor clr = TColor::Sys3dHilight;
    dc.SetPixel(point.x + 4, point.y + 8, clr);
    dc.SetPixel(point.x + 5, point.y + 7, clr);
    dc.SetPixel(point.x + 6, point.y + 6, clr);
    dc.SetPixel(point.x + 7, point.y + 5, clr);
  }
}
//
///   Draw this:
/// \code
///    01234567
///  0 ---xx---
///  1 --xxxx--
///  2 -xxxxxx-
///  3 -xxxxxx-
///  4 -xxxxxx-
///  5 --xxxx--
///  6 ---xx---
///  7 --------
/// \endcode
//
void 
TDrawMenuItem::PaintCheck_Dot(TDC& dc, TRect& rect)
{
  //rect.Inflate(-3,-3);
  //TPoint point(rect.left+4,rect.top+3);
  rect.Inflate(-2,-2);
  TPoint point(rect.left+4,rect.top+2);
  TColor color = SelectColor(IsEnabled());

  PaintCheckFace(dc,rect);

  //  Draw this:
  //
  //   01234567
  // 0 ---xx---
  // 1 --xxxx--
  // 2 -xxxxxx-
  // 3 -xxxxxx-
  // 4 -xxxxxx-
  // 5 --xxxx--
  // 6 ---xx---
  // 7 --------
  //
  
  TPen pen(color, 0, PS_SOLID);
  dc.SelectObject(pen);

  dc.MoveTo(point.x + 1, point.y + 2);
  dc.LineTo(point.x + 1, point.y + 5);
  dc.MoveTo(point.x + 2, point.y + 1);
  dc.LineTo(point.x + 2, point.y + 6);
  dc.MoveTo(point.x + 3, point.y + 0);
  dc.LineTo(point.x + 3, point.y + 7);
  dc.MoveTo(point.x + 4, point.y + 0);
  dc.LineTo(point.x + 4, point.y + 7);
  dc.MoveTo(point.x + 5, point.y + 1);
  dc.LineTo(point.x + 5, point.y + 6);
  dc.MoveTo(point.x + 6, point.y + 2);
  dc.LineTo(point.x + 6, point.y + 5);
  dc.RestorePen();
  if(!IsEnabled()){
    //  Add this:
    //
    //   01234567
    // 0 --------
    // 1 --------
    // 2 --------
    // 3 --------
    // 4 --------
    // 5 ------x-
    // 6 -----x--
    // 7 ---xx---
    //
    TColor clr = TColor::Sys3dHilight;
    dc.SetPixel(point.x + 3, point.y + 7, clr);
    dc.SetPixel(point.x + 4, point.y + 7, clr);
    dc.SetPixel(point.x + 5, point.y + 6, clr);
    dc.SetPixel(point.x + 6, point.y + 5, clr);
  }
}
//
///   Draw this:
/// \code
///    01234567
///  0 --------
///  1 --------
///  2 --------
///  3 --------
///  4 --------
///  5 --------
///  6 --------
///  7 --------
/// \endcode
//
void 
TDrawMenuItem::PaintCheck_None(TDC& dc, TRect& rect)
{
  //rect.Inflate(-3,-3);
  rect.Inflate(-2,-2);

  PaintCheckFace(dc,rect);

  //  Draw this:
  //
  //   01234567
  // 0 --------
  // 1 --------
  // 2 --------
  // 3 --------
  // 4 --------
  // 5 --------
  // 6 --------
  // 7 --------
  //
}
//
///   Draw this:
/// \code
///    01234567
///  0 --------
///  1 ---xx---
///  2 ---xx---
///  3 -xxxxxx-
///  4 -xxxxxx-
///  5 ---xx---
///  6 ---xx---
///  7 --------
/// \endcode
//
void 
TDrawMenuItem::PaintCheck_Plus(TDC& dc, TRect& rect)
{
  //rect.Inflate(-3,-3);
  //TPoint point(rect.left+4,rect.top+3);
  rect.Inflate(-2,-2);
  TPoint point(rect.left+4,rect.top+2);
  TColor color = SelectColor(IsEnabled());

  PaintCheckFace(dc,rect);

  //  Draw this:
  //
  //   01234567
  // 0 --------
  // 1 ---xx---
  // 2 ---xx---
  // 3 -xxxxxx-
  // 4 -xxxxxx-
  // 5 ---xx---
  // 6 ---xx---
  // 7 --------
  //

  TPen Pen(color, 0, PS_SOLID);
  dc.SelectObject(Pen);

  dc.MoveTo(point.x + 3, point.y + 1);
  dc.LineTo(point.x + 3, point.y + 7);
  dc.MoveTo(point.x + 4, point.y + 1);
  dc.LineTo(point.x + 4, point.y + 7);
  dc.MoveTo(point.x + 1, point.y + 3);
  dc.LineTo(point.x + 7, point.y + 3);
  dc.MoveTo(point.x + 1, point.y + 4);
  dc.LineTo(point.x + 7, point.y + 4);
  dc.RestorePen();
  if(!IsEnabled()){
    //  Add this:
    //
    //   01234567
    // 0 --------
    // 1 --------
    // 2 --------
    // 3 --------
    // 4 --------
    // 5 -----xx-
    // 6 -----x--
    // 7 ---xxx--
    //
    TColor clr = TColor::Sys3dHilight;
    dc.SetPixel(point.x + 3, point.y + 7, clr);
    dc.SetPixel(point.x + 4, point.y + 7, clr);
    dc.SetPixel(point.x + 5, point.y + 7, clr);
    dc.SetPixel(point.x + 5, point.y + 6, clr);
    dc.SetPixel(point.x + 5, point.y + 5, clr);
    dc.SetPixel(point.x + 6, point.y + 5, clr);
  }
}
//
///   Draw this:
/// \code
///    01234567
///  0 ------x-
///  1 -----xx-
///  2 x---xxx-
///  3 xx-xxx--
///  4 xxxxx---
///  5 -xxx----
///  6 --x-----
///  7 --------
/// \endcode
//
void 
TDrawMenuItem::PaintCheck_V(TDC& dc, TRect& rect)
{
  //rect.Inflate(-3,-3);
  //TPoint point(rect.left+4,rect.top+3);
  rect.Inflate(-2,-2);
  TPoint point(rect.left+4,rect.top+2);
  TColor color = SelectColor(IsEnabled());

  PaintCheckFace(dc,rect);

  //  Draw this:
  //
  //   01234567
  // 0 ------x-
  // 1 -----xx-
  // 2 x---xxx-
  // 3 xx-xxx--
  // 4 xxxxx---
  // 5 -xxx----
  // 6 --x-----
  // 7 --------
  //

  dc.SetPixel(point.x + 0, point.y + 2, color);
  dc.SetPixel(point.x + 0, point.y + 3, color);
  dc.SetPixel(point.x + 0, point.y + 4, color);
  dc.SetPixel(point.x + 1, point.y + 3, color);
  dc.SetPixel(point.x + 1, point.y + 4, color);
  dc.SetPixel(point.x + 1, point.y + 5, color);
  dc.SetPixel(point.x + 2, point.y + 4, color);
  dc.SetPixel(point.x + 2, point.y + 5, color);
  dc.SetPixel(point.x + 2, point.y + 6, color);
  dc.SetPixel(point.x + 3, point.y + 3, color);
  dc.SetPixel(point.x + 3, point.y + 4, color);
  dc.SetPixel(point.x + 3, point.y + 5, color);
  dc.SetPixel(point.x + 4, point.y + 2, color);
  dc.SetPixel(point.x + 4, point.y + 3, color);
  dc.SetPixel(point.x + 4, point.y + 4, color);
  dc.SetPixel(point.x + 5, point.y + 1, color);
  dc.SetPixel(point.x + 5, point.y + 2, color);
  dc.SetPixel(point.x + 5, point.y + 3, color);
  dc.SetPixel(point.x + 6, point.y + 0, color);
  dc.SetPixel(point.x + 6, point.y + 1, color);
  dc.SetPixel(point.x + 6, point.y + 2, color);
  if(!IsEnabled()){
    //  Add this:
    //
    //   01234567
    // 0 --------
    // 1 --------
    // 2 --------
    // 3 ------x-
    // 4 -----x--
    // 5 ----x---
    // 6 ---x----
    // 7 --x-----
    //
    TColor clr = TColor::Sys3dHilight;
    dc.SetPixel(point.x + 2, point.y + 7, clr);
    dc.SetPixel(point.x + 3, point.y + 6, clr);
    dc.SetPixel(point.x + 4, point.y + 5, clr);
    dc.SetPixel(point.x + 5, point.y + 4, clr);
    dc.SetPixel(point.x + 6, point.y + 3, clr);
  }
}
//
///   Draw this:
/// \code
///    01234567
///  0 --------
///  1 xx---xx-
///  2 -xx-xx--
///  3 --xx----
///  4 ---xx---
///  5 -xx-xx--
///  6 xx---xx-
///  7 --------
/// \endcode
//
void 
TDrawMenuItem::PaintCheck_X(TDC& dc, TRect& rect)
{
  //rect.Inflate(-3,-3);
  rect.Inflate(-2,-2);
  TColor color = SelectColor(IsEnabled());

  PaintCheckFace(dc,rect);

  //  Draw this:
  //
  //   01234567
  // 0 --------
  // 1 xx---xx-
  // 2 -xx-xx--
  // 3 --xx----
  // 4 ---xx---
  // 5 -xx-xx--
  // 6 xx---xx-
  // 7 --------
  //
  //TPoint point(rect.left+4,rect.top+3);
  TPoint point(rect.left+4,rect.top+2);
  dc.SetPixel(point.x + 0, point.y + 1, color);
  dc.SetPixel(point.x + 1, point.y + 1, color);
  dc.SetPixel(point.x + 1, point.y + 2, color);
  dc.SetPixel(point.x + 2, point.y + 2, color);
  dc.SetPixel(point.x + 2, point.y + 3, color);
  dc.SetPixel(point.x + 3, point.y + 3, color);
  dc.SetPixel(point.x + 3, point.y + 4, color);
  dc.SetPixel(point.x + 4, point.y + 4, color);
  dc.SetPixel(point.x + 4, point.y + 5, color);
  dc.SetPixel(point.x + 5, point.y + 5, color);
  dc.SetPixel(point.x + 5, point.y + 6, color);
  dc.SetPixel(point.x + 6, point.y + 6, color);
  dc.SetPixel(point.x + 5, point.y + 1, color);
  dc.SetPixel(point.x + 6, point.y + 1, color);
  dc.SetPixel(point.x + 4, point.y + 2, color);
  dc.SetPixel(point.x + 5, point.y + 2, color);
  dc.SetPixel(point.x + 1, point.y + 5, color);
  dc.SetPixel(point.x + 2, point.y + 5, color);
  dc.SetPixel(point.x + 0, point.y + 6, color);
  dc.SetPixel(point.x + 1, point.y + 6, color);
  if(!IsEnabled()){
    //  Add this:
    //
    //   01234567
    // 0 --------
    // 1 --------
    // 2 ------x-
    // 3 ----x---
    // 4 --------
    // 5 ---x----
    // 6 --x-----
    // 7 -x------
    //
    TColor clr = TColor::Sys3dHilight;
    dc.SetPixel(point.x + 6, point.y + 2, clr);
    dc.SetPixel(point.x + 4, point.y + 3, clr);
    dc.SetPixel(point.x + 3, point.y + 5, clr);
    dc.SetPixel(point.x + 2, point.y + 6, clr);
    dc.SetPixel(point.x + 1, point.y + 7, clr);
  }
}

//-------------------------------------------------------------------------
//
// class TDrawMenuItemProxy
// ~~~~~ ~~~~~~~~~~~~~~~~~~
//
/// Creates a font for use in menu bars.
//
TFont* 
TDrawMenuItemProxy::CreateProxyMenuFont()
{
  NONCLIENTMETRICS ncm;
  ncm.cbSize = sizeof(NONCLIENTMETRICS);
  SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
  return new TFont(ncm.lfMenuFont);
}
//
/// Constructs a TDrawMenuItemProxy object using the specified resource.
//
TDrawMenuItemProxy::TDrawMenuItemProxy(TResId resId, TModule* module)
:
 TDrawItemProxy(CreateProxyMenuFont())
{
  Array = new TUIntAray;
  TToolbarRes barRes(*module, resId, NoAutoDelete);
  int numRows = barRes.GetBitmap().Height()/barRes.GetHeight();
  { // map colors
    TBtnBitmap bitmap(barRes.GetBitmap(), TBtnBitmap::DefaultFaceColor, NoAutoDelete);
  }
  
  TCelArray* celAray = new TCelArray(
    &barRes.GetBitmap(),
    barRes.GetCount(),
    TSize(barRes.GetWidth(), barRes.GetHeight()), 
    TPoint(0, 0), // offset
    numRows);
  SetCelArray(celAray);
  
  for(int i = 0; i < (int)barRes.GetCount(); i++){
    if(barRes.GetIds()[i] >0)
      Array->Add((uint)barRes.GetIds()[i]);
  }
}
//
/// Destroys the instance.
//
TDrawMenuItemProxy::~TDrawMenuItemProxy()
{
  delete Array;
}
//
/// Returns the item with id cmdId if it exists; otherwise creates a new item and
/// returns it.
//
TDrawItem* 
TDrawMenuItemProxy::CreateItem(uint cmdId, const tstring& text)
{
  TDrawItemArray& items = GetItems();
  int index = GetIndex((int)cmdId);

  // if item already exist -> return existing one
  for(int i = 0; i < (int)items.Size(); i++){
    TButtonTextDrawItem* item = TYPESAFE_DOWNCAST(items[i], TButtonTextDrawItem);
    if(item->GetIndex() == index && item->GetText() == text)
      return items[i];
  }
  if(cmdId == static_cast<uint>(TButtonTextDrawItem::sepNoBitmap)) //JJH added static_cast
    index = TButtonTextDrawItem::sepNoBitmap;

  // if none found -> add new one
  TDrawMenuItem* item = new TDrawMenuItem(this, cmdId, index, text);
  RegisterItem(item);

  return item;
}
//
/// Walks through the menu tree hMenu adding seperators.
//
void 
TDrawMenuItemProxy::RemapMenu(HMENU hMenu)
{
  iRecurse = 0; // this->iRecurse = 0;
  
  iRecurse++;
  int nItem = ::GetMenuItemCount(hMenu);
  while ((--nItem)>=0) {
    uint itemId = ::GetMenuItemID(hMenu, nItem);
    if (itemId == (uint) -1){
      HMENU pops = ::GetSubMenu(hMenu, nItem);
      if (pops)
        RemapMenu(pops);
      if (iRecurse>0){
        tchar buffer[MAX_PATH];
        if (::GetMenuString(hMenu, nItem, buffer, MAX_PATH, MF_BYPOSITION) > 0){
          TDrawItem* item = CreateItem((uint)TButtonTextDrawItem::sepNoBitmap, buffer); // itemId = -1 
          ::ModifyMenu(hMenu, nItem, MF_BYPOSITION|MF_OWNERDRAW, (uint)-1, (LPCTSTR)item);
        }
      }
    }
    else {
      uint oldState = ::GetMenuState(hMenu, nItem, MF_BYPOSITION);
      if (!(oldState&MF_OWNERDRAW) && !(oldState&MF_BITMAP))  {
        if ((oldState&MF_SEPARATOR) || itemId == 0){
          // Insert separator items only if a background bitmap is drawn
          //
        }
        else{
          tchar buffer[MAX_PATH];
          if (::GetMenuString(hMenu, nItem, buffer, MAX_PATH, MF_BYPOSITION) > 0){
            TDrawItem* item = CreateItem(itemId, buffer); 
            ::ModifyMenu(hMenu, nItem, MF_BYPOSITION|MF_OWNERDRAW|oldState,
               (LPARAM)itemId, (LPCTSTR)item);
          }
        }
      }
    }
  }
  iRecurse--;
}
//
/// Returns true if a shortcut was pressed that fits into the menu and false if not.
//
bool 
TDrawMenuItemProxy::EvMenuChar(uint uChar, uint, HMENU hMenu, TParam2& param)
{
  PRECONDITION(hMenu);
  tchar keyPressed = (tchar)_totupper(uChar);
  int count = ::GetMenuItemCount(hMenu);
  while(count >= 0){
    uint itemId = ::GetMenuItemID(hMenu, count);
    if(itemId){
      TMenuItemInfo itemInfo(MIIM_DATA | MIIM_TYPE);
      if(::GetMenuItemInfo(hMenu, count, true, &itemInfo)){
        if(itemInfo.fType & MFT_OWNERDRAW){
          TDrawMenuItem* item = 0;
          try{
            item = TYPESAFE_DOWNCAST((TDrawMenuItem*)itemInfo.dwItemData,TDrawMenuItem);
          }
          catch(...){
            //.........
          }
          if(item){
            const tchar* p = _tcschr(item->GetText(),_T('&'));
            if(p){
              p++;
              if(_totupper(*p) == keyPressed){
                // Shortcut found.
                param = MkUint32((uint16)count, 2);
                return true;
              }
            }
          }
        }
      }
    }
    count--;
  }
  // Shortcut not found.
  return false;
}
//
/// Returns the TDrawItem with id cmdId, or 0 if it doesn't exist.
//
TDrawItem* TDrawMenuItemProxy::FindItem(uint cmdId)
{
  size_t index = GetIndex(cmdId); //JJH changed int -> size_t
  if(index != NPOS)
    return GetItems()[static_cast<int>(index)];

  TDrawItemArray& items = GetItems();
  for(index = 0; index < (size_t)items.Size(); index++){ //JJH changed int -> size_t
    TDrawItem* item = items[static_cast<int>(index)];
    TDrawMenuItem* mit = TYPESAFE_DOWNCAST(item, TDrawMenuItem);
    if(mit){
      if(mit->GetCmdId() == (int)cmdId)
        break;
    }
  }
  return index < (size_t)items.Size() ? items[static_cast<int>(index)] : 0; //JJH changed int -> size_t
}
//
/// Returns the array index of the item with id cmdId.
//
int TDrawMenuItemProxy::GetIndex(int cmdId)
{
  return Array->Find(cmdId);
}
//
} // OWL namespace

////////////////////////////////////////////////////////////////////////////////


