// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * lagbox.c:  Visual Latency Meter
 */

#include "client.h"

/***************************************************************************/

//ASSUME: Only one lagbox can be created.
//
static WNDPROC pfnDefLagboxProc = NULL;
static HWND hwndLagbox = NULL;
static object_node* pobjLagbox = NULL;
static DWORD dwLagboxLatency = 100;
static int nLastLagboxMetric = 0;

static char pszLagboxDescription[255] = "";
static char pszLagboxGuild[255] = "";
static char pszLagboxURL[MAX_URL] = "";

extern HPALETTE hPal;

static char szTooltip[80];
static bool bTracking = false;
static object_node lagbox;
static bool latencyDialogActive = false;

static const int updateToolTipEventId = 101;
static const int updateToolTipInterval = 1000;

/* local function prototypes */
static LRESULT CALLBACK Lagbox_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

/****************************************************************************/

// Number of latency metric strings (IDS_LATENCY0 + i).
//
#define MAXLATENCYMETRICS 9

DWORD Lagbox_GetLatency(void)
{
   return dwLagboxLatency;
}

static DWORD s_adwLatencyMetric[MAXLATENCYMETRICS] =
{
	// Roundtrip latency measurement, in milliseconds.
	// Note I've purposely set it so that you
	// should rarely have a "very good connection" and
	// only in deadly lag should you have a "very poor connection".
	//
	100, 250, 500, 750, 1000, 2000, 4000, 8000, 0x70000000
};

static DWORD s_adwLatencySpin[MAXLATENCYMETRICS] =
{
	// Frame-to-frame animation period, in milliseconds.
	55, 110, 165, 220, 275, 330, 385, 440, 495
};

static BYTE s_abyLatencyTranslate[MAXLATENCYMETRICS][2] =
{
	// Stoplight Order (go-green to halt-red and worse)
	(BYTE)XLAT_GRAYTOBGREEN,	(BYTE)XLAT_GRAYTOBGREEN,
	(BYTE)XLAT_GRAYTOBGREEN,	(BYTE)XLAT_GRAYTOGOLD,
	(BYTE)XLAT_GRAYTOGOLD,		(BYTE)XLAT_GRAYTOGOLD,
	(BYTE)XLAT_GRAYTOGOLD,		(BYTE)XLAT_GRAYTOORANGE,
	(BYTE)XLAT_GRAYTOORANGE,	(BYTE)XLAT_GRAYTOORANGE,
	(BYTE)XLAT_GRAYTOORANGE,	(BYTE)XLAT_GRAYTORED,
	(BYTE)XLAT_GRAYTORED,		(BYTE)XLAT_GRAYTORED,
	(BYTE)XLAT_GRAYTORED,		(BYTE)XLAT_GRAYTOPURPLE,
	(BYTE)XLAT_GRAYTOPURPLE,	(BYTE)XLAT_GRAYTOPURPLE,

};

/****************************************************************************/

// LagboxCreate:  Create the lagbox control.
//
BOOL Lagbox_Create()
{
	//REVIEW: shouldn't proceed if hwndLagbox != NULL

	LoadString(hInst, IDS_LAGBOXSUBNAME, pszLagboxGuild, sizeof(pszLagboxGuild));
	LoadString(hInst, IDS_LAGBOXDESC, pszLagboxDescription, sizeof(pszLagboxDescription));
	LoadString(hInst, IDS_LAGBOXURL, pszLagboxURL, sizeof(pszLagboxURL));

    pobjLagbox = ObjectGetBlank();
	if (!pobjLagbox)
	{
		debug(("Lagbox_Create: failed ObjectGetBlank()\n"));
		return FALSE;
	}
	pobjLagbox->icon_res = LAGBOXICON_RSC;
	pobjLagbox->name_res = LAGBOXNAME_RSC;
	pobjLagbox->normal_animate.animation = ANIMATE_CYCLE;
	pobjLagbox->normal_animate.period = s_adwLatencyMetric[0];
	pobjLagbox->normal_animate.tick = s_adwLatencyMetric[0];
	pobjLagbox->normal_animate.group = 1;
	pobjLagbox->normal_animate.group_low = 1;
	pobjLagbox->normal_animate.group_high = 1;
	pobjLagbox->flags = OF_DITHERTRANS;

	dwLagboxLatency = s_adwLatencyMetric[0];

	hwndLagbox = CreateWindow("button", "", WS_CHILD | BS_OWNERDRAW, 
		0, 0, 0, 0,
		hMain, (HMENU)IDS_LATENCY0, hInst, NULL);
	if (!hwndLagbox)
	{
		debug(("Lagbox_Create: failed CreateWindow()\n"));
		ObjectDestroyAndFree(pobjLagbox);
		pobjLagbox = NULL;
		return FALSE;
	}

	pfnDefLagboxProc = SubclassWindow(hwndLagbox, Lagbox_WndProc);
	TooltipAddWindowCallback(hwndLagbox, hInst);

	Lagbox_Reposition();
	Lagbox_Update(0);

	return TRUE;
}

