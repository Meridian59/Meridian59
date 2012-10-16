//-----------------------------------------------------------------------------
// File: cdeviceview.cpp
//
// Desc: CDeviceView is a window class derived from CFlexWnd.  It represents
//       the device view window in which the device and callouts are drawn.
//       Each CDeviceView only represents one view.  A device that has more
//       than one view should have a corresponding number of CDeviceView for it.
//
// Copyright (C) 1999-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------

#include "common.hpp"


CDeviceView::CDeviceView(CDeviceUI &ui) :
	m_ui(ui),
	m_pbmImage(NULL),
	m_pbmThumb(NULL),
	m_pbmSelThumb(NULL),
	m_SuperState(0),
	m_State(0),
	m_SubState(0),
	m_OldSuperState(0),
	m_OldState(0),
	m_OldSubState(0),
	m_pControlContext(NULL),
	m_ptszImagePath(NULL),
	m_bScrollEnable(FALSE),
	m_nScrollOffset(0),
	m_nViewHeight(g_sizeImage.cy),
	m_bForcePaint(FALSE)
{
	m_ptNextWLOText.x = m_ptNextWLOText.y = 0;
}

CDeviceView::~CDeviceView()
{
	Unpopulate();
}

CDeviceControl *CDeviceView::NewControl()
{
	CDeviceControl *pControl = new CDeviceControl(m_ui, *this);
	if (!pControl)
		return NULL;
	m_arpControl.SetAtGrow(m_arpControl.GetSize(), pControl);
	return pControl;
}

void CDeviceView::Remove(CDeviceControl *pControl)
{
	if (pControl == NULL)
		return;

	int i = pControl->GetControlIndex();
	if (i < 0 || i >= GetNumControls())
	{
		assert(0);
		return;
	}

	if (pControl == m_pControlContext)
		m_pControlContext = NULL;

	if (m_arpControl[i] != NULL)
		delete m_arpControl[i];
	m_arpControl[i] = NULL;

	m_arpControl.RemoveAt(i);

	Invalidate();
}

void CDeviceView::RemoveAll(BOOL bUser)
{
	m_pControlContext = NULL;

	for (int i = 0; i < GetNumControls(); i++)
	{
		if (m_arpControl[i] != NULL)
			delete m_arpControl[i];
		m_arpControl[i] = NULL;
	}
	m_arpControl.RemoveAll();

	Invalidate();
}

void CDeviceView::Unpopulate(BOOL bInternalOnly)
{
	DisableScrollBar();

	m_bScrollEnable = FALSE;

	if (m_pbmImage != NULL)
		delete m_pbmImage;
	if (m_pbmThumb != NULL)
		delete m_pbmThumb;
	if (m_pbmSelThumb != NULL)
		delete m_pbmSelThumb;
	m_pbmImage = NULL;
	m_pbmThumb = NULL;
	m_pbmSelThumb = NULL;
	free(m_ptszImagePath);
	m_ptszImagePath = NULL;

	if (!bInternalOnly)
		RemoveAll(FALSE);

	for (int i = 0; i < m_arpText.GetSize(); i++)
	{
		if (m_arpText[i])
			delete m_arpText[i];
		m_arpText[i] = NULL;
	}
	m_arpText.RemoveAll();
}

void AssureSize(CBitmap *&pbm, SIZE to)
{
	if (!pbm)
		return;

	SIZE from;
	if (!pbm->GetSize(&from))
		return;

	if (from.cx >= to.cx && from.cy >= to.cy)
		return;

	CBitmap *nbm = CBitmap::Create(to, RGB(0,0,0));
	if (!nbm)
		return;

	HDC hDC = nbm->BeginPaintInto();
	pbm->Draw(hDC);
	nbm->EndPaintInto(hDC);

	delete pbm;
	pbm = nbm;
	nbm = NULL;
}

