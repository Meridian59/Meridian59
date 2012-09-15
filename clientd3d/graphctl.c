// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * graphctl.c:  Custom graph control.  Graph has a minimum and maximum value, and user
 *   can optionally click on graph to set value.
 *
 * XXX Should make number color a graph color
 */
#include "client.h"

#define GWL_GRAPHCTLMEM 0    /* Pos in extra window bytes of pointer to data */

/* Size of graphical elements */
#define GRAPH_SLIDER_HEIGHT 6
/* Leave room for slider poking over sides of graph */
#define GRAPH_SIDE_BORDER   (GRAPH_SLIDER_HEIGHT / 2)  

/* Default values */
#define GRAPHDEFMIN   0
#define GRAPHDEFMAX   100
#define GRAPHDEFVAL   50
#define GRAPHDEFLIMIT 50

/* Default colors, in order of GRAPHCOLOR_ constants */
static WORD color_defaults[GRAPH_NUMCOLORS] = {
   COLOR_WINDOWFRAME,
   COLOR_BTNTEXT,
   COLOR_WINDOW,
   COLOR_WINDOWFRAME,
   COLOR_WINDOW,
   COLOR_BTNTEXT,
};

/* Per-control information */
typedef struct {
   int min_value;      // Value at left side of graph
   int max_value;      // Value at right side of graph
   int current_value;  // Current value
   int limit_value;    // Maximum value "current_value" can attain
   COLORREF colors[GRAPH_NUMCOLORS];
   Bool button_down;
   DWORD style;
} GraphCtlStruct;

extern HPALETTE hPal;

static char *GraphCtlName = "BlakGraph";  /* Class name for graph controls; use in CreateWindowx */

/* local function prototypes */
long CALLBACK GraphCtlWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void GraphCtlLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static void GraphCtlLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
static Bool GraphCtlCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
static void GraphCtlPaint(HWND hwnd);
static void GraphCtlMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
static void GraphCtlKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
static long GraphCtlMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void GraphCtlMoveBar(HWND hwnd, GraphCtlStruct *info, int x);
/*****************************************************************************/
/*
 * GraphCtlRegister:  One-time initialization to register graph control.
 *   Return True iff successful, or class already registered.
 */
Bool GraphCtlRegister(HINSTANCE hInst)
{
   static Bool registered = False;
   WNDCLASS wc;

   if (!registered)
   {
      wc.lpfnWndProc   = GraphCtlWndProc;
      wc.cbClsExtra    = 0;
      wc.cbWndExtra    = sizeof(long);
      wc.hInstance     = hInst;
      wc.hIcon         = NULL;
      wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground = NULL;
      wc.lpszMenuName  = NULL;
      wc.lpszClassName = GraphCtlName;
      wc.style         = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW | CS_GLOBALCLASS;
      
      if (RegisterClass(&wc) != 0)
	 registered = True;
      else debug(("Registering graph class failed\n"));
   }
   return registered;
}
/*****************************************************************************/
/*
 * GraphCtlUnregister:  Remove registration info for graph control.
 *   This function should only be called after all windows that contain graph
 *   controls have been destroyed, i.e. just before the program exits.
 *   Return True iff successful, or class already registered.
 */
Bool GraphCtlUnregister(HINSTANCE hInst)
{
   return UnregisterClass(GraphCtlName, hInst);
}
/*****************************************************************************/
char *GraphCtlGetClassName(void)
{
   return GraphCtlName;
}

/*****************************************************************************/
/*
 * GraphCtlWndProc:  Main window procedure for graph controls.
 */
