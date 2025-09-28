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
//  Syntax coloring text editor.
//----------------------------------------------------------------------------
#if !defined(COOLEDIT_H)
#define COOLEDIT_H

#include <owl/docview.h>
#include <owl/doctpl.h>
#include <owl/scroller.h>
#include <owl/opensave.h>
#include <owl/dialog.h>
#include <owl/combobox.h>
#include <coolprj/defs.h>


#if !defined(COOL_NODRAGDROP)
#include <coolprj/dragdrop.h>
#endif

namespace owl {
class _OWLCLASS TCheckBox;
class _OWLCLASS TRadioButton;
class _OWLCLASS TGlyphButton;
class _OWLCLASS TConfigFile;
class _OWLCLASS TCursor;
class _OWLCLASS TCelArray;
} // OWL namespace


#define LF_BOOKMARKID(id) (TCoolTextBuffer::TLineFlags)(TCoolTextBuffer::lfBookMarkFirst << id)

#if __DEBUG > 0
# define VERIFY_TEXTPOS(pt)   VerifyTextPos(pt);
#else
# define VERIFY_TEXTPOS(pt)
#endif

//
//
//
enum {
  //  Base colors
  COLORINDEX_WHITESPACE,
  COLORINDEX_NORMALTEXT,
  COLORINDEX_MARGIN,
  COLORINDEX_SELECTION,
  COLORINDEX_SINTAXCHECK,

  //  Syntax colors
  COLORINDEX_KEYWORD,
  COLORINDEX_COMMENT,
  COLORINDEX_NUMBER,
  COLORINDEX_OPERATOR,
  COLORINDEX_STRING,
  COLORINDEX_CHARACTER,
  COLORINDEX_PREPROCESSOR,
  COLORINDEX_FUNCNAME,
  //  Compiler/debugger colors
  COLORINDEX_ERROR,
  COLORINDEX_EXECUTION,
  COLORINDEX_BREAKPOINT,
  COLORINDEX_INVALIDBREAK,
  //  ...
  //  Custom elements are allowed.
  COLORINDEX_CUSTOM0,
  COLORINDEX_CUSTOM1,
  COLORINDEX_CUSTOM2,
  COLORINDEX_CUSTOM3,
  COLORINDEX_CUSTOM4,
  COLORINDEX_CUSTOM5,
  COLORINDEX_CUSTOM6,
  COLORINDEX_CUSTOM7,
  COLORINDEX_CUSTOM8,
  COLORINDEX_CUSTOM9,
  COLORINDEX_LAST,
};

//---------------------------
// class TEditPos
// ~~~~~ ~~~~~~~~
//
class _COOLCLASS TEditPos {
  public:
    TEditPos():col(-1),row(-1){}
    TEditPos(int _col, int _row):col(_col),row(_row){}
    TEditPos(const TEditPos& pos):col(pos.col),row(pos.row){}
    bool Valid() const
      {
        return col >= 0 && row >= 0;
      }
    bool operator ==(const TEditPos& other) const
      {
        return owl::ToBool(other.col == col && other.row == row);
      }
    bool operator !=(const TEditPos& other) const
      {
        return owl::ToBool(other.col != col || other.row != row);
      }

  public:
    int row;
    int col;
};

//---------------------------
// class TEditRange
// ~~~~~ ~~~~~~~~~~
//
class _COOLCLASS TEditRange {
  public:
    TEditRange():scol(-1),srow(-1),erow(-1),ecol(-1){}
    TEditRange(int _scol, int _srow, int _ecol, int _erow)
       :scol(_scol),srow(_srow),ecol(_ecol),erow(_erow){}
    TEditRange(const TEditRange& pos):scol(pos.scol),
        srow(pos.srow),ecol(pos.ecol),erow(pos.erow){}
    TEditRange(const TEditPos& spos, const TEditPos& epos):scol(spos.col),
        srow(spos.row),ecol(epos.col),erow(epos.row){}
    void Normalize();
    TEditPos Start() const { return TEditPos(scol,srow); }
    TEditPos End()const    { return TEditPos(ecol,erow); }
    bool Valid() const
      {
        return scol >= 0 && srow >= 0 && ecol >= 0 && erow >= 0;
      }
    bool Empty() const
      {
        return scol == ecol && srow == erow;
      }
    bool operator ==(const TEditRange& other) const
      {
        return owl::ToBool(other.scol == scol && other.srow == srow &&
                      other.ecol == ecol && other.erow == erow);
      }
    bool operator !=(const TEditRange& other) const
      {
        return owl::ToBool(other.scol != scol || other.srow == srow ||
                      other.ecol != ecol || other.erow == erow);
      }

  public:
    int srow;
    int scol;
    int erow;
    int ecol;
};

////////////////////////////////////
// class TCoolTextBuffer
// ~~~~~ ~~~~~~~~~~~~~~~
//
//{{GENERIC = TCoolTextBuffer}}
class _COOLCLASS TCoolTextBuffer {
  public:
    enum TLineFlags {
      lfBookMarkFirst = 0x00000001L,
      lfExecution     = 0x00010000L,
      lfBreakPoint    = 0x00020000L,
      lfCompError     = 0x00040000L,
      lfBookMarks     = 0x00080000L,
      lfInvalidBPoint = 0x00100000L,
      lfDisabledBPoint= 0x00200000L,
      lfReserved1     = 0x00400000L,
      lfReserved2     = 0x00800000L,
    };
    enum TBufferFlags {
      bfReadOnly      = 0x00000001L,
      bfModified      = 0x00010000L,
      bfCreateBackup  = 0x00020000L,
    };
    enum TCrLfStyle {
      clStyleAutomatic  = -1,
      clStyleDos        = 0,
      clStyleUnix       = 1,
      clStyleMac        = 2,
    };
    struct TSyntaxDescr {
      TSyntaxDescr():TxColor(0,0,0),BkColor(255,255,255),FontIndex(0){}
      TSyntaxDescr(const owl::TColor& txtClr, const owl::TColor& bkClr, int fontIdx=0)
        :TxColor(txtClr),BkColor(bkClr),FontIndex(fontIdx){}

      owl::TColor  TxColor;
      owl::TColor  BkColor;
      int     FontIndex;
    };
    struct _COOLCLASS TLineInfo {
      TLineInfo(LPCTSTR text, int len = -1);
      ~TLineInfo();

      bool Append(LPCTSTR text, int len = -1);
      void SetText(LPCTSTR text, int len = -1);

