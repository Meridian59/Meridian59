//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TEdit.  This defines the basic behavior
/// of all edit controls.
//----------------------------------------------------------------------------
#include <owl/pch.h>


#include <owl/edit.h>
#include <owl/validate.h>
#include <owl/updown.h>

#if !defined(__CYGWIN__)
//JJH
# if !defined(WINELIB)
#  include <dos.h>
# endif // !WINELIB
#endif
#include <string.h>
#include <ctype.h>
#include <algorithm>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
# pragma option -w-inl // Disable warning "Functions containing 'statement' is not expanded inline".
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;

/// \class TEdit
///A TEdit is an interface object that represents an edit control interface element. 
///A TEdit object must be used to create an edit control in a parent TWindow. A TEdit 
///can be used to facilitate communication between your application and the edit controls 
///of a TDialog. This class is streamable.
///
///There are two styles of edit control objects: single line and multiline. Multiline edit 
///controls allow editing of multiple lines of text.
///
///The position of the first character in an edit control is zero. For a multiline edit control, 
///the position numbers continue sequentially from line to line; line breaks count as two characters.
///
///Most of TEdit's member functions manage the edit control's text. TEdit also includes some 
///automatic member response functions that respond to selections from the edit control's parent window menu, 
///including cut, copy, and paste. Two important member functions inherited from TEdit's base class 
///(TStatic) are GetText and SetText.


//
/// Class pointer to an edit control that is trying to regain focus after losing
/// it with invalid contents. Is 0 in all other conditions.
//
/// Used to prevent 'oscillation' when a validated window with invalid
/// input is losing focus to another validated window with invalid input
/// Without this flag, the two windows will fight for focus
//
TEdit* TEdit::ValidatorReFocus = 0;

DEFINE_RESPONSE_TABLE1(TEdit, TStatic)
  EV_COMMAND(CM_EDITCUT, CmEditCut),
  EV_COMMAND(CM_EDITCOPY, CmEditCopy),
  EV_COMMAND(CM_EDITPASTE, CmEditPaste),
  EV_COMMAND(CM_EDITDELETE, CmEditDelete),
  EV_COMMAND(CM_EDITCLEAR, CmEditClear),
  EV_COMMAND(CM_EDITUNDO, CmEditUndo),
  EV_COMMAND_ENABLE(CM_EDITCUT, CmSelectEnable),
  EV_COMMAND_ENABLE(CM_EDITCOPY, CmSelectEnable),
  EV_COMMAND_ENABLE(CM_EDITDELETE, CmSelectEnable),
  EV_COMMAND_ENABLE(CM_EDITPASTE, CmPasteEnable),
  EV_COMMAND_ENABLE(CM_EDITCLEAR, CmCharsEnable),
  EV_COMMAND_ENABLE(CM_EDITUNDO, CmModEnable),
  EV_UDN_DELTAPOS(static_cast<unsigned int>(-1), EvUpDown),
  EV_NOTIFY_AT_CHILD(EN_ERRSPACE, ENErrSpace),
  EV_WM_CHAR,
  EV_WM_KEYDOWN,
  EV_WM_GETDLGCODE,
  EV_WM_SETFOCUS,
  EV_WM_KILLFOCUS,
  EV_WM_CHILDINVALID,
END_RESPONSE_TABLE;

//
/// Constructs an edit control object with a parent window (parent). 
//
/// Sets the creation attributes of the edit control and fills its Attr data members with the 
/// specified control ID (Id), position (x, y) relative to the origin of the parent window's client area, 
/// width (w), and height (h).
/// If text buffer length (textLimit) is 0 or 1, there is no explicit limit to the number of characters 
/// that can be entered. Otherwise textLimit - 1 characters can be entered. By default, initial text (text) 
/// in the edit control is left-justified and the edit control has a border. Multiline edit controls have 
/// horizontal and vertical scroll bars

TEdit::TEdit(TWindow* parent, int id, LPCTSTR text, int x, int y, int w, int h,
  uint textLen, bool multiline, TModule* module)
: TStatic(parent, id, text, x, y, w, h, textLen, module)
{
  // Undo the styles set by TStatic, & add in edit styles
  //
  ModifyStyle(SS_LEFT, ES_LEFT | ES_AUTOHSCROLL |WS_BORDER | WS_TABSTOP);
  ModifyExStyle(0, WS_EX_CLIENTEDGE);
  if (multiline)
    Attr.Style |= ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL;

  Validator = 0;
}