CBitmap *CDeviceView::GrabViewImage()
{
	CBitmap *pbm = CBitmap::Create(GetClientSize(), RGB(0, 0, 0), NULL);
	if (!pbm)
		return NULL;
	HDC hDC = pbm->BeginPaintInto();
	if (!hDC)
	{
		delete pbm;
		return NULL;
	}

	OnPaint(hDC);

	pbm->EndPaintInto(hDC);

	return pbm;
}

void CDeviceView::MakeMissingImages()
{
//	if (m_pbmImage)
//		AssureSize(m_pbmImage, g_sizeImage);

	if (m_pbmThumb == NULL)
	{
		if (m_pbmImage)
			m_pbmThumb = m_pbmImage->CreateResizedTo(g_sizeThumb);
		else
		{
			CBitmap *pbmImage = GrabViewImage();
			if (pbmImage)
			{
				AssureSize(pbmImage, g_sizeImage);
				m_pbmThumb = pbmImage->CreateResizedTo(g_sizeThumb);
			}
			delete pbmImage;
		}
	}

	if (m_pbmThumb == NULL)
		return;

	if (m_pbmSelThumb == NULL)
	{
		m_pbmSelThumb = m_pbmThumb->Dup();
		if (m_pbmSelThumb != NULL)
		{
			HDC hDC = m_pbmSelThumb->BeginPaintInto();
			{
				CPaintHelper ph(m_ui.m_uig, hDC);
				ph.SetPen(UIP_SELTHUMB);
				ph.Rectangle(0, 0, g_sizeThumb.cx, g_sizeThumb.cy, UIR_OUTLINE);
			}
			m_pbmSelThumb->EndPaintInto(hDC);
		}
	}
}

