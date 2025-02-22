// Meridian 59, Copyright 1994-2025 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.#pragma once
#include <afxdlgs.h>
#include "resource.h"

class CProfanitySettingsDialog : public CDialog
{
	DECLARE_DYNAMIC(CProfanitySettingsDialog)

public:
	CProfanitySettingsDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CProfanitySettingsDialog();

	// Dialog Data
	enum { IDD = IDC_PROFANESETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

	DECLARE_MESSAGE_MAP()
};