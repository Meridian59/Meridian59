
#include <owl/pch.h>
#include <owl/defs.h>

#include <algorithm>

//
// Borland includes this in run-time library
//
//#if !defined(BI_COMP_BORLANDC)

#include <owl/private/regexp.h>

namespace owl
{

/////////////////////////////////////
// TRegexp Class

typedef utchar PatternType;

int makepat(const tchar * exp, PatternType * pat, size_t maxpattern);
const tchar * matchs(const tchar * str,
                          const PatternType * pat,
                          tchar * * startpat);

const unsigned TRegexp::maxpat=128;

TRegexp::TRegexp(const tchar * str)
{
  gen_pattern( str );
}

TRegexp::TRegexp(const TRegexp  & r)
{
    copy_pattern( r );
}

TRegexp::~TRegexp()
{
    delete[] the_pattern;
}

void TRegexp::copy_pattern(const TRegexp  & r)
{
    the_pattern = new PatternType[maxpat];
    memcpy( the_pattern, r.the_pattern, maxpat );
    stat = r.stat;
}

void TRegexp::gen_pattern(const tchar * str)
{
    the_pattern = new PatternType[maxpat];
    stat = (StatVal)makepat( str, the_pattern, maxpat );
}

TRegexp  & TRegexp::operator = ( const tchar  *cp )
{
    delete the_pattern;
    gen_pattern( cp );
    return *this;
}

TRegexp  & TRegexp::operator = ( const TRegexp  & r )
{
    if(this != &r)
        {
        delete the_pattern;
        copy_pattern( r );
        }
    return *this;
}

TRegexp::StatVal TRegexp::status() throw()
{
  StatVal temp = stat;
  stat = OK;
  return temp;
}

int makepat(const tchar  *exp, PatternType   *pat, size_t maxpattern);
const tchar  * matchs( const tchar  *str,
                          const PatternType  *pat,
                          tchar  *  *startpat);

size_t TRegexp::find( const tstring & str,
                      size_t  *len,
                      size_t i ) const
{
  PRECONDITION( stat==OK );
  tchar  * startp;
  const tchar  * s = str.c_str();
  const tchar  * endp = matchs( s+i, the_pattern, &startp );
  if( endp ){
    *len = (size_t)((tchar  *)endp - startp + 1);
    return (size_t)(startp - (tchar  *)s);
  }
  else{
  *len = 0;
    return static_cast<size_t>(-1); //JJH added static_cast
  }
}

/*
 *
 * Author:    Allen I. Holub
 *
 * (c) C Gazette. May be used freely as long as author and publication are
 * acknowledged
 *
 */

/*
 *
 * Modified by Borland International to compile without warnings as C++.
 *
 * Modified by Yura Bidus to support UNICODE.
 */

/* Metacharacters in the input:                                     */
#define BOL     _T('^')     /* start-of-line anchor                 */
#define EOL     _T('$')     /* end-of-line anchor                   */
#define ANY     _T('.')     /* matches any character                */
#define CCL     _T('[')     /* start a character class              */
#define CCLEND  _T(']')     /* end a character class                */
#define NCCL    _T('^')     /* negates character class if 1st char. */
#define CLOSURE _T('*')     /* Kleene closure (matches 0 or more)   */
#define PCLOSE  _T('+')     /* Positive closure (1 or more)         */
#define OPT     _T('?')     /* Optional closure (0 or 1)            */

//?????????????????
// 0x80 will not work in UNICODE !!!!!!!!!!!!
typedef enum action {    /* These are put in the pattern string  */
                         /* to represent metacharacters.         */
  M_BOL =    (0x80 | _T('^')),
  M_EOL =    (0x80 | _T('$')),
  M_ANY =    (0x80 | _T('.')),
  M_CCL =    (0x80 | _T('[')),
  M_OPT =    (0x80 | _T('?')),
  M_CLOSE =  (0x80 | _T('*')),
  M_PCLOSE = (0x80 | _T('+'))
} action;

//typedef unsigned char pattern;  /* pattern strings are unsigned char */
typedef _TUCHAR pattern;

#define IS_ACTION(x) ((x)&0x80) /* true => element of pat. string is an   */
                                /* action that represents a metacharacter */

/*----------------------------------------------------------------------*/
#define MAPSIZE 16      /* need this many bytes for character class bit map */

/*
 * Advance a pointer into the pattern template
 * to the next pattern element, this is a +1 for
 * all pattern elements but M_CCL, where you
 * to skip past both the M_CCL character and the
 * bitmap that follows that character
 */

#define ADVANCE(pat) (pat += (*pat == (pattern)M_CCL) ? (MAPSIZE+1) : 1)

/*
 * Bitmap functions. Set bit b in the map and
 * test bit b to see if it was set previously.
 */

#define SETBIT(b,map) ((map)[((b) & 0x7f) >>3] |= pattern(1 << ((b) & 0x07)) )
#define TSTBIT(b,map) ((map)[((b) & 0x7f) >>3] &  (1<< ((b) & 0x07)) )
/*----------------------------------------------------------------------*/
#define E_NONE       0      /* Possible return values from pat_err. */
#define E_ILLEGAL    1      /* Set in makepat() to indicate prob-   */
#define E_NOMEM      2      /* lems that came up while making the   */
#define E_PAT        3      /* pattern template.                    */

/*----------------------------------------------------------------------*/

