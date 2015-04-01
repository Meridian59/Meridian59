//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TRichEdit.
//----------------------------------------------------------------------------

#if !defined(OWL_RICHEDIT_H)
#define OWL_RICHEDIT_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/editfile.h>
#include <owl/commctrl.h>

#include <richedit.h>
#include <richole.h>

namespace owl {

//
// Macro representing all CHARFORMAT flags
//
#if !defined(CFM_ALL)
#define CFM_ALL   (CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT |\
                   CFM_PROTECTED | CFM_SIZE | CFM_COLOR | CFM_FACE | \
                   CFM_OFFSET | CFM_CHARSET)
#endif

//
// Macro representing all the PARAFORMAT flags
//
#if !defined(PFM_ALL)
#define PFM_ALL (PFM_ALIGNMENT|PFM_NUMBERING|PFM_OFFSET|PFM_OFFSETINDENT|\
                 PFM_RIGHTINDENT|PFM_STARTINDENT|PFM_TABSTOPS)
#endif

//
// Forward References
//
class _OWLCLASS TRichEdit;

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup ctrl
/// @{
/// \class TCharFormat
// ~~~~~ ~~~~~~~~~~~
/// TCharFormat encapsulates the CHARFORMAT2 structure which contains
/// information about character formatting in a rich edit control.
//
class _OWLCLASS TCharFormat : public CHARFORMAT2 {
  public:
    TCharFormat(ulong mask = 0);
    TCharFormat(const TRichEdit&, bool selection = true,
                ulong mask = CFM_ALL);

    void    ToggleEffectsBit(ulong flag);
    void    ToggleMaskBit(ulong flag);

    void    EnableBold(bool = true);
    void    EnableItalic(bool = true);
    void    EnableUnderline(bool = true);
    void    EnableStrikeOut(bool = true);
    void    EnableProtected(bool = true);

    void    SetHeight(long);
    void    SetOffset(long);
    void    SetTextColor(const TColor& = TColor::None);
    void    SetCharSet(uint8);
    void    SetPitchAndFamily(uint8);
    void    SetFaceName(LPCTSTR);
    void SetFaceName(const tstring& f) {SetFaceName(f.c_str());}

    TColor  GetTextColor() const;

    void    SetFontInfo(LOGFONT& lf) const;
    void    GetFontInfo(const LOGFONT& lf);

    // RichEdit 2.0 specific
    //
    void    SetWeight(uint16 weigh);      ///< Font weight (LOGFONT value)
    void    SetSpacing(int16 spacing);    ///< Amount to space between letters
    void    SetBkColor(const TColor& clr);///< Background color
    void    SetLCID(LCID lcid);            ///< Locale ID
    void    SetStyle(int16 style);        ///< Style handle
    void    SetKerning(uint16 kern);      ///< Twip size above which to kern char pair
    void    SetUnderlineType(uint8 utype);///< Underline type
    void    SetAnimation(uint8 anim);      ///< Animated text like marching ants
    void    SetRevAuthor(uint8 revav);    ///< Revision author index
};

//
/// \class TCharRange
// ~~~~~ ~~~~~~~~~~
/// TCharRange encapsulates the CHARRANGE structure, which specifies a range of
/// characters in a rich edit control.
/// Can convert to and from TEdit::TRange.
//
class _OWLCLASS TCharRange : public CHARRANGE {
  public:
    TCharRange(int minValue = 0, int maxValue = -1);
    TCharRange(const TEdit::TRange& r) {cpMin = r.cpMin; cpMax = r.cpMax;}

    operator TEdit::TRange() {return TEdit::TRange(cpMin, cpMax);}
};

//
/// \class TParaFormat
// ~~~~~ ~~~~~~~~~~~
/// TParaFormat encapsulates the PARAFORMAT structure, which contains information
/// about paragraph formatting attributes of a rich edit control.
//
class _OWLCLASS TParaFormat : public PARAFORMAT2 {
  public:
    TParaFormat(ulong mask = 0);
    TParaFormat(const TRichEdit&, ulong mask = PFM_ALL);

