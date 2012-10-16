//------------------------------------------------------------------------------
// File: DlgWait.h
//
// Desc: DirectShow sample code
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_DLGWAIT_H__E66757E7_0C93_448B_B402_50E8111FCD7E__INCLUDED_)
#define AFX_DLGWAIT_H__E66757E7_0C93_448B_B402_50E8111FCD7E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgWait.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgWait dialog

class CDlgWait : public CDialog
{
// Construction
public:
    CDlgWait(int nMax, CWnd* pParent = NULL);   // standard constructor

    virtual ~CDlgWait()
    {
        DestroyWindow();
    };

    void SetPos( int n);

// Dialog Data
    //{{AFX_DATA(CDlgWait)
    enum { IDD = IDD_DIALOG_PROGRESS };
    CProgressCtrl   m_Progress;
    //}}AFX_DATA


// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDlgWait)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CDlgWait)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
private:
    int m_Max;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGWAIT_H__E66757E7_0C93_448B_B402_50E8111FCD7E__INCLUDED_)
