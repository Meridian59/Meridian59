// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
// BBGun - The Art of Lining Things Up
//
// BBGun is a hotspot editor for bitmaps.  It reads and writes .BBG
// files, which are command files used as input to makebgf.  BBGun
// allows you to load bitmaps, define hotspots and overlays, and view
// animations.

#include "bbgun.h"

#define MAX_HOTSPOT_LEN 4  // Max # of characters in a hotspot

static void ResetBBGList(void);
static void AddHotspot(int num);
static int  FindHotspot(int prev_value);

BOOL CALLBACK SettingsDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static long CALLBACK HotspotNumProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);

static WNDPROC lpfnDefEditProc;
static char szBuffer[MAX_PATH];

HINSTANCE hInst;
HWND hMain;

BBGdata BBGs[MAX_BBGS];
Config config;

int Zoom;
int CurrentBitmap = 0;
int CurrentGroup = 0;
int CurrentHotspot = 0;
int CurrentGroupMember = 0;
int CurrentBBG = 0;
int NumBBGs = 0;

static HMENU hMenu;

GraphSpot WanderOne;
GraphSpot Anchor;

char *szAppName = "BBGun";
static char *help_file = "bbgun.hlp";

static char *BMP_Filter = "Bitmaps\0*.bmp\0All Files\0*.*\0\0";
/************************************************************************/
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
   MSG msg;
   WNDCLASS wndclass;

   if(!hPrevInst)
   {
      wndclass.style = CS_HREDRAW | CS_VREDRAW;
      wndclass.lpfnWndProc = WndProc;
      wndclass.cbClsExtra = 0;
      wndclass.cbWndExtra = DLGWINDOWEXTRA;
      wndclass.hInstance = hInstance;
      wndclass.hCursor = NULL;
      wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
      wndclass.lpszMenuName = "BBGun";
      wndclass.hbrBackground = ((HBRUSH)GetStockObject(LTGRAY_BRUSH));
      wndclass.lpszClassName = szAppName;

      RegisterClass(&wndclass);
   }

   hInst = hInstance;
   // NOTE:  You must include this line if you have any Borland BWCC controls!
   //   BWCCRegister(hInst);

   DrawInit();

   hMain = CreateDialog(hInstance, "MainDialog", NULL, (DLGPROC) WndProc);

   DrawAreaInit();

   ShowWindow(hMain, nCmdShow);

   while( GetMessage(&msg,0,0,0) )
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }

   DrawClose();

   return msg.wParam;
}
/************************************************************************/
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   int hotspot, bbg, i;

   switch(msg)
   {
   case WM_INITDIALOG:
      hMenu = GetMenu(hWnd);

      config.show_hotspots = TRUE;
      config.transparent = TRUE;
      
      CheckMenuItem(hMenu, IDM_SHOWHOTSPOTS, config.show_hotspots ? MF_CHECKED : MF_UNCHECKED);
      CheckMenuItem(hMenu, IDM_TRANSPARENT, config.transparent ? MF_CHECKED : MF_UNCHECKED);

      Edit_LimitText(GetDlgItem(hWnd, IDHOTSPOT), MAX_HOTSPOT_LEN);

      Anchor.BBG = -1;
      WanderOne.BBG = -1;

      Zoom = ZOOM_INITIAL;
      SetScrollRange(GetDlgItem(hMain, IDZOOMBAR), SB_CTL, 1, ZOOM_MAX, 1);
      SetDlgItemInt(hMain, IDZOOMTEXT, Zoom, TRUE);
      SetScrollPos(GetDlgItem(hMain, IDZOOMBAR), SB_CTL, Zoom, TRUE);

      lpfnDefEditProc = SubclassWindow(GetDlgItem(hWnd, IDHOTSPOT), HotspotNumProc);
      return 0;

   case WM_VSCROLL:
      bbg = GetPrimaryBBG();
      if (BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].NumHotspots > 0)
	 switch(LOWORD(wParam))
	 {
	 case SB_LINEUP:
	    BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].Hotspots[BBGs[bbg].CurrentHotspot].Lap++;
	    FillHotlist();
	    return 0;
	    
	 case SB_LINEDOWN:
	    BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].Hotspots[BBGs[bbg].CurrentHotspot].Lap--;
	    FillHotlist();
	    return 0;
	 }
      return 0;
      
   case WM_HSCROLL:
      switch(GetDlgCtrlID((HWND)lParam))
      {
      case IDZOOMBAR:
         switch(LOWORD(wParam))
	 {
	 case SB_LINELEFT:
	    if (Zoom > 1)
               Zoom--; 
	    break;
	 case SB_LINERIGHT:
	    if (Zoom < ZOOM_MAX)
               Zoom++; 
	    break;
	 case SB_PAGELEFT:
	    if (Zoom > 5)
               Zoom -= 5; 
	    break;
	 case SB_PAGERIGHT:
	    if (Zoom + 5 <= ZOOM_MAX)
               Zoom += 5; 
	    break;
	 case SB_THUMBTRACK:
	    Zoom = HIWORD(wParam); 
	    break;
	 case SB_THUMBPOSITION:
	    Zoom = HIWORD(wParam); 
	    break;
	 }
	 if (Zoom == 0)
	    Zoom = 1;
         SetDlgItemInt(hWnd, IDZOOMTEXT, Zoom, 1);
         SetScrollPos((HWND)lParam, SB_CTL, Zoom, 1);
         DrawIt();
	 return 0;
	 
      case IDC_SHRINKSCROLL:
	 if (NumBBGs <= 0)
	    break;

	 bbg = GetPrimaryBBG();

	 switch(LOWORD(wParam))
	 {
	 case SB_LINELEFT:
	    if (BBGs[bbg].Shrink > 1) 
	       BBGs[bbg].Shrink--;
	    SetDlgItemInt(hWnd, IDC_SHRINK, BBGs[bbg].Shrink, TRUE);
	    BBGs[bbg].changed = TRUE;
	    DrawIt();
	    break;
	    
	 case SB_LINERIGHT:
	    BBGs[bbg].Shrink++;
	    SetDlgItemInt(hWnd, IDC_SHRINK, BBGs[bbg].Shrink, TRUE);
	    BBGs[bbg].changed = TRUE;
	    DrawIt();
	    break;
	 }
	 break;

      case IDC_SHRINKSCROLL2:
	 if (WanderOne.BBG == -1)
	    break;
	 
	 switch(LOWORD(wParam))
	 {
	 case SB_LINELEFT:
	    if (BBGs[WanderOne.BBG].Shrink > 1) 
	       BBGs[WanderOne.BBG].Shrink--;
	    SetDlgItemInt(hWnd, IDC_SHRINK2, BBGs[WanderOne.BBG].Shrink, TRUE);
	    BBGs[WanderOne.BBG].changed = TRUE;
	    DrawIt();
	    break;
	    
	 case SB_LINERIGHT:
	    BBGs[WanderOne.BBG].Shrink++;
	    SetDlgItemInt(hWnd, IDC_SHRINK2, BBGs[WanderOne.BBG].Shrink, TRUE);
	    BBGs[WanderOne.BBG].changed = TRUE;
	    DrawIt();
	    break;
	 }

	 break;
      }
      return 0;

   case WM_DRAWITEM:
      DrawIt();
      return TRUE;

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDVIEWHOTS:
         config.all_hotspots = SendDlgItemMessage(hWnd, IDVIEWHOTS, BM_GETCHECK, 0, 0);
         DrawIt();
         break;

      case IDC_DUALMODE:
	 if (Anchor.BBG == -1 || WanderOne.BBG == -1)
	 {
	    config.dual_mode = SendDlgItemMessage(hWnd, IDC_DUALMODE, BM_SETCHECK, 0, 0);
	    break;
	 }

	 config.dual_mode = SendDlgItemMessage(hWnd, IDC_DUALMODE, BM_GETCHECK, 0, 0);
	 if (!config.dual_mode)
	 {
	    WanderOne.BBG = -1;
	    SetDlgItemText(hWnd, WANDER1TEXT, " ");
	 }
	 FillBitmapList();
	 FillBitmapList2();
	 FillGroupList();
	 FillHotlist();
	 FillGroupiesList();
	 DrawIt();
	 break;

      case IDOPPOSITELAY:
	 bbg = GetPrimaryBBG();
         BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].Hotspots[BBGs[bbg].CurrentHotspot].Lap *= (-1);
         FillHotlist();
	 BBGs[bbg].changed = TRUE;
         DrawIt();
         return 0;

      case IDSETWANDERONE:
         if ((Anchor.BBG == -1)||(NumBBGs < 2)||(Anchor.BBG == CurrentBBG)) return 0;
         WanderOne.HotNum = Anchor.HotNum;
         WanderOne.Lap = Anchor.Lap;
         WanderOne.BBG = CurrentBBG;
         SetDlgItemText(hWnd,WANDER1TEXT,BBGs[WanderOne.BBG].Title);
	 FillBitmapList2();
         DrawIt();
         return 0;

      case IDSETANCHOR:
         if (NumBBGs < 1) return 0;
         Anchor.Lap = BBGs[CurrentBBG].Bitmaps[CurrentBitmap].Hotspots[CurrentHotspot].Lap;
         Anchor.BBG = CurrentBBG;
         Anchor.HotNum = CurrentHotspot;
         SetDlgItemText(hWnd, ANCHORTEXT, BBGs[Anchor.BBG].Title);
         DrawIt();
         return 0;

      case IDM_ANIME: // WILL LOAD ANIMATION DIALOG
          MessageBox(hWnd,"This Feature not yet Implemented","",MB_OK);
          return 0;

      case IDM_OTHER:
	 if (DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, SettingsDialogProc) == IDOK)
	    ResetBBGList();
	 return 0;

      case IDGROUPINSBIT:
          GroupInsertBitmap(CurrentBitmap + 1);
          return 0;

      case IDINSERTZERO:
          GroupInsertBitmap(0);
          return 0;

      case IDGROUPADDBIT: // Add a Bitmap to the currently selected group
          GroupAddBitmap();
          return 0;

      case IDGROUPREMOVEBIT: // Remove a Bitmap from the currently selected group
          GroupRemoveBitmap();
          return 0;

      case IDGROUPCREATE: // Create a new group
          GroupCreate();
          return 0;

      case IDGROUPREMOVE: // Remove (erase) currently selected group
          GroupRemove();
          return 0;

      case IDM_OPENBBG: // Load a BBG
          OpenBBG();
          return 0;

      case IDM_CREATEBBG: // Create a new (blank) BBG
          CreateBBG();
          return 0;

      case IDM_SAVEBBG: // Save currently selected BBG to a file
          SaveBBG();
          return 0;

      case IDM_CLOSEBBG: // Close BBG (DOES NOT SAVE)
          CloseBBG();
          return 0;

      case IDC_BITMAPLOAD: // Add a bitmap to the currently selelted BBG
          BitmapLoad();
          return 0;

      case IDC_BITMAPUNLOAD: // Remove a bitmap from currently selected BBG
          BitmapUnload();
          return 0;

      case IDNEWHOTSPOT: // Add a new hotspot to currently selected bitmap
          CreateHotspot();
          return 0;

      case IDDELHOTSPOT:  // Remove a hotspot from currently selected bitmap
          RemoveHotspot();
          return 0;

      case IDM_SHOWHOTSPOTS:
	 config.show_hotspots = !config.show_hotspots;
	 CheckMenuItem(hMenu, IDM_SHOWHOTSPOTS, config.show_hotspots ? MF_CHECKED : MF_UNCHECKED);
	 DrawIt();
	 return 0;

      case IDM_TRANSPARENT:
	 config.transparent = !config.transparent;
	 CheckMenuItem(hMenu, IDM_TRANSPARENT, config.transparent ? MF_CHECKED : MF_UNCHECKED);
	 DrawIt();
	 return 0;

      case IDM_EXIT: // Exit program
	 for (i=0; i < NumBBGs; i++)
	 {
	    sprintf(szBuffer, "BBG #%d has changed; are you sure you want to exit?", i + 1);
	    if (BBGs[i].changed &&
		MessageBox(hMain, szBuffer, "Confirm BBG close", MB_YESNO) == IDNO)
	       return 0;
	 }

          PostQuitMessage(0);
          return 0;

      case IDBBGLIST:
	 if (GET_WM_COMMAND_CMD(wParam, lParam) != LBN_SELCHANGE)
	    return 0;

	 /* The selected BBG has changed */
	 if (NumBBGs > 0)
	 {
	    CurrentBBG = (int)SendDlgItemMessage(hWnd, IDBBGLIST, LB_GETCURSEL, 0, 0L);
	    CurrentBitmap = BBGs[CurrentBBG].CurrentBitmap;
	    CurrentGroup = 0;
	    CurrentGroupMember = 0;
	    CurrentHotspot = BBGs[CurrentBBG].CurrentHotspot;
	    FillBitmapList();
	    FillGroupList();
	    FillHotlist();
	    FillGroupiesList();
	    
	    DrawIt();
          }
	 return 0;

      case IDC_FILELIST:
	 bbg = GetPrimaryBBG();
	 if (GET_WM_COMMAND_CMD(wParam, lParam) != LBN_SELCHANGE ||
	     BBGs[bbg].NumBitmaps == 0)
	    return 0;
	 
	 /* The selected bitmap has changed */

	 // Get current hotspot number
	 hotspot = 
	    BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].Hotspots[BBGs[bbg].CurrentHotspot].Lap;

	 CurrentBitmap = (int)SendDlgItemMessage(hWnd, IDC_FILELIST, LB_GETCURSEL, 0, 0L);
	 
	 CurrentHotspot = FindHotspot(hotspot);
	 BBGs[bbg].CurrentHotspot = CurrentHotspot;
	 BBGs[bbg].CurrentBitmap = CurrentBitmap;
	 DrawIt();
	 FillHotlist();
	 return 0;

      case IDC_FILELIST2:
	 if (GET_WM_COMMAND_CMD(wParam, lParam) != LBN_SELCHANGE ||
	     !config.dual_mode ||
	     WanderOne.BBG == -1)
	    return 0;
	 
	 /* The selected bitmap has changed */
	 BBGs[WanderOne.BBG].CurrentBitmap = 
	    SendDlgItemMessage(hWnd, IDC_FILELIST2, LB_GETCURSEL, 0, 0L);
	 DrawIt();
	 FillHotlist();
	 return 0;

      case IDGROUPLIST:
	 if (GET_WM_COMMAND_CMD(wParam, lParam) != LBN_SELCHANGE)
	    return 0;

	 bbg = GetPrimaryBBG();
	 /* The selected group has changed */
	 if (BBGs[bbg].NumGroups > 0)
	 {
	    CurrentGroup = (int)SendDlgItemMessage(hWnd, IDGROUPLIST, LB_GETCURSEL, 0, 0L);
	    BBGs[bbg].CurrentGroup = CurrentGroup;
	    FillGroupiesList();
	    CurrentGroupMember = 0;
	    SendDlgItemMessage(hWnd, IDC_FILELIST, LB_SETCURSEL, 
			       (LPARAM) BBGs[bbg].CurrentBitmap, 0L);
	    SendDlgItemMessage(hWnd, IDGROUPIES, LB_SETCURSEL, 
			       (LPARAM) CurrentGroupMember, 0L);
	    FillHotlist();
	 }
	 return 0;

      case IDGROUPIES:
	 if (GET_WM_COMMAND_CMD(wParam, lParam) != LBN_SELCHANGE)
	    return 0;
	 
	 bbg = GetPrimaryBBG();
	 /* The selected group member has changed */
	 if (BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Members > 0)
	 {
	    CurrentGroupMember = (int)SendDlgItemMessage(hWnd, IDGROUPIES, LB_GETCURSEL, 0, 0L);
	    SendDlgItemMessage(hWnd, IDC_FILELIST, LB_SETCURSEL, 
			       (LPARAM) BBGs[bbg].CurrentBitmap, 0L);
	    FillHotlist();
	 }
	 return 0;

      case IDHOTLIST:
	 if (GET_WM_COMMAND_CMD(wParam, lParam) != LBN_SELCHANGE)
	    return 0;

	 bbg = GetPrimaryBBG();
	 /* The selected hotspot has changed */
	 if (BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].NumHotspots > 0)
	 {
	    CurrentHotspot = (int)SendDlgItemMessage(hWnd, IDHOTLIST, LB_GETCURSEL, 0, 0L);
	    BBGs[bbg].CurrentHotspot = CurrentHotspot;
	    FillHotlist();
	    DrawIt();
	    return 0;
	 }
	 break;

      case IDM_HELP:
	 WinHelp(hMain, help_file, HELP_CONTENTS, 0);
	 return 0;
      }
      break;

   case WM_DESTROY:
      WinHelp(hMain, help_file, HELP_QUIT, 0);
      PostQuitMessage(0);
      return 0;
   }
   return DefWindowProc(hWnd, msg, wParam, lParam);
}

