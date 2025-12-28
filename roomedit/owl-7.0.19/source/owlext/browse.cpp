//------------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// BROWSE.CPP
//
// TBrowse Class
//
// Original code by Daniel BERMAN (dberman@inge-com.fr)
// Written: 13th December 1995
//------------------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#include <owl/menu.h>

#include <owlext/browse.h>

#if defined(_MSC_VER)
# pragma warning(disable: 4996) // Turn off deprecation warnings (triggered by GetVersion/GetVersionEx).
#endif

#define IDC_LISTBOX  24500
#define MULTIPLESEL  (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)

namespace OwlExt {

using namespace owl;

// Default Column Style
static TStyle DefaultStyle(0);


//-- TColumn -------------------------------------------------------------------
struct TColumn {
  TColumn() {}
  TColumn(int id, const owl::tstring& text, int styleNo = 0)
  {Id = id; Text = text; StyleNo = styleNo;}

  bool operator == (const TColumn& col) const {return col.Id == Id;}
  bool operator < (const TColumn& col) const {return col.Id < Id;}

  int         Id;
  owl::tstring  Text;
  int         StyleNo;

};

typedef TObjectArray<TColumn> TColumns;


//-- TBrowseList ---------------------------------------------------------------

class TBrowseList : public TListBox {
public:
  TBrowseList(TWindow* parent, TModule* module);
protected:
  void EvHScroll(uint scrollCode, uint thumbPos, HWND hWndCtl);
  void EvRButtonDown(uint modKeys, const TPoint& point);
  void EvRButtonUp(uint modKeys, const TPoint& point);

private:

  DECLARE_RESPONSE_TABLE(TBrowseList);
};

DEFINE_RESPONSE_TABLE1(TBrowseList, TListBox)
EV_WM_HSCROLL,
EV_WM_RBUTTONDOWN,
END_RESPONSE_TABLE;

TBrowseList::TBrowseList(TWindow* parent, TModule* module)
:
TListBox(parent, IDC_LISTBOX, 0, 0, 0, 0, module)
{
  ModifyStyle(WS_BORDER|LBS_SORT,LBS_OWNERDRAWFIXED|LBS_NOINTEGRALHEIGHT);
}

void TBrowseList::EvHScroll(uint scrollCode, uint thumbPos, HWND hWndCtl)
{
  TListBox::EvHScroll(scrollCode, thumbPos, hWndCtl);
  TRect r = Parent->GetClientRect();
  r.bottom = r.top + TYPESAFE_DOWNCAST(Parent, TBrowse)->GetHeadersHeight();
  Parent->InvalidateRect(r);
}

void TBrowseList::EvRButtonDown(uint modKeys, const TPoint& point)
{
  TPoint p = MapWindowPoints(*Parent, point);
  Parent->SendMessage(WM_RBUTTONDOWN, modKeys, MAKELPARAM(p.x, p.y));
}

void TBrowseList::EvRButtonUp(uint modKeys, const TPoint& point)
{
  TPoint p = MapWindowPoints(*Parent, point);
  Parent->SendMessage(WM_RBUTTONUP, modKeys, MAKELPARAM(p.x, p.y));
}


//-- THeader -------------------------------------------------------------------

THeader::THeader()
{
  Id = Width = TextFormat = 0;
  Visible = Pressed = false;
  ExactWidth = true;
  Color = TColor(0, 0, 128);
  Rect.SetNull();
  CompareFunc = NULL;
  CompareMemFunc = NULL;
  SortOrder = Undefined;
}

THeader::THeader(int id, const owl::tstring& title, int width, bool visible,
         bool exactWidth)
{
  Id = id;
  Title = title;
  Width = width;
  Visible = visible;
  Pressed = false;
  ExactWidth = exactWidth;
  Color = TColor(0, 0, 128);
  TextFormat = DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX;
  Rect.SetNull();
  CompareFunc = NULL;
  CompareMemFunc = NULL;
  SortOrder = Undefined;
}


//-- TStyle -------------------------------------------------------------------

TStyle::TStyle()
{
  Id = 0;
  IconImage = NULL;
  IconMask = NULL;
}

TStyle::TStyle(int id)
{
  Id = id;
  TextColor = GetSysColor(COLOR_WINDOWTEXT);
  BkColor = GetSysColor(COLOR_WINDOW);
  HilightTextColor = GetSysColor(COLOR_HIGHLIGHTTEXT);
  HilightBkColor = GetSysColor(COLOR_HIGHLIGHT);
  TextAlignment = DT_SINGLELINE | DT_VCENTER | DT_LEFT | DT_NOPREFIX;
  IconImage = NULL;
  IconMask = NULL;
}

TStyle::~TStyle()
{
  delete IconImage;
  delete IconMask;
}

void TStyle::SetStyle(TDC& dc, UINT itemState)
{
  if (itemState & ODS_SELECTED){
    dc.SetBkColor(HilightBkColor);
    dc.SetTextColor(HilightTextColor);
  }
  else{
    dc.SetBkColor(BkColor);
    dc.SetTextColor(TextColor);
  }
}


//-- TBrowse -------------------------------------------------------------------

DEFINE_RESPONSE_TABLE1(TBrowse, TControl)
EV_LBN_DBLCLK(IDC_LISTBOX, EvDblClk),
EV_LBN_SELCHANGE(IDC_LISTBOX, EvSelChange),
EV_WM_LBUTTONDOWN,
EV_WM_LBUTTONUP,
EV_WM_MOUSEMOVE,
EV_WM_NCHITTEST,
EV_WM_RBUTTONDOWN,
EV_WM_RBUTTONUP,
EV_WM_SETFOCUS,
EV_WM_SIZE,
EV_WM_SYSCOLORCHANGE,
END_RESPONSE_TABLE;

TBrowse::TBrowse(TWindow* parent, int id, int x, int y, int w, int h, TModule* module)
:
TControl(parent, id, _T(""), x, y, w, h, module)
{
  ModifyStyle(0,WS_BORDER|WS_TABSTOP);
  Init();
}


TBrowse::TBrowse(TWindow* parent, int resourceId, TModule* module)
:
TControl(parent, resourceId, module)
{
  Init();
}

TBrowse::~TBrowse()
{
  delete NormalFont;
  delete BoldFont;
  delete Headers;
  delete Styles;
}

void TBrowse::AddColumn(int id, LPCTSTR title, int width, bool visible,
            TUnits units)
{
  switch (units){
case Chars:
  width = width < 0 ? static_cast<int>(_tcslen(title)) : width;
  width *= BoldCharWidth;
  break;
case Pixels:
  width = width < 0 ? 0 : width;
  break;
case Percent:
  width = width < 0 || width > 100 ? 0 : width;
  break;
  }

  Headers->Add(new THeader(id, title, width, visible, (units != Percent)));
}

int TBrowse::AddRow()
{
  int index = LB_ERR;

  if (GetNumCols() && (index = List->AddString(_T(""))) != LB_ERR)
  {
    TColumns *columns = new TColumns(GetNumCols(), 0, 1);
    THeadersIterator i(*Headers);
    while (i){
      columns->Add(TColumn(i.Current()->Id, _T(""), 0));
      i++;
    }
    List->SetItemData(index, reinterpret_cast<LPARAM>(columns));
  }

  return index;
}

void TBrowse::AddStyle(int id)
{
  Styles->Add(new TStyle(id));
}

int TBrowse::InsertRow(int index)
{
  if (GetNumCols() && (index = List->InsertString(_T(""),index)) != LB_ERR)
  {
    TColumns *columns = new TColumns(GetNumCols(), 0, 1);
    THeadersIterator i(*Headers);
    while (i){
      columns->Add(TColumn(i.Current()->Id, _T(""), 0));
      i++;
    }
    List->SetItemData(index, reinterpret_cast<LPARAM>(columns));
  }

  return index;
}

void TBrowse::CalculateHeadersSize()
{
  TRect rect(0, 0, 0, HeadersHeight);
  int Extent = 0, visibleCols = 0;
  int browseWidth = List->GetWindowAttr().W;

  THeadersIterator i(*Headers);
  while (i){
    THeader* header = i++;
    if (header->Visible)
      visibleCols++;
  }
  browseWidth -= (visibleCols * 5);

  i.Restart();
  while (i){
    THeader* header = i++;
    if (header->Visible){
      if (header->ExactWidth)
        rect.right = rect.left + 4 + header->Width;
      else
        rect.right = rect.left + 4 + (int)(header->Width / 100.0 * browseWidth);
      header->Rect = rect;
      Extent += rect.Width() - 1;
      rect.left = rect.right - 1;
    }
  }

  if (Extent > List->GetHorizontalExtent())
    List->SetHorizontalExtent(HorzExtent=Extent);
  else if (Extent <= List->GetClientRect().Width()){
    List->SetHorizontalExtent(HorzExtent=Extent);
    List->SetScrollPos(SB_HORZ, 0);
    List->ShowScrollBar(SB_HORZ, false);
  }
}

void TBrowse::ClearList()
{
  List->ClearList();
}

void TBrowse::DeleteItem(DELETEITEMSTRUCT & deleteInfo)
{
  delete (TColumns*)deleteInfo.itemData;
}

int TBrowse::DeleteRow(int index)
{
  return List->DeleteString(index);
}

void TBrowse::DrawHeaderRect(TDC& dc, TRect& rect, bool down)
{
  dc.SaveDC();
  dc.FrameRect(rect, TColor(TColor::Black));
  dc.SelectObject(TBrush(GetSysColor(down ? COLOR_BTNSHADOW : COLOR_BTNHIGHLIGHT)));
  dc.PatBlt(rect.left + 1, rect.top + 1, rect.Width() - 2, 1, PATCOPY);
  dc.PatBlt(rect.left + 1, rect.top + 1, 1, rect.Height() - 2, PATCOPY);
  if (!down){
    dc.SelectObject(TBrush(GetSysColor(COLOR_BTNSHADOW)));
    dc.PatBlt(rect.right - 2, rect.top + 1, 1, rect.Height() - 2, PATCOPY);
    dc.PatBlt(rect.left + 1, rect.bottom - 2, rect.Width() - 2, 1, PATCOPY);
  }
  dc.RestoreDC();
}

void TBrowse::DrawHeaderText(TDC& dc, THeader* header)
{
  TRect hdrRect = header->Rect.InflatedBy(-2, -2);
  if (header->TextFormat & DT_RIGHT)
    hdrRect.right -= 2;
  else
    hdrRect.left += 2;
  hdrRect.top += header->Pressed;
  hdrRect.left += 2 + header->Pressed;

  if (header->SortOrder != THeader::Undefined)
    hdrRect.right -= 10;

  dc.DrawText(header->Title.c_str(), -1, hdrRect, header->TextFormat);

  if (header->SortOrder != THeader::Undefined){
    hdrRect.left = hdrRect.right + 2;
    hdrRect.right += 10;

    TRegion savedClipRgn;
    dc.GetClipRgn(savedClipRgn);
    dc.IntersectClipRect(hdrRect);

    TPoint points[3];
    if (header->SortOrder == THeader::Ascending){
      points[0].x = hdrRect.left + 3;
      points[0].y = hdrRect.top + 3;
      points[1].x = hdrRect.left;
      points[1].y = hdrRect.bottom - 3 + header->Pressed;
      points[2].x = hdrRect.left + 6;
      points[2].y = hdrRect.bottom - 3 + header->Pressed;
    }
    else{
      points[0].x = hdrRect.left;
      points[0].y = hdrRect.top + 3;
      points[1].x = hdrRect.left + 6;
      points[1].y = hdrRect.top + 3;
      points[2].x = hdrRect.left + 3;
      points[2].y = hdrRect.bottom - 3 + header->Pressed;
    }

    dc.Polygon(points, 3);

    dc.SelectClipRgn(savedClipRgn);
  }
}

void TBrowse::DrawItem(DRAWITEMSTRUCT & drawInfo)
{
  if (drawInfo.CtlType == ODT_LISTBOX)
    switch (drawInfo.itemAction)
  {
    case ODA_DRAWENTIRE:
    case ODA_SELECT: {

      TDC dc(drawInfo.hDC);
      dc.SelectObject(*NormalFont);
      dc.SetBkColor(GetSysColor(drawInfo.itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));
      dc.SetTextColor(GetSysColor(drawInfo.itemState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
      dc.TextRect(drawInfo.rcItem);

      TColumns* cols = (TColumns*)drawInfo.itemData;
      THeadersIterator i(*Headers);
      while (i){
        THeader* header = i.Current();
        if (header->Visible){
          TRect rect = header->Rect.InflatedBy(-1, -1);
          rect.top = drawInfo.rcItem.top;
          rect.bottom = drawInfo.rcItem.bottom;
          const auto loc = cols->Find(TColumn{header->Id, _T(""), 0});
          if (loc != cols->NPOS)
          {
            TStyle colStyle((*cols)[loc].StyleNo);
            auto locStyle = Styles->Find(&colStyle);
            if (locStyle == Styles->NPOS)
            {
              TStyle headerStyle(header->StyleNo);
              locStyle = Styles->Find(&headerStyle);
            }

            TStyle* style;
            if (locStyle == Styles->NPOS)
              style = &DefaultStyle;
            else
              style = (*Styles)[locStyle];

            if (style->IconImage){
              int h, w = style->IconImage->Width();
              TPoint maskOrigin, imageOrigin(0, 0);
              TMemoryDC imageDC(dc);
              TRect destRect ;

              if (style->IconMask){
                TMemoryDC maskDC(dc);
                h = style->IconImage->Height();
                maskOrigin = TPoint(0, 0);
                TPoint topleft = rect.TopLeft();
                ++topleft.y;
                destRect = TRect(topleft, TSize(w, h));
                imageDC.SelectObject(*style->IconImage);
                maskDC.SelectObject(*style->IconMask);
                dc.TextRect(destRect,GetSysColor(drawInfo.itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));
                dc.SetBkColor(TColor::White);
                dc.SetTextColor(TColor::Black);
                dc.BitBlt(destRect, imageDC, imageOrigin, SRCINVERT);
                dc.BitBlt(destRect, maskDC, maskOrigin, SRCAND);
                dc.BitBlt(destRect, imageDC, imageOrigin, SRCINVERT);
                imageDC.RestoreObjects();
                maskDC.RestoreObjects();
              }
              else{
                h = style->IconImage->Height() / 2 ;
                maskOrigin = TPoint(0, h);
                TPoint topleft = rect.TopLeft();
                ++topleft.y;
                destRect = TRect(topleft, TSize(w, h));
                imageDC.SelectObject(*style->IconImage);
                dc.BitBlt(destRect, imageDC, imageOrigin, SRCINVERT);
                dc.BitBlt(destRect, imageDC, maskOrigin, SRCAND);
                dc.BitBlt(destRect, imageDC, imageOrigin, SRCINVERT);
                imageDC.RestoreObjects();
              }
              rect.left = destRect.right + 2;
              dc.SetBkColor(GetSysColor(drawInfo.itemState & ODS_SELECTED ? COLOR_HIGHLIGHT : COLOR_WINDOW));
              dc.SetTextColor(GetSysColor(drawInfo.itemState & ODS_SELECTED ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));
            }

            style->SetStyle(dc, drawInfo.itemState);

            rect.right += style->TextAlignment & DT_RIGHT ? -2 : 2;

            int length = static_cast<int>((*cols)[loc].Text.size());
            TSize size = dc.GetTextExtent((*cols)[loc].Text, length);
            if (size.cx >= rect.Width())
            {
              _TCHAR* longWord = new _TCHAR[length + 3];
              _tcscpy(longWord, (*cols)[loc].Text.c_str());
              _TCHAR* pch;
              for (pch = longWord + length - 1; pch >= longWord; --pch){
                _tcscpy(pch, _T("..."));
                size = dc.GetTextExtent(longWord, static_cast<int>(_tcslen(longWord)));
                if (size.cx < rect.Width())
                  break;
              }
              dc.DrawText(longWord, -1, rect, style->TextAlignment);
              delete[] longWord;
            }
            else
              dc.DrawText((*cols)[loc].Text.c_str(), -1, rect,
              style->TextAlignment);
          }
        }
        i++;
      }

      if (drawInfo.itemState & ODS_FOCUS)
        dc.DrawFocusRect(drawInfo.rcItem);

      break;

             }

    case ODA_FOCUS:
      {
        TDC dc(drawInfo.hDC);
        dc.DrawFocusRect(drawInfo.rcItem);
      }
  }
}

void TBrowse::DrawSizingLine(TPoint& point)
{
  TRect rect = List->GetClientRect();
  TWindowDC dc(*List);

  TPen pen(TColor::Black, 1, PS_DOT);
  dc.SetROP2(R2_XORPEN);
  dc.SelectObject(pen);

  dc.MoveTo(PrevSizingRect.TopLeft());
  dc.LineTo(PrevSizingRect.BottomRight());

  dc.MoveTo(point.x, rect.top);
  dc.LineTo(point.x, rect.bottom);

  PrevSizingRect = TRect(point.x, rect.top, point.x, rect.bottom);
}

void TBrowse::EvDblClk()
{
  if (Parent)
    Parent->SendNotification(GetId(), LBN_DBLCLK, GetHandle(), WM_COMMAND);
}

void TBrowse::EvLButtonDown(uint modKeys, const TPoint& point)
{
  bool onBorder;
    TPoint p(point);
  Capture = GetHeaderFromPoint(p, onBorder);

  if (Capture != NULL){
    SetCapture();
    if (onBorder){
      ReSizing = true;
      StartDrawingSizingLine(p);
    }
    else if ((Capture->CompareFunc!=0) || (Capture->CompareMemFunc!=0)){
      ReSizing = false;
      Capture->Pressed = true;
      InvalidateRect(Capture->Rect.Offset(-ViewportOrgX, 0));
    }
    else{
      Capture = NULL;
      ReleaseCapture();
    }
  }
  TControl::EvLButtonDown(modKeys, point);
}

void TBrowse::EvLButtonUp(uint modKeys, const TPoint& point)
{
  if (Capture){
    if (ReSizing){
            TPoint p(point);
      StopDrawingSizingLine(p);
      TRect rect = Capture->Rect.OffsetBy(-ViewportOrgX, 0);
      if (p.x < rect.right - 1 || p.x > rect.right + 1){
        if (p.x < rect.left)
          p.x = rect.left;
        if (Capture->ExactWidth)
          Capture->Width = p.x - rect.left - 4;
        else
          Capture->Width = (int)((p.x - rect.left - 4) * 100.0 / List->GetWindowAttr().W);
        CalculateHeadersSize();
        Invalidate();
      }
    }
    else if (Capture->Pressed){
      Capture->SortOrder = (modKeys & MK_CONTROL) ? THeader::Descending : THeader::Ascending;
      SetCursor(0, IDC_WAIT);
      SortItems(Capture->Id, (Capture->SortOrder == THeader::Ascending));
      Capture->Pressed = false;
    }
    Capture = NULL;
    ReSizing = false;
    SetCursor(0, IDC_ARROW);
    ReleaseCapture();
  }
  TControl::EvLButtonUp(modKeys, point);
}

void TBrowse::EvMouseMove(uint modKeys, const TPoint& point)
{
  if (!Capture){
    bool onBorder;
    TPoint p(point);
    if (GetHeaderFromPoint(p, onBorder) != NULL && onBorder)
      SetCursor(0, IDC_SIZEWE);
    else
      SetCursor(0, IDC_ARROW);
  }
  else
    if (ReSizing){
      TPoint p(point);
      DrawSizingLine(p);
        }
    else{
      bool onBorder;
       TPoint p(point);
      THeader* header = GetHeaderFromPoint(p, onBorder);
      if (header == NULL || header->Id != Capture->Id){
        if (Capture->Pressed){
          Capture->Pressed = false;
          InvalidateRect(Capture->Rect.Offset(-ViewportOrgX, 0));
        }
      }
      else if (!Capture->Pressed){
        Capture->Pressed = true;
        InvalidateRect(Capture->Rect.Offset(-ViewportOrgX, 0));
      }
    }

    TControl::EvMouseMove(modKeys, point);
}

uint TBrowse::EvNCHitTest(const TPoint& /*point*/)
{
  return HTCLIENT;
}

void TBrowse::EvRButtonDown(uint modKeys, const TPoint& point_)
{
  if (Parent){
        TPoint point = MapWindowPoints(*Parent, point_);
    Parent->SendMessage(WM_RBUTTONDOWN, modKeys, MAKELPARAM(point.x, point.y));
  }
}

void TBrowse::EvRButtonUp(uint modKeys, const TPoint& point_)
{
  if (Parent){
        TPoint point = MapWindowPoints(*Parent, point_);
    Parent->SendMessage(WM_RBUTTONUP, modKeys, MAKELPARAM(point.x, point.y));
  }
}

void TBrowse::EvSelChange()
{
  if (Parent)
    Parent->SendNotification(GetId(), LBN_SELCHANGE, GetHandle(), WM_COMMAND);
}

void TBrowse::EvSetFocus(HWND hWndLostFocus)
{
  TControl::EvSetFocus(hWndLostFocus);
  List->SetFocus();
}

void TBrowse::EvSize(uint sizeType, const TSize& size)
{
  TControl::EvSize(sizeType, size);

  TRect rect = GetClientRect();
  rect.top += HeadersHeight;
  List->MoveWindow(rect);

  Invalidate();
}

void TBrowse::EvSysColorChange()
{
  DefaultStyle.TextColor = GetSysColor(COLOR_WINDOWTEXT);
  DefaultStyle.BkColor = GetSysColor(COLOR_WINDOW);
  DefaultStyle.HilightTextColor = GetSysColor(COLOR_HIGHLIGHTTEXT);
  DefaultStyle.HilightBkColor = GetSysColor(COLOR_HIGHLIGHT);
  TControl::EvSysColorChange();
}

bool TBrowse::EnableWindow(bool enable)
{
  TControl::EnableWindow(enable);
  return List->EnableWindow(enable);
}

bool TBrowse::GetColumnText(int id, owl::tstring& text, int index)
{
  bool bOk = false;

  LPARAM itemData = List->GetItemData(index);
  if (itemData != LB_ERR){
    TColumns* cols = (TColumns*)itemData;
    const auto loc = cols->Find(TColumn{id, _T(""), 0});
    if (loc != cols->NPOS)
    {
      text = (*cols)[loc].Text;
      bOk = true;
    }
  }

  return bOk;
}

bool TBrowse::GetColumnWidth(int id, int &width, bool& exactWidth)
{
  THeader idHeader(id, _T(""), 0, true, true);
  const auto loc = Headers->Find(&idHeader);
  if (loc != Headers->NPOS)
  {
    width = (*Headers)[loc]->Width;
    exactWidth = (*Headers)[loc]->ExactWidth;
    return true;
  }
  return false;
}

int TBrowse::GetCount() const
{
  return List->GetCount();
}

bool TBrowse::GetHeaderColor(int id, TColor& color)
{
  THeader idHeader(id, _T(""), 0, true, true);
  const auto loc = Headers->Find(&idHeader);
  if (loc != Headers->NPOS)
  {
    color = (*Headers)[loc]->Color;
    return true;
  }
  return false;
}

THeader* TBrowse::GetHeaderFromPoint(TPoint& point, bool& onBorder)
{
  THeadersIterator i(*Headers);
  while (i){
    THeader* header = i.Current();
    if (header->Visible){
      TRect rect = header->Rect.OffsetBy(-ViewportOrgX, 0);
      if (point.x >= rect.left && point.x < rect.right - 1 &&
        point.y >= rect.top && point.y <= rect.bottom){
          onBorder = false;
          return header;
      }
      else if (point.x >= rect.right - 1 && point.x <= rect.right + 1  &&
        point.y >= rect.top && point.y <= rect.bottom){
          onBorder = true;
          return header;
      }
    }
    i++;
  }
  return 0;
}

int TBrowse::GetNumCols()
{
  return Headers->GetItemsInContainer();
}

bool TBrowse::GetSel(int index) const
{
  return List->GetSel(index);
}

int TBrowse::GetSelCount() const
{
  return List->GetSelCount();
}

int TBrowse::GetSelIndex() const
{
  return List->GetSelIndex();
}

int TBrowse::GetSelIndexes(int* indexes, int maxCount) const
{
  return List->GetSelIndexes(indexes, maxCount);
}

int TBrowse::GetTopIndex() const
{
  return List->GetTopIndex();
}

void TBrowse::Init()
{
  List = 0;
  ReSizing = false;
  Capture = NULL;

  NormalFont = new TFont(_T("MS Shell Dlg"), -11);

  LOGFONT lf = NormalFont->GetObject();
  lf.lfWeight = FW_BOLD;
  BoldFont = new TFont(lf);

  {
    TRect rc;
    TScreenDC dc;
    dc.SelectObject(*BoldFont);
    dc.DrawText(_T("Abc"), -1, rc, DT_CENTER | DT_CALCRECT);
    HeadersHeight = rc.Height() + 4;

    TEXTMETRIC tm;
    dc.GetTextMetrics(tm);
    BoldCharWidth = (tm.tmAveCharWidth + tm.tmMaxCharWidth) / 2;
    RowsHeight = tm.tmHeight + tm.tmExternalLeading;
  }
  Headers = new THeaders(2, 0, 1);
  Styles = new TStyles(1, 0, 1);

  SetBkgndColor(GetSysColor(COLOR_BTNFACE));

  EnableTransfer();
}

bool TBrowse::IsColumnVisible(int id)
{
  THeader idHeader(id, _T(""), 0, true, true);
  const auto loc = Headers->Find(&idHeader);
  if (loc != Headers->NPOS)
  {
    return (*Headers)[loc]->Visible;
  }
  return false;
}

void TBrowse::MeasureItem(MEASUREITEMSTRUCT & measureInfo)
{
  if (measureInfo.CtlType == ODT_LISTBOX)
    measureInfo.itemHeight = RowsHeight;
}

bool TBrowse::ModifyColumnText(int id, LPCTSTR text, int index)
{
  if (SetColumnText(id, text, index)){
    List->Invalidate(false);
    return true;
  }
  return false;
}

void TBrowse::Paint(TDC& dc, bool /*erase*/, TRect& rect)
{
  CalculateHeadersSize();

  ViewportOrgX = (int)((float(HorzExtent) - List->GetClientRect().Width()) / 100 * List->GetScrollPos(SB_HORZ));
  TPoint offset(-ViewportOrgX, 0);
  dc.SetViewportOrg(offset);
  rect -= offset;

  dc.SelectObject(*BoldFont);
  dc.SetBkMode(TRANSPARENT);

  TRect thisRect(GetClientRect());
  thisRect.bottom = thisRect.top + HeadersHeight;
  dc.SelectClipRgn(thisRect);

  THeadersIterator i(*Headers);
  while (i){
    THeader* header = i.Current();
    if (header->Visible && header->Rect.right > rect.left){
      if (header->Rect.left > rect.right)
        break;
      dc.SetTextColor(header->Color);
      DrawHeaderRect(dc, header->Rect, header->Pressed);
      DrawHeaderText(dc, header);
      /*
      TRect r = header->Rect.InflatedBy(-2, -2);
      if (header->TextFormat & DT_RIGHT)
      r.right -= 2;
      else
      r.left += 2;
      r.top += header->Pressed;
      r.left += 2 + header->Pressed;
      dc.DrawText(header->Title.c_str(), -1, r, header->TextFormat);
      */
    }
    i++;
  }
}

void TBrowse::Reset()
{
  ClearList();
  Headers->Flush();
  Styles->Flush();
  Invalidate();
}

bool TBrowse::SetColumnStyle(int id, int styleNo, int index)
{
  bool bOk = false;

  if (GetCount() < 1)
    return bOk;

  if (index == -1)
    index = GetCount() - 1;

  LPARAM itemData = List->GetItemData(index);
  if (itemData != LB_ERR){
    TColumns *cols = (TColumns*)itemData;
    const auto loc = cols->Find(TColumn{id, _T(""), 0});
    if (loc != cols->NPOS)
    {
      (*cols)[loc].StyleNo = styleNo;
      bOk = true;
    }
  }

  return bOk;
}

bool TBrowse::SetColumnText(int id, LPCTSTR text, int index)
{
  bool bOk = false;

  if (GetCount() < 1)
    return bOk;

  if (index == -1)
    index = GetCount() - 1;

  LPARAM itemData = List->GetItemData(index);
  if (itemData != LB_ERR){
    TColumns *cols = (TColumns*)itemData;
    const auto loc = cols->Find(TColumn{id, _T(""), 0});
    if (loc != cols->NPOS)
    {
      (*cols)[loc].Text = text;
      bOk = true;
    }
  }

  return bOk;
}

bool TBrowse::SetColumnText(int id, LPCTSTR text, int styleNo, int index)
{
  if (SetColumnText(id, text, index))
    return SetColumnStyle(id, styleNo, index);
  return false;
}

bool TBrowse::SetColumnWidth(int id, int width, TUnits units)
{
  THeader idHeader(id, _T(""), 0, true, true);
  const auto loc = Headers->Find(&idHeader);
  if (loc != Headers->NPOS)
  {

    switch (units){
case Chars:
  width = width < 0 ? static_cast<int>((*Headers)[loc]->Title.size()) : width;
  width *= BoldCharWidth;
  break;
case Pixels:
  width = width < 0 ? 0 : width;
  break;
case Percent:
  width = width < 0 || width > 100 ? 0 : width;
  break;
    }

    (*Headers)[loc]->Width = width;
    (*Headers)[loc]->ExactWidth = (units != Percent);

    if (GetHandle())
      Invalidate();

    return true;
  }

  return false;
}

bool TBrowse::SetCompareItemProc(int colId, TCompareFunc compareFunc)
{
  THeader colIdHeader(colId, _T(""), 0, true, true);
  const auto loc = Headers->Find(&colIdHeader);
  if (loc != Headers->NPOS)
  {
    (*Headers)[loc]->CompareFunc = compareFunc;
    return true;
  }
  return false;
}

bool TBrowse::SetCompareItemProc(int colId, TCompareMemFunc compareMemFunc)
{
  THeader colIdHeader(colId, _T(""), 0, true, true);
  const auto loc = Headers->Find(&colIdHeader);
  if (loc != Headers->NPOS)
  {
    (*Headers)[loc]->CompareMemFunc = compareMemFunc;
    return true;
  }
  return false;
}

bool TBrowse::SetDefaultStyle(int id, int styleNo)
{
  THeader idHeader(id, _T(""), 0, true, true);
  const auto loc = Headers->Find(&idHeader);
  if (loc != Headers->NPOS)
  {
    (*Headers)[loc]->StyleNo = styleNo;
    if (GetHandle())
      Invalidate();
    return true;
  }
  return false;
}

bool TBrowse::SetHeaderColor(int id, TColor color)
{
  THeader idHeader(id, _T(""), 0, true, true);
  const auto loc = Headers->Find(&idHeader);
  if (loc != Headers->NPOS)
  {
    (*Headers)[loc]->Color = color;
    if (GetHandle())
      Invalidate();
    return true;
  }
  return false;
}

bool TBrowse::SetHeaderTextFormat(int id, uint16 textFormat)
{
  THeader idHeader(id, _T(""), 0, true, true);
  const auto loc = Headers->Find(&idHeader);
  if (loc != Headers->NPOS)
  {
    (*Headers)[loc]->TextFormat = textFormat;
    if (GetHandle())
      Invalidate();
    return true;
  }
  return false;
}

void TBrowse::SetHeadersHeight(int height)
{
  HeadersHeight = height;
  if (GetHandle())
    Invalidate();
}

int TBrowse::SetSel(int index, bool select)
{
  return List->SetSel(index, select);
}

int TBrowse::SetSelIndex(int index)
{
  return List->SetSelIndex(index);
}

bool TBrowse::SetStyleColor(int id, TColor textColor, TColor bkColor)
{
  TStyle idStyle(id);
  const auto loc = Styles->Find(&idStyle);
  if (loc != Styles->NPOS)
  {
    (*Styles)[loc]->TextColor = textColor;
    (*Styles)[loc]->BkColor = bkColor;
    if (GetHandle())
      Invalidate();
    return true;
  }
  return false;
}


bool TBrowse::SetStyleSelectedColor(int id, TColor textColor, TColor bkColor)
{
  TStyle idStyle(id);
  const auto loc = Styles->Find(&idStyle);
  if (loc != Styles->NPOS)
  {
    (*Styles)[loc]->HilightTextColor = textColor;
    (*Styles)[loc]->HilightBkColor = bkColor;
    if (GetHandle())
      Invalidate();
    return true;
  }
  return false;
}

// Use a image from a image/mask pair
bool TBrowse::SetStyleImage(int id, TDib& iconImageAndMask)
{
  TStyle idStyle(id);
  const auto loc = Styles->Find(&idStyle);
  if (loc != Styles->NPOS)
  {
    TStyle* Style = (*Styles)[loc];

    if (Style->IconImage)
      delete Style->IconImage ;
    Style->IconImage = new TBitmap(iconImageAndMask);

    delete Style->IconMask;
    Style->IconMask = NULL;

    if (List){
      if (Style->IconImage->Height() + 2 > List->GetItemHeight(0)){
        List->SetItemHeight(0, Style->IconImage->Height() / 2 + 2);
        List->Invalidate();
      }
    }
    else {
      if (Style->IconImage->Height() + 2 > RowsHeight)
        RowsHeight = Style->IconImage->Height() / 2 + 2;
    }

    if (GetHandle())
      Invalidate();
    return true;
  }
  return false;
}

// Create a mask from a given image and a transparent colour
bool TBrowse::SetStyleImage(int id, TDib& iconImage, TColor& faceColor)
{
  TStyle idStyle(id);
  const auto loc = Styles->Find(&idStyle);
  if (loc != Styles->NPOS)
  {
    TStyle* Style = (*Styles)[loc];

    delete Style->IconImage;
    Style->IconImage = new TBitmap(iconImage);

    delete Style->IconMask;

    TMemoryDC imageDC;
    imageDC.SelectObject(TBitmap(iconImage));
    imageDC.SetBkColor(faceColor);
    TMemoryDC maskDC;
    TSize maskSize = TSize(iconImage.Width(), iconImage.Height());
    TBitmap *maskBitmap = new TBitmap(maskSize.cx, maskSize.cy, 1, 1, 0);
    maskDC.SelectObject(*maskBitmap);
    maskDC.PatBlt(0, 0, maskSize.cx, maskSize.cy, WHITENESS);
    maskDC.BitBlt(0, 0, maskSize.cx, maskSize.cy, imageDC, 0, 0, SRCCOPY);
    maskDC.RestoreBitmap();
    imageDC.RestoreBitmap();

    Style->IconMask = maskBitmap ;

    if (List) {
      if (Style->IconImage->Height() + 2 > List->GetItemHeight(0)) {
        List->SetItemHeight(0,Style->IconImage->Height() + 2);
        List->Invalidate();
      }
    }
    else {
      if (Style->IconImage->Height() + 2 > RowsHeight)
        RowsHeight = Style->IconImage->Height() + 2;
    }

    if (GetHandle())
      Invalidate();
    return true;
  }
  return false;
}

bool TBrowse::SetStyleTextAlignment(int id, uint16 uiTextAlignment)
{
  TStyle idStyle(id);
  const auto loc = Styles->Find(&idStyle);
  if (loc != Styles->NPOS)
  {
    (*Styles)[loc]->TextAlignment = uiTextAlignment;

    if (GetHandle())
      Invalidate();
    return true;
  }
  return false;
}

int TBrowse::SetTopIndex(int index)
{
  return List->SetTopIndex(index);
}

void TBrowse::SetupWindow()
{
  List = new TBrowseList(this, GetModule());

  if (Attr.Style & TBS_MULTIPLESEL)
    List->GetWindowAttr().Style |= LBS_MULTIPLESEL;
  if (Attr.Style & TBS_EXTENDEDSEL)
    List->GetWindowAttr().Style |= LBS_EXTENDEDSEL;

  TControl::SetupWindow();

  ClearFlag(wfPredefinedClass);

  TRect rect = GetClientRect();
  rect.top += HeadersHeight;
  List->MoveWindow(rect);
}

// Performs an exchange sort
void TBrowse::SortItems(int colId, bool ascending)
{
  int curRow, minRow, nextRow;

  int rowCount = GetCount();
  if (rowCount < 2){
    if (Capture)
      InvalidateRect(Capture->Rect.Offset(-ViewportOrgX, 0));
    return;
  }

  THeader colIdHeader(colId, _T(""), 0, true, true);
  const auto loc = Headers->Find(&colIdHeader);
  if (loc == Headers->NPOS)
  {
    if (Capture)
      InvalidateRect(Capture->Rect.Offset(-ViewportOrgX, 0));
    return;
  }

  TCompareFunc func = (*Headers)[loc]->CompareFunc;
  TCompareMemFunc memFunc = (*Headers)[loc]->CompareMemFunc;
  if (func == NULL && memFunc == NULL){
    if (Capture)
      InvalidateRect(Capture->Rect.Offset(-ViewportOrgX, 0));
    return;
  }

  for (curRow = 0; curRow < rowCount; curRow++){
    minRow = curRow;
    for (nextRow = curRow; nextRow < rowCount; nextRow++){
      owl::tstring strNextRow, strMinRow;
      GetColumnText(colId, strNextRow, nextRow);
      GetColumnText(colId, strMinRow, minRow);

      if (func){
        bool greater = func(colId, strNextRow.c_str(), strMinRow.c_str());
        if ((greater && ascending) || (!greater && !ascending))
          minRow = nextRow;
      }
      else if (memFunc){
        bool greater = (this->*memFunc)(colId, strNextRow.c_str(), strMinRow.c_str());
        if ((greater && ascending) || (!greater && !ascending))
          minRow = nextRow;
      }
    }

    if (minRow > curRow){
      LPARAM tmp = List->GetItemData(curRow);
      List->SetItemData(curRow, List->GetItemData(minRow));
      List->SetItemData(minRow, tmp);
    }
  }

  if (GetHandle())
    Invalidate();
}

bool TBrowse::ShowColumn(int id, int visible)
{
  THeader idHeader(id, _T(""), 0, true, true);
  const auto loc = Headers->Find(&idHeader);
  if (loc != Headers->NPOS)
  {
    (*Headers)[loc]->Visible = visible;
    if (GetHandle())
      Invalidate();
    return true;
  }
  return false;
}

void TBrowse::StartDrawingSizingLine(TPoint& point)
{
  TRect rect = List->GetClientRect();
  TWindowDC dc(*List);

  TPen pen(TColor::Black, 1, PS_DOT);
  dc.SetROP2(R2_XORPEN);
  dc.SelectObject(pen);

  dc.MoveTo(point.x, rect.top);
  dc.LineTo(point.x, rect.bottom);

  PrevSizingRect = TRect(point.x, rect.top, point.x, rect.bottom);
}

void TBrowse::StopDrawingSizingLine(TPoint& /*point*/)
{
  TRect rect = List->GetClientRect();
  TWindowDC dc(*List);

  TPen pen(TColor::Black, 1, PS_DOT);
  dc.SetROP2(R2_XORPEN);
  dc.SelectObject(pen);

  dc.MoveTo(PrevSizingRect.TopLeft());
  dc.LineTo(PrevSizingRect.BottomRight());
}

uint TBrowse::Transfer(void *buffer, TTransferDirection direction)
{
  if (!buffer && direction != tdSizeData) return 0;
  long style = List->GetWindowLong(GWL_STYLE);
  TBrowseData* browseData = (TBrowseData*)buffer;

  if (direction == tdGetData){
    browseData->Clear();
    for (int i = 0; i < GetCount(); i++){
      TColumns* src = (TColumns*)List->GetItemData(i);
      browseData->AddRow();
      for (int j = 0; j < (int)src->GetItemsInContainer(); j++){
        browseData->SetColumnText((*src)[j].Id, (*src)[j].Text.c_str());
        browseData->SetColumnStyle((*src)[j].Id, (*src)[j].StyleNo);
      }
    }
    browseData->ResetSelections();
    if (!(style & MULTIPLESEL))
      browseData->Select(GetSelIndex());
    else {
      int selCount = GetSelCount();
      if (selCount > 0){
        int* selections = new int[selCount];
        GetSelIndexes(selections, selCount);
        for (int selIndex = 0; selIndex < selCount; selIndex++)
          browseData->Select(selections[selIndex]);
        delete[] selections;
      }
    }
  }
  else if (direction == tdSetData)
  {
    ClearList();
    const int noSelection = -1;
    int selCount = browseData->GetSelCount();
    int selIndex = noSelection;
    for (int i = 0; i < (int)browseData->GetItemDatas().GetItemsInContainer(); i++){
      int index = AddRow();
      TColumns* cols = (TColumns*)browseData->GetItemDatas()[i];
      for (int j = 0; j < (int)cols->GetItemsInContainer(); j++){
        SetColumnText((*cols)[j].Id, (*cols)[j].Text.c_str(), index);
        SetColumnStyle((*cols)[j].Id, (*cols)[j].StyleNo, index);
      }
      if (style & MULTIPLESEL){
        for (int k = 0; k < selCount; k++)
          if (browseData->GetSelIndices()[k] == i){
            SetSel(index, true);
            break;
          }
      }
      else {
        if (browseData->GetSelIndices()[0] == i)
          selIndex = index;
        else
          if (selIndex != noSelection && index <= selIndex)
            selIndex++;
      }
    }
    if (selIndex != noSelection && !(style & MULTIPLESEL))
      SetSelIndex(selIndex);
  }

  return sizeof(TBrowseData);
}


//-- TBrowseData ---------------------------------------------------------------

TBrowseData::TBrowseData()
:
ItemDatas(10, 0, 10),
SelIndices(10, 0, 10)
{
  NumCols = 0;
}

TBrowseData::TBrowseData(int numCols)
:
ItemDatas(10, 0, 10), SelIndices(10, 0, 10)
{
  NumCols = numCols;
}

void TBrowseData::AddRow(bool isSelected)
{
  TColumns *columns = new TColumns(NumCols, 0, 1);
  ItemDatas.Add((uint32*)columns);
  if (isSelected)
    Select(ItemDatas.GetItemsInContainer()-1);
}

void TBrowseData::SetColumnText(int id, LPCTSTR text)
{
  TColumns *cols = (TColumns*)ItemDatas[ItemDatas.GetItemsInContainer()-1];
  const auto loc = cols->Find(TColumn{id, _T(""), 0});
  if (loc != cols->NPOS)
    (*cols)[loc].Text = text;
  else
    cols->Add(TColumn(id, text, 0));
}

void TBrowseData::SetColumnStyle(int id, int styleNo)
{
  TColumns *cols = (TColumns*)ItemDatas[ItemDatas.GetItemsInContainer()-1];
  const auto loc = cols->Find(TColumn{id, _T(""), 0});
  if (loc != cols->NPOS)
    (*cols)[loc].StyleNo = styleNo;
  else
    cols->Add(TColumn(id, _T(""), styleNo));
}

void TBrowseData::Select(int index)
{
  if (index != LB_ERR)
    SelIndices.Add(index);
}


//-- TRowArrayIterator----------------------------------------------------------

TRowArrayIterator::TRowArrayIterator(TRowArray& array)
:
TPtrArrayIterator<uint32*,TIPtrArray<uint32*> >(array)
{
}

bool TRowArrayIterator::GetColumnText(int id, owl::tstring& text)
{
  TColumns* cols = (TColumns*)Current();
  const auto loc = cols->Find(TColumn{id, _T(""), 0});
  if (loc != cols->NPOS)
  {
    text = (*cols)[loc].Text;
    return true;
  }
  return false;
}

} // OwlExt namespace
//==============================================================================
