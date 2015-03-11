//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of class TRichEdit.
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/window.h>
#include <owl/control.h>
#include <owl/edit.h>
#include <owl/editsear.h>
#include <owl/editfile.h>
#include <owl/richedit.h>
#include <owl/filename.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;

// Range of the editor's font size
//
const int MinPointSize = 6;
const int MaxPointSize = 128;

// System DLL providing underlying support for RichEdit controls
//
const tchar RichEditDllName[]    = _T("RICHED32.DLL");
const tchar RichEdit20DllName[]  = _T("RICHED20.DLL");

#if defined(UNICODE)
# if defined(MSFTEDIT_CLASS)
#  define OWL_USERICHEDIT41
# endif
#endif


#if defined(OWL_USERICHEDIT41)
const tchar RichEdit41DllName[]  = _T("Msftedit.dll");
#endif

////////////////////////////////////////////////////////////////////////////////
//
//

//
/// Constructs a CharacterFormat structure from the current character attributes of
/// a RICHEDIT control.
/// \note Specifying 'true' for selection will return the attribute of the character
/// at the current location if there are no block of data selected in the control.
//
TCharFormat::TCharFormat(const TRichEdit& edit, bool selection, ulong mask)
{
  PRECONDITION((HWND)edit);

  memset(this, 0, sizeof(CHARFORMAT2));

  cbSize = sizeof(CHARFORMAT);
  if(TRichEditDll::Dll()->GetVersion() >= 2)
     cbSize = sizeof(CHARFORMAT2);

  dwMask = mask;
  edit.GetCharFormat(*this, selection);
}

//
/// Toggles the bold character attribute according to the boolean parameter specified.
//
void
TCharFormat::EnableBold(bool flag)
{
  dwMask |= CFM_BOLD;
  if (flag) 
    dwEffects |= CFE_BOLD;
  else 
    dwEffects &= ~CFE_BOLD;
}

//
/// Toggles italic character attribute based on the boolean parameter specified.
//
void
TCharFormat::EnableItalic(bool flag)
{
  dwMask |= CFM_ITALIC;
  if (flag) 
    dwEffects |= CFE_ITALIC;
  else 
    dwEffects &= ~CFE_ITALIC;
}

//
/// Toggles the underline character attribute based on the boolean parameter specified.
//
void
TCharFormat::EnableUnderline(bool flag)
{
  dwMask |= CFM_UNDERLINE;
  if (flag) 
    dwEffects |= CFE_UNDERLINE;
  else 
    dwEffects &= ~CFE_UNDERLINE;
}

//
/// Toggles the strike-out character attribute based on the boolean parameter specified.
//
void
TCharFormat::EnableStrikeOut(bool flag)
{
  dwMask |= CFM_STRIKEOUT;
  if (flag) 
    dwEffects |= CFE_STRIKEOUT;
  else 
    dwEffects &= ~CFE_STRIKEOUT;
}

//
/// Toggles the protected character attribute based on the boolean parameter specified.
//
void
TCharFormat::EnableProtected(bool flag)
{
  dwMask |= CFM_PROTECTED;
  if (flag) 
    dwEffects |= CFE_PROTECTED;
  else 
    dwEffects &= ~CFE_PROTECTED;
}

//
/// Retrieves the character color stored in the CHARFORMAT structure.
/// \note	Default to system text color of no explicit color was set in the
/// CHARFORMAT structure.
//
TColor
TCharFormat::GetTextColor() const
{
  if ((dwMask & CFM_COLOR) && !(dwEffects & CFE_AUTOCOLOR))
    return TColor(crTextColor);
  return TColor::SysWindowText;
}

//
/// Updates the CHARFORMAT structure with the specified color.
/// \note	If 'TColor::None' is specified, enable the flag specifying that the color
/// should default to the system text color.
//
void
TCharFormat::SetTextColor(const TColor& color)
{
  dwMask |= CFM_COLOR;
  if (color == TColor::None) 
    dwEffects |= CFE_AUTOCOLOR;
  else {
    dwEffects &= ~CFE_AUTOCOLOR;
    crTextColor = color;
  }
}

//
/// Sets the face name of the font.
//
void
TCharFormat::SetFaceName(LPCTSTR name)
{
  // !BB No unicode support!!!!!!!!!!
  ::_tcscpy(szFaceName, name);
}

//
/// Sets the character set of the font. Valid values include the following:
/// ANSI_CHARSET, OEM_CHARSET, and SYMBOL_CHARSET.
//
void
TCharFormat::SetCharSet(uint8 charSet)
{
  bCharSet = charSet;
}

//
/// Sets the pitch and family of the font. The two lower-bits specify the pitch of
/// the font and can be one of the following values:
/// DEFAULT_PITCH, FIXED_PITCH, or VARIABLE_PITCH.
///
/// Bits 4 through 7 of the member specify the font family and can be one of the
/// following values:
/// FF_DECORATIVE, FF_DONTCARE, FF_MODERN, FF_ROMAN, FF_SCRIPT or FF_SWISS.
//
void
TCharFormat::SetPitchAndFamily(uint8 pitchFam)
{
  bPitchAndFamily = pitchFam;
}

//
/// Sets the character height.
//
void
TCharFormat::SetHeight(long height)
{
  dwMask |= CFM_SIZE;
  yHeight = height;
}

//
/// Sets the character offset from the baseline. If the parameter is positive, the
/// character is a superscript; if it is negative, the character is a subscript.
//
void
TCharFormat::SetOffset(long offset)
{
  dwMask |= CFM_OFFSET;
  yOffset = offset;
}

//
/// Transfers the information currently in the underlying CHARFORMAT structure to a
/// LOGFONT structure. This is useful when changing the editor's font, as
/// initialized LOGFONT structure can subsequently be used when invoking the FONT
/// Common Dialog (i.e., TChooseFontDialog).
//
void
TCharFormat::SetFontInfo(LOGFONT& lf) const
{
  memset(&lf, 0, sizeof(LOGFONT));

  if (dwMask & CFM_SIZE)                //  (1 Point == 20 twips)
    lf.lfHeight = yHeight/-20;

  if (dwMask & CFM_BOLD)
    lf.lfWeight = dwEffects & CFE_BOLD ? FW_BOLD : FW_NORMAL;

  if (dwMask & CFM_ITALIC)
    lf.lfItalic = (uint8)(dwEffects & CFE_ITALIC ? TRUE : FALSE);

  if (dwMask & CFM_UNDERLINE)
    lf.lfUnderline = (uint8)(dwEffects & CFE_UNDERLINE ? TRUE : FALSE);

  if (dwMask & CFM_FACE) {
    lf.lfPitchAndFamily = bPitchAndFamily;
    ::_tcscpy(lf.lfFaceName, szFaceName);
  }

  lf.lfCharSet = DEFAULT_CHARSET;
  lf.lfQuality = DEFAULT_QUALITY;
}

//
/// Initializes the underlying CHARFORMAT structure using the information stored in
/// a LOGFONT structure.
//
void
TCharFormat::GetFontInfo(const LOGFONT& lf)
{
  dwMask = (CFM_SIZE | CFM_BOLD | CFM_ITALIC | CFM_OFFSET|
            CFM_STRIKEOUT | CFM_UNDERLINE | CFM_FACE);

  yHeight = lf.lfHeight * -20;

  if (FW_BOLD == lf.lfWeight)
    dwEffects |= CFE_BOLD;
  if (lf.lfItalic)
    dwEffects |= CFE_ITALIC;
  if (lf.lfStrikeOut)
    dwEffects |= CFE_STRIKEOUT;
  if (lf.lfUnderline)
    dwEffects |= CFE_UNDERLINE;

  bPitchAndFamily = lf.lfPitchAndFamily;
  ::_tcscpy(szFaceName, lf.lfFaceName);
}

