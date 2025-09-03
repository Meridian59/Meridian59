// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * mailnews.c:  Module interface file for mail and news DLL.
 */

#include "client.h"
#include "mailnews.h"

HINSTANCE hInst;          // Handle of this DLL

ClientInfo *cinfo;     // Holds data passed from main client
bool        exiting;

extern HWND hSendMailDlg; /* Non-NULL if Send Mail dialog is up */
extern HWND hReadMailDlg; /* Non-NULL if Read Mail dialog is up */

// Declare global HBITMAP objects
HBITMAP hbmUpArrow = NULL;
HBITMAP hbmDownArrow = NULL;

/* local function prototypes */
static bool HandleMail(char *ptr, long len);
static bool HandleLookupNames(char *ptr, long len);
static bool HandleArticles(char *ptr, long len);
static bool HandleArticle(char *ptr, long len);
static bool HandleLookNewsgroup(char *ptr, long len);
static void RegisterWindowClasses(void);
static void CommandMail(char *args);

// Server message handler table
static handler_struct handler_table[] = {
{ BP_MAIL,              HandleMail },
{ BP_LOOKUP_NAMES,      HandleLookupNames },
{ BP_ARTICLES,          HandleArticles },
{ BP_ARTICLE,           HandleArticle },
{ BP_LOOK_NEWSGROUP,    HandleLookNewsgroup, },
{ 0, NULL},
};

// Client message table
client_message msg_table[] = {
{ BP_REQ_GET_MAIL,         { PARAM_END }, },
{ BP_DELETE_MAIL,          { PARAM_INDEX, PARAM_END }, },
{ BP_SEND_MAIL,            { PARAM_ID_ARRAY, PARAM_STRING, PARAM_END }, },
{ BP_REQ_LOOKUP_NAMES,     { PARAM_WORD, PARAM_STRING, PARAM_END }, },
{ BP_REQ_ARTICLES,         { PARAM_NEWSID, PARAM_END}, },
{ BP_REQ_ARTICLE,          { PARAM_NEWSID, PARAM_INDEX, PARAM_END}, },
{ BP_POST_ARTICLE,         { PARAM_NEWSID, PARAM_STRING, PARAM_STRING, PARAM_END}, },
{ 0,                       { PARAM_END }, },
};

#define A_MAILREAD (A_MODULE + 100)

static AddButton mail_buttons[] = {
{ A_TEXTCOMMAND, "mail", A_NOACTION, NULL, NULL, IDB_MAILBOX,   IDS_READMAIL, },
};
static int num_buttons = (sizeof(mail_buttons) / sizeof(AddButton));

static TypedCommand commands[] = {
{ "mail",      CommandMail, },
{ NULL,        NULL},    // Must end table this way
};

/****************************************************************************/
BOOL WINAPI DllMain(HINSTANCE hModule, DWORD reason, LPVOID reserved)
{
   switch (reason)
   {
   case DLL_PROCESS_ATTACH:
      hInst = hModule;

      // Load sort arrow bitmaps
      hbmUpArrow = (HBITMAP)LoadImage(hInst, 
         MAKEINTRESOURCE(IDB_UPARROW), IMAGE_BITMAP, 0, 0, 
         LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);

      hbmDownArrow = (HBITMAP)LoadImage(hInst, 
         MAKEINTRESOURCE(IDB_DOWNARROW), IMAGE_BITMAP, 0, 0, 
         LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS);
         
      break;

   case DLL_PROCESS_DETACH:
      break;
   }
   return TRUE;
}
/****************************************************************************/
void WINAPI GetModuleInfo(ModuleInfo *info, ClientInfo *client_info)
{
   int i;

   info->event_mask = EVENT_SERVERMSG | EVENT_COLORCHANGED | EVENT_FONTCHANGED | 
      EVENT_USERACTION | EVENT_TEXTCOMMAND;
   info->priority   = PRIORITY_NORMAL;
   info->module_id  = MODULE_ID;

   cinfo = client_info;    // Save client info for our use later
   exiting = false;

   RegisterWindowClasses();


   for (i = 0; i < num_buttons; i++)
   {
      mail_buttons[i].hModule = hInst;
      ToolbarAddButton(&mail_buttons[i]);
   }
}
/****************************************************************************/
void WINAPI ModuleExit(void)
{
   bool has_dialog = false, retval;

   exiting = true;

   has_dialog = AbortMailDialogs();
   retval = AbortNewsDialogs();
   has_dialog = has_dialog || retval;

   if (!has_dialog)
      PostMessage(cinfo->hMain, BK_MODULEUNLOAD, 0, MODULE_ID);
}
/****************************************************************************/
/*
 * RegisterWindowClasses:  Register classes for modeless mail windows, so
 *   that they can have an icon when minimized.
 */
