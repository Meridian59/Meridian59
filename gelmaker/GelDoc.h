// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// GelDoc.h : interface of the CGelDoc class
//
//////////
//
// Copyright (C) The 3DO Company
//
// This unpublished source code contains trade secrets and
// copyrighted materials which are the property of The 3DO Company.
// Unauthorized use, copying or distribution is a violation of
// U.S. and international laws and is strictly prohibited.
//
//////////
//

/////////////////////////////////////////////////////////////////////////////

class CGelDoc : public CDocument
{
	DECLARE_DYNCREATE(CGelDoc)
protected:
	CGelDoc();

// Attributes
public:
	BYTE m_abyTransform[256];
	BYTE m_abySelection[256];
	class CDib* m_pUntransformed;
	class CDib* m_pTransformed;
	class CDib* m_pSelection;

// Operations
public:
	BOOL LoadSample(LPCTSTR pszFilename);
	void ApplySelection();
	void ApplyGel();

// Overrides
public:
	//{{AFX_VIRTUAL(CGelDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGelDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CGelDoc)
	afx_msg void OnGelIdentity();
	afx_msg void OnGelInverse();
	afx_msg void OnGelRevIdentity();
	afx_msg void OnGelHueLeft();
	afx_msg void OnGelHueRight();
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditSelectNone();
	afx_msg void OnEditSelectInvert();
	afx_msg void OnEditSelectDelta();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
