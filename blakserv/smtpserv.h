// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * smtpserv.h
 *
 */

#ifndef _SMTPSERV_H
#define _SMTPSERV_H

#define SMTP_MAX_LINE 1010

enum
{
   SMTP_READY, SMTP_DATA, SMTP_QUIT, SMTP_ERROR
};

typedef struct string_list_struct
{
   char *str;
   struct string_list_struct *next;
} string_list;

typedef struct
{
   int state;
   char source_name[200];
   char buf[SMTP_MAX_LINE];
   int len_buf;
   char cmd[SMTP_MAX_LINE+1]; /* gotta null terminate it */
   SOCKET sock;
   char *reverse_path;
   string_list *forward_path;
   string_list *data;
} smtp_node;



void HandleSMTPConnection(smtp_node *smtp);

#endif
