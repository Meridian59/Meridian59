# Client Text Formatting

This document covers the Meridian 59 client's text rendering system, including legacy format codes, markdown support, configurable colors, and configurable fonts.

## Rendering Contexts

Text is rendered differently depending on where it appears in the client. There are two rendering paths:

### Chat Path (Inline Only)

Used for server messages displayed in the main chat/text area. Entered via `DisplayServerMessage` -> `DisplayMessageMarkdown`. Supports:

- Legacy format codes (~B, ~I, ~U, ~n, color codes)
- Inline markdown (bold, italic, underline, strikethrough)

Does NOT support block-level markdown (headings, lists, code blocks, tables, blockquotes, horizontal rules). Autolinks and clickable links are disabled in chat -- URLs appear as plain text. Inline code spans (backtick-delimited) are not supported because backtick is reserved as the legacy format code prefix.

### Description Path (Full Markdown)

Used for item/NPC/sign descriptions, mail, news, and MOTD dialogs. Entered via `RichEditSetMarkdownText` with `MD_RENDER_FULL`. Supports everything:

- Legacy format codes (~B, ~I, ~U, ~n, color codes)
- All inline markdown (bold, italic, underline, strikethrough)
- All block-level markdown (headings, lists, code blocks, tables, blockquotes, horizontal rules)
- Task lists with checkbox glyphs
- Link syntax `[text](url)` renders as styled text but is NOT clickable by default

Clickable links (with URL tooltips, hover cursor, and click-to-open) are only enabled when the `MD_RENDER_LINKS` flag is passed. Currently only the MOTD (Message of the Day) uses this flag, since MOTD content is server-controlled and trusted. Player-editable contexts (descriptions, mail, news, books) never have clickable links to prevent phishing and abuse in the MMORPG.

---

## Legacy Format Codes

Legacy format codes use a tilde `~` or backtick `` ` `` prefix followed by a single character. Both prefixes are equivalent -- they are internally converted to single-byte sentinel characters (range 0x0E-0x1B, one unique byte per code) before parsing to avoid conflicts with markdown syntax. md4c is patched to classify these sentinel bytes as punctuation so they do not interfere with emphasis delimiter flanking rules. Inline code spans (`` `code` ``) are intentionally not supported because backtick has been the legacy format code prefix for decades and overloading it creates an unresolvable conflict.

### Color Codes

| Code | Color | RGB |
|------|-------|-----|
| `~r` | Maroon Red | 128, 0, 0 |
| `~g` | Dark Green | 0, 100, 0 |
| `~l` | Lime Green | 0, 255, 0 |
| `~b` | Blue | 0, 0, 255 |
| `~c` | Cyan | 0, 255, 255 |
| `~k` | Black | 0, 0, 0 |
| `~w` | White | 255, 255, 255 |
| `~y` | Yellow | 255, 255, 0 |
| `~o` | Orange | 255, 172, 28 |
| `~p` | Pink | 255, 192, 203 |

### Style Codes

| Code | Effect |
|------|--------|
| `~B` | Toggle bold |
| `~I` | Toggle italic |
| `~U` | Toggle underline |
| `~n` | Reset all formatting (color and style) to defaults |

Style codes are toggles -- applying `~B` turns bold on, applying `~B` again turns bold off. The `~n` reset code restores both color and style to the defaults passed by the server for that message.

### Legacy + Markdown Composition

Legacy format codes and markdown syntax compose naturally within the same text. For example:

```
~rThis is red **and bold** text~n
```

Renders "This is red" in red, "and bold" in red+bold, and "text" in red, then `~n` resets everything.

Legacy codes are processed within md4c text callbacks, so markdown structure (paragraphs, headings, lists) is parsed first, then legacy codes are applied to the text content within each element.

Legacy formatting resets at each block boundary (paragraph, heading, list item, etc.) to match pre-markdown behavior where format codes never carried across paragraphs.

---

## Markdown Syntax

Powered by md4c v0.5.2 (a CommonMark-compliant C parser). The specific GFM extensions and flags enabled are listed below.

### Inline Formatting

| Syntax | Result |
|--------|--------|
| `**bold**` | Bold text |
| `*italic*` or `_italic_` | Italic text |
| `__underline__` | Underlined text (MD_FLAG_UNDERLINE) |
| `~~strikethrough~~` | Strikethrough text (MD_FLAG_STRIKETHROUGH, descriptions only) |
| `[text](url)` | Link (clickable only with MD_RENDER_LINKS; see Rendering Contexts) |

Inline code spans (`` `code` ``) are intentionally not supported. The backtick character has been the legacy format code prefix for over two decades (`` `r `` for red, `` `B `` for bold, etc.). Supporting both meanings simultaneously creates an unresolvable parsing conflict with no clean solution -- heuristics would violate the project's code clarity guidelines.

Strikethrough is only available in the description/full-render path, not in chat.

### Block Formatting (Descriptions Only)

These features are only available in description-path rendering (items, signs, mail, news, MOTD).

#### Headings

```
# Heading 1
## Heading 2
### Heading 3
#### Heading 4 (and deeper)
```

Headings render in bold with increased font size (h1 = 280 twips, h2 = 240, h3 = 200, h4+ = 180 base) and extra paragraph spacing above (200 twips).

Permissive ATX headers are enabled (MD_FLAG_PERMISSIVEATXHEADERS), meaning the space after `#` is optional.

