// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * newsread.c:  Allow user to read news (articles in bulletin boards).
 */

#include "client.h"
#include "mailnews.h"
#include "string.h"

// Named constant for buffer size
static const int MAX_HEADER_TEXT = 256;

HWND hReadNewsDialog = NULL;

// Declare global HBITMAP objects
static HBITMAP hbmUpArrow = NULL;
static HBITMAP hbmDownArrow = NULL;

/* Stuff for making raw times into human-readable times */
static int months[] =
    {IDS_JANUARY, IDS_FEBRUARY, IDS_MARCH, IDS_APRIL, IDS_MAY, IDS_JUNE, IDS_JULY,
     IDS_AUGUST, IDS_SEPTEMBER, IDS_OCTOBER, IDS_NOVEMBER, IDS_DECEMBER};
static int weekdays[] =
    {IDS_SUNDAY, IDS_MONDAY, IDS_TUESDAY, IDS_WEDNESDAY, IDS_THURSDAY, IDS_FRIDAY, IDS_SATURDAY};

static list_type new_articles = NULL; /* Build up new articles arriving from server */

static RECT dlg_rect; // Screen position of dialog

// Constants for news dialog columns
typedef enum
{
   COL_TITLE = 0,
   COL_POSTER = 1,
   COL_TIME = 2
} NewsColumns;

// Constants used to track sorted column
static int currentSortColumn = -1;
static bool currentSortAscending = TRUE;

static ChildPlacement newsread_controls[] = {
    {IDC_NEWSEDIT, RDI_ALL},
    {IDC_NEWSLIST, RDI_RIGHT | RDI_LEFT | RDI_TOP},
    {0, 0}, // Must end this way
};

/* local function prototypes */
static INT_PTR CALLBACK ReadNewsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void UserReplyNewsMail(NewsArticle *article);
static void OnColumnClick(LPNMLISTVIEW pLVInfo);
static int CALLBACK CompareListItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
static void ListView_SetHeaderSortImage(HWND hListView, int sortedColumn, BOOL sortAscending);
static void ResetListSort(HWND hListView);

/****************************************************************************/
/*
 * UserReadNews:  Bring up read news dialog with index of articles.
 *   obj is object node for newsgroup.
 *   desc is text description of newsgroup.
 *   newsgroup is the newsgroup id.
 *   permissions gives the permissions of the newsgroup.
 */
void UserReadNews(object_node *obj, char *desc, WORD newsgroup, BYTE permissions)
{
   ReadNewsDialogStruct s;

   if (hReadNewsDialog != NULL)
      return;

   s.newsgroup = newsgroup;
   s.group_name_rsc = obj->name_res;
   s.permissions = permissions;
   s.desc = desc;

   DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_NEWSREAD), cinfo->hMain,
                  ReadNewsDialogProc, (LPARAM)&s);
}
/****************************************************************************/
/*
 * ReceiveArticles:  Display index in news dialog, if it's up.
 *   Articles may arrive in multiple messages from the server; this group
 *   of articles is part "part" out of "max_part" parts.
 */
void ReceiveArticles(WORD newsgroup, BYTE part, BYTE max_part, list_type articles)
{
   static int last_part = 0; /* Last part we received */

   // Make sure that parts are arriving in order
   if (hReadNewsDialog == NULL || part != last_part + 1)
   {
      list_destroy(articles);
      last_part = 0;
      return;
   }

   last_part = part;

   if (part == 1)
      new_articles = NULL;

   new_articles = list_append(new_articles, articles);

   if (part == max_part)
   {
      SendMessage(hReadNewsDialog, BK_ARTICLES, 0, (LPARAM)new_articles);
      last_part = 0;
   }
}
/****************************************************************************/
/*
 * UserReadArticle:  Display contents of article in read news dialog.
 */