//
/// String-aware overload
//
TEdit::TEdit(TWindow* parent, int id, const tstring& text, int x, int y, int w, int h, 
  uint textLen, bool multiline, TModule* module)
: TStatic(parent, id, text, x, y, w, h, textLen, module)
{
  // Undo the styles set by TStatic, & add in edit styles
  //
  ModifyStyle(SS_LEFT, ES_LEFT | ES_AUTOHSCROLL |WS_BORDER | WS_TABSTOP);
  ModifyExStyle(0, WS_EX_CLIENTEDGE);
  if (multiline)
    Attr.Style |= ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL;

  Validator = 0;
}

//
/// Constructor for TEdit associated with a MS-Windows interface element created by MS-Windows from a resource definition
//
/// Constructs a TEdit object to be associated with an edit control of a TDialog. 
/// Invokes the TStatic constructor with identical parameters. The resourceID
/// parameter must correspond to an edit resource that you define. Enables the data transfer mechanism 
/// by calling EnableTransfer.

TEdit::TEdit(TWindow* parent, int resourceId, uint textLen, TModule* module)
: TStatic(parent, resourceId, textLen, module)
{
  EnableTransfer();
  Validator = 0;
}

//
/// Constructs a TEdit object to encapsulate (alias) an existing control.
//
TEdit::TEdit(THandle hWnd, TModule* module)
: TStatic(hWnd, module)
{
  EnableTransfer();
  Validator = 0;
}


//
/// Destructor for a TEdit control
//
TEdit::~TEdit()
{
  SetValidator(0);
}

//
/// Beeps when edit control runs out of space
//
/// Sounds a beep in response to an error notification message that is sent when the
/// edit control unsuccessfully attempts to allocate more memory.
//
void
TEdit::ENErrSpace()
{
  MessageBeep(0);

  DefaultProcessing();  // give parent a chance to process
}

//
/// Responds to WM_GETDLGCODE messages that are sent to a dialog box associated with
/// a control. EvGetDlgCode  allows the dialog manager to intercept a message that
/// would normally go to a control and then ask the control if it wants to process
/// this message. If not, the dialog manager processes the message. The msg
/// parameter indicates the kind of message, for example a control, command, or edit
/// message, sent to the dialog box manager.
/// If the edit control contains valid input, then Tabs are allowed for changing
/// focus and a DLGC_WANTTABS code is returned.
/// Otherwise, requests that TABs be sent to Self, where
/// we will generate the Invalid message (See EvKeyDown()).
//
uint
TEdit::EvGetDlgCode(const MSG* msg)
{
  uint retVal = (uint)TStatic::EvGetDlgCode(msg);
  if (!IsValid(false))
    retVal |= DLGC_WANTTAB;
  return retVal;
}

//
/// Validates Self whenever a character is entered.  Allows
/// the character entry to be processed normally, then validates
/// the result and restores Self's text to its original state
/// if there is an incorrect entry.
//
/// By default, the SupressFill parameter of the IsValidInput
/// method call to the Validator is set to False, so that it
/// is free to modify the string, if it is so configured.
//
void
TEdit::EvChar(uint key, uint repeatCount, uint flags)
{
  if (Validator && key != VK_BACK) {
    int oldBuffLen = GetTextLen();
    TAPointer<tchar> oldBuff(new tchar[oldBuffLen+1]);
    GetText(oldBuff, oldBuffLen+1);

    int   startSel, endSel;
    GetSelection(startSel, endSel);
    bool wasAppending = endSel == oldBuffLen;

    bool preMsgModify = IsModified();             // Save (pre)  MODIFY flag

    TStatic::EvChar(key, repeatCount, flags);     // Process the new char...

    bool postMsgModify= IsModified();             // Save (post) MODIFY flag

    GetSelection(startSel, endSel);
    int buffLen = GetTextLen();

    // We add 1024 as a temporary fix to buffer size due to potential appending
    // to the buffer and we do not know how many fill bytes might get added.
    //
    LPTSTR buff = LockBuffer(std::max(static_cast<int>(TextLimit), std::max(oldBuffLen, buffLen)) + 1024);

    // Run the result of the edit through the validator.  If incorrect,
    // then restore the original text.  Otherwise, range check & position
    // the selection as needed.
    //
    if (!Validator->HasOption(voOnAppend) || wasAppending && endSel == buffLen) {
      if (!Validator->IsValidInput(buff, false)) {
        ::_tcscpy(buff, oldBuff);          // Restore old buffer
        postMsgModify = preMsgModify;   // Restore old modify state too!
      }
      else {
        if (wasAppending)
          startSel = endSel = static_cast<int>(::_tcslen(buff)); // may have autoFilled--move to end
      }
      UnlockBuffer(buff, true);
      SetSelection(startSel, endSel);
    }
    else {
      if (endSel == buffLen && !Validator->IsValidInput(buff, false))
        Validator->Error(this);
      UnlockBuffer(buff);
    }
    SendMessage(EM_SETMODIFY, (TParam1)postMsgModify);
  }
  else
    TStatic::EvChar(key, repeatCount, flags);
}

