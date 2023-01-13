// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * profane.c:  Profanity filter
 */

#include "client.h"

#include "profane.h"
#include "regexpr.h"
#include <ctype.h>

/***************************************************************************/

typedef struct tagREGEXP
{
	char* m_pszTerm;
	char* m_pszPattern;
	struct re_pattern_buffer m_rexp;
	char m_achFastMap[256];
	int m_nLength;
} REGEXP;

static char* _szProfaneFile = "mail\\profane.dat";

static REGEXP* _apExpressions = NULL;
static int _nExpressions = 0;
static int _nAllocated = 0;
#define CHUNKSIZE 20

static char _szPrefix[] = "\\([`~][rgbBIUn]\\)*";
static char _szPostfix[] = "\\([`~][rgbBIUn]\\)*";
static char _szGrout[] = "\\([`~][rgbBIUn]\\|[^a-zA-Z0-9\"]\\)*";
static int _nGrout;

static char* _szAlpha[] =
{
	// Letter lookups are based on similar or passable appearances in
	// the default character set and font for the text window.  Other
	// fonts may define characters differently.  Most users are not
	// setting their fonts at all, so this is a reasonable stopgap.
	//    (Times New Roman, Windows ANSI (single-byte) Western Character Set)
	//
	"[Aa@^]+",
	"[Bb]+",  // -D  ,B
	"[Cc]+", //  cents  (C)  C,  c,
	"[Dd]+", //  -D  dx
	"[Ee]+",
	"[Ff]+",  // poundsterling  function
	"[Gg]+",  // C,  c,
	"[Hh]+",
	"[Ii1|!]+",
	"[Jj]+",  //  !
	"[Kk]+",
	"[Ll1|!]+",  //  vbar  dagger  doubledagger  poundsterling  ^1
	"[Mm]+",  //  TM
	"[Nn]+",  //  N~  n~
	"[Oo0@]+",
	"[Pp]+", //  paragraph  Tongue  tongue
	"[Qq]+",  //  O/  o/
	"[Rr]+", //  (R)
	"[Ss$]+", //  S^  s^  section  beta[germanSS]
	"[Tt]+",  //  function  dagger  doubledagger  TM
	"[UuVv]+",  //  mu  (accents)
	"[VvUu]+",
	"[Ww]+",
	"[Xx]+", //  multiply
	"[Yy]+",
	"[Zz2]+" //  ^2
};

#define MAXPATTERN (sizeof(_szPrefix)+(150*MAXPROFANETERM)+sizeof(_szPostfix)+1)

static char* _szWith[] =
{
	"!#@*%",
	"@+$&!",
	"*!%#@",
	"&@!+$",
	"%#!@*",
	"!+@$&",
	"@*%!#",
	"$&!@+",
	NULL
};
static int _nWith;

/***************************************************************************/

void LoadProfaneTerms()
{
	FILE* pFile = NULL;

	_nWith = 0;
	_nGrout = strlen(_szGrout);

	pFile = fopen(_szProfaneFile, "rt");
	if (!pFile)
	{
		debug(("LoadProfaneTerms(): cannot open %s\n", _szProfaneFile));
		return;
	}

	while (!feof(pFile))
	{
		char input[100];
		char* p;

		if (NULL == fgets(input, sizeof(input)-1, pFile))
			break;

		p = strchr(input, '\r');
		if (p)
			*p = '\0';
		p = strchr(input, '\n');
		if (p)
			*p = '\0';

		p = input;
		while (*p == ' ' || *p == '\t')
			p++;

		if (!*p)
			continue;

		switch (*p)
		{
		case '+':					// add a literal that isn't encrypted
			AddProfaneTerm(p+1);
			break;

		case '-':					// remove a literal that isn't encrypted
			RemoveProfaneTerm(p+1);
			break;

		case ';':					// comment line
			break;

		default:
			while (*p)				// decrypt a literal and add it
			{
				*p ^= 0x05;
				p++;
			}
			AddProfaneTerm(input);
			break;
		}
	}

	fclose(pFile);
}