  static const tchar  *doccl(pattern  *, const tchar  *);
  static int        hex2bin(int);
  extern int        makepat( const tchar  *, pattern  *, size_t);
  extern const tchar  *matchs( const tchar  *, const pattern *, tchar  *  *);
  static int        oct2bin(int);
  static int        omatch(const tchar * *, const pattern  *, const tchar  *);
  extern const tchar  *patcmp(const tchar  *, const pattern  *, const tchar  *);
  extern int        esc( const tchar  *  *);

/*----------------------------------------------------------------------*/
int
makepat( const tchar  *exp, pattern  *pat, size_t maxpat)
//  char       *exp;      /* Regular expression */
//  pattern    *pat;      /* Assembled compiled pattern */
//  int             maxpat;   /* Length of pat */
{
  /*
   * Make a pattern template from the string pointed to by exp. Stop when
   * '\0' or '\n' is found in exp.  The pattern template is assembled
   * in pat whose length is given by maxpat.
   *
   * Return:
   * E_ILLEGAL       Illegal input pattern.
   * E_NOMEM         out of memory.
   * E_PAT           pattern too long.
   */

  pattern   *cur;       /* pointer to current pattern element  */
  pattern   *prev;      /* pointer to previous pattern element */
  int            Error = E_ILLEGAL;

  if (!*exp || *exp == _T('\n'))
    goto exit;

  if (*exp == CLOSURE || *exp == PCLOSE || *exp == OPT)
    goto exit;

  Error = E_NOMEM;
  if (!pat) 
    goto exit;      /* Check for bad pat */

  prev = cur = pat;
  Error = E_PAT;

  while (*exp && *exp != _T('\n')) {

    if (cur >= &pat[maxpat - 1]) 
      goto exit;

    switch (*exp) {
    case ANY:
      *cur = (pattern)M_ANY;
      prev = cur++;
      ++exp;
      break;

    case BOL:
      *cur = (cur == pat) ? (pattern)M_BOL : *exp;
      prev = cur++;
      ++exp;
      break;

    case EOL:
      *cur = (!exp[1] || exp[1] == _T('\n')) ? (pattern)M_EOL : *exp;
      prev = cur++;
      ++exp;
      break;

    case CCL:
      if (uint((cur - pat) + MAPSIZE) >= maxpat)
    goto exit;      /* not enough room for bit map */
      prev = cur;
      *cur++ = (pattern)M_CCL;
      exp = doccl(cur, exp);
      cur += MAPSIZE;
      break;

    case OPT:
    case CLOSURE:
    case PCLOSE:
      switch (*prev) {
      case M_BOL:
      case M_EOL:
      case M_OPT:
      case M_PCLOSE:
      case M_CLOSE:
    goto exit;
      }

      /* memmove( prev+1, prev, cur-prev ); */
      {
    pattern  *p = cur;
    while (p > prev) {
      *p = *(p - 1);
      p--;
    }
      }
      *prev = (*exp == OPT) ? (pattern)M_OPT :
    (*exp == PCLOSE) ? (pattern)M_PCLOSE : (pattern)M_CLOSE;
      ++cur;
      ++exp;
      break;

    default:
      prev = cur;
      *cur++ = (pattern)esc(&exp);
      break;
    }
  }

  *cur = _T('\0');
  Error = E_NONE;

 exit:
  return Error;
}

/*----------------------------------------------------------------------*/
static const tchar *
doccl( pattern  *map, const tchar * src)
{
  /*
   * Set bits in the map corresponding to characters specified in the src
   * character class.
   */

  int first, last, negative;
  const tchar  *start;

  ++src;            /* skip past the [          */
  negative = (*src == NCCL);
  if (negative)         /* check for negative ccl   */
    ++src;
  start = src;          /* start of characters in class */
  memset(map, 0, MAPSIZE);  /* bitmap initially empty       */

  while (*src && *src != CCLEND) {
    if (*src != _T('-')) {
      first = esc(&src);    /* Use temp. to avoid macro  */
      SETBIT(first, map);   /* side effects.             */
    }
    else if (src == start) {
      SETBIT(_T('-'), map);     /* literal dash at start or end */
      ++src;
    }
    else {
      ++src;            /* skip to end-of-sequence char */
      if (*src < src[-2]) {
        first = *src;
        last = src[-2];
      }
      else {
        first = src[-2];
        last = *src;
      }
      while (++first <= last)
        SETBIT(first, map);
      src++;
    }
  }

  if (*src == CCLEND)
    ++src;          /* Skip CCLEND */

  if (negative)
    for (first = MAPSIZE; --first >= 0;)
      *map++ ^= ~0;     /* invert all bits */

  return src;
}

/*----------------------------------------------------------------------*/
const tchar  *
matchs( const tchar * str, const pattern * pat, tchar * * startpat)
{
  const tchar * endp = NULL;
  const tchar * start;

  if (!pat)
    return NULL;

  if (*str == _T('\0')) {
    if ((*pat == (pattern)M_EOL) || (*pat == (pattern)M_BOL && (!pat[1] || pat[1] == (pattern)M_EOL)))
      endp = str;
  } 
  else {
    start = str;        /* Do a brute-force substring search,
                 * comparing a pattern against the input string */
    while (*str) {
      endp = patcmp(str, pat, start);
      if (endp)
    break;
      str++;
    }
  }
  *startpat = (tchar*)str;
  return endp;
}

/*----------------------------------------------------------------------*/
const tchar *
patcmp( const tchar * str, const pattern * pat, const tchar * start)
{
  /*
   * Like strcmp, but compares str against pat. Each element of str is
   * compared with the template until either a mis-match is found or the end
   * of the template is reached. In the former case a 0 is returned; in the
   * latter, a pointer into str (pointing to the last character in the
   * matched pattern) is returned. Strstart points at the first character in
   * the string, which might not be the same thing as line if the search
   * started in the middle of the string.
   */

  const tchar * bocl;     /* beginning of closure string.         */
  const tchar * end=0;    /* return value: end-of-string pointer. */

  if (!pat)         /* make sure pattern is valid   */
    return (NULL);

  while (*pat) {
    if (*pat == (pattern)M_OPT) {
      /*
       * Zero or one matches. It doesn't matter if omatch fails---it will
       * advance str past the character on success, though. Always advance
       * the pattern past both the M_OPT and the operand.
       */

      omatch(&str, ++pat, start);
      ADVANCE(pat);
    } 
    else if (!(*pat == (pattern)M_CLOSE || *pat == (pattern)M_PCLOSE)) {
      /*
       * Do a simple match. Note that omatch() fails if there's still
       * something in pat but we're at end of string.
       */

      if (!omatch(&str, pat, start))
        return NULL;

      ADVANCE(pat);

    } else {            /* Process a Kleene or positive closure */

      if (*pat++ == (pattern)M_PCLOSE)  /* one match required */
        if (!omatch(&str, pat, start))
          return NULL;

      /* Match as many as possible, zero is okay */

      bocl = str;
      while (*str && omatch(&str, pat, start)) { /* do nothing */ }

      /*
       * 'str' now points to the character that made made us fail. Try to
       * process the rest of the string. If the character following the
       * closure could have been in the closure (as in the pattern "[a-z]*t")
       * the final 't' will be sucked up in the while loop. So, if the match
       * fails, back up a notch and try to match the rest of the string
       * again, repeating this process recursively until we get back to the
       * beginning of the closure. The recursion goes, at most, one levels
       * deep.
       */

      if (*ADVANCE(pat)) {
        for (; bocl <= str; --str) {
          end = patcmp(str, pat, start);
          if (end) break;
        }
        return end;
      }
    break;
    }
  }

  /*
   * omatch() advances str to point at the next character to be matched. So
   * str points at the character following the last character matched when
   * you reach the end of the template. The exceptions are templates
   * containing only a BOLN or EOLN token. In these cases omatch doesn't
   * advance. Since we must return a pointer to the last matched character,
   * decrement str to make it point at the end of the matched string, making
   * sure that the decrement hasn't gone past the beginning of the string.
   *
   * Note that $ is a position, not a character, but in the case of a pattern
   * ^$, a pointer to the end of line character is returned. In ^xyz$, a
   * pointer to the z is returned.
   *
   * The --str is done outside the return statement because __max() was a macro
   * with side-effects.
   */

  --str;
  return (std::max(start, str));
}

/*----------------------------------------------------------------------*/
static int
omatch( const tchar * * strp,
        const pattern * pat,
        const tchar * start )
{
  /*
   * Match one pattern element, pointed at by pat, against the character at
   * **strp. Return 0 on a failure, 1 on success. *strp is advanced to skip
   * over the matched character on a successful match. Closure is handled one
   * level up by patcmp().
   *
   * "start" points at the character at the left edge of the line. This might
   * not be the same thing as *strp if the search is starting in the middle
   * of the string. An end-of- line anchor matches '\n' or '\0'.
   */

  int advance = -1; /* amount to advance *strp, -1 == error  */

  switch (*pat) {
  case M_BOL:           /* First char in string? */
    if (*strp == start)     /* Only one star here.   */
      advance = 0;
    break;

  case M_ANY:           /* . = anything but newline */
    if (**strp != _T('\n'))
      advance = 1;
    break;

  case M_EOL:
    if (**strp == _T('\n') || **strp == _T('\0'))
      advance = 0;
    break;

  case M_CCL:
    if (TSTBIT(**strp, pat + 1))
      advance = 1;
    break;

  default:          /* literal match */
    if (**strp == *pat)
      advance = 1;
    break;
  }

  if (advance > 0)
    *strp += advance;

  return (advance + 1);
}

#define ISHEXDIGIT(x) (_istdigit(x)                             \
                            || (_T('a')<=(x) && (x)<=_T('f'))   \
                            || (_T('A')<=(x) && (x)<=_T('F'))   )

