// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// GelDoc.cpp : implementation of the CGelDoc class
//
//////////
//
// Copyright (C) The 3DO Company
//
// This unpublished source code contains trade secrets and
// copyrighted materials which are the property of The 3DO Company.
// Unauthorized use, copying or distribution is a violation of
// U.S. and international laws and is strictly prohibited.
//
//////////
//

#include "StdAfx.h"
#include "GelMaker.h"

#include "MainFrame.h"
#include "GelDoc.h"
#include "Dib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGelDoc

IMPLEMENT_DYNCREATE(CGelDoc, CDocument)

BEGIN_MESSAGE_MAP(CGelDoc, CDocument)
	//{{AFX_MSG_MAP(CGelDoc)
	ON_COMMAND(ID_GEL_IDENTITY, OnGelIdentity)
	ON_COMMAND(ID_GEL_INVERSE, OnGelInverse)
	ON_COMMAND(ID_GEL_REVIDENTITY, OnGelRevIdentity)
	ON_COMMAND(ID_GEL_HUELEFT, OnGelHueLeft)
	ON_COMMAND(ID_GEL_HUERIGHT, OnGelHueRight)
	ON_COMMAND(ID_EDIT_SELECTALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_SELECTNONE, OnEditSelectNone)
	ON_COMMAND(ID_EDIT_SELECTINVERT, OnEditSelectInvert)
	ON_COMMAND(ID_EDIT_SELECTDELTA, OnEditSelectDelta)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGelDoc construction/destruction

CGelDoc::CGelDoc()
{
	// Note: the dibs themselves are merely support, not document state.
	m_pUntransformed = new CDib(IDB_SAMPLE1);
	m_pTransformed = new CDib(IDB_SAMPLE1);
	m_pSelection = new CDib(IDB_SAMPLE1);
}

CGelDoc::~CGelDoc()
{
	delete m_pUntransformed;
	delete m_pTransformed;
	delete m_pSelection;
}

BOOL CGelDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	AfxGetPaletteApp()->SetAppPalette(IDB_SAMPLE1);

	int i;
	for (i = 0; i < 256; i++)
		m_abyTransform[i] = i;

	for (i = 0; i < 256; i++)
		m_abySelection[i] = 0xFF;
	BYTE byMask = m_pUntransformed->GetPixel(CPoint(0, 0));
	m_abySelection[byMask] = 0x00;
	ApplySelection();

	return TRUE;
}

void CGelDoc::DeleteContents() 
{
	CDocument::DeleteContents();
}

/////////////////////////////////////////////////////////////////////////////

BOOL CGelDoc::LoadSample(LPCTSTR pszFilename)
{
	CFile file;
	CDib* pDib = NULL;

	TRY
	{
		if (!file.Open(pszFilename, CFile::modeRead | CFile::shareDenyNone))
			AfxThrowResourceException();

		pDib = new CDib();
		pDib->LoadDib(&file); // throws
		pDib->DeleteDib();
		delete pDib;
		pDib = NULL;

		// If we got this far, go for the gusto and replace our sample.

		delete m_pUntransformed;
		file.Seek(0L, CFile::begin);
		m_pUntransformed = new CDib(&file);

		delete m_pTransformed;
		file.Seek(0L, CFile::begin);
		m_pTransformed = new CDib(&file);

		delete m_pSelection;
		file.Seek(0L, CFile::begin);
		m_pSelection = new CDib(&file);

		file.Close();
	}
	CATCH_ALL(e)
	{
		if (pDib)
			delete pDib;

		if (file.m_hFile)
			file.Close();

		return FALSE;
	}
	END_CATCH_ALL

	ApplyGel();
	ApplySelection();
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (pFrame)
		pFrame->UpdateSample();

	return TRUE;
}

void CGelDoc::ApplySelection()
{
	m_pSelection->CopyBits(m_pUntransformed);
	m_pSelection->ApplyVectorTransform(m_abySelection);
	UpdateAllViews(NULL);
}

void CGelDoc::ApplyGel()
{
	m_pTransformed->CopyBits(m_pUntransformed);
	m_pTransformed->ApplyVectorTransform(m_abyTransform);
	UpdateAllViews(NULL);
}

/////////////////////////////////////////////////////////////////////////////

void CGelDoc::Serialize(CArchive& ar)
{
	CFile* pFile = ar.GetFile();

	if (ar.IsStoring())
	{
		pFile->Seek(0L, CFile::begin);
		pFile->Write(m_abyTransform, sizeof(m_abyTransform));
	}
	else
	{
		int i;
		for (i = 0; i < 256; i++)
			m_abySelection[i] = 0xFF;
		BYTE byMask = m_pUntransformed->GetPixel(CPoint(0, 0));
		m_abySelection[byMask] = 0x00;

		pFile->Seek(0L, CFile::begin);
		DWORD dwSize = pFile->Read(m_abyTransform, sizeof(m_abyTransform));

		ApplyGel();

		if (sizeof(m_abyTransform) != dwSize)
			AfxThrowArchiveException(CArchiveException::generic);
	}
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void CGelDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGelDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

void CGelDoc::OnGelIdentity()
{
	int i;
	for (i = 0; i < 256; i++)
		if (m_abySelection[i])
			m_abyTransform[i] = i;

	SetModifiedFlag();
	ApplyGel();
}

void CGelDoc::OnGelInverse()
{
	CPalette* pPal = AfxGetPaletteApp()->GetAppPalette();
	if (!pPal)
		return;

	int i;
	for (i = 0; i < 256; i++)
	{
		if (m_abySelection[i])
		{
			PALETTEENTRY entry;
			BYTE k;
			pPal->GetPaletteEntries(i, 1, &entry);
			k = pPal->GetNearestPaletteIndex(
				RGB(255-entry.peRed, 255-entry.peGreen, 255-entry.peBlue));
			m_abyTransform[i] = k;
		}
	}

	SetModifiedFlag();
	ApplyGel();
}

void CGelDoc::OnGelRevIdentity() 
{
	int i;
	for (i = 0; i < 256; i++)
		if (!m_abySelection[i])
			m_abyTransform[i] = i;

	SetModifiedFlag();
	ApplyGel();
}

void CGelDoc::OnGelHueLeft() 
{
	CPalette* pPal = AfxGetPaletteApp()->GetAppPalette();
	if (!pPal)
		return;

	int i;
	for (i = 0; i < 256; i++)
	{
		if (m_abySelection[i])
		{
			PALETTEENTRY entry;
			BYTE k;
			pPal->GetPaletteEntries(i, 1, &entry);
			k = pPal->GetNearestPaletteIndex(
				RGB(entry.peGreen, entry.peBlue, entry.peRed));
			m_abyTransform[i] = k;
		}
	}

	SetModifiedFlag();
	ApplyGel();
}

void CGelDoc::OnGelHueRight() 
{
	CPalette* pPal = AfxGetPaletteApp()->GetAppPalette();
	if (!pPal)
		return;

	int i;
	for (i = 0; i < 256; i++)
	{
		if (m_abySelection[i])
		{
			PALETTEENTRY entry;
			BYTE k;
			pPal->GetPaletteEntries(i, 1, &entry);
			k = pPal->GetNearestPaletteIndex(
				RGB(entry.peBlue, entry.peRed, entry.peGreen));
			m_abyTransform[i] = k;
		}
	}

	SetModifiedFlag();
	ApplyGel();
}

/////////////////////////////////////////////////////////////////////////////

void CGelDoc::OnEditSelectAll() 
{
	memset(m_abySelection, 0xFF, sizeof(m_abySelection));
	ApplySelection();
}

void CGelDoc::OnEditSelectNone() 
{
	memset(m_abySelection, 0x00, sizeof(m_abySelection));
	ApplySelection();
}

void CGelDoc::OnEditSelectInvert() 
{
	int i;
	for (i = 0; i < 256; i++)
		m_abySelection[i] = ~m_abySelection[i];

	ApplySelection();
}

void CGelDoc::OnEditSelectDelta() 
{
	int i;
	for (i = 0; i < 256; i++)
		m_abySelection[i] = ((m_abyTransform[i] == i)? 0x00 : 0xFF);

	ApplySelection();
}