      _TCHAR* Text;
      int     Length;
      int     MaxChar;
      owl::uint32  Flags;
    };
    struct TUndoNode {
      TUndoNode():Next(0),SavedPos(0,0),Dirty(false){}
      TUndoNode(const TEditPos& spos,bool dirty)
        :Next(0),SavedPos(spos),Dirty(dirty){}
      virtual ~TUndoNode(){}
      virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0) = 0;
      virtual bool GetDescription(_TCHAR* buffer, int len, owl::TModule* module = owl::Module) = 0;

      TUndoNode*  Next;
      TEditPos    SavedPos;
      bool        Dirty;
    };
    struct TRedoNode {
      TRedoNode():Next(0){}
      virtual ~TRedoNode(){}
      virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0) = 0;
      virtual bool GetDescription(_TCHAR* buffer, int len, owl::TModule* module = owl::Module) = 0;

      TRedoNode* Next;
    };
  public:
    TCoolTextBuffer(TCrLfStyle style = clStyleAutomatic);
    virtual ~TCoolTextBuffer();

    //  Attributes
    TCrLfStyle GetCRLFMode() const;
    void SetCRLFMode(TCrLfStyle style);

    //  'ReadOnly' flag
    bool IsReadOnly() const;
    void EnableReadOnly(bool enable = true);

    //  'Dirty' flag
    void SetDirty(bool dirty = true);
    bool IsDirty() const;

    int  GetLineWithFlag(owl::uint32 flag);
    int  SetLineFlag(int nLine, owl::uint32 dwFlag, bool bSet,
                     bool bRemoveFromPreviousLine = true);

    //  Undo/Redo
    TUndoNode* GetTopUndoNode();
    TRedoNode* GetTopRedoNode();
    bool CanUndo() const;
    bool CanRedo() const;
    bool Undo(TEditPos* newPos=0);
    bool Redo(TEditPos* newPos=0);
    bool AddUndoNode(TUndoNode* node);
    bool AddRedoNode(TRedoNode* node);
    void ClearUndoBuffer();
    void ClearRedoBuffer();

    // More bookmarks
    int FindNextBookmarkLine(int nCurrentLine = 0);
    int FindPrevBookmarkLine(int nCurrentLine = 0);

    virtual void Clear(bool addempty=true);
    //  Text access functions
    virtual int GetLineCount() const{ return 0; }
    virtual int GetLineLength(int /*index*/) const{ return 0; }
    virtual LPTSTR GetLineText(int /*index*/){ return 0;}
    virtual owl::uint32 GetLineFlags(int /*index*/) const{ return 0; }
    virtual void GetText(const TEditPos& /*startPos*/, const TEditPos& /*endPos*/, LPTSTR /*buffer*/, int /*buflen*/, LPCTSTR /*pszCRLF*/ = NULL){}
    virtual bool DeleteText(const TEditRange&){ return false;}
    virtual bool InsertText(const TEditPos& /*startPos*/, LPCTSTR /*text*/, TEditPos& /*endPos*/){ return false;}
    virtual bool AppendText(int /*line*/, LPCTSTR /*text*/, int /*len*/ = -1){ return false;}
    virtual void InsertLine(LPCTSTR /*text*/, int /*len*/ = -1, int /*pos*/ = -1) {}

    virtual bool GetUndoDescription(_TCHAR* buffer, int len, owl::TModule* module = owl::Module);
    virtual bool GetRedoDescription(_TCHAR* buffer, int len, owl::TModule* module = owl::Module);
    virtual bool Load(LPCTSTR /*filename*/, TCrLfStyle /*style*/ = clStyleAutomatic){ return false;}
    virtual bool Save(LPCTSTR /*filename*/, TCrLfStyle /*style*/ = clStyleAutomatic, bool /*clearDirty*/ = true){ return false;}
    // simple search
    TEditPos Search(const TEditRange& searchRange, LPCTSTR text,
                    bool caseSens=false, bool wholeWord=false,
                    bool up = false);
  protected:
    virtual int  FindLineWithFlag(owl::uint32 /*flag*/){ return -1;}
    virtual void SetLineFlags(owl::uint /*index*/, owl::uint32 /*flags*/){}
  public:
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
    void SetUndoSize(owl::uint newmax);
    owl::uint GetUndoSize();
    owl::uint GetUndoCnt();
    void SetRedoSize(owl::uint newmax);
    owl::uint GetRedoSize();
    owl::uint GetRedoCnt();

    static bool LoadStringIndex(_TCHAR* buffer, int len, int resId, int index, _TCHAR delimCh = _T('|'), owl::TModule* module = owl::Module);

  protected:
    void AdjustUndoSize();
    void AdjustRedoSize();

  protected_data:
    owl::uint32        Flags;
    TSyntaxDescr* SyntaxArray;// size == COLORINDEX_LAST

    //Y.B. Undo Redo Management -> maybe create class UndoManager ???
    TUndoNode*    UndoList;
    owl::uint          UndoCnt;
    owl::uint          UndoSize;
    TRedoNode*    RedoList;
    owl::uint          RedoCnt;
    owl::uint          RedoSize;

    TCrLfStyle    CRLFMode;
    LOGFONT       BaseFont;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TCoolTextBuffer(const TCoolTextBuffer&);
    TCoolTextBuffer& operator=(const TCoolTextBuffer&);
}; 
//{{TCoolTextBuffer = TCoolFileBuffer}}
class _COOLCLASS TCoolFileBuffer: public TCoolTextBuffer {
  typedef TCoolTextBuffer Inherited;
  public:
    TCoolFileBuffer(TCrLfStyle style = clStyleAutomatic)
        : TCoolTextBuffer(style){}
    ~TCoolFileBuffer(){}

  public:
    // Text File
    bool Load(owl::TFile& file, TCrLfStyle style = clStyleAutomatic);
    bool Save(owl::TFile& file, TCrLfStyle style = clStyleAutomatic,
              bool clearDirty = true);

    virtual int GetLineCount() const;
    virtual int GetLineLength(int index) const;
    virtual owl::uint32 GetLineFlags(int index) const;
    virtual LPTSTR GetLineText(int index);
    virtual void GetText(const TEditPos& startPos, const TEditPos& endPos,
                 LPTSTR buffer, int buflen, LPCTSTR pszCRLF = NULL);
    virtual bool DeleteText(const TEditRange& range);
    virtual bool InsertText(const TEditPos& startPos, LPCTSTR text, TEditPos& endPos);
    virtual bool AppendText(int line, LPCTSTR text, int len = -1);
    virtual void InsertLine(LPCTSTR text, int len = -1, int pos = -1);

    virtual void Clear(bool addempty=true);
    virtual bool Load(LPCTSTR filename, TCrLfStyle style = clStyleAutomatic);
    virtual bool Save(LPCTSTR filename, TCrLfStyle style = clStyleAutomatic,
                      bool clearDirty = true);
  protected:
    virtual int  FindLineWithFlag(owl::uint32 flag);
    virtual void SetLineFlags(owl::uint index, owl::uint32 flags);