//
/// Sets Font weight (LOGFONT value). 
/// \note RichEdit 2.0 specific
void
TCharFormat::SetWeight(uint16 weigh)
{
  dwMask |= CFM_WEIGHT;
  wWeight = weigh;
}

//
/// Sets Amount to space between letters.
/// \note RichEdit 2.0 specific
void
TCharFormat::SetSpacing(int16 spacing)
{
  dwMask |= CFM_SPACING;
  wWeight = spacing;
}

//
/// Sets Background color.
/// \note RichEdit 2.0 specific
void
TCharFormat::SetBkColor(const TColor& clr)
{
  dwMask |= CFM_COLOR;
  crBackColor = clr;
}

//
// Locale ID
// RichEdit 2.0 specific
void
TCharFormat::SetLCID(LCID _lcid)
{
  dwMask |= CFM_LCID;
  lcid = _lcid;
}

//
/// Sets Style handle.
/// \note RichEdit 2.0 specific
void
TCharFormat::SetStyle(int16 style)
{
  dwMask |= CFM_STYLE;
  sStyle = style;
}

//
/// Twip size above which to kern char pair.
/// \note RichEdit 2.0 specific
void
TCharFormat::SetKerning(uint16 kern)
{
  dwMask |= CFM_STYLE;
  wKerning = kern;
}

//
/// Set underline type.
/// \note RichEdit 2.0 specific
void
TCharFormat::SetUnderlineType(uint8 utype)
{
  dwMask |= CFM_UNDERLINETYPE;
  bUnderlineType = utype;
}

//
/// Animated text like marching ants.
/// \note RichEdit 2.0 specific
void
TCharFormat::SetAnimation(uint8 anim)
{
  dwMask |= CFM_ANIMATION;
  bAnimation = anim;
}

//
/// Sets Revision author index.
/// \note RichEdit 2.0 specific
void
TCharFormat::SetRevAuthor(uint8 revav)
{
  dwMask |= CFM_REVAUTHOR;
  bRevAuthor = revav;
}

////////////////////////////////////////////////////////////////////////////////
//
//

//
/// Constructs a TFormatRange object initializing data members with the specified parameters.
//
TFormatRange::TFormatRange(HDC renderDC, HDC devDC, const TRect& renderArea,
                           const TRect& entireArea, const TCharRange& range)
{
  SetRenderDC(renderDC);
  SetTargetDC(devDC);
  SetRenderRect(renderArea);
  SetPageRect(entireArea);
  SetRange(range);
}

//
/// Sets the device context of the device to render to.
//
void
TFormatRange::SetRenderDC(HDC renderDC)
{
  hdc = renderDC;
}

//
/// Sets the device context of the target device to format for.
//
void
TFormatRange::SetTargetDC(HDC targetDC)
{
  hdcTarget = targetDC;
}

//
/// Sets the area to render to.
/// \note The specified units are in TWIPS.
//
void
TFormatRange::SetRenderRect(const TRect& renderRect)
{
  rc = renderRect;
}

//
/// Sets the entire area of the rendering device.
/// \note The specified units are in TWIPS.
//
void
TFormatRange::SetPageRect(const TRect& pgRect)
{
  rcPage = pgRect;
}

//
/// Sets the range of text to format.
//
void
TFormatRange::SetRange(const TCharRange& charRange)
{
  chrg = charRange;
}

//
/// Sets the range of text to format, specifying the starting and ending character offsets.
//
void
TFormatRange::SetRange(long start, long end)
{
  chrg.cpMin = start;
  chrg.cpMax = end;
}

////////////////////////////////////////////////////////////////////////////////
//
/// Constructs a default TParaFormat structure.
//
TParaFormat::TParaFormat(ulong mask)
{
  memset(this, 0, sizeof(PARAFORMAT2));

  cbSize = sizeof(PARAFORMAT);
  if(TRichEditDll::Dll()->GetVersion() >= 2)
     cbSize = sizeof(PARAFORMAT2);

  dwMask = mask;
}

//
/// Constructs a TParaFormat structure whose members are initialized with the
/// paragraph formatting attributes of the current selection of a rich edit control.
//
TParaFormat::TParaFormat(const TRichEdit& edit, ulong mask)
{
  PRECONDITION((HWND)edit);

  memset(this, 0, sizeof(PARAFORMAT2));

  cbSize = sizeof(PARAFORMAT);
  if(TRichEditDll::Dll()->GetVersion() >= 2)
     cbSize = sizeof(PARAFORMAT2);

  dwMask = mask;
  edit.GetParaFormat(*this);
}

//
/// Toggles the specified flag in the member which describes which information of
/// the PARAFORMAT structures is valid.
//
void
TParaFormat::ToggleMaskBit(ulong flag)
{
  dwMask ^= flag;
}

//
/// Sets the numbering options. The only valid parameter is '0' or PFN_BULLET.
//
void
TParaFormat::SetNumbering(uint16 opt)
{
  PRECONDITION(opt == 0 || opt == PFN_BULLET);

  dwMask |= PFM_NUMBERING;
  wNumbering = opt;
}

//
/// Sets the indentation of the first line in the paragraph. If the paragraph
/// formatting is being set and the 'relative' parameter is true, the 'start' value
/// is treated as a relative value that is added to the starting indentation of each
/// affected paragraph.
//
void
TParaFormat::SetStartIndent(long start, bool relative)
{
  dwMask |= PFM_STARTINDENT;
  if (relative)
    dwMask |= PFM_OFFSETINDENT;
  else
    dwMask &= ~PFM_OFFSETINDENT;

  dxStartIndent = start;
}

//
/// Sets the size of the right identation, relative to the right margin.
//
void
TParaFormat::SetRightIndent(long indent)
{
  dwMask |= PFM_RIGHTINDENT;
  dxRightIndent = indent;
}

//
/// Sets the indentation of the second line and subsequent lines, relative to the
/// starting indentation. The first line is indented if the 'offset' parameter is
/// negative, or outdented if it is positive.
//
void
TParaFormat::SetOffset(long offset)
{
  dwMask |= PFM_OFFSET;
  dxOffset = offset;
}

//
/// Sets the alignment option. The 'opt' parameter can be one of the following values:
/// - \c \b  PFA_LEFT		Paragraphs are aligned with the left margin.
/// - \c \b  PFA_RIGHT		Paragraphs are aligned with the right margin.
/// - \c \b  PFA_CENTER		Paragraphs are centered.
//
void
TParaFormat::SetAlignment(uint16 opt)
{
  dwMask |= PFM_ALIGNMENT;
  wAlignment = opt;
}

//
/// Sets the number and absolute positions of the tab stops.
//
void
TParaFormat::SetTabCount(short cnt, long* tabs)
{
  PRECONDITION(tabs || !cnt);

  dwMask |= PFM_TABSTOPS;
  cTabCount = cnt;
  for (int i=0; i<cnt; i++)
    rgxTabs[i] = *tabs++;
}

//
// Vertical spacing before para
// RichEdit 2.0 specific
void
TParaFormat::SetSpaceBefore(long space)
{
  dwMask |= PFM_SPACEBEFORE;
  dySpaceBefore = space;
}

//
// Vertical spacing after para
// RichEdit 2.0 specific
void
TParaFormat::SetSpaceAfter(long space)
{
  dwMask |= PFM_SPACEAFTER;
  dySpaceAfter = space;
}