/************************************************************************/
void DrawIt(void)
{
   HWND hwnd;
   HDC hdc;

   hwnd = GetDlgItem(hMain, IDBITMAP);
   hdc = GetDC(hwnd);

   DoDraw(hdc, hMain);

   ReleaseDC(hwnd, hdc);
}
/************************************************************************/
BOOL CALLBACK SettingsDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch(msg)
   {
   case WM_INITDIALOG:
      Edit_LimitText(GetDlgItem(hDlg, IDC_COMMENTS), MAX_COMMENT - 1);

      SetDlgItemText(hDlg, IDFILENAME, BBGs[CurrentBBG].Title);
      SetDlgItemText(hDlg, IDC_COMMENTS, BBGs[CurrentBBG].Comment);
      return TRUE;

   case WM_COMMAND:
      switch(wParam)
      {
      case IDOK:
	 GetDlgItemText(hDlg, IDFILENAME, BBGs[CurrentBBG].Title, MAX_PATH);
	 GetDlgItemText(hDlg, IDC_COMMENTS, BBGs[CurrentBBG].Comment, MAX_COMMENT);
	 BBGs[CurrentBBG].changed = TRUE;
	 
	 EndDialog(hDlg, IDOK);
	 return TRUE;
	 
      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;
      }
      break;
   }
   return FALSE;
}
/************************************************************************/
/*
 * UpdateOffset:  Redisplay anchor bitmap offset.
 */