/****************************************************************************/

// Lagbox_Destroy:  Destroy the lagbox control.
//
void Lagbox_Destroy()
{
	DestroyWindow(hwndLagbox);
	hwndLagbox = NULL;

	ObjectDestroyAndFree(pobjLagbox);
	pobjLagbox = NULL;
}

/****************************************************************************/

// Lagbox_Reposition:  Move lagbox to appropriate location around toolbar.
//
void Lagbox_Reposition()
{
	RECT rcToolbar;

	if (hwndLagbox == NULL)
		return;

	ToolbarGetUnionRect(&rcToolbar);
	if (IsRectEmpty(&rcToolbar))
	{
		// a small square where the toolbar would start
		MoveWindow(hwndLagbox, TOOLBAR_X, TOOLBAR_Y, 20, 20, TRUE);
		ShowWindow(hwndLagbox, config.lagbox? SW_NORMAL : SW_HIDE);
		InvalidateRect(hwndLagbox, NULL, FALSE);
	}
	else
	{
		// a square the height of a button to the right of the visible toolbar
		MoveWindow(hwndLagbox,
			rcToolbar.right + TOOLBAR_SEPARATOR_WIDTH, rcToolbar.top,
			TOOLBAR_BUTTON_HEIGHT /* not a typo */, TOOLBAR_BUTTON_HEIGHT,
			TRUE);
		ShowWindow(hwndLagbox, config.lagbox? SW_NORMAL : SW_HIDE);
		InvalidateRect(hwndLagbox, NULL, FALSE);
	}
}

/****************************************************************************/

void Lagbox_GetRect(LPRECT lpRect)
{
	if (!lpRect)
		return;

	memset(lpRect, 0, sizeof(*lpRect));
	if (config.lagbox && hwndLagbox && IsWindowVisible(hwndLagbox))
	{
		GetWindowRect(hwndLagbox, lpRect);
		ScreenToClient(hMain, (LPPOINT)lpRect);
		ScreenToClient(hMain, (LPPOINT)lpRect+1);
	}
}

/****************************************************************************/

int Lagbox_FindLatencyMetric(DWORD dwLatency)
{
	int iMetric = MAXLATENCYMETRICS-1;
	int i;

	for (i = 0; i < MAXLATENCYMETRICS; i++)
	{
		if (dwLatency <= s_adwLatencyMetric[i])
		{
			iMetric = i;
			break;
		}
	}

	return iMetric;
}

/****************************************************************************/

// Lagbox_Update:  Recompute the animation speed and color.  Does not flip frames.
//
void Lagbox_Update(DWORD dwLatency)
{
	int iMetric;

	if (pobjLagbox == NULL)
		return;

	if (dwLatency != 0)
		dwLagboxLatency = dwLatency;

	iMetric = Lagbox_FindLatencyMetric(dwLagboxLatency);

	pobjLagbox->normal_animate.period = s_adwLatencySpin[iMetric];
	if (pobjLagbox->normal_animate.tick > pobjLagbox->normal_animate.period)
		pobjLagbox->normal_animate.tick = pobjLagbox->normal_animate.period;

	pobjLagbox->translation = s_abyLatencyTranslate[iMetric][0];
	pobjLagbox->normal_translation = s_abyLatencyTranslate[iMetric][1];

	if (hwndLagbox && (nLastLagboxMetric != iMetric))
		InvalidateRect(hwndLagbox, NULL, FALSE);

	nLastLagboxMetric = iMetric;
}

void Lagbox_Animate(int dt)
{
	if (config.spinning_cube)
	{
		bool bNeedRedraw = true;
		if (!latencyDialogActive)
		{
			// Animate by spinning the latency box.
			bNeedRedraw = AnimateObject(pobjLagbox, dt);
		}

		if (bNeedRedraw)
			InvalidateRect(hwndLagbox, NULL, FALSE);
	}
}