    void  ToggleMaskBit(ulong flag);

    void  SetNumbering(uint16);
    void  SetStartIndent(long, bool relative = false);
    void  SetRightIndent(long);
    void  SetOffset(long);
    void  SetAlignment(uint16);
    void  SetTabCount(short, long*);

    // RichEdit 2.0 specific
    //
    void  SetSpaceBefore(long space);     ///< Vertical spacing before para
    void  SetSpaceAfter(long space);       ///< Vertical spacing after para
    void  SetLineSpacing(long space, uint8 rule);///< Line spacing depending on Rule
    void  SetStyle(int16 style);          ///< Style handle
    void  SetShading(uint16 wight,uint16 style); ///< Shading in hundredths of a per cent
                                                 ///< Nibble 0: style, 1: cfpat, 2: cbpat
    void  SetNumStart(uint16 start);      ///< Starting value for numbering
    void  SetNumStyle(uint16 style);      ///< Alignment, roman/arabic, (), ), ., etc.
    void  SetNumTab(uint16 tab);          ///< Space bet 1st indent and 1st-line text
    // Space between border and text (twips)
    // Border pen width (twips)
    // Byte 0: bits specify which borders, Nibble 2: border style, 3: color index
    void  SetBorder(uint16 spac, uint16 width, uint16 border);
};

//
/// \class TFormatRange
// ~~~~~ ~~~~~~~~~~~~
/// TFormatRange encapsulates the FORMATRANGE structure, which contains information
/// that a rich edit control uses to format its output for a particular device.
//
class _OWLCLASS TFormatRange : public FORMATRANGE {
  public:
    TFormatRange();
    TFormatRange(HDC renderDC, HDC targetDC,
                 const TRect& renderArea, const TRect& entireArea,
                 const TCharRange& txtRange);

    TFormatRange(HDC renderDC, HDC targetDC,
                 const TRect& renderArea, const TRect& entireArea,
                 long firstChar, long lastChar);

    void  SetRenderDC(HDC);
    void  SetTargetDC(HDC);
    void  SetRenderRect(const TRect&);
    void  SetPageRect(const TRect&);
    void  SetRange(const TCharRange&);
    void  SetRange(long, long);
};

//
/// \class TEditStream
// ~~~~~ ~~~~~~~~~~~
/// TEditStream encapsulates the EDITSTREAM structure, which contains information
/// about a data stream used with a rich edit control.
//
class _OWLCLASS TEditStream : public EDITSTREAM {
  public:
    TEditStream();
    TEditStream(DWORD_PTR cookie, EDITSTREAMCALLBACK);
};

//
/// \class TTextRange
// ~~~~~ ~~~~~~~~~~
/// TTextRange encapsulates the TEXTRANGE structure, which contains information
/// about a range of text in a rich edit control.
//
class _OWLCLASS TTextRange : public ::TEXTRANGE {
  public:
    TTextRange();
    TTextRange(const TCharRange&, LPTSTR buffer);
    TTextRange(int beg, int end, LPTSTR buffer);
};

//
/// \class TFindText
// ~~~~~ ~~~~~~~~~
/// TFindText encapsulates the FINDTEXT structure, which contains information about
/// text to search for in a rich edit control.
//
class _OWLCLASS TFindText : public ::FINDTEXT {
  public:
    TFindText(const TCharRange& range, const tstring& text);
    TFindText(int beg, int end, const tstring& text);
  private:
    tstring t;
};

//----------------------------------------------------------------------------

//
/// \class TRichEditModule
// ~~~~~ ~~~~~~~~~~~~~~~
/// Wrapper object which loads the DLL providing the implementation of
/// the RichEdit control
//
class _OWLCLASS TRichEditModule : public TModule {
  public:
    TRichEditModule();

    static int GetVersion(bool force_old = false);
};

