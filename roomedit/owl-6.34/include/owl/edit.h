//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TEdit.  This defines the basic behavior of all edit
/// controls.
//----------------------------------------------------------------------------

#if !defined(OWL_EDIT_H)
#define OWL_EDIT_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/static.h>
#include <owl/edit.rh>
#include <utility>


namespace owl {

class _OWLCLASS TValidator;
class _OWLCLASS TNmUpDown;

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup ctrl
/// @{
/// \class TEdit
// ~~~~~ ~~~~~
class _OWLCLASS TEdit : public TStatic {
  public:
    TEdit(TWindow* parent, int id, LPCTSTR text, int x, int y, int w, int h, 
      uint textLimit = 0, bool multiline = false, TModule* = 0);
    TEdit(TWindow*  parent, int id, const tstring& text, int x, int y, int w, int h,
      uint textLimit = 0, bool multiline = false, TModule* = 0);
    TEdit(TWindow* parent, int resourceId, uint textLimit = 0, TModule* = 0);
    TEdit(THandle hWnd, TModule* = 0);

    ~TEdit();

    /// Represents a half-open range of positions in the edit control, e.g. a selection range.
    /// The default range is [0, -1) for compatibility with TCharRange (see "richedit.h").
    //
    struct TRange
    {
      TRange(int b = 0, int e = -1) : cpMin(b), cpMax(e) {}

      int GetSize() const {return cpMax - cpMin;}

      int cpMin, cpMax;
    };

    /// \name Accessors
    /// @{
    int     GetNumLines() const;
    int     GetLineLength(int lineNumber) const;
    bool    GetLine(LPTSTR str, int strSize, int lineNumber) const;
    tstring GetLine(int lineNumber) const;
    virtual void GetSelection(int& startPos, int& endPos) const;
    TRange GetSelection() const;
    virtual int GetCurrentPosition() const;
    virtual tstring GetTextRange(const TRange &) const;
    /// @}

    /// Deprecated. Use GetTextRange instead.
    //
    virtual void GetSubText(LPTSTR textBuf, int startPos, int endPos) const;

    bool    IsModified() const;
    void    ClearModify();

    virtual int GetLineFromPos(int charPos) const;
    int     GetLineIndex(int lineNumber) const;

    uint    Transfer(void* buffer, TTransferDirection direction);

    //@{
    /// Lock and unlock this edit control's buffer. Allows direct access to the
    /// text in the edit control.
    //
    LPTSTR  LockBuffer(uint newSize = 0);
    void    UnlockBuffer(LPCTSTR buffer, bool updateHandle = false);
    //@}

    /// \name Operations
    //@{
    //
    bool    DeleteSubText(int startPos, int endPos);
    bool    DeleteSubText(const TRange& r) {return DeleteSubText(r.cpMin, r.cpMax);}
    bool    DeleteLine(int lineNumber);
    bool    DeleteSelection();
    virtual bool SetSelection(int startPos, int endPos);
    bool    SetSelection(const TRange& r) {return SetSelection(r.cpMin, r.cpMax);}
    bool ClearSelection() {return SetSelection(-1, -1);}
    //@}

    //
    /// \name Scrolling text
    //@{
    //
    void    Scroll(int horizontalUnit, int verticalUnit);
    void    ScrollCaret();      ///< EM_SCROLLCARET
    int      LineDown();          ///< EM_SCROLL, SB_LINEDOWN  Scrolls down one line.
    int      LineUp();           ///< EM_SCROLL, SB_LINEUP  Scrolls up one line.
    int      PageDown();         ///< EM_SCROLL, SB_PAGEDOWN  Scrolls down one page.
    int      PageUp();           ///< EM_SCROLL, SB_PAGEUP  Scrolls up one page.
    //@}

    void    Insert(LPCTSTR str);
    void    Insert(const tstring& str) {Insert(str.c_str());}
    virtual int Search(int startPos, LPCTSTR text, bool caseSensitive=false, bool wholeWord=false, bool up=false);
    int Search(int startPos, const tstring& text, bool caseSensitive=false, bool wholeWord=false, bool up=false)
    {return Search(startPos, text.c_str(), caseSensitive, wholeWord, up);}

    void    GetRect(TRect& frmtRect) const;
    void    SetRect(const TRect& frmtRect);
    void    SetRectNP(const TRect& frmtRect);
    bool    FormatLines(bool addEOL);
    void    SetTabStops(int numTabs, const int * tabs);

