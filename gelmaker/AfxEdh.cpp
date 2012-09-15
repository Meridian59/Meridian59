// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// AfxEdh.cpp : Additional utility functions.
//
//////////
//
// Copyright (C) Ed Halley, Inc.
//
// This unpublished source code contains trade secrets and
// copyrighted materials which are the property of Ed Halley.
// Unauthorized use, copying or distribution is a violation of
// U.S. and international laws and is strictly prohibited.
//
//////////
//

#include "StdAfx.h"

#ifndef __AFXEDH_H__
#error Must include AfxEdh.h (in your stdafx.h) to support this module.
#endif

#include "PathString.h"

/////////////////////////////////////////////////////////////////////////////

BOOL AfxIsAppActive()
{
	// Check the active window to see if it belongs to this instance.

#ifdef WIN32

	// In Win32, GetActiveWindow returns NULL if the active window
	// belongs to another thread.  Unless this application is creating
	// windows from its other threads, or we don't have any windows,
	// this is a good enough indicator of whether this app is active.

	return TRUE && ::GetActiveWindow();

#else

	// In Win16, GetActiveWindow returns a valid HWND even if the
	// active window belongs to another thread or process.  We
	// compare the HINSTANCE for the returned window against our own
	// HINSTANCE; a match indicates we are active.

	CWnd* pWnd = CWnd::GetActiveWindow();
	if (!pWnd)
		return FALSE;
	while (pWnd->GetParent())
		pWnd = pWnd->GetParent();
	if (pWnd && AfxGetInstanceHandle() ==
		(HINSTANCE)::GetWindowWord(pWnd->m_hWnd, GWW_HINSTANCE) &&
		!pWnd->IsIconic())
	{
		return TRUE;
	}

	return FALSE;	

#endif

}

/////////////////////////////////////////////////////////////////////////////

CDocument* AFXAPI AfxGetActiveDoc(CRuntimeClass* pClass /* = NULL */)
{
	// Start with the application.
	CWinApp* pApp = AfxGetApp();
	if (!pApp)
		return NULL;

	// Assume it uses a CFrameWnd as its m_pMainWnd.
	CFrameWnd* pFrame = (CFrameWnd*)AfxGetApp()->m_pMainWnd;
	if (!pFrame || !pFrame->IsKindOf(RUNTIME_CLASS(CFrameWnd)))
		return NULL;

	// If using MDI, get the active CMDIChildWnd child frame instead.
	if (pFrame->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd)))
	{
		pFrame = ((CMDIFrameWnd*)pFrame)->MDIGetActive();
		if (!pFrame || !pFrame->IsKindOf(RUNTIME_CLASS(CFrameWnd)))
			return NULL;
	}

	// Get the active document (from the active view in the frame).
	CDocument* pDoc = pFrame->GetActiveDocument();
	if (!pDoc || !pDoc->IsKindOf(RUNTIME_CLASS(CDocument)))
		return NULL;

	// If a type is specified, check it.
	if (pClass && !pDoc->IsKindOf(pClass))
		return NULL;

	// Found one.
	return pDoc;
}

/////////////////////////////////////////////////////////////////////////////

BOOL AfxSaveResourceToFile(CFile* pFile, LPCSTR pszResName,
                           LPCSTR pszResType,
                           HINSTANCE hResInstance /* = NULL */)
{
	if (!hResInstance)
		hResInstance = AfxGetResourceHandle();

	// Load the resource.
	HRSRC hrcData = ::FindResource(hResInstance, pszResName, pszResType);
	if (!hrcData)
	{
		TRACE("AfxSaveResourceToFile: resource not found\n");
		AfxThrowResourceException();
	}

	DWORD dwCount = ::SizeofResource(hResInstance, hrcData);
	ASSERT(dwCount);

	HGLOBAL hData = ::LoadResource(hResInstance, hrcData);
	if (!hData)
		AfxThrowMemoryException();

	LPCSTR pData = (LPCSTR)::LockResource(hData);
	if (!pData)
		AfxThrowMemoryException();

	pFile->Write(pData, dwCount); // throw(CFileException);

	UnlockResource(hData);
	FreeResource(hData);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

UINT AfxGetPopupMenuCmd(CMenu* pMenu, UINT uFlags, POINT pt)
{
	return AfxGetPopupMenuCmd(pMenu, uFlags, pt.x, pt.y, NULL);
}

UINT AfxGetPopupMenuCmd(CMenu* pMenu, UINT uFlags, int x, int y,
                        LPCRECT lpRect /* = NULL */)
{
	ASSERT(AfxGetMainWnd());
	if (!pMenu || !pMenu->m_hMenu)
		return 0;

	//REVIEW: Beginning with Windows95, you can add TPM_RETURNCMD
	//        to avoid all this queue-stealing hack, but this hack
	//        works regardless of OS version. TPM_RETURNCMD was not
	//        documented since the return type of TrackPopupMenu()
	//        is defined to be BOOL.

	// HACK!
	// Least harmful solution if a WM_COMMAND is somehow already in the
	// queue: don't pop up the given menu.  The user will probably think
	// it is their own fault a menu didn't pop up, and try again; the
	// existing WM_COMMAND will be processed as usual.
	//
	MSG msg;
	if (::PeekMessage(&msg, AfxGetMainWnd()->m_hWnd,
	                  WM_COMMAND, WM_COMMAND, PM_NOREMOVE))
	{
		return 0;
	}

	// Pop up the menu and steal any resulting WM_COMMAND from the queue.
	//
	HWND hwndCapture = ::GetCapture();

	pMenu->TrackPopupMenu(uFlags, x, y, AfxGetMainWnd(), lpRect);
	UINT uCmd = 0;
	if (::PeekMessage(&msg, AfxGetMainWnd()->m_hWnd,
	                  WM_COMMAND, WM_COMMAND, PM_REMOVE))
	{
		ASSERT(msg.message == WM_COMMAND);
		uCmd = msg.wParam;
	}

	if (hwndCapture)
		::SetCapture(hwndCapture);

	return uCmd;
}

/////////////////////////////////////////////////////////////////////////////

UINT AfxDoBrowseButton(CWnd* pEditCtrl, LPCTSTR pInitialDir /* = NULL */,
					   LPCTSTR pFileTypes /* = NULL */)
{
	CPathString sInitialFile;
	pEditCtrl->GetWindowText(sInitialFile);
	sInitialFile.SelfCorrect();

	// Extract filter and extensions from the AppWizard-style multi-strings.
	// For instance, pFileTypes could be directly loaded from IDR_MAINFRAME in
	// many cases. The string is separated by \n's.  The third and fourth
	// substring written by AppWizard are the browse dialog data items.
	//
	CString sFilter;
	CString sExtension;
	CString sResource = pFileTypes? pFileTypes : _T("");
	AfxExtractSubString(sFilter, sResource, 3); // "Foo Files (*.foo)|*.foo|All Files (*.*)|*.*||"
	AfxExtractSubString(sExtension, sResource, 4); // ".foo"
	if (sFilter.IsEmpty())
		sFilter = "All Files (*.*)|*.*||";
	if (sExtension.IsEmpty())
		sExtension = ".*";

	// Set up the dialog.
	CFileDialog browser(TRUE, sExtension, sInitialFile,
	                    OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_SHAREAWARE,
	                    sFilter);
	browser.m_ofn.lpstrInitialDir = pInitialDir;

	// Do the dialog.
	UINT uReturn = browser.DoModal();
	if (uReturn == IDOK)
		pEditCtrl->SetWindowText(browser.m_ofn.lpstrFile);

	return uReturn;
}

