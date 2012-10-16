//-----------------------------------------------------------------------------
// File: cdevicecontrol.cpp
//
// Desc: CDeviceControl is a class that encapsulate the functionality of a
//       device control (or a callout).  CDeviceView accesses it to retrieve/
//       save information about the control.
//
// Copyright (C) 1999-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#include "common.hpp"


CDeviceControl::CDeviceControl(CDeviceUI &ui, CDeviceView &view) :
	m_ui(ui),
	m_view(view),
	m_bHighlight(FALSE),
	m_ptszCaption(NULL),
	m_dwDrawTextFlags(0),
	m_FontHeight(-1),
	m_bCalledCalcCallout(FALSE),
	m_bPlacedOnlyFirstCorner(FALSE),
	m_bInit(FALSE),
	m_dwCalloutAlign(CAF_TOPLEFT),
	m_nLinePoints(0),
	m_dwDeviceControlOffset((DWORD)-1),
	m_bOffsetAssigned(FALSE),
	m_pbmOverlay(NULL),
	m_pbmHitMask(NULL),
	m_ptszOverlayPath(NULL),
	m_bCaptionClipped(FALSE)
{
}

CDeviceControl::~CDeviceControl()
{
	DEVICEUINOTIFY uin;
	uin.from = DEVUINFROM_CONTROL;
	uin.control.pControl = (CDeviceControl *)this;
	uin.msg = DEVUINM_ONCONTROLDESTROY;
	m_ui.Notify(uin);
	if (m_ptszCaption)
		free(m_ptszCaption);
	delete m_pbmOverlay;
	delete m_ptszOverlayPath;
}

void CDeviceControl::SetCaption(LPCTSTR tszCaption, BOOL bFixed)
{
	LPTSTR tszNewCaption = NULL;

	m_bFixed = bFixed;

	if (tszCaption != NULL)
	{
		tszNewCaption = _tcsdup(tszCaption);

		if (tszNewCaption == NULL)
			return;
	}

	free(m_ptszCaption);
	m_ptszCaption = tszNewCaption;
	tszNewCaption = NULL;

	CalcCallout();
	Invalidate();
}

LPCTSTR CDeviceControl::GetCaption()
{
	return (LPCTSTR)m_ptszCaption;
}

BOOL CDeviceControl::HitControl(POINT point)
{
	return FALSE;
}

DEVCTRLHITRESULT CDeviceControl::HitTest(POINT test)
{
	if (!m_bInit)
		return DCHT_NOHIT;

	if (m_ui.InEditMode() &&
			PtInRect(&m_rectCalloutMax, test))
		return DCHT_MAXRECT;

	PrepCallout();

	if (PtInRect(&m_rectCallout, test))
		return DCHT_CAPTION;

	if (HitControl(test))
		return DCHT_CONTROL;

	return DCHT_NOHIT;
}

void CDeviceControl::Init()
{
	m_uin.from = DEVUINFROM_CONTROL;
	m_uin.control.pControl = this;

	CalcCallout();

	m_bInit = TRUE;
}

// We will have to know the view's scrolling offset to adjust the tooltip's position.
void CDeviceControl::OnMouseOver(POINT point)
{
	// Tooltip only if the callout text is clipped.
	if (m_bCaptionClipped)
	{
		TOOLTIPINITPARAM ttip;
		ttip.hWndParent = GetParent(m_view.m_hWnd);  // Parent is the page window.
		ttip.iSBWidth = 0;
		ttip.dwID = m_dwDeviceControlOffset;
		ttip.hWndNotify = m_view.m_hWnd;
		ttip.tszCaption = GetCaption();
		CFlexToolTip::UpdateToolTipParam(ttip);
	} else
		CFlexWnd::s_ToolTip.SetToolTipParent(NULL);

	m_uin.msg = DEVUINM_MOUSEOVER;
	m_ui.Notify(m_uin);
}

void CDeviceControl::OnClick(POINT point, BOOL bLeft, BOOL bDoubleClick)
{

	// If this control is not assigned, and we are in view mode, we should not do anything (highlight).
	if (!lstrcmp(m_ptszCaption, g_tszUnassignedControlCaption) && !m_ui.m_uig.InEditMode())
		return;

	m_uin.msg = bDoubleClick ? DEVUINM_DOUBLECLICK : DEVUINM_CLICK;
	m_uin.click.bLeftButton = bLeft;
	m_ui.Notify(m_uin);
}

