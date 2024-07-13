// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * mailread.c:  Handle receiving mail.
 */

#include "client.h"
#include "mailnews.h"
#include <unordered_map>
#include <time.h>
#include <locale>
#include <iomanip>
#include <iostream>
#include <sstream>

extern HWND hReadMailDlg; /* Non-NULL if Read Mail dialog is up */
extern HWND hSendMailDlg; /* Non-NULL if Send Mail dialog is up */

static RECT dlg_rect; // Screen position of dialog

// Constants for news dialog columns
typedef enum
{
   COL_ORDER = 0,
   COL_FROM = 1,
   COL_SUBJECT = 2,
   COL_TIME = 3
} MailColumns;

// Constants used to track sorted column
static int currentSortColumn = -1;
static bool currentSortAscending = TRUE;

// Hashtable to store converted mail timestamps
static std::unordered_map<int, time_t> mail_date_map;

static ChildPlacement mailread_controls[] = {
    {IDC_MAILLIST, RDI_TOP | RDI_LEFT | RDI_RIGHT},
    {IDC_MAILEDIT, RDI_ALL},
    {0, 0}, // Must end this way
};

/* local function prototypes */
static INT_PTR CALLBACK ReadMailDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void UserMailReply(int msg_num, Bool reply_all);
static void OnColumnClick(LPNMLISTVIEW pLVInfo);
static int CALLBACK CompareMailListItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
static int StringToTimestamp(const char *dateStr);
static void PrepareMailDateMap(HWND hListView);
static void ResetMailListSort(HWND hListView);

/****************************************************************************/
/*
 * UserReadMail:  Tell the server that the user wants to read mail.
 */