void CompileProfaneExpression(REGEXP* pExp)
{
	char buffer[MAXPATTERN];
	char* p;
	char* q;
	int i;

	if (!pExp)
		return;

	if (pExp->m_pszPattern)
	{
		SafeFree(pExp->m_pszPattern);
		pExp->m_pszPattern = NULL;
	}
	if (pExp->m_rexp.buffer)
	{
		SafeFree(pExp->m_rexp.buffer);
		pExp->m_rexp.buffer = NULL;
	}

	// We convert the term in m_pszTerm into a more powerful m_pszPattern.
	// This basically converts each letter 'a' into a pattern like '[Aa]+',
	// then it inserts the _szGrout between each letter so it ignores
	// stuff like color codes and spaces and punctuation.
	//
	// We want to find uses of "heck" and "hhh - eee - ccc - kkk".
	//
	p = buffer;
	q = pExp->m_pszTerm;
	if (!config.extraprofane)
	{
		strcpy(p, _szPrefix);
		p += strlen(_szPrefix);
	}
	while (*q)
	{
		if (((p-buffer) + 2*_nGrout) > sizeof(buffer)/sizeof(char))
			break;

    // Add letter-specific pattern to the main pattern.
    i = tolower(*q)-'a';
    if (i >= 0 && i < 26)
    {
      strcpy(p, _szAlpha[i]);
      while (*p)
        p++;
    }

		q++;

		if (*q)
		{
			strcpy(p, _szGrout);
			p += _nGrout;
		}
	}
	*p = '\0';
	if (!config.extraprofane)
	{
		strcpy(p, _szPostfix);
	}
	pExp->m_pszPattern = strdup(buffer);

	// Finish preparing the pattern into a precompiled regular expression.
	//
	pExp->m_rexp.fastmap = pExp->m_achFastMap;
	pExp->m_nLength = strlen(pExp->m_pszTerm);
	re_compile_pattern(pExp->m_pszPattern, strlen(pExp->m_pszPattern), &pExp->m_rexp);
	re_compile_fastmap(&pExp->m_rexp);
}

void RecompileAllProfaneExpressions()
{
	int i;
	for (i = 0; i < _nExpressions; i++)
		if (_apExpressions[i].m_pszTerm)
			CompileProfaneExpression(&_apExpressions[i]);
}

BOOL AddProfaneTerm(char* pszTerm)
{
	REGEXP* pExp = NULL;
	char term[MAXPROFANETERM+1];
	char* p;
	char* q;
	int i;

	if (!pszTerm || !*pszTerm)
		return FALSE;

	// If this is the first term, set up.
	//
	if (!_apExpressions)
	{
		_nAllocated = CHUNKSIZE;
		_nExpressions = 0;
		_apExpressions = (REGEXP *) SafeMalloc(_nAllocated * sizeof(REGEXP));
		if (!_apExpressions)
		{
			_nAllocated = 0;
			debug(("AddProfaneTerm(): could not allocate initial REGEXPs\n"));
			return FALSE;
		}
		memset(_apExpressions, 0, _nAllocated * sizeof(REGEXP));
	}

	// If the user's adding one already in the set, we remove and recompile.
	// This could be slow now if we recompile 'heck' a bezillion times, but
	// it is a lot better than searching for the pattern a bezillion times
	// per incoming message later.  Next save will cull out the duplicates,
	// anyway.
	//
    RemoveProfaneTerm(pszTerm);

	// Strip the term down to alphabetics.
	//
	p = term;
	q = pszTerm;
	while (*q)
	{
		if (isalpha(*q))
			*p++ = tolower(*q);
		q++;
	}
	*p = '\0';
	if (p == term)
		return FALSE;

	// Find a free structure if there is one.
	//
	pExp = NULL;
	for (i = 0; i < _nExpressions; i++)
	{
		if (!_apExpressions[i].m_pszTerm)
		{
			pExp = &_apExpressions[i];
			break;
		}
	}
	if (!pExp)
	{
		if (_nExpressions < _nAllocated)
		{
			pExp = &_apExpressions[_nExpressions];
			_nExpressions++;
		}
	}

	// Reallocate another CHUNKSIZE term structures if necessary.
	//
	if (!pExp)
	{
		pExp = (REGEXP *) SafeRealloc(_apExpressions, (_nAllocated+CHUNKSIZE)*sizeof(REGEXP));
		if (pExp)
		{
			_apExpressions = pExp;

			for (i = _nAllocated; i < _nAllocated+CHUNKSIZE; i++)
				memset(&_apExpressions[i], 0, sizeof(REGEXP));
			pExp = _apExpressions + _nExpressions;

			_nAllocated += CHUNKSIZE;
			_nExpressions++;
		}
	}
	if (!pExp)
		return FALSE;

	// Prepare the term/pattern structure.
	//
	pExp->m_pszTerm = strdup(term);
	CompileProfaneExpression(pExp);

	return TRUE;
}

