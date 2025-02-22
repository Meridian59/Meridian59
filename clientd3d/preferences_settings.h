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
#include "client.h"

class CGeneralSettingsPage : public CPropertyPage
{
	DECLARE_DYNAMIC(CGeneralSettingsPage)

public:
	CGeneralSettingsPage();
	virtual ~CGeneralSettingsPage();
	void SaveSettings();

	// Dialog Data
	enum { IDD = IDD_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedSfx();
	afx_msg void OnBnClickedProfaneSettings();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();

	DECLARE_MESSAGE_MAP()
};