// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * mailsend.c:  Deal with sending mail, as well as procedures that are common to
 *   sending and receiving mail.
 */

#include "client.h"
#include "mailnews.h"

HWND hSendMailDlg = NULL; /* Non-NULL if Send Mail dialog is up */
HWND hReadMailDlg = NULL; /* Non-NULL if Read Mail dialog is up */

static MailInfo *info = NULL; // Recipient list for message currently being sent

static RECT  dlg_rect;        // Screen position of dialog

static ChildPlacement mailsend_controls[] = {
{ IDC_MAILEDIT,   RDI_ALL },
{ IDC_SUBJECT,    RDI_LEFT | RDI_RIGHT | RDI_TOP},
{ IDC_RECIPIENTS, RDI_LEFT | RDI_RIGHT | RDI_TOP},
{ IDC_OK,         RDI_LEFT | RDI_HPIN | RDI_BOTTOM },
{ IDCANCEL,       RDI_LEFT | RDI_HPIN | RDI_BOTTOM },
{ 0,              0 },   // Must end this way
};

static void SendMailInitialize(HWND hDlg, MailInfo *reply);
static Bool SendMailMessage(HWND hDlg);
static BOOL CALLBACK SendMailDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
/****************************************************************************/
/*
 * UserSendMail:  Pop up the send mail dialog box, which does all the work.
 */
void UserSendMail(void)
{
   /* If dialog is already up, raise it to top */
   if (hSendMailDlg != NULL)
   {
      ShowWindow(hSendMailDlg, SW_SHOWNORMAL);
      SetFocus(hSendMailDlg);
      return;
   }

   CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAILSEND), NULL, SendMailDialogProc,
		     (LPARAM) NULL);
}
/****************************************************************************/
/*
 * MailSendReply:  Bring up send mail dialog, and initialize it with info 
 *   in given structure.  Only works if send mail dialog isn't already up.
 *   hParent is window from which request originated; used as parent for errors.
 */
void MailSendReply(HWND hParent, MailInfo *reply)
{
   int i;

   if (hSendMailDlg != NULL)
   {
      ClientError(hInst, hParent, IDS_MAILONEDIALOG);
      return;
   }

   // Can't reply to people with commas in their name (i.e. messages from the game)
   for (i=0; i < reply->num_recipients; i++)
     if (strchr(reply->recipients[i], ',') != NULL)
     {
       ClientError(hInst, hParent, IDS_CANTREPLY);
       return;
     }

   CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAILSEND), NULL, 
		     SendMailDialogProc, (LPARAM) reply);
}
/****************************************************************************/
BOOL CALLBACK SendMailDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   static HWND hEdit, hSubject, hRecipients;
   static MailInfo *reply;
   MINMAXINFO *lpmmi;

   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, cinfo->hMain);

      hEdit = GetDlgItem(hDlg, IDC_MAILEDIT);
      hSubject = GetDlgItem(hDlg, IDC_SUBJECT);
      hRecipients = GetDlgItem(hDlg, IDC_RECIPIENTS);
      
      /* Store dialog rectangle in case of resize */
      GetWindowRect(hDlg, &dlg_rect);

      Edit_LimitText(hEdit, MAXMAIL - 1);
      Edit_LimitText(hSubject, MAX_SUBJECT - 1);
      Edit_LimitText(hRecipients, (MAXUSERNAME + 2) * MAX_RECIPIENTS - 1);
      SendMessage(hDlg, BK_SETDLGFONTS, 0, 0);
      EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);

      hSendMailDlg = hDlg;

      // See if we should initialize dialog for a reply
      reply = (MailInfo *) lParam;
      if (reply != NULL)
      {
	 SendMailInitialize(hDlg, reply);
	 SetFocus(hEdit);
      }

      return 0;

   case WM_SIZE:
      ResizeDialog(hDlg, &dlg_rect, mailsend_controls);
      return TRUE;      

   case WM_GETMINMAXINFO:
      lpmmi = (MINMAXINFO *) lParam;
      lpmmi->ptMinTrackSize.x = 250;
      lpmmi->ptMinTrackSize.y = 200;
      return 0;

   case WM_ACTIVATE:
      if (wParam == 0)
	 *cinfo->hCurrentDlg = NULL;
      else *cinfo->hCurrentDlg = hDlg;
      return TRUE;

   case BK_SETDLGFONTS:
      SetWindowFont(hEdit, GetFont(FONT_MAIL), TRUE);
      SetWindowFont(hSubject, GetFont(FONT_MAIL), TRUE);
      SetWindowFont(hRecipients, GetFont(FONT_MAIL), TRUE);
      return TRUE;

   case BK_SETDLGCOLORS:
      InvalidateRect(hDlg, NULL, TRUE);
      return TRUE;

      HANDLE_MSG(hDlg, WM_CTLCOLOREDIT, MailCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORLISTBOX, MailCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORSTATIC, MailCtlColor);
      HANDLE_MSG(hDlg, WM_CTLCOLORDLG, MailCtlColor);

      HANDLE_MSG(hDlg, WM_INITMENUPOPUP, InitMenuPopupHandler);

   case WM_CLOSE:
      SendMessage(hDlg, WM_COMMAND, IDCANCEL, 0);
      break;

   case WM_DESTROY:
      hSendMailDlg = NULL;
      if (exiting)
	 PostMessage(cinfo->hMain, BK_MODULEUNLOAD, 0, MODULE_ID);
      break;

   case WM_COMMAND:
      UserDidSomething();

      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDCANCEL:
	 DestroyWindow(hDlg);
	 return TRUE;

      case IDOK:
	 /* User has pressed return somewhere */
	 if (GetFocus() == hSubject)
	    SetFocus(hEdit);
	 else if (GetFocus() == hRecipients)
	    SetFocus(hSubject);
	 return TRUE;

      case IDC_OK:
	 SendMailMessage(hDlg);
	 return TRUE;
      }
      break;
   }
   return FALSE;
}