long CALLBACK GraphCtlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   /* Send off control-specific messages */
   if (msg > WM_USER)
      return GraphCtlMessages(hwnd, msg, wParam, lParam);

   switch(msg)
   {
   case WM_NCCREATE:
      /* Allocate memory for our per-control structure */
      SetWindowLong(hwnd, GWL_GRAPHCTLMEM, (long) SafeMalloc(sizeof(GraphCtlStruct)));
      return 1L;

   case WM_NCDESTROY:
      /* Free control's memory */
      SafeFree((void *) GetWindowLong(hwnd, GWL_GRAPHCTLMEM));
      break;

      HANDLE_MSG(hwnd, WM_CREATE, GraphCtlCreate);
      HANDLE_MSG(hwnd, WM_PAINT, GraphCtlPaint);
      HANDLE_MSG(hwnd, WM_LBUTTONDOWN, GraphCtlLButtonDown);
      HANDLE_MSG(hwnd, WM_LBUTTONDBLCLK, GraphCtlLButtonDown);
      HANDLE_MSG(hwnd, WM_LBUTTONUP, GraphCtlLButtonUp);
      HANDLE_MSG(hwnd, WM_MOUSEMOVE, GraphCtlMouseMove);
      HANDLE_MSG(hwnd, WM_KEYDOWN, GraphCtlKey);
      HANDLE_MSG(hwnd, WM_KEYUP, GraphCtlKey);

   case WM_ERASEBKGND:
      /* don't repaint background */
	  return TRUE; /* yes, we repainted the background, honest */
      break;
      
   case WM_CANCELMODE:
      if (GetCapture() == hwnd)
	     ReleaseCapture();
      break;

   case WM_GETDLGCODE:  // Handle arrow keys
      return DLGC_WANTARROWS;
      break;

   case WM_SETFOCUS:
   case WM_KILLFOCUS:
      /* Redraw so that highlight is added or removed */
      InvalidateRect(hwnd, NULL, FALSE);
      break;
      

   default: 
      return DefWindowProc(hwnd, msg, wParam, lParam);
   }

   return 0L;
}
/*****************************************************************************/
Bool GraphCtlCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
   GraphCtlStruct *info;
   int i;

   info = (GraphCtlStruct *) GetWindowLong(hwnd, GWL_GRAPHCTLMEM);

   /* Start with default values for range */
   info->min_value     = GRAPHDEFMIN;
   info->max_value     = GRAPHDEFMAX;
   info->current_value = GRAPHDEFVAL;
   info->limit_value   = GRAPHDEFLIMIT;

   info->button_down = False;

   /* Copy style bits */
   info->style = lpCreateStruct->style;

   /* Set colors to defaults */
   for (i=0; i < GRAPH_NUMCOLORS; i++)
      info->colors[i] = (COLORREF) -1;

   return True;
}
/*****************************************************************************/
void GraphCtlLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   GraphCtlStruct *info = (GraphCtlStruct *) GetWindowLong(hwnd, GWL_GRAPHCTLMEM);

   if (!(info->style & GCS_INPUT))
      return;

   /* Give ourselves the focus */
   SetFocus(hwnd);

   GraphCtlMoveBar(hwnd, info, x);

   /* Capture mouse movements until button is released */
   info->button_down = True;
   SetCapture(hwnd);
}
/*****************************************************************************/
void GraphCtlLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
   GraphCtlStruct *info = (GraphCtlStruct *) GetWindowLong(hwnd, GWL_GRAPHCTLMEM);

   if (!(info->style & GCS_INPUT))
      return;

   info->button_down = False;
   ReleaseCapture();
}
/*****************************************************************************/
void GraphCtlMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
   GraphCtlStruct *info = (GraphCtlStruct *) GetWindowLong(hwnd, GWL_GRAPHCTLMEM);

   if (!(info->style & GCS_INPUT) || !info->button_down)
      return;

   GraphCtlMoveBar(hwnd, info, x);
}
/*****************************************************************************/
void GraphCtlKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
   GraphCtlStruct *info = (GraphCtlStruct *) GetWindowLong(hwnd, GWL_GRAPHCTLMEM);

   if (!(info->style & GCS_INPUT) || !fDown)
      return;

   switch (vk)
   {
   case VK_LEFT:
   case VK_SUBTRACT:
      SendMessage(hwnd, GRPH_POSSETUSER, 0, MAKELONG(info->current_value - 1, 0));
      break;

   case VK_RIGHT:
   case VK_ADD:
      SendMessage(hwnd, GRPH_POSSETUSER, 0, MAKELONG(info->current_value + 1, 0));
      break;

   }
}
/*****************************************************************************/
void GraphCtlPaint(HWND hwnd)
{
   PAINTSTRUCT ps;
   HDC hdc;
   RECT rect, bar_rect;
   GraphCtlStruct *info;
   HPEN pens[GRAPH_NUMCOLORS];
   HBRUSH bar_brush, bkgnd_brush;
   SIZE size;
   COLORREF colors[GRAPH_NUMCOLORS];
   int i, x;
   long bar_pos, value_pos;
   POINT triangle[3] = { { 0, 0}, {GRAPH_SLIDER_HEIGHT / 2, GRAPH_SLIDER_HEIGHT - 1},
			 { - GRAPH_SLIDER_HEIGHT / 2, GRAPH_SLIDER_HEIGHT - 1} };
   POINT points[3];
   Bool focus;
   char temp[MAXAMOUNT + 1];


   info = (GraphCtlStruct *) GetWindowLong(hwnd, GWL_GRAPHCTLMEM);
   hdc = BeginPaint(hwnd, &ps);

   SelectPalette(hdc, hPal, FALSE);

   /* See if we have the focus */
   focus = GetFocus() == hwnd;

   /* Make pens for our colors.  Must do this every time in case system colors change */
   for (i=0; i < GRAPH_NUMCOLORS; i++)
   {
      /* If color is default, get system color from defaults */
      if (info->colors[i] == -1)
	 colors[i] = GetSysColor(color_defaults[i]);
      else colors[i] = info->colors[i];
      pens[i] = CreatePen(PS_SOLID, 1, colors[i]);
   }

   bar_brush = CreateSolidBrush(colors[GRAPHCOLOR_BAR]);
   bkgnd_brush = CreateSolidBrush(colors[GRAPHCOLOR_BKGND]);

   /* Draw frame */
   SelectObject(hdc, pens[GRAPHCOLOR_FRAME]);
   GetClientRect(hwnd, &rect);

   if (info->style & GCS_SLIDER)
   {
      rect.left += GRAPH_SIDE_BORDER;
      rect.right -= GRAPH_SIDE_BORDER;
      rect.bottom -= GRAPH_SLIDER_HEIGHT;
   }
   
   Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

   /* Draw bar */
   if (info->min_value == info->max_value)
      bar_pos = 1000;
   else 
   {
      bar_pos = (info->current_value - info->min_value) * 1000 / 
	 (info->max_value - info->min_value);
      // Bring within legal range, in case current_value is outside limits
      bar_pos = min(max(0, bar_pos), 1000);	    
   }
   
   bar_pos = bar_pos * (rect.right - rect.left - 2) / 1000 + rect.left + 1;  /* Skip border */ 
   value_pos = bar_pos;
   bar_rect.left   = rect.left + 1;
   bar_rect.right  = bar_pos;
   bar_rect.top    = rect.top + 1;
   bar_rect.bottom = rect.bottom - 1;
   FillRect(hdc, &bar_rect, bar_brush);

   /* Draw limit bar, if appropriate */
   if (info->style & GCS_LIMITBAR && info->limit_value > info->current_value)
   {
      HBRUSH limit_brush;

      bar_rect.left  = bar_pos;

      bar_pos = (info->limit_value - info->min_value) * 1000 / 
	 (info->max_value - info->min_value);
      bar_pos = bar_pos * (rect.right - rect.left - 2) / 1000 + rect.left + 1;

      // Bring within legal range, in case limit_value is outside limits
      bar_pos = min(max(0, bar_pos), 1000);

      bar_rect.right = bar_pos;
      limit_brush = CreateSolidBrush(colors[GRAPHCOLOR_LIMITBAR]);
      FillRect(hdc, &bar_rect, limit_brush);
      DeleteObject(limit_brush);
   }

   /* Clear area where bar isn't */
   bar_rect.left = bar_pos;
   bar_rect.right = rect.right - 1;
   FillRect(hdc, &bar_rect, bkgnd_brush);

   // Draw value of stat, if appropriate
   if (info->style & GCS_NUMBER)
   {
      sprintf(temp, "%d", info->current_value);

      SetBkMode(hdc, TRANSPARENT);
      SelectObject(hdc, GetFont(FONT_STATNUM));
      SetTextColor(hdc, GetColor(COLOR_BAR4));
      GetTextExtentPoint32(hdc, temp, strlen(temp), &size);

      // If there's room past the bar, put it there, otherwise put it in bar
      if (rect.right - value_pos > size.cx)
	 x = value_pos + 1;
      else
	 x = value_pos - size.cx - 1;

      TextOut(hdc, x, max(0, (rect.bottom - size.cy) / 2), temp, strlen(temp));
   }
   
   /* Draw slider if appropriate */
   DeleteObject(bkgnd_brush);
   if (info->style & GCS_SLIDER)
   {
      /* Erase old slider */
      bkgnd_brush = CreateSolidBrush(colors[GRAPHCOLOR_SLIDERBKGND]);
      bar_rect.bottom = rect.bottom + GRAPH_SLIDER_HEIGHT;
      bar_rect.top = rect.bottom;
      bar_rect.left = rect.left - GRAPH_SIDE_BORDER;
      bar_rect.right = rect.right + GRAPH_SIDE_BORDER;
      FillRect(hdc, &bar_rect, bkgnd_brush);
      
      /* Draw new slider */
      for (i=0; i < 3; i++)
      {
	 points[i].x = triangle[i].x + bar_pos;
	 points[i].y = triangle[i].y + rect.bottom;
      }
      SelectObject(hdc, pens[GRAPHCOLOR_SLIDER]);
      
      /* If we have the focus, fill the slider */
      if (focus)
	 SelectObject(hdc, bar_brush);
      Polygon(hdc, points, 3);

      DeleteObject(bkgnd_brush);
   }

   EndPaint(hwnd, &ps);

   for (i=0; i < GRAPH_NUMCOLORS; i++)
      DeleteObject(pens[i]);
   DeleteObject(bar_brush);
}
/*****************************************************************************/
/*
 * GraphCtlMessages:  Handle window messages specific to graph controls.
 */
long GraphCtlMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   GraphCtlStruct *info = (GraphCtlStruct *) GetWindowLong(hwnd, GWL_GRAPHCTLMEM);
   COLORREF color;
   long retval;

   switch (msg)
   {
      /* Control-specific messages */
   case GRPH_RANGESET:
      if ((int) wParam > (int) lParam)
	 return 0;

      info->min_value = (int) wParam;
      info->max_value = (int) lParam;

      /* Force repaint */
      InvalidateRect(hwnd, NULL, FALSE);
      UpdateWindow(hwnd);
      return 0;

   case GRPH_MINGET:
      return info->min_value;

   case GRPH_MAXGET:
      return info->max_value;
      
   case GRPH_POSSETUSER:
      // Verify new value
      if (lParam < info->min_value || lParam > info->max_value)
	 return -1;

      // Fall through

   case GRPH_POSSET:
      retval = info->current_value;

      /* Notify parent that value is about to change */
      SendMessage(GetParent(hwnd), GRPHN_POSCHANGING, (WPARAM) hwnd, lParam);

      info->current_value = lParam;

      /* Force repaint if value changed */
      if (retval != info->current_value)
      {
	 InvalidateRect(hwnd, NULL, FALSE);
	 UpdateWindow(hwnd);
      }
      return retval;

   case GRPH_POSGET:
      return info->current_value;

   case GRPH_LIMITSET:
      retval = info->limit_value;
      info->limit_value = lParam;

      /* Force repaint if value changed */
      if (retval != info->limit_value)
      {
	 InvalidateRect(hwnd, NULL, FALSE);
	 UpdateWindow(hwnd);
      }
      
      return retval;

   case GRPH_LIMITGET:
      return info->limit_value;

   case GRPH_COLORSET:
      if (wParam >= GRAPH_NUMCOLORS)
	 return 0L;

      color = info->colors[wParam];
      info->colors[wParam] = (COLORREF) lParam;

      /* Force repaint */
      InvalidateRect(hwnd, NULL, FALSE);
      UpdateWindow(hwnd);

      return color;
      
   case GRPH_COLORGET:
      if (wParam >= GRAPH_NUMCOLORS)
	 return 0L;
      return (long) info->colors[wParam];
   }

   return 0L;
}

/*****************************************************************************/
/* Update bar when user clicks at x-coordinate x */
void GraphCtlMoveBar(HWND hwnd, GraphCtlStruct *info, int x)
{
   int bar_value;
   RECT rect;

   /* Find out graph value of mouse position; must skip frame area */
   GetClientRect(hwnd, &rect);
   rect.left += GRAPH_SIDE_BORDER;
   rect.right -= GRAPH_SIDE_BORDER;

   if (x <= rect.left + 1)
      bar_value = info->min_value;
   else if (x >= rect.right - 1)
      bar_value = info->max_value;
   else      
      bar_value = info->min_value + (x - rect.left - 1) * 
	 (info->max_value - info->min_value) / (rect.right - rect.left - 2);

   /* Bring into range */
   bar_value = max(min(bar_value, info->max_value), info->min_value);

   SendMessage(hwnd, GRPH_POSSETUSER, 0, bar_value);
}