//
/// TRichEditDll is a simple object which takes advantages of OWL's
/// TDllLoader to ensure that only one copy of an object [theoretically
/// representing a DLL] is created.
/// TRichEditDll provides the 'IsAvailable' method which returns true if the
/// DLL is available and loaded. TRichEditDll is used internally by OWL's
/// TRichEdit class to ensure that the DLL is available and loaded. You may
/// consider using TRichEditDll if your application creates rich edit controls
/// without instantiating a TRichEdit object. (For example, if the control
/// is part of a dialog resource). The following snippet illustrates:
/// \code
///    if (!TRichEditDll::IsAvailable()) {
///      Error("Unable to load Rich Edit DLL");
///    }
/// \endcode
typedef TDllLoader<TRichEditModule> TRichEditDll;

#if defined(_OWLDLL) || defined(BI_APP_DLL)
  //
  // Export template of TDllLoader<TRichEditModule> when building ObjectWindows
  // DLL and provide import declaration of DLL instance for users of the class.
  //
//  template class _OWLCLASS TDllLoader<TRichEditModule>;
#endif


//
/// \class TRichEdit
// ~~~~~ ~~~~~~~~~
/// TRichEdit encapsulates a rich edit control, a window in which a user can enter,
/// edit and format text.
//
class _OWLCLASS TRichEdit : public TEditFile {
  public:
    TRichEdit(TWindow*        parent,
              int             id,
              LPCTSTR         text,
              int x, int y, int w, int h,
              LPCTSTR         fileName = 0,
              TModule*        module = 0);

    TRichEdit(
      TWindow* parent,
      int id,
      const tstring& text,
      int x, int y, int w, int h,
      const tstring& fileName = tstring(),
      TModule* = 0);

    TRichEdit(TWindow*   parent,
              int        resourceId,
              TModule*   module = 0);

    // Generic Formatting Methods
    //
    ulong   GetCharFormat(TCharFormat &, bool selection = false) const;
    ulong   GetParaFormat(TParaFormat &) const;
    bool    SetCharFormat(const TCharFormat &, uint flags= SCF_SELECTION);
    bool    SetParaFormat(const TParaFormat &);
    TColor  SetBkgndColor(const TColor& = TColor::None);

    /// Enumeration of flags returned when querrying a RichEdit about
    /// a particular character attribute for a chunk of selected text.
    //
    enum TFmtStatus {
      No,               ///< The whole selection has the attribute
      Yes,              ///< The attribute is absent from the selection
      Partly            ///< Part of the selection has the attribute
    };

    // More specialized character formatting or querrying methods
    //
    uint    HasCharAttribute(ulong mask, uint32 effects);
    bool    ToggleCharAttribute(ulong mask, uint32 effects);
    bool    ChangeCharPointSize(int pointSizeDelta);

// !BB    Consider adding following font methods - investigate optimal
// !BB    encapsulation which can be flexible and easily overriden
// !BB    bool    SetFont(const char * fntName);
// !BB    bool    SetFont(const LOGFONT& lf);
// !BB
// !BB    bool    GetFont(char * fntName, int size);
// !BB    bool    GetFont(LOGFRONT& lf);

    // Selection and Hit Testing
    //
    bool HasSelection() const;
    void GetSelRange(TCharRange &) const; // Deprecated; use GetSelection instead.
    int SetSelRange(const TCharRange &); // Deprecated; use SetSelection instead.
    void HideSelection(bool hide, bool changeStyle);
    ulong GetSelectionType() const;

    // TEdit overrides
    //
    virtual void GetSelection(int& startPos, int& endPos) const;
    using TEditFile::GetSelection; // Inject TRange-based overload.
    virtual bool SetSelection(int startPos, int endPos);
    using TEditFile::SetSelection; // Inject TRange-based overload.