void UserReadArticle(char *article)
{
   if (hReadNewsDialog == NULL)
      return;

   SendMessage(hReadNewsDialog, BK_ARTICLE, 0, (LPARAM)article);
}
/****************************************************************************/
INT_PTR CALLBACK ReadNewsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   static ReadNewsDialogStruct *info = NULL;
   static HWND hEdit = NULL;
   static HWND hList = NULL;
   static int article_index = -1;
   static int lastRequestedIndex = -1;

   list_type l;
   NewsArticle *article;
   int index;
   MINMAXINFO *lpmmi;
   LV_COLUMN lvcol;
   LV_ITEM lvitem;
   LV_HITTESTINFO lvhit;
   NM_LISTVIEW *nm;

   char date[MAXDATE], title[MAX_SUBJECT + MAXDATE + MAXNAME + 10];

   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, cinfo->hMain);
      info = (ReadNewsDialogStruct *)lParam;

      hList = GetDlgItem(hDlg, IDC_NEWSLIST);
      hEdit = GetDlgItem(hDlg, IDC_NEWSEDIT);

      ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT,
                                          LVS_EX_FULLROWSELECT);

      SetWindowFont(hEdit, GetFont(FONT_MAIL), FALSE);
      SetWindowFont(hList, GetFont(FONT_MAIL), FALSE);

      /* Store dialog rectangle in case of resize */
      GetWindowRect(hDlg, &dlg_rect);

      // Set newsgroup description and window title
      SetWindowText(GetDlgItem(hDlg, IDC_NEWSDESC), info->desc);
      GetWindowText(hDlg, title, MAXNAME); // Append group name to window title
      strncat(title, LookupNameRsc(info->group_name_rsc), MAX_SUBJECT);
      SetWindowText(hDlg, title);

      EnableWindow(GetDlgItem(hDlg, IDC_REPLY), FALSE);
      EnableWindow(GetDlgItem(hDlg, IDC_REPLYMAIL), FALSE);

      // Add column headings
      lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
      lvcol.pszText = GetString(hInst, IDS_NHEADER1);
      lvcol.cx = 190;
      ListView_InsertColumn(hList, 0, &lvcol);
      lvcol.pszText = GetString(hInst, IDS_NHEADER2);
      lvcol.cx = 80;
      ListView_InsertColumn(hList, 1, &lvcol);
      lvcol.pszText = GetString(hInst, IDS_NHEADER3);
      lvcol.cx = 135;
      ListView_InsertColumn(hList, 2, &lvcol);

      article_index = -1;
      info->articles = NULL;

      // Ask for articles if we have read permission
      if (info->permissions & NEWS_READ)
         RequestArticles(info->newsgroup);
      else
         EnableWindow(GetDlgItem(hDlg, IDC_RESCAN), FALSE);

      // Disable post button if we don't have permission
      if (!(info->permissions & NEWS_POST))
         EnableWindow(GetDlgItem(hDlg, IDC_NEWSPOST), FALSE);

      SetTimer(hDlg, 1, 100, NULL);

      hReadNewsDialog = hDlg;

      // Load sort arrow bitmaps
      hbmUpArrow = (HBITMAP)LoadImage(hInst,
                                      MAKEINTRESOURCE(IDB_UPARROW), IMAGE_BITMAP, 0, 0,
                                      LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);

      hbmDownArrow = (HBITMAP)LoadImage(hInst,
                                        MAKEINTRESOURCE(IDB_DOWNARROW), IMAGE_BITMAP, 0, 0,
                                        LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);

      return TRUE;

   case WM_SIZE:
      ResizeDialog(hDlg, &dlg_rect, newsread_controls);
      return TRUE;

   case WM_GETMINMAXINFO:
      lpmmi = (MINMAXINFO *)lParam;
      lpmmi->ptMinTrackSize.x = 200;
      lpmmi->ptMinTrackSize.y = 300;
      return 0;

   case BK_ARTICLES:
      /* Get rid of old index, if any */
      if (info->articles != NULL)
         list_destroy(info->articles);

      info->articles = (list_type)lParam;

      /* Put article title lines in list box; save old position if appropriate */
      index = -1;
      if (ListView_GetItemCount(hList) > 0)
         if (!ListViewGetCurrentData(hList, &index, (int *)&article))
            index = -1;
      ListView_DeleteAllItems(hList);
      for (l = info->articles; l != NULL; l = l->next)
      {
         article = (NewsArticle *)(l->data);

         // Add article to list view
         if (!IsNameInIgnoreList(article->poster))
         {
            lvitem.mask = LVIF_TEXT | LVIF_PARAM;
            lvitem.iItem = ListView_GetItemCount(hList);
            lvitem.iSubItem = 0;
            lvitem.pszText = article->title;
            lvitem.lParam = (LPARAM)article;
            ListView_InsertItem(hList, &lvitem);

            // Add subitems
            lvitem.mask = LVIF_TEXT;
            lvitem.iSubItem = 1;
            lvitem.pszText = article->poster;
            ListView_SetItem(hList, &lvitem);

            DateFromSeconds(article->time, date);
            lvitem.iSubItem = 2;
            lvitem.pszText = date;
            ListView_SetItem(hList, &lvitem);
         }
      }

      /* Get first article by faking select message */
      if (index == -1)
      {
         ListView_SetItemState(hList, 0, LVIS_SELECTED, LVIS_SELECTED);
      }
      else
      {
         ListView_SetItemState(hList, index, LVIS_SELECTED, LVIS_SELECTED);
         ListView_EnsureVisible(hList, index, FALSE);
      }

      ListView_SetHeaderSortImage(hList, COL_TIME, FALSE);

      return TRUE;

   case BK_ARTICLE:
      /* Display article's text */
      SetWindowText(hEdit, (char *)lParam);
      if (info->permissions & NEWS_POST)
         EnableWindow(GetDlgItem(hDlg, IDC_REPLY), TRUE);
      EnableWindow(GetDlgItem(hDlg, IDC_REPLYMAIL), TRUE);
      SetTimer(hDlg, 1, 1000, NULL);
      return TRUE;

      HANDLE_MSG(hDlg, WM_CTLCOLOREDIT, MailCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORLISTBOX, MailCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORSTATIC, MailCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORDLG, MailCtlColor);

   case WM_NOTIFY:
      if ((((LPNMHDR)lParam)->idFrom == IDC_NEWSLIST) &&
          (((LPNMHDR)lParam)->code == LVN_COLUMNCLICK))
         OnColumnClick((LPNMLISTVIEW)lParam);

      if (wParam != IDC_NEWSLIST)
         return TRUE;

      nm = (NM_LISTVIEW *)lParam;

      switch (nm->hdr.code)
      {
      case NM_CLICK:
         // If you click on an item, select it--why doesn't control work this way by default?
         GetCursorPos(&lvhit.pt);
         ScreenToClient(hList, &lvhit.pt);
         lvhit.pt.x = 10;
         index = ListView_HitTest(hList, &lvhit);
         if (index == -1)
            break;

         ListView_SetItemState(hList, index,
                               LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
         break;

      case LVN_ITEMCHANGED:
         // New item selected; get its message number
         lvitem.mask = LVIF_STATE | LVIF_PARAM;
         lvitem.stateMask = LVIS_SELECTED;
         lvitem.iItem = nm->iItem;
         lvitem.iSubItem = 0;
         ListView_GetItem(hList, &lvitem);

         if (!(lvitem.state & LVIS_SELECTED))
            break;

         article = (NewsArticle *)lvitem.lParam;
         if (article->num == article_index)
            break;

         lastRequestedIndex = article->num;

         break;
      }
      return TRUE;

   case WM_TIMER:
      if (article_index != lastRequestedIndex)
      {
         article_index = lastRequestedIndex;
         RequestArticle(info->newsgroup, article_index);
      }
      SetTimer(hDlg, 1, 100, NULL);
      break;

   case WM_DESTROY:
      hReadNewsDialog = NULL;
      KillTimer(hDlg, 1);
      if (exiting)
         PostMessage(cinfo->hMain, BK_MODULEUNLOAD, 0, MODULE_ID);
      return TRUE;

   case WM_COMMAND:
      UserDidSomething();

      switch (GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDC_REPLY:
         if (!ListViewGetCurrentData(hList, &index, (int *)&article))
            break;

         /* If user posts article, rescan so that it will show up */
         if (UserPostArticle(hDlg, info->newsgroup, info->group_name_rsc, article->title))
            RequestArticles(info->newsgroup);
         SetFocus(hList);
         return TRUE;

      case IDC_REPLYMAIL:
         if (!ListViewGetCurrentData(hList, &index, (int *)&article))
            break;

         UserReplyNewsMail(article);
         return TRUE;

      case IDC_NEWSPOST:
         /* If user posts article, rescan so that it will show up */
         if (UserPostArticle(hDlg, info->newsgroup, info->group_name_rsc, NULL))
            RequestArticles(info->newsgroup);
         ResetListSort(hList);
         SetFocus(hList);
         return TRUE;

      case IDC_RESCAN:
         RequestArticles(info->newsgroup);
         ResetListSort(hList);
         SetFocus(hList);
         return TRUE;

      case IDOK:
      case IDCANCEL:
         info->articles = list_destroy(info->articles);
         new_articles = NULL;
         EndDialog(hDlg, 0);
         return TRUE;
      }
   }
   return FALSE;
}
/****************************************************************************/
/*
 * UserReplyNewsMail:  User wants to reply to given article's author by mail.
 */
