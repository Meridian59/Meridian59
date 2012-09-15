// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// AfxEdh.h : to be included in a project's stdafx.h for useful features
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
// This header should be included in your StdAfx.h to gain common useful
// macros and inline functions for debugging and general purpose utility.
//
// Some reusable classes in the AfxEdh library will depend on this header
// file.  Including it in your StdAfx.h will allow your whole project to use
// the common utility macros, classes and functions.
//
//////////
//

#ifndef __AFXEDH_H__
#define __AFXEDH_H__

#include <afxole.h>
#include <afxtempl.h>

/////////////////////////////////////////////////////////////////////////////

// AfxIsAppActive:
// Returns FALSE if this application is not currently active.
// An application is "active" (as defined by the operating system) if it owns
// the top-level window (no parents) which the system has last given the
// WM_ACTIVATE message.
//
BOOL AfxIsAppActive();

/////////////////////////////////////////////////////////////////////////////

// AfxGetActiveDoc:
// Returns the current active document, even if you do not have a document
// or window pointer to start from.  Note that this walks a long complicated
// chain of pointers from the globally accessible CWinApp to the document
// which MFC considers the "active" document.  This is *not recommended* if
// you have a CView pointer or a CDocument pointer at your disposal; this is
// for those rare cases you wish to access the document from CWinApp code
// or unclassed C code which does not have the CDocument pointer available.
//
// If you give no arguments, the return is NULL or a pointer to a CDocument.
// If you give a CRuntimeClass pointer (typically through a RUNTIME_CLASS()
// macro), then the active document is checked against that class, and only
// classes of that type or derived from that type are returned (other
// document types or unknown window hierarchy types will return NULL).
//
CDocument* AFXAPI AfxGetActiveDoc(CRuntimeClass* pClass = NULL);

/////////////////////////////////////////////////////////////////////////////

UINT AfxGetPopupMenuCmd(CMenu* pMenu, UINT uFlags, POINT pt);
UINT AfxGetPopupMenuCmd(CMenu* pMenu, UINT uFlags, int x, int y,
                        LPCRECT lpRect = NULL);

/////////////////////////////////////////////////////////////////////////////

UINT AfxDoBrowseButton(CWnd* pEditCtrl, LPCTSTR pInitialDir = NULL,
                       LPCTSTR pFileTypes = NULL);

/////////////////////////////////////////////////////////////////////////////

BOOL AfxSaveResourceToFile(CFile* pFile, LPCSTR pszResName,
                           LPCSTR pszResType, HINSTANCE hResInstance = NULL);

/////////////////////////////////////////////////////////////////////////////

template <class DATA>
class CStack : public CList<DATA, DATA&>
{
public:
	CStack() { ; }
	void Push(DATA d)
		{ AddHead(d); }
	DATA Pop()
		{ return RemoveHead(); }
	DATA Peek()
		{ return GetHead(); }
};

/////////////////////////////////////////////////////////////////////////////

template <class DATA>
class CQueue : public CList<DATA, DATA&>
{
public:
	CQueue() { ; }
	void Put(DATA d)
		{ AddTail(d); }
	DATA Get()
		{ return RemoveHead(); }
	DATA Peek()
		{ return GetHead(); }
};

/////////////////////////////////////////////////////////////////////////////

template <class DATA>
class CBag : public CMap<DATA,DATA&,int,int&>
{
public:
	CBag() { ; }

// Attributes
public:

// Operations
public:
	BOOL IsMember(DATA key) const
		{ int dummy; return Lookup(key, dummy); }
	void Add(DATA member)
		{ int dummy = 1; SetAt(member, dummy); }
	BOOL Remove(DATA member)
		{ return RemoveKey(member); }

	//POSITION GetStartPosition() const;
	DATA GetNext(POSITION& pos) const
		{ DATA key; int dummy; GetNextAssoc(pos, key, dummy); return key; }

// Overrides
public:

// Implementation
public:
	virtual ~CBag() { ; }
};

/////////////////////////////////////////////////////////////////////////////

//
// Some hefty debugging macros.
// Sprinkle these liberally in your code, especially in your class's
// AssertValid() override.  More details below on these macros.
//

#ifdef _DEBUG

#define ASSERT_DYNAMIC(pObj, theClass) \
                      ASSERT(pObj && pObj->IsKindOf(RUNTIME_CLASS(theClass)))

#define ASSERT_DYNAMIC_OR_NULL(pObj, theClass) \
                      ASSERT(!pObj || pObj->IsKindOf(RUNTIME_CLASS(theClass)))

#define NOTREACHED    ASSERT(0)

#define ASSERT_NODEFAULT() \
                      default: ASSERT(0); break;

#define MILEPOST(where) \
                      if (!AfxCheckMemory())                     \
                      { static BASED_CODE char* w =              \
                               "Milepost " __FILE__ " at "       \
                               #where " failed.\n";              \
					    TRACE(w);                                \
                        ASSERT(0);                               \
                      }
#else // !_DEBUG

#define ASSERT_DYNAMIC(pObj, theClass) // nothing

#define ASSERT_DYNAMIC_OR_NULL(pObj, theClass) // nothing

#define NOTREACHED // nothing

#define ASSERT_NODEFAULT() // nothing

#define MILEPOST(where) // nothing

#endif // !_DEBUG

/////////////////////////////////////////////////////////////////////////////

#endif // __AFXEDH_H__