  protected:
    TLineInfo* GetLine(int index);
  protected_data:
    // main Buffer data
    TCoolPtrArray<TLineInfo*> LineArray;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TCoolFileBuffer(const TCoolFileBuffer&);
    TCoolFileBuffer& operator=(const TCoolFileBuffer&);
};    

class _COOLCLASS TCoolTextWnd;
class _COOLCLASS TSyntaxParser;
//
typedef TSyntaxParser* (*TParserCreator)(TCoolTextWnd*);

//
// class TCoolScroller
// ~~~~~ ~~~~~~~~~~~~~
//
//
class _COOLCLASS TCoolScroller: public owl::TScroller {
    typedef owl::TScroller Inherited;
  public:
    TCoolScroller(owl::TWindow* window, int xUnit, int yUnit, long xRange,
                  long yRange);
    virtual void  ScrollTo(long x, long y);
#if 0 //OWLInternalVersion < 0x06001020L
    virtual void  SetSBarRange();
#endif

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TCoolScroller(const TCoolScroller&);
    TCoolScroller& operator=(const TCoolScroller&);
};

class _COOLCLASS TSyntaxParser;

//
// class TCoolTextWnd
// ~~~~~ ~~~~~~~~~~~~
//
//{{TControl = TCoolTextWnd}}
class _COOLCLASS TCoolTextWnd: public owl::TControl {
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
    //  Syntax coloring overrides
    struct TTextBlock{
      int CharPos;
      int SyntaxIndex;
    };
    enum TSelType{
      stStreamSelection, stLineSelection, stColumnSelection,
    };
    struct _COOLCLASS TTextSelection {
      public:
        TTextSelection(TCoolTextWnd* parent):Parent(parent){}
        virtual ~TTextSelection(){}
        virtual TSelType  GetType() = 0;
        virtual bool      HasSelection(int line) = 0;
        virtual bool      GetSelection(int line, int& startChar, int& endChar) = 0;
        virtual bool      IsEmpty();
        virtual TEditPos  GetStart() = 0;
        virtual TEditPos  GetEnd() = 0;
        virtual void      Extend(const TEditPos& pos) = 0;
        virtual bool      Delete() = 0;
        virtual bool      Copy(owl::tostream& os) = 0;
                bool      Copy();

      protected:
        TCoolTextWnd* GetParent();
        UINT  GetFormat();
        bool  TextToClipboard(owl::TClipboard& cb, LPCTSTR text, int len);
        TCoolTextBuffer* GetBuffer();
      private:
        TCoolTextWnd* Parent;
    };
    struct _COOLCLASS TLineSelection: public TTextSelection {
      private:
        typedef TTextSelection Inherited;
      public:
        TLineSelection(TCoolTextWnd* parent, int start, int end);
        virtual TSelType GetType();
        virtual bool      HasSelection(int line);
        virtual bool      GetSelection(int line, int& startChar, int& endChar);
        virtual TEditPos  GetStart();
        virtual TEditPos  GetEnd();
        virtual void      Extend(const TEditPos& point);
        virtual bool      Delete();
        virtual bool      Copy(owl::tostream& os);

      protected:
        int Start;
        int End;
    };
    struct _COOLCLASS TStreamSelection: public TTextSelection {
      private:
        typedef TTextSelection Inherited;
      public:
        TStreamSelection(TCoolTextWnd* parent, const TEditRange& range);
        virtual TSelType GetType();
        virtual bool      HasSelection(int line);
        virtual bool      GetSelection(int line, int& startChar, int& endChar);
        virtual TEditPos  GetStart();
        virtual TEditPos  GetEnd();
        virtual void      Extend(const TEditPos& point);
        virtual bool      Delete();
        virtual bool      Copy(owl::tostream& os);
      protected:
        TEditRange Range;
    };
    struct _COOLCLASS TColumnSelection: public TStreamSelection {
      private:
        typedef TStreamSelection Inherited;
      public:
        TColumnSelection(TCoolTextWnd* parent, const TEditRange& range);
        virtual TSelType GetType();
        virtual bool GetSelection(int line, int& startChar, int& endChar);
        virtual bool Delete();
        virtual bool Copy(owl::tostream& os);
    };
    enum TCoolTxtFlags{
      cfFocused           = 0x000001,
      cfOverType          = 0x000002,
      cfHasCaret          = 0x000004,
      cfViewTabs          = 0x000008,
      cfSelMargin         = 0x000010,
      cfShowInactiveSel   = 0x000020,
      cfCursorHidden      = 0x000040,
      cfBookmarkExist     = 0x000080,     // More bookmarks
      cfMouseDown         = 0x000100,
      cfSmartCursor       = 0x000200,
      cfNoUseCaret        = 0x000400,
      cfLookWrdUnderCur   = 0x000800,
      cfAutoIntend        = 0x001000,
      cfShowLineNumber    = 0x002000,
      cfCurbDrag          = 0x004000,
#if !defined(COOL_NODRAGDROP)
      cfNoDragDrop        = 0x010000,
      cfDragPending       = 0x020000,
      cfDraggingText      = 0x040000,
      cfDropPosVisible    = 0x080000,
#endif
    };
#if !defined(COOL_NODRAGDROP)
    class _COOLCLASS TTextDragDropProxy: public TDragDropProxy{
      public:
        TTextDragDropProxy():TDragDropProxy(0){}
    };
    friend class TTextDragDropProxy;
#endif

  public:
    TCoolTextWnd(owl::TWindow* parent,int id,LPCTSTR title,int x, int y,
                int w, int h, owl::TModule* module = 0);
    ~TCoolTextWnd();

  protected:
    virtual TCoolTextBuffer* GetBuffer() = 0;
    void Paint(owl::TDC&, bool erase, owl::TRect&) override;
    void GetWindowClass(WNDCLASS&) override;
    virtual void    DrawLine(owl::TDC& dc, const owl::TRect& rect, int lineIndex);
    virtual void    DrawMargin(owl::TDC& dc, const owl::TRect& rect, int lineIndex);
    virtual int     GetLineActualLength(int lineIndex);
    virtual TTextSelection* CreateSelection(const TEditRange& range);
    virtual void    AdjustTextPoint(owl::TPoint& point);
    virtual LPCTSTR GetBreakChars() const;
    virtual void    ResetView();
    virtual bool    CreateCaret(bool overType);
    virtual void    FormatLineNumber(LPTSTR buffer, int len, int index);
    virtual int     CellImageIndex(int lineIndex);
    virtual void    AdjustScroller();

