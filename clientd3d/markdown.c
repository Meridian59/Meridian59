// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * markdown.c:  Markdown-to-RichEdit renderer using md4c.
 *
 * Renders markdown and legacy format codes in Rich Edit controls.
 * md4c parses the markdown structure and delivers block/span/text callbacks.
 * Legacy format codes are processed within text segments so both systems
 * can be applied.
 */

#include "client.h"
#include "markdown.h"

#include <md4c.h>
#include <shellapi.h>

// Legacy format code types.
static const int LEGACY_COLOR = 0x01;
static const int LEGACY_STYLE = 0x02;

/************************************************************************/
/*
 * Link registry: stores URLs for clickable links in Rich Edit controls.
 * Cleared and rebuilt each time RichEditSetMarkdownText is called.
 */
static const int MD_MAX_LINKS = 128;
static const int MD_MAX_URL_LEN = 512;

struct MdLink {
   HWND hwnd;
   LONG charStart;
   LONG charEnd;
   char url[MD_MAX_URL_LEN];
};

static struct MdLink s_linkRegistry[MD_MAX_LINKS];
static int s_numLinks = 0;

/************************************************************************/
/*
 * MdClearLinksForHwnd:  Removes all stored links for a specific HWND
 *   from the global link registry, compacting the array.
 */
static void MdClearLinksForHwnd(HWND hwnd)
{
   int dst = 0;
   for (int i = 0; i < s_numLinks; i++)
   {
      if (s_linkRegistry[i].hwnd != hwnd)
         s_linkRegistry[dst++] = s_linkRegistry[i];
   }
   s_numLinks = dst;
}

/************************************************************************/
/*
 * MdClearLinks:  Removes all stored links for a Rich Edit control.
 *   Public wrapper around MdClearLinksForHwnd.
 */
void MdClearLinks(HWND hwnd)
{
   MdClearLinksForHwnd(hwnd);
}

/************************************************************************/
/*
 * MdStoreLink:  Adds a clickable link to the global link registry, storing
 *   the HWND, character range, and URL for later hit-testing.
 */
static void MdStoreLink(HWND hwnd, LONG start, LONG end,
                        const char *url, int urlLen)
{
   if (s_numLinks >= MD_MAX_LINKS)
      return;
   struct MdLink *link = &s_linkRegistry[s_numLinks++];
   link->hwnd = hwnd;
   link->charStart = start;
   link->charEnd = end;
   int copyLen = (urlLen < MD_MAX_URL_LEN - 1) ? urlLen : MD_MAX_URL_LEN - 1;
   memcpy(link->url, url, copyLen);
   link->url[copyLen] = '\0';
}

/************************************************************************/
/*
 * MdAdjustLinksForTrim:  Shifts stored link positions after the edit
 *   control trims old text.
 */
void MdAdjustLinksForTrim(HWND hwnd, LONG charsRemoved)
{
   int dst = 0;
   for (int i = 0; i < s_numLinks; i++)
   {
      if (s_linkRegistry[i].hwnd != hwnd)
      {
         s_linkRegistry[dst++] = s_linkRegistry[i];
         continue;
      }
      s_linkRegistry[i].charStart -= charsRemoved;
      s_linkRegistry[i].charEnd -= charsRemoved;
      if (s_linkRegistry[i].charEnd > 0)
         s_linkRegistry[dst++] = s_linkRegistry[i];
   }
   s_numLinks = dst;
}

/************************************************************************/
/*
 * Table buffer: stores table cell contents during md4c parsing so we can
 * calculate column widths and output a properly aligned table.
 */
static const int MD_MAX_TABLE_COLS = 8;
static const int MD_MAX_TABLE_ROWS = 64;
static const int MD_MAX_CELL_TEXT = 128;

struct MdTableData {
   char   cells[MD_MAX_TABLE_ROWS][MD_MAX_TABLE_COLS][MD_MAX_CELL_TEXT];
   int    cellLens[MD_MAX_TABLE_ROWS][MD_MAX_TABLE_COLS];
   bool   rowIsHeader[MD_MAX_TABLE_ROWS];
   int    numRows;
   int    numCols;
   int    currentRow;
   int    currentCol;
   bool   inCell;
};

struct LegacyCode {
   char  code;
   int   type;
   int   data;
};

static const struct LegacyCode legacy_codes[] = {
   { 'r', LEGACY_COLOR, PALETTERGB(128,   0,   0) },
   { 'g', LEGACY_COLOR, PALETTERGB(  0, 100,   0) },
   { 'l', LEGACY_COLOR, PALETTERGB(  0, 255,   0) },
   { 'b', LEGACY_COLOR, PALETTERGB(  0,   0, 255) },
   { 'c', LEGACY_COLOR, PALETTERGB(  0, 255, 255) },
   { 'k', LEGACY_COLOR, PALETTERGB(  0,   0,   0) },
   { 'w', LEGACY_COLOR, PALETTERGB(255, 255, 255) },
   { 'y', LEGACY_COLOR, PALETTERGB(255, 255,   0) },
   { 'o', LEGACY_COLOR, PALETTERGB(255, 172,  28) },
   { 'p', LEGACY_COLOR, PALETTERGB(255, 192, 203) },
   { 'B', LEGACY_STYLE, STYLE_BOLD },
   { 'I', LEGACY_STYLE, STYLE_ITALIC },
   { 'U', LEGACY_STYLE, STYLE_UNDERLINE },
   { 'n', LEGACY_STYLE, STYLE_RESET },
};

static const int num_legacy_codes = sizeof(legacy_codes) / sizeof(legacy_codes[0]);

// Characters that are valid legacy format codes after a ` or ~ prefix.
static const char *legacy_code_chars = "rglbckwyopBIUn";

/* Single-byte sentinel range for legacy format codes. Each legacy code char
 * maps to a unique byte: legacy_code_chars[i] -> SENTINEL_FIRST + i.
 * The range 0x0E-0x1B (SO through ESC) avoids collisions with whitespace
 * (0x09-0x0D) and null. md4c is patched to classify these bytes as
 * punctuation so they don't break emphasis delimiter flanking rules. */
static const unsigned char SENTINEL_FIRST = 0x0E;
static const unsigned char SENTINEL_LAST  = 0x1B;

static inline bool IsSentinel(char ch)
{
   return (unsigned char)ch >= SENTINEL_FIRST && (unsigned char)ch <= SENTINEL_LAST;
}

static inline int SentinelIndex(char ch)
{
   return (unsigned char)ch - SENTINEL_FIRST;
}

/************************************************************************/
/*
 * MdNormalizeLegacyCodes:  Returns a strdup'd copy of text with legacy
 *   format code sequences (backtick or tilde + code char) replaced by
 *   single-byte sentinels. Each code char maps to a unique sentinel byte
 *   so md4c treats the entire sequence as one punctuation character,
 *   preventing interference with emphasis delimiter flanking rules.
 *   Caller must free the returned string.
 */
static char *MdNormalizeLegacyCodes(const char *text)
{
   char *buf = strdup(text);
   if (!buf)
      return buf;

   char prevSrcChar = '\0';
   char *dst = buf;
   for (const char *src = buf; *src != '\0'; src++)
   {
      if ((*src == '`' || *src == '~') && *(src + 1) != '\0')
      {
         /* Don't consume ~X as a legacy code when preceded by another ~.
          * This preserves ~~strikethrough~~ syntax for words starting with
          * a legacy code letter (e.g. ~~cowardice~~ where ~c is cyan).
          * After guarding one pair, reset prevSrcChar so the next ~ can
          * start a new legacy code (e.g. ~~word~~~n has ~~ + ~n). */
         if (*src == '~' && prevSrcChar == '~')
         {
            prevSrcChar = '\0';
            *dst++ = *src;
            continue;
         }

         const char *found = strchr(legacy_code_chars, *(src + 1));
         if (found)
         {
            prevSrcChar = *(src + 1);
            *dst++ = (char)(SENTINEL_FIRST + (int)(found - legacy_code_chars));
            src++;   // skip code char
            continue;
         }
      }
      prevSrcChar = *src;
      *dst++ = *src;
   }
   *dst = '\0';

   return buf;
}

/************************************************************************/
/*
 * MdFixTaskListCodes:  In-place fix for task list items with legacy format
 *   codes between the list marker and the checkbox. md4c requires "- [ ]"
 *   or "- [x]" at the start of a list item to detect a task list; any text
 *   between "- " and "[" breaks detection. This moves format codes to
 *   after the checkbox.
 */