    virtual void LimitText(int);
    virtual int GetLimitText() const;

    HLOCAL  GetMemHandle() const;
    void    SetMemHandle(HLOCAL localMem);

    void    SetPasswordChar(uint ch);

    int     GetFirstVisibleLine() const;
    void    SetReadOnly(bool readOnly);
    uint    GetPasswordChar() const;

    EDITWORDBREAKPROC GetWordBreakProc() const;
    void    SetWordBreakProc(EDITWORDBREAKPROC proc);

    /// \name Clipboard operations
    //@{
    bool    CanUndo() const;
    void    EmptyUndoBuffer();
    void    Undo();
    virtual void Paste();
    void    Copy();
    void    Cut();
    //@}

    /// \name Validator functions
    //@{
    bool    IsValid(bool reportErr = false);
    TValidator*  GetValidator();
    void    SetValidator(TValidator* validator);
    void    ValidatorError();
    bool    IsRefocusing() const;
    //@}

    /// \name Margin functions
    //@{
    void    SetLeftMargin(uint16 margin);
    void    SetRightMargin(uint16 margin);
    void    SetMarginUseFontInfo();
    uint32  GetMargins() const;
    //@}

    /// \name Position and character functions
    //@{
    virtual uint32  CharFromPos(int16 x, int16 y);
    virtual uint32  PosFromChar(uint charIndex);
    //@}

    /// Override TStatic virtual member functions
    //
    void Clear();

  protected:
    /// \name Command response functions for edit menu items
    /// @{
    void    CmEditCut();      ///< CM_EDITCUT
    void    CmEditCopy();     ///< CM_EDITCOPY
    void    CmEditPaste();    ///< CM_EDITPASTE
    void    CmEditDelete();   ///< CM_EDITDELETE
    void    CmEditClear();    ///< CM_EDITCLEAR
    void    CmEditUndo();     ///< CM_EDITUNDO
    /// @}

    /// \name Command enabler functions for edit menu items
    // @{
    void CmSelectEnable(TCommandEnabler& commandHandler);
    void CmPasteEnable(TCommandEnabler& commandHandler);
    void CmCharsEnable(TCommandEnabler& commandHandler);
    void CmModEnable(TCommandEnabler& commandHandler);
    // @}

    /// \name Child id notification handled at the child
    /// @{
    void    ENErrSpace();  ///< EN_ERRSPACE
    // @}

    /// \name Override TWindow virtual member functions
    /// @{
    virtual TGetClassNameReturnType GetClassName();
    void      SetupWindow();

    void         EvChar(uint key, uint repeatCount, uint flags);
    void         EvKeyDown(uint key, uint repeatCount, uint flags);
    uint         EvGetDlgCode(const MSG*);
    void         EvSetFocus(HWND hWndLostFocus);
    void         EvKillFocus(HWND hWndGetFocus);
    bool         EvUpDown(TNmUpDown &);
    bool         CanClose();
    /// @}

    /// Handler for input validation message sent by parent
    //
    void         EvChildInvalid(HWND);

    int            ScrollText(int how);

  protected_data:
    /// Input validation object
    //
    TValidator*  Validator;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TEdit(const TEdit&);
    TEdit& operator =(const TEdit&);

    static TEdit* ValidatorReFocus;