BOOL RemoveProfaneTerm(char* pszTerm)
{
	char term[MAXPROFANETERM+1];
	char* p;
	char* q;
	int i;

	if (!pszTerm || !*pszTerm)
		return FALSE;

	// Strip the term down to alphabetics.
	//
	p = term;
	q = pszTerm;
	while (*q)
	{
		if (isalpha(*q))
			*p++ = tolower(*q);
		q++;
	}
	*p = '\0';
	if (p == term)
		return FALSE;

	// For all terms, if one matches the given term, disable and free it.
	// Note we don't actually remove the REGEXP struct from the array/list.
	//
	for (i = 0; i < _nExpressions; i++)
	{
		if (_apExpressions[i].m_pszTerm)
		{
			if (0 == strcmp(term, _apExpressions[i].m_pszTerm))
			{
				SafeFree(_apExpressions[i].m_pszTerm);
				_apExpressions[i].m_pszTerm = NULL;
				SafeFree(_apExpressions[i].m_pszPattern);
				_apExpressions[i].m_pszPattern = NULL;
				SafeFree(_apExpressions[i].m_rexp.buffer);
				_apExpressions[i].m_rexp.buffer = NULL;
			}
		}
	}

	return FALSE;
}

void SaveProfaneTerms()
{
	FILE* pFile = NULL;
	int i;

	if (!_apExpressions || !_nExpressions)
	{
		debug(("SaveProfaneTerms(): never been loaded\n"));
		return;
	}

	pFile = fopen(_szProfaneFile, "wt");
	if (!pFile)
	{
		debug(("SaveProfaneTerms(): cannot open %s to save\n", _szProfaneFile));
		return;
	}

	for (i = 0; i < _nExpressions; i++)
	{
		if (_apExpressions[i].m_pszTerm)
		{
			// Save this term.
			//
			char* term = strdup(_apExpressions[i].m_pszTerm);
			char* p = term;

			while (*p)
			{
				*p ^= 0x05;				// save literals encrypted
				p++;
			}
			fputs(term, pFile);
			fputs("\n", pFile);

			//TODO: free this pattern
			SafeFree(term);
		}
	}

	fclose(pFile);
}

void FreeProfaneTerms()
{
	// For all terms, free any memory allocated for the term.
	//
	int i;
	for (i = 0; i < _nExpressions; i++)
	{
		if (_apExpressions[i].m_pszTerm)
		{
			SafeFree(_apExpressions[i].m_pszTerm);
			_apExpressions[i].m_pszTerm = NULL;
		}
		if (_apExpressions[i].m_pszPattern)
		{
			SafeFree(_apExpressions[i].m_pszPattern);
			_apExpressions[i].m_pszPattern = NULL;
		}
		if (_apExpressions[i].m_rexp.buffer)
		{
			SafeFree(_apExpressions[i].m_rexp.buffer);
			_apExpressions[i].m_rexp.buffer = NULL;
		}
	}

	// Free the memory set up for the list of terms.
	SafeFree(_apExpressions);
	_apExpressions = NULL;
	_nAllocated = 0;
	_nExpressions = 0;
}

/***************************************************************************/

