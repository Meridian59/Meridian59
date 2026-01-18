//----------------------------------------------------------------------------
// ObjectWindow - OWL NExt
// Copyright 1999. Yura Bidus.
// All Rights reserved.
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//
//  OVERVIEW
//  ~~~~~~~~
//  Syntax coloring hex editor.
//----------------------------------------------------------------------------
#if !defined(COOL_HEXEDIT_H)
#define COOL_HEXEDIT_H


#include <owl/docview.h>
#include <owl/doctpl.h>
#include <owl/scroller.h>
#include <owl/opensave.h>
#include <owl/dialog.h>
#include <owl/combobox.h>

#include <coolprj/defs.h>
#include <coolprj/cooledit.h>

#if !defined(COOL_NODRAGDROP)
#include <coolprj/dragdrop.h>
#endif

/////////////////////////////////////////////////////////////////////////////
// THexEdit window

enum {
  //  Base colors
  HEXINDEX_WHITESPACE,
  HEXINDEX_ADDRESS,
  HEXINDEX_DIGITS,
  HEXINDEX_ASCII,
  HEXINDEX_SELECTION,

  HEXINDEX_LAST,
};



////////////////////////////////////
// class THexBuffer
// ~~~~~ ~~~~~~~~~~
//
//{{GENERIC = THexBuffer}}
class _COOLCLASS THexBuffer {
  public:
    typedef TCoolTextBuffer::TSyntaxDescr TSyntaxDescr;
    enum TBufferFlags {
      bfReadOnly      = 0x00000001L,
      bfModified      = 0x00010000L,
      bfCreateBackup  = 0x00020000L,
    };

  public:
    THexBuffer();
    virtual ~THexBuffer();

    //  'Dirty' flag
    void SetDirty(bool dirty = true);
    bool IsDirty() const;
    bool IsReadOnly() const;
    void EnableReadOnly(bool enable);

    // syntax colorArray
    void SetSyntaxDescr(int index, const TSyntaxDescr& node);
    TSyntaxDescr& GetSyntaxDescr(int index) const;
    bool SyntaxDescrExist() const;
    static TSyntaxDescr* GetDefSyntaxDescr(int index);
    void BuildDefSyntaxDescr();
    void SetSyntaxArray(TSyntaxDescr* array);
    // save and restore all format and font information
    void SaveSyntaxDescr(owl::TConfigFile& file);
    void RestoreSyntaxDescr(owl::TConfigFile& file);

    void GetFont(LOGFONT& lf) const;
    void SetFont(const LOGFONT& lf);

    virtual bool Load(LPCTSTR filename) = 0;
    virtual bool Save(LPCTSTR filename, bool clearDirty = true) = 0;
    virtual void Clear() = 0;
    virtual owl::uint GetDataSize() = 0;
    virtual owl::uint GetBuffer(owl::uint index, owl::uint8* buffer, owl::uint size) = 0;
    virtual owl::uint32 Search(const TEditPos& searchRange, owl::uint8* text, owl::uint len,
                          bool up = false);

  protected_data:
    owl::uint32        Flags;
    TSyntaxDescr* SyntaxArray;// size == COLORINDEX_LAST
    LOGFONT       BaseFont;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    THexBuffer(const THexBuffer&);
    THexBuffer& operator=(const THexBuffer&);
}; 

//{{THexBuffer = THexFileBuffer}}
class _COOLCLASS THexFileBuffer: public THexBuffer {
  typedef THexBuffer Inherited;
  public:
    THexFileBuffer();
    ~THexFileBuffer();

    bool Load(owl::TFile& file);
    bool Save(owl::TFile& file, bool clearDirty = true);

    virtual bool Load(LPCTSTR filename);
    virtual bool Save(LPCTSTR filename, bool clearDirty = true);
    virtual void Clear();
    virtual owl::uint GetDataSize();
    virtual owl::uint GetBuffer(owl::uint index, owl::uint8* buffer, owl::uint size);
//  virtual owl::uint32 Search(const TEditPos& searchRange, owl::uint8* text, owl::uint len,
//                      bool up = false);

  protected_data:
    owl::uint8*        BinaryData;
    owl::uint          DataSize;
};