    // Various settings
    //
    bool    EnableAutoURL(bool enable=true);    // RichEdit 2.0 specific
    int      GetIMEMode() const;                 // RichEdit 2.0 specific
    void    SetLangOptions(int options);        // RichEdit 2.0 specific
    int      GetLangOptions() const;             // RichEdit 2.0 specific
    void    SetTextMode(int mode);              // RichEdit 2.0 specific
    int      GetTextMode() const;                // RichEdit 2.0 specific
    void    SetUndoLimit(int maxnum);           // RichEdit 2.0 specific
    void    StopGroupTyping();                  // RichEdit 2.0 specific

    // Text Operations
    //
    int GetTextLength() const;
    int GetTextRange(TTextRange &) const; // deprecated
    int GetTextRange(const TCharRange &, LPTSTR buffer) const; // deprecated
    int GetSelectedText(LPTSTR buffer) const; // deprecated
    tstring GetSelectedText() const;
    int FindText(uint flags, const TFindText &);
    int FindText(uint flags, const TCharRange &, LPCTSTR text);
    int FindText(uint flags, const TCharRange& r, const tstring& text) {return FindText(flags, r, text.c_str());}

    // TEdit overrides
    //
    virtual tstring GetTextRange(const TRange &) const;
    virtual void GetSubText(LPTSTR textBuf, int startPos, int endPos) const; // deprecated
    virtual int Search(int startPos, LPCTSTR text, bool caseSensitive=false, bool wholeWord=false, bool up=false);
    int Search(int startPos, const tstring& text, bool caseSensitive=false, bool wholeWord=false, bool up=false)
    {return TEdit::Search(startPos, text, caseSensitive, wholeWord, up);}
    virtual void LimitText(int maxValue);
    virtual int GetLineFromPos(int charPos) const;       // EM_EXLINEFROMCHAR
    virtual uint32 CharFromPos(int16 x, int16 y);
    virtual uint32 PosFromChar(uint charIndex);

    // Word and Line Breaks
    //
    int     FindWordBreak(uint code, int start);

    // Lines and Scrolling
    //
// !BB    int     GetNumLines() const;
// !BB    int     GetLineLength(int lineNumber) const;
// !BB    bool    GetLine(char * str, int strSize, int lineNumber) const;
// !BB    uint    GetLineIndex(int lineNumber) const;
// !BB    void    Scroll(int horizontalUnit, int verticalUnit);

    // Editing Operations
    //
    bool    CanPaste(uint format) const;
    bool    CanRedo() const;     // RichEdit 2.0 specific
    int     GetUndoName() const; // RichEdit 2.0 specific
    int     GetRedoName() const; // RichEdit 2.0 specific
    void    Redo();              // RichEdit 2.0 specific
    void    PasteSpecial(uint format);

    // TEdit overrides
    //
    virtual void Paste();

    // Streams
    //
    ulong   StreamIn(uint format, TEditStream &);
    ulong   StreamOut(uint format, TEditStream &);

    // Printing
    //
    bool    DisplayBand(TRect &);
    int     FormatRange(const TFormatRange & range, bool render = true);
    int     FormatRange();
    bool    SetTargetDevice(HDC, int lineWidth);

    // Bottomless Rich Edit Controls
    //
    void    RequestResize();

    // OLE Interfaces
    //
    bool    GetOleInterface(IRichEditOle * &) const;
    bool    SetOleInterface(IRichEditOleCallback *);

    // Querry/Set control's data format
    //
    uint    GetFormat() const;
    void    SetFormat(uint fmt);
    bool    IsRTF() const;

    // Misc
    //
    ulong   GetEventMask() const;
    ulong   SetEventMask(ulong msk);
// !BB    void    SetReadOnly(bool readOnly);
// !BB    bool    IsModified() const;
// !BB    void    ClearModify() {HandleMessage(EM_SETMODIFY);}
// !BB    void    SetModify();      // !BB Add to TEdit ??

    // Override to use RichEdit streaming capabilities
    //
    virtual uint Transfer(void* buffer, TTransferDirection direction);

    // Override TEditFile's I/O
    //
    virtual bool Read(LPCTSTR fileName=0);
    using TEditFile::Read; ///< String-aware overload

