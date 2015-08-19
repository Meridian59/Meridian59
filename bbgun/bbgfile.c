// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * bbgfile:  Deal with loading and saving BBG files.
 */

#include "bbgun.h"

// Flags for choosing which BBGs to save
#define SAVE_ANCHOR  0x1
#define SAVE_OVERLAY 0x2

static int last_flags = SAVE_ANCHOR | SAVE_OVERLAY;

extern int CurrentBitmap;
extern int CurrentHotspot;
extern int CurrentBBG;
extern int CurrentGroup;
extern int CurrentGroupMember;
extern int NumBBGs;

static BOOL SaveBBGFile(int bbg, char *filename);
static BOOL CALLBACK SaveBBGDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

static char *BBG_Filter = "Command files\0*.bbg\0All Files\0*.*\0\0";
/************************************************************************/
void OpenBBG(void)
{
   int i, j, k;
   OPENFILENAME ofn;
   FILE* fp;
   int ch;
   char szBuffer[MAX_PATH];

   if (NumBBGs == MAX_BBGS)
      return;

   BBGs[NumBBGs].FullPath[0] = '\0';
   BBGs[NumBBGs].Title[0] = '\0';
   memset( &ofn, 0, sizeof(OPENFILENAME) );
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hwndOwner = hMain;
   ofn.lpstrFilter = BBG_Filter;
   ofn.lpstrFile = BBGs[NumBBGs].FullPath;
   ofn.nMaxFile = MAX_PATH;
   ofn.lpstrFileTitle = BBGs[NumBBGs].Title;
   ofn.nMaxFileTitle = 50;
   ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

   if( !GetOpenFileName(&ofn) )
   {
      MessageBeep(0);
      SetFocus(hMain);
      return;
   }
   SendDlgItemMessage(hMain, IDBBGLIST, LB_ADDSTRING, 0,
		      (LPARAM)((LPCSTR)BBGs[NumBBGs].Title));
   fp=fopen(BBGs[NumBBGs].FullPath, "r");

   if (fp == NULL)
   {
      MessageBox(hMain, "Unable to open BBG file for reading!\n", szAppName, MB_OK);
      return;
   }

   BBGs[NumBBGs].Comment[0] = '\0';
   BBGs[NumBBGs].Shrink = 1;

   // Read comments
   while ((ch = getc(fp)) == '#')
   {
      if (fgets(szBuffer, 200, fp) == NULL)
	 break;
      szBuffer[strlen(szBuffer) - 1] = 0;  // Kill newline
      strcat(BBGs[NumBBGs].Comment, szBuffer + 1);  // Skip comment character
      strcat(BBGs[NumBBGs].Comment, "\r\n");
   }
   ungetc(ch, fp);

   fscanf(fp, " ");
   fscanf(fp, "-s%hhd ", &BBGs[NumBBGs].Shrink);

   fscanf(fp, "%d", &BBGs[NumBBGs].NumBitmaps);
   for (i = 0; i < BBGs[NumBBGs].NumBitmaps;i++)
   {
      Bitmap *b;

      b = &BBGs[NumBBGs].Bitmaps[i];

      memset(b, 0, sizeof(Bitmap));
      fscanf(fp,"%s ",b->Title);

      for (k = 0; k < 2; k++)
      {
	 fscanf(fp,"[%d,%d] ", &b->XOffset, &b->YOffset);
	 
	 if ( fscanf(fp,":%hhd",&b->NumHotspots))
	    for (j = 0; j < b->NumHotspots; j++)
	       fscanf(fp,"%d [%d,%d]",&b->Hotspots[j].Lap,
		      &b->Hotspots[j].X, &b->Hotspots[j].Y);
      }
   }

   fscanf(fp,"%d",&BBGs[NumBBGs].NumGroups);

   for (i = 0; i < BBGs[NumBBGs].NumGroups; i++)
   {
      fscanf(fp,"%d",&BBGs[NumBBGs].Groups[i].Members);
      for (j = 0; j < BBGs[NumBBGs].Groups[i].Members; j++)
	 fscanf(fp,"%d",&BBGs[NumBBGs].Groups[i].Indices[j]);
   }
   fclose(fp);

   CurrentBitmap = 0;
   BBGs[CurrentBBG].CurrentBitmap = CurrentBitmap;
   CurrentBBG = NumBBGs;
   CurrentGroup = 0;
   CurrentGroupMember = 0;
   CurrentHotspot = 0;
   BBGs[CurrentBBG].CurrentHotspot = CurrentHotspot;
   BBGs[CurrentBBG].changed = FALSE;
   NumBBGs++;

   FillBitmapList();
   FillHotlist();
   FillGroupList();
   FillGroupiesList();

   SendDlgItemMessage(hMain, IDBBGLIST, LB_SETCURSEL, (LPARAM) CurrentBBG, 0L);

   DrawIt();
}
/************************************************************************/
void SaveBBG(void)
{
   int flags;
   int bbg = GetPrimaryBBG();

   // If in dual bitmap mode, ask to save both anchor and wanderer
   if (config.dual_mode)
   {
      flags = DialogBox(hInst, MAKEINTRESOURCE(IDD_SAVE), hMain, SaveBBGDialogProc);

      if (flags == 0)
	 return;

      if (flags & SAVE_ANCHOR)
	 SaveBBGFile(Anchor.BBG, BBGs[Anchor.BBG].Title);
      if (flags & SAVE_OVERLAY)
	 SaveBBGFile(WanderOne.BBG, BBGs[WanderOne.BBG].Title);
      last_flags = flags;
   }
   else SaveBBGFile(bbg, BBGs[bbg].Title);
}
/************************************************************************/
/*
 * SaveBBGFile:  Save given BBG number to given file.
 *   Return TRUE on success.
 */
