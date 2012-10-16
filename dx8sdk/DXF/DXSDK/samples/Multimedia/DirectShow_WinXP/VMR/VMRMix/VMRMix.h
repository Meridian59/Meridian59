//------------------------------------------------------------------------------
// File: VMRMix.h
//
// Desc: DirectShow sample code
//       Header file and class description for CVMRMixApp
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_VMRMix_H__D7D7485E_84F1_4BE2_ACF2_569097C46073__INCLUDED_)
#define AFX_VMRMix_H__D7D7485E_84F1_4BE2_ACF2_569097C46073__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CVMRMixApp:
// See VMRMix.cpp for the implementation of this class
//

class CVMRMixApp : public CWinApp
{
public:
    CVMRMixApp();

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CVMRMixApp)
    public:
    virtual BOOL InitInstance();
    //}}AFX_VIRTUAL

// Implementation

    //{{AFX_MSG(CVMRMixApp)
        // NOTE - the ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VMRMix_H__D7D7485E_84F1_4BE2_ACF2_569097C46073__INCLUDED_)
