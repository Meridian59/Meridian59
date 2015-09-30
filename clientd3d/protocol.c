// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * protocol.c: Piece together messages to send to server.
 */

#include "client.h"

/* Login mode messages */
static client_message login_msg_table[] = { 
{ AP_LOGIN,                { PARAM_BYTE, PARAM_BYTE, PARAM_INT, PARAM_INT, PARAM_INT, PARAM_INT,
				PARAM_INT, PARAM_WORD, PARAM_WORD, 
				PARAM_INT, PARAM_INT, PARAM_INT,
				PARAM_STRING, PARAM_STRING, PARAM_END }, },
{ AP_REQ_GAME,             { PARAM_INT, PARAM_INT, PARAM_STRING, PARAM_END }, },
{ AP_REQ_ADMIN,            { PARAM_END }, },
{ AP_GETRESOURCE,          { PARAM_END }, },
{ AP_GETALL,               { PARAM_END }, },
{ AP_UPLOAD,               { PARAM_END }, },
{ AP_REGISTER,             { PARAM_STRING, PARAM_END }, },
{ AP_RESYNC,               { PARAM_END }, },
{ AP_REQ_MENU,             { PARAM_END }, },
{ AP_ADMINNOTE,            { PARAM_STRING, PARAM_END }, },
{ AP_PING,                 { PARAM_END }, },
{ 0,                       { PARAM_END }, },    // Must terminate table this way
};

/* Game mode messages */
static client_message game_msg_table[] = { 
{ BP_REQ_MOVE,             { PARAM_COORD, PARAM_COORD, PARAM_BYTE, PARAM_ID, PARAM_END }, },
{ BP_REQ_TURN,             { PARAM_ID, PARAM_WORD, PARAM_END }, },
{ BP_REQ_GET,              { PARAM_ID, PARAM_END }, },
{ BP_REQ_INVENTORY,        { PARAM_END }, },
{ BP_REQ_INVENTORY_MOVE,   { PARAM_ID, PARAM_ID, PARAM_END }, },
{ BP_REQ_DROP,             { PARAM_OBJECT, PARAM_END }, },
{ BP_REQ_PUT,              { PARAM_OBJECT, PARAM_ID, PARAM_END }, },
{ BP_SAY_TO,               { PARAM_SAY_INFO, PARAM_STRING, PARAM_END }, },
{ BP_SAY_GROUP,            { PARAM_ID_LIST, PARAM_STRING, PARAM_END }, },
{ BP_REQ_LOOK,             { PARAM_ID, PARAM_END }, },
{ BP_REQ_USE,              { PARAM_ID, PARAM_END }, },
{ BP_REQ_UNUSE,            { PARAM_ID, PARAM_END }, },
{ BP_REQ_ATTACK,           { PARAM_ATTACK_INFO, PARAM_ID }, },
{ BP_PING,                 { PARAM_END }, },
{ BP_REQ_OFFER,            { PARAM_ID, PARAM_OBJECT_LIST, PARAM_END }, },
{ BP_REQ_DEPOSIT,          { PARAM_ID, PARAM_OBJECT_LIST, PARAM_END }, },
{ BP_CANCEL_OFFER,         { PARAM_END }, },
{ BP_ACCEPT_OFFER,         { PARAM_END }, },
{ BP_REQ_COUNTEROFFER,     { PARAM_OBJECT_LIST, PARAM_END }, },
{ BP_REQ_GO,               { PARAM_END }, },
{ BP_REQ_ACTIVATE,         { PARAM_ID, PARAM_END }, },
{ BP_REQ_BUY,              { PARAM_ID, PARAM_END }, },
{ BP_REQ_BUY_ITEMS,        { PARAM_ID, PARAM_OBJECT_LIST, PARAM_END }, },
{ BP_REQ_WITHDRAWAL,       { PARAM_ID, PARAM_END }, },
{ BP_REQ_WITHDRAWAL_ITEMS, { PARAM_ID, PARAM_OBJECT_LIST, PARAM_END }, },
{ BP_SEND_PLAYERS,         { PARAM_END }, },
{ BP_REQ_QUIT,             { PARAM_END }, },
{ BP_REQ_SHOOT,            { PARAM_COORD, PARAM_COORD, PARAM_END }, },
{ BP_REQ_APPLY,            { PARAM_ID, PARAM_ID, PARAM_END }, },
{ BP_REQ_CAST,             { PARAM_ID, PARAM_OBJECT_LIST, PARAM_END }, },
{ BP_RESYNC,               { PARAM_END }, },
{ BP_SEND_PLAYER,          { PARAM_END }, },
{ BP_SEND_ROOM_CONTENTS,   { PARAM_END }, },
{ BP_SEND_OBJECT_CONTENTS, { PARAM_ID, PARAM_END }, },
{ BP_ACTION,               { PARAM_BYTE, PARAM_END }, },
{ BP_SAY_BLOCKED,          { PARAM_ID, PARAM_END }, },
{ BP_LOGOFF,               { PARAM_END }, },
{ BP_CHANGE_DESCRIPTION,   { PARAM_ID, PARAM_STRING, PARAM_END }, },
{ BP_CHANGE_PASSWORD,      { PARAM_STRING, PARAM_STRING, PARAM_END }, },
{ BP_REQ_HIDE,             { PARAM_ID, PARAM_END }, },
{ BP_REQ_ADMIN,			   { PARAM_STRING, PARAM_END }, },	//	AJM -- added to support ActiveX admin interface
{ 0,                       { PARAM_END }, },    // Must terminate table this way
};

