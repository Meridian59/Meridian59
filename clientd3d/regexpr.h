// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*

regexpr.h

Author: Tatu Ylonen <ylo@ngs.fi>

Copyright (c) 1991 Tatu Ylonen, Espoo, Finland

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies.  This
software is provided "as is" without express or implied warranty.

Created: Thu Sep 26 17:15:36 1991 ylo
Last modified: Mon Nov  4 15:49:46 1991 ylo

*/

#ifndef REGEXPR_H
#define REGEXPR_H

#ifdef __cplusplus
extern "C" {
#endif

#define RE_NREGS	100  /* number of registers available */

typedef struct re_pattern_buffer
{
  char *buffer; 	 /* compiled pattern */
  int allocated;	 /* allocated size of compiled pattern */
  int used;		 /* actual length of compiled pattern */
  char *fastmap;	 /* fastmap[ch] is true if ch can start pattern */
  char *translate;	 /* translation to apply during compilation/matching */
  char fastmap_accurate; /* true if fastmap is valid */
  char can_be_null;	 /* true if can match empty string */
  char uses_registers;	 /* registers are used and need to be initialized */
  char anchor;		 /* anchor: 0=none 1=begline 2=begbuf */
} *regexp_t;

typedef struct re_registers
{
  int start[RE_NREGS];  /* start offset of region */
  int end[RE_NREGS];    /* end offset of region */
} *regexp_registers_t;

/* bit definitions for syntax */
#define RE_NO_BK_PARENS		1    /* no quoting for parentheses */
#define RE_NO_BK_VBAR		2    /* no quoting for vertical bar */
#define RE_BK_PLUS_QM		4    /* quoting needed for + and ? */
#define RE_TIGHT_VBAR		8    /* | binds tighter than ^ and $ */
#define RE_NEWLINE_OR		16   /* treat newline as or */
#define RE_CONTEXT_INDEP_OPS	32   /* ^$?*+ are special in all contexts */
#define RE_ANSI_HEX		64   /* ansi sequences (\n etc) and \xhh */
#define RE_NO_GNU_EXTENSIONS   128   /* no gnu extensions */

/* definitions for some common regexp styles */
#define RE_SYNTAX_AWK	(RE_NO_BK_PARENS|RE_NO_BK_VBAR|RE_CONTEXT_INDEP_OPS)
#define RE_SYNTAX_EGREP	(RE_SYNTAX_AWK|RE_NEWLINE_OR)
#define RE_SYNTAX_GREP	(RE_BK_PLUS_QM|RE_NEWLINE_OR)
#define RE_SYNTAX_EMACS	0

int re_set_syntax(int syntax);
/* This sets the syntax to use and returns the previous syntax.  The
   syntax is specified by a bit mask of the above defined bits. */

const char *re_compile_pattern(char *regex, int regex_size, regexp_t compiled);
/* This compiles the regexp (given in regex and length in regex_size).
   This returns NULL if the regexp compiled successfully, and an error
   message if an error was encountered.  The buffer field must be
   initialized to a memory area allocated by malloc (or to NULL) before
   use, and the allocated field must be set to its length (or 0 if buffer is
   NULL).  Also, the translate field must be set to point to a valid
   translation table, or NULL if it is not used. */

int re_match(regexp_t compiled, char *string, int size, int pos,
	     regexp_registers_t regs);
/* This tries to match the regexp against the string.  This returns the
   length of the matched portion, or -1 if the pattern could not be
   matched and -2 if an error (such as failure stack overflow) is
   encountered. */

int re_match_2(regexp_t compiled, char *string1, int size1,
	      char *string2, int size2, int pos, regexp_registers_t regs,
	       int mstop);
/* This tries to match the regexp to the concatenation of string1 and
   string2.  This returns the length of the matched portion, or -1 if the
   pattern could not be matched and -2 if an error (such as failure stack
   overflow) is encountered. */

int re_search(regexp_t compiled, char *string, int size, int startpos,
	      int range, regexp_registers_t regs);
/* This rearches for a substring matching the regexp.  This returns the first
   index at which a match is found.  range specifies at how many positions to
   try matching; positive values indicate searching forwards, and negative
   values indicate searching backwards.  mstop specifies the offset beyond
   which a match must not go.  This returns -1 if no match is found, and
   -2 if an error (such as failure stack overflow) is encountered. */

int re_search_2(regexp_t compiled, char *string1, int size1,
		char *string2, int size2, int startpos, int range,
		regexp_registers_t regs, int mstop);
/* This is like re_search, but search from the concatenation of string1 and
   string2.  */

void re_compile_fastmap(regexp_t compiled);
/* This computes the fastmap for the regexp.  For this to have any effect,
   the calling program must have initialized the fastmap field to point
   to an array of 256 characters. */

char *re_comp(char *s);
/* BSD 4.2 regex library routine re_comp.  This compiles the regexp into
   an internal buffer.  This returns NULL if the regexp was compiled
   successfully, and an error message if there was an error. */

int re_exec(char *s);
/* BSD 4.2 regexp library routine re_exec.  This returns true if the string
   matches the regular expression (that is, a matching part is found
   anywhere in the string). */

char *re_replace_all(regexp_t bufp, char *string, char *with, int (*verifycallback)(char*,int,int));
/* Utility function.  This returns a malloc'd result of a global
 * search and replace using the regular expression on the given string.
 * Supports register replacements (such as \31) defined in the regular
 * expression and found in the string, but the rest of 'with' is considered
 * literal.  If verifycallback is non-null, it is called on each match;
 * replacement is only performed if verifycallback returns nonzero.
 */

#ifdef __cplusplus
}; // extern "C"
#endif

#endif /* REGEXPR_H */