  DECLARE_RESPONSE_TABLE(TEdit);
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TEdit, 1);
  DECLARE_STREAMABLE_OWL(TEdit, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TEdit );

//
// edit control notification macros. methods are: void method()
//
// EV_EN_CHANGE(id, method)
// EV_EN_ERRSPACE(id, method)
// EV_EN_HSCROLL(id, method)
// EV_EN_KILLFOCUS(id, method)
// EV_EN_MAXTEXT(id, method)
// EV_EN_SETFOCUS(id, method)
// EV_EN_UPDATE(id, method)
// EV_EN_VSCROLL(id, method)

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Resets the change flag of the edit control causing IsModified to return false. 
/// The flag is set when text is modified.
//
inline void TEdit::ClearModify()
{
  PRECONDITION(GetHandle());
  SendMessage(EM_SETMODIFY);
}

//
/// SetPasswordChar sets the character to be displayed in place of a user-typed character. 
/// When the ES_PASSWORD style is specified, the default display character is an asterisk (*).
//
inline void TEdit::SetPasswordChar(uint ch)
{
  PRECONDITION(GetHandle());
  SendMessage(EM_SETPASSWORDCHAR, ch);
}

//
/// If an operation inside the edit control can be undone, 
/// the edit control undo flag is set. EmptyUndoBuffer resets or clears this flag.
//
inline void TEdit::EmptyUndoBuffer()
{
  PRECONDITION(GetHandle());
  SendMessage(EM_EMPTYUNDOBUFFER);
}

//
/// Undoes the last edit.
//
inline void TEdit::Undo()
{
  PRECONDITION(GetHandle());
  SendMessage(WM_UNDO);
}

//
/// Inserts text from the Clipboard into the edit control at the current text insertion point (cursor position).
//
inline void TEdit::Paste()
{
  PRECONDITION(GetHandle());
  SendMessage(WM_PASTE);
}

//
/// Copies the currently selected text into the Clipboard.
//
inline void TEdit::Copy()
{
  PRECONDITION(GetHandle());
  SendMessage(WM_COPY);
}

//
/// Deletes the currently selected text and copies it into the Clipboard.
//
inline void TEdit::Cut()
{
  PRECONDITION(GetHandle());
  SendMessage(WM_CUT);
}

//
/// Return the validator associated with this edit control.
//
inline TValidator* TEdit::GetValidator()
{
  return Validator;
}

//
/// Return true when this edit control is attempting to regain focus after an
/// EvKillFocus() with invalid contents
//
inline bool TEdit::IsRefocusing() const
{
  return ValidatorReFocus == this;
}

//
/// Automatically responds to a menu selection with a menu ID of CM_EDITCUT by calling Cut().
//
inline void TEdit::CmEditCut()
{
  Cut();
}

//
/// Automatically responds to a menu selection with a menu ID of CM_EDITCOPY by calling Copy().
//
inline void TEdit::CmEditCopy()
{
  Copy();
}

//
/// Automatically responds to a menu selection with a menu ID of CM_EDITPASTE by calling Paste().
//
inline void TEdit::CmEditPaste()
{
  Paste();
}

//
/// Automatically responds to a menu selection with a menu ID of CM_EDITDELETE by calling DeleteSelection().
//
inline void TEdit::CmEditDelete()
{
  DeleteSelection();
}

//
/// Automatically responds to a menu selection with a menu ID of CM_EDITCLEAR by calling Clear().
//
inline void TEdit::CmEditClear()
{
  Clear();
}

//
/// Automatically responds to a menu selection with a menu ID of CM_EDITUNDO by calling Undo().
//
inline void TEdit::CmEditUndo()
{
  Undo();
}

//
/// Return the number of lines in the associated edit control
//
/// \note return 1 when the edit control has no text (i.e. it has one line
/// with no text in it). Return zero if an error occurs
//
inline int TEdit::GetNumLines() const
{
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TEdit*,this)->SendMessage(EM_GETLINECOUNT);
}

//
/// Returns true if the user has changed the text in the edit control.
//
inline bool TEdit::IsModified() const
{
  PRECONDITION(GetHandle());
  return (bool)CONST_CAST(TEdit*,this)->SendMessage(EM_GETMODIFY);
}

//
/// Select the characters in the range "startPos .. endPos"
//
/// Forces the selection of the text between the positions specified by startPos and
/// endPos, but not including the character at endPos.
//
inline bool TEdit::SetSelection(int startPos, int endPos)
{
  PRECONDITION(GetHandle());
  return SendMessage(EM_SETSEL, startPos, endPos) != 0;
}

//
/// Return the starting and ending positions of the selected text
//
/// Returns the starting (startPos) and ending (endPos) positions of the currently
/// selected text. By using GetSelection in conjunction with GetSubText, you can get
/// the currently selected text.
//
inline void TEdit::GetSelection(int& startPos, int& endPos) const
{
  PRECONDITION(GetHandle());
  CONST_CAST(TEdit*,this)->SendMessage(EM_GETSEL, TParam1(&startPos), TParam2(&endPos));
}



//
/// Return the current caret position.
//
inline int TEdit::GetCurrentPosition() const
{
  PRECONDITION(GetHandle());
  int CurPos = 0;
  GetSelection(CurPos, CurPos);
  return CurPos;
}


