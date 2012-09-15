// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * annotate.c:  Handle map annotations.
 *
 * Annotations are stored in an array in the room structure.  Each annotation consists of an
 * (x, y) location in the room (in FINENESS units) and a string.  If this string has a null
 * character in its first position, then it is unused.
 *
 * Annotations are displayed as tooltips on the map.
 */

#include "client.h"

static HWND hAnnotateDialog;

extern room_type current_room;

static BOOL CALLBACK MapAnnotationDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
/*****************************************************************************/
/*
 * MapAnnotationsInitialize:  Set up tooltips for map annotations.
 */
void MapAnnotationsInitialize(void)
{
   int i;
   TOOLINFO ti;
   HWND hToolTips;

   // Create tooltips for map annotations
   hToolTips = TooltipGetControl();
   ti.cbSize = sizeof(TOOLINFO); 
   ti.uFlags = 0; 
   ti.hwnd = hMain; 

   for (i=0; i < MAX_ANNOTATIONS; i++)
   {
     // Set up tooltip for annotation
     ti.uId = i;
     ti.lpszText = LPSTR_TEXTCALLBACK; 
     SendMessage(hToolTips, TTM_ADDTOOL, 0, (LPARAM) &ti);
   }
}
/*****************************************************************************/
/*
 * MapAnnotationGetText:  Send annotation text to tooltip.
 */
void MapAnnotationGetText(TOOLTIPTEXT *ttt)
{
  int index;

//   if (!MapVisible())
//      return;

  index = ttt->hdr.idFrom;

  if (index < 0 || index >= MAX_ANNOTATIONS)
  {
     debug (("MapAnnotationGetText got out of range index %d\n", index));
     return;
  }

  if (current_room.annotations[index].text[0] != 0)
  {
    ttt->hinst = hInst;
    ttt->lpszText = current_room.annotations[index].text;
  }
}
/*****************************************************************************/
/*
 * MapMoveAnnotations:  Recompute tooltip rectangles for map annotations.
 */
void MapMoveAnnotations( MapAnnotation *annotations, int x, int y, float scale, Bool bMiniMap )
{
   int i;
   TOOLINFO ti;
   HWND hToolTips;
   AREA view;

   if( !bMiniMap )
		CopyCurrentView(&view);
   else
		CopyCurrentAreaMiniMap( &view );

   hToolTips = TooltipGetControl();
   ti.cbSize = sizeof(TOOLINFO); 
   ti.uFlags = 0; 
   ti.hwnd = hMain; 

   for (i=0; i < MAX_ANNOTATIONS; i++)
   {
     if (annotations[i].text[0] == 0)
       continue;

     // Set up tooltip for annotation
     ti.rect.left = view.x + x + (int) ((annotations[i].x - MAP_ANNOTATION_SIZE / 2) * scale);
     ti.rect.top  = view.y + y + (int) ((annotations[i].y - MAP_ANNOTATION_SIZE / 2) * scale);

     ti.rect.right  = ti.rect.left + (int) (MAP_ANNOTATION_SIZE * scale);
     ti.rect.bottom = ti.rect.top  + (int) (MAP_ANNOTATION_SIZE * scale);

     // Clip tooltip rectangle to graphics view window
     ti.rect.left = max(ti.rect.left, view.x);
     ti.rect.top = max(ti.rect.top, view.y);
     ti.rect.right = min(ti.rect.right, view.x + view.cx);
     ti.rect.bottom = min(ti.rect.bottom, view.y + view.cy);     

     ti.uId = i;
     ti.lpszText = LPSTR_TEXTCALLBACK; 
     SendMessage(hToolTips, TTM_SETTOOLINFO, 0, (LPARAM) &ti);
   }
}
/*****************************************************************************/
/*
 * MapAnnotationClick:  Mouse button pressed on main window while map is up;
 *   edit annotation that was clicked on, or add a new annotation.
 *		Modified by ajw 5/19/97. Annotations made possible only in MiniMap.
 */
void MapAnnotationClick(int x, int y)
{
  int i, index;
  MapAnnotation *a;
  Bool existed;    // True if editing an existing annotation
  
//  int stretchfactor = config.large_area ? 2 : 1;

//  // Map doesn't change with large or small graphics area; undo strechfactor correction
//  x *= stretchfactor;
//  y *= stretchfactor;

  MapScreenToRoom( &x, &y, TRUE );

  // See if close to an annotation
  index = -1;
  existed = False;
  for (i=0; i < MAX_ANNOTATIONS; i++)
  {
     a = &current_room.annotations[i];
     if (a->text[0] == 0)
       continue;

     if (a->x <= x + MAP_ANNOTATION_SIZE / 2 && a->x >= x - MAP_ANNOTATION_SIZE / 2 &&
	 a->y <= y + MAP_ANNOTATION_SIZE / 2 && a->y >= y - MAP_ANNOTATION_SIZE / 2)
       {
	 existed = True;
	 index = i;
	 break;
       }
  }

  if (index == -1)
  {
    // Look for a free annotation slot
    for (i=0; i < MAX_ANNOTATIONS; i++)
    {
      if (current_room.annotations[i].text[0] == 0)
      {
	index = i;
	break;
      }
    }
      if (index == -1)
      {
	GameMessagePrintf(GetString(hInst, IDS_ANNOTATIONSFULL), MAX_ANNOTATIONS);
	return;
      }
  }

  if (DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_ANNOTATE), hMain,
		     MapAnnotationDialogProc, (LPARAM) index) == IDOK)
    {
      current_room.annotations_changed = True;
      if (!existed)
	{
	  current_room.annotations[index].x = x;
	  current_room.annotations[index].y = y;
	}
    }

  TooltipReset();

  RedrawAll();    // In case we added or removed an annotation
}
/*****************************************************************************/
/*
 * MapAnnotationDialogProc:  Dialog procedure for editing annotation text.
 *   lParam of WM_INITDIALOG is index of annotation.
 */
BOOL CALLBACK MapAnnotationDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   static int index;
   HWND hEdit;

   switch (message)
   {
   case WM_INITDIALOG:
      index = lParam;

      hEdit = GetDlgItem(hDlg, IDC_ANNOTATE);
      Edit_LimitText(hEdit, MAX_ANNOTATION_LEN - 1);
      Edit_SetText(hEdit, current_room.annotations[index].text);

      CenterWindow(hDlg, GetParent(hDlg));

      hAnnotateDialog = hDlg;
      return TRUE;

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDC_DELETE:
	 current_room.annotations[index].text[0] = 0;
	 EndDialog(hDlg, IDOK);
	 return TRUE;
	
      case IDOK:
	 hEdit = GetDlgItem(hDlg, IDC_ANNOTATE);
	 Edit_GetText(hEdit, current_room.annotations[index].text, MAX_ANNOTATION_LEN - 1);
	 EndDialog(hDlg, IDOK);
	 return TRUE;

      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;
      }
      break;

   case WM_DESTROY:
      hAnnotateDialog = NULL;
      return TRUE;
   }
   return FALSE;
}
/*****************************************************************************/
void AbortAnnotateDialog(void)
{
  if (hAnnotateDialog != NULL)
    EndDialog(hAnnotateDialog, IDCANCEL);
}