void UpdateOffset(void)
{
   int bbg = GetPrimaryBBG();

   if ((BBGs[bbg].NumBitmaps == 0)||(NumBBGs == 0))
   {
      SetDlgItemText(hMain, IDC_XOFFSET, "-");
      SetDlgItemText(hMain, IDC_YOFFSET, "-");
      SetDlgItemText(hMain, IDC_SHRINK, "");
      return;
   }
   SetDlgItemInt(hMain, IDC_XOFFSET, BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].XOffset,1);
   SetDlgItemInt(hMain, IDC_YOFFSET, BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].YOffset,1);
   SetDlgItemInt(hMain, IDC_SHRINK, BBGs[bbg].Shrink,1);
}
/************************************************************************/
/*
 * UpdateOffset2:  Redisplay overlay bitmap offset.
 */
void UpdateOffset2(void)
{
   int bbg;

   bbg = WanderOne.BBG;

   if (WanderOne.BBG == -1 || BBGs[bbg].NumBitmaps == 0 || NumBBGs == 0)
   {
      SetDlgItemText(hMain, IDC_XOFFSET2, "-");
      SetDlgItemText(hMain, IDC_YOFFSET2, "-");
      SetDlgItemText(hMain, IDC_SHRINK2, "");
      return;
   }
   SetDlgItemInt(hMain, IDC_XOFFSET2,BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].XOffset,1);
   SetDlgItemInt(hMain, IDC_YOFFSET2,BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].YOffset,1);
   SetDlgItemInt(hMain, IDC_SHRINK2,BBGs[bbg].Shrink,1);
}
/************************************************************************/
void FillBitmapList(void)
{
   int i, bbg;
   HWND hList;

   bbg = GetPrimaryBBG();

   hList = GetDlgItem(hMain, IDC_FILELIST);
   
   ListBox_ResetContent(hList);
   for (i = 0; i < BBGs[bbg].NumBitmaps; i++)
   {
      ListBox_AddString(hList, BBGs[bbg].Bitmaps[i].Title);
   }
   ListBox_SetCurSel(hList, BBGs[bbg].CurrentBitmap);
   UpdateOffset();
}
/************************************************************************/
/*
 * Fill overlay bitmap list
 */