//
/// EvKeyDown translates the virtual key code into a movement. key indicates the
/// virtual key code of the pressed key, repeatCount holds the number of times the
/// same key is pressed, flags contains one of the messages that translates to a
/// virtual key (VK) code for the mode indicators. If the Tab key is sent to the
/// edit control, EvKeyDown checks the validity before allowing the focus to change.
/// The control will only get a TAB if
/// EvGetDlgCode(above) allows it, which is done when the control contains
/// invalid input (we re-validate here just for completeness, in case
/// descendants redefine any of this behavior).
//
/// We need to validate on TAB focus-changes because there is a case not handled
/// by EvKillFocus: when focus is lost to an OK or Cancel button by tabbing.
//
/// Otherwise, for validators with the OnAppend option, perform an input
/// validation if the selection moves to the end. i.e. it becomes appending.
//
void
TEdit::EvKeyDown(uint key, uint /*repeatCount*/, uint /*flags*/)
{
  if (key == VK_TAB && !IsValid(false)){
    ValidatorError();
    return;
  }
  if (Validator && Validator->HasOption(voOnAppend)) {
    int  startSel, endSel;
    GetSelection(startSel, endSel);

    int  buffLen = GetTextLen();   // length of buffer
    bool wasAppending = endSel == buffLen;

    DefaultProcessing();

    if (!wasAppending) {
      GetSelection(startSel, endSel);
      LPTSTR buff = LockBuffer();
      bool err = endSel == static_cast<int>(::_tcslen(buff)) && !Validator->IsValidInput(buff, false);
      UnlockBuffer(buff);
      if (err)
        Validator->Error(this);
    }
  }
  else
    DefaultProcessing();
}

//
/// Validates this whenever the focus is about to be lost.
//
/// In response to a WM_KILLFOCUS message sent to a window that is losing the
/// keyboard, EvKillFocus hides and then destroys the caret. EvKillFocus validates
/// text whenever the focus is about to be lost and holds onto the focus if the text
/// is not valid. Doesn't kill the focus if another application, a Cancel button, or
/// an OK button (in which case CanClose is called to validate text) has the focus.
//
/// \note Holds onto the focus if this is not valid.  Checks first to make sure that
/// the focus is not being taken by either (a) another app, or (b) a Cancel
/// button, or (c) an OK button (in which case CanClose will validate); in each
/// case, we don't want to validate.
//
void
TEdit::EvKillFocus(THandle hWndGetFocus)
{
  // If another validator is attempting to regain focus, then let it
  //
  if (Validator && !ValidatorReFocus) {
    // The window getting focus belongs to this app if any of the window
    // handles has an OWL object attached that we can get at.
    //
    THandle hWnd = hWndGetFocus;
    while (hWnd && !GetWindowPtr(hWnd))
      hWnd = ::GetParent(hWnd);

    if (hWnd) {
      int btnId = ::GetDlgCtrlID(hWndGetFocus);

      // Note that we do not allow IsValid to post the message box, since the
      // change of focus resulting from that message will interfere with the
      // change we are in the process of completing.  Instead, post a message
      // to the Parent informing it of the validation failure, and providing it
      // with a handle to us.
      //
      if (btnId != IDCANCEL && btnId != IDOK && !IsValid(false)) {
        ValidatorReFocus = this;
        GetParentO()->PostMessage(WM_CHILDINVALID, TParam1(TWindow::GetHandle()));
      }
    }
  }
  TControl::EvKillFocus(hWndGetFocus);
}

