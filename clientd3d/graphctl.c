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

// Fixed-point scale (thousandths) for the bar fill position
const long BAR_POS_SCALE = 1000;

// Used by the custom bar render
const int BAR_CORNER_RADIUS    = 3;   // Corner radius in pixels
const int BAR_GRADIENT_LIGHTEN = 70;  // Lighten the fill color at the top edge
const int BAR_GRADIENT_DARKEN  = 30;  // Darken the fill color at the bottom edge
const COLORREF BAR_NUMBER_OUTLINE = RGB(0, 0, 0);  // Outline behind the bar number for contrast
const int BAR_NUMBER_PAD    = 3;  // Gap from the bar edge to the number
const int BAR_NUMBER_GAP    = 6;  // Extra room needed past the bar to put the number outside
const int BAR_NUMBER_MARGIN = 4;  // Distance kept from the right edge

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
static void GraphCtlFillSlice(HDC hdc, const RECT *rect, long x0, long x1, COLORREF color, bool custom, int inset, int corner);
static void GraphCtlDrawNumber(HDC hdc, int value, long value_pos, const RECT *rect, bool custom);
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
/*
 * GraphCtlFillSlice:  Fill the horizontal slice [x0, x1] of a bar in the
 *   given color.  Custom bars use a vertical gradient clipped to a rounded outline.
 *   Default bars use a flat fill inset to sit inside the frame.
 */
static void GraphCtlFillSlice(HDC hdc, const RECT *rect, long x0, long x1,
                              COLORREF color, bool custom, int inset, int corner)
{
   if (x1 <= x0)
      return;

   if (!custom)
   {
      RECT fill = { x0, rect->top + inset, x1, rect->bottom - inset };
      HBRUSH brush = CreateSolidBrush(color);
      FillRect(hdc, &fill, brush);
      DeleteObject(brush);
      return;
   }

   BYTE r = GetRValue(color), g = GetGValue(color), b = GetBValue(color);
   BYTE lr = (BYTE) std::min(255, r + BAR_GRADIENT_LIGHTEN);
   BYTE lg = (BYTE) std::min(255, g + BAR_GRADIENT_LIGHTEN);
   BYTE lb = (BYTE) std::min(255, b + BAR_GRADIENT_LIGHTEN);
   BYTE dr = (BYTE) std::max(0, r - BAR_GRADIENT_DARKEN);
   BYTE dg = (BYTE) std::max(0, g - BAR_GRADIENT_DARKEN);
   BYTE db = (BYTE) std::max(0, b - BAR_GRADIENT_DARKEN);

   TRIVERTEX vert[2];
   vert[0].x = rect->left;
   vert[0].y = rect->top;
   vert[0].Red = (COLOR16) lr << 8;
   vert[0].Green = (COLOR16) lg << 8;
   vert[0].Blue = (COLOR16) lb << 8;
   vert[0].Alpha = 0;
   vert[1].x = rect->right;
   vert[1].y = rect->bottom;
   vert[1].Red = (COLOR16) dr << 8;
   vert[1].Green = (COLOR16) dg << 8;
   vert[1].Blue = (COLOR16) db << 8;
   vert[1].Alpha = 0;
   GRADIENT_RECT grect = { 0, 1 };

   // Clip the gradient to the slice within the rounded outline.  Region bounds
   // are exclusive, so extend right and bottom by 1 to reach the edges
   HRGN round = CreateRoundRectRgn(rect->left, rect->top, rect->right + 1, rect->bottom + 1,
                                   corner * 2, corner * 2);
   HRGN clip = CreateRectRgn(x0, rect->top, x1, rect->bottom);
   CombineRgn(clip, clip, round, RGN_AND);
   SelectClipRgn(hdc, clip);

   GradientFill(hdc, vert, 2, &grect, 1, GRADIENT_FILL_RECT_V);

   SelectClipRgn(hdc, NULL);
   DeleteObject(clip);
   DeleteObject(round);
}
/*****************************************************************************/
/*
 * GraphCtlDrawNumber:  Draw the bar's value at value_pos.  Custom bars add a
 *   dark outline for contrast against the fill and keep a small edge margin.
 *   Default bars draw the value as is.
 */
