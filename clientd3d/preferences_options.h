// Meridian 59, Copyright 1994-2025 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
#pragma once
#include <afxdlgs.h>
#include "resource.h"

class COptionsPage : public CPropertyPage
{
	DECLARE_DYNAMIC(COptionsPage)

public:
	COptionsPage();
	virtual ~COptionsPage();

	// Dialog Data
	enum { IDD = IDD_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnClassic();
	afx_msg void OnSoftware();
	afx_msg void OnMainhelp();
	afx_msg void OnRestoredefaults();
	afx_msg void OnAlwaysRun();
	afx_msg void OnDynamic();
	afx_msg void OnQuickChat();
	afx_msg void OnAttackOnTarget();
	afx_msg void OnGpuEfficiency();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
};