inline void THexBuffer::SetDirty(bool dirty)
  { dirty ? Flags |=bfModified : Flags &=~bfModified; }
inline bool THexBuffer::IsDirty() const
  { return (Flags&bfModified)!=0;    }
inline bool THexBuffer::IsReadOnly() const
  {  return (Flags&bfReadOnly)!=0;  }
inline void THexBuffer::EnableReadOnly(bool enable)
  {  enable ? Flags |= bfReadOnly : Flags &= ~bfReadOnly;  }
inline THexBuffer::TSyntaxDescr& THexBuffer::GetSyntaxDescr(int index) const
  {  return SyntaxArray[index];}
inline bool THexBuffer::SyntaxDescrExist() const
  {  return SyntaxArray != 0; }
inline owl::uint THexFileBuffer::GetDataSize()
  { return DataSize; }


//-----------------------------------------------------------------------------
// class THexViewWnd
// ~~~~~ ~~~~~~~~~~~~
//
//{{TControl = THexViewWnd}}
class _COOLCLASS THexViewWnd: public owl::TControl {
  typedef owl::TControl Inherited;
  public:
    enum TFindFlags {
      ffNone        = 0x0000,
      ffMatchCase   = 0x0001,
      ffWholeWord   = 0x0002,
      ffDirectionUp = 0x0004,
      ffWholeDoc    = 0x0008,

      ffFindNext    = 0x0010,
      ffReplace     = 0x0020,
      ffReplaceAll  = 0x0040,
      ffDlgTerm     = 0x0080,
      ffSEChanged   = 0x0100,
      ffMaskFlags   = ffFindNext|ffReplace|ffReplaceAll|ffDlgTerm|ffSEChanged,
    };
    enum TPosType{
      ptNone=-1, ptAddress, psDigit, psAscii,
    };
    enum TSelType{
      stStreamSelection, stLineSelection,
    };

    struct TTextSelection {
      public:
        TTextSelection(THexViewWnd* parent):Parent(parent){}
        virtual ~TTextSelection(){}
        virtual TSelType  GetType() = 0;
        virtual bool      HasSelection(int line) = 0;
        virtual bool      GetSelection(int line, int& startChar, int& endChar) = 0;
        virtual bool      IsEmpty();
        virtual TEditPos  GetStart() = 0;
        virtual TEditPos  GetEnd() = 0;
        virtual void      Extend(const TEditPos& pos) = 0;
        virtual bool      Copy(std::ostream& os) = 0;
                bool      Copy();

      protected:
        THexViewWnd* GetParent();
        UINT  GetFormat();
        bool  TextToClipboard(owl::TClipboard& cb, owl::uint8* text, int len);
        THexBuffer* GetBuffer();
      private:
        THexViewWnd* Parent;
    };
    struct TLineSelection: public TTextSelection {
      private:
        typedef TTextSelection Inherited;
      public:
        TLineSelection(THexViewWnd* parent, int start, int end);
        virtual TSelType GetType();
        virtual bool      HasSelection(int line);
        virtual bool      GetSelection(int line, int& startChar, int& endChar);
        virtual TEditPos  GetStart();
        virtual TEditPos  GetEnd();
        virtual void      Extend(const TEditPos& point);
        virtual bool      Copy(std::ostream& os);

      protected:
        int Start;
        int End;
    };
    struct TStreamSelection: public TTextSelection {
      private:
        typedef TTextSelection Inherited;
      public:
        TStreamSelection(THexViewWnd* parent, const TEditRange& range);
        virtual TSelType GetType();
        virtual bool      HasSelection(int line);
        virtual bool      GetSelection(int line, int& startChar, int& endChar);
        virtual TEditPos  GetStart();
        virtual TEditPos  GetEnd();
        virtual void      Extend(const TEditPos& point);
        virtual bool      Copy(std::ostream& os);
      protected:
        TEditRange Range;
    };
    enum THexTxtFlags{
      cfFocused           = 0x000001,
      cfHasCaret          = 0x000002,
      cfAddress           = 0x000004,
      cfDigit             = 0x000008,
      cfAscii             = 0x000010,
      cfSelMargin         = 0x000020,
      cfShowInactiveSel   = 0x000040,
      cfCursorHidden      = 0x000080,
      cfMouseDown         = 0x000100,
      cfNoUseCaret        = 0x000200,
      cfShowWideAddr      = 0x000400,
      cfSecondDigit        = 0x000800,
#if !defined(COOL_NODRAGDROP)
      cfNoDragDrop        = 0x010000,
      cfDragPending       = 0x020000,
      cfDraggingText      = 0x040000,
      cfDropPosVisible    = 0x080000,
#endif
    };
#if !defined(COOL_NODRAGDROP)
    class _COOLCLASS THexDragDropProxy: public TDragDropProxy{
      public:
        THexDragDropProxy():TDragDropProxy(0){}
    };
    friend class THexDragDropProxy;
#endif