#define ISOCTDIGIT(x) (_T('0')<=(x) && (x)<=_T('7'))

static int hex2bin( int c )
{
     /* Convert the hex digit represented by 'c' to an int. 'c'
      * must be one of: 0123456789abcdefABCDEF
      */
     return (_istdigit(c) ? (c)-_T('0') : ((_toupper(c))-_T('A'))+10)  & 0xf;
}

static int oct2bin( int c )
{
     /* Convert the hex digit represented by 'c' to an int. 'c'
      * must be a digit in the range '0'-'7'.
      */
     return ( ((c)-_T('0'))  &  0x7 );
}

/*------------------------------------------------------------*/

int esc( const tchar * * s)
{
     /* Map escape sequences into their equivalent symbols. Return
      * the equivalent ASCII character. *s is advanced past the
      * escape sequence. If no escape sequence is present, the
      * current character is returned and the string is advanced by
      * one. The following are recognized:
      *
      *  \b     backspace
      *  \f     formfeed
      *  \n     newline
      *  \r     carriage return
      *  \s     space
      *  \t     tab
      *  \e     ASCII ESC character ('\033')
      *  \DDD   number formed of 1-3 octal digits
      *  \xDDD  number formed of 1-3 hex digits
      *  \^C    C = any letter. Control code
      */

     int rval;

     if( **s != _T('\\') )
          rval = *( (*s)++ );
     else {
          ++(*s);                                 /* Skip the \ */
          switch( toupper(**s) ) {
            case _T('\0'):  rval = _T('\\');             break;
            case _T('B'):   rval = _T('\b') ;            break;
            case _T('F'):   rval = _T('\f') ;            break;
            case _T('N'):   rval = _T('\n') ;            break;
            case _T('R'):   rval = _T('\r') ;            break;
            case _T('S'):   rval = _T(' ')  ;            break;
            case _T('T'):   rval = _T('\t') ;            break;
            case _T('E'):   rval = _T('\033');           break;

            case _T('^'):
              rval = *++(*s) ;
              rval = _toupper(rval) - _T('@') ;
              break;

            case _T('X'):
              rval = 0;
              ++(*s);
              if( ISHEXDIGIT(**s) ) {
                   rval  = hex2bin( *(*s)++ );
              }
              if( ISHEXDIGIT(**s) ) {
                   rval <<= 4;
                   rval  |= hex2bin( *(*s)++ );
              }
              if( ISHEXDIGIT(**s) ) {
                   rval <<= 4;
                   rval  |= hex2bin( *(*s)++ );
              }
              --(*s);
              break;

            default:
              if( !ISOCTDIGIT(**s) )
                   rval = **s;
              else {
                   ++(*s);
                   rval = oct2bin( *(*s)++ );
                   if( ISOCTDIGIT(**s) ) {
                        rval <<= 3;
                        rval  |= oct2bin( *(*s)++ );
                   }
                   if( ISOCTDIGIT(**s) ) {
                        rval <<= 3;
                        rval  |= oct2bin( *(*s)++ );
                   }
                   --(*s);
              }
              break;
          }
          ++(*s);
     }
     return rval;
}

}

//#endif


//==============================================================================

