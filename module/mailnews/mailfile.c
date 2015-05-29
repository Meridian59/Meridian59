// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * mailfile.c:  Deal with mail messages saved as files on the user's disk.
 *
 * We keep a list of all the filenames of the mail messages in an invisible list box.
 * The item data of each item in the list is the message number.
 */

#include "client.h"
#include "mailnews.h"

static char MAIL_DIR[] = "mail";
static char MAIL_SPEC[]  = "*.msg";

static HWND hMsgList = NULL;  /* Invisible list box to hold names of saved mail messages */

extern HWND hReadMailDlg; /* Non-NULL if Read Mail dialog is up */

static int  MailFindIndex(int number);
static Bool MailSaveMessage(char *msg, int msgnum);
static Bool MailParseMessageHeader(int msgnum, char *filename, MailHeader *header);
/****************************************************************************/
/*
 * MailGetMessageList:  Load filenames of all mail messages, and store in a list box.
 */
void MailGetMessageList(void)
{
   HANDLE hFindFile;
   WIN32_FIND_DATA file_info;
   char path[MAX_PATH + FILENAME_MAX];
   int index, msgnum;
   MailHeader header;

   /* Load messages in, in sorted order */
   if (hMsgList == NULL)
      hMsgList = CreateWindow("listbox", "", WS_CHILD | LBS_SORT, 0, 0, 0, 0, 
                              cinfo->hMain, NULL, hInst, NULL);
   else ListBox_ResetContent(hMsgList);
   
   sprintf(path,"%s\\%s", MAIL_DIR, MAIL_SPEC);
   hFindFile = FindFirstFile(path, &file_info);
   if (hFindFile == INVALID_HANDLE_VALUE)
      return;
   
   while (1)
   {
      // Skip directories
      if (!(file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      {
         /* Set itemdata of each entry to message header info */
         msgnum = atoi(file_info.cFileName);
         sprintf(path, "%s\\%s", MAIL_DIR, file_info.cFileName);
         
         index = ListBox_AddString(hMsgList, path);
         
         ListBox_SetItemData(hMsgList, index, msgnum);
      }
      
      if (FindNextFile(hFindFile, &file_info) == FALSE)
         break;
   }
   FindClose(hFindFile);
   
   // Add messages to mail dialog
   for (index = 0; index < ListBox_GetCount(hMsgList); index++)
   {
      msgnum = ListBox_GetItemData(hMsgList, index);
      ListBox_GetText(hMsgList, index, path);
      
      // Send summary string to mail dialog
      if (hReadMailDlg != NULL && MailParseMessageHeader(msgnum, path, &header))
         SendMessage(hReadMailDlg, BK_NEWMAIL, msgnum, (LPARAM) &header); 
   }
}
/****************************************************************************/
void MailDeleteMessageList(void)
{
   if (hMsgList != NULL)
   {
      DestroyWindow(hMsgList);
      hMsgList = NULL;
   }
}
/****************************************************************************/
/*
 * MailLoadMessage:  Load mail message with given number.  Place at most
 *   max_chars characters of the message in the given buffer.
 *   Returns True iff successful.
 *   Assumes that Read Mail dialog is up.
 */
Bool MailLoadMessage(int number, int max_chars, char *buf)
{
   int infile, numbytes, index;
   char filename[MAX_PATH + FILENAME_MAX];

   if ((index = MailFindIndex(number)) == -1)
      return False;

   ListBox_GetText(hMsgList, index, filename);

   if ((infile = open(filename, _O_BINARY | _O_RDONLY)) == -1)
      return False;
   
   if ((numbytes = read(infile, buf, max_chars - 1)) <= 0)
   {
      close(infile);
      return False;
   }
   close(infile);
   buf[numbytes] = 0;
   return True;
}
/****************************************************************************/
/*
 * MailNewMessage:  We've received a new message from the server.  Format it
 *   and save it.  If read mail dialog is up, update the dialog's message list.
 */
void MailNewMessage(int server_index, char *sender, int num_recipients, 
		    char recipients[MAX_RECIPIENTS][MAXUSERNAME + 1], char *message, long msg_time)
{
   int index, num_msgs, msgnum;
   char new_msg[MAXMAIL + 200 + MAX_SUBJECT + MAXUSERNAME * MAX_RECIPIENTS];
   char *subject = NULL, *ptr = NULL, *subject_str;
   int i, num;
   char filename[FILENAME_MAX + MAX_PATH];
   char date[MAXDATE];
   MailHeader header;
   Bool subject_found = True;

   if (recipients == NULL)
   {
      /* No new mail */
      if (hReadMailDlg != NULL)
         SendMessage(hReadMailDlg, BK_NONEWMAIL, 0, 0);
      return;
   }

   // Format message into new_msg 
   /* Add To: and From: fields to message */
   sprintf(new_msg, "%s%s\r\n%s", GetString(hInst, IDS_FROM), 
           sender, GetString(hInst, IDS_TO)); 

   /* Add recipients' names */
   num = 0;
   for (i = 0; i < num_recipients; i++)
   {
      if (num++ > 0)
         strcat(new_msg, ", "); 
      strcat(new_msg, recipients[i]); 
   }
   strcat(new_msg, "\r\n");

   /* Take subject field out of main part of message */
   subject_str = GetString(hInst, IDS_SUBJECT_ENGLISH);
   if (strncmp(message, subject_str, strlen(subject_str)))
   {
      subject_str = GetString(hInst, IDS_SUBJECT_GERMAN);
      if (strncmp(message, subject_str, strlen(subject_str))) {
         subject_found = False;
         subject = "";
      }
   }

   if (subject_found)
   {
      /* Skip "Subject: " leader */
      subject = message + strlen(subject_str);

      /* Skip subject line; have to deal with \n (from users) or \r\n (from kod resources) */
      ptr = strchr(subject, '\n');
      if (ptr != NULL)
      {
         if (ptr != subject && *(ptr - 1) == '\r')
            *(ptr - 1) = 0;
         
         *ptr = 0;
         message = ptr + 1;
      }
   }

   /* Add "Subject:" field to message */
   strcat(new_msg, subject_str);
   strcat(new_msg, subject);
   strcat(new_msg, "\r\n");
   
   /* Add "Date:" field to message */
   date[0] = 0;
   if (DateFromSeconds(msg_time, date) == True)
   {
      strcat(new_msg, GetString(hInst, IDS_DATE));      
      strcat(new_msg, date);      
   }
      
   strcat(new_msg, "\r\n-------------\r\n");
   strcat(new_msg, message);

   // Save message

   /* Set number of message to one more than current highest message */
   num_msgs = ListBox_GetCount(hMsgList);
   if (num_msgs == 0)
      msgnum = 1;
   else msgnum = ListBox_GetItemData(hMsgList, num_msgs - 1) + 1;

   /* If we save message ok, tell server that it can delete message */
   if (MailSaveMessage(new_msg, msgnum) == True)
      RequestDeleteMail(server_index);

   sprintf(filename, "%s\\%04d.msg", MAIL_DIR, msgnum);
   // Store filename in main part of list box; message number in itemdata field
   index = ListBox_AddString(hMsgList, filename);
   ListBox_SetItemData(hMsgList, index, msgnum);

   if (hReadMailDlg != NULL && MailParseMessageHeader(msgnum, filename, &header))
      SendMessage(hReadMailDlg, BK_NEWMAIL, msgnum, (LPARAM) &header); 
}
/****************************************************************************/
/*
 * MailSaveMessage:  Save given message string as given message number.
 *   Return True iff message successfully saved.
 */
Bool MailSaveMessage(char *msg, int msgnum)
{
   Bool done, saved;
   int outfile;
   char filename[MAX_PATH + FILENAME_MAX];

   do
   {
      done = True;
      /* If mail directory doesn't exist, try to create it */
      saved = MakeDirectory(MAIL_DIR);
      
      sprintf(filename, "%s\\%04d.msg", MAIL_DIR, msgnum);
      
      if ((outfile = open(filename, _O_BINARY | _O_RDWR | _O_CREAT, _S_IWRITE | _S_IREAD)) == -1)
	 saved = False;
      if (write(outfile, msg, strlen(msg)) <= 0)
	 saved = False;

      close(outfile);

      // Ask user about retrying
      if (!saved)
	 if (AreYouSure(hInst, hReadMailDlg, YES_BUTTON, IDS_SAVEFAILED))
	    done = False;
   } while (!done);

   return saved;
}
/****************************************************************************/
/*
 * MailFindIndex:  Return index in list box of message with given number.
 *   Returns -1 if message not found.
 */
int MailFindIndex(int number)
{
   int i, msgnum, num_msgs;

   num_msgs = ListBox_GetCount(hMsgList);
   for (i=0; i < num_msgs; i++)
   {
      msgnum = ListBox_GetItemData(hMsgList, i);
      if (msgnum == number)
	 return i;
   }
   return -1;
}
/****************************************************************************/
/*
 * MailDeleteMessage:  Delete the mail message with the given number.
 *   Return True on success.
 */
Bool MailDeleteMessage(int number)
{
   char filename[MAX_PATH + FILENAME_MAX];
   int index;

   if ((index = MailFindIndex(number)) == -1)
   {
      debug(("Message number not found to delete\n"));
      return False;
   }

   ListBox_GetText(hMsgList, index, filename);
   if (unlink(filename) != 0)
   {
      ClientError(hInst, hReadMailDlg, IDS_CANTDELETEMAIL, filename);
      return False;
   }

   ListBox_DeleteString(hMsgList, index);
   return True;
}
/****************************************************************************/
/*
 * MailParseMessage:  Parse the message with the given number, and fill in reply
 *   with the data from the message header.
 *   Return True on success.
 */
Bool MailParseMessage(int msgnum, MailInfo *info)
{
   FILE *infile;
   char filename[MAX_PATH + FILENAME_MAX];
   char line[MAX_LINE];
   int num_fields = 5;  // Don't increase this without changing szLoadString (only 5 at a time)
   int field_ids[] = {IDS_SUBJECT_ENGLISH, IDS_SUBJECT_GERMAN,
                      IDS_FROM, IDS_TO, IDS_DATE};
   char *fields[5], *ptr = NULL;
   int i, index;

   if ((index = MailFindIndex(msgnum)) == -1)
   {
      debug(("Message number not found to delete\n"));
      return False;
   }

   ListBox_GetText(hMsgList, index, filename);

   if ((infile = fopen(filename, "r")) == NULL)
      return False;

   info->subject[0] = 0;
   info->sender[0] = 0;
   info->date[0] = 0;
   info->num_recipients = 0;

   for (i=0; i < num_fields; i++)
      fields[i] = GetString(hInst, field_ids[i]);
   
   while (fgets(line, MAX_LINE, infile) != NULL)
   {
      /* See if we've reached end of header */
      if (line[0] == '-')
         break;

      /* See if we've found a field.  If so, set ptr to just after field */
      for (i=0; i < num_fields; i++)
      {
         if (!strncmp(line, fields[i], strlen(fields[i])))
         {
            /* Remove newline at end of line */
            line[strlen(line) - 1] = 0;
            
            ptr = line + strlen(fields[i]);
            break;
         }
      }
      if (i >= num_fields)
         continue;
      
      /* Found a field; parse appropriately */
      switch (field_ids[i])
      {
      case IDS_SUBJECT_ENGLISH:
      case IDS_SUBJECT_GERMAN:
         strncpy(info->subject, ptr, MAX_SUBJECT);
         info->subject[MAX_SUBJECT - 1] = 0;
         break;
         
      case IDS_DATE:
         strncpy(info->date, ptr, MAXDATE);
         info->date[MAXDATE - 1] = 0;
         break;
         
      case IDS_FROM:
         strncpy(info->sender, ptr, MAXNAME);
         info->sender[MAXNAME - 1] = 0;
         break;
         
      case IDS_TO:
         /* Parse out multiple recipients on a line */
         ptr = strtok(ptr, ",");
         while (ptr != NULL && info->num_recipients < MAX_RECIPIENTS)
         {
            /* Skip leading space */
            while (*ptr == ' ')
               ptr++;
            
            strncpy(info->recipients[info->num_recipients], ptr, MAXUSERNAME);
            info->recipients[info->num_recipients][MAXUSERNAME - 1] = 0;
            info->num_recipients++;
            ptr = strtok(NULL, ",");
         }
         break;
      }
   }
   fclose(infile);
   
   return True;
}
/****************************************************************************/
/*
 * MailParseMessageHeader:  Fill header with a string describing the mail message
 *   in the given file.
 *   Return True on success.
 */
Bool MailParseMessageHeader(int msgnum, char *filename, MailHeader *header)
{
   FILE *infile;
   int num_fields = 5;  // Don't increase this without changing szLoadString (only 5 at a time)
   int field_ids[] = {IDS_SUBJECT_ENGLISH, IDS_SUBJECT_GERMAN,
                      IDS_FROM, IDS_TO, IDS_DATE};
   int i;
   char *fields[5], *ptr = NULL;
   char line[MAX_LINE];

   if ((infile = fopen(filename, "r")) == NULL)
      return False;
   
   memset(header, 0, sizeof(MailHeader));
   
   for (i=0; i < num_fields; i++)
      fields[i] = GetString(hInst, field_ids[i]);
   
   while (fgets(line, MAX_LINE, infile) != NULL)
   {
      /* See if we've reached end of header */
      if (line[0] == '-')
         break;
      
      /* See if we've found a field.  If so, set ptr to just after field */
      for (i=0; i < num_fields; i++)
      {
         if (!strnicmp(line, fields[i], strlen(fields[i])))
         {
            /* Remove newline at end of line */
            line[strlen(line) - 1] = 0;
            
            ptr = line + strlen(fields[i]);
            break;
         }
      }
      if (i >= num_fields)
         continue;
      
      /* Found a field; parse appropriately */
      switch (field_ids[i])
      {
      case IDS_SUBJECT_ENGLISH:
      case IDS_SUBJECT_GERMAN:
         strncpy(header->subject, ptr, MAX_SUBJECT);
         break;
         
      case IDS_DATE:
         strncpy(header->date, ptr, MAXDATE);
         break;
         
      case IDS_FROM:
         strncpy(header->sender, ptr, MAXUSERNAME);
         break;
      }
   }
   
   fclose(infile);

   return True;
}