void CDeviceControl::Unpopulate()
{
}

void CDeviceControl::Highlight(BOOL bHighlight)
{
	if (m_bHighlight == bHighlight)
		return;

	// If the callout text is the default text, no action is assigned, and we don't highlight it.
	if (!lstrcmp(m_ptszCaption, g_tszUnassignedControlCaption) && bHighlight && !m_ui.m_uig.InEditMode())
		return;

	m_bHighlight = bHighlight;

	// If the view has scrolling enabled, we need to adjust the scroll
	// bar position to make this callout visible.
	if (bHighlight)
		m_view.ScrollToMakeControlVisible(m_rectCalloutMax);

	CalcCallout();

	// We do not invalidate rectangle if we are unhighlighting.  Let CDeviceView handle that.
	if (bHighlight) Invalidate();
}

void CDeviceControl::GetInfo(GUID &rGuid, DWORD &rdwOffset)
{
	m_ui.GetDeviceInstanceGuid(rGuid);
	rdwOffset = m_dwDeviceControlOffset;
}

BOOL CDeviceControl::PrepCaption()
{
	if (m_ptszCaption != NULL)
		return TRUE;
	m_ptszCaption = _tcsdup(g_tszUnassignedControlCaption);
	return m_ptszCaption != NULL;
}

void CDeviceControl::PrepLinePoints()
{
	if (m_nLinePoints > 0)
		return;
	m_nLinePoints = 1;
	POINT pt = {0, 0};
	if (m_dwCalloutAlign & CAF_LEFT)
		pt.x = m_rectCalloutMax.left;
	if (m_dwCalloutAlign & CAF_RIGHT)
		pt.x = m_rectCalloutMax.right - 1;
	if (m_dwCalloutAlign & CAF_TOP)
		pt.y = m_rectCalloutMax.top;
	if (m_dwCalloutAlign & CAF_BOTTOM)
		pt.y = m_rectCalloutMax.bottom - 1;
	if (!(m_dwCalloutAlign & (CAF_LEFT | CAF_RIGHT)))
		pt.x = (m_rectCalloutMax.left + m_rectCalloutMax.right - 1) / 2;
	if (!(m_dwCalloutAlign & (CAF_BOTTOM | CAF_TOP)))
		pt.y = (m_rectCalloutMax.top + m_rectCalloutMax.bottom - 1) / 2;
	m_rgptLinePoint[0] = pt;
}

void CDeviceControl::PrepCallout()
{
	if (m_bCalledCalcCallout)
		return;
	CalcCallout();
}

void CDeviceControl::PrepFont()
{
	if (m_FontHeight != -1)
		return;

	HDC hDC = CreateCompatibleDC(NULL);
	if (hDC != NULL)
	{
		RECT rect = {0, 0, 500, 1};
		{
			CPaintHelper ph(m_ui.m_uig, hDC);
			ph.SetFont(UIF_CALLOUT);
			m_FontHeight = DrawText(hDC, _T("Testify"), -1, &rect, m_dwDrawTextFlags);
		}
		DeleteDC(hDC);
	}
}