void CDeviceView::OnPaint(HDC hDC)
{
	HDC hBDC = NULL, hODC = NULL;
	CBitmap *pbm = NULL;

	if (!InRenderMode())
	{
		hODC = hDC;
		pbm = CBitmap::Create(GetClientSize(), RGB(0, 0, 0), hDC);
		if (pbm != NULL)
		{
			hBDC = pbm->BeginPaintInto();
			if (hBDC != NULL)
				hDC = hBDC;
		}
	}

	// Black-fill first
	SIZE fillsz = GetClientSize();
	RECT fillrc = {0, 0, fillsz.cx, fillsz.cy};
	FillRect(hDC, &fillrc, (HBRUSH)GetStockObject(BLACK_BRUSH));

	if (m_pbmImage != NULL)
		m_pbmImage->Blend(hDC);

	BOOL bScroll = m_bScrollEnable && m_sb.m_hWnd;
	int sdc = 0;
	if (bScroll)
	{
		sdc = SaveDC(hDC);
		OffsetViewportOrgEx(hDC, 0, -m_nScrollOffset + g_iListHeaderHeight, NULL);
	}
	else
	if (m_bScrollEnable)
	{
		sdc = SaveDC(hDC);
		OffsetViewportOrgEx(hDC, 0, g_iListHeaderHeight, NULL);
	}

	int miny = 0 + m_nScrollOffset;
	int maxy = g_sizeImage.cy + m_nScrollOffset;

	int t, nt = GetNumTexts();
	for (t = 0; t < nt; t++)
	{
		CDeviceViewText *pText = m_arpText[t];
		if (pText != NULL &&
			!(pText->GetMinY() > maxy || pText->GetMaxY() < miny))
				pText->OnPaint(hDC);
	}

	BOOL bCFGUIEdit = m_ui.m_uig.InEditMode();
	BOOL bEitherEditMode = bCFGUIEdit;

	int c, nc = GetNumControls();
	for (c = 0; c < nc; c++)
		if (m_arpControl[c] != NULL && m_arpControl[c]->HasOverlay() &&
		    (m_arpControl[c]->IsHighlighted()
				)
				&& (bEitherEditMode || m_arpControl[c]->IsMapped()))
			m_arpControl[c]->DrawOverlay(hDC);
	for (c = 0; c < nc; c++)
	{
		CDeviceControl *pControl = m_arpControl[c];
		if (pControl != NULL && (bEitherEditMode || pControl->IsMapped()) &&
		    !(pControl->GetMinY() > maxy || pControl->GetMaxY() < miny))
			pControl->OnPaint(hDC);
	}

	if (bScroll || m_bScrollEnable)
	{
		RestoreDC(hDC, sdc);
		sdc = 0;
	}

	// Black fill the top portion if this is a list view
	if (bScroll)
	{
		GetClientRect(&fillrc);
		fillrc.bottom = g_iListHeaderHeight;
		FillRect(hDC, &fillrc, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}

	// Print out the headers
	TCHAR tszHeader[MAX_PATH];
	// Control column
	if (m_arpText.GetSize())
	{
		CPaintHelper ph(m_ui.m_uig, hDC);
		ph.SetElement(UIE_CALLOUT);

		for (int i = 0; i < 2; i++)
		{
			// Check if there are two columns, break out the 2nd iteration if not 2 columns.
			if (i == 1 && !(GetNumControls() > 1 &&
			    m_arpControl[0]->GetCalloutMaxRect().top == m_arpControl[1]->GetCalloutMaxRect().top))
				break;

			RECT rcheader;
			if (m_arpText.GetSize())
			{
				rcheader = m_arpText[i]->GetRect();
				rcheader.bottom -= rcheader.top;
				rcheader.top = 0;
				LoadString(g_hModule, IDS_LISTHEADER_CTRL, tszHeader, MAX_PATH);
				DrawText(hDC, tszHeader, -1, &rcheader, DT_LEFT|DT_NOPREFIX|DT_CALCRECT);
				if (rcheader.right > m_arpText[i]->GetRect().right)
					rcheader.left -= rcheader.right - m_arpText[i]->GetRect().right;
				DrawText(hDC, tszHeader, -1, &rcheader, DT_LEFT|DT_NOPREFIX);

				// Action column
				rcheader = m_arpControl[i]->GetCalloutMaxRect();
				rcheader.bottom -= rcheader.top;
				rcheader.top = 0;
				LoadString(g_hModule, IDS_LISTHEADER_ACTION, tszHeader, MAX_PATH);
				DrawText(hDC, tszHeader, -1, &rcheader, DT_CENTER|DT_NOPREFIX);
			}
		}
	}


	if (!InRenderMode())
	{
		if (pbm != NULL)
		{
			if (hBDC != NULL)
			{
				pbm->EndPaintInto(hBDC);
				pbm->Draw(hODC);
			}
			delete pbm;
		}
	}
}

int CDeviceView::GetNumControls()
{
	return m_arpControl.GetSize();
}

CDeviceControl *CDeviceView::GetControl(int nControl)
{
	if (nControl >= 0 && nControl < GetNumControls())
		return m_arpControl[nControl];
	else
		return NULL;
}

CBitmap *CDeviceView::GetImage(DVIMAGE dvi)
{
	switch (dvi)
	{
		case DVI_IMAGE: return m_pbmImage;
		case DVI_THUMB: return m_pbmThumb;
		case DVI_SELTHUMB: return m_pbmSelThumb;

		default:
			return NULL;
	}
}

void CDeviceView::OnMouseOver(POINT point, WPARAM wParam)
{
	if (m_bScrollEnable && m_sb.m_hWnd)
		point.y += m_nScrollOffset;


	// Check if we are over a control
	POINT adjPt = point;
	if (m_bScrollEnable) adjPt.y -= g_iListHeaderHeight;
	int c, nc = GetNumControls();
	for (c = 0; c < nc; c++)
		if (m_arpControl[c] != NULL && m_arpControl[c]->HitTest(adjPt) != DCHT_NOHIT)
		{
			m_arpControl[c]->OnMouseOver(adjPt);
			return;
		}

	// Check if we are over a viewtext
	nc = GetNumTexts();
	for (c = 0; c < nc; c++)
		if (m_arpText[c] != NULL && m_arpText[c]->HitTest(adjPt) != DCHT_NOHIT)
		{
			m_arpText[c]->OnMouseOver(adjPt);
			return;
		}

	CFlexWnd::s_ToolTip.SetEnable(FALSE);

	DEVICEUINOTIFY uin;
	uin.msg = DEVUINM_MOUSEOVER;
	uin.from = DEVUINFROM_VIEWWND;
	uin.mouseover.point = point;
	m_ui.Notify(uin);
}

void CDeviceView::OnClick(POINT point, WPARAM wParam, BOOL bLeft)
{
	if (m_bScrollEnable && m_sb.m_hWnd)
		point.y += m_nScrollOffset;


	POINT adjPt = point;
	if (m_bScrollEnable) adjPt.y -= g_iListHeaderHeight;
	int c, nc = GetNumControls();
	for (c = 0; c < nc; c++)
		// adjPt is the adjust click point for scrolling list view
		if (m_arpControl[c] != NULL && m_arpControl[c]->HitTest(adjPt) != DCHT_NOHIT)
		{
			m_arpControl[c]->OnClick(adjPt, bLeft);
			return;
		}

	{
		for (c = 0; c < GetNumTexts(); ++c)
			if (m_arpControl[c] != NULL && m_arpText[c] != NULL)
			{
				RECT rc = m_arpText[c]->GetRect();
				if (PtInRect(&rc, adjPt))
				{
					m_arpControl[c]->OnClick(adjPt, bLeft);
					return;
				}
			}
	}


	// Send notification
	DEVICEUINOTIFY uin;
	uin.msg = DEVUINM_CLICK;
	uin.from = DEVUINFROM_VIEWWND;
	uin.click.bLeftButton = bLeft;
	m_ui.Notify(uin);
}

void CDeviceView::OnDoubleClick(POINT point, WPARAM wParam, BOOL bLeft)
{
	if (m_bScrollEnable && m_sb.m_hWnd)
		point.y += m_nScrollOffset;

	POINT adjPt = point;
	if (m_bScrollEnable) adjPt.y -= g_iListHeaderHeight;
	int c, nc = GetNumControls();
	for (c = 0; c < nc; c++)
		if (m_arpControl[c] != NULL && m_arpControl[c]->HitTest(adjPt) != DCHT_NOHIT)
		{
			m_arpControl[c]->OnClick(adjPt, bLeft, TRUE);
			return;
		}

	for (c = 0; c < GetNumTexts(); ++c)
		if (m_arpControl[c] != NULL && m_arpText[c] != NULL)
		{
			RECT rc = m_arpText[c]->GetRect();
			if (PtInRect(&rc, adjPt))
			{
				m_arpControl[c]->OnClick(adjPt, bLeft, TRUE);
				return;
			}
		}

	DEVICEUINOTIFY uin;
	uin.msg = DEVUINM_DOUBLECLICK;
	uin.from = DEVUINFROM_VIEWWND;
	uin.click.bLeftButton = bLeft;
	m_ui.Notify(uin);
}

void CDeviceView::OnWheel(POINT point, WPARAM wParam)
{
	if (!m_bScrollEnable) return;

	if (m_sb.GetMin() == m_sb.GetMax()) return;

	int nPage = MulDiv(m_sb.GetPage(), 9, 10) >> 1;  // Half a page at a time

	if ((int)wParam >= 0)
		m_sb.AdjustPos(-nPage);
	else
		m_sb.AdjustPos(nPage);

	m_nScrollOffset = m_sb.GetPos();
	Invalidate();
}


BOOL CDeviceView::DoesCalloutExistForOffset(DWORD dwOfs)
{
	return DoesCalloutOtherThanSpecifiedExistForOffset(NULL, dwOfs);
}

BOOL CDeviceView::DoesCalloutOtherThanSpecifiedExistForOffset(CDeviceControl *pOther, DWORD dwOfs)
{
	int nc = GetNumControls();
	for (int i = 0; i < nc; i++)
	{
		CDeviceControl *pControl = GetControl(i);
		if (pControl == NULL || pControl == pOther)
			continue;
		if (!pControl->IsOffsetAssigned())
			continue;
		if (pControl->GetOffset() == dwOfs)
			return TRUE;
	}
	return FALSE;
}

// This function returns the index of a control with the specified offset
int CDeviceView::GetIndexFromOfs(DWORD dwOfs)
{
	for (int i = 0; i < GetNumControls(); ++i)
		if (m_arpControl[i]->GetOffset() == dwOfs)
			return i;

	return -1;
}


int CDeviceView::GetViewIndex()
{
	return m_ui.GetViewIndex(this);
}


BOOL CDeviceView::IsUnassignedOffsetAvailable()
{
	DIDEVOBJSTRUCT os;

	HRESULT hr = FillDIDeviceObjectStruct(os, m_ui.m_lpDID);
	if (FAILED(hr))
		return FALSE;

	if (os.nObjects < 1)
		return FALSE;

	assert(os.pdoi);
	if (!os.pdoi)
		return FALSE;

	for (int i = 0; i < os.nObjects; i++)
	{
		const DIDEVICEOBJECTINSTANCEW &o = os.pdoi[i];

		if (!DoesCalloutExistForOffset(o.dwOfs))
			return TRUE;
	}

	return FALSE;
}

CDeviceViewText *CDeviceView::AddText(
	HFONT f, COLORREF t, COLORREF b, const RECT &r, LPCTSTR text)
{
	CDeviceViewText *pText = NewText();
	if (!pText)
		return NULL;

	pText->SetLook(f, t, b);
	pText->SetRect(r);
	pText->SetText(text);

	return pText;
}

CDeviceViewText *CDeviceView::AddText(
	HFONT f, COLORREF t, COLORREF b, const POINT &p, LPCTSTR text)
{
	CDeviceViewText *pText = NewText();
	if (!pText)
		return NULL;

	pText->SetLook(f, t, b);
	pText->SetPosition(p);
	pText->SetTextAndResizeTo(text);

	return pText;
}

CDeviceViewText *CDeviceView::AddWrappedLineOfText(
	HFONT f, COLORREF t, COLORREF b, LPCTSTR text)
{
	CDeviceViewText *pText = NewText();
	if (!pText)
		return NULL;

	pText->SetLook(f, t, b);
	pText->SetPosition(m_ptNextWLOText);
	pText->SetTextAndResizeToWrapped(text);
	
	m_ptNextWLOText.y += pText->GetHeight();

	return pText;
}

CDeviceViewText *CDeviceView::NewText()
{
	CDeviceViewText *pText = new CDeviceViewText(m_ui, *this);
	if (!pText)
		return NULL;
	m_arpText.SetAtGrow(m_arpText.GetSize(), pText);
	return pText;
}

int CDeviceView::GetNumTexts()
{
	return m_arpText.GetSize();
}

CDeviceViewText *CDeviceView::GetText(int nText)
{
	if (nText < 0 || nText >= GetNumTexts())
		return NULL;
	return m_arpText[nText];
}

void CDeviceView::SetImage(CBitmap *&refpbm)
{
	delete m_pbmImage;
	m_pbmImage = refpbm;
	refpbm = NULL;
	MakeMissingImages();
	Invalidate();
}

void CDeviceView::SetImagePath(LPCTSTR tszPath)
{
	if (m_ptszImagePath)
		free(m_ptszImagePath);
	m_ptszImagePath = NULL;

	if (tszPath)
		m_ptszImagePath = _tcsdup(tszPath);
}

void CDeviceView::CalcDimensions()
{
	// go through all texts and controls to find the max y coord
	int max = g_sizeImage.cy - g_iListHeaderHeight;
	int i = 0;
	for (; i < GetNumTexts(); i++)
	{
		CDeviceViewText *pText = GetText(i);
		if (!pText)
			continue;
		int ty = pText->GetMaxY();
		if (ty > max)
			max = ty;
	}
	for (i = 0; i < GetNumControls(); i++)
	{
		CDeviceControl *pControl = GetControl(i);
		if (!pControl)
			continue;
		int cy = pControl->GetMaxY();
		if (cy > max)
			max = cy;
	}

	// set
	m_nViewHeight = max;
	m_nScrollOffset = 0;

	// enable scrollbar if view height more than window size
	if (m_nViewHeight > g_sizeImage.cy - g_iListHeaderHeight)
		EnableScrollBar();
}

void CDeviceView::DisableScrollBar()
{
	if (!m_sb.m_hWnd)
		return;

	m_sb.Destroy();
}

void CDeviceView::EnableScrollBar()
{
	if (m_sb.m_hWnd)
		return;

	FLEXSCROLLBARCREATESTRUCT cs;
	cs.dwSize = sizeof(cs);
	cs.dwFlags = FSBF_VERT;
	cs.min = 0;
	cs.max = m_nViewHeight;
	cs.page = g_sizeImage.cy - g_iListHeaderHeight;
	cs.pos = m_nScrollOffset;
	cs.hWndParent = m_hWnd;
	cs.hWndNotify = m_hWnd;
	RECT rect = {g_sizeImage.cx - DEFAULTVIEWSBWIDTH, g_iListHeaderHeight, g_sizeImage.cx, g_sizeImage.cy};
	cs.rect = rect;
	cs.bVisible = TRUE;
	m_sb.SetColors(
		m_ui.m_uig.GetBrushColor(UIE_SBTRACK),
		m_ui.m_uig.GetBrushColor(UIE_SBTHUMB),
		m_ui.m_uig.GetPenColor(UIE_SBBUTTON));
	m_sb.Create(&cs);
}

LRESULT CDeviceView::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_PAINT:
			m_bForcePaint = TRUE;
			return CFlexWnd::WndProc(hWnd, msg, wParam, lParam);

		case WM_FLEXVSCROLL:
		{
			int code = (int)wParam;
			CFlexScrollBar *pSB = (CFlexScrollBar *)lParam;
			if (!pSB)
				return 0;

			int nLine = 5;
			int nPage = MulDiv(pSB->GetPage(), 9, 10);

			switch (code)
			{
				case SB_LINEUP: pSB->AdjustPos(-nLine); break;
				case SB_LINEDOWN: pSB->AdjustPos(nLine); break;
				case SB_PAGEUP: pSB->AdjustPos(-nPage); break;
				case SB_PAGEDOWN: pSB->AdjustPos(nPage); break;
				case SB_THUMBTRACK: pSB->SetPos(pSB->GetThumbPos()); break;
			}

			m_nScrollOffset = pSB->GetPos();

			Invalidate();
			return 0;
		}

		case WM_FLEXHSCROLL:
			assert(0);
		default:
			return CFlexWnd::WndProc(hWnd, msg, wParam, lParam);
	}
}