    virtual void     DisplayCursorPosition(int /*nRow*/, int /*nCol*/) {}

#if !defined(COOL_NODRAGDROP)
    virtual DROPEFFECT GetDropEffect();
    virtual void DropSource(DROPEFFECT de);
    virtual void DropText(IDataObject* src, const owl::TPoint& point, DROPEFFECT de);
    virtual bool SetDragData();
    virtual void ExecuteDragDrop();
#endif
  public:
    virtual void    FileNew();
    virtual bool    FileOpen(LPCTSTR filename);
    void SetupWindow() override;
    void CleanupWindow() override;
    auto GetWindowClassName() -> owl::TWindowClassName override;
    virtual owl::TConfigFile* CreateConfigFile();
    virtual void    InvalidateLines(int startLine, int endLine2,
                                    bool invalidateMargin = false);
    virtual bool    Search(const TEditPos& startPos, LPCTSTR text,
                           TFindFlags flags = ffNone);

  // access functions
  public:
    // properties
    bool    IsTabsVisible() const;
    void    EnableTabs(bool viewTabs);
    bool    IsStreamSelMode() const;
    bool    IsLineSelMode() const;
    bool    IsColumnSelMode() const;
    void    SetSelMode(TSelType newtyp);
    bool    IsShowInactiveSel() const;
    void    EnableShowInactiveSel(bool enable = true);
    bool    IsItalic(int index) const;
    bool    IsBold(int index) const;
    bool    IsUnderline(int index) const;
    bool    IsSelectionMargin() const;
    void    EnableSelectionMargin(bool bSelMargin);
    bool    IsCaretEnable() const;
    void    EnableCaret(bool enable = true);
    bool    IsSmartCursor() const;
    void    EnableSmartCursor(bool enable = true);
    TEditPos  GetCursorPos();
    void    SetCursorPos(const TEditPos& newPos);
    bool    IsSelection() const;
    void    EnableDragDrop(bool enable = true);
    bool    IsDragDrop() const;
    void    EnableAutoIndent(bool enable = true);
    bool    IsAutoIndent() const;
    void    ShowLineNumbers(bool enable = true);
    bool    IsLineNumbers() const;
    bool    IsOverType() const;
    void    EnableOverType(bool enable = true);

    int     GetNumLines() const;
    bool    GetLine(LPTSTR str, int strSize, int lineNumber) const;
    void    GetFont(LOGFONT& lf) const;
    void    SetFont(const LOGFONT &lf);
    int     GetTabSize() const;
    void    SetTabSize(int tabSize);
    owl::TColor  GetTxColor(int index) const;
    owl::TColor  GetBkColor(int index) const;
    owl::TFont*  GetFont(int index);
    int     GetLineHeight() const;
    int     GetLineLength(int index) const;
    int     GetCharWidth() const;
    int     GetMarginWidth() const;
    void    SetMarginWidth(int width);
    int     GetLineNumberWidth() const;
    void    SetLineNumberWidth(int width);
    int     GetLineDigits() const;

    TCoolTextBuffer::TSyntaxDescr& GetSyntaxDescr(int index) const;
    TSyntaxParser* GetSyntaxParser();
    void    SetSyntaxParser(TSyntaxParser* parser);
    void    SetSelection(const TEditRange& range);
    bool    GetSelection(TEditRange& range);
    void    WordUnderCursor(LPTSTR text, owl::uint size);

  // operations
  public:
    void    GoToLine(int lineIndex, bool relative);
    void    ToggleBookmark(int bookmarkId);
    void    GoBookmark(int bookmarkId);
    void    ClearBookmarks();
    void    ToggleRandBookmark();
    void    NextRandBookmark();
    void    PrevRandBookmark();
    void    ClearRandBookmarks();

  // implementation
  protected:
    owl::TFont*  GetFont(bool italic = false, bool bold = false, bool underline = false);
    bool    IsInSelection(const TEditPos& texPos);
    void    DrawLineBlock(owl::TDC& dc, owl::TPoint& origin, const owl::TRect& clipRect,
                          int clrIndex, LPCTSTR text, int offset,
                          int count, const TEditPos& textPos);
    TEditPos  Client2Text(const owl::TPoint& point);
    owl::TPoint  Text2Client(const TEditPos& pos);
    int     GetScreenLines();
    int     GetScreenChars();
    int     GetMaxLineLength() const;
    void    ShowCursor();
    void    HideCursor();
    void    UpdateCaret();
    int     CalculateActualOffset(int lineIndex, int charIndex);
    void    ClearSelection();
    void    EnablerNextBookmark(owl::TCommandEnabler& tce);
    void    EnablerPrevBookmark(owl::TCommandEnabler& tce);
    void    EnablerClearRandBookmarks(owl::TCommandEnabler& tce);
    int     GetTopLine();
    int     GetOffsetChar();
    void    SelectAll();
    void    PreCheckSelection();
    void    PostCheckSelection();
    void    SetSyntaxFormat(owl::TDC& dc, int index);
    void    ScrollToCaret(const TEditPos& cursorPos);
#if __DEBUG > 0
    void    VerifyTextPos(const TEditPos& pos);
#endif
    LPCTSTR GetLineText(int lineIndex);
    bool    IsFlagSet(owl::uint flag) const;
    void    SetFlag(owl::uint flag);
    void    ClearFlag(owl::uint flag);

    // message handlers
  public:
    void    KeyEscape(int repeatCount=1);
    void    ExtendWordRight(int repeatCount=1);
    void    ExtendRight(int repeatCount=1);
    void    MoveWordRight(int repeatCount=1);
    void    MoveRight(int repeatCount=1);
    void    ExtendWordLeft(int repeatCount=1);
    void    ExtendLeft(int repeatCount=1);
    void    MoveWordLeft(int repeatCount=1);
    void    MoveLeft(int repeatCount=1);
    void    ExtendDown(int repeatCount=1);
    void    MoveDown(int repeatCount=1);
    void    ExtendUp(int repeatCount=1);
    void    MoveUp(int repeatCoun=1);
    void    ExtendPgDown(int repeatCount=1);
    void    MovePgDown(int repeatCount=1);
    void    ExtendPgUp(int repeatCount=1);
    void    MovePgUp(int repeatCount=1);
    void    ExtendCtrlHome(int repeatCount=1);
    void    ExtendHome(int repeatCount=1);
    void    MoveCtrlHome(int repeatCount=1);
    void    MoveHome(int repeatCount=1);
    void    ExtendCtrlEnd(int repeatCount=1);
    void    ExtendEnd(int repeatCount=1);
    void    MoveCtrlEnd(int repeatCount=1);
    void    MoveEnd(int repeatCount=1);
    void    ScrollDown(int repeatCount=1);
    void    ScrollUp(int repeatCount=1);

