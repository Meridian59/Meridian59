// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// PersistFrame.h : interface of the CPersistFrame class
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

#ifndef __PERSISTFRAME_H__
#define __PERSISTFRAME_H__

/////////////////////////////////////////////////////////////////////////////

// Define CPERSISTFRAME_BASE to CFrameWnd.
// Override this default definition by adding a #define to your stdafx.h.
//
#ifndef CPERSISTFRAME_BASE
#define CPERSISTFRAME_BASE CFrameWnd
#endif

//////////

#ifndef __AFXEDH_H__
#error Must include AfxEdh.h (in your stdafx.h) to support this module.
#endif

/////////////////////////////////////////////////////////////////////////////

//
//////////
//
// Using a CPersistFrame:
//
// * Just like a CFrameWnd or CMDIFrameWnd.
//   Derive your window frame type from CPersistFrame.
//
// * When window is destroyed, the window positioning information is saved.
//   When window is re-created at a later time, the information is read to
//   restore window positioning, toolbar positioning and visibility, and
//   other application-defined options.
//
// * Optionally set a persistence name.
//   If using more than one frame window, then call SetPersistName() either
//   in your derived class constructor, in your OnCreate() handler, or by
//   by the caller shortly after constructing each frame.
//   (Later, the persistence name can be retrieved with GetPersistName().)
//
// * Optionally override OnWriteFrameInfo() and OnRestoreFrameInfo().
//   In such overrides, record or restore any desired options with code
//   similar to:
//        CString s =
//           AfxGetApp()->GetProfileString("myinfo", GetPersistName());
//
// * A handy BOOL LoadToolBar(CToolBar* pBar, UINT uResID) is provided.
//   This expects three resources (bitmap, toolbar and a string) by the
//   given uResID.  The string is of the format:
//        "menuflyby\ntooltip\ncaption"
//   See the implementation for more details.
//   Use of this member is not required for toolbar positioning persistence.
//
//////////
//

/////////////////////////////////////////////////////////////////////////////

class CPersistFrame : public CPERSISTFRAME_BASE
{
	DECLARE_DYNAMIC(CPersistFrame)
public:
	CPersistFrame();

// Attributes
private:
	CString m_sPersistName;
	BOOL m_bPreActivation;
	BOOL m_bAutoPersist;
public:
	LPCSTR GetPersistName();
	void SetPersistName(LPCSTR pszName);
	//
	BOOL IsAutoPersist() const { return m_bAutoPersist; }
	void SetAutoPersist(BOOL bAuto) { m_bAutoPersist = bAuto; }

// Operations
public:
	BOOL LoadToolBar(CToolBar* pBar, UINT uID);
	void WriteFramePlacement();
	void RestoreFramePlacement();
	void ShoveFrameOntoDesktop(int iMargins = 0); // in SM_CXBORDER multiples

// Overridables
public:
	virtual void OnWriteFrameInfo() { ; }
	virtual void OnRestoreFrameInfo() { ; }
		// To override these, access profile information with the persist
		// name as the section in the user settings profile:
		//   AfxGetApp()->GetProfileString("myinfo", GetPersistName());

// Implementation
private:
	BOOL _FindSafePersistName();
	void _DataToHexString(CString& sHex, LPVOID pvData, int cbLength);
	BOOL _HexStringToData(CString& sHex, LPVOID pvData, int cbLength);

#ifdef _DEBUG
public:
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(CPersistFrame)
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif // __PERSISTFRAME_H__