  public:
    THexViewWnd(owl::TWindow* parent,int id,LPCTSTR title,int x, int y,
                int w, int h, owl::TModule* module = 0);
    ~THexViewWnd();

  public:
    owl::TColor  GetTxColor(int index) const;
    owl::TColor  GetBkColor(int index) const;
    THexBuffer::TSyntaxDescr& GetSyntaxDescr(int index) const;
    bool    IsUnderline(int index) const;
    bool    IsItalic(int index) const;
    bool    IsBold(int index) const;
    void    GetFont(LOGFONT& lf) const;
    void    SetFont(const LOGFONT &lf);
    int     GetCharWidth() const;
    int     GetLineHeight() const;
    owl::TFont*  GetFont(int fontindex);
    int      GetNumLines() const;
    int     GetCharsInLine() const;
    owl::uint    GetLineData(int index, owl::uint8* buff, owl::uint size) const;
    int      GetLineLength(int index);
    int     GetMarginWidth() const;
    bool    IsShowAddress() const;
    bool    IsShowDigit() const;
    bool    IsShowAscii() const;
    bool    IsShowWideAddress() const;
    void    EnableShowAddress(bool enable = true);
    void    EnableShowDigit(bool enable = true);
    void    EnableShowAscii(bool enable = true);
    void    EnableShowWideAddress(bool enable = true);
    bool    IsReadOnly() const;
    void    EnableReadOnly(bool enable);
    bool    IsStreamSelMode() const;
    bool    IsLineSelMode() const;
    bool    IsColumnSelMode() const;
    void    SetSelMode(TSelType newtyp);
    bool    IsShowInactiveSel() const;
    void    EnableShowInactiveSel(bool enable = true);
    bool    IsCaretEnable() const;
    void    EnableCaret(bool enable = true);
    TEditPos  GetCursorPos();
    void    SetCursorPos(const TEditPos& newPos);
    bool    IsSelection() const;
    void    EnableDragDrop(bool enable = true);
    bool    IsDragDrop() const;
    void    SetSelection(const TEditRange& range);
    bool    GetSelection(TEditRange& range);
  // operations
  public:
    void    GoToAddress(int address, bool relative);

  protected:
    void    AdjustScroller();
    owl::TFont*  GetFont(bool italic = false, bool bold = false, bool underline = false);
    int     GetTopLine();
    int     GetOffsetChar();
    void    SetSyntaxFormat(owl::TDC& dc, int index);
    int     GetScreenLines();
    int     GetScreenChars();
    bool    IsFlagSet(owl::uint flag) const;
    void    SetFlag(owl::uint flag);
    void    ClearFlag(owl::uint flag);
    bool    IsInSelection(const TEditPos& texPos) const;
    TEditPos  Client2Text(const owl::TPoint& point, bool* sPart = 0);
    owl::TPoint  Text2Client(const TEditPos& pos);
    TPosType Client2PosType(const owl::TPoint& point);
    void    ShowCursor();
    void    HideCursor();
    void    UpdateCaret();
    int     CalculateActualOffset(int lineIndex, int charIndex);
    void    ClearSelection();
    void    SelectAll();
    void    PreCheckSelection();
    void    PostCheckSelection();
    void    ScrollToCaret(const TEditPos& cursorPos);
    void    CopySelection();
#if !defined(COOL_NODRAGDROP)
    TDragDropProxy* SetDragDropProxy(TDragDropProxy* proxy);
#endif