static void MdFixTaskListCodes(char *text)
{
   char *p = text;

   while (*p != '\0')
   {
      // Skip leading whitespace (indented list items).
      while (*p == ' ' || *p == '\t')
         p++;

      // Check for unordered list marker: -, *, or +
      int isList = 0;
      if ((*p == '-' || *p == '*' || *p == '+') && *(p + 1) == ' ')
      {
         p += 2;
         isList = 1;
      }
      else if (*p >= '0' && *p <= '9')
      {
         // Check for ordered list marker: digits followed by . or )
         char *numStart = p;
         while (*p >= '0' && *p <= '9')
            p++;
         if ((*p == '.' || *p == ')') && *(p + 1) == ' ')
         {
            p += 2;
            isList = 1;
         }
         else
         {
            p = numStart;
         }
      }

      if (isList)
      {
         // Scan for contiguous format code sentinels.
         char *codeStart = p;
         while (IsSentinel(*p))
            p++;
         int codeLen = (int)(p - codeStart);

         // If format codes are followed by a checkbox, swap them.
         if (codeLen > 0 &&
             p[0] == '[' &&
             (p[1] == ' ' || p[1] == 'x' || p[1] == 'X') &&
             p[2] == ']')
         {
            int cbLen = (p[3] == ' ') ? 4 : 3;

            char cb[4];
            memcpy(cb, p, cbLen);
            memmove(codeStart + cbLen, codeStart, codeLen);
            memcpy(codeStart, cb, cbLen);
         }
      }

      // Advance to next line.
      while (*p != '\0' && *p != '\n' && *p != '\r')
         p++;
      if (*p == '\r' && *(p + 1) == '\n')
         p += 2;
      else if (*p != '\0')
         p++;
   }
}

/************************************************************************/
/*
 * MdPreserveBlankLines:  Returns a malloc'd copy of text with extra blank
 *   lines preserved. md4c collapses multiple blank lines into a single
 *   paragraph break. To preserve player-intended spacing, each extra blank
 *   line beyond the first is replaced with a line containing a single
 *   space, which md4c treats as its own paragraph (rendering as a visual
 *   blank line). Caller must free the returned string.
 */
static char *MdPreserveBlankLines(const char *text)
{
   int len = (int)strlen(text);
   // Worst case: every char becomes 3 chars (\n -> \n \n).
   char *buf = (char *)malloc(len * 3 + 1);
   if (!buf)
      return strdup(text);

   const char *src = text;
   char *dst = buf;
   int consecutiveNewlines = 0;

   while (*src != '\0')
   {
      if (*src == '\n' || *src == '\r')
      {
         // Handle \r\n as a single newline.
         if (*src == '\r' && *(src + 1) == '\n')
            src++;

         consecutiveNewlines++;

         if (consecutiveNewlines > 2)
         {
            /* Extra blank line: insert a non-breaking space paragraph.
             * 0xA0 is not whitespace in CommonMark, so md4c treats this
             * line as real content and creates a visible blank paragraph. */
            *dst++ = '\n';
            *dst++ = '\xA0';
         }

         *dst++ = '\n';
         src++;
      }
      else
      {
         consecutiveNewlines = 0;
         *dst++ = *src++;
      }
   }

   *dst = '\0';

   /* Trim trailing whitespace so trailing blank lines don't create
    * empty paragraphs with unnecessary vertical space. */
   while (dst > buf && ((unsigned char)*(dst - 1) <= ' ' || *(dst - 1) == '\xA0'))
      *(--dst) = '\0';

   return buf;
}

// Userdata passed through all md4c callbacks.
struct MdRenderState {
   HWND     hwnd;
   COLORREF defaultColor;
   COLORREF legacyColor;
   BYTE     legacyStyle;
   int      headingLevel;
   bool     inCodeBlock;
   int      listDepth;
   int      orderedListCounter;
   int      quoteDepth;
   bool     needsNewline;
   bool     firstBlockDone;
   bool     firstParagraphInLI;

   // Span stack: tracks md4c-driven bold/italic/underline.
   int      mdBold;
   int      mdItalic;
   int      mdUnderline;
   int      mdStrikethrough;
   bool     inLink;
   bool     inTable;
   bool     linksEnabled;
   bool     isChatPath;

   // Table rendering: when non-NULL, text is buffered for aligned output.
   struct MdTableData *tableData;

   // Link tracking: URL and start position for current MD_SPAN_A.
   char     linkUrl[MD_MAX_URL_LEN];
   int      linkUrlLen;
   LONG     linkCharStart;
};

/************************************************************************/
/*
 * MdBuildCharFormat:  Populates a CHARFORMAT2 combining the current md4c
 *   span state (bold, italic, underline, strikethrough) with the legacy
 *   format code state (color, style) and base font settings.
 */
static void MdBuildCharFormat(struct MdRenderState *s, CHARFORMAT2 *cf)
{
   memset(cf, 0, sizeof(*cf));
   cf->cbSize = sizeof(*cf);
   cf->dwMask = CFM_COLOR | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT |
                CFM_SIZE | CFM_FACE | CFM_BACKCOLOR;
   cf->crTextColor = s->legacyColor;
   cf->dwEffects = CFE_AUTOBACKCOLOR;
   strcpy(cf->szFaceName, "Times New Roman");

   // Link spans: use configurable link color.
   if (s->inLink)
      cf->crTextColor = GetColor(COLOR_LINKFGD);

   /* Code blocks/tables: configurable monospace font.
    * Code uses distinct foreground color; tables use default text color.
    * No crBackColor -- Rich Edit only paints background behind individual
    * characters, not the full line width, creating ugly partial rectangles.
    * Inline code spans (backtick-delimited) are intentionally not supported
    * because backtick is already the legacy format code prefix. The backtick
    * has been used for color/style codes (`r, `B, etc.) for decades and
    * overloading it with a second meaning creates an unresolvable conflict. */
   if (s->inCodeBlock || s->inTable)
   {
      LOGFONT *codeLf = GetLogfont(FONT_CODE);
      strcpy(cf->szFaceName, codeLf->lfFaceName);
      if (s->inCodeBlock)
         cf->crTextColor = GetColor(COLOR_CODEFGD);

      // Convert LOGFONT height (device units) to CHARFORMAT2 yHeight (twips).
      HDC hdc = GetDC(NULL);
      int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
      ReleaseDC(NULL, hdc);
      cf->yHeight = MulDiv(abs(codeLf->lfHeight), 1440, dpi);
      return;
   }

   /* Blockquotes: use configurable foreground color (no background --
    * Rich Edit crBackColor only paints behind characters, not the full
    * line width, which looks broken). */
   if (s->quoteDepth > 0 && !s->inCodeBlock)
   {
      cf->crTextColor = GetColor(COLOR_QUOTEFGD);
   }

   // Heading sizes: h1=280, h2=240, h3=200, others=normal (180 twips = 9pt)
   int baseTwips = 180;
   switch (s->headingLevel)
   {
   case 1: baseTwips = 280; break;
   case 2: baseTwips = 240; break;
   case 3: baseTwips = 200; break;
   default: break;
   }
   cf->yHeight = baseTwips;

   // Combine md4c bold with legacy bold.
   bool isBold = (s->mdBold > 0) || (s->legacyStyle & STYLE_BOLD) ||
                 (s->headingLevel > 0);
   bool isItalic = (s->mdItalic > 0) || (s->legacyStyle & STYLE_ITALIC);
   bool isUnderline = (s->mdUnderline > 0) || (s->legacyStyle & STYLE_UNDERLINE);
   bool isStrikethrough = (s->mdStrikethrough > 0);

   if (isBold)
      cf->dwEffects |= CFE_BOLD;
   if (isItalic)
      cf->dwEffects |= CFE_ITALIC;
   if (isUnderline)
      cf->dwEffects |= CFE_UNDERLINE;
   if (isStrikethrough)
      cf->dwEffects |= CFE_STRIKEOUT;
}

/************************************************************************/
/*
 * MdGetCharCount:  Returns the character count in a Rich Edit control
 *   consistent with EM_SETSEL and ENLINK character indices.
 *   WM_GETTEXTLENGTH (Edit_GetTextLength) counts \r\n as 2 bytes, but
 *   Rich Edit 2.0 selection indices count \r\n as 1 character.
 *   EM_GETTEXTLENGTHEX with GTL_NUMCHARS returns the correct count
 *   for selection operations.
 */
static LONG MdGetCharCount(HWND hwnd)
{
   GETTEXTLENGTHEX gtl;
   gtl.flags = GTL_NUMCHARS;
   gtl.codepage = CP_ACP;
   return (LONG)SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
}

/************************************************************************/
/*
 * MdAppendText:  Appends a text segment to the Rich Edit control with
 *   current formatting. The text does NOT need to be null-terminated;
 *   we copy len bytes.
 */