void RegisterWindowClasses(void)
{
   WNDCLASS wndclass;

   wndclass.style         = CS_HREDRAW | CS_VREDRAW;
   wndclass.lpfnWndProc   = DefDlgProc;
   wndclass.cbClsExtra    = 0;
   wndclass.cbWndExtra    = DLGWINDOWEXTRA;
   wndclass.hInstance     = hInst;
   wndclass.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LETTER));
   wndclass.hCursor       = NULL;
   wndclass.hbrBackground = NULL;
   wndclass.lpszMenuName  = NULL; 
   wndclass.lpszClassName = "Mail";
   if (!RegisterClass (&wndclass))
      debug(("RegisterClass failed!\n"));
}

/* event handlers */
/****************************************************************************/
/*
 * EVENT_SERVERMSG
 */
/****************************************************************************/
bool WINAPI EventServerMessage(char *message, long len)
{
   bool retval;

   retval = LookupMessage(message, len, handler_table);

   // If we handle message, don't pass it on to anyone else
   if (retval == true)
     return false;

   return true;    // Allow other modules to get other messages
}
/********************************************************************/
bool HandleMail(char *ptr, long len)
{
   long index;
   WORD num_recipients, i;
   char message[MAXMESSAGE + MAX_SUBJECT + 200];
   char* msg = message;
   char sender[MAXUSERNAME + 1];
   char recipients[MAX_RECIPIENTS][MAXUSERNAME + 1];
   ID resource_id;
   long msg_time;
   char *start = ptr;

   Extract(&ptr, &index, 4);
   len -= 4;
   len = ExtractString(&ptr, len, sender, MAXUSERNAME);
   if (len == -1)
      return false;
   Extract(&ptr, &msg_time, SIZE_TIME);
   Extract(&ptr, &num_recipients, SIZE_NUM_RECIPIENTS);
   len -= SIZE_TIME + SIZE_NUM_RECIPIENTS;

   // If somehow mail message is screwed up, delete it
   if (num_recipients > MAX_RECIPIENTS)
   {
      RequestDeleteMail(index);
      return false;
   }

   /* If no recipients, then there is no more mail */
   if (num_recipients == 0)
   {
      MailNewMessage(0, sender, 0, NULL, NULL, 0);
      return true;
   }

   for (i=0; i < num_recipients; i++)
   {
      len = ExtractString(&ptr, len, recipients[i], MAXUSERNAME);
      if (len == -1)
         return false;
   }
   
   Extract(&ptr, &resource_id, SIZE_ID);
   len -= SIZE_ID;
   
   /* Remove format string id # & other ids from length */
   if (!CheckServerMessage(&msg, &ptr, len, resource_id))
      return false;

   MailNewMessage(index, sender, num_recipients, recipients, msg, msg_time);

   return true;
}
/********************************************************************/
bool HandleArticles(char *ptr, long len)
{
   WORD newsgroup;
   WORD num_articles;
   BYTE part, max_part;
   NewsArticle *article;
   list_type list = NULL;
   int i;

   if (len < SIZE_NEWSGROUP_ID + 2 * SIZE_PART + 2)
      return false;

   Extract(&ptr, &newsgroup, SIZE_NEWSGROUP_ID);
   Extract(&ptr, &part, SIZE_PART);
   Extract(&ptr, &max_part, SIZE_PART);
   Extract(&ptr, &num_articles, 2);
   len -= SIZE_NEWSGROUP_ID + 2 * SIZE_PART + 2;

   for (i=0; i < num_articles; i++)
   {
      if (len < 4 + SIZE_TIME)
      {
	 list_destroy(list);
	 return false;
      }
      len -= 4 + SIZE_TIME;

      article = (NewsArticle *) SafeMalloc(sizeof(NewsArticle));
      Extract(&ptr, &article->num, 4);
      Extract(&ptr, &article->time, SIZE_TIME);
      len = ExtractString(&ptr, len, article->poster, MAXUSERNAME);
      if (len == -1)
      {
	 list_destroy(list);
	 return false;
      }

      len = ExtractString(&ptr, len, article->title, MAX_SUBJECT);
      if (len == -1)
      {
	 list_destroy(list);
	 return false;
      }
      list = list_add_item(list, article);     
   }

   if (len != 0)
      return false;

   ReceiveArticles(newsgroup, part, max_part, list);
   return true;
}
/********************************************************************/
bool HandleArticle(char *ptr, long len)
{
   char article[MAXMESSAGE + 1];

   len = ExtractString(&ptr, len, article, MAXMESSAGE);
   if (len != 0)
      return false;

   UserReadArticle(article);
   return true;
}
/********************************************************************/
bool HandleLookNewsgroup(char *ptr, long len)
{
   ID resource_id;
   WORD newsgroup;
   BYTE permission;
   char message[MAXMESSAGE + 1];
   char* msg = message;
   int temp_len;
   object_node obj;

   temp_len = SIZE_NEWSGROUP_ID + 1 + SIZE_ID * 4 + SIZE_ANIMATE;
   if (len < temp_len)
      return false;

   Extract(&ptr, &newsgroup, SIZE_NEWSGROUP_ID);
   Extract(&ptr, &permission, 1);

   ExtractObject(&ptr, &obj);
   Extract(&ptr, &resource_id, SIZE_ID);

   /* Remove format string id # & other ids from length */
   if (!CheckServerMessage(&msg, &ptr, len - temp_len, resource_id))
      return false;

   UserReadNews(&obj, msg, newsgroup, permission);

   return true;
}
/********************************************************************/
bool HandleLookupNames(char *ptr, long len)
{
   WORD num_objects;
   int i;
   ID objs[MAX_RECIPIENTS];
   char *start = ptr;

   Extract(&ptr, &num_objects, 2);

   if (num_objects > MAX_RECIPIENTS)
   {
      debug(("Too many recipients %d\n", (int) num_objects));
      return false;
   }
   
   for (i=0; i < num_objects; i++)
      Extract(&ptr, &objs[i], SIZE_ID);
   
   len -= (ptr - start);
   if (len != 0)
   {
      SafeFree(objs);
      return false;
   }

   MailRecipientsReceived(num_objects, objs);
   
   return true;
}
/****************************************************************************/
/*
 * EVENT_USERACTION
 */
