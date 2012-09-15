// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// Themap.cpp : implementation file
//

#include "stdafx.h"
#include "m59bind.h"
#include "Themap.h"
#include "Assignkey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CThemap property page

IMPLEMENT_DYNCREATE(CThemap, CPropertyPage)

CThemap::CThemap() : CPropertyPage(CThemap::IDD)
{
	//{{AFX_DATA_INIT(CThemap)
	//}}AFX_DATA_INIT
}

CThemap::~CThemap()
{
}

void CThemap::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CThemap)
	DDX_Control(pDX, IDC_MAPZOOMOUT_MOD, m_mapzoomout_mod);
	DDX_Control(pDX, IDC_MAPZOOMIN_MOD, m_mapzoomin_mod);
	DDX_Control(pDX, IDC_MAP_MOD, m_map_mod);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CThemap, CPropertyPage)
	//{{AFX_MSG_MAP(CThemap)
	ON_BN_CLICKED(IDC_MAP, OnMap)
	ON_BN_CLICKED(IDC_MAPZOOMIN, OnMapzoomin)
	ON_BN_CLICKED(IDC_MAPZOOMOUT, OnMapzoomout)
	ON_BN_CLICKED(IDC_MAP_MOD, OnMapMod)
	ON_BN_CLICKED(IDC_MAPZOOMIN_MOD, OnMapzoominMod)
	ON_BN_CLICKED(IDC_MAPZOOMOUT_MOD, OnMapzoomoutMod)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CThemap message handlers
void CThemap::UpdateDialogData()
{
  SetDlgItemText(IDC_MAP,TCMap);
  SetDlgItemText(IDC_MAPZOOMIN,TCMapzoomin);
  SetDlgItemText(IDC_MAPZOOMOUT,TCMapzoomout);

  InitModifierButton(TCMap,&m_map_mod);
  InitModifierButton(TCMapzoomin,&m_mapzoomin_mod);
  InitModifierButton(TCMapzoomout,&m_mapzoomout_mod);
}

BOOL CThemap::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

  UpdateDialogData();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CThemap::OnMap() 
{
  ProcessButtonPressed(TCMap,&m_map_mod,IDC_MAP,m_hWnd);
}

void CThemap::OnMapzoomin() 
{
  ProcessButtonPressed(TCMapzoomin,&m_mapzoomin_mod,IDC_MAPZOOMIN,m_hWnd);
}

void CThemap::OnMapzoomout() 
{
  ProcessButtonPressed(TCMapzoomout,&m_mapzoomout_mod,IDC_MAPZOOMOUT,m_hWnd);
}

void CThemap::OnMapMod() 
{
  ProcessModifierPressed(TCMap,IDC_MAP,m_hWnd);
}

void CThemap::OnMapzoominMod() 
{
  ProcessModifierPressed(TCMapzoomin,IDC_MAPZOOMIN,m_hWnd);
}

void CThemap::OnMapzoomoutMod() 
{
  ProcessModifierPressed(TCMapzoomout,IDC_MAPZOOMOUT,m_hWnd);
}