BOOL SaveBBGFile(int bbg, char *filename)
{
   int i, j;
   OPENFILENAME ofn;
   FILE* fp;
   char szBuffer2[MAX_COMMENT], *ptr;
   szBuffer2[0] = '\0';

   memset( &ofn, 0, sizeof(OPENFILENAME) );

   ofn.lStructSize = sizeof(OPENFILENAME );
   ofn.hwndOwner = hMain;
   ofn.lpstrFilter = BBG_Filter;
   ofn.lpstrFile = filename;
   ofn.nMaxFile = MAX_PATH;
   ofn.lpstrFileTitle = szBuffer2;
   ofn.nMaxFileTitle = MAX_PATH;
   ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

   if( !GetSaveFileName(&ofn))
      return FALSE;

   fp = fopen(filename, "w");
   if (fp == NULL)
   {
      wsprintf(szBuffer2, "Unable to open BBG file %s for writing!\n", filename);
      MessageBox(hMain, szBuffer2, szAppName, MB_OK);
      return FALSE;
   }

   // Write out comments
   strcpy(szBuffer2, BBGs[bbg].Comment);
   ptr = strtok(szBuffer2, "\r\n");
   while (ptr != NULL)
   {
      fprintf(fp, "# %s\n", ptr);
      ptr = strtok(NULL, "\r\n");
   }

   if (BBGs[bbg].Shrink > 1)
      fprintf(fp,"-s %d\n",BBGs[bbg].Shrink);
   fprintf(fp, "%d\n", BBGs[bbg].NumBitmaps);

   for (i = 0; i < BBGs[bbg].NumBitmaps; i++)
   {
      fprintf(fp,"%-15s",BBGs[bbg].Bitmaps[i].Title);
      fprintf(fp,"[%d, %d] ",BBGs[bbg].Bitmaps[i].XOffset,BBGs[bbg].Bitmaps[i].YOffset); 
      
      if (BBGs[bbg].Bitmaps[i].NumHotspots > 0)
      {
	 fprintf(fp, ":%-3d ",BBGs[bbg].Bitmaps[i].NumHotspots);
	 for (j = 0;j < BBGs[bbg].Bitmaps[i].NumHotspots; j++)
	    fprintf(fp, " %3d [%d, %d]",
		    BBGs[bbg].Bitmaps[i].Hotspots[j].Lap,
		    BBGs[bbg].Bitmaps[i].Hotspots[j].X,
		    BBGs[bbg].Bitmaps[i].Hotspots[j].Y);
	 
      }
      fprintf(fp,"\n");
   }
   if  (BBGs[bbg].NumGroups > 0)
   {
      fprintf(fp, "%d\n", BBGs[bbg].NumGroups);
      for (i = 0;i < BBGs[bbg].NumGroups; i++)
      {
	 fprintf(fp,"%2d   ",BBGs[bbg].Groups[i].Members);
	 for (j = 0; j < BBGs[bbg].Groups[i].Members; j++)
	    fprintf(fp,"%4d",BBGs[bbg].Groups[i].Indices[j]);
	 fprintf(fp,"\n");
      }
   }
   fclose(fp);

   BBGs[bbg].changed = FALSE;

   return TRUE;
}
/************************************************************************/
void CloseBBG(void)
{
   int i;
   PDIB pdib;
   HWND hList;

   if (NumBBGs == 0)
      return;

   if (BBGs[CurrentBBG].changed &&
       MessageBox(hMain, "This BBG has changed; are you sure you want to close it?",
		  "Confirm BBG close", MB_YESNO) == IDNO)
      return;

   hList = GetDlgItem(hMain, IDBBGLIST);

   // Free bitmaps
   for (i=0; i < BBGs[CurrentBBG].NumBitmaps; i++)
   {
      pdib = BBGs[CurrentBBG].Bitmaps[i].pdib;

      if (pdib != NULL)
	 DibFree(pdib);
   }

   NumBBGs--;
   for (i = CurrentBBG; i < NumBBGs; i++)
      BBGs[i] = BBGs[i + 1];

   CurrentBBG = ListBoxRemoveCurrentItem(hList);
   if (CurrentBBG == -1)
      CurrentBBG = 0;

   FillBitmapList();
   FillHotlist();
   FillGroupList();
   FillGroupiesList();

   WanderOne.BBG = -1;
   Anchor.BBG = -1;
   SetDlgItemText(hMain, ANCHORTEXT, " ");
   SetDlgItemText(hMain, WANDER1TEXT, " ");
   DrawIt();
}
/************************************************************************/
BOOL CALLBACK SaveBBGDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
   int flags;

   switch(msg)
   {
   case WM_INITDIALOG:
      SetDlgItemText(hDlg, IDC_ANCHORNAME, BBGs[Anchor.BBG].Title);
      SetDlgItemText(hDlg, IDC_OVERLAYNAME, BBGs[WanderOne.BBG].Title);

      if (last_flags & SAVE_ANCHOR)
	 SendDlgItemMessage(hDlg, IDC_SAVEANCHOR, BM_SETCHECK, 1, 0);
      if (last_flags & SAVE_OVERLAY)
	 SendDlgItemMessage(hDlg, IDC_SAVEOVERLAY, BM_SETCHECK, 1, 0);
      return TRUE;

   case WM_COMMAND:
      switch(wParam)
      {
      case IDOK:
	 flags = 0;

	 if (SendDlgItemMessage(hDlg, IDC_SAVEANCHOR, BM_GETCHECK, 0, 0))
	    flags |= SAVE_ANCHOR;
	 if (SendDlgItemMessage(hDlg, IDC_SAVEOVERLAY, BM_GETCHECK, 0, 0))
	    flags |= SAVE_OVERLAY;

	 EndDialog(hDlg, flags);
	 return TRUE;
	 
      case IDCANCEL:
	 EndDialog(hDlg, 0);
	 return TRUE;
      }
      break;
   }
   return FALSE;
}