/****************************************************************************/
bool WINAPI EventUserAction(int action, void *user_action)
{
   if (action != A_MAILREAD)
      return true;

   if (!IsWaitingGameState(GameGetState()))
      UserReadMail();

   return false;
}
/****************************************************************************/
void CommandMail(char *args)
{
   UserReadMail();
}
/****************************************************************************/
/*
 * EVENT_FONTCHANGED
 */
/****************************************************************************/
bool WINAPI EventFontChanged(WORD font_id, LOGFONT *font)
{
   MailChangeFonts();
   return true;
}
/****************************************************************************/
/*
 * EVENT_COLORCHANGED
 */
/****************************************************************************/
bool WINAPI EventColorChanged(WORD color_id, COLORREF color)
{
   MailChangeColor();
   return true;
}
/****************************************************************************/
/*
 * EVENT_TEXTCOMMAND
 */
/****************************************************************************/
bool WINAPI EventTextCommand(char *str)
{
   // Parse command, and don't pass it on if we handle it
   if (ParseCommand(str, commands))
      return false;

   return true;
}

/***************************************************************************/
/*
 * IsNameInIgnoreList checks ignore status by name, not by ID.
 */
/***************************************************************************/

bool IsNameInIgnoreList(const char *name)
{
  if (name != NULL)
  {
    for (int i = 0; i < MAX_IGNORE_LIST; ++i)
      if (0 == strcmp(cinfo->config->ignore_list[i], name))
        return true;
  }
  return false;
}

/***************************************************************************/
/*
 * ListView_SetHeaderSortImage sets sort indicator image for a given ListView
 * provided a column index and sort direction.
 */
/***************************************************************************/
void ListView_SetHeaderSortImage(HWND hListView, int sortedColumn, bool sortAscending)
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
