// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// m59bindDlg.h : header file
//
#include "Communication.h"
#include "Interaction.h"
#include "Movement.h"
#include "Targeting.h"
#include "Themap.h"
#include "Mouse.h"

#if !defined(AFX_M59BINDDLG_H__C3C9F389_EDA7_4D25_B35F_3B39C08B2278__INCLUDED_)
#define AFX_M59BINDDLG_H__C3C9F389_EDA7_4D25_B35F_3B39C08B2278__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CM59bindDlg dialog

class CM59bindDlg : public CDialog
{
// Construction
public:
	CM59bindDlg(CWnd* pParent = NULL);	// standard constructor
  ~CM59bindDlg();

// Dialog Data
	//{{AFX_DATA(CM59bindDlg)
	enum { IDD = IDD_M59BIND_DIALOG };
	BOOL	m_alwaysrun;
	BOOL	m_classic;
	BOOL	m_dynamic;
	BOOL	m_quickchat;
	BOOL	m_software;
	BOOL	m_attackontarget;
	BOOL	m_gpuefficiency;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CM59bindDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
  void ReadINIFile(void);
  void UpdateINIFile(void);

  // property sheet is wired to MDI child frame and is not displayed
	CPropertySheet* m_pPropSheet;

	// one page for each menu so we can initialize controls
	// using OnInitDialog
  CCommunication* m_pPageCommunication;
	CInteraction* m_pPageInteraction;
	CThemap* m_pPageMap;
	CMovement* m_pPageMovement;
	CTargeting* m_pPageTargeting;
	CMouse* m_pPageMouse;
protected:
	HICON m_hIcon;
 	CDialog* m_pModeless;

	// Generated message map functions
	//{{AFX_MSG(CM59bindDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClassic();
	afx_msg void OnSoftware();
	afx_msg void OnMainhelp();
	virtual void OnOK();
	afx_msg void OnRestoredefaults();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_M59BINDDLG_H__C3C9F389_EDA7_4D25_B35F_3B39C08B2278__INCLUDED_)