void CDeviceControl::CalcCallout()
{
	m_bCalledCalcCallout = TRUE;

	RECT max = m_rectCalloutMax;
	InflateRect(&max, -1, -1);
	RECT rect = max;
	rect.bottom = rect.top + 1;

	PrepFont();

	HDC hDC = CreateCompatibleDC(NULL);

	{
		CPaintHelper ph(m_ui.m_uig, hDC);
		ph.SetFont(UIF_CALLOUT);

		// We make sure the max rect height is at least same as the font requires.
		m_dwDrawTextFlags = DT_SINGLELINE | DT_CALCRECT | DT_NOPREFIX | DT_END_ELLIPSIS | DT_EDITCONTROL;
		RECT hrect = rect;
		DrawText(hDC, m_ptszCaption, -1, &hrect, m_dwDrawTextFlags);
		if (hrect.bottom > max.bottom) max.bottom = hrect.bottom;

		m_dwDrawTextFlags = DT_WORDBREAK | DT_CALCRECT | DT_NOPREFIX | DT_END_ELLIPSIS | DT_EDITCONTROL;

		// first, drawtext/calcrect into the temporary rect
		if (!PrepCaption())
		{
			return;
		}

		int th = DrawText(hDC, m_ptszCaption, -1, &rect, m_dwDrawTextFlags);

		m_bCaptionClipped = rect.bottom > max.bottom || rect.right > max.right;  // Set clipped flag.
		
		BOOL bSingleTextLine = th <= m_FontHeight;

		if (rect.right > max.right)
		{
			bSingleTextLine = TRUE;
			rect.right = max.right;
		}

		if (bSingleTextLine)
			m_dwDrawTextFlags &= ~DT_WORDBREAK;

		m_dwDrawTextFlags &= ~DT_CALCRECT;

		RECT rect2 = rect;
		if (rect2.bottom > max.bottom)
			rect2.bottom = max.bottom;
		th = DrawText(hDC, m_ptszCaption, -1, &rect2, m_dwDrawTextFlags);
		int ith = (th / m_FontHeight) * m_FontHeight;
		rect.bottom = rect.top + ith + 1;
	}

	DeleteDC(hDC);
	hDC = NULL;

	if (rect.bottom > max.bottom)
		rect.bottom = max.bottom;

	assert(rect.right <= max.right);
	assert(rect.bottom <= max.bottom);

	PrepLinePoints();
	POINT adj = {0, 0};

	assert(rect.left == max.left);
	assert(rect.top == max.top);

	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;
	int mw = max.right - max.left;
	int mh = max.bottom - max.top;
	int dw = mw - w, dh = mh - h;
	int cx = mw / 2 + max.left, cy = mh / 2 + max.top;
	int cl = cx - w / 2, ct = cy - h / 2;

	assert(dw >= 0);
	assert(dh >= 0);

	if (m_dwCalloutAlign & CAF_RIGHT && rect.right < max.right)
		adj.x = max.right - rect.right;
	if (m_dwCalloutAlign & CAF_BOTTOM && rect.bottom < max.bottom)
		adj.y = max.bottom - rect.bottom;
	if (!(m_dwCalloutAlign & (CAF_RIGHT | CAF_LEFT)) && w < mw && rect.left != cl)
		adj.x = cl - rect.left;
	if (!(m_dwCalloutAlign & (CAF_BOTTOM | CAF_TOP)) && h < mh && rect.top != ct)
		adj.y = ct - rect.top;

	OffsetRect(&rect, adj.x, adj.y);

	InflateRect(&rect, 1, 1);

	m_rectCallout = rect;
}

BOOL CDeviceControl::DrawOverlay(HDC hDC)
{
	if (m_pbmOverlay == NULL)
		return FALSE;

	return m_pbmOverlay->Blend(hDC, m_ptOverlay);
}

void CDeviceControl::OnPaint(HDC hDC)
{
	if (!m_bInit)
		return;

	// If we are in view mode and the callout is not assigned, don't draw anything.
	if (!m_ui.m_uig.InEditMode() && !lstrcmp(m_ptszCaption, g_tszUnassignedControlCaption))
		return;

	PrepCallout();

	CPaintHelper ph(m_ui.m_uig, hDC);
	UIELEMENT eCallout = m_bHighlight ? UIE_CALLOUTHIGH : UIE_CALLOUT;

	// draw lines...
	if (m_nLinePoints > 1)
	{
		ph.SetElement(UIE_CALLOUTSHADOW);
		PolyLineArrowShadow(hDC, m_rgptLinePoint, m_nLinePoints);
		ph.SetElement(eCallout);
		PolyLineArrow(hDC, m_rgptLinePoint, m_nLinePoints);
	}

	// draw text
	ph.SetElement(eCallout);
	RECT rect = m_rectCallout;
	InflateRect(&rect, -1, -1);

	// If this control is assigned an action with DIA_FIXED (m_bFixed), use gray color for text.
	COLORREF OldColor;
	if (m_bFixed)
	{
		OldColor = ::SetTextColor(hDC, 0);  // Set an arbitrary color to find out what we are currently using.
		::SetTextColor(hDC, RGB(GetRValue(OldColor) >> 1, GetGValue(OldColor) >> 1, GetBValue(OldColor) >> 1));
	}

	if (m_ptszCaption)
		DrawText(hDC, m_ptszCaption, -1, &rect, m_dwDrawTextFlags);

	if (m_bFixed)
		::SetTextColor(hDC, OldColor);
}