    // support functions
  protected:
    TEditPos  NextBreak(const TEditPos& pos);
    TEditPos  PrevBreak(const TEditPos& pos);
    void      CopySelection();
    TEditPos  StartWord(const TEditPos& pos);
    TEditPos  EndWord(const TEditPos& pos);
#if !defined(COOL_NODRAGDROP)
    TDragDropProxy* SetDragDropProxy(TDragDropProxy* proxy);
#endif
    void    DrawDraggedCurb(int position, bool redraw = false);

  private:
    owl::uint32  GetParseCookie(int lineIndex);
    void    DrawTextBlock(owl::TDC& dc, owl::TPoint& origin, const owl::TRect& clipRect,
                          LPCTSTR text, int offset, int count);
    void    ExpandText(LPCTSTR text, int offset, int count, owl::tstring& line);
    void    CalcLineCharDim();
    owl::TFont*  CreateFont(int index);
    void    AdjustXCurPos();
    bool    ToggleInsertStatus();

  // data
  protected_data:
    TEditPos        CursorPos;
    owl::uint32          Flags;
    int             TabSize;
    int             LineHeight;   //  Line/character dimensions
    int             CharWidth;
    int             ScreenLines;  //  Amount of lines/characters that
    int             ScreenChars;  //  completely fits the client area
    int             MaxLineLength;
    int             IdealCharPos;
    owl::TFont*          TextFonts[8];
    owl::TBitmap*        MemoryBitmap;
    owl::TCelArray*      CelArray;
    TSelType        SelType;
    TTextSelection* Selection;
    TSyntaxParser*  SyntaxParser;
    owl::uint32*         ParseCookies;
    int             ParseArraySize;
    int             ActualLengthArraySize;//  Pre-calculated line
    int*            ActualLineLength;     // lengths (in characters)
    owl::TCursor*        Cursor;
    owl::TPoint          LastHit;
    int              MarginWidth;
    int              LineDigits;

    int              CurbPos;   // curb drawing
    int              TmpCurbPos;

    static UINT     SelClipFormat;

#if !defined(COOL_NODRAGDROP)
    // drag drop support
    TDragDropProxy*   DragDropProxy;
    TDragDropSupport  DragDropSupport;
    owl::TPoint            SavedDragPos;
    UINT_PTR DragSelTimer;
#endif

  protected:
    void EvSize(owl::uint sizeType, const owl::TSize& size);
    bool EvSetCursor(HWND hWndCursor, owl::uint hitTest, owl::uint mouseMsg);
    void EvKillFocus(HWND hWndGetFocus);
    void EvSetFocus(HWND hWndLostFocus);
    void EvHScroll(owl::uint scrollCode, owl::uint thumbPos, HWND hWndCtl);
    void EvVScroll(owl::uint scrollCode, owl::uint thumbPos, HWND hWndCtl);
    void EvLButtonDown(owl::uint modKeys, const owl::TPoint& point);
    void EvMouseMove(owl::uint modKeys, const owl::TPoint& point);
    void EvLButtonUp(owl::uint modKeys, const owl::TPoint& point);
    void EvLButtonDblClk(owl::uint modKeys, const owl::TPoint& point);
    void EvKeyDown(owl::uint key, owl::uint repeatCount, owl::uint flags);
    void CmEditCopy();     // CM_EDITCOPY
    void CeSelectEnable(owl::TCommandEnabler& tce);
    void EvSysColorChange();
#if !defined(COOL_NODRAGDROP)
    void EvTimer(owl::uint timerId);
#endif
    void EvMouseWheel(owl::uint modKeys, int xDelta, const owl::TPoint& point);

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TCoolTextWnd(const TCoolTextWnd&);
    TCoolTextWnd& operator=(const TCoolTextWnd&);

  DECLARE_RESPONSE_TABLE(TCoolTextWnd);

  friend TSyntaxParser;
  friend struct TCoolTextWnd::TTextSelection;
}; 

//
// struct TSyntaxParser
// ~~~~~~ ~~~~~~~~~~~~~
//
class _COOLCLASS TSyntaxParser {
  public:
    TSyntaxParser(TCoolTextWnd* parent) : Parent(parent){}
    virtual ~TSyntaxParser() {}
    virtual owl::uint32 ParseLine(owl::uint32 cookie, int index, TCoolTextWnd::TTextBlock* buf, int& items) = 0;
  protected:
    LPCTSTR GetLineText(int index);
    LPCTSTR GetBreakChars() const;

    TCoolTextWnd* Parent;
};

//
// class TPlainTextParser
// ~~~~~ ~~~~~~~~~~~~~~~~
//
class _COOLCLASS TPlainTextParser: public TSyntaxParser {
  public:
    TPlainTextParser(TCoolTextWnd* parent):TSyntaxParser(parent){}
    virtual owl::uint32 ParseLine(owl::uint32 /*cookie*/, int /*index*/, TCoolTextWnd::TTextBlock* /*buf*/, int& /*items*/)
          { return 0; }
};
//
// class TCoolSearchEngine
// ~~~~~ ~~~~~~~~~~~~~~~~~
//
class _COOLCLASS TCoolSearchEngine {
  public:
    virtual ~TCoolSearchEngine(){}
    virtual TEditRange Search(TCoolTextBuffer& buffer,
                              const TEditRange& searchRange=TEditRange()) = 0;
};
//
//
//
class _COOLCLASS TBruteForceEngine: public TCoolSearchEngine {
  public:
    TBruteForceEngine(const owl::tstring& pattern,
                      bool caseSens=false,
                      bool wholeWord=false,
                      bool up = false)
      :Pattern(pattern),CaseSens(caseSens),WholeWord(wholeWord),DirectUp(up){}
    virtual TEditRange Search(TCoolTextBuffer& buffer,
                              const TEditRange& searchRange=TEditRange());
  protected:
    owl::tstring  Pattern;
    bool        CaseSens;
    bool        WholeWord;
    bool        DirectUp;
};
//
// class TBoyerMooreEngine
// ~~~~~ ~~~~~~~~~~~~~~~~~
//
class _COOLCLASS TBoyerMooreEngine: public TCoolSearchEngine {
  public:
    TBoyerMooreEngine(const owl::tstring& pattern,
                      bool caseSens=false,
                      bool wholeWord=false,
                      bool up = false);
    ~TBoyerMooreEngine();
    virtual TEditRange Search(TCoolTextBuffer& buffer,
                            const TEditRange& searchRange=TEditRange());
  protected:
    owl::uint  Delta(_TCHAR ch) { return DeltaTable[int(ch)]; }

  protected:
    // size of delta table
    static const size_t DeltaSize;
    // pointer to delta table
    owl::uint*       DeltaTable;
    owl::tstring  Pattern;

