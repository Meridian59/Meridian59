//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998, by Yura Bidus, All Rights Reserved
//
/// \file
/// Definitions of classes TDrawItemProxy, TDrawItem,TButtonTextDrawItem,
/// TDrawMenuItem, TDrawMenuItemProxy. Owner draw family.
// 
// Note this classes stil under development!!!!!!!!!!!!!!!
// 
//----------------------------------------------------------------------------

#if !defined(OWL_DRAWITEM_H)              // Sentry, use file only if it's not already included.
#define OWL_DRAWITEM_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/gadgetwi.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup newctrl
/// @{



//
/// \class TDrawItemProxy
// ~~~~~ ~~~~~~~~~~~~~~
/// Proxy parent for TDrawItem.
//
class _OWLCLASS TDrawItemProxy {
  public:
    typedef TIPtrArray<TDrawItem*> TDrawItemArray;
  public:
    TDrawItemProxy(TFont* font = new TGadgetWindowFont);
    virtual ~TDrawItemProxy();

            void        SetFont(TFont* font);
    virtual TFont*      GetFont()      { return Font; }
            void        SetCelArray(TCelArray* celarray, TAutoDelete del=AutoDelete);
            void        SetCelArray(TResId resId, uint count, TModule* module = &GetGlobalModule());
    virtual TCelArray*  GetCelArray()  { return CelArray; }
            void        RegisterItem(TDrawItem* item); ///< For automatic deletion of items
            TDrawItemArray& GetItems();

  // Data members -- will become private
  //
  protected_data:
    TFont*          Font;
    TCelArray*      CelArray;
    bool            DeleteCel;  ///< true if CelArray should be deleted.
    TDrawItemArray*  ItemArray;
};

//
//
/// \class TButtonTextDrawItem
// ~~~~~ ~~~~~~~~~~~~~~~~~~~
//
//
class _OWLCLASS TButtonTextDrawItem: public TDrawItem{
  public:
  	/// TAlign contains values that defines how text will be layed out.
    enum TAlign 
    {
    	aLeft, 		///< Aligns the text at the left edge of the bounding rectangle.
    	aCenter,	///< Aligns the text horizontally at the center of the bounding rectangle. 
    	aRight		///< Aligns the text at the right edge of the bounding rectangle.
    };
    
    /// TStyle contains values that defines how the button will be displayed
    enum TStyle 
    {
    	sNone=0,						///< Neither text or bitmap is displayed.
    	sBitmap=0x001,			///< Only the bitmap is displayed.
    	sText=0x002, 				///< Only text is displayed.
    	sBitmapText=0x003		///< Both text and bitmap are displayed.
    };
    
	/// TLayoutStyle contains values that defines how a bitmap and text will be layed
	/// out.
    enum TLayoutStyle 
    {
    	lTextLeft,		///< Text left, bitmap right.
    	lTextTop,			///< Text top, bitmap bottom.
    	lTextRight,		///< Text right, bitmap left.
    	lTextBottom		///< Text bottom, bitmap top.
    };
    
    enum TSeparator {sepNoBitmap=-2,sepUseBitmap=-1};
    

    TButtonTextDrawItem(TDrawItemProxy* proxy=0);
    ~TButtonTextDrawItem();

    // get/set
    LPCTSTR   GetText() const;
    void      SetText(LPCTSTR text);
    void SetText(const tstring& text) {SetText(text.c_str());}
    int        GetIndex() const;
    void      SetIndex(int index);
    TStyle     GetStyle() const;
    void      SetStyle(const TStyle style);
    TAlign    GetAlign() const;
    void      SetAlign(const TAlign align);
    TLayoutStyle GetLayoutStyle() const;
    void      SetLayoutStyle(const TLayoutStyle style);