void FillBitmapList2(void)
{
   int i;
   HWND hList;

   hList = GetDlgItem(hMain, IDC_FILELIST2);
   
   ListBox_ResetContent(hList);

   if (WanderOne.BBG == -1)
      return;

   for (i = 0; i < BBGs[WanderOne.BBG].NumBitmaps; i++)
   {
      ListBox_AddString(hList, BBGs[WanderOne.BBG].Bitmaps[i].Title);
   }
   ListBox_SetCurSel(hList, BBGs[WanderOne.BBG].CurrentBitmap);
   UpdateOffset2();
}
/************************************************************************/
void FillGroupiesList(void)
{
   int i, index;
   HWND hList;
   int bbg = GetPrimaryBBG();

   hList = GetDlgItem(hMain, IDGROUPIES);
   ListBox_ResetContent(hList);
   if ((BBGs[bbg].NumGroups == 0) ||
       (BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Members == 0) ||
       (NumBBGs == 0))
      return;

   for (i = 0; i < BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Members; i++)
   {
      index = BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Indices[i] - 1;
      if (index == -1) 
	 ListBox_AddString(hList, "(none)");
      else ListBox_AddString(hList, BBGs[bbg].Bitmaps[index].Title);
   }

   ListBox_SetCurSel(hList, CurrentGroupMember);
}
/************************************************************************/
void FillGroupList(void)
{
   int i;
   HWND hList;
   int bbg = GetPrimaryBBG();

   hList = GetDlgItem(hMain, IDGROUPLIST);

   ListBox_ResetContent(hList);
   if ((BBGs[bbg].NumGroups == 0)||(NumBBGs == 0))
      return;

   for (i = 0; i < BBGs[bbg].NumGroups; i++)
   {
      wsprintf(szBuffer,"%d",i);
      ListBox_AddString(hList, szBuffer);
   }
   BBGs[bbg].CurrentGroup = 0;
   ListBox_SetCurSel(hList, BBGs[bbg].CurrentGroup);
}
/************************************************************************/
/*
 * UpdateHotspot:  Update display of hotspot position.
 */