#### Lists

Unordered:
```
- Item one
- Item two
  - Nested item
```

Ordered:
```
1. First item
2. Second item
3. Third item
```

Lists use hanging indent (-200 twip offset) so the bullet/number sits left of the body text. Nested lists increase indent by 360 twips per level, combining both blockquote and list nesting depth.

Unordered list items use a bullet character (0x95). Ordered list items use `N. ` numbering.

#### Task Lists

Task lists are a GFM extension (MD_FLAG_TASKLISTS, descriptions only):

```
- [x] Completed task
- [ ] Incomplete task
```

Checkboxes render as Unicode ballot box glyphs:
- U+2611 (BALLOT BOX WITH CHECK) for checked items
- U+2610 (BALLOT BOX) for unchecked items

Glyphs are inserted via EM_SETTEXTEX with UTF-16 codepage (1200) since the Rich Edit control is created with the ANSI class name "RichEdit20A" but supports Unicode text insertion internally.

Checkboxes use inherited text color (no hardcoded color) so they respond to legacy color codes and match surrounding text.

Legacy format codes placed before a task list checkbox (e.g., `- ~r[x] text`) are automatically moved after the checkbox during normalization (`MdFixTaskListCodes`) so md4c can detect the task list pattern. The codes still apply to the checkbox and subsequent text.

#### Code Blocks

Fenced code blocks with triple backticks:

````
```
This is a code block
with multiple lines
```
````

Code blocks use the configurable code font (FONT_CODE, default: Courier New 9pt) and code foreground color (COLOR_CODEFGD). No background color is applied -- Rich Edit's crBackColor only paints behind individual characters, not the full line width, which creates ugly partial rectangles. Spacing before code blocks is 40 twips.

Code block text is split on newlines internally (`MdAppendCodeText`) so that `dySpaceBefore` from the initial paragraph doesn't propagate to every line.

Indented code blocks are disabled (MD_FLAG_NOINDENTEDCODEBLOCKS) to avoid accidental formatting.

#### Tables

```
| Header 1 | Header 2 |
|----------|----------|
| Cell 1   | Cell 2   |
| Cell 3   | Cell 4   |
```

Tables are rendered using the code font (FONT_CODE) for column alignment, with space-padded columns and a box-drawing separator line under the header row. Column separators use U+2502 (BOX DRAWINGS LIGHT VERTICAL) and junction points use U+253C (BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL). Header separator dashes use U+2500 (BOX DRAWINGS LIGHT HORIZONTAL). The last column omits trailing padding to save horizontal space. Header rows render in bold. Tables are only enabled in the full-render path (MD_FLAG_TABLES).