void UserReadMail(void)
{
   /* If dialog is already up, raise it to top; else create it */
   if (hReadMailDlg == NULL)
   {
      // Don't ask for new mail; might need to wait for user list
      if (CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAILREAD), NULL, ReadMailDialogProc) == NULL)
         debug(("CreateDialog failed for read mail dialog\n"));
   }
   else
   {
      ShowWindow(hReadMailDlg, SW_SHOWNORMAL);
      SetFocus(hReadMailDlg);

      // Ask server for new mail
      RequestReadMail();
   }
}
/****************************************************************************/
INT_PTR CALLBACK ReadMailDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   static HWND hEdit, hList;
   static int mail_index; /* Number of currently displayed message, -1 if none */
   MailHeader *header;
   int index, msg_num, count;
   char str[MAX_HEADERLINE], msg[MAXMAIL];
   MINMAXINFO *lpmmi;
   LV_COLUMN lvcol;
   LV_ITEM lvitem;
   LV_HITTESTINFO lvhit;
   NM_LISTVIEW *nm;

   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, cinfo->hMain);
      hReadMailDlg = hDlg;

      hEdit = GetDlgItem(hDlg, IDC_MAILEDIT);
      hList = GetDlgItem(hDlg, IDC_MAILLIST);
      SendMessage(hDlg, BK_SETDLGFONTS, 0, 0);

      ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT,
                                          LVS_EX_FULLROWSELECT);

      /* Store dialog rectangle in case of resize */
      GetWindowRect(hDlg, &dlg_rect);

      // Add column headings
      lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
      lvcol.pszText = GetString(hInst, IDS_MHEADER1);
      lvcol.cx = 25;
      ListView_InsertColumn(hList, 0, &lvcol);
      lvcol.pszText = GetString(hInst, IDS_MHEADER2);
      lvcol.cx = 80;
      ListView_InsertColumn(hList, 1, &lvcol);
      lvcol.pszText = GetString(hInst, IDS_MHEADER3);
      lvcol.cx = 150;
      ListView_InsertColumn(hList, 2, &lvcol);
      lvcol.pszText = GetString(hInst, IDS_MHEADER4);
      lvcol.cx = 135;
      ListView_InsertColumn(hList, 3, &lvcol);

      mail_index = -1;

      SetFocus(hReadMailDlg);

      MailGetMessageList();
      RequestReadMail();

      // Load sort arrow bitmaps
      hbmUpArrow = (HBITMAP)LoadImage(hInst,
                                      MAKEINTRESOURCE(IDB_UPARROW), IMAGE_BITMAP, 0, 0,
                                      LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);

      hbmDownArrow = (HBITMAP)LoadImage(hInst,
                                        MAKEINTRESOURCE(IDB_DOWNARROW), IMAGE_BITMAP, 0, 0,
                                        LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);

      PrepareMailDateMap(hList);
      ListView_SetHeaderSortImage(hList, COL_ORDER, FALSE);

      return TRUE;

   case WM_SIZE:
      ResizeDialog(hDlg, &dlg_rect, mailread_controls);
      return TRUE;

   case WM_GETMINMAXINFO:
      lpmmi = (MINMAXINFO *)lParam;
      lpmmi->ptMinTrackSize.x = 200;
      lpmmi->ptMinTrackSize.y = 300;
      return 0;

   case WM_ACTIVATE:
      if (wParam == 0)
         *cinfo->hCurrentDlg = NULL;
      else
         *cinfo->hCurrentDlg = hDlg;
      return TRUE;

   case BK_SETDLGFONTS:
      SetWindowFont(hEdit, GetFont(FONT_MAIL), TRUE);
      SetWindowFont(hList, GetFont(FONT_MAIL), TRUE);
      return TRUE;

   case BK_SETDLGCOLORS:
      ListView_SetTextColor(hList, GetColor(COLOR_LISTFGD));
      ListView_SetBkColor(hList, GetColor(COLOR_LISTBGD));
      InvalidateRect(hDlg, NULL, TRUE);
      return TRUE;

   case EN_SETFOCUS:
      /* By default, whole message becomes selected for some reason */
      Edit_SetSel(hEdit, -1, 0);
      break;

   case BK_NEWMAIL: /* wParam = message number, lParam = message header string */
      msg_num = wParam;
      header = (MailHeader *)lParam;

      // Add message to list view
      if (!IsNameInIgnoreList(header->sender))
      {
         sprintf(str, "%d", msg_num);
         lvitem.mask = LVIF_TEXT | LVIF_PARAM;
         lvitem.iItem = 0;
         lvitem.iSubItem = 0;
         lvitem.pszText = str;
         lvitem.lParam = msg_num;
         ListView_InsertItem(hList, &lvitem);

         // Add subitems
         lvitem.mask = LVIF_TEXT;
         lvitem.iSubItem = 1;
         lvitem.pszText = header->sender;
         ListView_SetItem(hList, &lvitem);
         lvitem.iSubItem = 2;
         lvitem.pszText = header->subject;
         ListView_SetItem(hList, &lvitem);
         lvitem.iSubItem = 3;
         lvitem.pszText = header->date;
         ListView_SetItem(hList, &lvitem);
      }

      // Erase message in status area
      SetDlgItemText(hDlg, IDC_MAILINFO, "");
      return TRUE;

   case BK_NONEWMAIL:
      SetDlgItemText(hDlg, IDC_MAILINFO, GetString(hInst, IDS_NONEWMAIL));
      return TRUE;

      HANDLE_MSG(hDlg, WM_CTLCOLOREDIT, MailCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORLISTBOX, MailCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORSTATIC, MailCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORDLG, MailCtlColor);

      HANDLE_MSG(hDlg, WM_INITMENUPOPUP, InitMenuPopupHandler);

   case WM_CLOSE:
      SendMessage(hDlg, WM_COMMAND, IDCANCEL, 0);
      return TRUE;

   case WM_DESTROY:
      hReadMailDlg = NULL;
      if (exiting)
         PostMessage(cinfo->hMain, BK_MODULEUNLOAD, 0, MODULE_ID);
      return TRUE;

   case WM_NOTIFY:
      if ((((LPNMHDR)lParam)->idFrom == IDC_MAILLIST) &&
          (((LPNMHDR)lParam)->code == LVN_COLUMNCLICK))
         OnColumnClick((LPNMLISTVIEW)lParam);

      if (wParam != IDC_MAILLIST)
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

         msg_num = lvitem.lParam;
         if (msg_num == mail_index)
            break;

         if (MailLoadMessage(msg_num, MAXMAIL, msg) == False)
         {
            ClientError(hInst, hReadMailDlg, IDS_CANTLOADMSG);
            break;
         }

         mail_index = msg_num;
         Edit_SetText(hEdit, msg);
         break;
      }
      return TRUE;

   case WM_COMMAND:
      UserDidSomething();

      switch (GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDC_DELETEMSG:
         if (!ListViewGetCurrentData(hList, &index, &msg_num))
            return TRUE;

         if (MailDeleteMessage(msg_num) == True)
         {
            /* Display new current message, if any */
            Edit_SetText(hEdit, "");
            ListView_DeleteItem(hList, index);

            count = ListView_GetItemCount(hList);
            if (count == 0)
               return TRUE;

            index = min(index, count - 1); // in case last message deleted
            ListView_SetItemState(hList, index, LVIS_SELECTED, LVIS_SELECTED);
         }
         return TRUE;

      case IDC_RESCAN:
         SetDlgItemText(hDlg, IDC_MAILINFO, GetString(hInst, IDS_GETTINGMSGS));
         RequestReadMail();
         ResetMailListSort(hList);
         PrepareMailDateMap(hList);
         return TRUE;

      case IDC_SEND:
         UserSendMail();
         return TRUE;

      case IDC_REPLY:
      case IDC_REPLYALL:
         /* Find message number for currently selected message */
         if (!ListViewGetCurrentData(hList, &index, &msg_num))
            return TRUE;

         UserMailReply(msg_num, (Bool)(GET_WM_COMMAND_ID(wParam, lParam) == IDC_REPLYALL));
         return TRUE;

      case IDCANCEL:
         /* Note:  This code is also used by the WM_CLOSE message */
         MailDeleteMessageList();
         DestroyWindow(hDlg);
         return TRUE;
      }
      break;
   }
   return FALSE;
}
/****************************************************************************/
/*
 * ListViewGetCurrentData:  Set index to index of currently selected item
 *   in given list view control, and data to its lParam value.
 * Return True if a selected item is found, False otherwise.
 */