//
// Line spacing depending on Rule
// RichEdit 2.0 specific
void
TParaFormat::SetLineSpacing(long space, uint8 rule)
{
  dwMask |= PFM_LINESPACING;
   dyLineSpacing      = space;
  bLineSpacingRule  = rule;
}

//
// Style handle
// RichEdit 2.0 specific
void
TParaFormat::SetStyle(int16 style)
{
  dwMask |= PFM_STYLE;
  sStyle = style;
}

//
// Shading in hundredths of a per cent
// Nibble 0: style, 1: cfpat, 2: cbpat
// RichEdit 2.0 specific
void
TParaFormat::SetShading(uint16 wight,uint16 style)
{
  dwMask |= PFM_SHADING;
   wShadingWeight  = wight;
  wShadingStyle   = style;
}

//
// Starting value for numbering
// RichEdit 2.0 specific
void
TParaFormat::SetNumStart(uint16 start)
{
  dwMask |= PFM_NUMBERINGSTART;
   wNumberingStart  = start;
}

//
// Alignment, roman/arabic, (), ), ., etc.
// RichEdit 2.0 specific
void
TParaFormat::SetNumStyle(uint16 style)
{
  dwMask |= PFM_NUMBERINGSTYLE;
   wNumberingStyle  = style;
}

//
// Space bet 1st indent and 1st-line text
// RichEdit 2.0 specific
void
TParaFormat::SetNumTab(uint16 tab)
{
  dwMask |= PFM_NUMBERINGTAB;
   wNumberingTab  = tab;
}

//
// Space between border and text (twips)
// RichEdit 2.0 specific
void
TParaFormat::SetBorder(uint16 space, uint16 width, uint16 border)
{
  dwMask |= PFM_BORDER;
  wBorderSpace  = space;
  wBorderWidth  = width;
  wBorders      = border;
}

////////////////////////////////////////////////////////////////////////////////
//
//

DEFINE_RESPONSE_TABLE1(TRichEdit, TEditFile)
  EV_COMMAND(CM_EDITCUT, CmEditCut),
  EV_COMMAND(CM_EDITCOPY, CmEditCopy),
  EV_COMMAND(CM_EDITPASTE, CmEditPaste),
  EV_COMMAND(CM_EDITDELETE, CmEditDelete),
  EV_COMMAND(CM_EDITCLEAR, CmEditClear),
  EV_COMMAND(CM_EDITUNDO, CmEditUndo),
  EV_COMMAND_ENABLE(CM_EDITCUT, CeHasSelect),
  EV_COMMAND_ENABLE(CM_EDITCOPY, CeHasSelect),
  EV_COMMAND_ENABLE(CM_EDITDELETE, CeHasSelect),
  EV_COMMAND_ENABLE(CM_EDITPASTE, CeEditPaste),
  EV_COMMAND_ENABLE(CM_EDITCLEAR, CeEditClear),
  EV_COMMAND_ENABLE(CM_EDITUNDO, CeEditUndo),
  EV_WM_CHAR,
  EV_WM_KEYDOWN,
  EV_WM_GETDLGCODE,
  EV_WM_SETFOCUS,
  EV_WM_KILLFOCUS,
END_RESPONSE_TABLE;

//
/// Constructor for a TRichEdit object. By default, edit control has a border and
/// its text is left-justified. Multiline edit control has horizontal vertical
/// scroll bars.
//
TRichEdit::TRichEdit(TWindow*        parent,
                     int             id,
                     LPCTSTR         text,
                     int x, int y, int w, int h,
                     LPCTSTR         fileName,
                     TModule*        module)
:
  TEditFile(parent, id, text, x, y, w, h, fileName, module)
{
  // Make sure the RichEdit DLL is available
  //
  if (!TRichEditDll::IsAvailable())
    TXCommCtrl::Raise();  // !BB Do we need a richedit-specific exception

  // Default to RTF data format
  //
  SetFormat(SF_RTF);
  // hide stuff that RichEdit doesn't support
  // Richedit support it now
  //GetSearchData().Flags |= FR_HIDEUPDOWN;

  // Undo the styles specific to "EDIT" controls; add richedit styles
  //
  ModifyStyle(ES_LOWERCASE|ES_PASSWORD|ES_OEMCONVERT|ES_UPPERCASE|ES_AUTOHSCROLL,
              ES_LEFT|WS_BORDER|WS_TABSTOP);
}

//
/// String-aware overload
//
TRichEdit::TRichEdit(
  TWindow* parent,
  int id,
  const tstring& text,
  int x, int y, int w, int h,
  const tstring& fileName,
  TModule* module
  )
  : TEditFile(parent, id, text, x, y, w, h, fileName, module)
{
  // Make sure the RichEdit DLL is available
  //
  if (!TRichEditDll::IsAvailable())
    TXCommCtrl::Raise();  // !BB Do we need a richedit-specific exception

  // Default to RTF data format
  //
  SetFormat(SF_RTF);
  // hide stuff that RichEdit doesn't support
  // Richedit support it now
  //GetSearchData().Flags |= FR_HIDEUPDOWN;

  // Undo the styles specific to "EDIT" controls; add richedit styles
  //
  ModifyStyle(ES_LOWERCASE|ES_PASSWORD|ES_OEMCONVERT|ES_UPPERCASE|ES_AUTOHSCROLL,
              ES_LEFT|WS_BORDER|WS_TABSTOP);
}

//
/// Constructor for TRichEdit associated with a MS-Windows interface element
/// created by MS-Windows from a resource definition.
///
/// By default, data transfer is enabled
//
TRichEdit::TRichEdit(TWindow*   parent,
                     int        resourceId,
                     TModule*   module)
:
  TEditFile(parent, resourceId, module ? *module : *parent->GetModule())
{
  // Make sure the RichEdit DLL is available
  //
  if (!TRichEditDll::IsAvailable())
    TXCommCtrl::Raise();  // !BB Do we need a richedit-specific exception
}


//
/// Retrieves the current character formatting in an edit control. If 'selection'
/// parameter is 'true', the attribute of the current selection is retrieved.
/// Otherwise, the default formatting attribute is retrieved.
//
ulong
TRichEdit::GetCharFormat(TCharFormat & cf, bool selection) const
{
  return CONST_CAST(TRichEdit*,this)->SendMessage(
                         EM_GETCHARFORMAT, selection ? SCF_SELECTION : 0,
                                                  TParam2(&cf));
}

//
/// Retrieves the paragraph formatting of the current selection of the rich edit control.
/// \note If more than one paragraph is selected, the structure receives the
/// attributes of the first paragraph, and the dwMask member specifies which
/// attributes are consistent throughout the entire selection.
//
ulong
TRichEdit::GetParaFormat(TParaFormat & pf) const
{
  return CONST_CAST(TRichEdit*,this)->SendMessage(EM_GETPARAFORMAT, 0,
                                                    TParam2(&pf));
}

//
/// Sets the character formatting of a rich edit control. The 'flags' parameter can
/// be one of the following:
/// - \c \b  SCF_SELECTION	Applies the formatting to the current selection, or sets the
/// default formatting if the selection is empty.
/// - \c \b  SCF_WORD		Applies the formatting to the selected word or words. If the
/// selection is empty but the insertion point is inside a word, the formatting is
/// applied to the word. This value must be used in conjunction with the
/// SCF_SELECTION value.
//
bool
TRichEdit::SetCharFormat(const TCharFormat & cf, uint flags)
{
  return SendMessage(EM_SETCHARFORMAT, TParam1(flags), TParam2(&cf)) != 0;
}