void CDeviceControl::Invalidate()
{
	m_view.Invalidate();
}

void MakeRect(RECT &rect, POINT a, POINT b)
{
	rect.left = min(a.x, b.x);
	rect.right = max(a.x, b.x);
	rect.top = min(a.y, b.y);
	rect.bottom = max(a.y, b.y);
}

void CDeviceControl::PlaceCalloutMaxCorner(int nCorner, POINT point)
{
	switch (nCorner)
	{
		case 0:
			m_ptFirstCorner = point;
			m_bPlacedOnlyFirstCorner = TRUE;
			Invalidate();
			break;

		case 1:
			MakeRect(m_rectCalloutMax, m_ptFirstCorner, point);
			m_bPlacedOnlyFirstCorner = FALSE;
			if (!m_bInit)
				Init();
			else
				CalcCallout();
			Invalidate();
			break;

		default:
			assert(0);
			break;
	}
}

void CDeviceControl::SetLastLinePoint(int nPoint, POINT point, BOOL bShiftDown)
{
	if (!(nPoint >= 0 && nPoint < MAX_DEVICECONTROL_LINEPOINTS))
		return;

	// Check for SHIFT key state
	if (nPoint && bShiftDown)  // SHIFT key only makes a difference if we are setting 2nd and subsequent points.
	{
		// SHIFT key down.  Need to draw controlled line.
		if (labs(m_rgptLinePoint[nPoint-1].x - point.x) > labs(m_rgptLinePoint[nPoint-1].y - point.y))
		{
			// Wider. Draw horizontal.
			m_rgptLinePoint[nPoint].x = point.x;
			m_rgptLinePoint[nPoint].y = m_rgptLinePoint[nPoint-1].y;
		} else
		{
			// Taller. Draw vertical
			m_rgptLinePoint[nPoint].x = m_rgptLinePoint[nPoint-1].x;
			m_rgptLinePoint[nPoint].y = point.y;
		}
	} else
		m_rgptLinePoint[nPoint] = point; // SHIFT key not down.  Draw line as usual.
	m_nLinePoints = nPoint + 1;
	Invalidate();

	if (m_nLinePoints < 2)
		return;

	POINT prev = m_rgptLinePoint[m_nLinePoints - 2];

	// remove identical points
	if (point.x == prev.x && point.y == prev.y)
	{
		m_nLinePoints--;
		return;
	}
}

void PlaceRectCenter(RECT &rect, POINT point)
{
	POINT center = {
		(rect.left + rect.right) / 2,
		(rect.top + rect.bottom) / 2};

	OffsetRect(&rect, point.x - center.x, point.y - center.y);
}

void OffsetRectToWithin(RECT &rect, const RECT &bounds)
{
	POINT adj = {0, 0};

	if (rect.left < bounds.left)
		adj.x = bounds.left - rect.left;
	if (rect.right > bounds.right)
		adj.x = bounds.right - rect.right;
	if (rect.top < bounds.top)
		adj.y = bounds.top - rect.top;
	if (rect.bottom > bounds.bottom)
		adj.y = bounds.bottom - rect.bottom;

	OffsetRect(&rect, adj.x, adj.y);
}

void CDeviceControl::Position(POINT point)
{
	PlaceRectCenter(m_rectCalloutMax, point);
	RECT client;
	m_view.GetClientRect(&client);
	OffsetRectToWithin(m_rectCalloutMax, client);
	CalcCallout();
	Invalidate();
}