/*****************************************************************************/
/*
 * SendMailInitialize:  Initialize dialog to reflect settings in "reply" variable.
 */
void SendMailInitialize(HWND hDlg, MailInfo *reply)
{
   HWND hRecipients;
   int i;

   /* Set recipients, separated by commas in list box */
   hRecipients = GetDlgItem(hDlg, IDC_RECIPIENTS);
   WindowBeginUpdate(hRecipients);
   for (i=0; i < reply->num_recipients; i++)
   {
      if (i != 0)
      {
	 Edit_SetSel(hRecipients, -1, -1);
	 Edit_ReplaceSel(hRecipients, ", ");
      }
      Edit_SetSel(hRecipients, -1, -1);
      Edit_ReplaceSel(hRecipients, reply->recipients[i]);
   }
   WindowEndUpdate(hRecipients);

   /* Set subject */
   SetDlgItemText(hDlg, IDC_SUBJECT, reply->subject);

   // Free memory for reply info
   SafeFree(reply);
}
/*****************************************************************************/
/*
 * SendMailMessage:  hDlg is filled in with mail message info; build up message
 *   and ask server to validate recipient names.
 *   Return True if message successfully sent.
 */
Bool SendMailMessage(HWND hDlg)
{
   char line[MAX_LINE + 1], *ptr, *temp;
   int i, j, k;

   info = (MailInfo *) SafeMalloc(sizeof(MailInfo));
   
   info->num_recipients = 0;

   // Read recipients
   GetDlgItemText(hDlg, IDC_RECIPIENTS, line, MAX_LINE);
   
   ptr = strtok(line, ",");
   while (ptr != NULL)
   {
      if (info->num_recipients >= MAX_RECIPIENTS)
      {
	 if (!AreYouSure(hInst, hDlg, NO_BUTTON, IDS_TOOMANYRECIPIENTS, MAX_RECIPIENTS))
	 {
	    SafeFree(info);
	    info = NULL;
	    return False;
	 }
	 break;
      }

      // Skip leading spaces
      while (*ptr == ' ')
	 ptr++;

      // Skip trailing spaces
      temp = ptr + strlen(ptr) - 1;
      while (temp > ptr && *temp == ' ')
	 temp--;
      *(temp + 1) = 0;

      strncpy(info->recipients[info->num_recipients], ptr, MAXUSERNAME);
      info->recipients[info->num_recipients][MAXUSERNAME - 1] = 0;

      info->num_recipients++;

      ptr = strtok(NULL, ",");
   }

   /* Must have >= 1 recipient */
   if (info->num_recipients == 0)
   {
      ClientError(hInst, hDlg, IDS_NORECIPIENTS);
      return False;
   }

   // Remove duplicate recipients
   for (i = 0; i < info->num_recipients; i++)
      for (j = i + 1; j < info->num_recipients; j++)
	 if (!stricmp(info->recipients[i], info->recipients[j]))
	 {
	    // Move everyone else up one slot
	    for (k = j; k < info->num_recipients - 1; k++)
	       strcpy(info->recipients[k], info->recipients[k + 1]);
	    info->num_recipients--;
	    j--;
	 }

   // Translate names into object numbers
   line[0] = 0;
   for (i=0; i < info->num_recipients; i++)
   {
      if (i != 0)
	 strcat(line, ",");
      strcat(line, info->recipients[i]);
   }

   RequestLookupNames(info->num_recipients, line);

   // Disable OK button, so that only one lookup happens at once
   EnableWindow(GetDlgItem(hDlg, IDC_OK), FALSE);
   SetDlgItemText(hDlg, IDC_SENDMAILMSG, GetString(hInst, IDS_CHECKNAMES));

   return True;
}
/*****************************************************************************/
/*
 * MailRecipientsReceived:  Server has translated recipient names into given object
 *   numbers (in the same order).  Now send the mail message.
 */
