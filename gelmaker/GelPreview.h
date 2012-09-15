// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// GelPreview.h : header file
//

/////////////////////////////////////////////////////////////////////////////

#include "DibView.h"

class CGelPreview : public CDibView
{
	DECLARE_DYNCREATE(CGelPreview)
protected:
	CGelPreview();

// Attributes
public:

// Operations
public:

// Overrides
public:
	//{{AFX_VIRTUAL(CGelPreview)
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CGelPreview();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(CGelPreview)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