Tables support checkbox substitution: `[x]` and `[X]` display as a filled ballot box glyph (U+2611), and `[ ]` displays as an empty ballot box glyph (U+2610). Checkboxes use inherited text color. Column width calculation (`MdCellVisualWidth`) accounts for the 3-char-to-1-glyph substitution.

Legacy format codes (`~r`, `~B`, etc.) work inside table cells. `MdOutputCellText` processes sentinel-prefixed codes inline, updating the color/style state and applying it to subsequent text. `MdCellVisualWidth` also skips format code sequences when calculating column width so columns align correctly.

#### Blockquotes

```
> This is a blockquote
> It can span multiple lines
```

Blockquotes display a vertical bar prefix using the U+258C (LEFT HALF BLOCK) Unicode glyph, rendered in the configurable COLOR_QUOTEBGD color. One bar glyph is inserted per nesting level, followed by a space. The bar appears on every visual line -- `MdInsertQuoteBar` is called at the start of each paragraph inside a blockquote and after every line break (SOFTBR / BR).

Blockquote text uses the configurable COLOR_QUOTEFGD foreground color. No background color is applied (same crBackColor limitation as code blocks). Paragraph spacing inside blockquotes is tighter than normal (40 twips instead of 120) so the bar looks like a continuous strip.

No left indent is applied to blockquotes -- the bar glyph prefix handles the visual nesting.

#### Horizontal Rules

```
---
```

Renders as a full-width horizontal line using U+2500 (BOX DRAWINGS LIGHT HORIZONTAL) characters. The number of glyphs is calculated dynamically from the Rich Edit control's client width so the rule spans the full available space. Spacing is 80 twips above and below.

### Autolinks

Permissive autolinks are only enabled when `MD_RENDER_LINKS` is passed. Currently this means autolinks only work in the MOTD. URLs are automatically detected and made clickable without requiring `[text](url)` syntax. Only URL and WWW autolinks are enabled; email autolinks (`user@example.com`) are not detected.

Detected URL forms:

- `www.example.com` -- detected (must start with `www.`)
- `http://example.com` -- detected
- `https://example.com` -- detected
- `example.com` -- NOT detected (no `www.` prefix or scheme)

This follows the GFM autolink extension specification. Bare domain names without `www.` are not auto-detected to avoid false positives.

Autolinks are completely disabled in chat and in player-editable content (descriptions, mail, news, books). URLs in these contexts appear as plain text.

### Link Security

Clickable links are restricted to server-controlled content (MOTD) to prevent phishing and social engineering in the MMORPG. Player-editable text (descriptions, mail, news, books, chat) never produces clickable links.

When links are enabled, only `http://` and `https://` URLs can be opened. The following are blocked:

- `file://` paths
- `ftp://` URLs
- `javascript:` URIs
- `data:` URIs
- Local paths (`C:\...`, `\\UNC\...`)
- Any non-HTTP scheme

Bare domains (no scheme, no `www.` prefix) must contain a dot to be considered a valid URL. A bare word like "blah" is rejected; "discord.com" is allowed.

Hovering over a link shows a tooltip with the target URL. Blocked URLs show `[blocked]` in the tooltip.

---

## Configurable Colors

Colors are stored in the client's INI file as `ColorN` keys (R,G,B triplets) and can be changed via the Options menu (Options -> Colors).

The complete list of color identifiers, their INI keys, and default values is defined in:
- `clientd3d/color.h` -- enum with all `COLOR_*` identifiers and inline comments
- `clientd3d/color.c` -- default RGB values for each color

### Markdown-Specific Colors

These colors were added for the markdown rendering system:

| Enum | INI Key | Default RGB | Description |
|------|---------|-------------|-------------|
| COLOR_CODEFGD | Color29 | 90,90,90 | Code block text foreground |
| COLOR_LINKFGD | Color30 | 0,0,255 | Link text color |
| COLOR_QUOTEFGD | Color31 | 90,90,90 | Blockquote text color |
| COLOR_QUOTEBGD | Color32 | 100,100,100 | Blockquote bar color |

- COLOR_CODEFGD -- Used for fenced code blocks. Accessible via Options -> Colors -> Code blocks.
- COLOR_LINKFGD -- Used for clickable links in contexts where MD_RENDER_LINKS is enabled (currently MOTD only). Accessible via Options -> Colors -> Links.
- COLOR_QUOTEFGD -- Used for blockquote body text color.
- COLOR_QUOTEBGD -- Used for the blockquote vertical bar glyph (U+258C) color.

Note: COLOR_EDITFGD is used as the default text color for description dialogs. COLOR_MAILFGD is used for mail/news text.

---

## Configurable Fonts

Fonts are stored in the client's INI file and can be changed via the Options menu.

### All Client Fonts

| Enum | Description |
|------|-------------|
| FONT_LIST | Object list boxes |
| FONT_TITLES | Object titles, large text |
| FONT_EDIT | Main window text edit box |
| FONT_MAIL | Mail message text |
| FONT_STATS | Game statistics |
| FONT_INPUT | Input edit boxes |
| FONT_SYSMSG | System messages |
| FONT_LABELS | Labels in graphics area |
| FONT_ADMIN | Admin mode text |
| FONT_STATNUM | Numbers in statistic graph bars |
| FONT_MAP_TITLE | Map title |
| FONT_MAP_LABEL | Map labels |
| FONT_MAP_TEXT | Map text |
| FONT_MAP_ANNOTATIONS | Map and mini-map annotations |
| FONT_CODE | Code blocks and table cells |

### Markdown-Specific Font

FONT_CODE was added for the markdown system. Default: Courier New 9pt. Configurable via Options -> Fonts -> Code blocks.

This font is used for:
- Fenced code blocks
- Table cell text (for column alignment)

---

## Architecture

### Parser

md4c v0.5.2 is used as the markdown parser. It is a pure C library with a SAX-like callback interface -- it delivers enter/leave events for blocks and spans, and text events for content. The library lives in `external/md4c/`.

### Renderer

The client-side renderer is in `clientd3d/markdown.c`. It receives md4c callbacks and translates them into Win32 Rich Edit control operations (EM_SETCHARFORMAT with CHARFORMAT2, EM_SETPARAFORMAT with PARAFORMAT2, EM_REPLACESEL for text insertion, EM_SETTEXTEX for Unicode glyph insertion).

### Rich Edit Control

The client uses Rich Edit v2/3 (`"RichEdit20A"` class from `riched20.dll`). This provides:

- CHARFORMAT2 for character formatting (font, size, color, bold, italic, underline, strikethrough)
- PARAFORMAT2 for paragraph formatting (indent, space before/after, hanging indent)
- EM_SETTEXTEX with codepage 1200 (UTF-16) for inserting Unicode glyphs into the ANSI-class control

crBackColor limitation: Rich Edit only paints background color behind individual characters, not the full line width. This creates ugly partial rectangles, so no background color is applied to code blocks or blockquotes.

### Link System

Links use a subclass-based system instead of CFE_LINK/EN_LINK. CFE_LINK was removed because Rich Edit overrides crTextColor at paint time when CFE_LINK is set, forcing the system hyperlink color regardless of any color configured by the client.

The entire link system is gated behind the `MD_RENDER_LINKS` flag. When this flag is not set, `MD_SPAN_A` enter/leave callbacks are no-ops, autolinks are not detected (`MD_FLAG_PERMISSIVEURLAUTOLINKS` and `MD_FLAG_PERMISSIVEWWWAUTOLINKS` are omitted), and `MdInstallLinkHandler` is not called. Currently only the MOTD passes `MD_RENDER_LINKS`. Email autolinks (`MD_FLAG_PERMISSIVEEMAILAUTOLINKS`) are never enabled.

