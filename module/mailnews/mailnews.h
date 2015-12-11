// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * mailnews.h:  Main header file for mail and news DLL
 */

#ifndef _MAILNEWS_H
#define _MAILNEWS_H

#include "mail.h"
#include "news.h"
#include "mailnwrc.h"

#define MODULE_ID 102    // Unique module id #

#define BK_NEWMAIL        (WM_USER + 103)
#define BK_NOPREVMSG      (WM_USER + 104)
#define BK_NONEWMAIL      (WM_USER + 105)
#define BK_ARTICLE        (WM_USER + 121)
#define BK_ARTICLES       (WM_USER + 122)

// Sending messages to server
#define RequestReadMail()               ToServer(BP_REQ_GET_MAIL, msg_table)
#define SendMail(num, r, msg)           ToServer(BP_SEND_MAIL, msg_table, num, r, msg)
#define RequestDeleteMail(index)        ToServer(BP_DELETE_MAIL, msg_table, index)
#define RequestDeleteNews(group, index) ToServer(BP_DELETE_NEWS, msg_table, group, index)
#define RequestLookupNames(num, str)    ToServer(BP_REQ_LOOKUP_NAMES, msg_table, num, str)
#define RequestArticles(group)          ToServer(BP_REQ_ARTICLES, msg_table, group)
#define RequestArticle(group, index)    ToServer(BP_REQ_ARTICLE, msg_table, group, index)
#define SendArticle(group, s1, s2)      ToServer(BP_POST_ARTICLE, msg_table, group, s1, s2)
extern client_message msg_table[];

extern ClientInfo *cinfo;         // Holds data passed from main client
extern Bool exiting;              // True when module is exiting and should be unloaded

extern HINSTANCE hInst;  // module handle

bool IsNameInIgnoreList(const char *name);

#endif /* #ifndef _MAILNEWS_H */