void CDeviceControl::ConsiderAlignment(POINT point)
{
	POINT center = {
		(m_rectCalloutMax.right + m_rectCalloutMax.left) / 2,
		(m_rectCalloutMax.bottom + m_rectCalloutMax.top) / 2};
	SIZE dim = {
		m_rectCalloutMax.right - m_rectCalloutMax.left,
		m_rectCalloutMax.bottom - m_rectCalloutMax.top};
	SIZE delta = {point.x - center.x, point.y - center.y};
	int MININ = m_FontHeight;
	SIZE in = {max(dim.cx / 4, MININ), max(dim.cy / 4, MININ)};
	DWORD align = 0;
	if (delta.cx < -in.cx)
		align |= CAF_LEFT;
	if (delta.cx > in.cx)
		align |= CAF_RIGHT;
	if (delta.cy < -in.cy)
		align |= CAF_TOP;
	if (delta.cy > in.cy)
		align |= CAF_BOTTOM;
	m_dwCalloutAlign = align;
	CalcCallout();
	Invalidate();
}

DWORD CDeviceControl::GetOffset()
{
	if (m_bOffsetAssigned)
		return m_dwDeviceControlOffset;

	return (DWORD)-1;
}

BOOL CDeviceControl::IsOffsetAssigned()
{
	return m_bOffsetAssigned;
}

void CDeviceControl::FillImageInfo(DIDEVICEIMAGEINFOW *pImgInfo)
{
	if (!pImgInfo) return;

	if (m_ptszOverlayPath != NULL)
		CopyStr(pImgInfo->tszImagePath, m_ptszOverlayPath, MAX_PATH);
	else
		wcscpy(pImgInfo->tszImagePath, L"");  // Overlay Image not yet supported

	SIZE size = {0, 0};
	if (m_pbmOverlay != NULL)
		m_pbmOverlay->GetSize(&size);
	RECT rect = {m_ptOverlay.x, m_ptOverlay.y,
		m_ptOverlay.x + size.cx, m_ptOverlay.y + size.cy};

	pImgInfo->dwFlags = DIDIFT_OVERLAY;  // This is an overlay
	pImgInfo->rcOverlay = rect;
	pImgInfo->dwObjID = GetOffset();
	pImgInfo->dwcValidPts = m_nLinePoints;
	DWORD dwPtsToCopy = m_nLinePoints > 5 ? 5 : m_nLinePoints;
	for (DWORD i = 0; i < dwPtsToCopy; ++i)
		pImgInfo->rgptCalloutLine[i] = m_rgptLinePoint[i];
	pImgInfo->rcCalloutRect = m_rectCalloutMax;
	pImgInfo->dwTextAlign = m_dwCalloutAlign;
}


BOOL CDeviceControl::IsMapped()
{
	return m_ui.IsControlMapped(this);
}

int CDeviceControl::GetControlIndex()
{
	for (int i = 0; i < m_view.GetNumControls(); i++)
		if (m_view.GetControl(i) == this)
			return i;

	return -1;
}

void CDeviceControl::SetLinePoints(int n, POINT *rgpt)
{
	assert(n >= 0 && n <= MAX_DEVICECONTROL_LINEPOINTS && rgpt);

	if (n < 0)
		n = 0;
	if (n > MAX_DEVICECONTROL_LINEPOINTS)
		n = MAX_DEVICECONTROL_LINEPOINTS;

	if (!rgpt)
		n = 0;

	m_nLinePoints = n;

	for (int i = 0; i < n; i++)
		m_rgptLinePoint[i] = rgpt[i];
}

void CDeviceControl::SetOverlayPath(LPCTSTR tszPath)
{
	if (m_ptszOverlayPath)
		free(m_ptszOverlayPath);
	m_ptszOverlayPath = NULL;

	if (tszPath)
		m_ptszOverlayPath = _tcsdup(tszPath);

	delete m_pbmOverlay;
	m_pbmOverlay = NULL;

	if (m_ptszOverlayPath)
	{
		LPDIRECT3DSURFACE8 pSurf = m_ui.m_uig.GetSurface3D();  // GetSurface3D() calls AddRef() on the surface.
		m_pbmOverlay = CBitmap::CreateViaD3DX(m_ptszOverlayPath, pSurf);
		if (pSurf)
		{
			// Release surface instance after we are done with it so we don't leak memory.
			pSurf->Release();
			pSurf = NULL;
		}
	}
}

void CDeviceControl::SetOverlayRect(const RECT &r)
{
	m_ptOverlay.x = r.left;
	m_ptOverlay.y = r.top;
}