    bool        CaseSens;
    bool        WholeWord;
    bool        DirectUp;
};
//
// class TRegularExEngine
// ~~~~~ ~~~~~~~~~~~~~~~~
//
class _COOLCLASS TRegularExEngine: public TCoolSearchEngine {
  public:
    TRegularExEngine(const owl::tstring& pattern,
                      bool caseSens=false,
                      bool wholeWord=false,
                      bool up = false);
    virtual TEditRange Search(TCoolTextBuffer& buffer,
                            const TEditRange& searchRange=TEditRange());
  protected:
    bool        CaseSens;
    bool        WholeWord;
    bool        DirectUp;

    static const size_t MaxPat;
    _TUCHAR*    Pattern;
    int         Status;
    //TRegexp Regexp;
};
/*
**  The Metaphone algorithm was developed by Lawrence Phillips. It compares
**  words that sound alike but are spelled differently.
**
**  This implementation was written by Gary A. Parker and originally published
**  in the June/July, 1991 (vol. 5 nr. 4) issue of C Gazette. As published,
**  this code was explicitly placed in the public domain by the author.
*/
class _COOLCLASS TPhoneticEngine: public TCoolSearchEngine {
  public:
    TPhoneticEngine(const owl::tstring& pattern, bool up = false);
    virtual TEditRange Search(TCoolTextBuffer& buffer,
                            const TEditRange& searchRange=TEditRange());
  protected:
    owl::tstring  Pattern;
    bool        DirectUp;
    //static const owl::uint MaxMetaph;
};

//
// Approximate string matching
//
class _COOLCLASS TFuzzyEngine: public TCoolSearchEngine {
  public:
    TFuzzyEngine(const owl::tstring& pattern, bool up = false, owl::uint level = 0);
    ~TFuzzyEngine();
    virtual TEditRange Search(TCoolTextBuffer& buffer,
                            const TEditRange& searchRange=TEditRange());
  protected:
    void Next(_TCHAR **start, _TCHAR **end, int* howclose, int textloc, LPCTSTR text);

    owl::tstring  Pattern;
    bool        DirectUp;
    int          Plen;     /* length of Pattern                */
    int          Degree;   /* max degree of allowed mismatch   */
    int *Ldiff, *Rdiff;   /* dynamic difference arrays        */
    int *Loff,  *Roff;    /* used to calculate start of match */
    //static const size_t MaxPat;
};
//
//
//
class _COOLCLASS TCoolEngineDescr {
  public:
    enum{
      ecNoWholeWord = 0x0001,
      ecNoCaseSens  = 0x0002,
      ecNoGlobal    = 0x0004,
      ecNoDirection = 0x0008,
    };
  public:
    TCoolEngineDescr(){}
    virtual ~TCoolEngineDescr(){}
    virtual owl::tstring   GetName() = 0;
    virtual owl::uint GetCapability() { return 0;}
    virtual TCoolSearchEngine* Create(const owl::tstring& pattern,
                                      bool caseSens=false,
                                      bool wholeWord=false,
                                      bool up = false) = 0;
};
//
//
//
class _COOLCLASS TBruteForceDescr: public TCoolEngineDescr {
  public:
    TBruteForceDescr(){}
    owl::tstring GetName(){ return _T("Brute-Force search"); }
    TCoolSearchEngine* Create(const owl::tstring& pattern,
                              bool caseSens=false,
                              bool wholeWord=false,
                              bool up = false)
      { return new TBruteForceEngine(pattern,caseSens,wholeWord,up);}
};
//
//
//
class _COOLCLASS TBoyerMooreDescr: public TCoolEngineDescr {
  public:
    TBoyerMooreDescr(){}
    owl::tstring GetName(){ return _T("Boyer-Moore search"); }
    TCoolSearchEngine* Create(const owl::tstring& pattern,
                              bool caseSens=false,
                              bool wholeWord=false,
                              bool up = false)
      { return new TBoyerMooreEngine(pattern,caseSens,wholeWord,up);}
};
//
//
//
class _COOLCLASS TRegularExDescr: public TCoolEngineDescr {
  public:
    TRegularExDescr(){}
    owl::tstring GetName(){ return _T("Standard Regular expression"); }
    virtual owl::uint GetCapability() { return ecNoWholeWord|ecNoCaseSens;}
    TCoolSearchEngine* Create(const owl::tstring& pattern,
                              bool caseSens=false,
                              bool wholeWord=false,
                              bool up = false)
      { return new TRegularExEngine(pattern,caseSens,wholeWord,up);}
};

//
//
//
class _COOLCLASS TPhoneticDescr: public TCoolEngineDescr {
  public:
    TPhoneticDescr(){}
    owl::tstring GetName(){ return _T("Phonetic word matching"); }
    virtual owl::uint GetCapability() { return ecNoWholeWord|ecNoCaseSens;}
    TCoolSearchEngine* Create(const owl::tstring& pattern,
                              bool /*caseSens*/=false,
                              bool /*wholeWord*/=false,
                              bool up = false)
      { return new TPhoneticEngine(pattern,up);}
};
//
//
//
class _COOLCLASS TFuzzyDescrLevel1: public TCoolEngineDescr {
  public:
    TFuzzyDescrLevel1(){}
    owl::tstring GetName(){ return _T("Approximate string matching, level 1"); }
    virtual owl::uint GetCapability() { return ecNoWholeWord|ecNoCaseSens;}
    TCoolSearchEngine* Create(const owl::tstring& pattern,
                              bool /*caseSens*/=false,
                              bool /*wholeWord*/=false,
                              bool up = false)
      { return new TFuzzyEngine(pattern,up, 1);}
};
//
//
//
class _COOLCLASS TFuzzyDescrLevel2: public TCoolEngineDescr {
  public:
    TFuzzyDescrLevel2(){}
    owl::tstring GetName(){ return _T("Approximate string matching, level 2"); }
    virtual owl::uint GetCapability() { return ecNoWholeWord|ecNoCaseSens;}
    TCoolSearchEngine* Create(const owl::tstring& pattern,
                              bool /*caseSens*/=false,
                              bool /*wholeWord*/=false,
                              bool up = false)
      { return new TFuzzyEngine(pattern,up, 2);}
};
//
//
//
class _COOLCLASS TFuzzyDescrLevel3: public TCoolEngineDescr {
  public:
    TFuzzyDescrLevel3(){}
    owl::tstring GetName(){ return _T("Approximate string matching, level 3"); }
    virtual owl::uint GetCapability() { return ecNoWholeWord|ecNoCaseSens;}
    TCoolSearchEngine* Create(const owl::tstring& pattern,
                              bool /*caseSens*/=false,
                              bool /*wholeWord*/=false,
                              bool up = false)
      { return new TFuzzyEngine(pattern,up, 3);}
};
//
//
//
//