//
/// Sets the paragraph formatting of the current selection of the rich edit control.
//
bool
TRichEdit::SetParaFormat(const TParaFormat & pf)
{
  return SendMessage(EM_SETPARAFORMAT, 0, TParam2(&pf)) != 0;
}

//
/// Sets the background color of the rich edit control.
/// \note If 'TColor::None' is specified, the color is set to the window background
/// system color.
//
TColor
TRichEdit::SetBkgndColor(const TColor& bkColor)
{
  TParam1 p1 = bkColor == TColor::None ? true : false;
  TParam2 p2 = bkColor == TColor::None ? 0    : (COLORREF)bkColor;
  return TColor((COLORREF)SendMessage(EM_SETBKGNDCOLOR, p1, p2));
}

//
/// Function returns whether or not the current selection has a particular
/// attribute. The 'mask' identifies the attribute of interest. The 'effects'
/// contains the state of the attributes. The function returns
/// - \c \b  TFmtStatus::Yes : if the attribute is enabled.
/// - \c \b  TFmtStatus::No: if the attribute is absent.
/// - \c \b  TFmtStatus::Partly: if the attribute is partly present.
//
uint
TRichEdit::HasCharAttribute(ulong mask, uint32 effects)
{
  TCharFormat cf(*this);
  if (cf.dwMask & mask) {
    if (cf.dwEffects & effects)
      return Yes;
    else
      return No;
  }
  else
    return Partly;
}

//
/// Toggles a set of character attributes. The 'mask' identifies the attributes of
/// interest while 'effects' identifies the state of the attributes.
//
bool
TRichEdit::ToggleCharAttribute(ulong mask, uint32 effects)
{
  TCharFormat cf(*this);
  cf.dwMask = mask;
  cf.ToggleEffectsBit(effects);
  return SetCharFormat(cf);
}

//
/// Increases or decreases (using a positive or negative value respectively) the
/// point size of the current selection.
//
bool
TRichEdit::ChangeCharPointSize(int pointSizeDelta)
{
  TCharFormat cf(*this);
  cf.dwMask = CFM_SIZE;
  if (((cf.yHeight + 20*pointSizeDelta) <= (MaxPointSize*20)) &&
      ((cf.yHeight + 20*pointSizeDelta) >= (MinPointSize*6))) {
    cf.yHeight += 20*pointSizeDelta;
    return SetCharFormat(cf);
  }
  return false;
}

//
/// Returns true if the rich edit control has an active selection. Returns false
/// otherwise.
//
bool
TRichEdit::HasSelection() const
{
  TRange r = GetSelection();
  return r.cpMin != r.cpMax;
}

//
/// Retrieves the starting and ending character position of the selection in the
/// rich edit control.
//
void
TRichEdit::GetSelection(int& startPos, int& endPos) const
{
  TCharRange cr;
  CONST_CAST(TRichEdit*,this)->SendMessage(EM_EXGETSEL, 0, TParam2(&cr));
  startPos = cr.cpMin;
  endPos = cr.cpMax;
}

//
/// Retrieves the starting and ending character positions of the selection of the
/// richedit control.
//
void
TRichEdit::GetSelRange(TCharRange & cr) const
{
  WARN(true, _T("TRichEdit::GetSelRange is deprecated. Use GetSelection instead."));
  CONST_CAST(TRichEdit*,this)->SendMessage(EM_EXGETSEL, 0, TParam2(&cr));
}

//
/// Selects a range of characters in the rich edit control.
//
bool
TRichEdit::SetSelection(int startPos, int endPos)
{
  TCharRange cr(startPos, endPos);
  return SetSelRange(cr) >= 0;
}

//
/// Selects a range of characters in the rich edit control.
//
int
TRichEdit::SetSelRange(const TCharRange & cr)
{
  WARN(true, _T("TRichEdit::SetSelRange is deprecated. Use SetSelection instead."));
  return (int)SendMessage(EM_EXSETSEL, 0, TParam2(&cr));
}

//
/// Shows or hides the selection in the rich edit control. The 'hide' parameter
/// specifies whether to hide or show the selection. If it is 'false' the selection
/// is shown. Otherwise, the selection is hidden. The 'changeStyle' parameter
/// specifies whether to change the control's ES_NOHIDESEL window style. If this
/// parameter is 'false', the selection is temporarily shown or hidden. Otherwise,
/// the style is changed. If this parameter is 'true' and the control has the focus,
/// the selection is hidden or shown as appropriate.
//
void
TRichEdit::HideSelection(bool hide, bool changeStyle)
{
  SendMessage(EM_HIDESELECTION, TParam1(hide), TParam2(changeStyle));
}

//
/// Returns the selection type of the rich edit control. Returns SEL_EMPTY if the
/// selection is empty, or one or more of the following values:
/// - \c \b  SEL_TEXT		Text
/// - \c \b  SEL_OBJECT		At least one OLE object
/// - \c \b  SEL_MULTICHAR	More than one character of text
/// - \c \b  SEL_MULTIOBJECT	More than one OLE object
//
ulong
TRichEdit::GetSelectionType() const
{
  return CONST_CAST(TRichEdit*,this)->SendMessage(EM_SELECTIONTYPE);
}


//
// Return the lenght of the text in the richedit control
//   for RichEdit 2.0 uses EM_GETTEXTLENGTH
//
int
TRichEdit::GetTextLength() const
{
  if (TRichEditDll::Dll()->GetVersion() >= 2)
  {
    GETTEXTLENGTHEX gtl;
    gtl.flags = GTL_DEFAULT;

//Jogy ???  From MSDN: Code page used in the translation. It is CP_ACP for ANSI Code Page and 1200 for Unicode.
#if defined(UNICODE)
    gtl.codepage = 1200;
#else
    gtl.codepage = CP_ACP;
#endif

    return SendMessage(EM_GETTEXTLENGTHEX, TParam1(&gtl), TParam2(0));
  }
  else
  {
    return GetWindowTextLength();
  }
}

//
/// Retrieves a specified range of text from the rich edit control.
/// NB! Deprecated. Use GetTextRange(const TRange&) instead.
//
int
TRichEdit::GetTextRange(TTextRange & tr) const
{
  WARN(true, _T("TRichEdit::GetTextRange(TTextRange&) is deprecated. Use GetTextRange(const TRange&) instead."));
  return (int)CONST_CAST(TRichEdit*,this)->SendMessage(EM_GETTEXTRANGE, 0, TParam2(&tr));
}

//
/// Retrieves a specified range of text from the rich edit control.
/// NB! Deprecated. Use GetTextRange(const TRange&) instead.
//
int
TRichEdit::GetTextRange(const TCharRange & cr, LPTSTR buffer) const
{
  WARN(true, _T("TRichEdit::GetTextRange(const TCharRange&, LPTSTR) is deprecated. Use GetTextRange(const TRange&) instead."));
  TTextRange tr(cr, buffer);
  return GetTextRange(tr);
}

//
/// For use with CopyText
//
struct TRichEditGetTextRange
{
  const TRichEdit& edit;
  const TCharRange& range;
  TRichEditGetTextRange(const TRichEdit& e, const TCharRange& r) : edit(e), range(r) {}

  int operator()(LPTSTR buf, int)
  {
    TTextRange tr(range, buf);
    return (int)CONST_CAST(TRichEdit*, &edit)->SendMessage(EM_GETTEXTRANGE, 0, TParam2(&tr));
  }
};