//
/// Handle the set focus message and make sure the anti-oscillation flag is cleared
//
void
TEdit::EvSetFocus(THandle hWndLostFocus)
{
  // If we're getting focus back because of invalid input, then clear the
  // anti-oscillation flag
  //
  if (ValidatorReFocus == this)
    ValidatorReFocus = 0;

  TControl::EvSetFocus(hWndLostFocus);
}


//
/// Handles up-down messages from an up-down control and adjusts contents if there
/// is a validator to help.
// !Y.B think about it, this message send before change not after ??????????????
bool
TEdit::EvUpDown(TNmUpDown & /*not*/)
{
  if (Validator) {
    int  startSel, endSel;
    GetSelection(startSel, endSel);

    LPTSTR buff = LockBuffer();
    tstring s = buff;
    bool changed = Validator->Adjust(s, startSel, endSel, 1) != 0; // !CQ need +- 1
    if (changed)
      ::_tcscpy(buff, s.c_str());
    UnlockBuffer(buff, changed);
    SetSelection(startSel, endSel);
  }
  return false; // always allow changes for now
}

//
/// Handle input validation message sent by parent
//
void
TEdit::EvChildInvalid(THandle)
{
  ValidatorError();
}

//
/// Handles validation errors that occur as a result of validating the edit control.
//
void
TEdit::ValidatorError()
{
  // Temporarily unset our validator & call its Error function directly. This
  // is to avoid and loops caused when we lose focus.
  //
  if (Validator) {
    SetFocus();
    TValidator* savedValidator = Validator;
    Validator = 0;
    savedValidator->Error(this);
    Validator = savedValidator;
  }
}

//
/// Overrides TStatic's virtual member function and clears all text.
//
void
TEdit::Clear()
{
  DeleteSubText(0, -1);
}

//
/// Checks to see if all child windows can be closed before closing the current
/// window. If any child window returns false, CanClose returns false and terminates
/// the process. If all child windows can be closed, CanClose returns true.
//
bool
TEdit::CanClose()
{
  bool okToClose = TStatic::CanClose();
  if (okToClose)
    if (IsWindowEnabled() && !IsValid(true)) {
      ValidatorReFocus = this;
      SetFocus();
      return false;
    }
  return okToClose;
}

//
/// This function is called for Cut/Copy/Delete menu items to determine
/// whether or not the item is enabled.
//
void
TEdit::CmSelectEnable(TCommandEnabler& commandHandler)
{
  int sPos, ePos;

  GetSelection(sPos, ePos);
  commandHandler.Enable(sPos != ePos);
}

//
/// This function is called for the Paste menu item to determine whether or
/// not the item is enabled.
//
void
TEdit::CmPasteEnable(TCommandEnabler& commandHandler)
{
  TClipboard clipboard(*this, false);
  commandHandler.Enable(clipboard.IsClipboardFormatAvailable(CF_TEXT));
}

//
/// This function is called for the Clear menu item to determine whether or
/// not the item is enabled.
//
void
TEdit::CmCharsEnable(TCommandEnabler& commandHandler)
{
  commandHandler.Enable(!(GetNumLines() == 1 && GetLineLength(0) == 0));
}

//
/// This function is called for the Undo menu item to determine whether or
/// not the item is enabled.
//
void
TEdit::CmModEnable(TCommandEnabler& commandHandler)
{
  commandHandler.Enable(IsModified());
}

//
/// Return the length of line number "lineNumber"
//
/// From a multiline edit control, GetLineLength returns the number of characters in
/// the line specified by lineNumber. If it is -1, the following applies: 
/// - if no text is selected, GetLineLength returns the length of the line 
///   where the caret is positioned; 
/// - if text is selected on the line, GetLineLength returns the line
///   length minus the number of selected characters; 
/// - if selected text spans more than one line, GetLineLength returns 
///   the length of the lines minus the number of selected characters.
//
int
TEdit::GetLineLength(int lineNumber) const
{
  return (int)CONST_CAST(TEdit*,this)->SendMessage(EM_LINELENGTH,
                                       (lineNumber > -1) ? GetLineIndex(lineNumber) : -1);
}

