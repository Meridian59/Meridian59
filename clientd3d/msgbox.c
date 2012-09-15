// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * msgbox.c:  Display error and information messages in MessageBoxes.
 */

#include "client.h"

#define ERROR_LENGTH 1024 /* Max length of an error string */

typedef struct {
   char *text;
   char *title;
   UINT style;
} MsgBoxStruct;

static Bool error_dialog_up = False;

static BOOL CALLBACK ClientMsgBoxProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
/************************************************************************/
/*
 * ClientError: Print an error in a message box.  fmt_id should be the string
 *   resource id # of a printf-style format string.
 *   The message box has hParent as its parent, and so appears over that window.
 *   Restores focus to window that had the focus on entry.
 *   Can only have one dialog up at a time.
 */
void _cdecl ClientError(HINSTANCE hModule, HWND hParent, int fmt_id, ...)
{
   char msg[ERROR_LENGTH];
   char fmt[ERROR_LENGTH];
   va_list marker;
   HWND hwndFocus = GetFocus();

   if (error_dialog_up)
      return;

   error_dialog_up = True;

   if (LoadString(hModule, (int) fmt_id, fmt, ERROR_LENGTH - 1) == 0)
   {
      sprintf(msg, "Can't load message string #%d", fmt_id);
      ClientMessageBox(hParent, msg, szAppName, MB_APPLMODAL);
      SetFocus(hwndFocus);
      return;
   }

   va_start(marker, fmt_id);
   vsprintf(msg, fmt, marker);
   va_end(marker);

   ClientMessageBox(hParent, msg, szAppName, MB_APPLMODAL | MB_ICONEXCLAMATION);
   SetFocus(hwndFocus);

   error_dialog_up = False;
}
/************************************************************************/
/*
 * Info: Print an information string in a message box.  fmt_id should be the string
 *   resource id # of a printf-style format string.
 *   The message box has hParent as its parent, and so appears over that window.
 *   Restores focus to window that had the focus on entry.
 */
void _cdecl Info(HINSTANCE hModule, HWND hParent, int fmt_id, ...)
{
   char msg[ERROR_LENGTH];
   char fmt[ERROR_LENGTH];
   va_list marker;
   HWND hwndFocus = GetFocus();

   if (LoadString(hModule, fmt_id, fmt, ERROR_LENGTH - 1) == 0)
   {
      sprintf(msg, "Can't load message string #%d", fmt_id);
      ClientMessageBox(hParent, msg, "Blakston Error", MB_APPLMODAL);
      SetFocus(hwndFocus);
      return;
   }

   va_start(marker, fmt_id);
   vsprintf(msg, fmt, marker);
   va_end(marker);
   
   ClientMessageBox(hParent, msg, szAppName, MB_APPLMODAL | MB_ICONINFORMATION);
   SetFocus(hwndFocus);
}
/************************************************************************/
/*
 * AreYouSure:  Bring up message box, and return True iff
 *   user selects "Yes".  
 *   defbutton should be YES_BUTTON or NO_BUTTON to indicate default button.
 *   fmt_id is RC-file resource identifier (NOT Blakston resource id) of format string.
 *   Restores focus to window that had the focus on entry.
 */
Bool _cdecl AreYouSure(HINSTANCE hModule, HWND hParent, int defbutton, int fmt_id, ...)
{
   char msg[ERROR_LENGTH], *fmt;
   HWND hwndFocus = GetFocus();
   int retval;
   va_list marker;

   fmt = GetString(hModule, fmt_id);

   va_start(marker, fmt_id);
   vsprintf(msg, fmt, marker);
   va_end(marker);
   
   if (defbutton == YES_BUTTON)
      retval = ClientMessageBox(hParent, msg, szAppName, 
			  MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1);
   else
      retval = ClientMessageBox(hParent, msg, szAppName, 
			  MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);

   SetFocus(hwndFocus);
   return IDYES == retval;
}
/************************************************************************/
/*
 * ClientMessageBox:  Like a normal MessageBox, but centered on the main window.
 *   Handles only the following MessageBox styles:
 *   - These button styles: YESNO, OK, OKCANCEL
 *   - All icon styles (ICONHAND, etc.)
 *   - All default button styles (DEFBUTTON1, etc.)
 *   The box is application modal.
 */
int ClientMessageBox(HWND hwndParent, char *text, char *title, UINT style)
{
   MsgBoxStruct s;
   static Bool box_up = False;
   int retval;

   if (box_up)
   {
      debug(("Tried to create 2 client message boxes!\n"));
      return 0;
   }

   box_up = True;

   s.text = text;
   s.title = title;
   s.style = style;

   retval = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_MSGBOX), hwndParent, 
			   ClientMsgBoxProc, (LPARAM) &s);
   box_up = False;
   return retval;
} 
/************************************************************************/
/*
 * ClientMsgBoxProc:  A substitute implementation of MessageBox's window procedure.
 *   We have 2 OK buttons and 2 Cancel buttons, to account for the case where one
 *   or the other is the default button.  We hide the buttons we don't need.
 */