    virtual bool Write(LPCTSTR fileName=0);
    using TEditFile::Write; ///< String-aware overload

    bool    ReadFromStream(tistream&, uint format = SF_RTF);
    bool    WriteToStream(tostream&, uint format = SF_RTF);

  protected:

    // Data format of control
    //
    uint    Format;

    // Command response functions
    //
    void    CmEditCut();       // CM_EDITCUT
    void    CmEditCopy();      // CM_EDITCOPY
    void    CmEditPaste();     // CM_EDITPASTE
    void    CmEditDelete();    // CM_EDITDELETE
    void    CmEditClear();     // CM_EDITCLEAR
    void    CmEditUndo();      // CM_EDITUNDO

    // Command enabler functions
    //
    void    CeHasSelect(TCommandEnabler& commandHandler);
    void    CeEditPaste(TCommandEnabler& commandHandler);
    void    CeEditClear(TCommandEnabler& commandHandler);
    void    CeEditUndo(TCommandEnabler& commandHandler);

    // Child id notification handled at the child
    //
    void    ENErrSpace();  // EN_ERRSPACE

    // Override TWindow virtual member functions
    //
    virtual TGetClassNameReturnType GetClassName();
    void      SetupWindow();

    // Event handlers to override TEdit's handlers
    //
    void         EvChar(uint key, uint repeatCount, uint flags);
    void         EvKeyDown(uint key, uint repeatCount, uint flags);
    uint         EvGetDlgCode(const MSG*);
    void         EvSetFocus(HWND hWndLostFocus);
    void         EvKillFocus(HWND hWndGetFocus);
    void         EvChildInvalid(HWND);

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TRichEdit(const TRichEdit&);
    TRichEdit& operator =(const TRichEdit&);

    // The following are TEdit methods which encapsulates messages/API
    // which are *NOT* supported by the RICHEDIT class.
    //
    void    FormatLines(bool addEOL);
    LPTSTR  LockBuffer(uint newSize = 0);
    void    UnlockBuffer(LPCTSTR, bool = false);
    void    SetTabStops(int numTabs, const int *);
    HLOCAL  GetMemHandle() const;             // Yura 06/15/98
    void    SetMemHandle(HLOCAL localMem);    // Yura 06/15/98
    uint    GetPasswordChar() const;
    void    SetPasswordChar(uint ch);
    void    SetRectNP(const TRect& frmtRect);

    void    SetLeftMargin(uint16 margin);     // Yura 19/23/98
    void    SetRightMargin(uint16 margin);    // Yura 19/23/98
    void    SetMarginUseFontInfo();           // Yura 19/23/98
    uint32   GetMargins() const;               // Yura 19/23/98

    // Validators are not applicable to RichEdit where the text
    // is not plain ASCII...
    //
    bool    IsValid(bool reportErr = false);
    void    SetValidator(TValidator* validator);
    void    ValidatorError();