//
/// Retrieves a specified range of text from the rich edit control.
/// The range is half-open; i.e. range [0,2) for "abc" returns "ab".
/// An empty string is returned if either
///
/// - the control is empty (no text), or 
/// - the range is invalid (empty or inverted), or
/// - both startPos and endPos is beyond the extents of the actual text.
///
/// If endPos is beyond the valid range then it is limited to the valid range.
/// A special case is the range [0, -1). It will return the full text.
///
/// \todo Must be tested
//
tstring 
TRichEdit::GetTextRange(const TRange& r) const
{
  const int begin = r.cpMin;
  int end = r.cpMax; // May be adjusted.
  const int n = GetTextLength();

  // Check input arguments against EM_GETTEXTRANGE requirements.
  // Note that [0, -1) for an empty control is not valid for EM_GETTEXTRANGE,
  // but we'll ignore that, since no buffer issues are involved here.
  // Otherwise we reject negative positions, as well as empty and inverted ranges.
  // EM_GETTEXTRANGE will not null-terminate the result in these cases.

  if (begin == 0 && end == -1)
  {
    end = n;
  }
  else if (begin < 0 || end < 0)
  {
    WARN(true, _T("Arguments out of range"));
    return tstring();
  }
  else if (begin == end) 
  {
    WARN(true, _T("Empty range"));
    return tstring();
  }
  else if (begin > end) 
  {
    WARN(true, _T("Inverted range"));
    return tstring();
  }

  // Return empty if the entire range is outside the extents of the actual text.
  // This is valid for EM_GETTEXTRANGE so we wont complain.

  if (begin >= n) 
    return tstring();

  // Limit end to the actual text and calculate the resulting range length.

  end = std::min(end, n);
  const int range_length = end - begin; // length of half-open range

  // Do a sanity check, in case someone changes the code, etc,
  // then finally copy!

  CHECK(begin >= 0 && end >= 0 && begin < n && end <= n); 
  TCharRange adjusted_range(begin, end);
  return CopyText(range_length, TRichEditGetTextRange(*this, adjusted_range));
}

//
/// Retrieves a specified range of text from the rich edit control.
/// NB! Deprecated. Use GetTextRange instead.
//
void
TRichEdit::GetSubText(LPTSTR str, int startPos, int endPos) const
{
  WARN(true, _T("TRichEdit::GetSubText is deprecated. Use GetTextRange instead."));
  TTextRange tr(startPos, endPos, str);
  GetTextRange(tr);
}

//
/// Retrieves the currently-selected text of the rich edit control.
//
int
TRichEdit::GetSelectedText(LPTSTR buffer) const
{
  WARN(true, _T("TRichEdit::GetSelectedText(LPTSTR) is deprecated. Use GetSelectedText() instead."));
  return (int)CONST_CAST(TRichEdit*,this)->SendMessage(EM_GETSELTEXT, 0, TParam2(buffer));
}

//
/// For use with CopyText
//
struct TRichEditGetSelectedText
{
  const TRichEdit& edit;
  TRichEditGetSelectedText(const TRichEdit& e) : edit(e){}

  int operator()(LPTSTR buf, int)
  {return (int)CONST_CAST(TRichEdit*, &edit)->SendMessage(EM_GETSELTEXT, 0, TParam2(buf));}
};

//
/// String-aware overload
//
tstring
TRichEdit::GetSelectedText() const
{
  const int n = GetSelection().GetSize();
  return (n > 0) ? CopyText(n, TRichEditGetSelectedText(*this)) : tstring();
}

//
/// Sets an upper limit to the amount of text in the richedit control.
//
void
TRichEdit::LimitText(int maxValue)
{
  SendMessage(EM_EXLIMITTEXT, 0, TParam2(maxValue));
}

//
/// Finds text within the rich edit control. The 'flags' parameter can be a
/// combination of the following values:
/// - \c \b  FT_MATCHCASE	Performs a case sensitiv search.
/// - \c \b  FT_MATCHWORD	Matches whole words.
//
int
TRichEdit::FindText(uint flags, const TFindText & ft)
{
  return (int)SendMessage(EM_FINDTEXT, TParam1(flags), TParam2(&ft));
}

//
/// Finds text within the rich edit control. The 'flags' parameter can be a
/// combination of the following values:
/// - \c \b  FT_MATCHCASE	Performs a case sensitiv search.
/// - \c \b  FT_MATCHWORD	Matches whole words.
//
int
TRichEdit::FindText(uint flags, const TCharRange & cr, LPCTSTR text)
{
  TFindText ft(cr, text);
  return FindText(flags, ft);
}

//
// Search for the specified text in the rich edit control. If found, select
// the text and return the offset of the text. Otherwise, return -1.
//
// NOTE: If the 'startPos' is -1, it is assumed that the starting position is
// the end [or beginning, depending on the direction parameter, 'up'] of the
// current selection
//
int
TRichEdit::Search(int startPos, LPCTSTR text, bool caseSensitive,
                  bool wholeWord, bool up)
{
  if (!text || !text[0])
    return -1;

  if (startPos == -1) {
    int sBeg, sEnd;
    GetSelection(sBeg, sEnd);
    startPos = up ? sBeg : sEnd;
  }

  // The current docs. mention the FT_MATCHCASE and FT_WHOLEWORD flags which
  // are not defined currently. I suspect they meant the FR_xxxx flags (used
  // in CommDlg API).
  // Yes it is FR_MATCHCASE and FR_WHOLEWORD
  TFindText findText(startPos, -1, text);
  uint flags = (caseSensitive ? FR_MATCHCASE : 0) |
               (wholeWord ? FR_WHOLEWORD : 0) |
               (up ? 0 : FR_DOWN);
  int index = FindText(flags, findText);

  //
  // If we've got a match, select the text
  //
  if (index >= 0) 
  {
    size_t end = index + ::_tcslen(text);
    if (!IsRepresentable<int>(end))
      return -1;
    SetSelection(index, static_cast<int>(end));
  }
  return index;
}

//
// Find the next work break before or after the specified character position,
// or retrieve information about the character at that position. The 'code'
// parameter can be one of the following:
//
//  WB_CLASSIFY       Returns the character class and word break flags of the
//                    character at the specified position.
//  WB_ISDELIMITER    Returns TRUE if the character at the specified position
//                    is a delimiter, or FALSE otherwise.
//  WB_LEFT           Finds the nearest character before the specified
//                    position that begins a word.
//  WB_LEFTBREAK      Finds the next word end before the specified position.
//  WB_MOVEWORDLEFT   Finds the next character that begins a word before the
//                    specified position. This value is used during CTRL+LEFT key processing.
//  WB_MOVEWORDRIGHT  Finds the next character that begins a word after the
//                    specified position. This value is used during
//                    CTRL+RIGHT key processing.
//  WB_RIGHT          Finds the next character that begins a word after the
//                    specified position.
//  WB_RIGHTBREAK     Finds the next end-of-word delimiter after the
//                    specified position.
//
// The return value is the character index of the word break, unless the
// 'code' parameter is WB_CLASSIFY or WB_ISDELIMETER
int
TRichEdit::FindWordBreak(uint code, int start)
{
  return (int)SendMessage(EM_FINDWORDBREAK, TParam1(code), TParam2(start));
}

//
// Determine which line contains the specified character in the richedit
// control.
// NOTE: The return value is zero-based.
//
int
TRichEdit::GetLineFromPos(int charPos) const
{
  return (int)CONST_CAST(TRichEdit*,this)->SendMessage(EM_EXLINEFROMCHAR,
                                                         0, TParam2(charPos));
}


