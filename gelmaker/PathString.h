// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// PathString.h : header file
//
//////////
//
// Copyright (C) Ed Halley.
//
// This unpublished source code contains trade secrets and
// copyrighted materials which are the property of Ed Halley.
// Unauthorized use, copying or distribution is a violation of
// U.S. and international laws and is strictly prohibited.
//
//////////
//
// A CPathString is just like any other CString in most regards.
// However, it is intended to hold MSDOS path and filename strings.
// Whenever a string value is assigned into a CPathString, it
// automatically changes all Unix-style forward slashes into backslashes.
// It can check itself to determine if it is a relative or absolute
// pathname, and other useful utilities.
//
//////////
//

#ifndef __PATHSTRING_H__
#define __PATHSTRING_H__

/////////////////////////////////////////////////////////////////////////////

#ifndef __AFXEDH_H__
#error Must include AfxEdh.h (in your stdafx.h) to support this module.
#endif

#ifdef _MAC
#error This class is not (yet) implemented as a cross-platform data type.
#endif

/////////////////////////////////////////////////////////////////////////////

class CPathString : public CString
{
public:
	CPathString() : CString() { ; }
	CPathString(const CString& stringSrc) : CString(stringSrc) { SelfCorrect(); }
	CPathString(TCHAR ch, int nRepeat = 1) : CString(ch, nRepeat) { AfxThrowNotSupportedException(); }
	CPathString(LPCSTR lpsz) : CString(lpsz) { SelfCorrect(); }
	CPathString(LPCWSTR lpsz) : CString(lpsz) { SelfCorrect(); }
	CPathString(LPCTSTR lpch, int nLength) : CString(lpch, nLength) { SelfCorrect(); }
	CPathString(const unsigned char* psz) : CString(psz) { SelfCorrect(); }
		// Note that the constructors are all the same as CString, except
		// that single character and repeated character strings are not
		// supported (filenames are not usually constructed that way).
		// All constructors call SelfCorrect() automatically.

// Attributes
public:
	TCHAR operator[](int nIndex) const { return CString::operator[](nIndex); }
	void SetAt(int nIndex, TCHAR ch) { CString::SetAt(nIndex, ch); SelfCorrect(); }
	operator LPCTSTR() const { return m_pchData; }

// Operations
public:

	void SelfCorrect();
		// Converts slashes to backslashes, illegal doubles to singles, etc.
		// Use this function after a new string value is put in the string.
		// All the constructors and assignment operators perform this call
		// automatically, but this should be called if you get the string
		// from other means, such as via GetWindowText() or DDX routines,
		// or after modifying the string through GetBuffer()/ReleaseBuffer().
		// This does not validate the filename; it just corrects common
		// string problems so that APIs can use the string with fewer errors.

	LPCTSTR GetFilename() const;
		// Returns pointer to first character of filename without path.
		// For example, "f:foo.bar\bar.baz\baz.foo\file.txt" will return a
		// pointer to the "file.txt" portion.  This does not validate the
		// filename; it just returns its guess as to the file portion of the
		// string.

	BOOL IsRelative() const;
		// Determines if the path begins with relative position, or an
		// absolute position.  For example: "c:\absol\foo.txt" is absolute
		// and IsRelative() would return FALSE; "foo.txt" and "q:dir\foo.txt"
		// are relative, so would return TRUE.

	BOOL FileExists() const;
		// Determines if the file exists on the filesystem.  (Attempts to
		// open the file in a read-only, share-compatible mode.)

// Overrides
public:
	// overloaded assignment
	const CPathString& operator=(const CString& stringSrc) { CString::operator=(stringSrc); SelfCorrect(); return *this; }
	const CPathString& operator=(TCHAR ch) { AfxThrowNotSupportedException(); return *this; }
#ifdef _UNICODE
	const CPathString& operator=(char ch) { AfxThrowNotSupportedException(); return *this; }
#endif
	const CPathString& operator=(LPCSTR lpsz) { CString::operator=(lpsz); SelfCorrect(); return *this; }
	const CPathString& operator=(LPCWSTR lpsz) { CString::operator=(lpsz); SelfCorrect(); return *this; }
	const CPathString& operator=(const unsigned char* psz) { CString::operator=(psz); SelfCorrect(); return *this; }

	BOOL LoadString(UINT uID) { BOOL b = CString::LoadString(uID); SelfCorrect(); return b; }

// Implementation
protected:
};

/////////////////////////////////////////////////////////////////////////////

#endif // __PATHSTRING_H__