/////////////////////////////////////////////////
//{{TDialog = TCoolFindDlg}}
class _COOLCLASS TCoolFindDlg: public owl::TDialog {
    typedef owl::TDialog Inherited;
  public:
    typedef TCoolTextWnd::TFindFlags TFindFlags;
  public:
    class _COOLCLASS TData {
      typedef TCoolPtrArray<TCoolEngineDescr*> TEngineDescr;
      public:
        owl::uint32        Flags;
        owl::uint32        Error;
        LPTSTR        FindWhat;
        LPTSTR        ReplaceWith;
        int           BuffSize;
        int           EngineIndex;
        TEngineDescr  EngineDescr;

        TData(TFindFlags flags = TCoolTextWnd::ffNone, int buffSize = 81);
        ~TData();
        static void AddDefaultEngines(TData& data);

        //void     Write(opstream& os);
        //void     Read(ipstream& is);

      private:
        // Hidden to prevent accidental copying or assignment
        //
        TData(const TData&);
        TData& operator=(const TData&);
    };

  public:
    TCoolFindDlg(owl::TWindow* parent, TData& data, owl::TResId resId=0, owl::TModule* module = 0);
    virtual ~TCoolFindDlg ();

    void SetupWindow() override;
    void CleanupWindow() override;
    virtual owl::TConfigFile* CreateConfigFile();

  protected:
    owl::TComboBox*    FindTextBox;
    owl::TComboBox*    FindEngine;
    owl::TCheckBox*    FindWord;
    owl::TCheckBox*    FindCase;
    owl::TCheckBox*    FindGlobal;
    owl::TCheckBox*    FindInBlock;
    owl::TRadioButton* FindFrw;
    owl::TRadioButton* FindBack;


    TData&        Data;
    static owl::uint   MessageId;

  protected:
    void CmOk();
    void EvCommandEnableOk(owl::TCommandEnabler& tce);
    void EvCommandenableFindword(owl::TCommandEnabler& tce);
    void EvCommandenableFindcase(owl::TCommandEnabler& tce);
    void EvCommandenableFindfrw(owl::TCommandEnabler& tce);
    void EvCommandenableFindback(owl::TCommandEnabler& tce);
    void EvCommandenableFindglobal(owl::TCommandEnabler& tce);

    void GetSelection();

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TCoolFindDlg(const TCoolFindDlg&);
    TCoolFindDlg& operator=(const TCoolFindDlg&);

  DECLARE_RESPONSE_TABLE(TCoolFindDlg);
}; 
//{{TCoolFindDlg = TCoolReplaceDlg}}
class _COOLCLASS TCoolReplaceDlg: public TCoolFindDlg {
    typedef TCoolFindDlg Inherited;
  public:
    TCoolReplaceDlg(owl::TWindow* parent, TData& data, owl::TResId resId=0, owl::TModule* module = 0);
    virtual ~TCoolReplaceDlg ();

  protected:
    owl::TComboBox*  ReplaceTextBox;

  protected:
    void CmReplace();
    void CeReplace(owl::TCommandEnabler& tce);
    void CmReplaceAll();
    void CeReplaceAll(owl::TCommandEnabler& tce);

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TCoolReplaceDlg(const TCoolReplaceDlg&);
    TCoolReplaceDlg& operator=(const TCoolReplaceDlg&);

  DECLARE_RESPONSE_TABLE(TCoolReplaceDlg);
}; 
//-----------------------------------------------------
// class TCoolSearchWnd
// ~~~~~ ~~~~~~~~~~~~~~
//
//{{TCoolTextWnd = TCoolSearchWnd}}
class _COOLCLASS TCoolSearchWnd: public TCoolTextWnd {
  private:
    typedef TCoolTextWnd Inherited;
  public:
    TCoolSearchWnd(owl::TWindow* parent,int id,LPCTSTR title,int x, int y, int w, int h, owl::TModule* module = 0);
    ~TCoolSearchWnd();

    void SetupWindow() override;
    virtual void DoSearch();
    auto Search(const TEditPos& startPos, LPCTSTR, TFindFlags = ffNone) -> bool override;

    TCoolFindDlg::TData&  GetSearchData();
    TCoolFindDlg*         GetSearchDialog();
    void                  SetSearchDialog(TCoolFindDlg* searchdialog);
    owl::uint                  GetSearchCmd();
    void                  SetSearchCmd(owl::uint searchcmd);

  protected:
    void    CmEditFind();               // CM_EDITFIND
    void    CmEditFindNext();           // CM_EDITFINDNEXT
    void    CeEditFindNext(owl::TCommandEnabler& ce);
    owl::TResult EvFindMsg(owl::TParam1 param1, owl::TParam2 param2);  // Registered commdlg message
  // key handlers
  protected:

  public_data:
    owl::uint                SearchCmd;     // command set that opened dialog
    TCoolFindDlg::TData SearchData;
    TCoolFindDlg*       SearchDialog;  // Find or replace dialog
    TCoolSearchEngine*  SearchEngine;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TCoolSearchWnd(const TCoolSearchWnd&);
    TCoolSearchWnd& operator=(const TCoolSearchWnd&);

  DECLARE_RESPONSE_TABLE(TCoolSearchWnd);
}; 
////////////////////////////////////
// class TCoolEdit
// ~~~~~ ~~~~~~~~~
//
//{{TCoolSearchWnd = TCoolEdit}}
class _COOLCLASS TCoolEdit: public TCoolSearchWnd {
  private:
    typedef TCoolSearchWnd Inherited;
  public:
#if !defined(COOL_NODRAGDROP)
    class _COOLCLASS TEditDragDropProxy: public TDragDropProxy{
      public:
        TEditDragDropProxy(TCoolEdit* wnd);
        virtual void ShowDropIndicator(owl::TPoint& point);
        virtual void HideDropIndicator();  // Hide Drop Caret
        virtual owl::uint IsDroppable(const owl::TPoint& point, const owl::TRect& rect);
        virtual bool PerformDrop(IDataObject* dataObj, const owl::TPoint& point, DROPEFFECT de);

        virtual STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
    };
    friend class TEditDragDropProxy;
#endif
  public:
    TCoolEdit(owl::TWindow* parent,int id,LPCTSTR title,int x, int y, int w, int h, owl::TModule* module = 0);
    ~TCoolEdit();