static void MdAppendText(struct MdRenderState *s, const char *text, int len)
{
   if (len <= 0)
      return;

   char *buf = (char *)malloc(len + 1);
   if (!buf)
      return;
   memcpy(buf, text, len);
   buf[len] = '\0';

   HWND hwnd = s->hwnd;
   LONG txtlen = MdGetCharCount(hwnd);
   Edit_SetSel(hwnd, txtlen, txtlen);

   CHARFORMAT2 cformat;
   MdBuildCharFormat(s, &cformat);
   SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cformat);
   Edit_ReplaceSel(hwnd, buf);

   free(buf);
}

/************************************************************************/
/*
 * MdCellVisualWidth:  Returns the visual display width of a table cell's
 *   text after accounting for checkbox substitution: [x], [X] and [ ]
 *   each display as a single glyph character instead of three characters.
 */
static int MdCellVisualWidth(const char *text, int len)
{
   int width = 0;
   for (int i = 0; i < len; i++)
   {
      // Legacy format code sentinels are invisible.
      if (IsSentinel(text[i]))
         continue;

      if (i + 2 < len && text[i] == '[' && text[i + 2] == ']' &&
          (text[i + 1] == 'x' || text[i + 1] == 'X' || text[i + 1] == ' '))
      {
         width++;   // 3-char pattern displays as 1 glyph
         i += 2;    // skip ] (loop will advance past it)
      }
      else
      {
         width++;
      }
   }
   return width;
}

/************************************************************************/
/*
 * MdOutputCellText:  Outputs a table cell's text, replacing [x]/[X] with
 *   a green U+2611 and [ ] with a gray U+2610, similar to task list
 *   rendering. Other text is output normally via MdAppendText.
 */
static void MdOutputCellText(struct MdRenderState *s, const char *text, int len)
{
   int i = 0;
   while (i < len)
   {
      // Legacy format code sentinels: update color/style state without output.
      if (IsSentinel(text[i]))
      {
         int idx = SentinelIndex(text[i]);
         const struct LegacyCode *lc = &legacy_codes[idx];

         if (lc->type == LEGACY_COLOR)
            s->legacyColor = lc->data;
         else if (lc->type == LEGACY_STYLE)
         {
            switch (lc->data)
            {
            case STYLE_NORMAL:
               s->legacyStyle = STYLE_NORMAL;
               break;
            case STYLE_RESET:
               s->legacyStyle = STYLE_NORMAL;
               s->legacyColor = s->defaultColor;
               break;
            default:
               s->legacyStyle = s->legacyStyle ^ lc->data;
               break;
            }
         }
         i++;
         continue;
      }

      // Check for checkbox pattern: [x], [X], or [ ]
      if (i + 2 < len && text[i] == '[' && text[i + 2] == ']' &&
          (text[i + 1] == 'x' || text[i + 1] == 'X' || text[i + 1] == ' '))
      {
         bool checked = (text[i + 1] != ' ');
         WCHAR glyph[2];
         glyph[0] = checked ? 0x2611 : 0x2610;
         glyph[1] = L'\0';

         /* Use inherited text color (no forced color) so checkboxes
          * match surrounding text and respond to legacy color codes. */
         LONG pos = MdGetCharCount(s->hwnd);
         Edit_SetSel(s->hwnd, pos, pos);

         CHARFORMAT2 taskCf;
         MdBuildCharFormat(s, &taskCf);
         SendMessage(s->hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&taskCf);

         SETTEXTEX ste;
         ste.flags = ST_SELECTION;
         ste.codepage = 1200;  // UTF-16
         SendMessage(s->hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)glyph);

         i += 3;
      }
      else
      {
         // Find the next checkbox, legacy code, or end of text.
         int start = i;
         while (i < len)
         {
            if (i + 2 < len && text[i] == '[' && text[i + 2] == ']' &&
                (text[i + 1] == 'x' || text[i + 1] == 'X' || text[i + 1] == ' '))
               break;
            if (IsSentinel(text[i]))
               break;
            i++;
         }
         MdAppendText(s, text + start, i - start);
      }
   }
}

/************************************************************************/
/*
 * MdAppendUnicode:  Appends a single Unicode glyph to the Rich Edit
 *   control at the current insertion point, using the current character
 *   format from MdBuildCharFormat. Inserted via EM_SETTEXTEX with
 *   UTF-16 codepage.
 */
static void MdAppendUnicode(struct MdRenderState *s, WCHAR glyph)
{
   WCHAR buf[2] = { glyph, L'\0' };

   LONG pos = MdGetCharCount(s->hwnd);
   Edit_SetSel(s->hwnd, pos, pos);

   CHARFORMAT2 cf;
   MdBuildCharFormat(s, &cf);
   SendMessage(s->hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

   SETTEXTEX ste;
   ste.flags = ST_SELECTION;
   ste.codepage = 1200;  // UTF-16
   SendMessage(s->hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)buf);
}

/************************************************************************/
/*
 * MdAppendNewline:  Appends a newline to the Rich Edit control with
 *   default formatting.
 */
static void MdAppendNewline(struct MdRenderState *s)
{
   MdAppendText(s, "\r\n", 2);
}

/************************************************************************/
/*
 * MdInsertQuoteBar:  Inserts the blockquote left-bar prefix (one U+258C
 *   per nesting level plus a trailing space) in the configurable bar
 *   color. Called at the start of each paragraph inside a blockquote and
 *   after every line break (SOFTBR / BR) so the bar appears on every
 *   visual line.
 */
static void MdInsertQuoteBar(struct MdRenderState *s)
{
   if (s->quoteDepth <= 0)
      return;

   WCHAR barBuf[16];
   int barLen = 0;
   for (int q = 0; q < s->quoteDepth && barLen < 8; q++)
      barBuf[barLen++] = 0x258C;  // \u258c
   barBuf[barLen++] = L' ';
   barBuf[barLen] = L'\0';

   CHARFORMAT2 barCf;
   MdBuildCharFormat(s, &barCf);
   barCf.crTextColor = GetColor(COLOR_QUOTEBGD);
   barCf.dwEffects &= ~(CFE_ITALIC | CFE_BOLD | CFE_UNDERLINE);

   LONG pos = MdGetCharCount(s->hwnd);
   Edit_SetSel(s->hwnd, pos, pos);
   SendMessage(s->hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&barCf);

   SETTEXTEX ste;
   ste.flags = ST_SELECTION;
   ste.codepage = 1200;  // UTF-16
   SendMessage(s->hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)barBuf);
}

/************************************************************************/
/*
 * MdBeginParagraph:  Starts a new paragraph in the Rich Edit control
 *   with PARAFORMAT2. Inserts a newline to create a new paragraph
 *   (unless this is the first block), then applies paragraph-level
 *   formatting: spacing before/after, left indent, and hanging indent
 *   for list items.
 *
 *   spaceBefore/spaceAfter are in twips (1/20 pt, so 120 = 6pt).
 *   leftIndent is the absolute left margin in twips.
 *   hangingIndent is the offset of 2nd+ lines relative to the 1st line
 *   (negative = hanging indent where the bullet/number hangs left).
 */