void UpdateHotspot(void)
{
   int bbg = GetPrimaryBBG();

   if (BBGs[bbg].NumBitmaps == 0 || NumBBGs == 0 ||
       BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].NumHotspots == 0)
   {
      SetDlgItemText(hMain, IDC_HOTX, "-");
      SetDlgItemText(hMain, IDC_HOTY, "-");
      SetDlgItemText(hMain, IDHOTSPOT, "");
      return;
   }

   SetDlgItemInt(hMain, IDC_HOTX,
		 BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].Hotspots[BBGs[bbg].CurrentHotspot].X, 
		 TRUE);
   SetDlgItemInt(hMain, IDC_HOTY,
		 BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].Hotspots[BBGs[bbg].CurrentHotspot].Y, 
		 TRUE);
   SetDlgItemInt(hMain, IDHOTSPOT,
		 BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].Hotspots[BBGs[bbg].CurrentHotspot].Lap,
		 TRUE);   
}
/************************************************************************/
void FillHotlist(void)
{
   int i;
   HWND hList;
   int bbg = GetPrimaryBBG();
   

   hList = GetDlgItem(hMain, IDHOTLIST);

   ListBox_ResetContent(hList);

   for (i = 0; i < BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].NumHotspots; i++)
   {
      wsprintf(szBuffer, "%d" , BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].Hotspots[i].Lap);
      ListBox_AddString(hList, szBuffer);
   }
   ListBox_SetCurSel(hList, BBGs[bbg].CurrentHotspot);

   UpdateHotspot();
}
/************************************************************************/
void RemoveHotspot(void)
{
   int i;
   int bbg = GetPrimaryBBG();

   if (BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].NumHotspots == 0)
      return;

   for (i = BBGs[bbg].CurrentHotspot; i < BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].NumHotspots; i++)
      BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].Hotspots[i] = 
	 BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].Hotspots[i + 1];

   BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].NumHotspots--;
   BBGs[bbg].CurrentHotspot = 0;
   SendDlgItemMessage(hMain, IDHOTLIST, LB_SETCURSEL, (LPARAM) BBGs[bbg].CurrentHotspot, 0L);

   FillHotlist();
   BBGs[bbg].changed = TRUE;
   DrawIt();
}
/************************************************************************/
void CreateHotspot(void)
{
   int bbg = GetPrimaryBBG();

   if (BBGs[bbg].NumBitmaps == 0 || 
       BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].NumHotspots == MAX_HOTSPOTS)
      return;

   CurrentHotspot = BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].NumHotspots;
   BBGs[bbg].CurrentHotspot = CurrentHotspot;
   BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].NumHotspots++;
   BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].Hotspots[BBGs[bbg].CurrentHotspot].X = 0;
   BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].Hotspots[BBGs[bbg].CurrentHotspot].Y = 0;
   BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].Hotspots[BBGs[bbg].CurrentHotspot].Lap = 
      CurrentHotspot;
   wsprintf(szBuffer, "%d" , CurrentHotspot);
   SendDlgItemMessage(hMain, IDHOTLIST, LB_ADDSTRING, 0, (LPARAM) szBuffer);
   SendDlgItemMessage(hMain, IDHOTLIST, LB_SETCURSEL, CurrentHotspot, 0);
   BBGs[bbg].changed = TRUE;
   DrawIt();
}
/************************************************************************/
/*
 * AddHotspot:  Add hotspot of given # to current group, or select hotspot
 *   if it already exists.
 */
