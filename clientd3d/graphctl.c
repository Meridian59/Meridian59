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
   bool button_down;
   DWORD style;
} GraphCtlStruct;

extern HPALETTE hPal;

static const char *GraphCtlName = "BlakGraph";  /* Class name for graph controls; use in CreateWindowx */

/* local function prototypes */
LRESULT CALLBACK GraphCtlWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void GraphCtlLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static void GraphCtlLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
static bool GraphCtlCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
static void GraphCtlPaint(HWND hwnd);
static void GraphCtlMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
static void GraphCtlKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);
static long GraphCtlMessages(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void GraphCtlMoveBar(HWND hwnd, GraphCtlStruct *info, int x);
/*****************************************************************************/
/*
 * GraphCtlRegister:  One-time initialization to register graph control.
 *   Return true iff successful, or class already registered.
 */
bool GraphCtlRegister(HINSTANCE hInst)
{
   static bool registered = false;
   WNDCLASS wc;

   if (!registered)
   {
      wc.lpfnWndProc   = GraphCtlWndProc;
      wc.cbClsExtra    = 0;
      wc.cbWndExtra    = sizeof(void *);
      wc.hInstance     = hInst;
      wc.hIcon         = NULL;
      wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
      wc.hbrBackground = NULL;
      wc.lpszMenuName  = NULL;
      wc.lpszClassName = GraphCtlName;
      wc.style         = CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW | CS_GLOBALCLASS;
      
      if (RegisterClass(&wc) != 0)
	 registered = true;
      else debug(("Registering graph class failed\n"));
   }
   return registered;
}
/*****************************************************************************/
/*
 * GraphCtlUnregister:  Remove registration info for graph control.
 *   This function should only be called after all windows that contain graph
 *   controls have been destroyed, i.e. just before the program exits.
 *   Return true iff successful, or class already registered.
 */
bool GraphCtlUnregister(HINSTANCE hInst)
{
   return UnregisterClass(GraphCtlName, hInst);
}
/*****************************************************************************/
const char *GraphCtlGetClassName(void)
{
   return GraphCtlName;
}

/*****************************************************************************/
/*
 * GraphCtlWndProc:  Main window procedure for graph controls.
 */
LRESULT CALLBACK GraphCtlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   /* Send off control-specific messages */
   if (msg > WM_USER)
      return GraphCtlMessages(hwnd, msg, wParam, lParam);

   switch(msg)
   {
   case WM_NCCREATE:
      /* Allocate memory for our per-control structure */
     SetWindowLongPtr(hwnd, GWL_GRAPHCTLMEM, (LONG_PTR) SafeMalloc(sizeof(GraphCtlStruct)));
      return 1L;

   case WM_NCDESTROY:
      /* Free control's memory */
      SafeFree((void *) GetWindowLongPtr(hwnd, GWL_GRAPHCTLMEM));
      break;

      HANDLE_MSG(hwnd, WM_CREATE, GraphCtlCreate);
      HANDLE_MSG(hwnd, WM_PAINT, GraphCtlPaint);
      HANDLE_MSG(hwnd, WM_LBUTTONDOWN, GraphCtlLButtonDown);
      HANDLE_MSG(hwnd, WM_LBUTTONDBLCLK, GraphCtlLButtonDown);
      HANDLE_MSG(hwnd, WM_LBUTTONUP, GraphCtlLButtonUp);
      HANDLE_MSG(hwnd, WM_MOUSEMOVE, GraphCtlMouseMove);
      HANDLE_MSG(hwnd, WM_KEYDOWN, GraphCtlKey);
      HANDLE_MSG(hwnd, WM_KEYUP, GraphCtlKey);

   case WM_MOUSEWHEEL:
      /* Forward wheel events to the parent so the stats area scrolls
         even when the cursor is over a graph control. */
      SendMessage(GetParent(hwnd), WM_MOUSEWHEEL, wParam, lParam);
      return 0;

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
bool GraphCtlCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
   GraphCtlStruct *info;
   int i;

   info = (GraphCtlStruct *) GetWindowLongPtr(hwnd, GWL_GRAPHCTLMEM);

   /* Start with default values for range */
   info->min_value     = GRAPHDEFMIN;
   info->max_value     = GRAPHDEFMAX;
   info->current_value = GRAPHDEFVAL;
   info->limit_value   = GRAPHDEFLIMIT;

   info->button_down = false;

   /* Copy style bits */
   info->style = lpCreateStruct->style;

   /* Set colors to defaults */
   for (i=0; i < GRAPH_NUMCOLORS; i++)
      info->colors[i] = (COLORREF) -1;

   return true;
}
/*****************************************************************************/
void GraphCtlLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   GraphCtlStruct *info = (GraphCtlStruct *) GetWindowLongPtr(hwnd, GWL_GRAPHCTLMEM);

   if (!(info->style & GCS_INPUT))
      return;

   /* Give ourselves the focus */
   SetFocus(hwnd);

   GraphCtlMoveBar(hwnd, info, x);

   /* Capture mouse movements until button is released */
   info->button_down = true;
   SetCapture(hwnd);
}
/*****************************************************************************/
void GraphCtlLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
   GraphCtlStruct *info = (GraphCtlStruct *) GetWindowLongPtr(hwnd, GWL_GRAPHCTLMEM);

   if (!(info->style & GCS_INPUT))
      return;

   info->button_down = false;
   ReleaseCapture();
}
/*****************************************************************************/
void GraphCtlMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
   GraphCtlStruct *info = (GraphCtlStruct *) GetWindowLongPtr(hwnd, GWL_GRAPHCTLMEM);

   if (!(info->style & GCS_INPUT) || !info->button_down)
      return;

   GraphCtlMoveBar(hwnd, info, x);
}
/*****************************************************************************/
void GraphCtlKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
   GraphCtlStruct *info = (GraphCtlStruct *) GetWindowLongPtr(hwnd, GWL_GRAPHCTLMEM);

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
   bool focus;
   char temp[MAXAMOUNT + 1];


   info = (GraphCtlStruct *) GetWindowLongPtr(hwnd, GWL_GRAPHCTLMEM);
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

   GetClientRect(hwnd, &rect);

   /*
    * Non-default themes: flat rounded bars with no ornate frame border.
    * Default theme: original rectangular bar with optional slider triangle.
    */
   if (config.theme != THEME_DEFAULT)
   {
      static const int CORNER = 3;

      /* Fill entire background with rounded rect */
      HPEN hNullPen = (HPEN)GetStockObject(NULL_PEN);
      SelectObject(hdc, hNullPen);
      SelectObject(hdc, bkgnd_brush);
      RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, CORNER * 2, CORNER * 2);

      /* Compute bar position */
      if (info->min_value == info->max_value)
         bar_pos = 1000;
      else
      {
         bar_pos = (info->current_value - info->min_value) * 1000 /
            (info->max_value - info->min_value);
         bar_pos = std::clamp(bar_pos, 0L, 1000L);
      }

      bar_pos = bar_pos * (rect.right - rect.left) / 1000 + rect.left;
      value_pos = bar_pos;

      /* Draw filled bar as a clipped vertical gradient (top highlight to bottom shade) */
      if (bar_pos > rect.left)
      {
         COLORREF barColor = colors[GRAPHCOLOR_BAR];
         BYTE r = GetRValue(barColor);
         BYTE g = GetGValue(barColor);
         BYTE b = GetBValue(barColor);

         /* Top edge: lighter highlight */
         BYTE lr = (BYTE)std::min(255, r + 70);
         BYTE lg = (BYTE)std::min(255, g + 70);
         BYTE lb = (BYTE)std::min(255, b + 70);
         /* Bottom edge: slightly darker shade */
         BYTE dr = (BYTE)std::max(0, r - 30);
         BYTE dg = (BYTE)std::max(0, g - 30);
         BYTE db = (BYTE)std::max(0, b - 30);

         TRIVERTEX vert[2];
         vert[0].x = rect.left;
         vert[0].y = rect.top;
         vert[0].Red   = (COLOR16)lr << 8;
         vert[0].Green = (COLOR16)lg << 8;
         vert[0].Blue  = (COLOR16)lb << 8;
         vert[0].Alpha = 0;
         vert[1].x = rect.right;
         vert[1].y = rect.bottom;
         vert[1].Red   = (COLOR16)dr << 8;
         vert[1].Green = (COLOR16)dg << 8;
         vert[1].Blue  = (COLOR16)db << 8;
         vert[1].Alpha = 0;
         GRADIENT_RECT gRect = { 0, 1 };

         /* Clip gradient to the bar fill region inside the rounded shape */
         HRGN hRound = CreateRoundRectRgn(rect.left, rect.top,
            rect.right + 1, rect.bottom + 1, CORNER * 2, CORNER * 2);
         HRGN hBar = CreateRectRgn(rect.left, rect.top, bar_pos, rect.bottom);
         CombineRgn(hBar, hBar, hRound, RGN_AND);
         SelectClipRgn(hdc, hBar);

         GradientFill(hdc, vert, 2, &gRect, 1, GRADIENT_FILL_RECT_V);

         SelectClipRgn(hdc, NULL);
         DeleteObject(hBar);
         DeleteObject(hRound);
      }

      /* Draw limit bar gradient if appropriate */
      if (info->style & GCS_LIMITBAR && info->limit_value > info->current_value)
      {
         long limit_pos;

         limit_pos = (info->limit_value - info->min_value) * 1000 /
            (info->max_value - info->min_value);
         limit_pos = std::clamp(limit_pos, 0L, 1000L);
         limit_pos = limit_pos * (rect.right - rect.left) / 1000 + rect.left;

         if (limit_pos > bar_pos)
         {
            COLORREF limColor = colors[GRAPHCOLOR_LIMITBAR];
            BYTE r = GetRValue(limColor);
            BYTE g = GetGValue(limColor);
            BYTE b = GetBValue(limColor);
            BYTE lr = (BYTE)std::min(255, r + 70);
            BYTE lg = (BYTE)std::min(255, g + 70);
            BYTE lb = (BYTE)std::min(255, b + 70);
            BYTE dr = (BYTE)std::max(0, r - 30);
            BYTE dg = (BYTE)std::max(0, g - 30);
            BYTE db = (BYTE)std::max(0, b - 30);

            TRIVERTEX lv[2];
            lv[0].x = rect.left;
            lv[0].y = rect.top;
            lv[0].Red   = (COLOR16)lr << 8;
            lv[0].Green = (COLOR16)lg << 8;
            lv[0].Blue  = (COLOR16)lb << 8;
            lv[0].Alpha = 0;
            lv[1].x = rect.right;
            lv[1].y = rect.bottom;
            lv[1].Red   = (COLOR16)dr << 8;
            lv[1].Green = (COLOR16)dg << 8;
            lv[1].Blue  = (COLOR16)db << 8;
            lv[1].Alpha = 0;
            GRADIENT_RECT lgRect = { 0, 1 };

            HRGN hRound = CreateRoundRectRgn(rect.left, rect.top,
               rect.right + 1, rect.bottom + 1, CORNER * 2, CORNER * 2);
            HRGN hLim = CreateRectRgn(bar_pos, rect.top, limit_pos, rect.bottom);
            CombineRgn(hLim, hLim, hRound, RGN_AND);
            SelectClipRgn(hdc, hLim);

            GradientFill(hdc, lv, 2, &lgRect, 1, GRADIENT_FILL_RECT_V);

            SelectClipRgn(hdc, NULL);
            DeleteObject(hLim);
            DeleteObject(hRound);
         }
      }

      /* Draw rounded outline border over the filled bar */
      SelectObject(hdc, pens[GRAPHCOLOR_FRAME]);
      SelectObject(hdc, (HBRUSH)GetStockObject(HOLLOW_BRUSH));
      RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, CORNER * 2, CORNER * 2);

      /* Draw value number */
      if (info->style & GCS_NUMBER)
      {
         snprintf(temp, sizeof(temp), "%d", info->current_value);

         SetBkMode(hdc, TRANSPARENT);

         HFONT hBarFont = CreateFont(-11, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
         HFONT hOldFont = (HFONT)SelectObject(hdc, hBarFont);
         GetTextExtentPoint32(hdc, temp, (int) strlen(temp), &size);


         if (rect.right - value_pos > size.cx + 6)
            x = value_pos + 3;
         else
            x = value_pos - size.cx - 3;

         /* Keep text from crowding the right edge of the bar */
         if (x + size.cx > rect.right - 8)
            x = rect.right - size.cx - 8;

         int ty = std::max(0L, (rect.bottom - size.cy) / 2);

         /* Draw 1px dark outline in 4 directions for contrast against bar fills */
         SetTextColor(hdc, RGB(0, 0, 0));
         TextOut(hdc, x - 1, ty, temp, (int) strlen(temp));
         TextOut(hdc, x + 1, ty, temp, (int) strlen(temp));
         TextOut(hdc, x, ty - 1, temp, (int) strlen(temp));
         TextOut(hdc, x, ty + 1, temp, (int) strlen(temp));

         SetTextColor(hdc, GetColor(COLOR_BAR4));
         TextOut(hdc, x, ty, temp, (int) strlen(temp));

         SelectObject(hdc, hOldFont);
         DeleteObject(hBarFont);
      }

      DeleteObject(bkgnd_brush);
   }
   else
   {
      /* Default theme: original rectangular bar with frame and optional slider */

      if (info->style & GCS_SLIDER)
      {
         rect.left += GRAPH_SIDE_BORDER;
         rect.right -= GRAPH_SIDE_BORDER;
         rect.bottom -= GRAPH_SLIDER_HEIGHT;
      }

      /* Draw frame */
      SelectObject(hdc, pens[GRAPHCOLOR_FRAME]);
      Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

      /* Draw bar */
      if (info->min_value == info->max_value)
         bar_pos = 1000;
      else
      {
         bar_pos = (info->current_value - info->min_value) * 1000 /
            (info->max_value - info->min_value);
         bar_pos = std::clamp(bar_pos, 0L, 1000L);
      }

      bar_pos = bar_pos * (rect.right - rect.left - 2) / 1000 + rect.left + 1;
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
         bar_pos = std::clamp(bar_pos, 0L, 1000L);

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
         snprintf(temp, sizeof(temp), "%d", info->current_value);

         SetBkMode(hdc, TRANSPARENT);
         SelectObject(hdc, GetFont(FONT_STATNUM));
         SetTextColor(hdc, GetColor(COLOR_BAR4));
         GetTextExtentPoint32(hdc, temp, (int) strlen(temp), &size);


         if (rect.right - value_pos > size.cx)
            x = value_pos + 1;
         else
            x = value_pos - size.cx - 1;

         /* Keep text from crowding the right edge of the bar */
         if (x + size.cx > rect.right - 4)
            x = rect.right - size.cx - 4;

         TextOut(hdc, x, std::max(0L, (rect.bottom - size.cy) / 2), temp, (int) strlen(temp));
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
   GraphCtlStruct *info = (GraphCtlStruct *) GetWindowLongPtr(hwnd, GWL_GRAPHCTLMEM);
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
   bar_value = std::clamp(bar_value, info->min_value, info->max_value);

   SendMessage(hwnd, GRPH_POSSETUSER, 0, bar_value);
}