static void MdBeginParagraph(struct MdRenderState *s, int spaceBefore,
                             int spaceAfter, int leftIndent, int hangingIndent)
{
   if (s->firstBlockDone)
      MdAppendText(s, "\r\n", 2);

   PARAFORMAT2 pf;
   memset(&pf, 0, sizeof(pf));
   pf.cbSize = sizeof(pf);
   pf.dwMask = PFM_STARTINDENT | PFM_OFFSET | PFM_SPACEBEFORE | PFM_SPACEAFTER;
   pf.dxStartIndent = leftIndent;
   pf.dxOffset = hangingIndent;
   pf.dySpaceBefore = spaceBefore;
   pf.dySpaceAfter = spaceAfter;
   SendMessage(s->hwnd, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
}

/************************************************************************/
/*
 * MdResetParaFormat:  Resets paragraph formatting to defaults (no indent,
 *   no spacing). Used after newlines in multi-line blocks (code, tables)
 *   to prevent spacing from the first paragraph from propagating to
 *   subsequent lines.
 */
static void MdResetParaFormat(struct MdRenderState *s)
{
   PARAFORMAT2 pf;
   memset(&pf, 0, sizeof(pf));
   pf.cbSize = sizeof(pf);
   pf.dwMask = PFM_STARTINDENT | PFM_OFFSET | PFM_SPACEBEFORE | PFM_SPACEAFTER;
   SendMessage(s->hwnd, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
}

/************************************************************************/
/*
 * MdAppendCodeText:  Appends code block text, splitting on newlines and
 *   resetting paragraph format after each line break. This prevents
 *   dySpaceBefore from the initial code paragraph from propagating to
 *   every line of the block. Preserves dxStartIndent for consistent
 *   code block indentation.
 */
static void MdAppendCodeText(struct MdRenderState *s, const char *text,
                             int len, int indent)
{
   const char *start = text;
   const char *end = text + len;

   for (const char *p = text; p < end; p++)
   {
      if (*p == '\n')
      {
         int segLen = (int)(p - start);
         if (segLen > 0)
            MdAppendText(s, start, segLen);

         MdAppendText(s, "\r\n", 2);

         // Reset spacing on the new paragraph but keep indent.
         PARAFORMAT2 pf;
         memset(&pf, 0, sizeof(pf));
         pf.cbSize = sizeof(pf);
         pf.dwMask = PFM_STARTINDENT | PFM_OFFSET | PFM_SPACEBEFORE | PFM_SPACEAFTER;
         pf.dxStartIndent = indent;
         SendMessage(s->hwnd, EM_SETPARAFORMAT, 0, (LPARAM)&pf);

         start = p + 1;
      }
   }

   int remaining = (int)(end - start);
   if (remaining > 0)
      MdAppendText(s, start, remaining);
}

/************************************************************************/
/*
 * MdProcessLegacyText:  Processes a text segment for legacy format codes
 *   and outputs the text with proper formatting. Format codes are consumed
 *   and the remaining text is appended via MdAppendText with updated state.
 */
static void MdProcessLegacyText(struct MdRenderState *s,
                                const char *text, int len)
{
   const char *str = text;
   const char *end = text + len;

   for (const char *ptr = text; ptr < end; ptr++)
   {
      if (!IsSentinel(*ptr))
         continue;

      int idx = SentinelIndex(*ptr);
      const struct LegacyCode *lc = &legacy_codes[idx];

      // Output text segment before the format code.
      int segLen = (int)(ptr - str);
      if (segLen > 0)
         MdAppendText(s, str, segLen);

      if (lc->type == LEGACY_COLOR)
         s->legacyColor = lc->data;
      else if (lc->type == LEGACY_STYLE)
      {
         switch (lc->data)
         {
         case STYLE_NORMAL:
            s->legacyStyle = STYLE_NORMAL;
            break;
         case STYLE_RESET:
            s->legacyStyle = STYLE_NORMAL;
            s->legacyColor = s->defaultColor;
            break;
         default:
            s->legacyStyle = s->legacyStyle ^ lc->data;
            break;
         }
      }

      str = ptr + 1;   // Next segment starts after sentinel.
   }

   int remaining = (int)(end - str);
   if (remaining > 0)
      MdAppendText(s, str, remaining);
}

/************************************************************************/
/*
 * MdEnterBlock:  Returns 0 on success. md4c callback for entering a
 *   block element.
 */
static int MdEnterBlock(MD_BLOCKTYPE type, void *detail, void *userdata)
{
   struct MdRenderState *s = (struct MdRenderState *)userdata;

   /* Reset legacy format codes at each block boundary. Legacy codes (~B, ~r, etc.)
    * were always inline in the pre-markdown client -- they never carried across
    * paragraphs. Preserve that behavior by resetting at each new block. */
   if (type != MD_BLOCK_DOC)
   {
      s->legacyColor = s->defaultColor;
      s->legacyStyle = STYLE_NORMAL;
   }

   switch (type)
   {
   case MD_BLOCK_DOC:
      break;

   case MD_BLOCK_P:
   {
      /* Blockquotes use no indent -- the "|" bar prefix handles nesting.
       * Lists indent based on nesting depth. */
      int indent = 0;
      if (s->listDepth > 0)
         indent = (s->quoteDepth + s->listDepth) * 360;

      if (s->firstParagraphInLI)
      {
         /* LI already created this paragraph; don't insert another newline.
          * Subsequent P blocks in the same LI will create new paragraphs. */
         s->firstParagraphInLI = false;
      }
      else
      {
         /* Use tighter spacing inside blockquotes so the bar looks
          * like a continuous strip; normal paragraphs keep 120 twips. */
         int spaceBefore = s->firstBlockDone
            ? (s->quoteDepth > 0 ? 40 : 120)
            : 0;
         MdBeginParagraph(s, spaceBefore, 0, indent, 0);
      }

      // Insert the blockquote left-bar prefix on this paragraph.
      MdInsertQuoteBar(s);

      s->firstBlockDone = true;
      break;
   }

   case MD_BLOCK_H:
   {
      MD_BLOCK_H_DETAIL *h = (MD_BLOCK_H_DETAIL *)detail;
      s->headingLevel = h->level;
      int spaceBefore = s->firstBlockDone ? 200 : 0;
      MdBeginParagraph(s, spaceBefore, 60, 0, 0);
      s->firstBlockDone = true;
      break;
   }

   case MD_BLOCK_CODE:
   {
      s->inCodeBlock = true;
      int spaceBefore = s->firstBlockDone ? 40 : 0;
      MdBeginParagraph(s, spaceBefore, 0, 0, 0);
      s->firstBlockDone = true;
      break;
   }

   case MD_BLOCK_UL:
   case MD_BLOCK_OL:
      s->listDepth++;
      if (type == MD_BLOCK_OL)
      {
         MD_BLOCK_OL_DETAIL *ol = (MD_BLOCK_OL_DETAIL *)detail;
         s->orderedListCounter = ol->start;
      }
      break;

   case MD_BLOCK_LI:
   {
      MD_BLOCK_LI_DETAIL *li = (MD_BLOCK_LI_DETAIL *)detail;

      /* Indent based on blockquote + list nesting depth.
       * Hanging indent places the bullet/number left of the body text. */
      int indent = s->quoteDepth * 360 + s->listDepth * 360;
      int spaceBefore = s->firstBlockDone ? 20 : 0;
      MdBeginParagraph(s, spaceBefore, 20, indent, -200);

      if (li->is_task)
      {
         /* Task list: Unicode checkbox glyphs.
          * U+2611 = BALLOT BOX WITH CHECK, U+2610 = BALLOT BOX.
          * Inserted via EM_SETTEXTEX with Unicode codepage since the
          * control is created with "RichEdit20A" (ANSI class name) but
          * Rich Edit 2.0 supports Unicode text insertion internally.
          * Uses inherited text color so legacy color codes can tint it. */
         WCHAR glyph[2];
         if (li->task_mark == 'x' || li->task_mark == 'X')
            glyph[0] = 0x2611;  // checked
         else
            glyph[0] = 0x2610;  // unchecked
         glyph[1] = L'\0';

         LONG pos = MdGetCharCount(s->hwnd);
         Edit_SetSel(s->hwnd, pos, pos);

         CHARFORMAT2 taskCf;
         MdBuildCharFormat(s, &taskCf);
         SendMessage(s->hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&taskCf);

         SETTEXTEX ste;
         ste.flags = ST_SELECTION;
         ste.codepage = 1200;  // UTF-16
         SendMessage(s->hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)glyph);

         /* Non-breaking space prevents Rich Edit from collapsing the
          * gap between the checkbox glyph and the following text. */
         MdAppendText(s, "\xA0", 1);
      }
      else if (s->orderedListCounter > 0)
      {
         char marker[16];
         sprintf(marker, "%d. ", s->orderedListCounter);
         MdAppendText(s, marker, (int)strlen(marker));
         s->orderedListCounter++;
      }
      else
      {
         MdAppendText(s, "\x95 ", 2);  // 0x95 = bullet (U+2022) in Windows-1252
      }
      s->firstParagraphInLI = true;
      s->firstBlockDone = true;
      break;
   }

   case MD_BLOCK_QUOTE:
      // Container only: P blocks inside handle paragraph spacing/indent.
      s->quoteDepth++;
      break;

   case MD_BLOCK_HR:
   {
      int spaceBefore = s->firstBlockDone ? 80 : 0;
      MdBeginParagraph(s, spaceBefore, 80, 0, 0);

      /* Draw a horizontal line that spans the control width.
       * Measure how many U+2500 (BOX DRAWINGS LIGHT HORIZONTAL) glyphs
       * fit in the formatting rectangle, then insert them via EM_SETTEXTEX.
       * EM_GETRECT returns the text area excluding margins, borders, and
       * scrollbar -- this is the actual width available for text before
       * Rich Edit word-wraps to the next line. */
      RECT fmtRect;
      SendMessage(s->hwnd, EM_GETRECT, 0, (LPARAM)&fmtRect);
      int textWidth = fmtRect.right - fmtRect.left;

      /* Measure glyph width using the font that MdBuildCharFormat will
       * actually apply (Times New Roman at base twips size), NOT the
       * control's WM_GETFONT font which is the dialog font. */
      CHARFORMAT2 hrCf;
      MdBuildCharFormat(s, &hrCf);
      hrCf.dwEffects &= ~(CFE_BOLD | CFE_ITALIC | CFE_UNDERLINE);

      HDC hdc = GetDC(s->hwnd);
      LOGFONT lf;
      memset(&lf, 0, sizeof(lf));
      strcpy(lf.lfFaceName, hrCf.szFaceName);
      // Convert twips to logical units: yHeight is in twips (1/1440 inch).
      int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
      lf.lfHeight = -MulDiv(hrCf.yHeight, dpi, 1440);
      HFONT hMeasureFont = CreateFontIndirect(&lf);
      HFONT hOldFont = (HFONT)SelectObject(hdc, hMeasureFont);
      SIZE charSize;
      WCHAR measure = 0x2500;
      GetTextExtentPoint32W(hdc, &measure, 1, &charSize);
      SelectObject(hdc, hOldFont);
      DeleteObject(hMeasureFont);
      ReleaseDC(s->hwnd, hdc);

      int charWidth = charSize.cx > 0 ? charSize.cx : 6;
      /* Subtract 2 characters as a safety margin -- Rich Edit's internal
       * wrapping logic can differ slightly from GetTextExtentPoint32W
       * measurements due to rounding, inter-character spacing, and DPI
       * scaling.  A margin of 1 was not always sufficient. */
      int numChars = (textWidth / charWidth) - 2;
      if (numChars < 10) numChars = 10;
      if (numChars > 200) numChars = 200;

      // Build a buffer of U+2500 glyphs.
      WCHAR hrBuf[201];
      for (int i = 0; i < numChars; i++)
         hrBuf[i] = 0x2500;
      hrBuf[numChars] = L'\0';

      LONG pos = MdGetCharCount(s->hwnd);
      Edit_SetSel(s->hwnd, pos, pos);
      SendMessage(s->hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&hrCf);

      SETTEXTEX ste;
      ste.flags = ST_SELECTION;
      ste.codepage = 1200;  // UTF-16
      SendMessage(s->hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)hrBuf);

      s->firstBlockDone = true;
      break;
   }

   case MD_BLOCK_TABLE:
   {
      int spaceBefore = s->firstBlockDone ? 120 : 0;
      MdBeginParagraph(s, spaceBefore, 0, 0, 0);
      s->tableData = (struct MdTableData *)calloc(1, sizeof(struct MdTableData));
      if (s->tableData)
      {
         s->tableData->currentRow = -1;
         s->tableData->currentCol = -1;
      }
      s->firstBlockDone = true;
      break;
   }

   case MD_BLOCK_THEAD:
      break;

   case MD_BLOCK_TBODY:
      break;

   case MD_BLOCK_TR:
      if (s->tableData && s->tableData->numRows < MD_MAX_TABLE_ROWS)
      {
         s->tableData->currentRow = s->tableData->numRows;
         s->tableData->currentCol = -1;
         s->tableData->numRows++;
      }
      break;

   case MD_BLOCK_TH:
      if (s->tableData)
      {
         int row = s->tableData->currentRow;
         int col = s->tableData->currentCol + 1;
         if (row >= 0 && col < MD_MAX_TABLE_COLS)
         {
            s->tableData->currentCol = col;
            s->tableData->cellLens[row][col] = 0;
            s->tableData->cells[row][col][0] = '\0';
            s->tableData->rowIsHeader[row] = true;
            s->tableData->inCell = true;
            if (col + 1 > s->tableData->numCols)
               s->tableData->numCols = col + 1;
         }
      }
      break;

   case MD_BLOCK_TD:
      if (s->tableData)
      {
         int row = s->tableData->currentRow;
         int col = s->tableData->currentCol + 1;
         if (row >= 0 && col < MD_MAX_TABLE_COLS)
         {
            s->tableData->currentCol = col;
            s->tableData->cellLens[row][col] = 0;
            s->tableData->cells[row][col][0] = '\0';
            s->tableData->inCell = true;
            if (col + 1 > s->tableData->numCols)
               s->tableData->numCols = col + 1;
         }
      }
      break;

   default:
      break;
   }

   return 0;
}

