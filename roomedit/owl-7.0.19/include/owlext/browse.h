//------------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// BROWSE.H
//
// Header file for TBrowse Class
//
// Original code by Daniel BERMAN (dberman@inge-com.fr)
//
//------------------------------------------------------------------------------
#if !defined(__OWLEXT_BROWSE_H)
#define __OWLEXT_BROWSE_H

// TBrowse styles
#define TBS_MULTIPLESEL    0x0040L
#define TBS_EXTENDEDSEL    0x0080L

#ifndef RC_INVOKED

// Grab the core OWLEXT file we need
//
#if !defined (__OWLEXT_CORE_H) && !defined (__OWLEXT_ALL_H)
# include <owlext/core.h>
#endif

#include <owl/color.h>
#include <owl/control.h>
#include <owl/listbox.h>
#include <owl/geometry.h>
#include <owl/gdiobjec.h>

namespace OwlExt {

class TBrowse;
class TBrowseList;

// Those functions should return true if str1 < str2
typedef bool (*TCompareFunc)(int colId, LPCTSTR str1, LPCTSTR str2);
typedef bool (TBrowse::*TCompareMemFunc)(int colId, LPCTSTR str1, LPCTSTR str2);

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>


class OWLEXTCLASS THeader {
  public:
    enum TSortOrder {Undefined, Ascending, Descending};

    THeader();
    THeader(int id, const owl::tstring& title, int width, bool visible, bool exactWidth);

    bool operator == (const THeader& header) const {return header.Id == Id;}

    int           Id;
    owl::tstring    Title;
    bool           Visible;
    int           Width;
    bool           ExactWidth;
    owl::TRect   Rect;
    owl::uint16         TextFormat;
    owl::TColor   Color;
    int           StyleNo;
    bool           Pressed;
    TCompareFunc   CompareFunc;
    TSortOrder    SortOrder;
    TCompareMemFunc CompareMemFunc;
};
typedef owl::TIPtrArray<THeader*> THeaders;
typedef owl::TIPtrArray<THeader*>::Iterator THeadersIterator;


class OWLEXTCLASS TStyle {
  public:
    TStyle();
    TStyle(int id);
    ~TStyle();

    TStyle(const TStyle& style);
    void operator = (const TStyle& style);

    bool operator == (const TStyle& style) const { return style.Id == Id;}

    void SetStyle(owl::TDC &dc, UINT itemState);

    int             Id;
    owl::TColor     TextColor;
    owl::TColor     BkColor;
    owl::TColor     HilightTextColor;
    owl::TColor     HilightBkColor;
    owl::TBitmap*  IconImage;
    owl::TBitmap*  IconMask;
    owl::uint16           TextAlignment;
};
typedef owl::TIPtrArray<TStyle*> TStyles;
typedef owl::TIPtrArray<TStyle*>::Iterator TStylesIterator;


class OWLEXTCLASS TBrowse : public owl::TControl{
  public:
    enum TUnits {Chars, Pixels, Percent};

    TBrowse(owl::TWindow* parent, int id, int x, int y, int w, int h, owl::TModule* module = 0);
    TBrowse(owl::TWindow* parent, int resourceId, owl::TModule* module = 0);
    virtual ~TBrowse();

    virtual void  AddColumn(int id, LPCTSTR title, int width = -1,
                             bool visible = true, TUnits units = Chars);
    virtual int    AddRow();
    virtual void  AddStyle(int id);
    virtual int    InsertRow(int index);
    virtual void  ClearList();
    virtual int    DeleteRow(int index);
    auto EnableWindow(bool enable) -> bool override;
    bool          GetColumnText(int id, owl::tstring& text, int index);
    bool          GetColumnWidth(int id, int& width, bool& exactWidth);
    virtual int    GetCount() const;
    bool          GetHeaderColor(int id, owl::TColor& color);
    int            GetHeadersHeight() {return HeadersHeight;}
    int            GetNumCols();
    bool          GetSel(int index) const;
    int            GetSelCount() const;
    int            GetSelIndex() const;
    int            GetSelIndexes(int* indexes, int maxCount) const;
    int            GetTopIndex() const;
    bool          IsColumnVisible(int id);
    virtual bool  ModifyColumnText(int id, LPCTSTR text, int index);
    void          Reset();
    bool          SetColumnStyle(int id, int styleNo, int index = -1);
    virtual bool  SetColumnText(int id, LPCTSTR text, int index = -1);
    virtual bool  SetColumnText(int id, LPCTSTR text, int styleNo, int index);
    bool          SetColumnWidth(int id, int width, TUnits units = Chars);
    bool          SetCompareItemProc(int colId, TCompareFunc compareFunc);
    bool          SetCompareItemProc(int colId, TCompareMemFunc compareMemFunc);
    bool          SetDefaultStyle(int id, int styleNo);
    bool          SetHeaderColor(int id, owl::TColor color);
    bool          SetHeaderTextFormat(int id, owl::uint16 uiTextFormat);
    void          SetHeadersHeight(int height);
    int            SetSel(int index, bool select);
    virtual int    SetSelIndex(int index);
    bool          SetStyleColor(int id, owl::TColor textColor, owl::TColor bkColor);
    bool          SetStyleSelectedColor(int id, owl::TColor textColor, owl::TColor bkColor);
    bool          SetStyleImage(int id, owl::TDib& iconImageAndMask );
    bool          SetStyleImage(int id, owl::TDib& iconImage, owl::TColor& faceColor);
    bool          SetStyleTextAlignment(int id, owl::uint16 uiTextAlignment);
    int            SetTopIndex(int index);
    bool          ShowColumn(int id, int visible = true);
    virtual void  SortItems(int colId, bool ascending);
    auto Transfer(void *buffer, owl::TTransferDirection) -> owl::uint override;