  private:
    void    CalcLineCharDim();
    owl::TFont*  CreateFont(int index);

  public:
    virtual THexBuffer* GetBuffer() = 0;
    virtual TTextSelection* CreateSelection(const TEditRange& range);
    void Paint(owl::TDC&, bool erase, owl::TRect&) override;
    void GetWindowClass(WNDCLASS&) override;
    auto GetWindowClassName() -> owl::TWindowClassName override;
    virtual void ResetView();
    virtual void DrawLine(owl::TDC& dc, const owl::TRect& rect, int lineIndex);
    virtual void DrawMargin(owl::TDC& dc, const owl::TRect& rect, int lineIndex);
    virtual bool CreateCaret();
#if !defined(COOL_NODRAGDROP)
    virtual DROPEFFECT GetDropEffect();
    virtual void DropSource(DROPEFFECT de);
    virtual void DropText(IDataObject* src, const owl::TPoint& point, DROPEFFECT de);
    virtual bool SetDragData();
    virtual void ExecuteDragDrop();
#endif
  public:
    virtual void FileNew();
    virtual bool FileOpen(LPCTSTR filename);
    void SetupWindow() override;
    void CleanupWindow() override;
    virtual owl::TConfigFile* CreateConfigFile();
    virtual void InvalidateLines(int startLine, int endLine2,
                                 bool invalidateMargin = false);
    virtual bool Search(const TEditPos& startPos, LPCTSTR text,
                        owl::uint len, TFindFlags flags = ffNone);

    void EvSize(owl::uint sizeType, const owl::TSize& size);
    bool EvSetCursor(HWND hWndCursor, owl::uint hitTest, owl::uint mouseMsg);
    void EvKillFocus(HWND hWndGetFocus);
    void EvSysColorChange();
    void CmEditCopy();     // CM_EDITCOPY
    void CeSelectEnable(owl::TCommandEnabler& tce);
    void EvSetFocus(HWND hWndLostFocus);
    void EvHScroll(owl::uint scrollCode, owl::uint thumbPos, HWND hWndCtl);
    void EvVScroll(owl::uint scrollCode, owl::uint thumbPos, HWND hWndCtl);
    void EvMouseWheel(owl::uint modKeys, int zDelta, const owl::TPoint& point);
    void EvTimer(owl::uint timerId);
    void EvLButtonDown(owl::uint modKeys, const owl::TPoint& point);
    void EvMouseMove(owl::uint modKeys, const owl::TPoint& point);
    void EvLButtonUp(owl::uint modKeys, const owl::TPoint& point);
    void EvKeyDown(owl::uint key, owl::uint repeatCount, owl::uint flags);

    // message handlers
  public:
    void KeyEscape(int repeatCount=1);
    void ExtendRight(int repeatCount=1);
    void MoveRight(int repeatCount=1);
    void ExtendLeft(int repeatCount=1);
    void MoveLeft(int repeatCount=1);
    void ExtendDown(int repeatCount=1);
    void MoveDown(int repeatCount=1);
    void ExtendUp(int repeatCount=1);
    void MoveUp(int repeatCoun=1);
    void ExtendPgDown(int repeatCount=1);
    void MovePgDown(int repeatCount=1);
    void ExtendPgUp(int repeatCount=1);
    void MovePgUp(int repeatCount=1);
    void ExtendCtrlHome(int repeatCount=1);
    void ExtendHome(int repeatCount=1);
    void MoveCtrlHome(int repeatCount=1);
    void MoveHome(int repeatCount=1);
    void ExtendCtrlEnd(int repeatCount=1);
    void ExtendEnd(int repeatCount=1);
    void MoveCtrlEnd(int repeatCount=1);
    void MoveEnd(int repeatCount=1);
    void ScrollDown(int repeatCount=1);
    void ScrollUp(int repeatCount=1);