// VerifyProfaneUsage:
// This is called when a suspected curse is found in a larger string,
// either as a callback for the re_replace_all(), or after a call to
// re_search().  The match decides if it really is an intentional curse
// or if it is likely an innocent inclusion, based on the way it lays
// in the string as a whole.  The more "visible" the word boundaries
// are, the more likely it was an intentional curse.
//
int VerifyProfaneUsage(char* string, int from, int to)
{
	BOOL bStart = FALSE;
	BOOL bEnd = FALSE;

	if (!string || !*string)
		return FALSE;

	// if we're suspicious by nature, kill the curse anywhere
	if (config.extraprofane)
		return TRUE;

	// curse is at start of string?
	if (from == 0)
		bStart = TRUE;

	// curse is at end of string?
	if (string[to] == '\0')
		bEnd = TRUE;

	// curse starts with color?
	if (string[from] == '~' || string[from] == '`')
		bStart = TRUE;

	// curse ends with color?
	if (to >= 2 && (string[to-2] == '~' || string[to-2] == '`'))
		bEnd = TRUE;

	// curse follows nonalpha?
	if (from >= 1 && !isalpha(string[from-1]))
		bStart = TRUE;

	// curse precedes nonalpha?
	if (!isalpha(string[to]))
		bEnd = TRUE;

	// If both the start and the end are suspicious,
	// kill the curse.
	//
	if (bStart && bEnd)
		return TRUE;

	// If the curse is not broken up by spaces or
	// color codes, then it is likely intentional.
	//
	{
		BOOL bBroken = FALSE;
		char* p = string+from;
		while (*p && p < string+to)
		{
			while ((*p == '~' || *p == '`') && *(p+1) && p < string+to)
				p += 2;
			if (!*p || p >= string+to)
				break;
			if (!isalpha(*p))
				bBroken = TRUE;
			p++;
		}
		if (!bBroken)
			return TRUE;
	}

	// If they're trying to be sneaky with extended
	// characters, it's probably intentional.
	//
	{
		BOOL bExtended = FALSE;
		char* p = string+from;
		while (*p && p < string+to)
		{
			if (*p & 0x80)
				bExtended = TRUE;
			p++;
		}
		if (bExtended)
			return TRUE;
	}

	return FALSE;
}

BOOL ContainsProfaneTerms(char* pszText)
{
	struct re_registers regs;
	int i, a, l;

	// We scan the string with each profane term in turn, until
	// the first profane term is found, or we exhaust the list of terms.
	// This doesn't count the terms found, it stops at the first term.
	// String is not modified.

	l = strlen(pszText);
	for (i = 0; i < _nExpressions; i++)
	{
		if (_apExpressions[i].m_pszTerm)
		{
			a = re_search(&_apExpressions[i].m_rexp, pszText, l, 0, l, &regs);
			if (a >= 0)
				return VerifyProfaneUsage(pszText, regs.start[0], regs.end[0]);
		}
	}

	return FALSE;
}

char* CleanseProfaneString(char* pszText)
{
	char* ping = strdup(pszText);
	char* pong = NULL;
	int ball = 1;
	int i;

	// We bounce the string between two pointers, cleansing it of a new
	// profane term each time the ball goes from 0 to 1 or 1 to 0.
	// At all times, the pointer with the ball (either ping or pong)
	// has the current string.

	// We walk forwards.
	// Thus, longer and more inclusive words should be placed in the file first.
	//
	for (i = 0; i < _nExpressions; i++)
	{
		if (_apExpressions[i].m_pszTerm)
		{
			char* buffer;
			char* with;

			_nWith++;
			if (!_szWith[_nWith])
				_nWith = 0;

			with = _szWith[_nWith];

			buffer = re_replace_all(&_apExpressions[i].m_rexp, ball?ping:pong, with, VerifyProfaneUsage);
			if (!buffer)
				break;

			SafeFree(ball?ping:pong);
			ball = !ball;
			if (ball)
				ping = buffer;
			else
				pong = buffer;
		}
	}

	return (ball?ping:pong);
}

/***************************************************************************/