// BP_USERCOMMAND messages
client_message user_msg_table[] = {
{ UC_CHANGE_URL,           { PARAM_ID, PARAM_STRING, PARAM_END, }, },
{ UC_REQ_RESCUE,           { PARAM_END, }, },
{ UC_REQ_TIME,           { PARAM_END, }, },
{ 0,                       { PARAM_END, }, },    // Must end table this way
};

#define BUFSIZE 5000
static unsigned char buf[BUFSIZE];  /* Buffer for storing message to be sent out */

static void Insert(BYTE **buf, void *data, UINT numbytes);
static WORD InsertString(BYTE **buf, char *str);
/********************************************************************/
/* 
 * Insert: Copy numbytes bytes from data to buf, and increment
 *    buf by numbytes.
 */
void Insert(BYTE **buf, void *data, UINT numbytes)
{
   memcpy(*buf, data, numbytes);
   *buf += numbytes;
}
/********************************************************************/
/* 
 * InsertString: Insert 1 word for the string's length, and then
 *   the string itself, without the null character.  str must be 
 *   null terminated, however.
 *   Returns the length of str.
 */
WORD InsertString(BYTE **buf, char *str)
{
   WORD len = (WORD)strlen(str);

   Insert(buf, &len, SIZE_STRING_LEN);
   Insert(buf, str, len);
   return len;
}
/********************************************************************/
/*
 * Logoff: Send REQ_QUIT to server and close connection.
 */
void Logoff(void)
{
   if (state == STATE_GAME)
      RequestQuit();
   CloseConnection();
   MainSetState(STATE_OFFLINE);
}
/********************************************************************/
/*
 * ToServer:  Send a message to the server.  
 *   "type" gives the type of the message.
 *   The variable arguments are inserted into the message.  They are
 *   parsed according to "table" (an array of size "num_entries"),
 *   which contains one entry for each possible message type.
 *   If "table" is NULL, a default table is chosen depending on the
 *   current state.
 */