//
/// Return the line number associated with character index "CharPos"
//
/// From a multiline edit control, returns the line number on which the character
/// position specified by charPos occurs. If charPos is greater than the position of
/// the last character, the number of the last line is returned. If charPos is-1,
/// the number of the line that contains the first selected character is returned.
/// If there is no selection, the line containing the caret is returned.
//
inline int TEdit::GetLineFromPos(int charPos) const
{
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TEdit*,this)->SendMessage(EM_LINEFROMCHAR, charPos);
}

//
/// In a multiline edit control, GetLineIndex returns the number of characters that
/// appear before the line number specified by lineNumber. If lineNumber is -1,
/// GetLineIndex returns the number of the line that contains the caret is returned.
//
inline int TEdit::GetLineIndex(int lineNumber) const
{
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TEdit*,this)->SendMessage(EM_LINEINDEX, lineNumber);
}

//
/// Scroll the text by the specified horizontal and vertical amounts
//
/// Scrolls a multiline edit control horizontally and vertically using the numbers
/// of characters specified in horizontalUnit and verticalUnit. Positive values
/// result in scrolling to the right or down in the edit control, and negative
/// values result in scrolling to the left or up.
//
inline void TEdit::Scroll(int horizUnit, int vertUnit)
{
  PRECONDITION(GetHandle());
  SendMessage(EM_LINESCROLL, horizUnit, vertUnit);
}
//
/// Sroll the caret into view in an edit control.
//
inline void TEdit::ScrollCaret(){
  PRECONDITION(GetHandle());
  SendMessage(EM_SCROLLCARET, 0, 0);
}

//
//
//
inline int TEdit::ScrollText(int how){
  TResult ret = SendMessage(EM_SCROLL, how, 0);
  if(HiUint16(ret) == TRUE)
    return LoUint16(ret);
  return -1;
}

//
//
//
inline int TEdit::LineDown(){          // EM_SCROLL, SB_LINEDOWN  Scrolls down one line.
  return ScrollText(SB_LINEDOWN);
}

//
//
//
inline int TEdit::LineUp(){           // EM_SCROLL, SB_LINEUP  Scrolls up one line.
  return ScrollText(SB_LINEUP);
}

//
//
//
inline int TEdit::PageDown(){         // EM_SCROLL, SB_PAGEDOWN  Scrolls down one page.
  return ScrollText(SB_PAGEDOWN);
}

//
//
//
inline int TEdit::PageUp(){           // EM_SCROLL, SB_PAGEUP  Scrolls up one page.
  return ScrollText(SB_PAGEUP);
}

//
/// Inserts the text supplied in str into the edit control at the current text
/// insertion point (cursor position), and replaces any currently selected text.
/// Insert is similar to Paste(), but does not affect the Clipboard.
//
inline void TEdit::Insert(LPCTSTR str)
{
  PRECONDITION(GetHandle());
  SendMessage(EM_REPLACESEL, 0, TParam2(str));
}

//
/// Gets the formatting rectangle of a multiline edit control.
//
inline void TEdit::GetRect(TRect& frmtRect) const
{
  PRECONDITION(GetHandle());
  CONST_CAST(TEdit*,this)->SendMessage(EM_GETRECT, 0, TParam2((TRect *)&frmtRect));
}

//
/// Sets the formatting rectangle for a multiline edit control.
//
inline void TEdit::SetRect(const TRect& frmtRect)
{
  PRECONDITION(GetHandle());
  SendMessage(EM_SETRECT, 0, TParam2((const TRect *)&frmtRect));
}

//
/// Sets the formatting rectangle for a multiline edit control. Unlike SetRect(),
/// SetRectNP does not repaint the edit control.
//
inline void TEdit::SetRectNP(const TRect& frmtRect)
{
  PRECONDITION(GetHandle());
  SendMessage(EM_SETRECTNP, 0, TParam2((const TRect *)&frmtRect));
}

//
/// Indicates if the end-of-line characters (carriage return, linefeed) are to be
/// added or removed from text lines that are wordwrapped in a multiline edit
/// control. Returns true if these characters are placed at the end of wordwrapped
/// lines or false if they are removed.
//
inline bool TEdit::FormatLines(bool addEOL)
{
  PRECONDITION(GetHandle());
  return SendMessage(EM_FMTLINES, addEOL);
}

//
/// Sets the tab stop positions in a multiline edit control.
//
inline void TEdit::SetTabStops(int numTabs, const int * tabs)
{
  PRECONDITION(GetHandle());
  SendMessage(EM_SETTABSTOPS, numTabs, TParam2(tabs));
}