void CDeviceView::ScrollToMakeControlVisible(const RECT &rc)
{
	RECT viewrc;

	if (!m_bScrollEnable)
		return;

	GetClientRect(&viewrc);
	viewrc.bottom -= g_iListHeaderHeight;
	viewrc.top += m_nScrollOffset;
	viewrc.bottom += m_nScrollOffset;

	// If scroll enabled, we scroll the view to make the control visible if not already so.
	if (m_bScrollEnable && m_sb.m_hWnd &&
	    !(viewrc.left <= rc.left &&
	      viewrc.right >= rc.right &&
	      viewrc.top <= rc.top &&
	      viewrc.bottom >= rc.bottom))
	{
		// If the callout is below the view window, scroll so it shows up at the bottom of the window.
		if (viewrc.bottom < rc.bottom)
			m_sb.SetPos(m_sb.GetPos() + rc.bottom - viewrc.bottom);
		else
			m_sb.SetPos(rc.top);
		m_nScrollOffset = m_sb.GetPos();
		Invalidate();
	}
}

void CDeviceView::SwapControls(int i, int j)
{
	RECT rect;
	CDeviceControl *pTmpControl;
	CDeviceViewText *pTmpViewText;

	pTmpControl = m_arpControl[i];
	m_arpControl[i] = m_arpControl[j];
	m_arpControl[j] = pTmpControl;
	pTmpViewText = m_arpText[i];
	m_arpText[i] = m_arpText[j];
	m_arpText[j] = pTmpViewText;
	// Swap the rect back so everything will display properly.
	rect = m_arpControl[i]->GetCalloutMaxRect();
	m_arpControl[i]->SetCalloutMaxRect(m_arpControl[j]->GetCalloutMaxRect());
	m_arpControl[j]->SetCalloutMaxRect(rect);
	rect = m_arpText[i]->GetRect();
	m_arpText[i]->SetRect(m_arpText[j]->GetRect());
	m_arpText[j]->SetRect(rect);
	// Exchange the text rect width, so the correct width stays with the correct text.
	RECT rc1 = m_arpText[i]->GetRect();
	RECT rc2 = m_arpText[j]->GetRect();
	// Store rc1's new width first
	int iTempWidth = rc1.right - (rc2.right - rc2.left);
	rc2.left = rc2.right - (rc1.right - rc1.left);  // Adjust rc2's width
	rc1.left = iTempWidth;  // Adjust rc1's width
	m_arpText[i]->SetRect(rc1);
	m_arpText[j]->SetRect(rc2);
}