void _cdecl ToServer(BYTE type, ClientMsgTable table, ...)
{
   va_list marker;
   int i, index, argnum;
   int integer, count;
   long temp;
   WORD len;
   list_type l, list;
   param_type *params;
   char *str;
   BYTE *ptr;
   ID *array;
   object_node *obj;

   if (table == NULL)
      switch(state)
      {
      case STATE_LOGIN:
      case STATE_DOWNLOAD:
	 table = login_msg_table;
	 break;
      case STATE_GAME:
	 table = game_msg_table;
	 break;
      default:
	 debug(("ToServer called without message table\n"));
	 return;
      }

   va_start(marker, table);
   ptr = buf;               /* Current position in buf */

   // If message type is BP_SYSTEM, add this to buffer, and parse message according to
   // secondary message type, which should be the first argument.
   if (type == BP_SYSTEM)
   {
      *ptr = BP_SYSTEM;
      ptr++;
      type = va_arg(marker, BYTE);
   }
   else if (type == BP_USERCOMMAND)
   {
      // Same method with user commands
      *ptr = (BYTE) BP_USERCOMMAND;
      ptr++;
      type = va_arg(marker, BYTE);
   }

   /* Find message type in table */
   index = 0;
   while (table[index].type != 0)
   {
      if (type == table[index].type)
	 break;
      index++;
   }

   if (table[index].type == 0)
   {
      debug(("Unknown message type %d in ToServer\n", (int) type));
      return;
   }

   params = table[index].params;
   argnum = 0;
   *ptr = type;
   ptr++;

   while (params[argnum] != PARAM_END)
   {
      switch (params[argnum])
      {
      case PARAM_INDEX:
      case PARAM_ID:
	 temp = GetObjId(va_arg(marker, ID));
	 Insert(&ptr, &temp, SIZE_ID);
	 break;

      case PARAM_OBJECT:
	 /* If object's is tagged, then also send amount of object */
	 obj = va_arg(marker, object_node * );	 
	 if (IsNumberObj(obj->id) && obj->temp_amount <= 0)
	    break;
	 Insert(&ptr, &obj->id, SIZE_ID);
	 if (IsNumberObj(obj->id))
	    Insert(&ptr, &obj->temp_amount, SIZE_AMOUNT);	    
	 break;

      case PARAM_NEWSID:
      case PARAM_COORD:
      case PARAM_WORD:
	 temp = va_arg(marker, WORD);
	 Insert(&ptr, &temp, 2);
	 break;

      case PARAM_INT:
	 integer = va_arg(marker, int);
	 Insert(&ptr, &integer, 4);
	 break;

      case PARAM_BYTE:
      case PARAM_GROUP:
      case PARAM_ATTACK_INFO:
      case PARAM_SAY_INFO:
	 temp = va_arg(marker, BYTE);
	 Insert(&ptr, &temp, 1);
	 break;

      case PARAM_STRING:
	 str = va_arg(marker, char *);
	 len = InsertString(&ptr, str);
	 break;
	 
      case PARAM_ID_ARRAY:
	 temp = va_arg(marker, int);
	 len = (WORD) temp;
	 Insert(&ptr, &len, SIZE_NUM_RECIPIENTS);

	 array = va_arg(marker, ID *);
	 	 
	 for (i = 0; i < len; i++)
	    Insert(&ptr, &array[i], SIZE_ID);
	 break;
	 
      case PARAM_OBJECT_LIST:
	 list = va_arg(marker, list_type);
	 len = (WORD) list_length(list);

	 // We send the list, but we cull out any zero or negative numberitems.
	 // Note that this means that we might send a zero element list.
	 //
	 //Insert(&ptr, &len, SIZE_LIST_LEN);
	 
	 count = 0;
	 for (l = list; l != NULL; l = l->next)
	 {
	    obj = (object_node *) l->data;
	    if (IsNumberObj(obj->id) && obj->temp_amount <= 0)
	       continue;
	    count++;
	 }
	 Insert(&ptr, &count, SIZE_LIST_LEN);

	 for (l = list; l != NULL; l = l->next)
	 {
	    obj = (object_node *) l->data;
	    if (IsNumberObj(obj->id) && obj->temp_amount <= 0)
	       continue;
	    Insert(&ptr, &obj->id, SIZE_ID);
	    if (IsNumberObj(obj->id))
	       Insert(&ptr, &obj->temp_amount, SIZE_AMOUNT);
	 }
	 break;

      case PARAM_ID_LIST:
	 list = va_arg(marker, list_type);
	 len = (WORD) list_length(list);
	 Insert(&ptr, &len, SIZE_LIST_LEN);
	 
	 for (l = list; l != NULL; l = l->next)
	    Insert(&ptr, &l->data, SIZE_ID);
	 break;

      case PARAM_INT_ARRAY:
	 len = va_arg(marker, int);
	 Insert(&ptr, &len, SIZE_LIST_LEN);
	 
	 array = va_arg(marker, ID *);
	 for (i = 0; i < len; i++)
	 {
	    integer = array[i];
	    Insert(&ptr, &integer, SIZE_VALUE);
	 }
	 break;
      }
      argnum++;
   }   
   va_end(marker);
   SendServer((char *) buf, ptr - buf);

   // Record the fact that we've sent a messasge
   PingSentMessage();
}