void AddHotspot(int num)
{
   Bitmap *b;
   int i, index = -1;
   int bbg = GetPrimaryBBG();

   if (BBGs[bbg].NumBitmaps == 0 ||
       BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap].NumHotspots == MAX_HOTSPOTS)
      return;

   b = &BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap];

   // See if hotspot already exists
   for (i=0; i < b->NumHotspots; i++)
   {
      if (abs(b->Hotspots[i].Lap) == abs(num))
      {
	 index = i;
	 break;
      }
   }

   if (index == -1)
   {
      // Add new hotspot
      CurrentHotspot = b->NumHotspots;
      BBGs[bbg].CurrentHotspot = CurrentHotspot;
      b->NumHotspots++;
      b->Hotspots[CurrentHotspot].X = 0;
      b->Hotspots[CurrentHotspot].Y = 0;
      b->Hotspots[CurrentHotspot].Lap = num;
      wsprintf(szBuffer, "%d" , num);
      SendDlgItemMessage(hMain, IDHOTLIST, LB_ADDSTRING, 0, (LPARAM)szBuffer);
      SendDlgItemMessage(hMain, IDHOTLIST, LB_SETCURSEL, CurrentHotspot, 0);
      BBGs[bbg].changed = TRUE;
   }
   else
   {
      // Go to existent hotspot
      CurrentHotspot = index;
      BBGs[bbg].CurrentHotspot = CurrentHotspot;
      UpdateHotspot();
   }

   DrawIt();
}
/************************************************************************/
void CreateBBG(void)
{
   if (NumBBGs == MAX_BBGS)
      return;

   CurrentBBG = NumBBGs;
   wsprintf(BBGs[NumBBGs].FullPath,"NEWBBG%d.BBG", NumBBGs);
   wsprintf(BBGs[NumBBGs].Title, "NEWBBG%d.BBG", NumBBGs);
   BBGs[CurrentBBG].NumGroups = 0;
   BBGs[CurrentBBG].NumBitmaps = 0;
   CurrentBitmap = 0;
   BBGs[CurrentBBG].CurrentBitmap = CurrentBitmap;
   CurrentGroup = 0;
   SendDlgItemMessage(hMain, IDBBGLIST, LB_ADDSTRING, 0, (LPARAM) BBGs[NumBBGs].Title);
   BBGs[CurrentBBG].changed = FALSE;
   NumBBGs++;
   
   if (!config.dual_mode)
   {
      SendDlgItemMessage(hMain, IDBBGLIST, LB_SETCURSEL, (LPARAM) CurrentBBG, 0L);
      SendDlgItemMessage(hMain, IDGROUPIES, LB_RESETCONTENT, 0, 0L);
      SendDlgItemMessage(hMain, IDHOTLIST, LB_RESETCONTENT, 0, 0L);
      SendDlgItemMessage(hMain, IDC_FILELIST, LB_RESETCONTENT, 0, 0L);
      SendDlgItemMessage(hMain, IDGROUPLIST, LB_RESETCONTENT, 0, 0L);
   }

   DrawIt();
}
/************************************************************************/
void GroupCreate(void)
{
   int bbg = GetPrimaryBBG();

   if ((NumBBGs == 0)||(BBGs[bbg].NumGroups == MAX_GROUPS))
      return;

   BBGs[bbg].CurrentGroup = BBGs[bbg].NumGroups;
   BBGs[bbg].NumGroups++;
   BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Members = 0;
   wsprintf(szBuffer, "%d" , BBGs[bbg].CurrentGroup);
   SendDlgItemMessage(hMain, IDGROUPLIST, LB_ADDSTRING, 0, (LPARAM) szBuffer);
   SendDlgItemMessage(hMain, IDGROUPLIST, LB_SETCURSEL, (LPARAM) BBGs[bbg].CurrentGroup, 0L);
   BBGs[bbg].changed = TRUE;
}
/************************************************************************/
void GroupRemove(void)
{
   int i;
   int bbg = GetPrimaryBBG();

   if (BBGs[bbg].NumGroups == 0)
      return;

   BBGs[bbg].NumGroups--;
   for (i = BBGs[bbg].CurrentGroup; i < BBGs[bbg].NumGroups; i++)
      BBGs[bbg].Groups[i] = BBGs[bbg].Groups[i + 1];

   FillGroupList();
   FillGroupiesList();
   BBGs[bbg].changed = TRUE;
}
/************************************************************************/
void GroupRemoveBitmap(void)
{
   int i;
   int bbg = GetPrimaryBBG();

   if (BBGs[bbg].NumBitmaps == 0)
      return;

   BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Members--;

   for (i = CurrentGroupMember; i < BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Members; i++)
      BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Indices[i] = 
	 BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Indices[i + 1];

   CurrentGroupMember = ListBoxRemoveCurrentItem(GetDlgItem(hMain, IDGROUPIES));
   BBGs[bbg].changed = TRUE;
}
/************************************************************************/
/*
 * GroupInsertBitmap:  Add given bitmap index to current group at current selection position
 */