  // data
  protected_data:
    owl::TBitmap*        MemoryBitmap;
    TEditPos        CursorPos;
    owl::uint32          Flags;
    int             LineHeight;   //  Line/character dimensions
    int             CharWidth;
    owl::TFont*          TextFonts[8];
    int             ScreenLines;
    int             ScreenChars;
    TSelType        SelType;
    TTextSelection* Selection;
    TPosType        PosType;
    static UINT     SelClipFormat;

#if !defined(COOL_NODRAGDROP)
    // drag drop support
    TDragDropProxy*   DragDropProxy;
    TDragDropSupport  DragDropSupport;
    owl::TPoint            SavedDragPos;
    UINT_PTR DragSelTimer;
#endif

  friend struct THexViewWnd::TTextSelection;
  DECLARE_RESPONSE_TABLE(THexViewWnd);
}; 
////////////////////////////////////////////////////////////////////
//
inline THexViewWnd* THexViewWnd::TTextSelection::GetParent()
  {  return Parent;}
inline bool THexViewWnd::TTextSelection::IsEmpty()
  {  return GetStart() == GetEnd(); }
inline UINT THexViewWnd::TTextSelection::GetFormat()
  {  return THexViewWnd::SelClipFormat;}
inline THexBuffer* THexViewWnd::TTextSelection::GetBuffer()
  {  return Parent->GetBuffer();}
inline THexViewWnd::TSelType THexViewWnd::TLineSelection::GetType()
  { return THexViewWnd::stLineSelection; }
inline bool THexViewWnd::TLineSelection::HasSelection(int line)
  { return (line >= Start && line < End) || (line < Start && line >= End); }
inline TEditPos THexViewWnd::TLineSelection::GetStart()
  { return TEditPos(0,Start);}
inline TEditPos THexViewWnd::TLineSelection::GetEnd()
  { return TEditPos(GetParent()->GetLineLength(End),End);}
inline void THexViewWnd::TLineSelection::Extend(const TEditPos& pos)
  { End = pos.row; }
inline THexViewWnd::TSelType THexViewWnd::TStreamSelection::GetType()
  { return THexViewWnd::stStreamSelection; }
inline THexViewWnd::TStreamSelection::TStreamSelection(
  THexViewWnd* parent, const TEditRange& range)
  : TTextSelection(parent), Range(range)
  {}
inline bool THexViewWnd::TStreamSelection::HasSelection(int line){
  return (line >= Range.srow && line <= Range.erow) ||
         (line <= Range.srow && line >= Range.erow);
}
inline TEditPos THexViewWnd::TStreamSelection::GetStart(){
  return Range.Start();
}
inline TEditPos THexViewWnd::TStreamSelection::GetEnd()
  { return Range.End(); }
inline void THexViewWnd::TStreamSelection::Extend(const TEditPos& pos) {
  Range.ecol = pos.col;
  Range.erow = pos.row;
}
//
inline THexBuffer::TSyntaxDescr& THexViewWnd::GetSyntaxDescr(int index) const {
  PRECONDITION(CONST_CAST(THexViewWnd*,this)->GetBuffer());
  return CONST_CAST(THexViewWnd*,this)->GetBuffer()->GetSyntaxDescr(index);
}
//
inline owl::TColor THexViewWnd::GetTxColor(int index) const
  { return GetSyntaxDescr(index).BkColor; }
//
inline owl::TColor THexViewWnd::GetBkColor(int index) const
  { return GetSyntaxDescr(index).BkColor; }
//
inline bool THexViewWnd::IsUnderline(int index) const
  { return GetSyntaxDescr(index).FontIndex & 0x04;}
//
inline bool THexViewWnd::IsItalic(int index) const
  { return GetSyntaxDescr(index).FontIndex & 0x02; }
//
inline bool THexViewWnd::IsBold(int index) const
  { return GetSyntaxDescr(index).FontIndex & 0x01; }
//
inline void THexViewWnd::GetFont(LOGFONT& lf) const {
  PRECONDITION(CONST_CAST(THexViewWnd*,this)->GetBuffer());
  CONST_CAST(THexViewWnd*,this)->GetBuffer()->GetFont(lf);
}
//
inline owl::TFont* THexViewWnd::GetFont(int fontindex) {
  PRECONDITION(fontindex < 4 && fontindex >=0);
  return CreateFont(fontindex);
}
//
inline bool THexViewWnd::IsFlagSet(owl::uint flag) const
  {return (Flags&flag) != 0;}