//
//   Enable/Disable Auto URL detection
// RichEdit 2.0 specific
bool
TRichEdit::EnableAutoURL(bool enable)
{
  if(TRichEditDll::Dll()->GetVersion() >= 2)
    return ToBool(SendMessage(EM_AUTOURLDETECT, TParam1(enable)));
  return false;
}

//
//
// RichEdit 2.0 specific
int
TRichEdit::GetIMEMode() const
{
  if(TRichEditDll::Dll()->GetVersion() >= 2)
    return CONST_CAST(TRichEdit*,this)->SendMessage(EM_GETIMECOMPMODE);
  return ICM_NOTOPEN;
}

//
//
// RichEdit 2.0 specific
void
TRichEdit::SetLangOptions(int options)
{
  if(TRichEditDll::Dll()->GetVersion() >= 2)
    SendMessage(EM_SETLANGOPTIONS,0,TParam2(options));
}

//
//
// RichEdit 2.0 specific
int
TRichEdit::GetLangOptions() const
{
  if(TRichEditDll::Dll()->GetVersion() >= 2)
    return CONST_CAST(TRichEdit*,this)->SendMessage(EM_GETLANGOPTIONS);
  return 0;
}

//
//
// RichEdit 2.0 specific
void
TRichEdit::SetTextMode(int mode)
{
  if(TRichEditDll::Dll()->GetVersion() >= 2)
    SendMessage(EM_SETTEXTMODE,TParam1(mode));

  if(mode & TM_PLAINTEXT)
    Format = SF_TEXT;
  else if(mode & TM_RICHTEXT)
    Format = SF_RTF;
}

//
//
// RichEdit 2.0 specific
int
TRichEdit::GetTextMode() const
{
  if(TRichEditDll::Dll()->GetVersion() >= 2)
    return CONST_CAST(TRichEdit*,this)->SendMessage(EM_GETTEXTMODE);

  if(IsRTF())
    return TM_RICHTEXT;
  else
    return TM_PLAINTEXT;
}

//
//
// RichEdit 2.0 specific
void
TRichEdit::SetUndoLimit(int maxnum)
{
  if(TRichEditDll::Dll()->GetVersion() >= 2)
    SendMessage(EM_SETUNDOLIMIT,TParam1(maxnum));
}

//
//
// RichEdit 2.0 specific
void
TRichEdit::StopGroupTyping()
{
  if(TRichEditDll::Dll()->GetVersion() >= 2)
    SendMessage(EM_STOPGROUPTYPING);
}

//
// Return true if the richedit can paste the specified clipboard format, or
// false otherwise.
//
bool
TRichEdit::CanPaste(uint format) const
{
  return ToBool(CONST_CAST(TRichEdit*,this)->SendMessage(EM_CANPASTE,TParam1(format)));
}

//
//  RichEdit 2.0 specific
//
bool
TRichEdit::CanRedo() const
{
  if(TRichEditDll::Dll()->GetVersion() >= 2)
    return ToBool(CONST_CAST(TRichEdit*,this)->SendMessage(EM_CANREDO));
  return CanUndo();
}

//
//  RichEdit 2.0 specific
// The value returned is an UNDONAMEID enumeration
int
TRichEdit::GetUndoName() const
{
  if(TRichEditDll::Dll()->GetVersion() >= 2)
    return CONST_CAST(TRichEdit*,this)->SendMessage(EM_GETUNDONAME);
  return 0;
}

//
//  RichEdit 2.0 specific
// The value returned is an UNDONAMEID enumeration
int
TRichEdit::GetRedoName() const
{
  if(TRichEditDll::Dll()->GetVersion() >= 2)
    return CONST_CAST(TRichEdit*,this)->SendMessage(EM_GETREDONAME);
  return 0;
}

//
//  RichEdit 2.0 specific
//
void
TRichEdit::Redo()
{
  if(TRichEditDll::Dll()->GetVersion() >= 2)
    SendMessage(EM_REDO);
  else
    Undo();
}

//
// Paste the specified clipboard format in the rich edit control.
//
void
TRichEdit::PasteSpecial(uint format)
{
  SendMessage(EM_PASTESPECIAL, TParam1(format));
}

//
// Paste a compatible clipboard format in the rich edit control.
//
void
TRichEdit::Paste()
{
  SendMessage(WM_PASTE);
// !BB  //
// !BB  // Iterator through clipboard to locate 'pastable' format
// !BB  //
// !BB  TClipboard clip(*this);
// !BB  for (TClipboardFormatIterator iter(clip); iter; iter++) {
// !BB    if (CanPaste(iter.Current())) {
// !BB      PasteSpecial(iter.Current());
// !BB      return;
// !BB    }
// !BB  }

}

//
// Replace the contents of the rich edit control with the specified data
// stream. The 'format' parameter can be one of the following data formats,
// optionally combined with the SFF_SELECTION flag:
//
//    Value               Meaning
//    -----               -------
//    SF_TEXT             Text
//    SF_RTF              Rich-text format
//
// If the SFF_SELECTION flag is specified, the stream replaces the contents of
// the current selection. Otherwise, the stream replaces the entire contents
// of the control.
//
ulong
TRichEdit::StreamIn(uint format, TEditStream & es)
{
  return SendMessage(EM_STREAMIN, TParam1(format), TParam2(&es));
}

//
// Write the contents of the rich edit control to the specified data stream.
// The 'format' parameter can be one of the following values, optionally
// combined with the SFF_SELECTION flag:
//
//    Value               Meaning
//    -----               -------
//    SF_TEXT             Text with spaces in place of OLE objects
//    SF_RTF              Rich-text format (RTF)
//    SF_RTFNOOBJS        RTF with spaces in place of OLE object.
//    SF_TEXTIZED         Text with a text representation of OLE objects.
//
// NOTE: The SF_RTFNOOBJS option is useful if an application stores OLE
//       objects itself, as RTF representation of OLE objects is not very
//       compact.
//       If the SFF_SELECTION flag is specified, only the contents of the
//       current selection are streamed out. Otherwise, the entire contents of
//       the control are streamed out.
//
ulong
TRichEdit::StreamOut(uint format, TEditStream & es)
{
  return SendMessage(EM_STREAMOUT, TParam1(format), TParam2(&es));
}

//
// Display a portion of the richedit control's content within the specified
// rectangle.
// NOTE: The content of the control must first be formatted via a call to the
//       'FormatRange' method.
//
bool
TRichEdit::DisplayBand(TRect & rc)
{
  return SendMessage(EM_DISPLAYBAND, 0, TParam2(&rc)) != 0;
}

//
// Formats a range of text (specified via the 'chrg' member of the
// specified TFormatRange) for the device(s) specified via the 'hdcTarget'
// and 'hdc' members of the TFormatRange structure.
//
int
TRichEdit::FormatRange(const TFormatRange & fr, bool render)
{
  return (int)SendMessage(EM_FORMATRANGE, TParam1(render), TParam2(&fr));
}

//
// Frees the Formatting information cached by the RichEdit control...
//
int
TRichEdit::FormatRange()
{
  return (int)SendMessage(EM_FORMATRANGE, TParam1(TRUE), 0);
}

//
// Set the target device and line width used for WYSIWYG (what you see is
// what you get) formatting of the rich edit control.
//
bool
TRichEdit::SetTargetDevice(HDC dc, int lineWidth)
{
  return SendMessage(EM_SETTARGETDEVICE, TParam1(dc),
                       TParam2(lineWidth)) != 0;
}

//
// Force the rich edit control to send an EN_REQUESTRESIZE notification
// message to its parent window.
//
// NOTE: This message is useful during WM_SIZE processing for the parent of a
//       bottomless rich edit control.
//
void
TRichEdit::RequestResize()
{
  SendMessage(EM_REQUESTRESIZE);
}