static void GraphCtlDrawNumber(HDC hdc, int value, long value_pos, const RECT *rect,
                               bool custom)
{
   char temp[MAXAMOUNT + 1];
   SIZE size;
   int x, y, pad = custom ? BAR_NUMBER_PAD : 1;  // Default bars keep the original 1px gap

   snprintf(temp, sizeof(temp), "%d", value);
   SetBkMode(hdc, TRANSPARENT);
   SelectObject(hdc, GetFont(FONT_STATNUM));
   GetTextExtentPoint32(hdc, temp, (int) strlen(temp), &size);

   // Put the number past the bar if it fits, otherwise inside the bar
   if (rect->right - value_pos > size.cx + (custom ? BAR_NUMBER_GAP : 0))
      x = value_pos + pad;
   else
      x = value_pos - size.cx - pad;
   if (custom && x + size.cx > rect->right - BAR_NUMBER_MARGIN)
      x = rect->right - size.cx - BAR_NUMBER_MARGIN;

   y = std::max(0L, (rect->bottom - size.cy) / 2);

   if (custom)
   {
      SetTextColor(hdc, BAR_NUMBER_OUTLINE);
      TextOut(hdc, x - 1, y, temp, (int) strlen(temp));
      TextOut(hdc, x + 1, y, temp, (int) strlen(temp));
      TextOut(hdc, x, y - 1, temp, (int) strlen(temp));
      TextOut(hdc, x, y + 1, temp, (int) strlen(temp));
   }
   SetTextColor(hdc, GetColor(COLOR_BAR4));
   TextOut(hdc, x, y, temp, (int) strlen(temp));
}
/*****************************************************************************/
/*
 * GraphCtlPaint:  Draw the graph control's bar, either the default rectangular
 *   bar or the custom rounded bar when the control sets GCS_CUSTOM and the theme
 *   supplies custom stat bars.  Resolve each color slot to a pen and brush,
 *   compute the fill position from the current value, then paint in order: the
 *   empty track, the value slice, the limit slice up to the maximum, the frame,
 *   the value number, and the slider handle for slider graphs.
 */