To enable clickable links in another context, add `| MD_RENDER_LINKS` to the `RichEditSetMarkdownText` call for that context. For example:

```c
// Enable links in news posts:
RichEditSetMarkdownText(hEdit, msg->message, defaultColor, MD_RENDER_FULL | MD_RENDER_LINKS);
```

This single flag enables autolink detection, `[text](url)` clickability, link styling (underline + COLOR_LINKFGD), tooltips, and the subclass click handler -- all at once. Each call site is independent, so links can be enabled per-context without affecting others.

The link system has three parts:

1. A link registry (`s_linkRegistry[]`, max 128 entries) stores the URL, HWND, and character range for each link
2. `MdLinkSubclassProc` is installed on Rich Edit controls via `SetWindowSubclass` (ID 1001) and handles:
   - WM_LBUTTONUP: hit-tests via EM_CHARFROMPOS, looks up the URL in the registry, opens via `WebLaunchBrowser` if safe
   - WM_MOUSEMOVE: shows/hides a tooltip with the target URL (or `[blocked]` prefix for unsafe URLs)
   - WM_SETCURSOR: shows IDC_HAND cursor when hovering over a link
   - WM_NCDESTROY: removes the subclass on window destruction
3. Link styling uses underline + COLOR_LINKFGD applied via EM_SETCHARFORMAT (no CFE_LINK flag)

The tooltip uses TTTOOLINFOA_V1_SIZE (44 bytes) for `cbSize` because the client has no comctl32 v6 manifest. The tooltip control is v5, which rejects the larger v6 TOOLINFO struct (48 bytes from `sizeof(TOOLINFO)` on modern SDKs).

`MdClearLinks` removes all stored links for a given HWND. It is called when switching from rendered preview back to raw text editing (e.g., in mail/news dialogs) so stale link regions don't trigger clicks on unformatted text.