/************************************************************************/
/*
 * MdLeaveBlock:  Returns 0 on success. md4c callback for leaving a
 *   block element.
 */
static int MdLeaveBlock(MD_BLOCKTYPE type, void *detail, void *userdata)
{
   struct MdRenderState *s = (struct MdRenderState *)userdata;

   switch (type)
   {
   case MD_BLOCK_H:
      s->headingLevel = 0;
      break;

   case MD_BLOCK_CODE:
      s->inCodeBlock = false;
      break;

   case MD_BLOCK_UL:
      s->listDepth--;
      s->orderedListCounter = 0;
      break;

   case MD_BLOCK_OL:
      s->listDepth--;
      s->orderedListCounter = 0;
      break;

   case MD_BLOCK_LI:
      s->firstParagraphInLI = false;
      break;

   case MD_BLOCK_QUOTE:
      if (s->quoteDepth > 0)
         s->quoteDepth--;
      break;

   case MD_BLOCK_TABLE:
      // Output the buffered table with code font and space-padded columns.
      if (s->tableData)
      {
         struct MdTableData *t = s->tableData;
         s->inTable = true;

         /* Calculate max column widths (visual width after checkbox
          * substitution: [x]/[ ] each display as a single glyph). */
         int colWidths[8] = {0};
         for (int r = 0; r < t->numRows; r++)
         {
            for (int c = 0; c < t->numCols; c++)
            {
               int vw = MdCellVisualWidth(t->cells[r][c], t->cellLens[r][c]);
               if (vw > colWidths[c])
                  colWidths[c] = vw;
            }
         }

         for (int r = 0; r < t->numRows; r++)
         {
            if (r > 0)
            {
               MdAppendNewline(s);
               MdResetParaFormat(s);
            }

            // Bold for header rows.
            if (t->rowIsHeader[r])
               s->mdBold++;

            for (int c = 0; c < t->numCols; c++)
            {
               // U+2502 BOX DRAWINGS LIGHT VERTICAL as column separator.
               if (c > 0)
                  MdAppendUnicode(s, 0x2502);

               /* Output cell text with checkbox substitution, then pad.
                * Skip padding on the last column to save horizontal space. */
               MdOutputCellText(s, t->cells[r][c], t->cellLens[r][c]);
               if (c < t->numCols - 1)
               {
                  int vw = MdCellVisualWidth(t->cells[r][c], t->cellLens[r][c]);
                  int padding = colWidths[c] - vw;
                  for (int p = 0; p < padding; p++)
                     MdAppendText(s, " ", 1);
               }
            }

            if (t->rowIsHeader[r])
            {
               if (s->mdBold > 0) s->mdBold--;

               /* Draw separator line under header using box-drawing chars.
                * U+2500 LIGHT HORIZONTAL for dashes, U+253C LIGHT VERTICAL
                * AND HORIZONTAL for junctions -- these connect. */
               MdAppendNewline(s);
               MdResetParaFormat(s);
               for (int c = 0; c < t->numCols; c++)
               {
                  if (c > 0)
                     MdAppendUnicode(s, 0x253C);
                  int w = colWidths[c];
                  if (w > 200) w = 200;
                  WCHAR hrBuf[201];
                  for (int i = 0; i < w; i++)
                     hrBuf[i] = 0x2500;
                  hrBuf[w] = L'\0';

                  LONG pos = MdGetCharCount(s->hwnd);
                  Edit_SetSel(s->hwnd, pos, pos);
                  CHARFORMAT2 cf;
                  MdBuildCharFormat(s, &cf);
                  SendMessage(s->hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
                  SETTEXTEX ste;
                  ste.flags = ST_SELECTION;
                  ste.codepage = 1200;
                  SendMessage(s->hwnd, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)hrBuf);
               }
            }
         }

         s->inTable = false;
         free(s->tableData);
         s->tableData = NULL;
      }
      break;

   case MD_BLOCK_THEAD:
   case MD_BLOCK_TBODY:
      break;

   case MD_BLOCK_TR:
      break;

   case MD_BLOCK_TH:
   case MD_BLOCK_TD:
      if (s->tableData)
         s->tableData->inCell = false;
      break;

   default:
      break;
   }

   return 0;
}

// Forward declaration: chat-path output (defined later, needed by MdEnterSpan).
static void MdChatOutput(struct MdRenderState *s, const char *text, int len);

/************************************************************************/
/*
 * MdEnterSpan:  Returns 0 on success. md4c callback for entering an
 *   inline span.
 */