  protected:
    void EvChar(owl::uint key, owl::uint repeatCount, owl::uint flags);
    void EvKeyDown(owl::uint key, owl::uint repeatCount, owl::uint flags);
    void CmEditReplace(); // CM_EDITREPLACE
    void CeEditFindReplace(owl::TCommandEnabler& tce);
    void CmEditDelete();
    void CmEditCut();
    void CmEditPaste();
    void CmEditClear();
    void CePasteEnable(owl::TCommandEnabler& tce);
    void CeCharsEnable(owl::TCommandEnabler& tce);
    void CmEditUndo();
    void CeUndoEnable(owl::TCommandEnabler& tce);
    void CmEditRedo();
    void CeRedoEnable(owl::TCommandEnabler& tce);
  protected:
    // smart Intending
    virtual int GetSmartPosition(const TEditPos& pos);
    void DoSearch() override;
#if !defined(COOL_NODRAGDROP)
    auto GetDropEffect() -> DROPEFFECT override;
    void DropSource(DROPEFFECT) override;
    void DropText(IDataObject* src, const owl::TPoint&, DROPEFFECT) override;
#endif



  // key handlers
  public:
    bool IsReadOnly() const;
    void EnableReadOnly(bool enable = true);
    void InsertChar(owl::uint key, owl::uint repeatCount=1);
    void KeyEnter(owl::uint repeatCount=1);
    void KeyUnTab(owl::uint repeatCount=1);
    void KeyTab(owl::uint repeatCount=1);
    void DeleteWordBack(owl::uint repeatCount=1);
    void DeleteCharBack(owl::uint repeatCount=1);
    void DeleteWord(owl::uint repeatCount=1);
    void DeleteChar(owl::uint repeatCount=1);
    int  DeleteSelection(owl::uint repeatCount=1);
    void Delete();
    bool Paste();
    void ClearAll();
    void Cut();
    void PasteFromMemory(_TCHAR* pMemory, int size, TSelType selection);
#if !defined(COOL_NODRAGDROP)
    void ShowDropIndicator(const owl::TPoint& point);
    void HideDropIndicator();
    void PastFromDataObject(IDataObject* dataObj);
#endif

  public:
    void InsertText(LPCTSTR text);
    void ReplaceText(LPCTSTR text);

  // data Members
  protected:
#if !defined(COOL_NODRAGDROP)
    TEditPos          SavedCaretPos;
#endif

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TCoolEdit(const TCoolEdit&);
    TCoolEdit& operator=(const TCoolEdit&);

  DECLARE_RESPONSE_TABLE(TCoolEdit);

    friend class TUndoInsertChar;
    friend class TRedoInsertChar;
    friend class TUndoKeyEnter;
    friend class TRedoKeyEnter;
    friend class TUndoKeyTab;
    friend class TRedoKeyTab;
    friend class TUndoKeyUnTab;
    friend class TRedoKeyUnTab;
    friend class TUndoDeleteChar;
    friend class TRedoDeleteChar;
    friend class TUndoDeleteWord;
    friend class TRedoDeleteWord;
    friend class TUndoDeleteCharBack;
    friend class TRedoDeleteCharBack;
    friend class TUndoDeleteWordBack;
    friend class TRedoDeleteWordBack;
    friend class TUndoDeleteSelection;
    friend class TRedoDeleteSelection;
    friend class TUndoCopy;
    friend class TRedoCopy;
    friend class TUndoPaste;
    friend class TRedoPaste;
    friend class TUndoClearAll;
    friend class TRedoClearAll;
    friend class TUndoDragDropMoveExt;
    friend class TRedoDragDropMoveExt;
    friend class TUndoDragDropCopy;
    friend class TRedoDragDropCopy;
    friend class TUndoDragDropMove;
    friend class TRedoDragDropMove;

}; 

////////////////////////////////////
// class TCoolEditFile
// ~~~~~ ~~~~~~~~~~~~~
//
//{{TCoolEdit = TCoolEditFile}}
class _COOLCLASS TCoolEditFile: public TCoolEdit {
  private:
    typedef TCoolEdit Inherited;
  public:
    TCoolEditFile(owl::TWindow* parent = 0,
                  int id = 0,
                  LPCTSTR title = 0,
                  int x = 0, int y = 0, int w = 0, int h = 0,
                  LPCTSTR fileName = 0,
                  owl::TModule* module = 0);
    ~TCoolEditFile();

    LPCTSTR       GetFileName() const;
    void          SetFileName(LPCTSTR fileName);
    owl::TOpenSaveDialog::TData& GetFileData();
    void   SetFileData(const owl::TOpenSaveDialog::TData& fd);

  protected:
    void CmFileNew();     // CM_FILENEW
    void CmFileOpen();    // CM_FILEOPEN
    void CmFileSave();    // CM_FILESAVE
    void CmFileSaveAs();  // CM_FILESAVEAS
    void CmSaveEnable(owl::TCommandEnabler& tce);
  public:
    void FileNew() override;
    auto FileOpen(LPCTSTR filename) -> bool override;
    virtual bool FileSave();
    virtual bool FileSaveAs();
    virtual bool CanClear();
    auto CanClose() -> bool override;
    void SetupWindow() override;

  // key handlers
  protected:

  public_data:
    owl::TOpenSaveDialog::TData FileData;
    LPTSTR FileName;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TCoolEditFile(const TCoolEditFile&);
    TCoolEditFile& operator=(const TCoolEditFile&);

  DECLARE_RESPONSE_TABLE(TCoolEditFile);

}; 
////////////////////////////////////
// class TCoolSearchView
// ~~~~~ ~~~~~~~~~~~~~~
//
//{{TCoolSearchWnd = TCoolSearchView}}
class _COOLCLASS TCoolSearchView: public TCoolSearchWnd, public owl::TView {
  public:
    TCoolSearchView(owl::TDocument& doc, owl::TWindow* parent = 0);
    virtual ~TCoolSearchView();

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
    TCoolSearchView(const TCoolSearchView&);
    TCoolSearchView& operator=(const TCoolSearchView&);

  DECLARE_RESPONSE_TABLE(TCoolSearchView);
}; 

////////////////////////////////////
// class TCoolEditView
// ~~~~~ ~~~~~~~~~~~~~
//
//{{TCoolEdit = TCoolEditView}}
class _COOLCLASS TCoolEditView: public TCoolEdit, public owl::TView {
  public:
    TCoolEditView(owl::TDocument& doc, owl::TWindow* parent = 0);
    virtual ~TCoolEditView();

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
    TCoolEditView(const TCoolEditView&);
    TCoolEditView& operator=(const TCoolEditView&);

  DECLARE_RESPONSE_TABLE(TCoolEditView);
}; 
//-----------------------------------------------------------------------------
//
// inlines
#include <coolprj/cooledit.inl>
//
//-----------------------------------------------------------------------------

/*============================================================================*/
#endif // COOLEDIT_H
