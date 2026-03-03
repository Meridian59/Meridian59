// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * markdown.h:  Header file for markdown.c
 *
 * Renders markdown and legacy format codes in Rich Edit controls
 * using md4c for parsing and CHARFORMAT2 for styling.
 */

#ifndef _MARKDOWN_H
#define _MARKDOWN_H

/* Flags to control which markdown features are enabled (bitmask). */
static const int MD_RENDER_FULL   = 0;  /* All features (headings, lists, bold, italic, etc.) */
static const int MD_RENDER_INLINE = 1;  /* Inline only (bold, italic -- no blocks) */
static const int MD_RENDER_LINKS  = 2;  /* Enable clickable links and autolink detection */
static const int MD_RENDER_APPEND = 4;  /* Append to existing content (don't clear control) */

/* Default font size in twips (9pt * 20 twips/pt = 180).
 * Used to reset CHARFORMAT2.yHeight after markdown rendering. */
static const int MD_DEFAULT_FONT_TWIPS = 180;

/*
 * RichEditSetMarkdownText:  Renders markdown text into a Rich Edit control.
 *   Parses both markdown syntax and legacy format codes (~B, ~I, ~r, etc.).
 *   The flags parameter controls which features are enabled
 *   (MD_RENDER_FULL, MD_RENDER_INLINE, MD_RENDER_LINKS, MD_RENDER_APPEND).
 */
M59EXPORT void RichEditSetMarkdownText(HWND hwndRichEdit, const char *text,
                                       COLORREF defaultColor, int flags);

/*
 * DisplayMessageMarkdown:  Displays a chat/broadcast message with inline
 *   markdown support (bold, italic, underline, strikethrough). Handles
 *   profanity filtering and legacy format codes alongside markdown spans.
 */
M59EXPORT void DisplayMessageMarkdown(const char *message, COLORREF start_color,
                                      BYTE start_style);

/*
 * MdInstallLinkHandler:  Installs a window subclass on a Rich Edit control
 *   that handles link click, hover tooltip, and hand cursor behavior. Safe
 *   to call multiple times on the same HWND -- SetWindowSubclass with the
 *   same ID updates without duplicating.
 */
M59EXPORT void MdInstallLinkHandler(HWND hwndRichEdit);

/*
 * MdClearLinks:  Removes all stored links for a Rich Edit control. Call
 *   when switching from rendered preview back to raw text editing.
 */
M59EXPORT void MdClearLinks(HWND hwnd);

/*
 * MdAdjustLinksForTrim:  Adjusts link registry positions after the edit
 *   box trims charsRemoved characters from the start. Removes links
 *   that have scrolled out of range.
 */
void MdAdjustLinksForTrim(HWND hwnd, LONG charsRemoved);

#endif /* #ifndef _MARKDOWN_H */