static int MdEnterSpan(MD_SPANTYPE type, void *detail, void *userdata)
{
   struct MdRenderState *s = (struct MdRenderState *)userdata;

   switch (type)
   {
   case MD_SPAN_EM:
      s->mdItalic++;
      break;
   case MD_SPAN_STRONG:
      s->mdBold++;
      break;
   case MD_SPAN_U:
      s->mdUnderline++;
      break;
   case MD_SPAN_DEL:
      s->mdStrikethrough++;
      break;
   case MD_SPAN_CODE:
      /* Inline code spans are not supported -- backtick is the legacy
       * format code prefix. Output a literal backtick so the delimiter
       * is visible as plain text. */
      if (s->isChatPath)
         MdChatOutput(s, "`", 1);
      else
         MdAppendText(s, "`", 1);
      break;
   case MD_SPAN_A:
   {
      /* Store link URL and record start position for clickable link.
       * If links are disabled, just render the text without link styling. */
      if (!s->linksEnabled)
         break;
      MD_SPAN_A_DETAIL *a = (MD_SPAN_A_DETAIL *)detail;
      if (a && a->href.text && a->href.size > 0)
      {
         int copyLen = (a->href.size < MD_MAX_URL_LEN - 1) ? (int)a->href.size : MD_MAX_URL_LEN - 1;
         memcpy(s->linkUrl, a->href.text, copyLen);
         s->linkUrl[copyLen] = '\0';
         s->linkUrlLen = copyLen;
      }
      else
      {
         s->linkUrl[0] = '\0';
         s->linkUrlLen = 0;
      }
      s->linkCharStart = MdGetCharCount(s->hwnd);
      s->mdUnderline++;
      s->inLink = true;
      break;
   }
   default:
      break;
   }

   return 0;
}

/************************************************************************/
/*
 * MdLeaveSpan:  Returns 0 on success. md4c callback for leaving an
 *   inline span.
 */
static int MdLeaveSpan(MD_SPANTYPE type, void *detail, void *userdata)
{
   struct MdRenderState *s = (struct MdRenderState *)userdata;

   switch (type)
   {
   case MD_SPAN_EM:
      if (s->mdItalic > 0) s->mdItalic--;
      break;
   case MD_SPAN_STRONG:
      if (s->mdBold > 0) s->mdBold--;
      break;
   case MD_SPAN_U:
      if (s->mdUnderline > 0) s->mdUnderline--;
      break;
   case MD_SPAN_DEL:
      if (s->mdStrikethrough > 0) s->mdStrikethrough--;
      break;
   case MD_SPAN_CODE:
      if (s->isChatPath)
         MdChatOutput(s, "`", 1);
      else
         MdAppendText(s, "`", 1);
      break;
   case MD_SPAN_A:
   {
      if (!s->linksEnabled)
         break;
      if (s->mdUnderline > 0) s->mdUnderline--;
      s->inLink = false;
      /* Apply underline and link color to the link text range and store
       * the URL. We intentionally do NOT use CFE_LINK because Rich Edit
       * overrides crTextColor at paint time when CFE_LINK is set, forcing
       * the system hyperlink color (blue) regardless of any color we set.
       * Instead we style links with underline + COLOR_LINKFGD and handle
       * click/hover detection ourselves via the link registry and a
       * window subclass (see MdLinkSubclassProc). */
      LONG linkEnd = MdGetCharCount(s->hwnd);
      if (s->linkUrlLen > 0 && s->hwnd && linkEnd > s->linkCharStart)
      {
         Edit_SetSel(s->hwnd, s->linkCharStart, linkEnd);

         CHARFORMAT2 linkCf;
         memset(&linkCf, 0, sizeof(linkCf));
         linkCf.cbSize = sizeof(linkCf);
         linkCf.dwMask = CFM_COLOR | CFM_UNDERLINE;
         linkCf.dwEffects = CFE_UNDERLINE;
         linkCf.crTextColor = GetColor(COLOR_LINKFGD);
         SendMessage(s->hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&linkCf);

         Edit_SetSel(s->hwnd, linkEnd, linkEnd);

         MdStoreLink(s->hwnd, s->linkCharStart, linkEnd,
                     s->linkUrl, s->linkUrlLen);
      }
      s->linkUrl[0] = '\0';
      s->linkUrlLen = 0;
      break;
   }
   default:
      break;
   }

   return 0;
}

/************************************************************************/
/*
 * MdText:  Returns 0 on success. md4c callback for text content.
 */