  DECLARE_RESPONSE_TABLE(TRichEdit);
   //DECLARE_STREAMABLE(_OWLCLASS, owl::TRichEdit, 1);
  DECLARE_STREAMABLE_OWL(TRichEdit, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TRichEdit );

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementation
//

//
/// Handler of the CM_EDITCUT command - Invokes the 'Cut' method.
//
inline void TRichEdit::CmEditCut()
{
  Cut();
}

//
/// Handler of the CM_EDITCOPY command - invokes the'Copy' method
//
inline void TRichEdit::CmEditCopy()
{
  Copy();
}

//
/// Handler of the CM_EDITPASTE command - invokes the 'Paste' method
//
inline void TRichEdit::CmEditPaste()
{
  Paste();
}

//
/// Handler of the CM_EDITDELETE command - invokes the 'DeleteSelection' method
//
inline void TRichEdit::CmEditDelete()
{
  DeleteSelection();
}

//
/// Handler of the CM_EDITCLEAR command - invokes the 'Clear' method
//
inline void TRichEdit::CmEditClear()
{
  Clear();
}

//
/// Handler of the CM_EDITUNDO command - invokes the 'Undo' method
//
inline void TRichEdit::CmEditUndo()
{
  Undo();
}

//
/// Constructor of a 'TCharFormat' structure initialized with the specified mask. 
/// \note This constructor is typically used for constructing an
/// object which will be filled with the appropriate character format
/// information by a rich edit control. The 'mask' specifies which field
/// should be filled.
//
inline TCharFormat::TCharFormat(ulong mask)
{
  cbSize = sizeof(CHARFORMAT);
  dwMask = mask;
}

//
/// Toggles the effect bits specified in the 'flag' parameter.
//
inline void TCharFormat::ToggleEffectsBit(ulong flag)
{
  dwEffects ^= flag;
}

//
/// Toggles the mask bits specified in the 'flag' parameter.
//
inline void TCharFormat::ToggleMaskBit(ulong flag)
{
  dwMask ^= flag;
}

//
/// Constructs a TCharRange structure initialized with the specified 'minValue' and 'maxValue' parameters.
//
inline TCharRange::TCharRange(int minValue, int maxValue)
{
  cpMin = minValue;
  cpMax = maxValue;
}

//
/// Constructs a default 'TEditStream' object. The members of the base 'EDITSTREAM'
/// object are initialized to zero.
//
inline TEditStream::TEditStream()
{
  dwCookie = 0;
  dwError  = 0;
  pfnCallback = 0;
}

//
/// Constructs a 'TEditStream' object initializing the members with the specified
/// parameter.
//
inline TEditStream::TEditStream(DWORD_PTR cookie, EDITSTREAMCALLBACK callback)
{
  dwCookie = cookie;
  dwError  = 0;
  pfnCallback = callback;
}

//
/// Constructs a 'TFindText' describing the text to search for and the range of the
/// search using the 'text' and 'range' parameters respectively.
//
inline TFindText::TFindText(const TCharRange& range, const tstring& text)
: t(text)
{
  chrg = range;
  lpstrText = &t[0];
}

//
/// Constructs a 'TFindText' describing the text to search for via the 'text'
/// parameter, and the range to search for via the 'beg' and 'end' parameters,
/// respectively.
//
inline TFindText::TFindText(int beg, int end, const tstring& text)
: t(text)
{
  chrg.cpMin = beg;
  chrg.cpMax = end;
  lpstrText = &t[0];
}

//
/// Constructs a default 'TFormatRange' object with all members initialized to zero.
//
inline TFormatRange::TFormatRange()
{
  memset((FORMATRANGE*)this, 0, sizeof(FORMATRANGE));
}

//
/// Constructs a default 'TTextRange' structure with the data members describing the
/// range and text initialized to zero.
//
inline TTextRange::TTextRange()
{
  chrg.cpMin = 0;
  chrg.cpMax = 0;
  lpstrText  = 0;
}

//
/// Constructs a 'TTextRange' structure by initializing the range and text members
/// with the 'rng' and 'buffer' parameters respectively.
//
inline TTextRange::TTextRange(const TCharRange& rng, LPTSTR buffer)
{
  chrg = rng;
  lpstrText = buffer;
}

//
/// Constructs a 'TTextRange' structure by initializing the text with the 'buffer'
/// parameter, and the range using 'beg' and 'end'.
//
inline TTextRange::TTextRange(int beg, int end, LPTSTR buffer)
{
  chrg.cpMin = beg;
  chrg.cpMax = end;
  lpstrText = buffer;
}

//
//
//
inline uint
TRichEdit::GetFormat() const {
  return Format;
}

//
//
//
inline void
TRichEdit::SetFormat(uint fmt) {
  Format = fmt;
}

//
//
//
inline bool
TRichEdit::IsRTF() const {
  return Format == SF_RTF;
}


} // OWL namespace


#endif  // OWL_RICHEDIT_H