void MailRecipientsReceived(WORD num_objects, ID *objs)
{
   int i, len, chars;
   HWND hEdit;
   char buf[MAXMAIL + MAX_SUBJECT + 20];
   Bool names_legal;

   if (hSendMailDlg == NULL || info == NULL)
      return;

   if (num_objects != info->num_recipients)
   {
      debug(("Mismatch in # of recipients (got %d, expected %d)\n", 
	      num_objects, info->num_recipients));
      SafeFree(info);
      return;
   }

   // Verify that object numbers are legal
   names_legal = True;
   for (i=0; i < num_objects; i++)
      if (objs[i] == 0)
      {
	 ClientError(hInst, hSendMailDlg, IDS_BADNAME, info->recipients[i]);
	 SetDlgItemText(hSendMailDlg, IDC_SENDMAILMSG, "");
	 names_legal = False;
	 break;
      }

   if (names_legal)
   {
      /* Get subject and text, and send the message */
      chars = sprintf(buf, "%s", GetString(hInst, IDS_SUBJECT));
      
      len = GetDlgItemText(hSendMailDlg, IDC_SUBJECT, buf + chars, MAX_SUBJECT);
      buf[chars + len] = '\n';
      chars += len + 1;
      
      hEdit = GetDlgItem(hSendMailDlg, IDC_MAILEDIT);
      len = Edit_GetTextLength(hEdit);      						   
      Edit_GetText(hEdit, buf + chars, len + 1);
      
      SendMail(num_objects, objs, buf);
      
      SendMessage(hSendMailDlg, WM_CLOSE, 0, 0);
   }
   else EnableWindow(GetDlgItem(hSendMailDlg, IDC_OK), TRUE);
   SafeFree(info);
   info = NULL;
}

/****************************************************************************/
/*
 * MailCtlColor:  Set color of various sections of mail dialogs.
 */
HBRUSH MailCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type)
{
   if (type == CTLCOLOR_EDIT)
   {
      SetTextColor(hdc, GetColor(COLOR_MAILFGD));
      SetBkColor(hdc, GetColor(COLOR_MAILBGD));
      return GetBrush(COLOR_MAILBGD);
   }
   return DialogCtlColor(hwnd, hdc, hwndChild, type);
}

/****************************************************************************/
/*
 * MailChangeFonts:  Tell mail dialogs that fonts have changed.
 */
void MailChangeFonts(void)
{
   if (hSendMailDlg != NULL)
      SendMessage(hSendMailDlg, BK_SETDLGFONTS, 0, 0);

   if (hReadMailDlg != NULL)
      SendMessage(hReadMailDlg, BK_SETDLGFONTS, 0, 0);
}
/****************************************************************************/
/*
 * MailChangeColor:  Tell mail dialogs that colors have changed.
 */
void MailChangeColor(void)
{
   if (hSendMailDlg != NULL)
      SendMessage(hSendMailDlg, BK_SETDLGCOLORS, 0, 0);

   if (hReadMailDlg != NULL)
      SendMessage(hReadMailDlg, BK_SETDLGCOLORS, 0, 0);
}
/****************************************************************************/
/*
 * AbortMailDialogs:  Close mail dialogs.  Return True iff any was open.
 */
Bool AbortMailDialogs(void)
{
   Bool retval = False;

   // If we're in the middle of sending a message, stop
   if (info != NULL)
   {
      SafeFree(info);
      info = NULL;
   }

   if (hSendMailDlg != NULL)
   {
      SendMessage(hSendMailDlg, WM_CLOSE, 0, 0);
      retval = True;
   }

   if (hReadMailDlg != NULL)
   {
      SendMessage(hReadMailDlg, WM_CLOSE, 0, 0);
      retval = True;
   }
   return retval;
}