void GraphCtlPaint(HWND hwnd)
{
   PAINTSTRUCT ps;
   HDC hdc;
   RECT rect;
   GraphCtlStruct *info;
   HPEN pens[GRAPH_NUMCOLORS];
   HBRUSH bkgnd_brush;
   COLORREF colors[GRAPH_NUMCOLORS];
   int i, inset, corner;
   long bar_pos, limit_pos, value_pos, slider_pos;
   POINT triangle[3] = { { 0, 0}, {GRAPH_SLIDER_HEIGHT / 2, GRAPH_SLIDER_HEIGHT - 1},
			 { - GRAPH_SLIDER_HEIGHT / 2, GRAPH_SLIDER_HEIGHT - 1} };
   POINT points[3];
   bool focus, custom;


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

   bkgnd_brush = CreateSolidBrush(colors[GRAPHCOLOR_BKGND]);

   GetClientRect(hwnd, &rect);

   // Decide whether to draw the custom bar
   custom = (info->style & GCS_CUSTOM) && ThemeUsesCustomStatBars();
   // Custom draws its own outline so the fill needs no inset.  Default insets 1px inside the frame
   corner = custom ? BAR_CORNER_RADIUS : 0;
   inset  = custom ? 0 : 1;

   if (info->style & GCS_SLIDER)
   {
      rect.left += GRAPH_SIDE_BORDER;
      rect.right -= GRAPH_SIDE_BORDER;
      rect.bottom -= GRAPH_SLIDER_HEIGHT;
   }

   // Bar fill position
   if (info->min_value == info->max_value)
      bar_pos = BAR_POS_SCALE;
   else
   {
      bar_pos = (info->current_value - info->min_value) * BAR_POS_SCALE /
         (info->max_value - info->min_value);
      // Clamp in case the value sits outside the min/max range
      bar_pos = std::clamp(bar_pos, 0L, BAR_POS_SCALE);
   }
   bar_pos = bar_pos * (rect.right - rect.left - 2 * inset) / BAR_POS_SCALE + rect.left + inset;
   value_pos = bar_pos;
   slider_pos = bar_pos;

   // Fill the empty track.  The value and limit slices paint on top
   if (custom)
   {
      SelectObject(hdc, (HPEN) GetStockObject(NULL_PEN));
      SelectObject(hdc, bkgnd_brush);
      RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, corner * 2, corner * 2);
   }
   else
   {
      RECT inner = { rect.left + inset, rect.top + inset,
                     rect.right - inset, rect.bottom - inset };
      FillRect(hdc, &inner, bkgnd_brush);
   }

   GraphCtlFillSlice(hdc, &rect, rect.left + inset, bar_pos, colors[GRAPHCOLOR_BAR],
                     custom, inset, corner);

   // Draw the gap from the value up to its limit
   if (info->style & GCS_LIMITBAR && info->limit_value > info->current_value)
   {
      limit_pos = (info->limit_value - info->min_value) * BAR_POS_SCALE /
         (info->max_value - info->min_value);
      // Clamp in case the limit sits outside the min/max range
      limit_pos = std::clamp(limit_pos, 0L, BAR_POS_SCALE);
      limit_pos = limit_pos * (rect.right - rect.left - 2 * inset) / BAR_POS_SCALE + rect.left + inset;
      GraphCtlFillSlice(hdc, &rect, bar_pos, limit_pos, colors[GRAPHCOLOR_LIMITBAR],
                        custom, inset, corner);
      slider_pos = limit_pos;
   }

   // Frame outline
   SelectObject(hdc, pens[GRAPHCOLOR_FRAME]);
   SelectObject(hdc, (HBRUSH) GetStockObject(HOLLOW_BRUSH));
   if (custom)
      RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, corner * 2, corner * 2);
   else
      Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

   if (info->style & GCS_NUMBER)
      GraphCtlDrawNumber(hdc, info->current_value, value_pos, &rect, custom);

   // Slider handle, drawn only for slider graphs
   if (info->style & GCS_SLIDER)
   {
      HBRUSH bar_brush = CreateSolidBrush(colors[GRAPHCOLOR_BAR]);
      HBRUSH slider_bkgnd = CreateSolidBrush(colors[GRAPHCOLOR_SLIDERBKGND]);
      RECT sr = { rect.left - GRAPH_SIDE_BORDER, rect.bottom,
                  rect.right + GRAPH_SIDE_BORDER, rect.bottom + GRAPH_SLIDER_HEIGHT };

      // Clear the slider strip, since the background is not erased on repaint
      FillRect(hdc, &sr, slider_bkgnd);

      // Draw the slider handle
      for (i=0; i < 3; i++)
      {
         points[i].x = triangle[i].x + slider_pos;
         points[i].y = triangle[i].y + rect.bottom;
      }
      SelectObject(hdc, pens[GRAPHCOLOR_SLIDER]);
      // Fill the handle when focused, otherwise the default white fill
      SelectObject(hdc, focus ? (HGDIOBJ) bar_brush : GetStockObject(WHITE_BRUSH));
      Polygon(hdc, points, 3);

      DeleteObject(slider_bkgnd);
      DeleteObject(bar_brush);
   }

   EndPaint(hwnd, &ps);

   for (i=0; i < GRAPH_NUMCOLORS; i++)
      DeleteObject(pens[i]);
   DeleteObject(bkgnd_brush);
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
