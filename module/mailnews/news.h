// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * news.h:  Header file for newssend.c, newsread.c
 */

#ifndef _NEWS_H
#define _NEWS_H

#define MAXARTICLE 4096      /* Max length of news article */

/* Permissions to be able to read and post news */
#define NEWS_READ 0x01
#define NEWS_POST 0x02

/* Per-article information */
typedef struct
{
   long num;                       /* Index # of article */
   long time;                      /* Time article was posted */
   char poster[MAXUSERNAME];       /* Person who posted article */
   char title[MAX_SUBJECT];        /* Title string of article */
} NewsArticle;

typedef struct {
   WORD newsgroup;
   ID   group_name_rsc;        /* Name resource of newsgroup */
   char *subject;              /* Initial subject; NULL if none */
} PostNewsDialogStruct;

typedef struct {
   WORD newsgroup;
   ID   group_name_rsc;        /* Name resource of newsgroup */
   char *desc;                 /* Description of newsgroup */
   BYTE permissions;           /* Read/write permissions of this newsgroup */
   list_type articles;         /* Used internally; should not be set by caller */
} ReadNewsDialogStruct;

Bool UserPostArticle(HWND hParent, WORD newsgroup, ID name_rsc, char *title);
void UserReadNews(object_node *obj, char *desc, WORD newsgroup, BYTE permissions);
void UserReadArticle(char *article);

void MakeReplySubject(char *subject, int max_chars);

void ReceiveArticles(WORD newsgroup, BYTE part, BYTE max_part, list_type articles);
Bool AbortNewsDialogs(void);
Bool DateFromSeconds(long seconds, char *str);

#endif /* #ifndef _NEWS_H */