/****************************************************************************/
// Lagbox_UpdateLatencyText: Set the current tooltip and dialog latency text
//
LRESULT Lagbox_UpdateLatencyText()
{
	if (TooltipGetControl() == NULL)
		return 1;

	int iMetric = Lagbox_FindLatencyMetric(dwLagboxLatency);
	char szMetric[80];
	char szFormat[80];

	// Format the metric of the latency into a tooltip string.
	// Example:  "very good connection: approx. 0.073sec latency"

	LoadString(hInst, IDS_LATENCY0 + iMetric, szTooltip, sizeof(szTooltip));
	strcat(szTooltip, ": ");

	LoadString(hInst, IDS_LATENCYMETRIC, szFormat, sizeof(szFormat));
	sprintf(szMetric, szFormat, dwLagboxLatency);
	strcat(szTooltip, szMetric);

	TOOLINFO ti = { 0 };
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hwndLagbox;
	ti.uId = (UINT_PTR)hwndLagbox;
	ti.lpszText = (LPSTR)szTooltip;

	SendMessage(TooltipGetControl(), TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);

	if(latencyDialogActive)
	{
		SetDialogFixedString(szTooltip);
	}	
	
	return 1;
}

// Lagbox_Command:  Show our look-at information.
//
void Lagbox_Command(HWND hWnd, int id, HWND hwndCtrl, UINT uNotify)
{
	// Make a temporary copy of the lagbox to view close up.
	// This allows it to spin freely and not affect the real lagbox.

	if (!pobjLagbox)
		return;

	latencyDialogActive = true;

	SetDescParams(hWnd, DESC_NONE);
	DisplayDescription(pobjLagbox,
		0/*flags*/,
		pszLagboxDescription, 
		szTooltip,
		pszLagboxURL,
		ITEM_RARITY_GRADE_NORMAL);
	SetDescParams(hWnd, 0);

	latencyDialogActive = false;
}

BOOL Lagbox_OnDrawItem(HWND hWnd, const DRAWITEMSTRUCT *lpdis)
{
	RECT rcWindow;
	AREA areaWindow;
	POINT ptWindowOrg;

	switch (lpdis->itemAction)
	{
	case ODA_SELECT:
	case ODA_DRAWENTIRE:
		SelectPalette(lpdis->hDC, hPal, FALSE);
		RealizePalette(lpdis->hDC);

		GetWindowRect(lpdis->hwndItem, &rcWindow);
		ptWindowOrg.x = rcWindow.left;
		ptWindowOrg.y = rcWindow.top;
		ScreenToClient(hMain, &ptWindowOrg);

		areaWindow.x = 0;
		areaWindow.y = 0;
		areaWindow.cx = rcWindow.right-rcWindow.left;
		areaWindow.cy = rcWindow.bottom-rcWindow.top;

		OffscreenWindowBackground(NULL,
			ptWindowOrg.x, ptWindowOrg.y, areaWindow.cx, areaWindow.cy);
		DrawObject(lpdis->hDC, pobjLagbox, pobjLagbox->animate->group,
			TRUE, &areaWindow, NULL, 0, 0, 0, TRUE);

		return TRUE;
	}

	return FALSE;
}

/****************************************************************************/

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	Lagbox_UpdateLatencyText();
}

LRESULT CALLBACK Lagbox_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MSG msg;
	msg.hwnd = hWnd;
	msg.message = uMsg;
	msg.wParam = wParam;
	msg.lParam = lParam;
	TooltipForwardMessage(&msg);

	switch (uMsg)
	{
	case WM_ERASEBKGND:
		// don't erase the background
		return TRUE; // yes, we erased the background, honest
	case WM_NOTIFY:
	{
		NMHDR* pHdr = (NMHDR*)lParam;
		if (pHdr && pHdr->code == TTN_NEEDTEXT)
			Lagbox_UpdateLatencyText();
		else
			return CallWindowProc(pfnDefLagboxProc, hWnd, uMsg, wParam, lParam);
	}
	break;
	case WM_MOUSEMOVE:
		if (!bTracking)
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.hwndTrack = hwndLagbox;
			tme.dwFlags = TME_LEAVE;
			TrackMouseEvent(&tme);

			SetTimer(hwndLagbox, updateToolTipEventId, updateToolTipInterval,
				(TIMERPROC)TimerProc);

			bTracking = true;
		}
		break;
	case WM_RBUTTONDOWN:
		Lagbox_Command(hWnd, IDS_LATENCY0, hWnd, 0);
		return TRUE;

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		return TRUE;

	case WM_DESTROY:
		KillTimer(hWnd, updateToolTipEventId);
		bTracking = false;
		latencyDialogActive = false;
	   break;
	case WM_MOUSELEAVE:
		if (!latencyDialogActive)
		{
			KillTimer(hWnd, updateToolTipEventId);
			bTracking = false;
		}
	   break;

	default:
		return CallWindowProc(pfnDefLagboxProc, hWnd, uMsg, wParam, lParam);
	}

	return 0;
}