//
/// Return the text of line number "lineNumber" (0-terminated)
///
/// Retrieves a line of text (whose line number is supplied) from the edit control
/// and returns it in str (NULL-terminated). strSize indicates how many characters
/// to retrieve. GetLine returns false if it is unable to retrieve the text or if
/// the supplied buffer is too small.
///
/// \note Requires a buffer of at least 2 bytes, even when the line length is 1
//
bool
TEdit::GetLine(LPTSTR textString, int strSize, int lineNumber) const
{
  if (strSize <= 0)
    return false;

  int lineLength = GetLineLength(lineNumber);
  bool success = strSize >= lineLength + 1;

  // The first 16bit word of the buffer is used by EM_GETLINE as the buffer
  // size. Always need a buffer of at least 2 bytes to pass this word.
  //
  if (static_cast<unsigned int>(strSize) < sizeof(int16)) { //JJH added static cast
    textString[0] = 0;
    return lineLength == 0;  // If lineLen was 0, only then are we successful
  }
  ((int16 *)textString)[0] = int16(strSize);

  int bytesCopied = (int)CONST_CAST(TEdit*,this)->
                    SendMessage(EM_GETLINE, lineNumber, TParam2(textString));

  textString[bytesCopied] = 0; // Windows returns non-0 terminated string

  if (bytesCopied != lineLength)
    return false;
  return success;
}

//
/// For use with CopyText.
//
struct TEditGetLine 
{
  const TEdit& edit;
  int line;
  TEditGetLine(const TEdit& c, int line_) : edit(c), line(line_) {}

  int operator()(LPTSTR buf, int buf_size)
  {
    bool ok = edit.GetLine(buf, buf_size, line);
    WARN(!ok, _T("TEdit::GetLine failed, line = ") << line);
    static_cast<void>(ok); // Suppress not-in-use warning.
    return buf_size - 1; // line length
  }
};

//
/// String-aware overload
/// If the length of the returned string differs from GetLineLength an error occured.
//
tstring 
TEdit::GetLine(int lineNumber) const
{
  return CopyText(GetLineLength(lineNumber), TEditGetLine(*this, lineNumber));
}

//
/// Lock the edit control's buffer, optionally resizing it first, and return
/// a pointer to the beginning of it, or 0 if failure.
/// Must call UnlockBuffer when finished with buffer.
//
LPTSTR
TEdit::LockBuffer(uint newSize)
{
  // Single line edit controls or Win32s will fail the GetMemHandle(), 
  // so do it the hard way and make a copy in this case. 
  // We also copy to avoid a problem when using the new XP common controls.
  //
  if (!(GetWindowLong(GWL_STYLE) & ES_MULTILINE) || TCommCtrl::Dll()->GetCtrlVersion() >= 0x60000)
  {
    if (!newSize)
      newSize = GetTextLen() + 1;
    LPTSTR buffer = new tchar[newSize];
    GetText(buffer, newSize);
    return buffer;
  }

  HLOCAL hBuffer = GetMemHandle();
  if (newSize) 
  {
    hBuffer = LocalReAlloc(hBuffer, newSize * sizeof(tchar), LHND);
    if (!hBuffer)
      return 0;
  }
  return static_cast<LPTSTR>(LocalLock(hBuffer));
}

//
/// Unlock the edit control's buffer locked by LockBuffer.
//
/// If the contents were changed (buffer is resized or written to), updateHandle should be true.
/// Ignores call if buffer is 0
//
void
TEdit::UnlockBuffer(LPCTSTR buffer, bool updateHandle)
{
  if (!buffer)
    return;

  // If a copy was made on lock, copy it back if requested, and then free the buffer.
  //
  if (!(GetWindowLong(GWL_STYLE) & ES_MULTILINE) || TCommCtrl::Dll()->GetCtrlVersion() >= 0x60000) 
  {
    if (updateHandle)
      SetText(buffer);
    delete[] const_cast<LPTSTR>(buffer);
    return;
  }

  HLOCAL hBuffer = LocalHandle(const_cast<LPTSTR>(buffer));
  LocalUnlock(hBuffer);

  // Handle may have moved or buffer contents written.
  //
  if (updateHandle)
    SetMemHandle(hBuffer);
}