//
// Retrieve an IRichEditOle object that a client can use to access a rich edit
// control's OLE functionality. Returns 'true' if successful, or false
// otherwise.
//
bool
TRichEdit::GetOleInterface(IRichEditOle * &pInterface) const
{
  return CONST_CAST(TRichEdit*,this)->SendMessage(EM_GETOLEINTERFACE,
                                0, TParam2((void * *)&pInterface)) != 0;
}

//
// Set an IRichEditOleCallback object that the rich edit control uses to get
// OLE-related resources and information from the client. Returns 'true' if
// successful, or false otherwise.
//
bool
TRichEdit::SetOleInterface(IRichEditOleCallback * pInterface)
{
  // Y.B 06/16/98 MFC uses EM_SETOLECALLBACK not EM_SETOLEINTERFACE ?????
  // afxxmn.inl line 620. func CRichEditCtrl::SetOLECallback(...)
  // #define EM_SETOLECALLBACK    (WM_USER + 70)  // not documented !!!!!!
  return SendMessage(EM_SETOLECALLBACK, 0, TParam2(pInterface)) != 0;
}

//
// Retrieve the event mask for the rich edit control. The event mask specifies
// which notification messages the control sends to its parent window.
//
ulong
TRichEdit::GetEventMask() const
{
  return CONST_CAST(TRichEdit*,this)->SendMessage(EM_GETEVENTMASK);
}

//
// Set the event mask for a rich edit control. The event mask specifies which
// notification messages the control sends to its parent window. The 'msk'
// parameter can be zero or more of the following values:
//
//  Value               Meaning
//  -----               -------
//  ENM_CHANGE          Sends EN_CHANGE notifications.
//  ENM_CORRECTTEXT     Sends EN_CORRECTTEXT notifications.
//  ENM_DROPFILES       Sends EN_DROPFILES notifications.
//  ENM_KEYEVENTS       Sends EN_MSGFILTER  notifications for keyboard events.
//  ENM_MOUSEEVENTS     Sends EN_MSGFILTER notifications for mouse events.
//  ENM_PROTECTED       Sends EN_PROTECTED notifications.
//  ENM_RESIZEREQUEST   Sends EN_REQUESTRESIZE notifications.
//  ENM_SCROLL          Sends EN_HSCROLL notifications.
//  ENM_SELCHANGE       Sends EN_SELCHANGE notifications.
//  ENM_UPDATE          Sends EN_UPDATE notifications
//
ulong
TRichEdit::SetEventMask(ulong msk)
{
  return SendMessage(EM_SETEVENTMASK, 0, TParam2(msk));
}

//
/// WM_GETDLGCODE handler to bypass TEdit's handler (which caters to validators).
//
uint
TRichEdit::EvGetDlgCode(const MSG* msg)
{
  return TWindow::EvGetDlgCode(msg);
}

//
/// WM_CHAR handler to bypass TEdit's handler (which caters to validators).
//
void
TRichEdit::EvChar(uint /*key*/, uint /*repeatCount*/, uint /*flags*/)
{
  DefaultProcessing();
}

//
/// WM_GETDLGCODE handler to bypass TEdit's handler (which caters to validators).
//
void
TRichEdit::EvKeyDown(uint /*key*/, uint /*repeatCount*/, uint /*flags*/)
{
  DefaultProcessing();
}

//
/// WM_KILLFOCUS handler to bypass TEdit's handler (which caters to validators).
//
void
TRichEdit::EvKillFocus(THandle hGetFocus)
{
  TControl::EvKillFocus(hGetFocus);
}

//
/// WM_SETFOCUS handler to bypass TEdit's handler (which caters to validators).
//
void
TRichEdit::EvSetFocus(THandle /*hLostFocus*/)
{
  DefaultProcessing();
}

//
// This function is called for Cut/Copy/Delete menu items to determine
// whether or not the item is enabled.
//
void
TRichEdit::CeHasSelect(TCommandEnabler& commandHandler)
{
  int sPos, ePos;

  GetSelection(sPos, ePos);
  commandHandler.Enable(sPos != ePos);
}

//
// This function is called for the Paste menu item to determine whether or
// not the item is enabled.
//
void
TRichEdit::CeEditPaste(TCommandEnabler& ce)
{
/*
  return (CountClipboardFormats() != 0) &&
    (IsClipboardFormatAvailable(CF_TEXT) ||
    IsClipboardFormatAvailable(_oleData.cfRichTextFormat) ||
    IsClipboardFormatAvailable(_oleData.cfEmbedSource) ||
    IsClipboardFormatAvailable(_oleData.cfEmbeddedObject) ||
    IsClipboardFormatAvailable(_oleData.cfFileName) ||
    IsClipboardFormatAvailable(_oleData.cfFileNameW) ||
    IsClipboardFormatAvailable(CF_METAFILEPICT) ||
    IsClipboardFormatAvailable(CF_DIB) ||
    IsClipboardFormatAvailable(CF_BITMAP) ||
    GetRichEditCtrl().CanPaste());
*/
  TClipboard clip(*this, false);
  if (clip &&
      (clip.CountClipboardFormats()!=0) &&
      (  clip.IsClipboardFormatAvailable(CF_TEXT) ||
        clip.IsClipboardFormatAvailable(CF_OEMTEXT) ||
        clip.IsClipboardFormatAvailable(CF_UNICODETEXT)
//        || clip.IsClipboardFormatAvailable(CF_METAFILEPICT) ||  //?? check
//        clip.IsClipboardFormatAvailable(CF_ENHMETAFILE) ||   //??
//        clip.IsClipboardFormatAvailable(CF_DIB)              //???
      )
      ){
    ce.Enable(true);
  }
  else
    ce.Enable(false);
}

//
// This function is called for the Clear menu item to determine whether or
// not the item is enabled.
//
void
TRichEdit::CeEditClear(TCommandEnabler& commandHandler)
{
  commandHandler.Enable(!(GetNumLines() == 1 && GetLineLength(0) == 0));
}

//
// This function is called for the Undo menu item to determine whether or
// not the item is enabled.
//
void
TRichEdit::CeEditUndo(TCommandEnabler& commandHandler)
{
  commandHandler.Enable(CanUndo());
}

//
/// Returns name of predefined Windows edit class.
//
TWindow::TGetClassNameReturnType
TRichEdit::GetClassName()
{
#if defined(OWL_USERICHEDIT41)
  if(TRichEditDll::Dll()->GetVersion() >= 4)
    return MSFTEDIT_CLASS;
#endif

  if(TRichEditDll::Dll()->GetVersion() >= 2)
    return RICHEDIT_CLASS;
  else
    return _T("RICHEDIT");
}

//
/// Updates the list of filters describing files which can be opened by
/// the rich edit control.
//
//
void
TRichEdit::SetupWindow()
{
  TEditFile::SetupWindow();

  //Load RichEdit's filter
  //
  GetFileData().SetFilter(LoadString(IDS_RICHEDITFILTER).c_str());
}


uint32
TRichEdit::CharFromPos(int16 x, int16 y)
{
  PRECONDITION(TWindow::GetHandle());
  POINTL pt;
  pt.x = x;
  pt.y = y;

  return (uint32)SendMessage(EM_CHARFROMPOS, 0, TParam2(&pt));
}