static int MdText(MD_TEXTTYPE type, const MD_CHAR *text, MD_SIZE size,
                  void *userdata)
{
   struct MdRenderState *s = (struct MdRenderState *)userdata;

   switch (type)
   {
   case MD_TEXT_NORMAL:
      /* Suppress non-breaking space filler from MdPreserveBlankLines.
       * These are placeholder paragraphs for extra blank lines -- the
       * paragraph spacing from MdEnterBlock(P) handles the visual gap. */
      if (size == 1 && (unsigned char)text[0] == 0xA0)
         break;

      // Buffer text if inside a table cell.
      if (s->tableData && s->tableData->inCell)
      {
         int row = s->tableData->currentRow;
         int col = s->tableData->currentCol;
         if (row >= 0 && col >= 0 && row < MD_MAX_TABLE_ROWS && col < MD_MAX_TABLE_COLS)
         {
            int curLen = s->tableData->cellLens[row][col];
            int space = MD_MAX_CELL_TEXT - 1 - curLen;
            int copyLen = ((int)size < space) ? (int)size : space;
            if (copyLen > 0)
            {
               memcpy(s->tableData->cells[row][col] + curLen, text, copyLen);
               s->tableData->cellLens[row][col] = curLen + copyLen;
               s->tableData->cells[row][col][curLen + copyLen] = '\0';
            }
         }
         break;
      }

      // In code blocks, split on newlines to reset paragraph spacing.
      if (s->inCodeBlock)
         MdAppendCodeText(s, text, size, 0);
      else
         MdProcessLegacyText(s, text, size);
      break;

   case MD_TEXT_CODE:
      /* For code blocks, split on newlines to reset paragraph spacing.
       * For inline code, md4c delivers text as MD_TEXT_CODE but we treat
       * it as normal text (inline code spans are disabled -- see
       * MD_SPAN_CODE in MdEnterSpan). */
      if (s->inCodeBlock)
         MdAppendCodeText(s, text, size, 0);
      else
         MdProcessLegacyText(s, text, size);
      break;

   case MD_TEXT_BR:
      MdAppendNewline(s);
      // Reset spacing so the new line doesn't inherit paragraph gap.
      {
         PARAFORMAT2 pf;
         memset(&pf, 0, sizeof(pf));
         pf.cbSize = sizeof(pf);
         pf.dwMask = PFM_SPACEBEFORE | PFM_SPACEAFTER;
         SendMessage(s->hwnd, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
      }
      // Re-insert blockquote bar on the new line.
      MdInsertQuoteBar(s);
      break;

   case MD_TEXT_SOFTBR:
      /* Preserve single newlines as visible line breaks for backward
       * compatibility with existing player descriptions. */
      MdAppendNewline(s);
      // Reset spacing so the new line doesn't inherit paragraph gap.
      {
         PARAFORMAT2 pf;
         memset(&pf, 0, sizeof(pf));
         pf.cbSize = sizeof(pf);
         pf.dwMask = PFM_SPACEBEFORE | PFM_SPACEAFTER;
         SendMessage(s->hwnd, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
      }
      // Re-insert blockquote bar on the new line.
      MdInsertQuoteBar(s);
      break;

   case MD_TEXT_ENTITY:
      /* Just output the entity text verbatim; Rich Edit doesn't handle HTML
       * entities, and we're not converting to HTML. */
      MdAppendText(s, text, size);
      break;

   case MD_TEXT_NULLCHAR:
      break;

   default:
      MdAppendText(s, text, size);
      break;
   }

   return 0;
}

/************************************************************************/
/*
 * MdChatOutput:  Chat-path output: appends text to the chat edit box via
 *   EditBoxAddText, combining markdown span state with legacy format state
 *   into the style bitmask expected by EditBoxAddText.
 */
static void MdChatOutput(struct MdRenderState *s, const char *text, int len)
{
   if (len <= 0)
      return;

   char *buf = (char *)malloc(len + 1);
   if (!buf)
      return;
   memcpy(buf, text, len);
   buf[len] = '\0';

   // Combine markdown spans with legacy style bitmask.
   BYTE style = s->legacyStyle;
   if (s->mdBold > 0)
      style |= STYLE_BOLD;
   if (s->mdItalic > 0)
      style |= STYLE_ITALIC;
   if (s->mdUnderline > 0)
      style |= STYLE_UNDERLINE;

   COLORREF color = s->legacyColor;

   EditBoxAddText(buf, color, style);
   free(buf);
}

/************************************************************************/
/*
 * MdChatProcessLegacyText:  Chat-path legacy text processor: walks text
 *   for format codes and outputs segments via MdChatOutput.
 */
static void MdChatProcessLegacyText(struct MdRenderState *s,
                                    const char *text, int len)
{
   const char *str = text;
   const char *end = text + len;

   for (const char *ptr = text; ptr < end; ptr++)
   {
      if (!IsSentinel(*ptr))
         continue;

      int idx = SentinelIndex(*ptr);
      const struct LegacyCode *lc = &legacy_codes[idx];

      int segLen = (int)(ptr - str);
      if (segLen > 0)
         MdChatOutput(s, str, segLen);

      if (lc->type == LEGACY_COLOR)
         s->legacyColor = lc->data;
      else if (lc->type == LEGACY_STYLE)
      {
         switch (lc->data)
         {
         case STYLE_NORMAL:
            s->legacyStyle = STYLE_NORMAL;
            break;
         case STYLE_RESET:
            s->legacyStyle = STYLE_NORMAL;
            s->legacyColor = s->defaultColor;
            break;
         default:
            s->legacyStyle = s->legacyStyle ^ lc->data;
            break;
         }
      }

      str = ptr + 1;
   }

   int remaining = (int)(end - str);
   if (remaining > 0)
      MdChatOutput(s, str, remaining);
}

/************************************************************************/
/*
 * MdChatBlockCb:  Returns 0. Block enter/leave are no-ops for inline chat.
 */
static int MdChatBlockCb(MD_BLOCKTYPE type, void *detail, void *userdata)
{
   return 0;
}

/************************************************************************/
/*
 * MdChatTextCb:  Returns 0 on success. Processes legacy format codes in
 *   chat text and outputs via EditBoxAddText.
 */
static int MdChatTextCb(MD_TEXTTYPE type, const MD_CHAR *text, MD_SIZE size,
                        void *userdata)
{
   struct MdRenderState *s = (struct MdRenderState *)userdata;

   switch (type)
   {
   case MD_TEXT_NORMAL:
      MdChatProcessLegacyText(s, text, size);
      break;

   case MD_TEXT_CODE:
      /* Inline code spans are disabled (see MD_SPAN_CODE in MdEnterSpan),
       * but md4c still delivers text as MD_TEXT_CODE. Process it normally. */
      MdChatProcessLegacyText(s, text, size);
      break;

   case MD_TEXT_SOFTBR:
      MdChatOutput(s, " ", 1);
      break;

   case MD_TEXT_BR:
      MdChatOutput(s, " ", 1);
      break;

   case MD_TEXT_ENTITY:
      MdChatOutput(s, text, size);
      break;

   case MD_TEXT_NULLCHAR:
      break;

   default:
      MdChatOutput(s, text, size);
      break;
   }

   return 0;
}

/************************************************************************/
/*
 * DisplayMessageMarkdown:  Displays a chat/broadcast message with inline
 *   markdown support (bold, italic, underline, strikethrough). Parses
 *   the message with md4c using inline-only flags, processes legacy format
 *   codes within text segments, and outputs via EditBoxAddText. Applies
 *   the profanity filter before parsing.
 */
void DisplayMessageMarkdown(const char *message, COLORREF start_color,
                            BYTE start_style)
{
   bool bFree = false;

   char *new_message = strdup(message);
   char *p = new_message;

   if (config.antiprofane)
   {
      if (config.ignoreprofane)
      {
         if (ContainsProfaneTerms(new_message))
         {
            new_message = (char *)SafeMalloc(MAXSTRINGLEN);
            LoadString(hInst, IDS_PROFANITYREMOVED, new_message, MAXSTRINGLEN);
            bFree = true;
         }
      }
      else
      {
         new_message = CleanseProfaneString(new_message);
         bFree = true;
      }
   }

   /* Normalize legacy format prefixes (backtick and tilde) to single-byte
    * sentinels so md4c doesn't misinterpret them as inline code or
    * strikethrough markup. In-place since sentinels are shorter (1 byte)
    * than the original 2-byte sequences. */
   {
      char prevSrcChar = '\0';
      char *dst = new_message;
      for (const char *src = new_message; *src != '\0'; src++)
      {
         if ((*src == '`' || *src == '~') && *(src + 1) != '\0')
         {
            /* Don't consume ~X when preceded by ~ (preserves ~~ pairs).
             * Reset prevSrcChar after guard so ~~~n = ~~ + ~n works. */
            if (*src == '~' && prevSrcChar == '~')
            {
               prevSrcChar = '\0';
               *dst++ = *src;
               continue;
            }

            const char *found = strchr(legacy_code_chars, *(src + 1));
            if (found)
            {
               prevSrcChar = *(src + 1);
               *dst++ = (char)(SENTINEL_FIRST + (int)(found - legacy_code_chars));
               src++;   // skip code char
               continue;
            }
         }
         prevSrcChar = *src;
         *dst++ = *src;
      }
      *dst = '\0';
   }

   struct MdRenderState state;
   memset(&state, 0, sizeof(state));
   state.hwnd = EditBoxWindow();
   state.defaultColor = start_color;
   state.legacyColor = start_color;
   state.legacyStyle = start_style;
   state.isChatPath = true;

   MD_PARSER parser;
   memset(&parser, 0, sizeof(parser));
   parser.abi_version = 0;
   parser.flags = MD_FLAG_NOHTML |
                  MD_FLAG_NOINDENTEDCODEBLOCKS |
                  MD_FLAG_COLLAPSEWHITESPACE |
                  MD_FLAG_UNDERLINE;
   parser.enter_block = MdChatBlockCb;
   parser.leave_block = MdChatBlockCb;
   parser.enter_span = MdEnterSpan;
   parser.leave_span = MdLeaveSpan;
   parser.text = MdChatTextCb;

   md_parse(new_message, (MD_SIZE)strlen(new_message), &parser, &state);

   if (bFree)
      SafeFree(new_message);

   SafeFree(p);
}

/************************************************************************/
/*
 * RichEditSetMarkdownText:  Renders markdown text into a Rich Edit control.
 *   Parses both markdown syntax and legacy format codes.
 *   The flags parameter controls which features are enabled
 *   (MD_RENDER_FULL, MD_RENDER_INLINE, MD_RENDER_LINKS, MD_RENDER_APPEND).
 */
void RichEditSetMarkdownText(HWND hwndRichEdit, const char *text,
                             COLORREF defaultColor, int flags)
{
   if (!text || !hwndRichEdit)
      return;

   // Clear existing content unless appending.
   if (!(flags & MD_RENDER_APPEND))
   {
      SetWindowText(hwndRichEdit, "");
      MdClearLinksForHwnd(hwndRichEdit);
   }

   // Only install link-handler subclass when links are enabled.
   if (flags & MD_RENDER_LINKS)
      MdInstallLinkHandler(hwndRichEdit);

   struct MdRenderState state;
   memset(&state, 0, sizeof(state));
   state.hwnd = hwndRichEdit;
   state.defaultColor = defaultColor;
   state.legacyColor = defaultColor;
   state.legacyStyle = STYLE_NORMAL;

   /* When appending, the control already has content, so the first
    * block should insert a leading newline to separate from it. */
   if (flags & MD_RENDER_APPEND)
      state.firstBlockDone = true;

   // Configure md4c parser flags.
   unsigned parserFlags = MD_FLAG_NOHTML |
                          MD_FLAG_NOINDENTEDCODEBLOCKS |
                          MD_FLAG_STRIKETHROUGH |
                          MD_FLAG_UNDERLINE |
                          MD_FLAG_PERMISSIVEATXHEADERS;

   /* Only enable autolink detection and link handling when explicitly
    * opted in.  Player-editable content (descriptions, mail, news, books)
    * should NOT have clickable links to prevent phishing and abuse.
    * Email autolinks are excluded -- only URL and www forms are detected. */
   bool enableLinks = (flags & MD_RENDER_LINKS) != 0;
   if (enableLinks)
      parserFlags |= MD_FLAG_PERMISSIVEURLAUTOLINKS | MD_FLAG_PERMISSIVEWWWAUTOLINKS;

   state.linksEnabled = enableLinks;

   if ((flags & MD_RENDER_INLINE) != 0)
   {
      /* For inline-only mode, we still use md4c but configure the parser
       * to produce minimal block structure. The enter_block/leave_block
       * callbacks handle this gracefully since paragraphs just become
       * inline text without extra spacing. */
      parserFlags |= MD_FLAG_COLLAPSEWHITESPACE;
   }
   else
   {
      // Full mode: enable tables and task lists.
      parserFlags |= MD_FLAG_TABLES | MD_FLAG_TASKLISTS;
   }

   MD_PARSER parser;
   memset(&parser, 0, sizeof(parser));
   parser.abi_version = 0;
   parser.flags = parserFlags;
   parser.enter_block = MdEnterBlock;
   parser.leave_block = MdLeaveBlock;
   parser.enter_span = MdEnterSpan;
   parser.leave_span = MdLeaveSpan;
   parser.text = MdText;

   /* Replace legacy format prefixes (backtick and tilde) with single-byte
    * sentinels so md4c doesn't consume them as markdown syntax. */
   char *normalized = MdNormalizeLegacyCodes(text);

   /* Move legacy sentinels that appear between a list marker and a task
    * checkbox to after the checkbox so md4c can detect the task list. */
   MdFixTaskListCodes(normalized);

   // Preserve extra blank lines that md4c would otherwise collapse.
   char *preserved = MdPreserveBlankLines(normalized);
   free(normalized);

   md_parse(preserved, (MD_SIZE)strlen(preserved), &parser, &state);
   free(preserved);
}

/************************************************************************/
/*
 * MdIsUrlSafe:  Returns true if the URL is safe to open in a browser.
 *   Allows http://, https://, and bare domain names. Blocks file://,
 *   ftp://, javascript:, data:, local paths (C:\, \\UNC), and any
 *   other non-http scheme.
 */
static bool MdIsUrlSafe(const char *url)
{
   if (!url || url[0] == '\0')
      return false;

   // Explicitly allowed schemes.
   if (_strnicmp(url, "http://", 7) == 0 ||
       _strnicmp(url, "https://", 8) == 0)
      return true;

   /* Reject anything with a non-http scheme (colon appears before the
    * first slash or dot, indicating file:, ftp:, javascript:, etc). */
   const char *colon = strchr(url, ':');
   if (colon)
   {
      const char *slash = strchr(url, '/');
      if (!slash || colon < slash)
         return false;
   }

   // Reject local paths.
   if (url[0] == '\\' || (strlen(url) > 2 && url[1] == ':' && url[2] == '\\'))
      return false;

   /* Bare domain (e.g. "google.com") -- must contain a dot to be
    * a plausible domain.  A bare word like "blah" is not valid. */
   if (strchr(url, '.') != NULL)
      return true;

   return false;
}

/************************************************************************/
/*
 * MdShowLinkTooltip:  Shows the target URL as a tooltip when hovering
 *   over a link. Uses a static HWND so only one tooltip exists at a time.
 */
static HWND s_linkTooltip = NULL;
static char s_lastTooltipUrl[768] = "";

static void MdShowLinkTooltip(HWND hwndRichEdit, const char *url, int x, int y)
{
   if (!url || url[0] == '\0')
      return;

   // Show the URL as-is for safe links, or prefixed with [blocked] if unsafe.
   char displayUrl[768];
   if (MdIsUrlSafe(url))
   {
      strncpy(displayUrl, url, sizeof(displayUrl) - 1);
      displayUrl[sizeof(displayUrl) - 1] = '\0';
   }
   else
      snprintf(displayUrl, sizeof(displayUrl), "[blocked] %s", url);

   // Don't recreate if showing the same URL.
   if (s_linkTooltip && strcmp(s_lastTooltipUrl, displayUrl) == 0)
      return;

   strncpy(s_lastTooltipUrl, displayUrl, sizeof(s_lastTooltipUrl) - 1);
   s_lastTooltipUrl[sizeof(s_lastTooltipUrl) - 1] = '\0';

   if (!s_linkTooltip)
   {
      s_linkTooltip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
         WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
         CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
         hwndRichEdit, NULL, hInst, NULL);
      if (!s_linkTooltip)
         return;

      /* TTF_TRACK: manually positioned tooltip, not managed by mouse hover.
       * Use TTTOOLINFOA_V1_SIZE (44 bytes) because the client has no
       * comctl32 v6 manifest, so the tooltip control is v5 and rejects
       * the larger v6 TOOLINFO struct (48 bytes). */
      TOOLINFO ti;
      memset(&ti, 0, sizeof(ti));
      ti.cbSize = TTTOOLINFOA_V1_SIZE;
      ti.uFlags = TTF_TRACK | TTF_ABSOLUTE;
      ti.hwnd = hwndRichEdit;
      ti.uId = 0;
      ti.lpszText = s_lastTooltipUrl;
      SendMessage(s_linkTooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);
      SendMessage(s_linkTooltip, TTM_SETMAXTIPWIDTH, 0, 500);
   }

   TOOLINFO ti;
   memset(&ti, 0, sizeof(ti));
   ti.cbSize = TTTOOLINFOA_V1_SIZE;
   ti.uFlags = TTF_TRACK | TTF_ABSOLUTE;
   ti.hwnd = hwndRichEdit;
   ti.uId = 0;
   ti.lpszText = s_lastTooltipUrl;
   SendMessage(s_linkTooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
   POINT pt = { x, y };
   ClientToScreen(hwndRichEdit, &pt);
   SendMessage(s_linkTooltip, TTM_TRACKPOSITION, 0, MAKELPARAM(pt.x, pt.y + 20));

   TOOLINFO tiActivate;
   memset(&tiActivate, 0, sizeof(tiActivate));
   tiActivate.cbSize = TTTOOLINFOA_V1_SIZE;
   tiActivate.hwnd = hwndRichEdit;
   tiActivate.uId = 0;
   SendMessage(s_linkTooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&tiActivate);
}

/************************************************************************/
/*
 * MdHideLinkTooltip:  Destroys the link tooltip window and resets the
 *   static tooltip state.
 */
static void MdHideLinkTooltip(void)
{
   if (s_linkTooltip)
   {
      DestroyWindow(s_linkTooltip);
      s_linkTooltip = NULL;
      s_lastTooltipUrl[0] = '\0';
   }
}

/************************************************************************/
/*
 * MdLinkFromPoint:  Returns a pointer to the MdLink under the given
 *   client-area point, or NULL if no link is at that position. Uses
 *   EM_CHARFROMPOS to map pixel coordinates to a character index, then
 *   searches the link registry.
 */
static struct MdLink *MdLinkFromPoint(HWND hwndRichEdit, int x, int y)
{
   POINTL pt = { x, y };
   LONG charIndex = (LONG)SendMessage(hwndRichEdit, EM_CHARFROMPOS, 0, (LPARAM)&pt);
   if (charIndex < 0)
      return NULL;

   for (int i = 0; i < s_numLinks; i++)
   {
      struct MdLink *link = &s_linkRegistry[i];
      if (link->hwnd == hwndRichEdit &&
          charIndex >= link->charStart &&
          charIndex < link->charEnd &&
          link->url[0] != '\0')
      {
         return link;
      }
   }
   return NULL;
}

/************************************************************************/
/*
 * Subclass ID for the link-handler subclass installed on Rich Edit controls.
 */
static const UINT_PTR MD_LINK_SUBCLASS_ID = 1001;

/************************************************************************/
/*
 * MdLinkSubclassProc:  Returns LRESULT for the subclass proc. Handles
 *   WM_LBUTTONUP (open link), WM_MOUSEMOVE (show tooltip), and
 *   WM_SETCURSOR (hand cursor over links) for any Rich Edit control
 *   that has markdown links.
 */
static LRESULT CALLBACK MdLinkSubclassProc(HWND hwnd, UINT msg,
   WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
   switch (msg)
   {
   case WM_LBUTTONUP:
   {
      int x = GET_X_LPARAM(lParam);
      int y = GET_Y_LPARAM(lParam);
      struct MdLink *link = MdLinkFromPoint(hwnd, x, y);
      if (link)
      {
         MdHideLinkTooltip();
         if (MdIsUrlSafe(link->url))
            WebLaunchBrowser(link->url);
         // Let Rich Edit also process the button-up for internal state.
      }
      break;
   }

   case WM_MOUSEMOVE:
   {
      int x = GET_X_LPARAM(lParam);
      int y = GET_Y_LPARAM(lParam);
      struct MdLink *link = MdLinkFromPoint(hwnd, x, y);
      if (link)
         MdShowLinkTooltip(hwnd, link->url, x, y);
      else
         MdHideLinkTooltip();
      break;
   }

   case WM_SETCURSOR:
   {
      if (LOWORD(lParam) == HTCLIENT)
      {
         POINT pt;
         GetCursorPos(&pt);
         ScreenToClient(hwnd, &pt);
         struct MdLink *link = MdLinkFromPoint(hwnd, pt.x, pt.y);
         if (link)
         {
            SetCursor(LoadCursor(NULL, IDC_HAND));
            return TRUE;
         }
      }
      break;
   }

   case WM_NCDESTROY:
      RemoveWindowSubclass(hwnd, MdLinkSubclassProc, uIdSubclass);
      break;
   }

   return DefSubclassProc(hwnd, msg, wParam, lParam);
}

/************************************************************************/
/*
 * MdInstallLinkHandler:  Installs the link-handler subclass on a Rich
 *   Edit control. Safe to call multiple times -- SetWindowSubclass with
 *   the same callback+ID just updates the reference data without adding
 *   a duplicate. Call this once after creating a Rich Edit control that
 *   will display markdown content.
 */
void MdInstallLinkHandler(HWND hwndRichEdit)
{
   if (!hwndRichEdit)
      return;
   SetWindowSubclass(hwndRichEdit, MdLinkSubclassProc,
                     MD_LINK_SUBCLASS_ID, 0);
}