void GroupInsertBitmap(int bitmap_index)
{
   int i;
   int bbg = GetPrimaryBBG();

   if (BBGs[bbg].NumBitmaps == 0 || BBGs[bbg].NumGroups == 0)
      return;

   for (i = BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Members; i > CurrentGroupMember ; i--)
      BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Indices[i] = 
	 BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Indices[i - 1];

   BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Indices[CurrentGroupMember] = bitmap_index;
   BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Members += 1;

   FillGroupiesList();
   BBGs[bbg].changed = TRUE;
}
/************************************************************************/
void GroupAddBitmap(void)
{
   int bbg = GetPrimaryBBG();

   if (BBGs[bbg].NumBitmaps == 0 || BBGs[bbg].NumGroups == 0)
      return;

   CurrentGroupMember = BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Members;
   BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Indices[CurrentGroupMember] = 
      BBGs[bbg].CurrentBitmap + 1;
   BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Members += 1;

   SendDlgItemMessage(hMain, IDGROUPIES , LB_ADDSTRING, 0,
		      (LPARAM) BBGs[bbg].Bitmaps[BBGs[bbg].Groups[BBGs[bbg].CurrentGroup].Indices[CurrentGroupMember] - 1].Title);
   SendDlgItemMessage(hMain, IDGROUPIES, LB_SETCURSEL, (LPARAM) CurrentGroupMember, 0L);
   BBGs[bbg].changed = TRUE;
}
/************************************************************************/
void BitmapUnload(void)
{
   int i, j, k;
   int bbg = GetPrimaryBBG();
   
   if (BBGs[bbg].NumBitmaps == 0)
      return;
   
   /* UPDATE BITMAP STRUCT */
   for (i = BBGs[bbg].CurrentBitmap; i < BBGs[bbg].NumBitmaps; i++)
      BBGs[bbg].Bitmaps[i] = BBGs[bbg].Bitmaps[i + 1];
   BBGs[bbg].NumBitmaps--;
   
   /* UPDATE GROUP STRUCTS */
   for (i = 0; i < BBGs[bbg].NumGroups; i++)
      for (j = 0; j < BBGs[bbg].Groups[i].Members; j++)
      {
	 if (BBGs[bbg].Groups[i].Indices[j] == BBGs[bbg].CurrentBitmap + 1)
	 {
	    for (k = j; k < BBGs[bbg].Groups[i].Members; k++)
	       BBGs[bbg].Groups[i].Indices[k] = BBGs[bbg].Groups[i].Indices[k + 1];
	    BBGs[bbg].Groups[i].Members--;
	 }
	 if (BBGs[bbg].Groups[i].Indices[j] > BBGs[bbg].CurrentBitmap + 1)
	    BBGs[bbg].Groups[i].Indices[j]--;
      }

   CurrentBitmap = ListBoxRemoveCurrentItem(GetDlgItem(hMain, IDC_FILELIST));
   BBGs[bbg].CurrentBitmap = CurrentBitmap;

   UpdateOffset();
   FillGroupiesList();
   FillHotlist();
   DrawIt();
   BBGs[bbg].changed = TRUE;
}
/************************************************************************/
void BitmapLoad(void)
{
   char path[MAX_PATH];
   char title[MAX_PATH];
   Bitmap *b;
   int i, j;
   int bbg = GetPrimaryBBG();

   OPENFILENAME ofn;
   if (NumBBGs == 0 || BBGs[bbg].NumBitmaps == MAX_BITMAPS)
      return;

   BBGs[bbg].Bitmaps[BBGs[bbg].NumBitmaps].Title[0] = '\0';
   path[0] = '\0';
   memset( &ofn, 0, sizeof(OPENFILENAME) );
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hwndOwner = hMain;
   ofn.lpstrFilter = BMP_Filter;
   ofn.lpstrFile = path;
   ofn.nMaxFile =  MAX_PATH;
   ofn.lpstrFileTitle = title;
   ofn.nMaxFileTitle = 50;
   ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
   if( !GetOpenFileName(&ofn) )
   {
      MessageBeep(0);

      SetFocus(hMain);
      return;
   }

   // Add new bitmap to internal structures

   // Update bitmap struct 
   for (i = BBGs[bbg].NumBitmaps; i > BBGs[bbg].CurrentBitmap; i--)
      BBGs[bbg].Bitmaps[i] = BBGs[bbg].Bitmaps[i - 1];
   BBGs[bbg].NumBitmaps++;

   // Update group indices
   for (i = 0; i < BBGs[bbg].NumGroups; i++)
      for (j = 0; j < BBGs[bbg].Groups[i].Members; j++)
	 if (BBGs[bbg].Groups[i].Indices[j] > BBGs[bbg].CurrentBitmap)
	    BBGs[bbg].Groups[i].Indices[j]++;	 

   // Set up new bitmap entry
   b = &BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap];
   memset(b, 0, sizeof(Bitmap));
   strcpy(b->Title, title);   

   FillBitmapList();
   DrawIt();
   BBGs[bbg].changed = TRUE;
}
/************************************************************************/
/* 
 * ResetBBGList:  Reset contents of BBG file listbox
 */