//
uint32
TRichEdit::PosFromChar(uint charIndex)
{
  PRECONDITION(TWindow::GetHandle());

  if (TRichEditDll::Dll()->GetVersion() == 2)
	  return (uint32)SendMessage(EM_POSFROMCHAR, charIndex);
  else
  {
    POINTL pt;
    SendMessage(EM_POSFROMCHAR, (TParam1)&pt, charIndex);
    return MkUint32((uint16)pt.y, (uint16)pt.x);
  }
}


//
// Callback used when reading data from a stream into a rich edit control.
//
DWORD CALLBACK
RichEditStrmInWithIStream(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
  PRECONDITION(dwCookie);

  tistream& is = *reinterpret_cast<tistream*>(dwCookie);

  // Return 0 if transfer is complete.
  //
  if (is.eof()) 
  {
   *pcb = 0;
    return 0;
  }

  // Read data in buffer.
  // Note: We know that the gcount cannot exceed cb, so we can cast unchecked.
  //
#if defined(UNICODE)
  tstring tempBuff(cb, _T('\0'));
  is.read(&tempBuff[0], cb);
  LONG n = static_cast<LONG>(is.gcount());
  ::WideCharToMultiByte(CP_ACP, 0, &tempBuff[0], n, reinterpret_cast<char*>(pbBuff), cb, NULL, NULL);
#else
  is.read(reinterpret_cast<char*>(pbBuff), cb); 
  LONG n = static_cast<LONG>(is.gcount());
#endif

  // Indicate amount of data read.
  //
  *pcb = n;
  return 0;
}

//
// Callback used when writing out the contents of a rich edit control to
// a data stream.
//
DWORD CALLBACK
RichEditStrmOutWithOstream(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
  PRECONDITION(dwCookie);

  tostream& os = *reinterpret_cast<tostream*>(dwCookie);

  // Save current stream location and write data to buffer
  //
  streampos pCnt = os.tellp();
  
#if defined(UNICODE)
  tchar * tempBuff = new tchar[cb];
  ::MultiByteToWideChar(CP_ACP, 0, (char*)pbBuff, cb, tempBuff, cb);  
  os.write(tempBuff, cb);
  delete[] tempBuff;
#else
  os.write((char*)pbBuff, cb);
#endif 

  // Indicate the number of bytes written to the file
  //
  *pcb = (LONG)(os.tellp() - pCnt);
  return 0;
}

//
// Overriden to bypass TEdit's 'Transfer' method.
// NOTE: There's no transfer-support for rich edit controls.
//
uint
TRichEdit::Transfer(void* /*buffer*/, TTransferDirection /*direction*/)
{
  // NOTE: No transfer support for rich edit control
  //
  return 0;
}

//
// Read the data from the specified stream into the rich edit control. The
// 'fmt' parameter can be one of the following data formats, optionally
// combined with the SFF_SELECTION flag:
//
//    Value               Meaning
//    -----               -------
//    SF_TEXT             Text
//    SF_RTF              Rich-text format
//
// If the SFF_SELECTION flag is specified, the stream replaces the contents of
// the current selection. Otherwise, the stream replaces the entire contents
// of the control.
//
bool
TRichEdit::ReadFromStream(tistream& is, uint fmt)
{
  DWORD_PTR cookie = reinterpret_cast<DWORD_PTR>(&is);
  TEditStream edStrm(cookie, RichEditStrmInWithIStream);
  StreamIn(fmt, edStrm);
  return edStrm.dwError == 0;
}

//
// Write the contents of the rich edit control to the specified data stream.
// The 'fmt' parameter can be one of the following values, optionally
// combined with the SFF_SELECTION flag:
//
//    Value               Meaning
//    -----               -------
//    SF_TEXT             Text with spaces in place of OLE objects
//    SF_RTF              Rich-text format (RTF)
//    SF_RTFNOOBJS        RTF with spaces in place of OLE object.
//    SF_TEXTIZED         Text with a text representation of OLE objects.
//
// NOTE: The SF_RTFNOOBJS option is useful if an application stores OLE
//       objects itself, as RTF representation of OLE objects is not very
//       compact.
//       If the SFF_SELECTION flag is specified, only the contents of the
//       current selection are streamed out. Otherwise, the entire contents of
//       the control are streamed out.
//
bool
TRichEdit::WriteToStream(tostream& os, uint fmt)
{
  DWORD_PTR cookie = reinterpret_cast<DWORD_PTR>(&os);
  TEditStream edStrm(cookie, RichEditStrmOutWithOstream);
  StreamOut(fmt, edStrm);
  return edStrm.dwError == 0;
}

//
// Read the contents of the specified file in the rich edit control. Returns
// 'true' if successful, or false otherwise.
//
bool
TRichEdit::Read(LPCTSTR fileName)
{
  if (!fileName)
    if (GetFileName())
      fileName = GetFileName();
    else
      return false;
  
  _USES_CONVERSION;
  tifstream ifs(_W2A(fileName), ios::in|ios::binary);
  if (ifs) {
    // Could check for a valid file (eg. FileSize != 0)
    // before proceeding with a call to Clear() here.
    //
    Clear();

    // Stream in data from file
    //
    if (ReadFromStream(ifs, Format)) {
      ClearModify();
      return true;
    }
  }
  return false;
}

//
// Write the contents of the edit control to the specified file. Returns
// 'true' if successful, or false otherwise.
//
bool
TRichEdit::Write(LPCTSTR fileName)
{
  if (!fileName)
    if (GetFileName())
      fileName = GetFileName();
    else
      return false;

  _USES_CONVERSION;
  tofstream ofs(_W2A(fileName), ios::out|ios::binary);
  if (ofs) {
    if (WriteToStream(ofs, Format)) {
      ClearModify();
      return true;
    }
  }
  return false;
}


IMPLEMENT_STREAMABLE1(TRichEdit, TEditFile);

#if !defined(BI_NO_OBJ_STREAMING)
//
//
void*
TRichEdit::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TEditFile*)GetObject(), is);
  return GetObject();
}

//
//
//
void
TRichEdit::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TEditFile*)GetObject(), os);
}

#endif //BI_NO_OBJ_STREAMING
//
// Object wrapper which loads the RichEdit DLL
//
TRichEditModule::TRichEditModule()
:
  TModule(
#if defined(OWL_USERICHEDIT41)
    GetVersion() >= 4 ? RichEdit41DllName :
#endif
    (GetVersion() >= 2 ? RichEdit20DllName : RichEditDllName), 
    true, true,false
  )
{
}

//
// check new rich edit library
//
static int CheckREVersion()
{
  TErrorMode loadMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);
  
#if defined(OWL_USERICHEDIT41)
  HINSTANCE Handle41 = ::LoadLibrary(RichEdit41DllName);
  if (Handle41 > HINSTANCE(HINSTANCE_ERROR))
  {
    ::FreeLibrary(Handle41);
    return 4;
  }
#endif

  HINSTANCE Handle = ::LoadLibrary(RichEdit20DllName);
  if (Handle <= HINSTANCE(HINSTANCE_ERROR))
    return 1;
  ::FreeLibrary(Handle);
  return 2;
}

//
//
//
int TRichEditModule::GetVersion(bool force_old)
{
  static int REVersion = force_old ? 1 : CheckREVersion();
  return REVersion;
}

#if defined(_BUILDOWLDLL)
// The template instances only need to be generated when building the
// ObjectWindows DLL - These instances are exported by OWL and imported
// by user code.

  //
  // Export template of TDllLoader<TRichEditModule> when building ObjectWindows
  // DLL and provide import declaration of DLL instance for users of the class.
  //
  template class _OWLCLASS TDllLoader<TRichEditModule>;
#endif

} // OWL namespace
/* ========================================================================== */