void UserReplyNewsMail(NewsArticle *article)
{
   MailInfo *reply;

   reply = (MailInfo *)SafeMalloc(sizeof(MailInfo)); // Freed when reply dialog ends

   reply->num_recipients = 1;
   strcpy(reply->recipients[0], article->poster);
   strcpy(reply->subject, article->title);
   MakeReplySubject(reply->subject, MAX_SUBJECT);
   MailSendReply(hReadNewsDialog, reply);
}
/****************************************************************************/
/*
 * DateFromSeconds:  Given a time in minutes since midnight, Jan. 1, 1996 UT,
 *   fill in str to contain a string describing the date and time, such as
 *   "Dec 25, 1994 18:00".
 *   str must have length at least MAXDATE.
 *   Returns True iff minutes is legal, i.e. positive.
 */
Bool DateFromSeconds(long seconds, char *str)
{
   struct tm *t;
   time_t local_time = seconds;

   local_time += 1534000000L; // Offset to sometime in mid-2018
   t = localtime(&local_time);

   if (t == NULL)
      return False;

   snprintf(str, MAXDATE, "%s %s %.2ld, %.4ld %.2ld:%.2ld",
            GetString(hInst, weekdays[t->tm_wday]), GetString(hInst, months[t->tm_mon]),
            t->tm_mday, t->tm_year + 1900, t->tm_hour, t->tm_min);
   return True;
}