//
/// Similar to strstr(), but is case sensitive or insensitive, uses Windows
/// string functions to work with different language drivers
//
static LPCTSTR
strstrcd(LPCTSTR str1, LPCTSTR str2, bool caseSens)
{
  PRECONDITION(str1 && str2);
  int len2 = static_cast<int>(::_tcslen(str2));
  LPTSTR p = (LPTSTR)str1;
  LPCTSTR endp = str1 + ::_tcslen(str1) - len2 + 1;

  if (caseSens)
    while (p < endp) {
      tchar c = p[len2];           // must term p to match str2 len
      p[len2] = 0;                 // for _tcscmp to work.
      if (_tcscmp(p, str2) == 0) {
        p[len2] = c;
        return p;
      }
      p[len2] = c;
#if defined(BI_DBCS_SUPPORT) && !defined(UNICODE)
      LPTSTR p2 = ::AnsiNext(p);
      if (p2 <= p)
        break;
      p = p2;
#else
      p++;
#endif
    }
  else
    while (p < endp) {
      tchar c = p[len2];
      p[len2] = 0;
      if (_tcsicmp(p, str2) == 0) {
        p[len2] = c;
        return p;
      }
      p[len2] = c;
#if defined(BI_DBCS_SUPPORT) && !defined(UNICODE)
      LPTSTR p2 = ::AnsiNext(p);
      if (p2 <= p)
        break;
      p = p2;
#else
      p++;
#endif
    }
  return 0;
}

//
/// Similar to strstrcd(), but searches backwards. Needs the length of str1
/// to know where to start.
//
static LPCTSTR
strrstrcd(LPCTSTR str1, uint len1, LPCTSTR str2,
           bool caseSens)
{
  PRECONDITION(str1 && str2);
  int len2 = static_cast<int>(::_tcslen(str2));
  LPTSTR p = (LPTSTR)(str1 + len1 - len2);

#if defined(BI_DBCS_SUPPORT) && !defined(UNICODE)
  if (p >= str1)
    p = ::AnsiPrev(str1, p+1);
#endif
  if (caseSens)
    while (p >= str1) {
      tchar c = p[len2];            // must term p to match str2 len
      p[len2] = 0;                 // for _tcscmp to work.
      if (_tcscmp(p, str2) == 0) {
        p[len2] = c;
        return p;
      }
      p[len2] = c;
#if defined(BI_DBCS_SUPPORT) && !defined(UNICODE)
      LPTSTR p2 = ::AnsiPrev(str1, p);
      if (p2 >= p)
        break;
      p = p2;
#else
      p--;
#endif
    }
  else
    while (p >= str1) {
      tchar c = p[len2];
      p[len2] = 0;
      if (_tcsicmp(p, str2) == 0) {
        p[len2] = c;
        return p;
      }
      p[len2] = c;
#if defined(BI_DBCS_SUPPORT) && !defined(UNICODE)
      LPTSTR p2 = ::AnsiPrev(str1, p);
      if (p2 >= p)
        break;
      p = p2;
#else
      p--;
#endif
    }
  return 0;
}

//
/// searches for and selects the given text and returns the offset of the text or -1 if the text is not found
//
/// Performs either a case-sensitive or case-insensitive search for the supplied
/// text. If the text is found, the matching text is selected, and Search returns
/// the position of the beginning of the matched text. If the text is not found in
/// the edit control's text, Search returns -1. If -1 is passed as startPos, then
/// the search starts from either the end or the beginning of the currently selected
/// text, depending on the search direction.
//
int
TEdit::Search(int startPos, LPCTSTR text, bool caseSensitive,
              bool wholeWord, bool up)
{
  if (!text || !text[0])
    return -1;

  if (startPos == -1) {
    int sBeg, sEnd;
    GetSelection(sBeg, sEnd);
    startPos = up ? sBeg : sEnd;
  }
  int textLen = static_cast<int>(::_tcslen(text));

  // Lock the text buffer to get the pointer, and search thru it for the text
  //
  LPCTSTR buffer = LockBuffer();
  LPCTSTR pos;
  for (;;) {
    if (up)
      pos = strrstrcd(buffer, startPos, text, caseSensitive);
    else
      pos = strstrcd(buffer+startPos, text, caseSensitive);

    // If whole-word matching is enabled and we have a match so far, then make
    // sure the match is on word boundaries.
    //
    if (wholeWord && pos) {
#if defined(BI_DBCS_SUPPORT)
      LPTSTR prevPos;
      if (pos > buffer)
         prevPos = ::AnsiPrev(buffer, pos);

      if (pos > buffer && _istalnum((tchar)*prevPos) || // Match is in preceding word
        textLen < (int)::_tcslen(pos) && isalnum((utchar)pos[textLen])) {
        if (up)
          startPos = (prevPos - buffer) + ::_tcslen(text);
        else
          startPos = ::AnsiNext(pos) - buffer;
        continue;  // Skip this match and keep searching
      }
#else
      if (pos > buffer && (_istdigit(pos[-1])||_istalpha(pos[-1])) || // Match is in preceding word
        textLen < (int)::_tcslen(pos) && (_istdigit(pos[textLen])||_istalpha(pos[textLen])) ) { //_istalnum is not yet implemented in WineLib
        startPos = (uint)(pos-buffer) + !up;
        continue;  // Skip this match and keep searching
      }
#endif
    }
    break;  // Out of for loop
  }

  // If we've got a match, select that text, cleanup & return.
  //
  if (pos) {
    int sBeg = pos - buffer;
    UnlockBuffer(buffer);
    SetSelection(sBeg, sBeg + textLen);
    SendMessage(WM_KEYDOWN, VK_RIGHT);
    SetSelection(sBeg, sBeg + textLen);

    return sBeg;
  }
  UnlockBuffer(buffer);
  return -1;
}


