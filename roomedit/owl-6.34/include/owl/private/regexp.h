/*------------------------------------------------------------------------*/
/*                                                                        */
/*  REGEXP.H                                                              */
/*                                                                        */
/*------------------------------------------------------------------------*/

/*
 *      C/C++ Run Time Library - Version 7.0
 *
 *      Copyright (c) 1987, 1996 by Borland International
 *      All Rights Reserved.
 *
 */


#ifndef __cplusplus
#error Must use C++ for REGEXP.H
#endif

#ifndef __REGEXP_H
#define __REGEXP_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>


///  \class TRegexp
//
/// This class represents regular expressions. TRegexp is a support class used by
/// the string class for string searches.
/// Regular expressions use these special characters:
/// \code
/// 	.	[	]	-	^	*	?	+	$
/// \endcode
///
/// General Rules
///
/// Characters other than the special characters match themselves. For example
/// "yardbird" matches "yardbird".
/// A backslash (\\) followed by a special character, matches the special character
/// itself. For example "Pardon\\?" matches "Pardon?".
/// The following escape codes can be used to match control characters:
/// \code
/// \b	backspace
/// \e	Esc
/// \f	formfeed
/// \n	newline
/// \r	carriage return
/// \t	tab
/// \xddd	the literal hex number 0xddd
/// \^x	where x matches some control-code (for example \^c, \^c)
/// \endcode
///
/// One-Character Regular Expressions
/// - The . special character matches any single character except a newline character.
/// For example ".ive" would match "jive" or "five".
/// - The [ and ] special characters are used to denote one-character regular
/// expressions that will match any of the characters within the brackets. For
/// example, "[aeiou]" would match either "a", "e", "i", "o", or "u".
/// - The - special character is used within the [ ] special characters to denote a
/// range of characters to match. For example, "[ a-z ]" would match on any
/// lowercase alphabetic character between a and z.
/// - The ^ special character is used to specify search for any character but those
/// specified. For example, "[ ^g-v ]" would match on any lowercase alphabetic
/// character NOT between g and v.
/// 
/// 	Multiple-Character Regular Expressions
/// - The * special character following a one-character regular expression matches
/// zero or more occurrences of that regular expression. For example, "[ a-z ]*"
/// matches zero or more occurrences of lowercase alphabetic characters.
/// - The + special character following a one-character regular expression matches one
/// or more occurrences of that regular expression. For example, "[ 0-9 ]+" matches
/// one or more occurrences of lowercase alphabetic characters.
/// - The ? special character specifies that the following character is optional. For
/// example "xy?z" matches on "xy" or "xyz".
///
/// Regular expressions can be concatentated. For example, "[ A-Z ][ a-z ]*" matches
/// capitalized words.
///
/// 	Matching at the Beginning and End of a Line
/// If the ^ special character is at the beginning of a regular expression, then a
/// match occurs only if the string is at the beginning of a line. For example, "^[
/// A-Z ][ a-z ]*" matches capitalized words at the beginning of a line.
/// If the $ special character is at the end of a regular expression, the then a
/// match occurs only if the string is at the end of a line. For example, "[ A-Z ][
/// a-z ]*$" matches capitalized words at the end of a line.

class _OWLCLASS TRegexp {
  public:
/// StatVal enumerates the status conditions returned by TRegexp::status
    enum StatVal {
        OK=0,			///< Means the given regular expression is legal
        ILLEGAL,	///< Means the pattern was illegal
        NOMEMORY,
        TOOLONG		///< Means the pattern exceeded maximum length (128)
    };

     TRegexp( const tchar  *cp );
     TRegexp( const TRegexp  &r );
     ~TRegexp();

    TRegexp  &  operator = ( const TRegexp  &r );
    TRegexp  &  operator = ( const tchar  *cp );
    size_t  find( const tstring  &s,
                           size_t  *len,
                           size_t start = 0 ) const;
    StatVal  status() throw();

private:

    void  copy_pattern( const TRegexp  &r );
    void  gen_pattern( const tchar  *cp );

    _TUCHAR  *the_pattern;
    StatVal stat;
    static const unsigned maxpat;
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OWL namespace

#endif /* __REGEXP_H */
