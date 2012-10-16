//------------------------------------------------------------------------------
// File: DlgWait.cpp
//
// Desc: DirectShow sample code
//       Progress bar 'wait' dialog
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "VMRMix.h"
#include "DlgWait.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgWait dialog


CDlgWait::CDlgWait(int nMax, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgWait::IDD, pParent)
{
    m_Max = nMax;
    //{{AFX_DATA_INIT(CDlgWait)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}


void CDlgWait::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgWait)
    DDX_Control(pDX, IDC_PROGRESS, m_Progress);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgWait, CDialog)
    //{{AFX_MSG_MAP(CDlgWait)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgWait message handlers

BOOL CDlgWait::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    m_Progress.SetRange(0, (WORD) m_Max);
        
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgWait::SetPos( int n)
{
    m_Progress.SetPos( n);
}