//
/// Limit the amount of new text that can be entered in the edit control.
//
inline void TEdit::LimitText(int txtLen)
{
  PRECONDITION(GetHandle());
  SendMessage(EM_LIMITTEXT, TParam1(txtLen));
}

//
/// Return the memory handle for the edit control's buffer.
//
inline HLOCAL TEdit::GetMemHandle() const
{
  PRECONDITION(GetHandle());
  return (HLOCAL)CONST_CAST(TEdit*,this)->SendMessage(EM_GETHANDLE);
}

//
/// Sets the memory handle for the edit control's buffer.
//
inline void TEdit::SetMemHandle(HLOCAL localMem)
{
  PRECONDITION(GetHandle());
  SendMessage(EM_SETHANDLE, TParam1(localMem));
}

//
/// Indicates the topmost visible line in an edit control. For single-line edit
/// controls, the return value is 0. For multiline edit controls, the return value
/// is the index of the topmost visible line.
//
inline int TEdit::GetFirstVisibleLine() const
{
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TEdit*,this)->SendMessage(EM_GETFIRSTVISIBLELINE);
}

//
/// Sets the edit control to be read-only or read-write.
//
inline void TEdit::SetReadOnly(bool readOnly)
{
  PRECONDITION(GetHandle());
  SendMessage(EM_SETREADONLY, readOnly);
}

//
/// Returns the character to be displayed in place of a user-typed character. When
/// the edit control is created with the ES_PASSWORD style specified, the default
/// display character is an asterisk (*).
//
inline uint TEdit::GetPasswordChar() const
{
  PRECONDITION(GetHandle());
  return (uint)CONST_CAST(TEdit*,this)->SendMessage(EM_GETPASSWORDCHAR);
}

//
/// Get word-breaking procedure
//
inline EDITWORDBREAKPROC TEdit::GetWordBreakProc() const
{
  PRECONDITION(GetHandle());
  return (EDITWORDBREAKPROC)CONST_CAST(TEdit*,this)->SendMessage(EM_GETWORDBREAKPROC);
}

//
/// In a multiline edit control, SetWordBreakProc indicates that an
/// application-supplied word-break function has replaced the default word-break
/// function. The application-supplied word-break function might break the words in
/// the text buffer at a character other than the default blank character.
//
inline void TEdit::SetWordBreakProc(EDITWORDBREAKPROC proc)
{
  PRECONDITION(GetHandle());
  SendMessage(EM_SETWORDBREAKPROC, 0, TParam2(proc));
}

//
/// Returns true if it is possible to undo the last edit. 
//
inline bool TEdit::CanUndo() const
{
  PRECONDITION(GetHandle());
  return (bool)CONST_CAST(TEdit*,this)->SendMessage(EM_CANUNDO);
}

//
inline void TEdit::SetLeftMargin(uint16 margin)
{
  PRECONDITION(GetHandle());
  SendMessage(EM_SETMARGINS, EC_LEFTMARGIN, MkUint32(0, margin));
}

//
inline void TEdit::SetRightMargin(uint16 margin)
{
  PRECONDITION(GetHandle());
  SendMessage(EM_SETMARGINS, EC_RIGHTMARGIN, MkUint32(margin, 0));
}

//
inline void TEdit::SetMarginUseFontInfo()
{
  PRECONDITION(GetHandle());
  SendMessage(EM_SETMARGINS, EC_USEFONTINFO, 0);
}

//
inline uint32 TEdit::GetMargins() const
{
  PRECONDITION(GetHandle());
  return (uint32)CONST_CAST(TEdit*,this)->SendMessage(EM_GETMARGINS);
}

//
inline int TEdit::GetLimitText() const
{
  PRECONDITION(GetHandle());
  return (int)CONST_CAST(TEdit*,this)->SendMessage(EM_GETLIMITTEXT);
}

//
inline uint32 TEdit::CharFromPos(int16 x, int16 y) {
  PRECONDITION(GetHandle());
  return (uint32)SendMessage(EM_CHARFROMPOS, 0, MkUint32(x, y));
}

//
inline uint32 TEdit::PosFromChar(uint charIndex) {
  PRECONDITION(GetHandle());
  return (uint32)SendMessage(EM_POSFROMCHAR, charIndex);
}

} // OWL namespace

#endif  // OWL_EDIT_H