void ResetBBGList(void)
{
   int index, i;
   HWND hList = GetDlgItem(hMain, IDBBGLIST);
   
   index = ListBox_GetCurSel(hList);

   ListBox_ResetContent(hList);
   
   for (i=0; i < NumBBGs; i++)
      ListBox_AddString(hList, BBGs[i].Title);

   ListBox_SetCurSel(hList, index);
}


/************************************************************************/
/*
 * HotspotNumProc:  Subclassed window procedure for hotspot edit box.
 */
long CALLBACK HotspotNumProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
   int num;

   switch (message)
   {
   case WM_CHAR:
      if (wParam != VK_RETURN)
	 break;

      if (Edit_GetText(hwnd, szBuffer, MAX_HOTSPOT_LEN) == 0)
	 break;

      if (sscanf(szBuffer, "%d", &num) != 1)
	 break;

      if (num == 0)
	 num = 1;
      AddHotspot(num);

      break;
   }
   return CallWindowProc(lpfnDefEditProc, hwnd, message, wParam, lParam);
}
/************************************************************************/
/*
 * FindHotspot:  Return index of hotspot that has same value as prev_value.  
 *   Defaults to first hotspot if can't match.
 */
int FindHotspot(int prev_value)
{
   int i;
   Bitmap *b;
   int bbg = GetPrimaryBBG();

   b = &BBGs[bbg].Bitmaps[BBGs[bbg].CurrentBitmap];
   for (i=0; i < b->NumHotspots; i++)
      if (abs(b->Hotspots[i].Lap) == abs(prev_value))  // Allow for overlay/underlay differences
	 return i;
   return 0;
}

/************************************************************************/
/*
 * ListBoxRemoveCurrentItem:  Delete currently selected item from list box and return
 *   new selection index, or -1 if none.
 */
int ListBoxRemoveCurrentItem(HWND hList)
{
   int index;
   index = ListBox_GetCurSel(hList);

   SendMessage(hList, LB_DELETESTRING, index, 0);

   index = min(index, ListBox_GetCount(hList) - 1);
   ListBox_SetCurSel(hList, index);
   return index;
}
/************************************************************************/
/*
 * GetPrimaryBBG:  In single bitmap mode, return current BBG; in dual bitmap mode,
 *   return anchor BBG.
 */
int GetPrimaryBBG(void)
{
   if (config.dual_mode)
      return Anchor.BBG;
   return CurrentBBG;
}
