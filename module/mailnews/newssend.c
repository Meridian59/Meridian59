// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * newssend.c:  Allow user to post a news article.
 */

#include "client.h"
#include "mailnews.h"

static HWND hPostNewsDialog = NULL;
extern HWND hReadNewsDialog;

static RECT  dlg_rect;        // Screen position of dialog

static ChildPlacement newssend_controls[] = {
{ IDC_NEWSEDIT,   RDI_ALL },
{ IDC_SUBJECT,    RDI_LEFT | RDI_RIGHT | RDI_TOP},
{ IDOK,           RDI_LEFT | RDI_HPIN | RDI_BOTTOM },
{ IDCANCEL,       RDI_LEFT | RDI_HPIN | RDI_BOTTOM },
{ 0,              0 },   // Must end this way
};

BOOL CALLBACK PostNewsDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

/*****************************************************************************/
/*
 * UserPostArticle:  User wants to post an article to given newsgroup.
 *   Pop up article posting dialog.
 *   title should be NULL to have no default subject line.
 *   Returns True iff user actually posted article.
 */
Bool UserPostArticle(HWND hParent, WORD newsgroup, ID name_rsc, char *title)
{
   PostNewsDialogStruct s;
   int retval;

   if (hPostNewsDialog != NULL)
      return False;

   s.newsgroup = newsgroup;
   s.subject = title;
   s.group_name_rsc = name_rsc;
   retval = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_NEWSPOST), hParent,
			   PostNewsDialogProc, (LPARAM) &s);

   if (retval == IDOK)
      return True;
   return False;
}
/****************************************************************************/
BOOL CALLBACK PostNewsDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   static HWND hEdit, hSubject;
   static PostNewsDialogStruct *info;
   char *buf, subject[MAX_SUBJECT];
   int len;
   MINMAXINFO *lpmmi;
   
   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, cinfo->hMain);
      info = (PostNewsDialogStruct *) lParam;

      hEdit = GetDlgItem(hDlg, IDC_NEWSEDIT);
      hSubject = GetDlgItem(hDlg, IDC_SUBJECT);

      Edit_LimitText(hEdit, MAXARTICLE);
      SetWindowFont(hEdit, GetFont(FONT_MAIL), TRUE);

      Edit_LimitText(hSubject, MAX_SUBJECT - 1);
      SetWindowFont(hSubject, GetFont(FONT_MAIL), TRUE);

      /* Store dialog rectangle in case of resize */
      GetWindowRect(hDlg, &dlg_rect);

      /* Set default subject if appropriate */
      if (info->subject != NULL)
      {
	 MakeReplySubject(info->subject, MAX_SUBJECT);
	 Edit_SetText(hSubject, info->subject);
	 SetFocus(hEdit);
      }
      else SetFocus(hSubject);

      /* Display group name */
      SetWindowText(GetDlgItem(hDlg, IDC_GROUPNAME), 
		    LookupNameRsc(info->group_name_rsc));
      
      EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
      hPostNewsDialog = hDlg;
      return FALSE;

      HANDLE_MSG(hDlg, WM_CTLCOLOREDIT, MailCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORLISTBOX, MailCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORSTATIC, MailCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORDLG, MailCtlColor);

   case WM_SIZE:
      ResizeDialog(hDlg, &dlg_rect, newssend_controls);
      return TRUE;      

   case WM_GETMINMAXINFO:
      lpmmi = (MINMAXINFO *) lParam;
      lpmmi->ptMinTrackSize.x = 200;
      lpmmi->ptMinTrackSize.y = 300;
      return 0;

   case WM_DESTROY:
      hPostNewsDialog = NULL;
      if (exiting)
	 PostMessage(cinfo->hMain, BK_MODULEUNLOAD, 0, MODULE_ID);
      return TRUE;

   case WM_COMMAND:
      UserDidSomething();

      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDC_NEWSEDIT:
	 if (GET_WM_COMMAND_CMD(wParam, lParam) != EN_CHANGE)
	    return TRUE;

	 /* Only allow user to post when article is nonempty */
	 if (Edit_GetTextLength(hEdit) > 0)
	    EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
	 else EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
	    
	 return TRUE;

      case IDOK:
	 /* Get title and article; post article */
	 Edit_GetText(hSubject, subject, MAX_SUBJECT);
	 
	 len = Edit_GetTextLength(hEdit);
	 buf = (char *) SafeMalloc(len + 1);
	 Edit_GetText(hEdit, buf, len + 1);
	 SendArticle(info->newsgroup, subject, buf);
	
	 SafeFree(buf);
	 EndDialog(hDlg, IDOK);
	 return TRUE;

      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;
      }
   }
   return FALSE;
}
/****************************************************************************/
/*
 * MakeReplySubject:  Add "Re:" to the front of the given subject string.
 *   The string is modified in place, and will contain no more than max_chars
 *   characters.
 */
void MakeReplySubject(char *subject, int max_chars)
{
   char *re_string, *en_re_string, *de_re_string;
   int re_len, en_re_len, de_re_len;

   // Need to check each of the currently implemented reply strings.
   en_re_string = "Re: ";
   de_re_string = "Aw: ";
   en_re_len = strlen(en_re_string);
   de_re_len = strlen(de_re_string);

   /* Skip stupid case */
   if (en_re_len >= max_chars || de_re_len >= max_chars)
      return;
   
   /* Add "Re: " to the beginning if appropriate */
   if (strnicmp(subject, en_re_string, en_re_len)
         && strnicmp(subject, de_re_string, de_re_len))
   {
      // If neither English or German reply string is present, add one.
      // String added depends on which language the user has enabled.
      re_string = GetString(hInst, IDS_RE);
      re_len = strlen(re_string);

      memmove(subject + re_len, subject, min((int) strlen(subject) + 1, max_chars - re_len));
      memcpy(subject, re_string, re_len);
      subject[max_chars - 1] = 0;
   }
}
/****************************************************************************/
/*
 * AbortNewsDialogs:  Kill off any news dialogs that are up.  Return True iff any was up.
 */
Bool AbortNewsDialogs(void)
{
   Bool retval = False; 

   if (hPostNewsDialog != NULL)
   {
      SendMessage(hPostNewsDialog, WM_COMMAND, IDCANCEL, 0);
      retval = True;
   }
   if (hReadNewsDialog != NULL)
   {
      SendMessage(hReadNewsDialog, WM_COMMAND, IDCANCEL, 0);
      retval = True;
   }
   return retval;
}