`MdAdjustLinksForTrim` shifts link positions after characters are trimmed from the start of a Rich Edit control (used by the chat window's scroll-back trimming).

### Text Flow

#### Chat Messages

```
Server message
  -> DisplayServerMessage()      [srvrstr.c]
    -> DisplayMessageMarkdown()  [markdown.c]
      -> md4c (inline-only flags)
        -> MdChatBlockCb (no-op)
        -> MdEnterSpan / MdLeaveSpan (bold, italic, underline)
        -> MdChatTextCb -> MdChatProcessLegacyText -> MdChatOutput -> EditBoxAddText
```

Links and autolinks are not processed in the chat path. Inline code spans are not supported (see Inline Formatting).

#### Descriptions / Mail / News / MOTD

```
Description text
  -> RichEditSetMarkdownText()   [markdown.c]
    -> MdNormalizeLegacyCodes()
    -> MdFixTaskListCodes()
    -> MdPreserveBlankLines()
    -> md4c (full flags: MD_FLAG_TABLES | MD_FLAG_TASKLISTS | ...)
      -> MdEnterBlock / MdLeaveBlock (paragraphs, headings, lists, code, tables, etc.)
      -> MdEnterSpan / MdLeaveSpan (bold, italic, underline, strikethrough, links)
      -> MdText -> MdProcessLegacyText -> MdAppendText -> EM_REPLACESEL + EM_SETCHARFORMAT
```

#### Mail Message Formatting

Mail messages stored on the server have a fixed format: `From:`, `To:`, `Subject:`, and `Date:` header lines followed by a dashed separator line (`-------------`) and the message body. When the mail read dialog displays a message, `FormatMailAsMarkdown()` in `mailread.c` reformats it into markdown before rendering:

- Header labels are wrapped in bold (`**From:**`, `**To:**`, etc.)
- Two trailing spaces are appended to each header line to create markdown line breaks so headers render as separate lines without paragraph spacing
- The dashed separator is replaced with a markdown horizontal rule (`---`)
- The result is a single string rendered in one `RichEditSetMarkdownText(MD_RENDER_FULL)` pass

```
Mail message (raw)
  -> RenderMailMessage()          [mailread.c]
    -> FormatMailAsMarkdown()     [mailread.c]
      -> RichEditSetMarkdownText() [markdown.c]
```

If `FormatMailAsMarkdown()` fails (allocation failure or missing separator), the raw message text is rendered directly as a fallback.

### Legacy Format Code Normalization

Before passing text to md4c, legacy format code sequences (both backtick and tilde prefixes) are converted to single-byte sentinel characters by `MdNormalizeLegacyCodes`. Each of the 14 legacy codes maps to a unique byte in the range 0x0E-0x1B. md4c's `ISPUNCT_` macro is patched to classify these bytes as punctuation -- this is critical because CommonMark's emphasis flanking rules treat adjacent punctuation differently than adjacent letters. Without the patch, server-injected reset codes like `~n` place letter `n` adjacent to emphasis delimiters (`**`, `__`), breaking bold and underline in chat.

Both backtick and tilde prefixes remain supported for backward compatibility. Backtick-prefixed codes (`` `r ``, `` `B ``, etc.) are normalized to single-byte sentinels alongside tilde-prefixed codes. The sentinel bytes pass through md4c to text callbacks where `MdProcessLegacyText` / `MdChatProcessLegacyText` interpret them via direct index lookup. Backticks followed by a legacy code character are always consumed as format codes, never as markdown inline code delimiters.

When a `~` is immediately preceded by another `~`, it is NOT consumed as a legacy code prefix. This preserves `~~strikethrough~~` syntax for words that start with a legacy code letter (e.g. `~~cowardice~~` where `~c` would otherwise be consumed as cyan). After guarding one `~~` pair, the guard resets so a third consecutive tilde starts a new legacy code (e.g. `~~word~~~n` correctly parses as `~~word~~` strikethrough + `~n` reset).

### Task List Code Reordering

`MdFixTaskListCodes` runs after normalization to handle the case where legacy format codes appear between a list marker (`- `) and a task list checkbox (`[ ]` or `[x]`). md4c requires the checkbox immediately after the list marker to detect the task list extension. The function moves any sentinel bytes from before the checkbox to after it, preserving the format codes while allowing md4c to detect the task list pattern.

### Blank Line Preservation

md4c collapses multiple consecutive blank lines into a single paragraph break. To preserve player-intended spacing, extra blank lines beyond the first are replaced with lines containing a non-breaking space (0xA0) character, which md4c treats as separate paragraphs. The 0xA0 content is then suppressed in the text callback -- the paragraph spacing from the block enter callback handles the visual gap.

### Autolink Boundary Patch

The md4c library's `md_analyze_permissive_autolink` function was patched to add `"` and `'` to the left boundary characters and `"`, `'`, and `~` to the right boundary characters. This allows autolinks to work inside quoted text (the server wraps spoken text in double quotes, and the MOTD may contain URLs adjacent to quote characters).

---

## Blakod Usage

### Description Resources

Item and NPC descriptions in Blakod (.kod files) can use both legacy format codes and markdown syntax in their resource strings:

```
resources:
   myitem_desc_rsc = "An **enchanted** sword.\n\n"
      "## Attributes\n"
      "- ~gMagic damage~n\n"
      "- ~bIce resistance~n\n"
```

The double spacing rule for Blakod descriptions (two spaces after periods) applies to the text content, not the markdown syntax.

### Chat Messages

Player chat messages support inline markdown only (bold, italic, underline). The server wraps spoken text in quotes and appends a reset code:

```
Player says, "Check out **this** cool item!~n"
```

Links and autolinks are not active in chat. URLs typed by players appear as plain text.