  protected:

    virtual void    CalculateHeadersSize();
    void DeleteItem(DELETEITEMSTRUCT&) override;
    virtual void    DrawHeaderRect(owl::TDC& dc, owl::TRect& rect, bool down);
    virtual void    DrawHeaderText(owl::TDC& dc, THeader* header);
    void DrawItem(DRAWITEMSTRUCT&) override;
    virtual void    DrawSizingLine(owl::TPoint& point);
    void            EvDblClk();
    void            EvLButtonDown(owl::uint modKeys, const owl::TPoint& point);
    void            EvLButtonUp(owl::uint modKeys, const owl::TPoint& point);
    void            EvMouseMove(owl::uint modKeys, const owl::TPoint& point);
    owl::uint        EvNCHitTest(const owl::TPoint& point);
    void            EvRButtonDown(owl::uint modKeys, const owl::TPoint& point);
    void            EvRButtonUp(owl::uint modKeys, const owl::TPoint& point);
    void            EvSelChange();
    void            EvSetFocus(HWND hWndLostFocus);
    void            EvSize(owl::uint sizeType, const owl::TSize& size);
    void            EvSysColorChange();
    virtual THeader*  GetHeaderFromPoint(owl::TPoint& point, bool& onBorder);
    void MeasureItem(MEASUREITEMSTRUCT&) override;
    void Paint(owl::TDC&, bool erase, owl::TRect&) override;
    void SetupWindow() override;
    virtual void    StartDrawingSizingLine(owl::TPoint& point);
    virtual void    StopDrawingSizingLine(owl::TPoint& point);

  private:

    owl::TFont*   NormalFont;
    owl::TFont*   BoldFont;
    int           HeadersHeight;
    int           BoldCharWidth;
    int           HorzExtent;
    int           ViewportOrgX;
    int           RowsHeight;

    THeader*       Capture;
    bool           ReSizing;
    owl::TRect   PrevSizingRect;

    THeaders*     Headers;
    TBrowseList*   List;
    TStyles*       Styles;

    void Init();

    DECLARE_RESPONSE_TABLE(TBrowse);
};

typedef owl::TIPtrArray<owl::uint32*> TRowArray;

class OWLEXTCLASS TRowArrayIterator : public owl::TPtrArrayIterator<owl::uint32*,owl::TIPtrArray<owl::uint32*> >{
  public:
    TRowArrayIterator(TRowArray& array);
    bool GetColumnText(int id, owl::tstring& text);
};


class OWLEXTCLASS TBrowseData {
  public:
    TBrowseData();
    TBrowseData(int numCols);

    void        AddRow(bool isSelected = false);
    void        Clear() {ItemDatas.Flush(); ResetSelections();}
    int          GetNumRows() {return ItemDatas.GetItemsInContainer();}
    TRowArray&  GetItemDatas() {return ItemDatas;}
    int          GetNumCols(int numCols) {return NumCols;}
    int          GetSelCount() const {return SelIndices.GetItemsInContainer();}
    owl::TIntArray&  GetSelIndices() {return SelIndices;}
    void        ResetSelections() {SelIndices.Flush();}
    void        Select(int index);
    void        SetColumnText(int id, LPCTSTR text);
    void        SetColumnStyle(int id, int StyleNo);
    void        SetNumCols(int numCols) {NumCols = numCols;}


  protected:

    int       NumCols;
    TRowArray ItemDatas;
    owl::TIntArray SelIndices;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif  // RC_INVOKED

#endif  // __OWLEXT_BROWSE_H