Bool ListViewGetCurrentData(HWND hList, int *index, int *data)
{
   int i, num;
   LV_ITEM lvitem;

   num = ListView_GetItemCount(hList);

   lvitem.mask = LVIF_STATE | LVIF_PARAM;
   lvitem.iSubItem = 0;
   lvitem.stateMask = LVIS_SELECTED;
   for (i = 0; i < num; i++)
   {
      lvitem.iItem = i;
      ListView_GetItem(hList, &lvitem);

      if (lvitem.state & LVIS_SELECTED)
      {
         *index = i;
         *data = lvitem.lParam;
         return True;
      }
   }
   return False;
}
/****************************************************************************/
/*
 * UserMailReply:  Set up a reply to the given message.  If reply_all is True, reply to
 *   all recipients of given message number; otherwise reply just to sender.
 */
void UserMailReply(int msg_num, Bool reply_all)
{
   MailInfo *reply;

   reply = (MailInfo *)SafeMalloc(sizeof(MailInfo)); // Freed when reply dialog ends

   MailParseMessage(msg_num, reply);

   if (reply_all)
   {
      if (reply->num_recipients >= MAX_RECIPIENTS)
      {
         // If user wants to go ahead anyway, just truncate recipients
         if (!AreYouSure(hInst, hReadMailDlg, NO_BUTTON, IDS_TOOMANYRECIPIENTS, MAX_RECIPIENTS))
            return;
         reply->num_recipients--; // Add sender below
      }

      strcpy(reply->recipients[reply->num_recipients], reply->sender);
      reply->num_recipients++;
   }
   else
   {
      reply->num_recipients = 1;
      strcpy(reply->recipients[0], reply->sender);
   }

   MakeReplySubject(reply->subject, MAX_SUBJECT);
   MailSendReply(hReadMailDlg, reply);
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
   ListView_SortItemsEx(pLVInfo->hdr.hwndFrom, CompareMailListItems, lParamSort);
}

