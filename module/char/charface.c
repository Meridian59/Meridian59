// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * charface.c:  Let user choose his appearance.
 */

#include "client.h"
#include "char.h"

#define GENDER_MALE   1
#define GENDER_FEMALE 2

extern CharAppearance *ap;

// Hotspots for mouth, eye, nose, hair
static int hotspots[] = { 12, 11, 14, 13 };

static list_type face_overlays;                 /* List of overlays to draw on face */
static HWND hFace;
extern HWND hMakeCharDialog;

static WNDPROC lpfnButtonProc;     // Default window procedure for owner-drawn button

static void CharFaceInitDialog(HWND hDlg);
static void CharFaceCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static void CharFaceHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos);
static void CharDrawFace(void);
static void CharRecomputeFace(void);
static long CALLBACK CharFaceButtonProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
/********************************************************************/
BOOL CALLBACK CharFaceDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
   case WM_INITDIALOG:
      CharFaceInitDialog(hDlg);
      break;

   case WM_NOTIFY:
      switch (((LPNMHDR) lParam)->code)
      {
      case PSN_SETACTIVE:
	 break;
      }
      return TRUE;      

   case WM_DRAWITEM:
      CharDrawFace();
      return TRUE;
      
      HANDLE_MSG(hDlg, WM_COMMAND, CharFaceCommand);
      HANDLE_MSG(hDlg, WM_HSCROLL, CharFaceHScroll);      
   }
   return FALSE;
}
/********************************************************************/
/*
 * CharFaceInitDialog:  Set up drawing of character face.
 *   hDlg is handle of appearance dialog.
 */
void CharFaceInitDialog(HWND hDlg)
{
   HWND hSlider;
   int i;

   hFace = GetDlgItem(hDlg, IDC_FACE);

   // Make blank overlay list for face
   face_overlays = NULL;
   for (i=0; i < NUM_FACE_OVERLAYS; i++)
   {
      Overlay *overlay = (Overlay *) SafeMalloc(sizeof(Overlay));

      memset(overlay, 0, sizeof(Overlay));

      overlay->hotspot = hotspots[i];
      overlay->animate.animation = ANIMATE_NONE;
      overlay->animate.group = 0;

      face_overlays = list_add_item(face_overlays, overlay);
   }

   // Set state of dialog buttons
   if (ap->gender_choice == 0)
      CheckDlgButton(hDlg, IDC_MALE, TRUE);
   else CheckDlgButton(hDlg, IDC_FEMALE, TRUE);

   // Set slider state
   hSlider = GetDlgItem(hDlg, IDC_COLOR);
   
   Trackbar_SetRange(hSlider, 0, ap->num_face_translations - 1, FALSE);
   Trackbar_SetPos(hSlider, ap->facet_choice);
   
   // Subclass button window
   lpfnButtonProc = SubclassWindow(hFace, CharFaceButtonProc);

   CharDrawFace();
}
/********************************************************************/
/*
 * CharFaceInit:  Initialize data for face choices.
 */
void CharFaceInit(void)
{
   FaceInfo *info;

   // Pick random initial face settings
   srand((unsigned) time(NULL));
   ap->gender_choice = rand() % NUM_GENDERS;
   ap->hairt_choice  = rand() % ap->num_hair_translations;
   ap->facet_choice  = rand() % ap->num_face_translations;
   info = &ap->parts[ap->gender_choice];

   ap->eye_choice   = rand() % info->num_eyes;
   ap->nose_choice  = rand() % info->num_noses;
   ap->mouth_choice = rand() % info->num_mouths;
   ap->hair_choice  = rand() % info->num_hair;
}
/********************************************************************/
/*
 * CharFaceExit:  Free memory for face drawing.
 */
void CharFaceExit(void)
{
   face_overlays = list_destroy(face_overlays);
}
/********************************************************************/
/*
 * CharFaceGetChoices:  Fill in stats with currently selected facial choices.
 */
void CharFaceGetChoices(CharAppearance *ap, int *stats, BYTE *gender)
{
   FaceInfo *info;

   if (ap->gender_choice == 0)
      *gender = GENDER_MALE;
   else *gender = GENDER_FEMALE;

   info = &ap->parts[ap->gender_choice];
   stats[0] = info->head;
   stats[1] = info->hair[ap->hair_choice];
   stats[2] = info->eyes[ap->eye_choice];
   stats[3] = info->noses[ap->nose_choice];
   stats[4] = info->mouths[ap->mouth_choice];
}

/********************************************************************/
/*
 * CharFaceCommand:  Handle WM_COMMAND messages for face appearance tab page.
 */
void CharFaceCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
   FaceInfo *info;

   info = &ap->parts[ap->gender_choice];

   UserDidSomething();

   switch (id)
   {
   case IDC_HAIR1:
      ap->hair_choice = (ap->hair_choice + info->num_hair - 1) % info->num_hair;
      break;
   case IDC_HAIR2:
      ap->hair_choice = (ap->hair_choice + 1) % info->num_hair;
      break;

   case IDC_EYES1:
      ap->eye_choice = (ap->eye_choice + info->num_eyes - 1) % info->num_eyes;
      break;
   case IDC_EYES2:
      ap->eye_choice = (ap->eye_choice + 1) % info->num_eyes;
      break;

   case IDC_NOSE1:
      ap->nose_choice = (ap->nose_choice + info->num_noses - 1) % info->num_noses;
      break;
   case IDC_NOSE2:
      ap->nose_choice = (ap->nose_choice + 1) % info->num_noses;
      break;

   case IDC_MOUTH1:
      ap->mouth_choice = (ap->mouth_choice + info->num_mouths - 1) % info->num_mouths;
      break;
   case IDC_MOUTH2:
      ap->mouth_choice = (ap->mouth_choice + 1) % info->num_mouths;
      break;

   case IDC_HAIRCOLOR1:
      ap->hairt_choice = (ap->hairt_choice + ap->num_hair_translations - 1) % ap->num_hair_translations;
      break;
   case IDC_HAIRCOLOR2:
      ap->hairt_choice = (ap->hairt_choice + 1) % ap->num_hair_translations;
      break;

   case IDC_MALE:      ap->gender_choice = 0;      break;
   case IDC_FEMALE:    ap->gender_choice = 1;      break;
   case IDC_COLOR1:    ap->facet_choice = 0;       break;
   case IDC_COLOR2:    ap->facet_choice = 1;       break;
   case IDC_COLOR3:    ap->facet_choice = 2;       break;
   case IDC_COLOR4:    ap->facet_choice = 3;       break;

   default:
      CharTabPageCommand(hwnd, id, hwndCtl, codeNotify);
      return;
   }

   // If gender or color changes, make sure that face selections are still in range
   CharRecomputeFace();
   CharDrawFace();
}
/********************************************************************/
/*
 * CharFaceHScroll:  Handle WM_HSCROLL messages.
 */
void CharFaceHScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
   switch (code)
   {
   case TB_LINEUP:
   case TB_LINEDOWN:
   case TB_PAGEUP:
   case TB_PAGEDOWN:
   case TB_THUMBPOSITION:
   case TB_THUMBTRACK:
   case TB_BOTTOM:
   case TB_TOP:
      // Fake a WM_COMMAND message corresponding to color value
      pos = Trackbar_GetPos(hwndCtl);
      FORWARD_WM_COMMAND(hwnd, IDC_COLOR1 + pos, hwndCtl, 0, CharFaceDialogProc);
      break;
      
   default:
      // Pointless "SB_ENDSCROLL" added recently
      return;
   }
}
/********************************************************************/
/*
 * CharDrawFace:  Draw user's face, with overlays selected in dialog
 */
void CharDrawFace(void)
{
   HDC hdc;
   AREA area;
   RECT rect;
   object_node obj;  // Fake object for drawing player's face
   list_type face_item;
   FaceInfo *info;
   Overlay *overlay;
   BYTE face_translation;

   if (hMakeCharDialog == NULL || face_overlays == NULL)
      return;

   GetClientRect(hFace, &rect);
   RectToArea(&rect, &area);
   memset(&obj, 0, sizeof(object_node));

   face_translation = ap->face_translations[ap->facet_choice];
   info = &ap->parts[ap->gender_choice];
   obj.icon_res = info->head;
   obj.translation = face_translation;

   // Build up list of overlays
   face_item = face_overlays;

   overlay = (Overlay *) (face_item->data);
   overlay->icon_res = info->mouths[ap->mouth_choice];
   overlay->translation = face_translation;
   face_item = face_item->next;
   overlay = (Overlay *) (face_item->data);
   overlay->icon_res = info->eyes[ap->eye_choice];
   overlay->translation = face_translation;
   face_item = face_item->next;
   overlay = (Overlay *) (face_item->data);
   overlay->icon_res = info->noses[ap->nose_choice];
   overlay->translation = face_translation;
   face_item = face_item->next;
   overlay = (Overlay *) (face_item->data);
   overlay->icon_res = info->hair[ap->hair_choice];
   overlay->translation = ap->hair_translations[ap->hairt_choice];

   obj.overlays = &face_overlays;

   hdc = GetDC(hFace);
   DrawStretchedObjectDefault(hdc, &obj, &area, 
			      GetSysColorBrush(COLOR_3DFACE));
   ReleaseDC(hFace, hdc);
}
/************************************************************************/
/* 
 * CharFaceButtonProc:  Subclass face window to have transparent background.
 */
long CALLBACK CharFaceButtonProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
   switch (message)
   {
   case WM_ERASEBKGND:
      return 1;
   }

   return CallWindowProc(lpfnButtonProc, hwnd, message, wParam, lParam);
}
/************************************************************************/
/*
 * CharRecomputeFace:  Bring all face part selections into legal ranges
 *   (used when gender or color choice changes).
 */
void CharRecomputeFace(void)
{
   FaceInfo *info;

   info = &ap->parts[ap->gender_choice];

   ap->hair_choice  = ap->hair_choice % info->num_hair;
   ap->eye_choice   = ap->eye_choice % info->num_eyes;
   ap->nose_choice  = ap->nose_choice % info->num_noses;
   ap->mouth_choice = ap->mouth_choice % info->num_mouths;
}
/************************************************************************/
/*
 * CharAppearanceDestroy:  Free a CharAppearance structure and return NULL.
 */
CharAppearance *CharAppearanceDestroy(CharAppearance *ap)
{
   int i;
   
   for (i=0; i < NUM_GENDERS; i++)
   {
      FaceInfo *info = &ap->parts[i];
      SafeFree(info->hair);
      SafeFree(info->eyes);
      SafeFree(info->noses);
      SafeFree(info->mouths);
   }
   SafeFree(ap->hair_translations);
   SafeFree(ap->face_translations);
   SafeFree(ap);
   return NULL;
}
