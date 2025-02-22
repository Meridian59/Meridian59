// Meridian 59, Copyright 1994-2025 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// Themap.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "preferences_map.h"
#include "preferences_assignkey.h"
#include "preferences_modifier.h"
#include "preferences_common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapPage property page

IMPLEMENT_DYNCREATE(CMapPage, CPropertyPage)

CMapPage::CMapPage() : CPropertyPage(CMapPage::IDD)
{
	//{{AFX_DATA_INIT(CMapPage)
	//}}AFX_DATA_INIT
}

CMapPage::~CMapPage()
{
}

void CMapPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapPage)
	DDX_Control(pDX, IDC_MAPZOOMOUT_MOD, m_mapzoomout_mod);
	DDX_Control(pDX, IDC_MAPZOOMIN_MOD, m_mapzoomin_mod);
	DDX_Control(pDX, IDC_MAP_MOD, m_map_mod);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMapPage, CPropertyPage)
	//{{AFX_MSG_MAP(CMapPage)
	ON_BN_CLICKED(IDC_MAP, OnMap)
	ON_BN_CLICKED(IDC_MAPZOOMIN, OnMapzoomin)
	ON_BN_CLICKED(IDC_MAPZOOMOUT, OnMapzoomout)
	ON_BN_CLICKED(IDC_MAP_MOD, OnMapMod)
	ON_BN_CLICKED(IDC_MAPZOOMIN_MOD, OnMapzoominMod)
	ON_BN_CLICKED(IDC_MAPZOOMOUT_MOD, OnMapzoomoutMod)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapPage message handlers
void CMapPage::UpdateDialogData()
{
	SetDlgItemText(IDC_MAP, TCMap);
	SetDlgItemText(IDC_MAPZOOMIN, TCMapzoomin);
	SetDlgItemText(IDC_MAPZOOMOUT, TCMapzoomout);

	InitModifierButton(TCMap, &m_map_mod);
	InitModifierButton(TCMapzoomin, &m_mapzoomin_mod);
	InitModifierButton(TCMapzoomout, &m_mapzoomout_mod);
}

BOOL CMapPage::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	UpdateDialogData();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CMapPage::ProcessButtonPressed(TCHAR* TCButtonstring, CButton* modifier, int nID, HWND m_hWnd)
{
	CAssignkey dlg;
	dlg.DoModal();

	strcpy(TCButtonstring, TCNewkey);

	if (IsModifier(TCButtonstring))
	{
		// Disable Modifier button
		modifier->EnableWindow(FALSE);
	}
	else
	{
		AppendModifier(TCButtonstring, iModifier);
		// Enable Modifier button
		modifier->EnableWindow(TRUE);
	}

	SetDlgItemText(nID, TCButtonstring);

	CheckforDuplicateBindM(TCButtonstring);
}

void CMapPage::ProcessModifierPressed(TCHAR* TCString, int nID, HWND m_hWnd)
{
	CModifier dlg;

	iModifier = MODIFIER_NONE;
	auto nResponse = dlg.DoModal();
	if (nResponse == IDCANCEL)
	{
		return;
	}

	StripOffModifier(TCString);
	AppendModifier(TCString, iModifier);

	SetDlgItemText(nID, TCString);

	CheckforDuplicateBindM(TCString);
}

void CMapPage::OnMap()
{
	ProcessButtonPressed(TCMap, &m_map_mod, IDC_MAP, m_hWnd);
}

void CMapPage::OnMapzoomin()
{
	ProcessButtonPressed(TCMapzoomin, &m_mapzoomin_mod, IDC_MAPZOOMIN, m_hWnd);
}

void CMapPage::OnMapzoomout()
{
	ProcessButtonPressed(TCMapzoomout, &m_mapzoomout_mod, IDC_MAPZOOMOUT, m_hWnd);
}

void CMapPage::OnMapMod()
{
	ProcessModifierPressed(TCMap, IDC_MAP, m_hWnd);
}

void CMapPage::OnMapzoominMod()
{
	ProcessModifierPressed(TCMapzoomin, IDC_MAPZOOMIN, m_hWnd);
}

void CMapPage::OnMapzoomoutMod()
{
	ProcessModifierPressed(TCMapzoomout, IDC_MAPZOOMOUT, m_hWnd);
}