BOOL CALLBACK ClientMsgBoxProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   static MsgBoxStruct *s;
   char *icon = NULL, *temp;
   int style, button_style, num_lines, yincrease;
   HICON hIcon;
   HWND hEdit, hText;
   HWND hOK, hCancel, hOK2, hCancel2;
   HFONT hFont;
   RECT dlg_rect, edit_rect;

   switch (message)
   {
   case WM_INITDIALOG:
      s = (MsgBoxStruct *) lParam;
      button_style = s->style & MB_TYPEMASK;
      hText = GetDlgItem(hDlg, IDC_TEXT);
      hOK = GetDlgItem(hDlg, IDOK);
      hCancel = GetDlgItem(hDlg, IDCANCEL);
      hOK2 = GetDlgItem(hDlg, IDOK2);
      hCancel2 = GetDlgItem(hDlg, IDCANCEL2);

      // Display text

      // Put text in invisible edit box to see how much space it will take
      hEdit = GetDlgItem(hDlg, IDC_EDIT);

      hFont = GetWindowFont(hText);
      SetWindowFont(hEdit, hFont, TRUE);
      SetWindowFont(hOK, hFont, FALSE);
      SetWindowFont(hCancel, hFont, FALSE);
      SetWindowFont(hOK2, hFont, FALSE);
      SetWindowFont(hCancel2, hFont, FALSE);
      SetWindowText(hEdit, s->text);

      Edit_GetRect(hEdit, &edit_rect);
      num_lines = Edit_GetLineCount(hEdit);

      // Count blank lines separately, since edit box not handling them correctly
      temp = s->text;
      do {
	 temp = strstr(temp, "\n\n");
	 if (temp != NULL)
	 {
	    num_lines++;
	    temp += 2;
	 }
      } while (temp != NULL);

      yincrease = GetFontHeight(hFont) * num_lines;

      // Resize dialog and text area
      GetWindowRect(hDlg, &dlg_rect);
      MoveWindow(hDlg, dlg_rect.left, dlg_rect.top, dlg_rect.right - dlg_rect.left, 
		 dlg_rect.bottom - dlg_rect.top + yincrease, FALSE);
      ResizeDialogItem(hDlg, hText, &dlg_rect, RDI_ALL, False);

      // Move buttons; center OK button if it's the only one
      if (button_style == MB_OK)
      {
	 ResizeDialogItem(hDlg, hOK, &dlg_rect, RDI_BOTTOM | RDI_HCENTER, False);
	 ShowWindow(hCancel, SW_HIDE);
	 ShowWindow(hCancel2, SW_HIDE);
      }
      else 
      {
	 ResizeDialogItem(hDlg, hOK, &dlg_rect, RDI_BOTTOM, False);
	 ResizeDialogItem(hDlg, hCancel, &dlg_rect, RDI_BOTTOM, False);
	 ResizeDialogItem(hDlg, hOK2, &dlg_rect, RDI_BOTTOM, False);
	 ResizeDialogItem(hDlg, hCancel2, &dlg_rect, RDI_BOTTOM, False);
      }

      SetWindowText(hDlg, s->title);
      SetWindowText(hText, s->text);
      ShowWindow(hEdit, SW_HIDE);

      // Set icon to appropriate system icon
      style = s->style & MB_ICONMASK;
      if (style == MB_ICONSTOP) icon = IDI_HAND;
      else if (style == MB_ICONINFORMATION) icon = IDI_ASTERISK;
      else if (style == MB_ICONEXCLAMATION) icon = IDI_EXCLAMATION;
      else if (style == MB_ICONQUESTION) icon = IDI_QUESTION;

      if (icon != NULL)
      {
	 hIcon = LoadIcon(NULL, icon);
	 Static_SetIcon(GetDlgItem(hDlg, IDC_MSGBOXICON), hIcon);
      }

      // Display correct button text
      switch (button_style)
      {
      case MB_YESNO:
	 SetWindowText(hOK, GetString(hInst, IDS_YES));
	 SetWindowText(hCancel, GetString(hInst, IDS_NO));
	 SetWindowText(hOK2, GetString(hInst, IDS_YES));
	 SetWindowText(hCancel2, GetString(hInst, IDS_NO));
	 break;
      }

      // Show correct button as default
      style = s->style & MB_DEFMASK;
      switch (style)
      {
      case MB_DEFBUTTON1:
      default:
	 SetFocus(hOK);
	 ShowWindow(hOK2, SW_HIDE);
	 ShowWindow(hCancel, SW_HIDE);
	 break;

      case MB_DEFBUTTON2:
	 SetFocus(hCancel);
	 ShowWindow(hOK, SW_HIDE);
	 ShowWindow(hCancel2, SW_HIDE);
	 break;
      }
	 	 
      CenterWindow(hDlg, GetParent(hDlg));
      return 0;

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDOK:
      case IDOK2:
	 button_style = s->style & MB_TYPEMASK;
	 if (button_style == MB_YESNO)
	    EndDialog(hDlg, IDYES);
	 else EndDialog(hDlg, IDOK);
	 return TRUE;
      case IDCANCEL:
      case IDCANCEL2:
	 button_style = s->style & MB_TYPEMASK;
	 if (button_style == MB_YESNO)
	    EndDialog(hDlg, IDNO);
	 else EndDialog(hDlg, IDCANCEL);
	 return TRUE;
      }
      break;
   }

   return FALSE;
}