void OnColumnClick(LPNMLISTVIEW pLVInfo)
{
   int nSortColumn = pLVInfo->iSubItem;
   bool bSortAscending = TRUE;

   if (nSortColumn == currentSortColumn)
   {
      // Toggle sort direction if the same column is clicked again
      currentSortAscending = !currentSortAscending;
   }
   else
   {
      // Set ascending order for new column
      currentSortColumn = nSortColumn;
      currentSortAscending = TRUE;
   }

   LPARAM lParamSort = (currentSortAscending ? 1 : -1) * (currentSortColumn + 1);

   if (hbmUpArrow != NULL && hbmDownArrow != NULL)
   {
      ListView_SetHeaderSortImage(pLVInfo->hdr.hwndFrom, currentSortColumn, currentSortAscending);
   }

   // Sort list
   ListView_SortItems(pLVInfo->hdr.hwndFrom, CompareListItems, lParamSort);
}

/****************************************************************************/
/*
 * CompareListItems: Comparison function for sorting items in the news dialog
 * list view.
 */
int CALLBACK CompareListItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
   bool bSortAscending = (lParamSort > 0);
   int nColumn = abs(lParamSort) - 1;

   NewsArticle *article1 = (NewsArticle *)lParam1;
   NewsArticle *article2 = (NewsArticle *)lParam2;

   // `ListView_SortItems` is unstable, so we append the article index for stability
   char title1[MAX_SUBJECT + 10];
   snprintf(title1, sizeof(title1), "%s %d", article1->title, article1->num);
   char title2[MAX_SUBJECT + 10];
   snprintf(title2, sizeof(title2), "%s %d", article2->title, article2->num);

   char poster1[MAXUSERNAME + 10];
   snprintf(poster1, sizeof(poster1), "%s %d", article1->poster, article1->num);
   char poster2[MAXUSERNAME + 10];
   snprintf(poster2, sizeof(poster2), "%s %d", article2->poster, article2->num);

   switch (nColumn)
   {
   case COL_TITLE:
      return bSortAscending ? stricmp(title1, title2) : stricmp(title2, title1);
   case COL_POSTER:
      return bSortAscending ? stricmp(poster1, poster2) : stricmp(poster2, poster1);
   case COL_TIME:
      return bSortAscending ? article1->time - article2->time : article2->time - article1->time;
   default:
      debug(("Unhandled column (column #%d) in CompareListItems\n", nColumn));
      return 0;
   }
}

void ListView_SetHeaderSortImage(HWND hListView, int sortedColumn, BOOL sortAscending)
{
   // Get the handle to the header control associated with the list view
   HWND hHeader = ListView_GetHeader(hListView);
   if (!hHeader)
      return;

   int columnCount = Header_GetItemCount(hHeader);

   for (int i = 0; i < columnCount; i++)
   {
      HDITEM hdi = {0};
      hdi.mask = HDI_FORMAT | HDI_BITMAP;

      // Get the current format of the header item
      Header_GetItem(hHeader, i, &hdi);

      if (i == sortedColumn)
      {
         hdi.fmt |= (HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
         hdi.hbm = sortAscending ? hbmUpArrow : hbmDownArrow;
      }
      else
      {
         hdi.fmt &= ~(HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
         hdi.hbm = NULL;
      }

      // Set the header item at the current index with the updated details
      Header_SetItem(hHeader, i, &hdi);
   }
}

/*
 * Resets the news list to its default sort: by time, descending
 */
void ResetListSort(HWND hListView)
{
   // 1-based column
   ListView_SortItems(hListView, CompareListItems, -(COL_TIME + 1));
   // 0-based column
   ListView_SetHeaderSortImage(hListView, COL_TIME, FALSE);
}