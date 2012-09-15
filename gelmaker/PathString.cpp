// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// PathString.cpp : implementation file
//
//////////
//
// Copyright (C) Ed Halley, Inc.
//
// This unpublished source code contains trade secrets and
// copyrighted materials which are the property of Ed Halley.
// Unauthorized use, copying or distribution is a violation of
// U.S. and international laws and is strictly prohibited.
//
//////////
//

#include "StdAfx.h"

#include "PathString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

// SelfCorrect:
// Adjust the pathname in any way necessary to make it a legal pathname.
// Note that it is not the responsibility of a CPathString to validate a
// pathname, just to try to be more valid to any APIs which will use it.
//
void CPathString::SelfCorrect()
{
	LPTSTR p = GetBuffer(GetLength());
	LPTSTR pWork;

	// Forward slashes, Unix style, are not allowed by some
	// Windows95 and MSDOS API.  Convert them to backslashes.
	//
	pWork = p;
	do
	{
		pWork = strchr(pWork, '/');
		if (pWork)
			*pWork = '\\';
	} while (pWork);

	ReleaseBuffer();
}

// GetFilename:
// Returns a read-only character pointer into the path at the first character
// of the base filename portion.  Note that this does not validate the
// filename (or check if it is a file or directory).  For example, if the
// string is "c:foo\bar.baz\baz.foo\blah.txt", then the returned pointer will
// be pointing at the "blah.txt" portion.
//
LPCTSTR CPathString::GetFilename() const
{
	LPCTSTR p = (LPCTSTR)*this;
	LPCTSTR pWork;

	// Find last occurring '\\' or ':' in the string.
	// That is where the filename begins.
	//
	pWork = strrchr(p, '\\');
	if (!pWork)
		pWork = strrchr(p, ':');
	if (pWork)
		pWork++;

	return pWork;
}

/////////////////////////////////////////////////////////////////////////////

// IsRelative:
// Return FALSE for an absolute path specification (relative to a root), or
// TRUE for a relative path specification (relative to a current path).
//
BOOL CPathString::IsRelative() const
{
	LPCTSTR p = (LPCTSTR)*this;

	// Handle these properly:
	//		".\foo"                   -> TRUE
	//		"c:.\foo"                 -> TRUE
	//		"f:rel\foo"               -> TRUE
	//		"f:..\rel\foo"            -> TRUE
	//		"f:rel.foo\foo"           -> TRUE
	//		"\absol.foo\foo.txt"      -> FALSE
	//		"q:\absol.foo\foo.txt"    -> FALSE
	//		"\\server\share\foo.txt"  -> FALSE

	// Ignore any leading device data up to the first ':' if any.
	//
	LPCTSTR pStart = strchr(p, ':');
	if (pStart)
		pStart++;
	if (!pStart)
		pStart = p;

	// If the first character isn't a backslash, we're relative.
	//
	if (*pStart != '\\')
		return TRUE;

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////

// FileExists:
// Return FALSE if the file is not found by the filesystem, or TRUE if there
// is a file by the given name.  Note that the current system rights apply;
// the user must have read access to the given file at the time it is
// checked.
//
BOOL CPathString::FileExists() const
{
	// Attempt to open the file for reading in a share-aware compatibility
	// mode.  This will allow us to check for a file even if another
	// application has opened it for writing.
	//
	CFile file;
	CFileException e;
	if (file.Open(*this, CFile::modeRead | CFile::shareCompat, &e))
	{
		file.Close();
		return TRUE;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