/****************************************************************************/
/*
 * CompareMailListItems: Comparison function for sorting items in the dialog
 * list view.
 */
int CALLBACK CompareMailListItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
   bool bSortAscending = (lParamSort > 0);
   int nColumn = abs(lParamSort) - 1;
   char szBuf1[MAX_SUBJECT], szBuf2[MAX_SUBJECT];
   int time1 = 0;
   int time2 = 0;

   HWND hListView = GetDlgItem(hReadMailDlg, IDC_MAILLIST);

   // Extract the msg_num values from the list view items
   ListView_GetItemText(hListView, lParam1, COL_ORDER, szBuf1, sizeof(szBuf1));
   ListView_GetItemText(hListView, lParam2, COL_ORDER, szBuf2, sizeof(szBuf2));
   int msg_num1 = atoi(szBuf1);
   int msg_num2 = atoi(szBuf2);

   ListView_GetItemText(hListView, lParam1, nColumn, szBuf1, sizeof(szBuf1));
   ListView_GetItemText(hListView, lParam2, nColumn, szBuf2, sizeof(szBuf2));

   switch (nColumn)
   {
   case COL_ORDER:
      return bSortAscending ? atoi(szBuf1) - atoi(szBuf2) : atoi(szBuf2) - atoi(szBuf1);
   case COL_FROM:
      return bSortAscending ? stricmp(szBuf1, szBuf2) : stricmp(szBuf2, szBuf1);
   case COL_SUBJECT:
      return bSortAscending ? stricmp(szBuf1, szBuf2) : stricmp(szBuf2, szBuf1);
   case COL_TIME:
      time1 = mail_date_map[msg_num1];
      time2 = mail_date_map[msg_num2];

      return bSortAscending ? (time1 - time2) : (time2 - time1);
   default:
      debug(("Unhandled column (column #%d) in CompareMailListItems\n", nColumn));
      return 0;
   }
}

/***************************************************************************/
/*
 * StringToTimestamp converts mail date strings to integers for comparison
 */
/***************************************************************************/
int StringToTimestamp(const char *dateStr)
{
   struct tm tm = {0};

   const char *localeName = setlocale(LC_ALL, "");

   // Skip the day of the week
   const char *remainingStr = dateStr + 4; // Skip first 4 characters ("Sat ")

   std::istringstream input(remainingStr);

   input.imbue(std::locale(localeName));

   input >> std::get_time(&tm, "%b %d, %Y %H:%M");

   if (input.fail())
   {
      std::cerr << "Failed to parse date string: " << dateStr << std::endl;
      return 0;
   }

   time_t timestamp = mktime(&tm);

   return static_cast<int>(timestamp);
}

/***************************************************************************/
/*
 * PrepareMailDateMap caches converted mail date string versions for sorting
 */
/***************************************************************************/
void PrepareMailDateMap(HWND hListView)
{
   mail_date_map.clear();
   int itemCount = ListView_GetItemCount(hListView);
   char szBuf[MAX_SUBJECT];
   char orderBuf[30];

   for (int i = 0; i < itemCount; ++i)
   {
      // Retrieve the value from the first column (COL_ORDER)
      ListView_GetItemText(hListView, i, COL_ORDER, orderBuf, sizeof(orderBuf));
      int msg_num = atoi(orderBuf);

      // Retrieve the time string from the time column (COL_TIME)
      ListView_GetItemText(hListView, i, COL_TIME, szBuf, sizeof(szBuf));
      int timestamp = StringToTimestamp(szBuf);

      // Store the timestamp in the hash table
      mail_date_map[msg_num] = timestamp;
   }
}

/***************************************************************************/
/*
 * ResetMailListSort Resets the list sort order and sort indicator image
 */
/***************************************************************************/
void ResetMailListSort(HWND hListView)
{
   // 1-based column
   ListView_SortItemsEx(hListView, CompareMailListItems, -(COL_ORDER + 1));
   // 0-based column
   ListView_SetHeaderSortImage(hListView, COL_ORDER, false);
}