//
/// Deletes the currently selected text, and returns false if no text is selected.
//
bool
TEdit::DeleteSelection()
{
  int  startPos, endPos;

  GetSelection(startPos, endPos);

  if (startPos != endPos) {
    SendMessage(WM_CLEAR);
    return true;
  }
  return false;
}

//
/// Deletes the text between the starting and ending positions 
/// specified by startPos and endPos, respectively. 
/// DeleteSubText returns true if successful.
//
bool
TEdit::DeleteSubText(int startPos, int endPos)
{
  if (SetSelection(startPos, endPos))
    return DeleteSelection();

  else
    return false;
}

//
/// Deletes the text in the line specified by lineNumber in a multiline edit control. 
/// If -1 passed, deletes the current line. DeleteLine does not delete the line break 
/// and affects no other lines. Returns true if successful. Returns false if lineNumber 
/// is not -1 and is out of range or if an error occurs.
//
bool
TEdit::DeleteLine(int lineNumber)
{
  if (lineNumber == -1)
    lineNumber = GetLineFromPos(GetLineIndex(-1));

  int firstPos = GetLineIndex(lineNumber);

  if (firstPos != -1) {
    int  lastPos = GetLineIndex(lineNumber + 1);

    if (lastPos == -1)
      lastPos = firstPos + GetLineLength(lineNumber);

    if (firstPos == 0  && firstPos == lastPos) {
      SetText(_T(""));
      return true;
    }
    else {
      return DeleteSubText(firstPos, lastPos);
    }
  }

  return false;
}

//
/// Retrieve the text of the associated edit control between the given positions.
/// Note that the buffer must be large enough to hold the text, otherwise buffer
/// overrun will occur. The destination is always null-terminated.
///
/// NB! This function is deprecated. Use GetTextRange instead.
//
void
TEdit::GetSubText(LPTSTR textBuf, int startPos, int endPos) const
{
  WARN(true, _T("TEdit::GetSubText is deprecated. Use GetTextRange instead."));
  if (!textBuf) return;
  tstring t = GetTextRange(TRange(startPos, endPos));
  _tcscpy(textBuf, t.c_str());
}

//
/// Retrieves a specified range of text from the edit control.
/// The range is half-open; i.e. range [0,2) for "abc" returns "ab".
/// An empty string is returned if either
///
/// - the control is empty (no text), or 
/// - the range is invalid (empty or inverted), or
/// - whole range is beyond the extents of the actual text.
///
/// If the end of the range is beyond the valid range then it is limited to the valid range.
/// A special case is the range [0, -1). It will return the full text.
///
/// \todo Must be tested
//
tstring
TEdit::GetTextRange(const TRange& r) const
{
  const int begin = r.cpMin;
  int end = r.cpMax; // May be adjusted.

  // Check input arguments against EM_GETTEXTRANGE requirements.
  // Note that [0, -1) for an empty control is not valid for EM_GETTEXTRANGE,
  // but we'll ignore that, since no buffer issues are involved here.
  // Otherwise we reject negative positions, as well as empty and inverted ranges.
  // EM_GETTEXTRANGE would not null-terminate the result in these cases,
  // so to adhere to the same policy we warn about these cases.

  if (begin == 0 && end == -1)
  {
    return GetText();
  }

  tstring s;
  if (begin < 0 || end < 0)
  {
    WARN(true, _T("Arguments out of range"));
    return s;
  }
  else if (begin == end) 
  {
    WARN(true, _T("Empty range"));
    return s;
  }
  else if (begin > end) 
  {
    WARN(true, _T("Inverted range"));
    return s;
  }

  // Return empty if the entire range is outside the extents of the actual text.
  // This is valid for EM_GETTEXTRANGE so we wont complain.

  int n = GetTextLen(); 
  if (begin >= n) 
    return s; 

  // Limit end to the actual text size.

  end = std::min(end, n);

  // Calculate line indexes and char indexes (within lines).
  // Then append line by line to the result string, cropping
  // the start and end line specially.

  const int startLine = GetLineFromPos(begin);
  const int startChar = begin - GetLineIndex(startLine);
  const int endLine = GetLineFromPos(end);
  const int endChar = end - GetLineIndex(endLine);
  for (int i = startLine; i <= endLine; ++i) 
  {
    tstring line = GetLine(i) + _T("\r\n"); // CRLF-terminated
    int b = (i == startLine) ? startChar : 0;
    int e = (i == endLine) ? endChar : static_cast<int>(line.length());
    CHECK(b <= e); // sanity check
    s.append(line, b, e - b);
  }
  return s;
}