    // usually it uses Proxy CelArray and Font
            void        SetFont(TFont* font);
    virtual TFont*      GetFont()      { return Font; }
            void        SetCelArray(TCelArray* celarray, TAutoDelete del=AutoDelete);
            void        SetCelArray(TResId resId, uint count, TModule* module = &GetGlobalModule());
    virtual TCelArray*  GetCelArray()  { return CelArray; }

    // overriden virtuals
    virtual void Draw(DRAWITEMSTRUCT & drawItem);
    virtual void Measure(MEASUREITEMSTRUCT & drawItem);
    virtual int  Compare(COMPAREITEMSTRUCT & drawItem);
    
    // new virtuals
    virtual void Paint(TDC& rc, TRect& rect);
    virtual void Layout(const TRect& src, TRect& textRect, TRect& bmpRect);
    virtual void PaintBorder(TDC& rc, TRect& rect);
    virtual void PaintText(TDC& rc, TRect& rect);
    virtual void PaintBitmap(TDC& rc, TRect& rect);
    virtual TColor GetBkColor();
    virtual TColor GetTextColor();
    
/// Returns true if (Flags & ODS_DISABLED)==0; false otherwise.
    bool IsEnabled()    { return (Flags&ODS_DISABLED)==0; }
    
/// Returns true if (Flags & ODS_SELECTED)==0; false otherwise.
    bool IsSelected()    { return (Flags&ODS_SELECTED)!=0;  }
    
/// Returns true if (Flags & ODS_FOCUS)==0; false otherwise.
    bool IsFocused()    { return (Flags&ODS_FOCUS)!=0;    }
    
/// Returns true if (Flags & ODS_GRAYED)==0; false otherwise.
    bool IsGrayed()      { return (Flags&ODS_GRAYED)!=0;    }
    
/// Returns true if (Flags & ODS_CHECKED)==0; false otherwise.
    bool IsChecked()    { return (Flags&ODS_CHECKED)!=0;  }

    // helpers
    virtual void GetTextSize(TSize& size);
    virtual void GetButtonSize(TSize& btnSize);

  // Data members -- will become private
  //
  protected_data:
    LPTSTR          Text;       ///< new'd copy of the text for this gadget
    int              Index;      ///< Index of Bitmap in Proxy CellArray if >= 0
    TAlign          Align;      ///< Alignment: left, center or right
    TStyle          Style;      ///< Style Bitmap, Text, Bitmap and Text
    TLayoutStyle    LayoutStyle;///< Layout style

    //
    uint            Flags;

    //
    TFont*          Font;        ///< Local if overriding  ?? I need this? 
    TCelArray*      CelArray;   ///< Local if overriding  ?? I need this? 
    bool            DeleteCel;  ///< Local
    TDrawItemProxy* Proxy;      ///< Proxy
};


//
//
// class TDrawMenuItem
// ~~~~~ ~~~~~~~~~~~~~
/// Derived from TButtonTextDrawItem, the TDrawMenuItem class is used for owner
/// drawn menus.
//
class _OWLCLASS TDrawMenuItem: public TButtonTextDrawItem {
  public:
/// Prototype for a function used to paint the check image of a menu item.
    typedef void (TDrawMenuItem::*TCheckPainter)(TDC& dc, TRect& rect);
    	
  public:
    TDrawMenuItem(TDrawItemProxy* proxy, int cmdId, int index, LPCTSTR title=0);
    TDrawMenuItem(TDrawItemProxy* proxy, int cmdId, int index, const tstring& title);
//FMM DECLARE_CASTABLE line inserted
    DECLARE_CASTABLE;
    virtual void PaintBitmap(TDC& dc, TRect& rect);
    virtual void PaintText(TDC& rc, TRect& rect);
    virtual void GetTextSize(TSize& size);
    virtual void GetButtonSize(TSize& btnSize);
    virtual TColor GetBkColor();
    virtual TColor GetTextColor();
    virtual void PaintCheckFace(TDC& dc, TRect& rect);

            int  GetCmdId();
            void SetCheckPainter(TCheckPainter painter);

            void PaintCheck_None(TDC& dc, TRect& rect);
            void PaintCheck_3Angle(TDC& dc, TRect& rect);
            void PaintCheck_Arrow(TDC& dc, TRect& rect);
            void PaintCheck_Box(TDC& dc, TRect& rect);
            void PaintCheck_Diamond(TDC& dc, TRect& rect);
            void PaintCheck_Dot(TDC& dc, TRect& rect);
            void PaintCheck_Plus(TDC& dc, TRect& rect);
            void PaintCheck_V(TDC& dc, TRect& rect);
            void PaintCheck_X(TDC& dc, TRect& rect);

  // Data members -- will become private
  //
  protected_data:
    int            CmdId;
    TCheckPainter CheckPainter;
};

//
//
/// \class TDrawMenuItemProxy
// ~~~~~ ~~~~~~~~~~~~~~~~~~
/// Derived from TDrawItemProxy, TDrawMenuItemProxy is a proxy parent for
/// TDrawMenuItem.
//
class _OWLCLASS TDrawMenuItemProxy : public TDrawItemProxy {
  public:
		/// Container of TDrawItem's.
    typedef TTypedArray<uint,uint,TStandardAllocator>  TUIntAray;
    
  public:
    TDrawMenuItemProxy(TResId resId, TModule* module = &GetGlobalModule());
    virtual ~TDrawMenuItemProxy();
    
    TDrawItem* FindItem(uint cmdId);
    void  RemapMenu(HMENU hMenu);
    bool  EvMenuChar(uint uChar, uint, HMENU hMenu, TParam2& param);

  protected:
    int  GetIndex(int cmdId);
    virtual TDrawItem*  CreateItem(uint itemId, const tstring& text);
    static TFont* CreateProxyMenuFont();

  protected:
/// Array of id's.
    TUIntAray*  Array;
    
  private:
/// Level of recursion when remapping menu.
    int iRecurse;
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//-------------------------------------------------------------------------
//
// Inlines
//
//

//
/// Returns ItemArray.
//
inline TDrawItemProxy::TDrawItemArray& TDrawItemProxy::GetItems()
{
  return *ItemArray;
}
//
/// Returns Text, the text do display on the button.
//
inline LPCTSTR TButtonTextDrawItem::GetText() const
{
  return Text;
}
//
/// Returns Index, the index of Bitmap in the Proxy CellArray or -1.
//
inline int TButtonTextDrawItem::GetIndex() const
{
  return Index;
}
//
/// Returns Style: Bitmap only, Text only, or Bitmap and Text.
//
inline TButtonTextDrawItem::TStyle TButtonTextDrawItem::GetStyle() const
{
  return Style;
}
//
/// Sets Style to style.
//
inline void TButtonTextDrawItem::SetStyle(const TStyle style)
{
  Style = style;
}
//
/// Returns Align.
//
inline TButtonTextDrawItem::TAlign TButtonTextDrawItem::GetAlign() const
{
  return Align;
}
//
/// Sets Align to align.
//
inline void TButtonTextDrawItem::SetAlign(const TAlign align)
{
  Align = align;
}
//
/// Returns LayoutStyle.
//
inline TButtonTextDrawItem::TLayoutStyle TButtonTextDrawItem::GetLayoutStyle() const
{
  return LayoutStyle;
}
//
/// Sets LayoutStyle to style.
//
inline void TButtonTextDrawItem::SetLayoutStyle(const TLayoutStyle style)
{
  LayoutStyle = style;
}
//
/// Set the routine to be used to paint the check mark.
//
inline void TDrawMenuItem::SetCheckPainter(TCheckPainter painter)
{
  CheckPainter = painter;
  Index = CheckPainter ? -3 : -1;
}
//
/// Returns CmdId.
//
inline int TDrawMenuItem::GetCmdId()
{
  return CmdId;
}
//

} // OWL namespace

#endif  // OWL_DRAWITEM_H sentry.