//
inline void THexViewWnd::SetFlag(owl::uint flag)
  {  Flags |= flag;}
inline void THexViewWnd::ClearFlag(owl::uint flag)
  {  Flags &= ~flag;}
inline int THexViewWnd::GetMarginWidth() const
{  return 0; }
inline bool THexViewWnd::IsShowAddress() const
  { return IsFlagSet(cfAddress); }
inline bool THexViewWnd::IsShowDigit() const
  { return IsFlagSet(cfDigit); }
inline bool THexViewWnd::IsShowAscii() const
  { return IsFlagSet(cfAscii); }
inline bool THexViewWnd::IsShowWideAddress() const
  { return IsFlagSet(cfShowWideAddr); }
inline bool THexViewWnd::IsReadOnly() const{
  PRECONDITION(CONST_CAST(THexViewWnd*,this)->GetBuffer());
  return CONST_CAST(THexViewWnd*,this)->GetBuffer()->IsReadOnly();
}
inline void THexViewWnd::EnableReadOnly(bool enable){
  PRECONDITION(GetBuffer());
  GetBuffer()->EnableReadOnly(enable);
}
inline bool THexViewWnd::IsStreamSelMode() const
  { return SelType == stStreamSelection;}
inline bool THexViewWnd::IsLineSelMode() const
  { return SelType == stLineSelection; }
inline bool THexViewWnd::IsSelection() const
  {  return Selection != 0;}
inline TEditPos THexViewWnd::GetCursorPos()
  {  return CursorPos;}
inline void THexViewWnd::CopySelection(){
  PRECONDITION(Selection);
  if(Selection)
    Selection->Copy();
}
inline void THexViewWnd::CmEditCopy()
  { CopySelection(); }
inline void THexViewWnd::CeSelectEnable(owl::TCommandEnabler& ce)
  { ce.Enable(Selection); }
inline bool THexViewWnd::IsShowInactiveSel() const
  { return IsFlagSet(cfShowInactiveSel);}
inline bool THexViewWnd::IsDragDrop() const
  { return !IsFlagSet(cfNoDragDrop); }
inline bool THexViewWnd::IsCaretEnable() const
  { return !IsFlagSet(cfNoUseCaret); }


///////////////////////////////////////////////////////////////////
// class THexEditView
// ~~~~~ ~~~~~~~~~~~~~
//
//{{THexViewWnd = THexEditView}}
class _COOLCLASS THexEditView: public THexViewWnd, public owl::TView {
  public:
    THexEditView(owl::TDocument& doc, owl::TWindow* parent = 0);
    virtual ~THexEditView();

    static LPCTSTR StaticName();  // put in resource

    auto CanClose() -> bool override;
    auto GetViewName() -> LPCTSTR override;
    auto GetWindow() -> owl::TWindow* override;
    auto SetDocTitle(LPCTSTR docname, int index) -> bool override;

  protected:
    bool     VnIsWindow(HWND hWnd);

  public_data:

  private:
    // Hidden to prevent accidental copying or assignment
    //
    THexEditView(const THexEditView&);
    THexEditView& operator=(const THexEditView&);

  DECLARE_RESPONSE_TABLE(THexEditView);
}; 

//
inline LPCTSTR THexEditView::StaticName()
  {  return _T("HexEdit View");}
//
inline bool THexEditView::CanClose()
  { return THexViewWnd::CanClose() && (Doc->NextView(this) ||
           Doc->NextView(0) != this || Doc->CanClose()); }
//
inline LPCTSTR THexEditView::GetViewName()
  {  return StaticName(); }
//
inline owl::TWindow* THexEditView::GetWindow()
  {  return (owl::TWindow*)this; }
//
inline bool THexEditView::VnIsWindow(HWND hWnd)
  { return hWnd == GetHandle() || IsChild(hWnd); }
//
inline bool THexEditView::SetDocTitle(LPCTSTR docname, int index)
  {  return THexViewWnd::SetDocTitle(docname, index); }
//
//
//

#endif // COOL_HEXEDIT_H