//
/// Functional style overload
//
TEdit::TRange
TEdit::GetSelection() const
{
  TRange r; 
  GetSelection(r.cpMin, r.cpMax);
  return r;
}


//
/// Return name of predefined Windows edit class
//
TWindow::TGetClassNameReturnType
TEdit::GetClassName()
{
  return _T("EDIT");
}

//
/// \note If the textLimit data member is nonzero, SetupWindow limits the number of
/// characters that can be entered into the edit control to textLimit -1.
//
void
TEdit::SetupWindow()
{
  TStatic::SetupWindow();

  if (TextLimit != 0)
    LimitText(TextLimit - 1);
  else
    LimitText(0); // This will remove the 32K limit under WinNT and later 
}

//
/// Always returns true if the TEdit object does not have an associated TValidator
/// object (i.e. if TEdit.Validator == 0).
/// If the edit control has a validator, and the reportErr parameter is set to true,
/// then IsValid calls the validator's Valid method. If the reportErr parameter is
/// false, IsValid calls the validator's IsValid method.
//
bool
TEdit::IsValid(bool reportError)
{
  if (Validator) {
    LPTSTR buffer = LockBuffer();
    bool valid = reportError ? Validator->Valid(buffer, this) :
                               Validator->IsValid(buffer);
    UnlockBuffer(buffer);
    return valid;
  }
  return true;
}

//
/// Sets a new validator for this control, can be 0. Cleans up the old validator
//
void
TEdit::SetValidator(TValidator* validator)
{
  delete (TStreamableBase*)Validator;  // upcast to avoid explicit call to dtor
  Validator = validator;
}

//
/// Transfers state information for TEdit controls
//
/// Transfers information for TEdit controls and sends information to the Validator
/// if one exists, and if it has the transfer option set. Transfer can perform type
/// conversion when validators are in place, for example, when TRangeValidator
/// transfers integers. The return value is the size (in bytes) of the transfer
/// data.
//
/// Delegates to the Validator if there is one & it has the transfer option set,
/// allowing the Validator to convert the text to/from the appropriate type.
/// Else passes to base, TStatic.
//
/// The return value is the size (in bytes) of the transfer data
//
uint
TEdit::Transfer(void* buffer, TTransferDirection direction)
{
  if (Validator && Validator->HasOption(voTransfer) && GetNumLines() <= 1) 
  {
    CHECK(static_cast<uint>(GetWindowTextLength()) < TextLimit);

    // Allocate a buffer for the validator.
    // Use a "big enough" size; there's no protocol for the buffer size, unfortunately.
    //
    const size_t bigEnough = max<size_t>(1024, TextLimit + 1);
    vector<tchar> text(bigEnough);
    GetText(&text[0], text.size());

    uint result = Validator->Transfer(&text[0], buffer, direction);
    if (result == 0)
      result = TStatic::Transfer(buffer, direction);
    else if (direction == tdSetData)
      SetText(&text[0]);
    return result;
  }
  return TStatic::Transfer(buffer, direction);
}


IMPLEMENT_STREAMABLE1(TEdit, TStatic);

#if !defined(BI_NO_OBJ_STREAMING)

//
/// reads an instance of TEdit from the given ipstream
//
void*
TEdit::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TStatic*)GetObject(), is);
  is >> GetObject()->Validator;
  return GetObject();
}

//
/// writes the TEdit to the given opstream
//
void
TEdit::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TStatic*)GetObject(), os);
  os << GetObject()->Validator;
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace

