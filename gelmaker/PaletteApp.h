// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// PaletteApp.h : interfaces for the CPaletteApp class
//
//////////
//
// Copyright (C) Ed Halley.
//
// This unpublished source code contains trade secrets and
// copyrighted materials which are the property of Ed Halley.
//
// Unauthorized use, copying or distribution is a violation of
// U.S. and international laws and is strictly prohibited.
//
//////////
//
// The CPaletteApp is a CWinApp with a simple palette-management
// system built in.  Derive your application from CPaletteApp, and
// call SetAppPalette() with an identifier for a DIB resource.
// This may be done during the overridden InitInstance() or at any
// other time.  When the application changes active/inactive state,
// the next call to RealizeAppPalette() will select a palette
// suitable for the foreground or background state of the
// application.
//

#ifndef __PALAPP_H__
#define __PALAPP_H__

// Define CPALETTEAPP_BASE to any suitable derivative of CWinApp.
//
#define CPALETTEAPP_BASE CWinApp

//////////

#ifndef __AFXEDH_H__
#error Must include AfxEdh.h (in your StdAfx.h) to support this module.
#endif

/////////////////////////////////////////////////////////////////////////////

/* declare */ class CDib;

/////////////////////////////////////////////////////////////////////////////

class CPaletteApp : public CPALETTEAPP_BASE
{
	DECLARE_DYNAMIC(CPaletteApp)
public:
	CPaletteApp();

// Attributes
protected:
	CPalette* m_pAppActivePalette;
	CPalette* m_pAppInactivePalette;
public:
	CPalette* GetAppPalette();
	void SetAppPalette(UINT uIdDib);
	void SetAppPalette(CDib* pDib);

// Operations
public:
	UINT RealizeAppPalette(CDC* pDC);

// Implementation
public:
	~CPaletteApp();
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//{{AFX_MSG(CPaletteApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

inline CPaletteApp* AFXAPI AfxGetPaletteApp()
	{ return (CPaletteApp*)AfxGetApp(); }

/////////////////////////////////////////////////////////////////////////////

#endif // __PALAPP_H__
