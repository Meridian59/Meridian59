//-----------------------------------------------------------------------------
// File: cdeviceview.h
//
// Desc: CDeviceView is a window class derived from CFlexWnd.  It represents
//       the device view window in which the device and callouts are drawn.
//       Each CDeviceView only represents one view.  A device that has more
//       than one view should have a corresponding number of CDeviceView for it.
//
// Copyright (C) 1999-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#ifdef FORWARD_DECLS


	enum DVIMAGE;

	class CDeviceView;

#define DEFAULTVIEWSBWIDTH 11


#else // FORWARD_DECLS

#ifndef __CDEVICEVIEW_H__
#define __CDEVICEVIEW_H__


enum DVIMAGE {
	DVI_IMAGE,
	DVI_THUMB,
	DVI_SELTHUMB
};


class CDeviceView : public CFlexWnd
{
private:
friend class CDeviceUI;	// CDeviceUI has exclusive right to create/destroy views
friend class CDIDeviceActionConfigPage;
	CDeviceView(CDeviceUI &ui);
	~CDeviceView();
	CDeviceUI &m_ui;

public:
	// control information
	int GetNumControls();
	CDeviceControl *GetControl(int nControl);
	CDeviceControl *GetControlFromOfs(DWORD dwOfs)
		{ return GetControl(GetIndexFromOfs(dwOfs)); }

	// text information
	int GetNumTexts();
	CDeviceViewText *GetText(int nText);

	// text addition
	CDeviceViewText *NewText();
	CDeviceViewText *AddText(
		HFONT, COLORREF, COLORREF, const RECT &, LPCTSTR text);
	CDeviceViewText *AddText(
		HFONT, COLORREF, COLORREF, const POINT &, LPCTSTR text);
	CDeviceViewText *AddWrappedLineOfText(
		HFONT, COLORREF, COLORREF, LPCTSTR text);

	void SetImage(CBitmap *&refpbm);
	void SetImagePath(LPCTSTR tszPath);

	// imaging
	CBitmap *GetImage(DVIMAGE dvi);
	
	// editing
	void Remove(CDeviceControl *pControl);
	void RemoveAll(BOOL bUser = TRUE);
	BOOL DoesCalloutOtherThanSpecifiedExistForOffset(CDeviceControl *, DWORD);
	BOOL DoesCalloutExistForOffset(DWORD);
	BOOL IsUnassignedOffsetAvailable();

	int GetViewIndex();

	int GetIndexFromOfs(DWORD dwOfs);  // For writing to INI


	void MakeMissingImages();

	CDeviceControl *NewControl();

protected:
	virtual void OnPaint(HDC hDC);
	virtual void OnMouseOver(POINT point, WPARAM fwKeys);
	virtual void OnClick(POINT point, WPARAM fwKeys, BOOL bLeft);
	virtual void OnDoubleClick(POINT point, WPARAM fwKeys, BOOL bLeft);
	virtual void OnWheel(POINT point, WPARAM wParam);
	virtual LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	// helpers
	void Unpopulate(BOOL bInternalOnly = FALSE);
	
	// images/visualization
	CBitmap *m_pbmImage, *m_pbmThumb, *m_pbmSelThumb;
	LPTSTR m_ptszImagePath;
	CBitmap *GrabViewImage();

	LPCTSTR GetImagePath() { return m_ptszImagePath; }

	// controls
	CArray<CDeviceControl *, CDeviceControl *&> m_arpControl;

	// text
	CArray<CDeviceViewText *, CDeviceViewText *&> m_arpText;
	POINT m_ptNextWLOText;

	// Special painting
	virtual void DoOnPaint(HDC hDC);
	BOOL m_bForcePaint;  // This indicates that we need painting even if GetUpdateRect returns FALSE.

	// Sort assigned for keyboard devices
	void SwapControls(int i, int j);
	void SortAssigned(BOOL bSort);
	void SortCallouts(int iStart, int iEnd);

	// editting state machine
	int m_SuperState, m_State, m_SubState;
	int m_OldSuperState, m_OldState, m_OldSubState;
	CDeviceControl *m_pControlContext;

	// scrolling (vertical only)
	BOOL m_bScrollEnable;
	int m_nScrollOffset;
	int m_nViewHeight;
public:
	void EnableScrolling() {m_bScrollEnable = TRUE;}	
	void ScrollToMakeControlVisible(const RECT &rc);
	void CalcDimensions();
private:
	void DisableScrollBar();
	void EnableScrollBar();
	CFlexScrollBar m_sb;
};


#endif //__CDEVICEVIEW_H__

#endif // FORWARD_DECLS