// Implements a simple selection sort algorithm to sort the control array and viewtext array.
// - iStart is the starting index, inclusive.
// - iEnd is the last index, exclusive.
void CDeviceView::SortCallouts(int iStart, int iEnd)
{
	for (int i = iStart; i < iEnd - 1; ++i)
	{
		DWORD dwSmallestOfs = m_arpControl[i]->GetOffset();
		int iSmallestIndex = i;
		for (int j = i + 1; j < iEnd; ++j)
			if (m_arpControl[j]->GetOffset() < dwSmallestOfs)
			{
				dwSmallestOfs = m_arpControl[j]->GetOffset();
				iSmallestIndex = j;
			}
		// Swap the smallest element with i-th element.
		if (iSmallestIndex != i)
			SwapControls(i, iSmallestIndex);
	}
}

void CDeviceView::SortAssigned(BOOL bSort)
{
	// If less than 2 controls, no need for sorting.
	if (m_arpControl.GetSize() < 2)
		return;

	int iCalloutX[2] = {m_arpControl[0]->GetMinX(), m_arpControl[1]->GetMinX()};  // Callout X for the two columns

	// Sort the text array and control array.
	if (bSort)
	{
		// First move all the assigned controls to the first n elements.
		int iNextAssignedWriteIndex = 0;
		for (int i = 0; i < m_arpControl.GetSize(); ++i)
			if (m_arpControl[i]->HasAction())
			{
				// Swap the controls
				SwapControls(i, iNextAssignedWriteIndex);
				++iNextAssignedWriteIndex;  // Increment the write index
			}

		// Sort the two parts now
		SortCallouts(0, iNextAssignedWriteIndex);
		SortCallouts(iNextAssignedWriteIndex, m_arpControl.GetSize());
	} else
		SortCallouts(0, m_arpControl.GetSize());
}

void CDeviceView::DoOnPaint(HDC hDC)
{
	// Paint only if we have an update region.
	if (GetUpdateRect(m_hWnd, NULL, FALSE) || m_bForcePaint)
		OnPaint(hDC);
	m_bForcePaint = FALSE;
}
