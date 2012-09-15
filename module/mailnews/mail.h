// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * mail.h:  Header file for mailsend.c, mailread.c, and mailfile.c
 */

#ifndef _MAIL_H
#define _MAIL_H

#define MAXMAIL 4096       /* Max length of mail message body */
#define MAXDATE 23         /* Max length of date string */
#define MAX_SUBJECT 50     /* Max length of mail subject */
#define MAX_RECIPIENTS 20  /* Max # of recipients of a message */
#define MAX_HEADERLINE 200 /* Max length of message summary line in list box */
#define MAX_LINE (20 + (MAXUSERNAME + 2) * MAX_RECIPIENTS) /* Max length of line in header */

typedef struct {
   char      subject[MAX_SUBJECT + 1];
   char      sender[MAXUSERNAME + 1];
   char      date[MAXDATE + 1];
} MailHeader;

// Info to make a reply
typedef struct {
   char      subject[MAX_SUBJECT];
   char      sender[MAXUSERNAME + 1];
   int       num_recipients;
   char      recipients[MAX_RECIPIENTS][MAXUSERNAME + 1];
   char      date[MAXDATE + 1];
} MailInfo;

HBRUSH MailCtlColor(HWND hwnd, HDC hdc, HWND hwndChild, int type);
void MailSendReply(HWND hParent, MailInfo *reply);
void MailChangeFonts(void);
void MailChangeColor(void);
void UserSendMail(void);
void UserReadMail(void);
Bool AbortMailDialogs(void);
Bool ListViewGetCurrentData(HWND hList, int *index, int *data);
void MailRecipientsReceived(WORD num_objects, ID *objs);

/* mailfile.c */
void MailGetMessageList(void);
void MailDeleteMessageList(void);
Bool MailLoadMessage(int number, int max_chars, char *buf);
void MailNewMessage(int server_index, char *sender, int num_recipients, 
		    char recipients[MAX_RECIPIENTS][MAXUSERNAME + 1], char *message, long msg_time);
Bool MailDeleteMessage(int number);
Bool MailParseMessage(int msgnum, MailInfo *info);

#endif /* #ifndef _MAIL_H */
