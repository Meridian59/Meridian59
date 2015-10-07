// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* ccode.c
*

  This module has all of the C functions that are callable from Blakod.
  The parameters describe what class/message called the function, its
  parameters, and its local variables.  The return value of these
  functions is really a val_type, and is returned to the Blakod.
  
*/

#include "blakserv.h"

#define iswhite(c) ((c)==' ' || (c)=='\t' || (c)=='\n' || (c)=='\r')

// global buffers for zero-terminated string manipulation
static char buf0[LEN_MAX_CLIENT_MSG+1];
static char buf1[LEN_MAX_CLIENT_MSG+1];

/* just like strstr, except any case-insensitive match will be returned */
const char* stristr(const char* pSource, const char* pSearch)
{
   if (!pSource || !pSearch || !*pSearch)
      return NULL;
	
   int nSearch = strlen(pSearch);
   // Don't search past the end of pSource
   const char *pEnd = pSource + strlen(pSource) - nSearch;
   while (pSource <= pEnd)
   {
      if (0 == strnicmp(pSource, pSearch, nSearch))
         return pSource;

      pSource++;
   }
	
   return NULL;
}


int C_Invalid(int object_id,local_var_type *local_vars,
			  int num_normal_parms,parm_node normal_parm_array[],
			  int num_name_parms,parm_node name_parm_array[])
{
	bprintf("C_Invalid called--bad C function number");
	return NIL;
}

/*
 * C_SaveGame: Performs a system save, but without garbage collection. We
 *    can't garbage collect when the game is saved from blakod as object,
 *    list, timer and string references (in local vars) may be incorrect when
 *    control passes back to the calling message. Returns a blakod string
 *    containing the time of the saved game if successful.
 */
int C_SaveGame(int object_id,local_var_type *local_vars,
               int num_normal_parms,parm_node normal_parm_array[],
               int num_name_parms,parm_node name_parm_array[])
{
   val_type ret_val;
   int save_time = 0;
   string_node *snod;
   char timeStr[15];

   PauseTimers();
   lprintf("C_SaveGame saving game\n");
   save_time = SaveAll();
   UnpauseTimers();

   // Check for a sane time value.
   if (save_time < 0 || save_time > INT_MAX)
   {
      bprintf("C_SaveGame got invalid save game time!");
      return NIL;
   }

   ret_val.v.tag = TAG_STRING;
   ret_val.v.data = CreateString("");

   snod = GetStringByID(ret_val.v.data);
   if (snod == NULL)
   {
      bprintf("C_SaveGame can't set invalid string %i,%i\n",
         ret_val.v.tag, ret_val.v.data);
      return NIL;
   }

   // Make a string with the save game time.
   sprintf(timeStr, "%d", save_time);

   // Make a blakod string using the string value of the save game time.
   SetString(snod, timeStr, 10);

   return ret_val.int_val;
}

/*
 * C_LoadGame: Takes a blakod string as a parameter, which contains a save
 *    game time.  Posts a message to the blakserv main thread which triggers
 *    a load game, using the save game time value sent in the message. All
 *    users are disconnected when the game reload triggers.
 */
int C_LoadGame(int object_id, local_var_type *local_vars,
               int num_normal_parms, parm_node normal_parm_array[],
               int num_name_parms, parm_node name_parm_array[])
{
   val_type game_val;
   string_node *snod;
   int save_time = 0;

   game_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
      normal_parm_array[0].value);

   if (game_val.v.tag != TAG_STRING)
   {
      bprintf("C_LoadGame can't process invalid string %i,%i\n",
         game_val.v.tag, game_val.v.data);
      return NIL;
   }

   snod = GetStringByID(game_val.v.data);
   if (snod == NULL)
   {
      bprintf("C_LoadGame can't get invalid string %i,%i\n",
         game_val.v.tag, game_val.v.data);
      return NIL;
   }

   // Convert string time to integer.
   save_time = atoi(snod->data);

   // Check for a sane time value.
   if (save_time < 0 || save_time > INT_MAX)
   {
      bprintf("C_LoadGame got invalid save game time!");
      return NIL;
   }

   PostThreadMessage(main_thread_id, WM_BLAK_MAIN_LOAD_GAME, 0, save_time);

   return NIL;
}

int C_AddPacket(int object_id,local_var_type *local_vars,
				int num_normal_parms,parm_node normal_parm_array[],
				int num_name_parms,parm_node name_parm_array[])
{
	int i;
	val_type send_len,send_data;
	
	i = 0;
	while (i < num_normal_parms)
	{
		send_len = RetrieveValue(object_id,local_vars,normal_parm_array[i].type,
			normal_parm_array[i].value);
		i++;
		if (i >= num_normal_parms)
		{
			bprintf("C_AddPacket has # of bytes, needs object\n");
			break;
		}
		
		send_data = RetrieveValue(object_id,local_vars,normal_parm_array[i].type,
			normal_parm_array[i].value);
		i++;
		AddBlakodToPacket(send_len,send_data);
	}
	
	return NIL;
}

int C_SendPacket(int object_id,local_var_type *local_vars,
				 int num_normal_parms,parm_node normal_parm_array[],
				 int num_name_parms,parm_node name_parm_array[])
{
	val_type temp;
	
	temp = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (temp.v.tag != TAG_SESSION)
	{
		bprintf("C_SendPacket object %i can't send to non-session %i,%i\n",
			object_id,temp.v.tag,temp.v.data);
		return NIL;
	}
	
	SendPacket(temp.v.data);
	
	return NIL;
}

int C_SendCopyPacket(int object_id,local_var_type *local_vars,
					 int num_normal_parms,parm_node normal_parm_array[],
					 int num_name_parms,parm_node name_parm_array[])
{
	val_type temp;
	
	temp = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (temp.v.tag != TAG_SESSION)
	{
		bprintf("C_SendPacket object %i can't send to non-session %i,%i\n",
			object_id,temp.v.tag,temp.v.data);
		return NIL;
	}
	
	SendCopyPacket(temp.v.data);
	
	return NIL;
}

int C_ClearPacket(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	ClearPacket();
	
	return NIL;
}

int C_GodLog(int object_id,local_var_type *local_vars,
			int num_normal_parms,parm_node normal_parm_array[],
			int num_name_parms,parm_node name_parm_array[])
{
	int i;
	val_type each_val;
	class_node *c;
	char buf[2000];
	kod_statistics *kstat;
	
	/* need the current interpreting class in case there are debug strings,
	which are stored in the class. */

	kstat = GetKodStats();
	
	c = GetClassByID(kstat->interpreting_class);
	if (c == NULL)
	{
		bprintf("C_GodLog can't find class %i, can't print out debug strs\n",
			kstat->interpreting_class);
		return NIL;
	}
	
	sprintf(buf,"[%s] ",BlakodDebugInfo());
	
	for (i=0;i<num_normal_parms;i++)
	{
		each_val = RetrieveValue(object_id,local_vars,normal_parm_array[i].type,
			normal_parm_array[i].value);
		
		switch (each_val.v.tag)
		{
		case TAG_DEBUGSTR :
			sprintf(buf+strlen(buf),"%s",GetClassDebugStr(c,each_val.v.data));
			break;
			
		case TAG_RESOURCE :
			{
				resource_node *r;
				r = GetResourceByID(each_val.v.data);
				if (r == NULL)
				{
					sprintf(buf+strlen(buf),"<unknown RESOURCE %i>",each_val.v.data);
				}
				else
				{
					sprintf(buf+strlen(buf),"%s",r->resource_val[0]);
				}
			}
			break;
			
		case TAG_INT :
			sprintf(buf+strlen(buf),"%d",(int)each_val.v.data);
			break;
			
		case TAG_CLASS :
			{
				class_node *c;
				c = GetClassByID(each_val.v.data);
				if (c == NULL)
				{
					sprintf(buf+strlen(buf),"<unknown CLASS %i>",each_val.v.data);
				}
				else
				{
					strcat(buf,"&");
					strcat(buf,c->class_name);
				}
			}
			break;
			
		case TAG_STRING :
			{
				int lenBuffer, lenString;
				string_node *snod = GetStringByID(each_val.v.data);
				
				if (snod == NULL)
				{
					bprintf("C_GodLog can't find string %i\n",each_val.v.data);
					return NIL;
				}
				lenString = snod->len_data;
				lenBuffer = strlen(buf);
				memcpy(buf + lenBuffer,snod->data,snod->len_data);
				*(buf + lenBuffer + snod->len_data) = 0;
			}
			break;
			
		case TAG_TEMP_STRING :
			{
				int len_buf;
				string_node *snod;
				
				snod = GetTempString();
				len_buf = strlen(buf);
				memcpy(buf + len_buf,snod->data,snod->len_data);
				*(buf + len_buf + snod->len_data) = 0;
			}
			break;
			
		case TAG_OBJECT :
			{
				object_node *o;
				class_node *c;
				user_node *u;
				
				/* for objects, print object number */
				
				o = GetObjectByID(each_val.v.data);
				if (o == NULL)
				{
					sprintf(buf+strlen(buf),"<OBJECT %i invalid>",each_val.v.data);
					break;
				}
				c = GetClassByID(o->class_id);
				if (c == NULL)
				{
					sprintf(buf+strlen(buf),"<OBJECT %i unknown class>",each_val.v.data);
					break;
				}
				
				if (c->class_id == USER_CLASS || c->class_id == DM_CLASS ||
					c->class_id == GUEST_CLASS || c->class_id == ADMIN_CLASS)
				{
					u = GetUserByObjectID(o->object_id);
					if (u == NULL)
					{
						sprintf(buf+strlen(buf),"<OBJECT %i broken user>",each_val.v.data);
						break;
					}
					sprintf(buf+strlen(buf),"OBJECT %i",each_val.v.data);
					break;
				}
			}
			//FALLTHRU
		default :
			sprintf(buf+strlen(buf),"%s %s",GetTagName(each_val),GetDataName(each_val));
			break;
      }
   }
   gprintf("%s\n",buf);
   return NIL;
}

int C_Debug(int object_id,local_var_type *local_vars,
			int num_normal_parms,parm_node normal_parm_array[],
			int num_name_parms,parm_node name_parm_array[])
{
	int i;
	val_type each_val;
	class_node *c;
	char buf[2000];
	kod_statistics *kstat;
	
	/* need the current interpreting class in case there are debug strings,
	which are stored in the class. */

	kstat = GetKodStats();
	
	c = GetClassByID(kstat->interpreting_class);
	if (c == NULL)
	{
		bprintf("C_Debug can't find class %i, can't print out debug strs\n",
			kstat->interpreting_class);
		return NIL;
	}
	
	sprintf(buf,"[%s] ",BlakodDebugInfo());
	
	for (i=0;i<num_normal_parms;i++)
	{
		each_val = RetrieveValue(object_id,local_vars,normal_parm_array[i].type,
			normal_parm_array[i].value);
		
		switch (each_val.v.tag)
		{
		case TAG_DEBUGSTR :
			sprintf(buf+strlen(buf),"%s",GetClassDebugStr(c,each_val.v.data));
			break;
			
		case TAG_RESOURCE :
			{
				resource_node *r;
				r = GetResourceByID(each_val.v.data);
				if (r == NULL)
				{
					sprintf(buf+strlen(buf),"<unknown RESOURCE %i>",each_val.v.data);
				}
				else
				{
					sprintf(buf+strlen(buf),"%s",r->resource_val[0]);
				}
			}
			break;
			
		case TAG_INT :
			sprintf(buf+strlen(buf),"%d",(int)each_val.v.data);
			break;
			
		case TAG_CLASS :
			{
				class_node *c;
				c = GetClassByID(each_val.v.data);
				if (c == NULL)
				{
					sprintf(buf+strlen(buf),"<unknown CLASS %i>",each_val.v.data);
				}
				else
				{
					strcat(buf,"&");
					strcat(buf,c->class_name);
				}
			}
			break;
			
		case TAG_STRING :
			{
				int lenBuffer, lenString;
				string_node *snod = GetStringByID(each_val.v.data);
				
				if (snod == NULL)
				{
					bprintf("C_Debug can't find string %i\n",each_val.v.data);
					return NIL;
				}
				lenString = snod->len_data;
				lenBuffer = strlen(buf);
				memcpy(buf + lenBuffer,snod->data,snod->len_data);
				*(buf + lenBuffer + snod->len_data) = 0;
			}
			break;
			
		case TAG_TEMP_STRING :
			{
				int len_buf;
				string_node *snod;
				
				snod = GetTempString();
				len_buf = strlen(buf);
				memcpy(buf + len_buf,snod->data,snod->len_data);
				*(buf + len_buf + snod->len_data) = 0;
			}
			break;
			
		case TAG_OBJECT :
			{
				object_node *o;
				class_node *c;
				user_node *u;
				
				/* for objects, print account if it's a user */
				
				o = GetObjectByID(each_val.v.data);
				if (o == NULL)
				{
					sprintf(buf+strlen(buf),"<OBJECT %i invalid>",each_val.v.data);
					break;
				}
				c = GetClassByID(o->class_id);
				if (c == NULL)
				{
					sprintf(buf+strlen(buf),"<OBJECT %i unknown class>",each_val.v.data);
					break;
				}
				
				if (c->class_id == USER_CLASS || c->class_id == DM_CLASS ||
					c->class_id == GUEST_CLASS || c->class_id == ADMIN_CLASS)
				{
					u = GetUserByObjectID(o->object_id);
					if (u == NULL)
					{
						sprintf(buf+strlen(buf),"<OBJECT %i broken user>",each_val.v.data);
						break;
					}
					sprintf(buf+strlen(buf),"ACCOUNT %i OBJECT %i",u->account_id,each_val.v.data);
					break;
				}
			}
			//FALLTHRU
		default :
			sprintf(buf+strlen(buf),"%s %s",GetTagName(each_val),GetDataName(each_val));
			break;
      }
      
      if (i != num_normal_parms-1)
		  sprintf(buf+strlen(buf),",");
   }
   dprintf("%s\n",buf);
   return NIL;
}

int C_GetInactiveTime(int object_id,local_var_type *local_vars,
					  int num_normal_parms,parm_node normal_parm_array[],
					  int num_name_parms,parm_node name_parm_array[])
{
	val_type session_val,ret_val;
	session_node *s;
	
	session_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	
	if (session_val.v.tag != TAG_SESSION)
	{
		bprintf("C_GetInactiveTime can't use non-session %i,%i\n",
			session_val.v.tag,session_val.v.data);
		return NIL;
	}
	
	s = GetSessionByID(session_val.v.data);
	if (s == NULL)
	{
		bprintf("C_GetInactiveTime can't find session %i\n",session_val.v.data);
		return NIL;
	}
	if (s->state != STATE_GAME)
	{
		bprintf("C_GetInactiveTime can't use session %i in state %i\n",
			session_val.v.data,s->state);
		return NIL;
	}
	
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = GetTime() - s->game->game_last_message_time;
	
	return ret_val.int_val;   
}

int C_DumpStack(int object_id,local_var_type *local_vars,
					  int num_normal_parms,parm_node normal_parm_array[],
					  int num_name_parms,parm_node name_parm_array[])
{
	char buf[2000];

	sprintf(buf,"Stack:\n%s\n",BlakodStackInfo());
	dprintf("%s",buf);

	return NIL;
}

int C_SendMessage(int object_id,local_var_type *local_vars,
            int num_normal_parms,parm_node normal_parm_array[],
            int num_name_parms,parm_node name_parm_array[])
{
   val_type object_val,message_val;

   /* Get the object (or class or int) to which we are sending the message */
   /* Not to be confused with object_id, which is the 'self' object sending the message */
   object_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
      normal_parm_array[0].value);

   /* Handle the message to send first; that way other errors are more descriptive */
   message_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
      normal_parm_array[1].value);
   if (message_val.v.tag != TAG_MESSAGE)
   {
      // Handle message names passed as strings.
      if (message_val.v.tag == TAG_STRING)
      {
         message_val.v.data = GetIDByName(GetStringByID(message_val.v.data)->data);
         if (message_val.v.data == INVALID_ID)
         {
            bprintf("C_SendMessage OBJECT %i can't use bad string message %i,\n",
               object_id, message_val.v.tag);
            return NIL;
         }
      }
      else
      {
         bprintf("C_SendMessage OBJECT %i can't send non-message %i,%i\n",
               object_id, message_val.v.tag, message_val.v.data);
         return NIL;
      }
   }

   if (object_val.v.tag == TAG_OBJECT)
      return SendBlakodMessage(object_val.v.data, message_val.v.data, num_name_parms, name_parm_array);

   if (object_val.v.tag == TAG_INT)
   {
      /* Can send to built-in objects using constants. */
      object_val.v.data = GetBuiltInObjectID(object_val.v.data);
      if (object_val.v.data > INVALID_OBJECT)
         return SendBlakodMessage(object_val.v.data, message_val.v.data,
                     num_name_parms, name_parm_array);
   }

   if (object_val.v.tag == TAG_CLASS)
      return SendBlakodClassMessage(object_val.v.data, message_val.v.data, num_name_parms, name_parm_array);

   /* Assumes object_id (the current 'self') is a valid object */
   bprintf("C_SendMessage OBJECT %i CLASS %s can't send MESSAGE %s (%i) to non-object %i,%i\n",
      object_id,
      GetClassByID(GetObjectByID(object_id)->class_id)->class_name,
      GetNameByID(message_val.v.data), message_val.v.data,
      object_val.v.tag,object_val.v.data);
   return NIL;
}

int C_PostMessage(int object_id,local_var_type *local_vars,
            int num_normal_parms,parm_node normal_parm_array[],
            int num_name_parms,parm_node name_parm_array[])
{
   val_type object_val,message_val;

   object_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
      normal_parm_array[0].value);

   /* Handle the message to send first; that way other errors are more descriptive */
   message_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
      normal_parm_array[1].value);
   if (message_val.v.tag != TAG_MESSAGE)
   {
      // Handle message names passed as strings.
      if (message_val.v.tag == TAG_STRING)
      {
         message_val.v.data = GetIDByName(GetStringByID(message_val.v.data)->data);
         if (message_val.v.data == INVALID_ID)
         {
            bprintf("C_PostMessage OBJECT %i can't use bad string message %i,\n",
               object_id, message_val.v.tag);
            return NIL;
         }
      }
      else
      {
         bprintf("C_PostMessage OBJECT %i can't send non-messsage %i,%i\n",
            object_id,message_val.v.tag,message_val.v.data);
         return NIL;
      }
   }

   if (object_val.v.tag != TAG_OBJECT)
   {
      /* Assumes object_id (the current 'self') is a valid object */
      bprintf("C_PostMessage OBJECT %i CLASS %s can't send MESSAGE %s (%i) to non-object %i,%i\n",
         object_id,
         GetClassByID(GetObjectByID(object_id)->class_id)->class_name,
         GetNameByID(message_val.v.data), message_val.v.data,
         object_val.v.tag,object_val.v.data);
      return NIL;
   }

   PostBlakodMessage(object_val.v.data,message_val.v.data,
      num_name_parms,name_parm_array);
   return NIL;
}

/*
* C_SendListMessage: Takes a list, a list position (n), a message and message
*   parameters. If n = 0, sends the message to all objects in the list given.
*   If n > 1, sends the message to the Nth object in each element of the list
*   given, which should be a list containing sublists. Handles TRUE and FALSE
*   returns from the messages, returns TRUE by default and FALSE if any called
*   object returns FALSE. Rationale is that a call to multiple objects would
*   be looking for a FALSE condition, not a TRUE one.
*/
int C_SendListMessage(int object_id,local_var_type *local_vars,
            int num_normal_parms,parm_node normal_parm_array[],
            int num_name_parms,parm_node name_parm_array[])
{
   val_type list_val, pos_val, message_val, ret_val;

   ret_val.v.tag = TAG_INT;
   ret_val.v.data = True;

   // Get the list we're going to use.
   list_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
      normal_parm_array[0].value);

   // If $ list, just return.
   if (list_val.v.tag == TAG_NIL)
      return ret_val.int_val;

   // List 'position', 0 for obj in top list, >0 for obj = Nth(list,pos).
   pos_val = RetrieveValue(object_id, local_vars, normal_parm_array[1].type,
      normal_parm_array[1].value);
   if (pos_val.v.tag != TAG_INT || pos_val.v.data < 0)
   {
      bprintf("C_SendListMessage OBJECT %i can't use non-int list pos %i, %i\n",
         object_id, pos_val.v.tag, pos_val.v.data);
      return ret_val.int_val;
   }

   // Get the message to send, either a message ID or a string containing a message name.
   message_val = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
      normal_parm_array[2].value);
   if (message_val.v.tag != TAG_MESSAGE)
   {
      // Handle message names passed as strings.
      if (message_val.v.tag == TAG_STRING)
      {
         message_val.v.data = GetIDByName(GetStringByID(message_val.v.data)->data);
         if (message_val.v.data == INVALID_ID)
         {
            bprintf("C_SendListMessage OBJECT %i can't use bad string message %i,\n",
               object_id, message_val.v.tag);
            return ret_val.int_val;
         }
      }
      else
      {
         bprintf("C_SendListMessage OBJECT %i can't send non-message %i,%i\n",
               object_id, message_val.v.tag, message_val.v.data);
         return ret_val.int_val;
      }
   }

   if (list_val.v.tag != TAG_LIST)
   {
      bprintf("C_SendListMessage OBJECT %i can't send to non-list %i, %i\n",
         object_id, list_val.v.tag, list_val.v.data);
      return ret_val.int_val;
   }

   // Separate functions handle each of the cases: objects in top list, objects
   // as first element of sublist, objects as nth element of sublist.
   if (pos_val.v.data == 0)
      ret_val.v.data = SendListMessage(list_val.v.data, False, message_val.v.data,
         num_name_parms, name_parm_array);
   else if (pos_val.v.data == 1)
      ret_val.v.data = SendFirstListMessage(list_val.v.data, False, message_val.v.data,
         num_name_parms, name_parm_array);
   else
      ret_val.v.data = SendNthListMessage(list_val.v.data, pos_val.v.data, False,
         message_val.v.data, num_name_parms, name_parm_array);

   return ret_val.int_val;
}

/*
* C_SendListMessageBreak: Works the same as SendListMessage, except
*   breaks on the first FALSE return from the sent messages. Used to
*   speed up algorithms that rely on calling every object in a list
*   until reaching a FALSE return.
*/
int C_SendListMessageBreak(int object_id, local_var_type *local_vars,
            int num_normal_parms, parm_node normal_parm_array[],
            int num_name_parms, parm_node name_parm_array[])
{
   val_type list_val, pos_val, message_val, ret_val;

   ret_val.v.tag = TAG_INT;
   ret_val.v.data = True;

   // Get the list we're going to use.
   list_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
      normal_parm_array[0].value);

   // If $ list, just return.
   if (list_val.v.tag == TAG_NIL)
      return ret_val.int_val;

   // List 'position', 0 for obj in top list, >0 for obj = Nth(list,pos).
   pos_val = RetrieveValue(object_id, local_vars, normal_parm_array[1].type,
      normal_parm_array[1].value);
   if (pos_val.v.tag != TAG_INT || pos_val.v.data < 0)
   {
      bprintf("C_SendListMessageBreak OBJECT %i can't use non-int list pos %i, %i\n",
         object_id, pos_val.v.tag, pos_val.v.data);
      return ret_val.int_val;
   }

   // Get the message to send, either a message ID or a string containing a message name.
   message_val = RetrieveValue(object_id, local_vars, normal_parm_array[2].type,
      normal_parm_array[2].value);
   if (message_val.v.tag != TAG_MESSAGE)
   {
      // Handle message names passed as strings.
      if (message_val.v.tag == TAG_STRING)
      {
         message_val.v.data = GetIDByName(GetStringByID(message_val.v.data)->data);
         if (message_val.v.data == INVALID_ID)
         {
            bprintf("C_SendListMessageBreak OBJECT %i can't use bad string message %i,\n",
               object_id, message_val.v.tag);
            return ret_val.int_val;
         }
      }
      else
      {
         bprintf("C_SendListMessageBreak OBJECT %i can't send non-message %i,%i\n",
            object_id, message_val.v.tag, message_val.v.data);
         return ret_val.int_val;
      }
   }

   if (list_val.v.tag != TAG_LIST)
   {
      bprintf("C_SendListMessageBreak OBJECT %i can't send to non-list %i, %i\n",
         object_id, list_val.v.tag, list_val.v.data);
      return ret_val.int_val;
   }

   // Separate functions handle each of the cases: objects in top list, objects
   // as first element of sublist, objects as nth element of sublist.
   if (pos_val.v.data == 0)
      ret_val.v.data = SendListMessage(list_val.v.data, True, message_val.v.data,
      num_name_parms, name_parm_array);
   else if (pos_val.v.data == 1)
      ret_val.v.data = SendFirstListMessage(list_val.v.data, True, message_val.v.data,
      num_name_parms, name_parm_array);
   else
      ret_val.v.data = SendNthListMessage(list_val.v.data, pos_val.v.data, True,
      message_val.v.data, num_name_parms, name_parm_array);

   return ret_val.int_val;
}

/*
* C_SendListMessageByClass: Takes a list, a list position (n), a class, a
*   message and message parameters. Works the same as C_SendListMessage,
*   except the message is only sent to objects of the given class.
*/
int C_SendListMessageByClass(int object_id, local_var_type *local_vars,
            int num_normal_parms, parm_node normal_parm_array[],
            int num_name_parms, parm_node name_parm_array[])
{
   val_type list_val, pos_val, message_val, class_val, ret_val;

   ret_val.v.tag = TAG_INT;
   ret_val.v.data = True;

   // Get the list we're going to use.
   list_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
      normal_parm_array[0].value);

   // If $ list, just return.
   if (list_val.v.tag == TAG_NIL)
      return ret_val.int_val;

   // List 'position', 0 for obj in top list, >0 for obj = Nth(list,pos).
   pos_val = RetrieveValue(object_id, local_vars, normal_parm_array[1].type,
      normal_parm_array[1].value);
   if (pos_val.v.tag != TAG_INT || pos_val.v.data < 0)
   {
      bprintf("C_SendListMessageByClass OBJECT %i can't use non-int list pos %i, %i\n",
         object_id, pos_val.v.tag, pos_val.v.data);
      return ret_val.int_val;
   }

   // Get the class we want to send to.
   class_val = RetrieveValue(object_id, local_vars, normal_parm_array[2].type,
      normal_parm_array[2].value);
   if (class_val.v.tag != TAG_CLASS)
   {
      bprintf("C_SendListMessageByClass OBJECT %i can't use non-class %i, %i\n",
         object_id, class_val.v.tag, class_val.v.data);
      return ret_val.int_val;
   }

   // Get the message to send, either a message ID or a string containing a message name.
   message_val = RetrieveValue(object_id, local_vars, normal_parm_array[3].type,
      normal_parm_array[3].value);
   if (message_val.v.tag != TAG_MESSAGE)
   {
      // Handle message names passed as strings.
      if (message_val.v.tag == TAG_STRING)
      {
         message_val.v.data = GetIDByName(GetStringByID(message_val.v.data)->data);
         if (message_val.v.data == INVALID_ID)
         {
            bprintf("C_SendListMessageByClass OBJECT %i can't use bad string message %i,\n",
               object_id, message_val.v.tag);
            return ret_val.int_val;
         }
      }
      else
      {
         bprintf("C_SendListMessageByClass OBJECT %i can't send non-message %i,%i\n",
            object_id, message_val.v.tag, message_val.v.data);
         return ret_val.int_val;
      }
   }

   if (list_val.v.tag != TAG_LIST)
   {
      bprintf("C_SendListMessageByClass OBJECT %i can't send to non-list %i, %i\n",
         object_id, list_val.v.tag, list_val.v.data);
      return ret_val.int_val;
   }

   // Separate functions handle each of the cases: objects in top list, objects
   // as first element of sublist, objects as nth element of sublist.
   if (pos_val.v.data == 0)
      ret_val.v.data = SendListMessageByClass(list_val.v.data, class_val.v.data, False,
         message_val.v.data, num_name_parms, name_parm_array);
   else if (pos_val.v.data == 1)
      ret_val.v.data = SendFirstListMessageByClass(list_val.v.data, class_val.v.data, False,
         message_val.v.data, num_name_parms, name_parm_array);
   else
      ret_val.v.data = SendNthListMessageByClass(list_val.v.data, pos_val.v.data, 
         class_val.v.data, False, message_val.v.data, num_name_parms, name_parm_array);

   return ret_val.int_val;
}

/*
* C_SendListMessageByClassBreak: Works the same as SendListMessageByClass,
*   except breaks on the first FALSE return from the sent messages. Used to
*   speed up algorithms that rely on calling every object in a list until
*   reaching a FALSE return.
*/
int C_SendListMessageByClassBreak(int object_id, local_var_type *local_vars,
   int num_normal_parms, parm_node normal_parm_array[],
   int num_name_parms, parm_node name_parm_array[])
{
   val_type list_val, pos_val, message_val, class_val, ret_val;

   ret_val.v.tag = TAG_INT;
   ret_val.v.data = True;

   // Get the list we're going to use.
   list_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
      normal_parm_array[0].value);

   // If $ list, just return.
   if (list_val.v.tag == TAG_NIL)
      return ret_val.int_val;

   // List 'position', 0 for obj in top list, >0 for obj = Nth(list,pos).
   pos_val = RetrieveValue(object_id, local_vars, normal_parm_array[1].type,
      normal_parm_array[1].value);
   if (pos_val.v.tag != TAG_INT || pos_val.v.data < 0)
   {
      bprintf("C_SendListMessageByClassBreak OBJECT %i can't use non-int list pos %i, %i\n",
         object_id, pos_val.v.tag, pos_val.v.data);
      return ret_val.int_val;
   }

   // Get the class we want to send to.
   class_val = RetrieveValue(object_id, local_vars, normal_parm_array[2].type,
      normal_parm_array[2].value);
   if (class_val.v.tag != TAG_CLASS)
   {
      bprintf("C_SendListMessageByClassBreak OBJECT %i can't use non-class %i, %i\n",
         object_id, class_val.v.tag, class_val.v.data);
      return ret_val.int_val;
   }

   // Get the message to send, either a message ID or a string containing a message name.
   message_val = RetrieveValue(object_id, local_vars, normal_parm_array[3].type,
      normal_parm_array[3].value);
   if (message_val.v.tag != TAG_MESSAGE)
   {
      // Handle message names passed as strings.
      if (message_val.v.tag == TAG_STRING)
      {
         message_val.v.data = GetIDByName(GetStringByID(message_val.v.data)->data);
         if (message_val.v.data == INVALID_ID)
         {
            bprintf("C_SendListMessageByClassBreak OBJECT %i can't use bad string message %i,\n",
               object_id, message_val.v.tag);
            return ret_val.int_val;
         }
      }
      else
      {
         bprintf("C_SendListMessageByClassBreak OBJECT %i can't send non-message %i,%i\n",
            object_id, message_val.v.tag, message_val.v.data);
         return ret_val.int_val;
      }
   }

   if (list_val.v.tag != TAG_LIST)
   {
      bprintf("C_SendListMessageByClassBreak OBJECT %i can't send to non-list %i, %i\n",
         object_id, list_val.v.tag, list_val.v.data);
      return ret_val.int_val;
   }

   // Separate functions handle each of the cases: objects in top list, objects
   // as first element of sublist, objects as nth element of sublist.
   if (pos_val.v.data == 0)
      ret_val.v.data = SendListMessageByClass(list_val.v.data, class_val.v.data, True,
      message_val.v.data, num_name_parms, name_parm_array);
   else if (pos_val.v.data == 1)
      ret_val.v.data = SendFirstListMessageByClass(list_val.v.data, class_val.v.data, True,
      message_val.v.data, num_name_parms, name_parm_array);
   else
      ret_val.v.data = SendNthListMessageByClass(list_val.v.data, pos_val.v.data,
      class_val.v.data, True, message_val.v.data, num_name_parms, name_parm_array);

   return ret_val.int_val;
}

int C_CreateObject(int object_id,local_var_type *local_vars,
				   int num_normal_parms,parm_node normal_parm_array[],
				   int num_name_parms,parm_node name_parm_array[])
{
	val_type ret_val,class_val;
	
	class_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
			     normal_parm_array[0].value);
	if (class_val.v.tag != TAG_CLASS)
	{
		bprintf("C_CreateObject can't create non-class %i,%i\n",
			class_val.v.tag,class_val.v.data);
		return NIL;
	}
	
	ret_val.v.tag = TAG_OBJECT;
	ret_val.v.data = CreateObject(class_val.v.data,num_name_parms,name_parm_array);
	return ret_val.int_val;
}

int C_IsClass(int object_id,local_var_type *local_vars,
			  int num_normal_parms,parm_node normal_parm_array[],
			  int num_name_parms,parm_node name_parm_array[])
{
	val_type object_val,class_val,ret_val;
	object_node *o;
	class_node *c;
	
	object_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (object_val.v.tag != TAG_OBJECT)
	{
		bprintf("C_IsClass can't deal with non-object %i,%i\n",
			object_val.v.tag,object_val.v.data);
		return NIL;
	}
	
	class_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
			     normal_parm_array[1].value);
	if (class_val.v.tag != TAG_CLASS)
	{
		bprintf("C_IsClass can't look for non-class %i,%i\n",
			class_val.v.tag,class_val.v.data);
		return NIL;
	}
	
	o = GetObjectByID(object_val.v.data);
	if (o == NULL)
	{
		bprintf("C_IsClass can't find object %i\n",object_val.v.data);
		return NIL;
	}
	
	c = GetClassByID(o->class_id);
	if (c == NULL)
	{
		bprintf("C_IsClass can't find class %i, DIE totally\n",o->class_id);
		FlushDefaultChannels();
		return NIL;
	}
	
	do
	{
		if (c->class_id == class_val.v.data)
		{
			ret_val.v.tag = TAG_INT;
			ret_val.v.data = True;
			return ret_val.int_val;
		}
		c = c->super_ptr;
	} while (c != NULL);
	
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = False;
	return ret_val.int_val;
}

int C_GetClass(int object_id,local_var_type *local_vars,
			   int num_normal_parms,parm_node normal_parm_array[],
			   int num_name_parms,parm_node name_parm_array[])
{
	val_type object_val,ret_val;
	object_node *o;
	
	object_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (object_val.v.tag != TAG_OBJECT)
	{
		bprintf("C_GetClass can't deal with non-object %i,%i\n",
			object_val.v.tag,object_val.v.data);
		return NIL;
	}
	
	o = GetObjectByID(object_val.v.data);
	if (o == NULL)
	{
		bprintf("C_GetClass can't find object %i\n",object_val.v.data);
		return NIL;
	}
	
	ret_val.v.tag = TAG_CLASS;
	ret_val.v.data = o->class_id;
	return ret_val.int_val;
}

// Look up the string given by val.  If found, return true and set *str and *len
// to the string value and length respectively.  function_name is the C function
// name used in reporting errors.
// If the string isn't found (including if val corresponds to NIL), false is returned.
bool LookupString(val_type val, const char *function_name, const char **str, int *len)
{
	string_node *snod;

	switch(val.v.tag)
	{
	case TAG_STRING :
		snod = GetStringByID(val.v.data);
		if (snod == NULL)
		{
			bprintf( "%s can't use invalid string %i,%i\n",
                  function_name, val.v.tag, val.v.data );
			return false;
		}
		*str = snod->data;
		break;
		
	case TAG_TEMP_STRING :
		snod = GetTempString();
		*str = snod->data;
		break;
		
	case TAG_RESOURCE :
      *str = GetResourceStrByLanguageID(val.v.data, ConfigInt(RESOURCE_LANGUAGE));
      if (*str == NULL)
		{
			bprintf( "%s can't use invalid resource %i as string\n",
                  function_name, val.v.data );
			return false;
		}
		break;
		
	case TAG_DEBUGSTR :
   {
      kod_statistics *kstat;
      class_node *c;
		
      kstat = GetKodStats();
		
      c = GetClassByID(kstat->interpreting_class);
      if (c == NULL)
      {
         bprintf("%s can't find class %i, can't get debug str\n",
                 function_name, kstat->interpreting_class);
         return false;
      }
      *str = GetClassDebugStr(c, val.v.data);
      break;
   }

   case TAG_NIL:
		bprintf( "%s can't use nil as string\n", function_name );
      return false;
   
	default :
		bprintf( "%s can't use with non-string thing %i,%i\n",
               function_name, val.v.tag, val.v.data );
		return false;
	}

   if (*str == NULL)
      return false;
   *len = strlen(*str);
   
   return true;
}


int C_StringEqual(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
   val_type s1_val, s2_val, ret_val;
   const char *s1 = NULL, *s2 = NULL;
   int len1, len2;
   resource_node *r1 = NULL, *r2 = NULL;
   Bool s1_resource = False, s2_resource = False;

   ret_val.v.tag = TAG_INT;
   ret_val.v.data = False;

   s1_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
      normal_parm_array[0].value);
   s2_val = RetrieveValue(object_id, local_vars, normal_parm_array[1].type,
      normal_parm_array[1].value);

   if (s1_val.v.tag == TAG_RESOURCE)
   {
      r1 = GetResourceByID(s1_val.v.data);
      if (r1 == NULL)
      {
         bprintf("C_StringEqual can't use invalid resource %i as string\n",
            s1_val.v.data);
         return ret_val.int_val;
      }
      s1_resource = True;
   }
   else
   {
      if (!LookupString(s1_val, "C_StringEqual", &s1, &len1))
         return NIL;
   }

   if (s2_val.v.tag == TAG_RESOURCE)
   {
      r2 = GetResourceByID(s2_val.v.data);
      if (r2 == NULL)
      {
         bprintf("C_StringEqual can't use invalid resource %i as string\n",
            s2_val.v.data);
         return ret_val.int_val;
      }
      s2_resource = True;
   }
   else
   {
      if (!LookupString(s2_val, "C_StringEqual", &s2, &len2))
         return NIL;
   }

   // If both are resources, just compare the English string (first array position).
   if (s1_resource && s2_resource)
   {
      s1 = r1->resource_val[0];
      if (s1 == NULL)
      {
         bprintf("C_StringEqual got NULL string resource 1");
         return ret_val.int_val;
      }
      s2 = r2->resource_val[0];
      if (s2 == NULL)
      {
         bprintf("C_StringEqual got NULL string resource 2");
         return ret_val.int_val;
      }

      len1 = strlen(s1);
      len2 = strlen(s2);
      ret_val.v.data = FuzzyBufferEqual(s1, len1, s2, len2);

      return ret_val.int_val;
   }

   // First string is resource, second isn't.
   if (s1_resource)
   {
      len2 = strlen(s2);
      for (int i = 0; i < MAX_LANGUAGE_ID; i++)
      {
         s1 = r1->resource_val[i];
         if (s1 == NULL)
         {
            if (i == 0)
            {
               bprintf("C_StringEqual got NULL string resource 1");
               return ret_val.int_val;
            }
            continue;
         }

         len1 = strlen(s1);
         if (FuzzyBufferEqual(s1, len1, s2, len2))
         {
            ret_val.v.data = True;
            return ret_val.int_val;
         }
      }
      return ret_val.int_val;
   }

   // Second string is resource, first isn't.
   if (s2_resource)
   {
      len1 = strlen(s1);
      for (int i = 0; i < MAX_LANGUAGE_ID; i++)
      {
         s2 = r2->resource_val[i];
         if (s2 == NULL)
         {
            if (i == 0)
            {
               bprintf("C_StringEqual got NULL string resource 2");
               return ret_val.int_val;
            }
            continue;
         }

         len2 = strlen(s2);
         if (FuzzyBufferEqual(s1, len1, s2, len2))
         {
            ret_val.v.data = True;
            return ret_val.int_val;
         }
      }
      return ret_val.int_val;
   }

   // Neither strings are resources.
   len1 = strlen(s1);
   len2 = strlen(s2);
   ret_val.v.data = FuzzyBufferEqual(s1, len1, s2, len2);
   return ret_val.int_val;
}

void FuzzyCollapseString(char* pTarget, const char* pSource, int len)
{
	if (!pTarget || !pSource || len <= 0)
	{
		*pTarget = '\0';
		return;
	}
	
	// skip over leading and trailing whitespace
	while (len && iswhite(*pSource)) { pSource++; len--; }
	while (len && iswhite(pSource[len-1])) { len--; }
	
	// copy the core string in uppercase
	while (len)
	{
		*pTarget++ = toupper(*pSource++);
		len--;
	}
	
	*pTarget = '\0';
}

bool FuzzyBufferEqual(const char *s1,int len1,const char *s2,int len2)
{
	if (!s1 || !s2 || len1 <= 0 || len2 <= 0)
		return false;
	
	// skip over leading whitespace
	while (len1 && iswhite(*s1)) { s1++; len1--; }
	while (len2 && iswhite(*s2)) { s2++; len2--; }
	
	// cut off trailing whitespace
	while (len1 && iswhite(s1[len1-1])) { len1--; }
	while (len2 && iswhite(s2[len2-1])) { len2--; }
	
	// empty strings can't match anything
	if (!len1 || !len2)
		return false;
	
	// walk the strings until we find a mismatch or an end
	while (len1 && len2 && toupper(*s1) == toupper(*s2))
	{
		s1++;
		s2++;
		len1--;
		len2--;
	}
	
	// we matched only if we finished both strings at the same time
	return (len1 == 0 && len2 == 0);
}

//	Blakod parameters; string0, string1, string2
//	Substitute first occurrence of string1 in string0 with string2
//	Returns 1 if substituted, 0 if not found, NIL if error
int C_StringSubstitute(int object_id,local_var_type *local_vars,
					   int num_normal_parms,parm_node normal_parm_array[],
					   int num_name_parms,parm_node name_parm_array[])
{
	val_type s0_val, s1_val, s2_val, r_val;
	string_node *snod0, *snod1;
	char buf0[LEN_MAX_CLIENT_MSG+1], buf1[LEN_MAX_CLIENT_MSG+1];
	char *s0, *copyspot;
   const char *s1, *s2, *subspot;
	int len1, len2, new_len;
	
	s0 = buf0;
	s1 = buf1;
	s2 = subspot = copyspot = NULL;
	
	s0_val = RetrieveValue( object_id, local_vars, normal_parm_array[0].type,
		normal_parm_array[0].value);
	snod0 = NULL;
	if (s0_val.v.tag == TAG_STRING)
		snod0 = GetStringByID(s0_val.v.data);
	else if (s0_val.v.tag == TAG_TEMP_STRING)
		snod0 = GetTempString();
	
	if (snod0 == NULL)
	{
		bprintf( "C_StringSub can't modify first argument non-string %i,%i\n",
			s0_val.v.tag, s0_val.v.data );
		return NIL;
	}
	
	s1_val = RetrieveValue( object_id, local_vars, normal_parm_array[1].type,
		normal_parm_array[1].value );
	
	switch(s1_val.v.tag)
	{
	case TAG_STRING :
		snod1 = GetStringByID( s1_val.v.data);
		if( snod1 == NULL )
		{
			bprintf( "C_StringSub can't sub for invalid string %i,%i\n",
				s1_val.v.tag, s1_val.v.data );
			return NIL;
		}
		
		// make a zero-terminated scratch copy of string1
		len1 = snod1->len_data;
		memcpy( buf1, snod1->data, len1 );
		buf1[len1+1] = 0x0;
		break;
		
	case TAG_TEMP_STRING :
		snod1 = GetTempString();
		
		// make a zero-terminated scratch copy of string1
		len1 = snod1->len_data;
		memcpy( buf1, snod1->data, len1 );
		buf1[len1+1] = 0x0;
		break;
		
	case TAG_RESOURCE :
      s1 = GetResourceStrByLanguageID(s1_val.v.data, ConfigInt(RESOURCE_LANGUAGE));
      if (s1 == NULL)
		{
			bprintf( "C_StringSub can't sub for invalid resource %i\n", s1_val.v.data );
			return NIL;
		}
		len1 = strlen(s1);
		break;
		
	case TAG_DEBUGSTR :
		{
			kod_statistics *kstat;
			class_node *c;
			
			kstat = GetKodStats();
			
			c = GetClassByID(kstat->interpreting_class);
			if (c == NULL)
			{
				bprintf("C_StringSub can't find class %i, can't get debug str\n",
					kstat->interpreting_class);
				return NIL;
			}
			s1 = GetClassDebugStr(c,s1_val.v.data);
			len1 = 0;
			if (s1 != NULL)
				len1 = strlen(s1);
			break;
		}
		
	case TAG_NIL :
		bprintf( "C_StringSub can't sub for nil\n" );
		return NIL;
		
	default :
		bprintf( "C_StringSub can't sub for non-string thing %i,%i\n",
			s1_val.v.tag, s1_val.v.data );
		return NIL;
	}
	
	if( ( len1 < 1 ) || ( len1 > LEN_MAX_CLIENT_MSG ) )
	{
		bprintf( "C_StringSub can't sub for null string %i,%i\n",
			s1_val.v.tag, s1_val.v.data );
		return NIL;
	}
	
	s2_val = RetrieveValue( object_id, local_vars, normal_parm_array[2].type,
		normal_parm_array[2].value );

   if (!LookupString(s2_val, "C_StringSub", &s2, &len2))
      return NIL;
	
	new_len = snod0->len_data - len1 + len2;
	if( new_len > LEN_MAX_CLIENT_MSG )
	{
		bprintf("C_StringSub can't sub, string too long.");
		return NIL;
	}
	
	// now make a zero-terminated scratch copy of string0
	memcpy( s0, snod0->data, snod0->len_data );
	s0[snod0->len_data+1] = 0x0;
	
	// so we can use stristr to do the work
	subspot = stristr( s0, s1 );
	
    r_val.v.tag = TAG_INT;
    r_val.v.data = 0;
	
	if( subspot != NULL )	// only substitute if string1 is found in string0
	{
		char* source;
		int source_len;
		
		source_len = snod0->len_data;
		source = (char *)AllocateMemory(MALLOC_ID_STRING, source_len+1);
		memcpy(source, snod0->data, source_len);
		source[source_len] = '\0';
		
		if (snod0 != GetTempString())
		{
			// free the old string0 and allocate a new (possibly longer) string0
			FreeMemory(MALLOC_ID_STRING,snod0->data,snod0->len_data);
			snod0->data = (char *) AllocateMemory( MALLOC_ID_STRING, new_len+1);
		}
		
		// copy the piece before string1
		copyspot = snod0->data;
		memcpy( copyspot, s0, subspot - s0 );
		
		// copy string2
		copyspot += ( subspot - s0 );
		memcpy( copyspot, s2, len2 );
		
		// copy the piece after string1
		copyspot += len2;
		memcpy( copyspot, subspot + len1, new_len - (subspot - s0) - len2 );
		snod0->len_data = new_len;
		snod0->data[snod0->len_data] = '\0';
		
		FreeMemory(MALLOC_ID_STRING,source,source_len+1);
		
		r_val.v.data = 1;
	}
	
	return r_val.int_val;
}

int C_StringContain(int object_id,local_var_type *local_vars,
               int num_normal_parms,parm_node normal_parm_array[],
               int num_name_parms,parm_node name_parm_array[])
{
   val_type s1_val, s2_val, ret_val;
   const char *s1 = NULL,*s2 = NULL;
   int len1, len2;
   resource_node *r1 = NULL, *r2 = NULL;
   Bool s1_resource = False, s2_resource = False;
   
   ret_val.v.tag = TAG_INT;
   ret_val.v.data = False;

   s1_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
      normal_parm_array[0].value);
   s2_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
      normal_parm_array[1].value);

   if (s1_val.v.tag == TAG_RESOURCE)
   {
      r1 = GetResourceByID(s1_val.v.data);
      if (r1 == NULL)
      {
         bprintf("C_StringContain can't use invalid resource %i as string\n",
            s1_val.v.data);
         return ret_val.int_val;
      }
      s1_resource = True;
   }
   else
   {
      if (!LookupString(s1_val, "C_StringContain", &s1, &len1))
         return NIL;
   }

   if (s2_val.v.tag == TAG_RESOURCE)
   {
      r2 = GetResourceByID(s2_val.v.data);
      if (r2 == NULL)
      {
         bprintf("C_StringContain can't use invalid resource %i as string\n",
            s2_val.v.data);
         return ret_val.int_val;
      }
      s2_resource = True;
   }
   else
   {
      if (!LookupString(s2_val, "C_StringContain", &s2, &len2))
         return NIL;
   }

   // If both are resources, just compare the English string (first array position).
   if (s1_resource && s2_resource)
   {
      s1 = r1->resource_val[0];
      if (s1 == NULL)
      {
         bprintf("C_StringContain got NULL string resource 1");
         return ret_val.int_val;
      }
      s2 = r2->resource_val[0];
      if (s2 == NULL)
      {
         bprintf("C_StringContain got NULL string resource 2");
         return ret_val.int_val;
      }

      len1 = strlen(s1);
      len2 = strlen(s2);
      ret_val.v.data = FuzzyBufferContain(s1, len1, s2, len2);

      return ret_val.int_val;
   }

   // First string is resource, second isn't.
   if (s1_resource)
   {
      len2 = strlen(s2);
      ret_val.v.tag = TAG_INT;
      for (int i = 0; i < MAX_LANGUAGE_ID; i++)
      {
         s1 = r1->resource_val[i];
         if (s1 == NULL)
         {
            if (i == 0)
            {
               bprintf("C_StringContain got NULL string resource 1");
               return ret_val.int_val;
            }
            continue;
         }

         len1 = strlen(s1);
         if (FuzzyBufferContain(s1, len1, s2, len2))
         {
            ret_val.v.data = True;
            return ret_val.int_val;
         }
      }
      return ret_val.int_val;
   }

   // Second string is resource, first isn't.
   if (s2_resource)
   {
      len1 = strlen(s1);
      ret_val.v.tag = TAG_INT;
      for (int i = 0; i < MAX_LANGUAGE_ID; i++)
      {
         s2 = r2->resource_val[i];
         if (s2 == NULL)
         {
            if (i == 0)
            {
               bprintf("C_StringContain got NULL string resource 2");
               return ret_val.int_val;
            }
            continue;
         }

         len2 = strlen(s2);
         if (FuzzyBufferContain(s1, len1, s2, len2))
         {
            ret_val.v.data = True;
            return ret_val.int_val;
         }
      }
      return ret_val.int_val;
   }

   // Neither strings are resources.
   len1 = strlen(s1);
   len2 = strlen(s2);
   ret_val.v.data = FuzzyBufferContain(s1, len1, s2, len2);
   return ret_val.int_val;
}

/*
"   orc teeth" == "orc teeth"
" orc  teeth" == "orc teeth"
"orcteeth" == "orc teeth"
*/

// return true if s1 contains s2,
//	first converting to uppercase and squashing tabs and spaces to a single space
bool FuzzyBufferContain(const char *s1,int len_s1,const char *s2,int len_s2)
{
	if (!s1 || !s2 || len_s1 <= 0 || len_s2 <= 0)
		return false;
	
	FuzzyCollapseString(buf0, s1, len_s1);
	FuzzyCollapseString(buf1, s2, len_s2);
	
	return (NULL != strstr(buf0, buf1));
}

int C_SetResource(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type drsc_val,str_val;
	resource_node *r;
	string_node *snod;
	int new_len;
	char *new_str;
	
	drsc_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (drsc_val.v.tag != TAG_RESOURCE)
	{
		bprintf("C_SetResource can't set non-resource %i,%i\n",
			drsc_val.v.tag,drsc_val.v.data);
		return NIL;
	}
	
	if (drsc_val.v.data < MIN_DYNAMIC_RSC)
	{
		bprintf("C_SetResource can't set non-dynamic resource %i,%i\n",
			drsc_val.v.tag,drsc_val.v.data);
		return NIL;
	}
	
	str_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	switch (str_val.v.tag)
	{
	case TAG_TEMP_STRING :
		snod = GetTempString();
		new_len = snod->len_data;
		new_str = snod->data;
		break;
		
	case TAG_RESOURCE :
		{
			r = GetResourceByID(str_val.v.data);
			if (r == NULL)
			{
				bprintf("C_SetResource can't set from bad resource %i\n",
					str_val.v.data);
				return NIL;
			}
			new_len = strlen(r->resource_val[0]);
			new_str = r->resource_val[0];
			break;
		}
	case TAG_STRING :
		snod = GetStringByID(str_val.v.data);
		if (snod == NULL)
		{
			bprintf( "C_SetResource can't set from bad string %i\n",
				str_val.v.data);
			return NIL;
		}
		new_len = snod->len_data;
		new_str = snod->data;
		break;
	default :
		bprintf("C_SetResource can't set from non temp string %i,%i\n",
			str_val.v.tag,str_val.v.data);
		return NIL;
	}
	
	r = GetResourceByID(drsc_val.v.data);
	if (r == NULL)
	{
		eprintf("C_SetResource got dyna rsc number that doesn't exist\n");
		return NIL;
	}
	
	ChangeDynamicResource(r,new_str,new_len);
	
	return NIL;
}

int C_ParseString(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type parse_str_val,separator_str_val,callback_val,string_val;
	parm_node p[1];
	string_node *snod;
	const char *separators;
	kod_statistics *kstat;
	class_node *c;
	char *each_str;
	
	kstat = GetKodStats();
	
	if (kstat->interpreting_class == INVALID_CLASS)
	{
		eprintf("C_ParseString can't find current class\n");
		return NIL;
	}
	
	c = GetClassByID(kstat->interpreting_class);
	if (c == NULL)
	{
		eprintf("C_ParseString can't find class %i\n",kstat->interpreting_class);
		return NIL;
	}
	
	parse_str_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (parse_str_val.v.tag != TAG_TEMP_STRING)
	{
		bprintf("C_ParseString can't parse non-temp string %i,%i\n",
			parse_str_val.v.tag,parse_str_val.v.data);
		return NIL;
	}
	
	snod = GetTempString();
	/* null terminate it to do strtok */
	snod->data[std::min(LEN_TEMP_STRING-1,snod->len_data)] = 0;
	
	separator_str_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	if (separator_str_val.v.tag != TAG_DEBUGSTR)
	{
		bprintf("C_ParseString can't use separator non-debugstr %i,%i\n",
			separator_str_val.v.tag,separator_str_val.v.data);
		return NIL;
	}
	separators = GetClassDebugStr(c,separator_str_val.v.data);
	
	callback_val = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
		normal_parm_array[2].value);
	if (callback_val.v.tag != TAG_MESSAGE)
	{
		bprintf("C_ParseString can't callback non-message %i,%i\n",
			callback_val.v.tag,callback_val.v.data);
		return NIL;
	}
	
	/* setup our parameter to callback */
	string_val.v.tag = TAG_TEMP_STRING;
	string_val.v.data = 0;
	
	p[0].type = CONSTANT;
	p[0].value = string_val.int_val;
	p[0].name_id = STRING_PARM;
	
	each_str = strtok(snod->data,separators);
	while (each_str != NULL)
	{
	/* move the parsed string to beginning of the temp string for kod's use.
	also, fake the length on it for kod's sake.  Doesn't matter to
		us because we null terminated the real string*/
		
		strcpy(snod->data,each_str);
		snod->len_data = strlen(snod->data);
		
		SendBlakodMessage(object_id,callback_val.v.data,1,p);
		
		each_str = strtok(NULL,separators);
	}
	return NIL;
}

int C_SetString(int object_id,local_var_type *local_vars,
            int num_normal_parms,parm_node normal_parm_array[],
            int num_name_parms,parm_node name_parm_array[])
{
   val_type s1_val,s2_val;
   string_node *snod,*snod2;
   const char *str;

   s1_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
      normal_parm_array[0].value);

   if (s1_val.v.tag != TAG_STRING)
   {
      /* If we're passed a NULL string, create one and use that. Allows
       * us to create and set a string with one call.*/
      if (s1_val.v.tag == TAG_NIL)
      {
         s1_val.v.tag = TAG_STRING;
         s1_val.v.data = CreateString("");
      }
      else
      {
         bprintf("C_SetString can't set non-string %i,%i\n",
            s1_val.v.tag,s1_val.v.data);
         return NIL;
      }
   }

   snod = GetStringByID(s1_val.v.data);
   if (snod == NULL)
   {
      bprintf("C_SetString can't set invalid string %i,%i\n",
         s1_val.v.tag,s1_val.v.data);
      return NIL;
   }

   s2_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
      normal_parm_array[1].value);
   switch (s2_val.v.tag)
   {
   case TAG_STRING :
      snod2 = GetStringByID( s2_val.v.data);
      if( snod2 == NULL )
      {
         bprintf( "C_SetString can't find string %i,%i\n",
            s2_val.v.tag, s2_val.v.data );
         return NIL;
      }
      //bprintf("SetString string%i<--string%i\n",s1_val.v.data,s2_val.v.data);
      SetString(snod,snod2->data,snod2->len_data);
      break;

   case TAG_TEMP_STRING :
      snod2 = GetTempString();
      //bprintf("SetString string%i<--tempstring\n",s1_val.v.data);
      SetString(snod,snod2->data,snod2->len_data);
      break;

   case TAG_RESOURCE :
      str = GetResourceStrByLanguageID(s2_val.v.data, ConfigInt(RESOURCE_LANGUAGE));
      if (str == NULL)
      {
         bprintf("C_SetString can't set from invalid resource %i\n",s2_val.v.data);
         return NIL;
      }
      //bprintf("SetString string%i<--resource%i\n",s1_val.v.data,s2_val.v.data);
      SetString(snod, (char*)str, strlen(str));
      break;

   case TAG_MESSAGE :
      str = GetNameByID(s2_val.v.data);
      if (str == NULL)
      {
         bprintf("C_SetString can't set from invalid message %i\n",s2_val.v.data);
         return NIL;
      }
      SetString(snod,GetNameByID(s2_val.v.data),strlen(GetNameByID(s2_val.v.data)));
      break;

   case TAG_DEBUGSTR :
      str = GetClassDebugStr(GetClassByID(GetKodStats()->interpreting_class),s2_val.v.data);
      if (str == NULL)
      {
         bprintf("C_SetString can't set from invalid debug string %i\n",s2_val.v.data);
         return NIL;
      }
      SetString(snod,(char*)str,strlen(str));
      break;

   default :
      bprintf("C_SetString can't set from non-string thing %i,%i\n",
         s2_val.v.tag,s2_val.v.data);
      return NIL;
   }

   return s1_val.int_val;
}

int C_ClearTempString(int object_id,local_var_type *local_vars,
					  int num_normal_parms,parm_node normal_parm_array[],
					  int num_name_parms,parm_node name_parm_array[])
{
	val_type ret_val;
	
	ClearTempString();
	
	ret_val.v.tag = TAG_TEMP_STRING;
	ret_val.v.data = 0;		/* doesn't matter for TAG_TEMP_STRING */
	return ret_val.int_val;
}

int C_GetTempString(int object_id,local_var_type *local_vars,
					int num_normal_parms,parm_node normal_parm_array[],
					int num_name_parms,parm_node name_parm_array[])
{
	val_type ret_val;
	
	ret_val.v.tag = TAG_TEMP_STRING;
	ret_val.v.data = 0;		/* doesn't matter for TAG_TEMP_STRING */
	return ret_val.int_val;
}

int C_AppendTempString(int object_id,local_var_type *local_vars,
					   int num_normal_parms,parm_node normal_parm_array[],
					   int num_name_parms,parm_node name_parm_array[])
{
	val_type s_val;
	string_node *snod;
	
	s_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	
	switch (s_val.v.tag)
	{
	case TAG_INT :
		AppendNumToTempString(s_val.v.data);
		break;
		
	case TAG_STRING :
		snod = GetStringByID( s_val.v.data);
		if(snod == NULL )
		{
			bprintf( "C_AppendTempString can't find string %i,%i\n", s_val.v.tag, s_val.v.data );
			return NIL;
		}
		AppendTempString(snod->data,snod->len_data);
		break;
		
	case TAG_TEMP_STRING :
		bprintf("C_AppendTempString attempting to append temp string to itself!\n");
		return NIL;
		
   case TAG_RESOURCE:
   {
      const char *pStrConst;
      pStrConst = GetResourceStrByLanguageID(s_val.v.data, ConfigInt(RESOURCE_LANGUAGE));
      if (pStrConst == NULL)
      {
         bprintf("C_AppendTempString can't set from invalid resource %i\n", s_val.v.data);
         return NIL;
      }
      AppendTempString(pStrConst, strlen(pStrConst));
   }
      break;
	case TAG_DEBUGSTR :
		{
			kod_statistics *kstat = GetKodStats();
			class_node *c = GetClassByID(kstat->interpreting_class);
			const char *pStrConst;
			int strLen = 0;
			
			if (c == NULL)
			{
				bprintf("C_AppendTempString can't find class %i, can't get debug str\n",kstat->interpreting_class);
				return NIL;
			}
			pStrConst = GetClassDebugStr(c,s_val.v.data);
			strLen = 0;
			if (pStrConst != NULL)
			{
				strLen = strlen(pStrConst);
				AppendTempString(pStrConst,strLen);
			}
			else
			{
				bprintf("C_AppendTempString: GetClassDebugStr returned NULL");
				return NIL;
			}
		}
		break;
		
	case TAG_NIL :
		bprintf("C_AppendTempString can't set from NIL\n");
		break;
		
	default :
		bprintf("C_AppendTempString can't set from non-string thing %i,%i\n",s_val.v.tag,s_val.v.data);
		return NIL;
	}
	return NIL;
}

int C_CreateString(int object_id,local_var_type *local_vars,
				   int num_normal_parms,parm_node normal_parm_array[],
				   int num_name_parms,parm_node name_parm_array[])
{
	val_type ret_val;
	
	ret_val.v.tag = TAG_STRING;
	ret_val.v.data = CreateString("");
	
	return ret_val.int_val;
}

int C_IsString(int object_id,local_var_type *local_vars,
         int num_normal_parms,parm_node normal_parm_array[],
         int num_name_parms,parm_node name_parm_array[])
{
   val_type var_check,ret_val;

   var_check = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
            normal_parm_array[0].value);

   ret_val.v.tag = TAG_INT;
   if (var_check.v.tag == TAG_STRING)
      ret_val.v.data = True;
   else
      ret_val.v.data = False;

   return ret_val.int_val;
}

int C_StringLength(int object_id,local_var_type *local_vars,
				   int num_normal_parms,parm_node normal_parm_array[],
				   int num_name_parms,parm_node name_parm_array[])
{
	val_type s1_val,ret_val;
	const char *s1;
	int len;
	
	s1_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
   if (!LookupString(s1_val, "C_StringLength", &s1, &len))
      return NIL;
   
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = len;
	
	return ret_val.int_val;
}

int C_StringConsistsOf(int object_id,local_var_type *local_vars,
                       int num_normal_parms,parm_node normal_parm_array[],
                       int num_name_parms,parm_node name_parm_array[])
{
	val_type s1_val,s2_val,ret_val;
	const char *s1,*s2;
	int len1,len2;
	
	s1_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
   if (!LookupString(s1_val, "C_StringConsistsOf", &s1, &len1))
      return NIL;
	
	s2_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
   if (!LookupString(s2_val, "C_StringConsistsOf", &s2, &len2))
      return NIL;

   // See if all characters in s1 are from s2.
   bool all_found = true;
   for (int i = 0; i < len1; ++i)
   {
      if (strchr(s2, s1[i]) == NULL)
      {
         all_found = false;
         break;
      }
   }
   
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = (int) all_found;
	
	return ret_val.int_val;
}

int C_CreateTimer(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type object_val,message_val,time_val,ret_val;
	object_node *o;
	
	o = GetObjectByID(object_id);
	if (o == NULL)
	{
		eprintf("C_CreateTimer can't find object %i\n",object_id);
		return NIL;
	}
	
	object_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (object_val.v.tag != TAG_OBJECT)
	{
		bprintf("C_CreateTimer can't create a timer for non-object %i,%i\n",
			object_val.v.tag,object_val.v.data);
		return NIL;
	}
	
	message_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	if (message_val.v.tag != TAG_MESSAGE)
	{
		bprintf("C_CreateTimer can't create timer w/ non-message id %i,%i\n",
			message_val.v.tag,message_val.v.data);
		return NIL;
	}
	
	time_val = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
		normal_parm_array[2].value);
	
	if (time_val.v.tag != TAG_INT || time_val.v.data < 0)
	{
		bprintf("C_CreateTimer can't create timer in negative int %i,%i milliseconds\n",
			time_val.v.tag,time_val.v.data);
		return NIL;
	}
	
	if (GetMessageByID(o->class_id,message_val.v.data,NULL) == NULL)
	{
		bprintf("C_CreateTimer can't create timer w/ message %i not for class %i\n",
			message_val.v.data,o->class_id);
		return NIL;
	}
	
	ret_val.v.tag = TAG_TIMER;
	ret_val.v.data = CreateTimer(o->object_id,message_val.v.data,time_val.v.data);
	/*   dprintf("create timer %i\n",ret_val.v.data); */
	
	return ret_val.int_val;
}

int C_DeleteTimer(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type timer_val,ret_val;
	
	timer_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
			     normal_parm_array[0].value);
	if (timer_val.v.tag != TAG_TIMER)
	{
		bprintf("C_DeleteTimer can't delete non-timer %i,%i\n",
			timer_val.v.tag,timer_val.v.data);
		return NIL;
	}
	ret_val.v.tag = TAG_INT; /* really a boolean */
	ret_val.v.data = DeleteTimer(timer_val.v.data);
	
	return ret_val.int_val;
}

int C_GetTimeRemaining(int object_id,local_var_type *local_vars,
					   int num_normal_parms,parm_node normal_parm_array[],
					   int num_name_parms,parm_node name_parm_array[])
{
	val_type timer_val,ret_val;
	timer_node *t;
	
	timer_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
			     normal_parm_array[0].value);
	if (timer_val.v.tag != TAG_TIMER)
	{
		bprintf("C_GetTimeRemaining can't use non-timer %i,%i\n",
			timer_val.v.tag,timer_val.v.data);
		return NIL;
	}
	
	t = GetTimerByID(timer_val.v.data);
	if (t == NULL)
	{
		bprintf("C_GetTimeRemaining can't find timer %i,%i\n",
			timer_val.v.tag,timer_val.v.data);
		return NIL;
	}
	
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = (int)(t->time - GetMilliCount());
	if (ret_val.v.data < 0)
		ret_val.v.data = 0;
	
	return ret_val.int_val;
}

int C_IsTimer(int object_id,local_var_type *local_vars,
			 int num_normal_parms,parm_node normal_parm_array[],
			 int num_name_parms,parm_node name_parm_array[])
{
	val_type var_check,ret_val;
	
	var_check = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
			     normal_parm_array[0].value);
	
	ret_val.v.tag = TAG_INT;
	
	if (var_check.v.tag == TAG_NIL)
	{
		ret_val.v.data = False;
		bprintf("C_IsTimer called with NIL timer by object %i",object_id);
		return ret_val.int_val;
	}
	
	if (var_check.v.tag == TAG_TIMER)
		ret_val.v.data = True;
	else
		ret_val.v.data = False;
	
	return ret_val.int_val;
}

int C_LoadRoom(int object_id,local_var_type *local_vars,
			   int num_normal_parms,parm_node normal_parm_array[],
			   int num_name_parms,parm_node name_parm_array[])
{
	val_type room_val;
	
	room_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (room_val.v.tag != TAG_RESOURCE)
	{
		bprintf("C_CreateRoomData can't use non-resource %i,%i\n",
			room_val.v.tag,room_val.v.data);
		return NIL;
	}
	
	return LoadRoom(room_val.v.data);
}

/*
 * C_FreeRoom: Takes a room's room data (TAG_ROOM_DATA) and removes the
 *             room from the server's list of rooms. Frees the memory
 *             associated with the room.
 */
int C_FreeRoom(int object_id,local_var_type *local_vars,
            int num_normal_parms,parm_node normal_parm_array[],
            int num_name_parms,parm_node name_parm_array[])
{
   val_type room_val;
   room_node *room;

   room_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
      normal_parm_array[0].value);
   if (room_val.v.tag != TAG_ROOM_DATA)
   {
      bprintf("C_FreeRoom can't operate on non-room %i,%i\n",
         room_val.v.tag,room_val.v.data);
      return NIL;
   }

   room = GetRoomDataByID(room_val.v.data);
   if (room == NULL)
   {
      bprintf("C_FreeRoom can't find room id %i\n",room_val.v.data);
      return NIL;
   }

   UnloadRoom(room);

   return NIL;
}

int C_RoomData(int object_id,local_var_type *local_vars,
			   int num_normal_parms,parm_node normal_parm_array[],
			   int num_name_parms,parm_node name_parm_array[])
{
	val_type room_val,ret_val,rows,cols,security,rowshighres,colshighres;
	room_node *room;
	
	room_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (room_val.v.tag != TAG_ROOM_DATA)
	{
		bprintf("C_RoomSize can't operate on non-room %i,%i\n",
			room_val.v.tag,room_val.v.data);
		return NIL;
	}
	
	room = GetRoomDataByID(room_val.v.data);
	if (room == NULL)
	{
		bprintf("C_RoomSize can't find room id %i\n",room_val.v.data);
		return NIL;
	}
	
	rows.v.tag = TAG_INT;
	rows.v.data = room->data.rows;
	cols.v.tag = TAG_INT;
	cols.v.data = room->data.cols;
	security.v.tag = TAG_INT;
	security.v.data = room->data.security;
	rowshighres.v.tag = TAG_INT;
	rowshighres.v.data = room->data.rowshighres;
	colshighres.v.tag = TAG_INT;
	colshighres.v.data = room->data.colshighres;

	ret_val.int_val = NIL;
	
	ret_val.v.data = Cons(colshighres,ret_val);
	ret_val.v.tag = TAG_LIST;
	
	ret_val.v.data = Cons(rowshighres,ret_val);
	ret_val.v.tag = TAG_LIST;
	
	ret_val.v.data = Cons(security,ret_val);
	ret_val.v.tag = TAG_LIST;
	
	ret_val.v.data = Cons(cols,ret_val);
	ret_val.v.tag = TAG_LIST;
	
	ret_val.v.data = Cons(rows,ret_val);
	ret_val.v.tag = TAG_LIST;
	
	return ret_val.int_val;
}

int C_GetLocationInfoBSP(int object_id, local_var_type *local_vars,
	int num_normal_parms, parm_node normal_parm_array[],
	int num_name_parms, parm_node name_parm_array[])
{
	val_type ret_val, room_val, queryflags;
	val_type row, col, finerow, finecol;
	val_type returnflags, floorheight, floorheightwd, ceilingheight, serverid;
	room_node *r;

	ret_val.v.tag = TAG_INT;
	ret_val.v.data = false;

	room_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
		normal_parm_array[0].value);
	queryflags = RetrieveValue(object_id, local_vars, normal_parm_array[1].type,
		normal_parm_array[1].value);
	row = RetrieveValue(object_id, local_vars, normal_parm_array[2].type,
		normal_parm_array[2].value);
	col = RetrieveValue(object_id, local_vars, normal_parm_array[3].type,
		normal_parm_array[3].value);
	finerow = RetrieveValue(object_id, local_vars, normal_parm_array[4].type,
		normal_parm_array[4].value);
	finecol = RetrieveValue(object_id, local_vars, normal_parm_array[5].type,
		normal_parm_array[5].value);

	// local 'out' vars
	returnflags = RetrieveValue(object_id, local_vars, normal_parm_array[6].type,
		normal_parm_array[6].value);
	floorheight = RetrieveValue(object_id, local_vars, normal_parm_array[7].type,
		normal_parm_array[7].value);
	floorheightwd = RetrieveValue(object_id, local_vars, normal_parm_array[8].type,
		normal_parm_array[8].value);
	ceilingheight = RetrieveValue(object_id, local_vars, normal_parm_array[9].type,
		normal_parm_array[9].value);
	serverid = RetrieveValue(object_id, local_vars, normal_parm_array[10].type,
		normal_parm_array[10].value);

	if (room_val.v.tag != TAG_ROOM_DATA)
	{
		bprintf("C_GetLocationInfoBSP can't use non room %i,%i\n",
			room_val.v.tag, room_val.v.data);
		return ret_val.int_val;
	}
	if (queryflags.v.tag != TAG_INT)
	{
		bprintf("C_GetLocationInfoBSP queryflags can't use non int %i,%i\n",
			queryflags.v.tag, queryflags.v.data);
		return ret_val.int_val;
	}
	if (row.v.tag != TAG_INT)
	{
		bprintf("C_GetLocationInfoBSP row source can't use non int %i,%i\n",
			row.v.tag, row.v.data);
		return ret_val.int_val;
	}
	if (col.v.tag != TAG_INT)
	{
		bprintf("C_GetLocationInfoBSP col source can't use non int %i,%i\n",
			col.v.tag, col.v.data);
		return ret_val.int_val;
	}
	if (finerow.v.tag != TAG_INT)
	{
		bprintf("C_GetLocationInfoBSP finerow source can't use non int %i,%i\n",
			finerow.v.tag, finerow.v.data);
		return ret_val.int_val;
	}
	if (finecol.v.tag != TAG_INT)
	{
		bprintf("C_GetLocationInfoBSP finecol source can't use non int %i,%i\n",
			finecol.v.tag, finecol.v.data);
		return ret_val.int_val;
	}

	if (returnflags.v.tag != TAG_INT)
	{
		bprintf("C_GetLocationInfoBSP returnflags can't use non int %i,%i\n",
			returnflags.v.tag, returnflags.v.data);
		return ret_val.int_val;
	}
	if (floorheight.v.tag != TAG_INT)
	{
		bprintf("C_GetLocationInfoBSP floorheight can't use non int %i,%i\n",
			floorheight.v.tag, floorheight.v.data);
		return ret_val.int_val;
	}
	if (floorheightwd.v.tag != TAG_INT)
	{
		bprintf("C_GetLocationInfoBSP floorheightwd can't use non int %i,%i\n",
			floorheightwd.v.tag, floorheightwd.v.data);
		return ret_val.int_val;
	}
	if (ceilingheight.v.tag != TAG_INT)
	{
		bprintf("C_GetLocationInfoBSP ceilingheight can't use non int %i,%i\n",
			ceilingheight.v.tag, ceilingheight.v.data);
		return ret_val.int_val;
	}
	if (serverid.v.tag != TAG_INT)
	{
		bprintf("C_GetLocationInfoBSP serverid can't use non int %i,%i\n",
			serverid.v.tag, serverid.v.data);
		return ret_val.int_val;
	}


	r = GetRoomDataByID(room_val.v.data);
	if (r == NULL)
	{
		bprintf("C_GetLocationInfoBSP can't find room %i\n", room_val.v.data);
		return ret_val.int_val;
	}

	V2 p;
	p.X = GRIDCOORDTOROO(col.v.data, finecol.v.data);
	p.Y = GRIDCOORDTOROO(row.v.data, finerow.v.data);

	// params of query
	unsigned int qflags = (unsigned int)queryflags.v.data;
	unsigned int rflags;
	float heightF, heightFWD, heightC;
	BspLeaf* leaf = NULL;
	
	// query
	bool ok = BSPGetLocationInfo(&r->data, &p, qflags, &rflags, &heightF, &heightFWD, &heightC, &leaf);

	if (ok)
	{
		// set output vars
		local_vars->locals[returnflags.v.data].v.tag = TAG_INT;
		local_vars->locals[returnflags.v.data].v.data = (int)rflags;

		local_vars->locals[floorheight.v.data].v.tag = TAG_INT;
		local_vars->locals[floorheight.v.data].v.data = FLOATTOKODINT(FINENESSROOTOKOD(heightF));

		local_vars->locals[floorheightwd.v.data].v.tag = TAG_INT;
		local_vars->locals[floorheightwd.v.data].v.data = FLOATTOKODINT(FINENESSROOTOKOD(heightFWD));

		local_vars->locals[ceilingheight.v.data].v.tag = TAG_INT;
		local_vars->locals[ceilingheight.v.data].v.data = FLOATTOKODINT(FINENESSROOTOKOD(heightC));

		if (leaf && leaf->Sector)
		{
			local_vars->locals[serverid.v.data].v.tag = TAG_INT;
			local_vars->locals[serverid.v.data].v.data = leaf->Sector->ServerID;
		}

		// mark succeeded
		ret_val.v.data = true;
	}

	return ret_val.int_val;
}

int C_CanMoveInRoomBSP(int object_id, local_var_type *local_vars,
	int num_normal_parms, parm_node normal_parm_array[],
	int num_name_parms, parm_node name_parm_array[])
{
	val_type ret_val, room_val;
	val_type row_source, col_source, finerow_source, finecol_source;
	val_type row_dest, col_dest, finerow_dest, finecol_dest;
	val_type objectid, move_outside_bsp;
	room_node *r;

	ret_val.v.tag = TAG_INT;
	ret_val.v.data = false;

	room_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
		normal_parm_array[0].value);
	row_source = RetrieveValue(object_id, local_vars, normal_parm_array[1].type,
		normal_parm_array[1].value);
	col_source = RetrieveValue(object_id, local_vars, normal_parm_array[2].type,
		normal_parm_array[2].value);
	finerow_source = RetrieveValue(object_id, local_vars, normal_parm_array[3].type,
		normal_parm_array[3].value);
	finecol_source = RetrieveValue(object_id, local_vars, normal_parm_array[4].type,
		normal_parm_array[4].value);

	row_dest = RetrieveValue(object_id, local_vars, normal_parm_array[5].type,
		normal_parm_array[5].value);
	col_dest = RetrieveValue(object_id, local_vars, normal_parm_array[6].type,
		normal_parm_array[6].value);
	finerow_dest = RetrieveValue(object_id, local_vars, normal_parm_array[7].type,
		normal_parm_array[7].value);
	finecol_dest = RetrieveValue(object_id, local_vars, normal_parm_array[8].type,
		normal_parm_array[8].value);
	objectid = RetrieveValue(object_id, local_vars, normal_parm_array[9].type,
		normal_parm_array[9].value);
	move_outside_bsp = RetrieveValue(object_id, local_vars, normal_parm_array[10].type,
		normal_parm_array[10].value);

	if (room_val.v.tag != TAG_ROOM_DATA)
	{
		bprintf("C_CanMoveInRoomBSP can't use non room %i,%i\n",
			room_val.v.tag, room_val.v.data);
		return ret_val.int_val;
	}

	if (row_source.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoomBSP row source can't use non int %i,%i\n",
			row_source.v.tag, row_source.v.data);
		return ret_val.int_val;
	}

	if (col_source.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoomBSP col source can't use non int %i,%i\n",
			col_source.v.tag, col_source.v.data);
		return ret_val.int_val;
	}

	if (finerow_source.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoomBSP finerow source can't use non int %i,%i\n",
			finerow_source.v.tag, finerow_source.v.data);
		return ret_val.int_val;
	}

	if (finecol_source.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoomBSP finecol source can't use non int %i,%i\n",
			finecol_source.v.tag, finecol_source.v.data);
		return ret_val.int_val;
	}

	if (row_dest.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoomBSP row dest can't use non int %i,%i\n",
			row_dest.v.tag, row_dest.v.data);
		return ret_val.int_val;
	}

	if (col_dest.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoomBSP col dest can't use non int %i,%i\n",
			col_dest.v.tag, col_dest.v.data);
		return ret_val.int_val;
	}

	if (finerow_dest.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoomBSP finerow dest can't use non int %i,%i\n",
			finerow_dest.v.tag, finerow_dest.v.data);
		return ret_val.int_val;
	}

	if (finecol_dest.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoomBSP finecol dest can't use non int %i,%i\n",
			finecol_dest.v.tag, finecol_dest.v.data);
		return ret_val.int_val;
	}

	if (objectid.v.tag != TAG_OBJECT)
	{
		bprintf("C_CanMoveInRoomBSP objectid can't use non obj %i,%i\n",
			objectid.v.tag, objectid.v.data);
		return ret_val.int_val;
	}

	if (move_outside_bsp.v.tag != TAG_INT)
	{
		bprintf("C_CanMoveInRoomBSP move outside BSP bool can't use non int %i,%i\n",
			move_outside_bsp.v.tag, move_outside_bsp.v.data);
		return ret_val.int_val;
	}

	r = GetRoomDataByID(room_val.v.data);
	if (r == NULL)
	{
		bprintf("C_CanMoveInRoomBSP can't find room %i\n", room_val.v.data);
		return ret_val.int_val;
	}

	V2 s;
	s.X = GRIDCOORDTOROO(col_source.v.data, finecol_source.v.data);
	s.Y = GRIDCOORDTOROO(row_source.v.data, finerow_source.v.data);

	V2 e;
	e.X = GRIDCOORDTOROO(col_dest.v.data, finecol_dest.v.data);
	e.Y = GRIDCOORDTOROO(row_dest.v.data, finerow_dest.v.data);

	ret_val.v.data = BSPCanMoveInRoom(&r->data, &s, &e, objectid.v.data, (move_outside_bsp.v.data != 0));

#if DEBUGMOVE
	//dprintf("MOVE:%i R:%i S:(%1.2f/%1.2f) E:(%1.2f/%1.2f)", ret_val.v.data, r->data.roomdata_id, s.X, s.Y, e.X, e.Y);
#endif

	return ret_val.int_val;
}

int C_LineOfSightBSP(int object_id, local_var_type *local_vars,
	int num_normal_parms, parm_node normal_parm_array[],
	int num_name_parms, parm_node name_parm_array[])
{
	val_type ret_val, room_val;
	val_type row_source, col_source, finerow_source, finecol_source;
	val_type row_dest, col_dest, finerow_dest, finecol_dest;
	room_node *r;
 
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = false;

	room_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
		normal_parm_array[0].value);
	row_source = RetrieveValue(object_id, local_vars, normal_parm_array[1].type,
		normal_parm_array[1].value);
	col_source = RetrieveValue(object_id, local_vars, normal_parm_array[2].type,
		normal_parm_array[2].value);
	finerow_source = RetrieveValue(object_id, local_vars, normal_parm_array[3].type,
		normal_parm_array[3].value);
	finecol_source = RetrieveValue(object_id, local_vars, normal_parm_array[4].type,
		normal_parm_array[4].value);

	row_dest = RetrieveValue(object_id, local_vars, normal_parm_array[5].type,
		normal_parm_array[5].value);
	col_dest = RetrieveValue(object_id, local_vars, normal_parm_array[6].type,
		normal_parm_array[6].value);
	finerow_dest = RetrieveValue(object_id, local_vars, normal_parm_array[7].type,
		normal_parm_array[7].value);
	finecol_dest = RetrieveValue(object_id, local_vars, normal_parm_array[8].type,
		normal_parm_array[8].value);

	if (room_val.v.tag != TAG_ROOM_DATA)
	{
		bprintf("C_LineOfSightBSP can't use non room %i,%i\n",
			room_val.v.tag, room_val.v.data);
		return ret_val.int_val;
	}

	if (row_source.v.tag != TAG_INT)
	{
		bprintf("C_LineOfSightBSP row source can't use non int %i,%i\n",
			row_source.v.tag, row_source.v.data);
		return ret_val.int_val;
	}

	if (col_source.v.tag != TAG_INT)
	{
		bprintf("C_LineOfSightBSP col source can't use non int %i,%i\n",
			col_source.v.tag, col_source.v.data);
		return ret_val.int_val;
	}

	if (finerow_source.v.tag != TAG_INT)
	{
		bprintf("C_LineOfSightBSP finerow source can't use non int %i,%i\n",
			finerow_source.v.tag, finerow_source.v.data);
		return ret_val.int_val;
	}

	if (finecol_source.v.tag != TAG_INT)
	{
		bprintf("C_LineOfSightBSP finecol source can't use non int %i,%i\n",
			finecol_source.v.tag, finecol_source.v.data);
		return ret_val.int_val;
	}

	if (row_dest.v.tag != TAG_INT)
	{
		bprintf("C_LineOfSightBSP row dest can't use non int %i,%i\n",
			row_dest.v.tag, row_dest.v.data);
		return ret_val.int_val;
	}

	if (col_dest.v.tag != TAG_INT)
	{
		bprintf("C_LineOfSightBSP col dest can't use non int %i,%i\n",
			col_dest.v.tag, col_dest.v.data);
		return ret_val.int_val;
	}

	if (finerow_dest.v.tag != TAG_INT)
	{
		bprintf("C_LineOfSightBSP finerow dest can't use non int %i,%i\n",
			finerow_dest.v.tag, finerow_dest.v.data);
		return ret_val.int_val;
	}

	if (finecol_dest.v.tag != TAG_INT)
	{
		bprintf("C_LineOfSightBSP finecol dest can't use non int %i,%i\n",
			finecol_dest.v.tag, finecol_dest.v.data);
		return ret_val.int_val;
	}

	r = GetRoomDataByID(room_val.v.data);
	if (r == NULL)
	{
		bprintf("C_LineOfSightBSP can't find room %i\n", room_val.v.data);
		return ret_val.int_val;
	}

	BspLeaf* leaf;
	float tmp1;
	float tmp2;

	V3 s;
	s.X = GRIDCOORDTOROO(col_source.v.data, finecol_source.v.data);
	s.Y = GRIDCOORDTOROO(row_source.v.data, finerow_source.v.data);

	// use floor height with depth modifier
	V2 s2d = { s.X, s.Y };
	BSPGetHeight(&r->data, &s2d, &tmp1, &s.Z, &tmp2, &leaf);
	s.Z += OBJECTHEIGHTROO;

	V3 e;
	e.X = GRIDCOORDTOROO(col_dest.v.data, finecol_dest.v.data);
	e.Y = GRIDCOORDTOROO(row_dest.v.data, finerow_dest.v.data);

	// use floor height with depth modifier
	V2 e2d = { e.X, e.Y };
	BSPGetHeight(&r->data, &e2d, &tmp1, &e.Z, &tmp2, &leaf);
	e.Z += OBJECTHEIGHTROO;

	ret_val.v.data = BSPLineOfSight(&r->data, &s, &e);

#if DEBUGLOS
	dprintf("LOS:%i S:(%1.2f/%1.2f/%1.2f) E:(%1.2f/%1.2f/%1.2f)", ret_val.v.data, s.X, s.Y, s.Z, e.X, e.Y, e.Z);
#endif

	return ret_val.int_val;
}

int C_ChangeTextureBSP(int object_id, local_var_type *local_vars,
    int num_normal_parms, parm_node normal_parm_array[],
    int num_name_parms, parm_node name_parm_array[])
{
	val_type ret_val, room_val, server_id, new_texnum, flags;
	room_node *r;

	ret_val.v.tag = TAG_INT;
	ret_val.v.data = false;

	room_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
		normal_parm_array[0].value);
	server_id = RetrieveValue(object_id, local_vars, normal_parm_array[1].type,
		normal_parm_array[1].value);
	new_texnum = RetrieveValue(object_id, local_vars, normal_parm_array[2].type,
		normal_parm_array[2].value);
	flags = RetrieveValue(object_id, local_vars, normal_parm_array[3].type,
		normal_parm_array[3].value);

	if (room_val.v.tag != TAG_ROOM_DATA)
	{
		bprintf("C_ChangeTextureBSP can't use non room %i,%i\n",
			room_val.v.tag, room_val.v.data);
		return ret_val.int_val;
	}

	if (server_id.v.tag != TAG_INT)
	{
		bprintf("C_ChangeTextureBSP serverid can't use non int %i,%i\n",
			server_id.v.tag, server_id.v.data);
		return ret_val.int_val;
	}

	if (new_texnum.v.tag != TAG_INT)
	{
		bprintf("C_ChangeTextureBSP new_texnum can't use non int %i,%i\n",
			new_texnum.v.tag, new_texnum.v.data);
		return ret_val.int_val;
	}

	if (flags.v.tag != TAG_INT)
	{
		bprintf("C_ChangeTextureBSP flags can't use non int %i,%i\n",
			flags.v.tag, flags.v.data);
		return ret_val.int_val;
	}

	r = GetRoomDataByID(room_val.v.data);
	if (r == NULL)
	{
		bprintf("C_ChangeTextureBSP can't find room %i\n", room_val.v.data);
		return ret_val.int_val;
	}

	BSPChangeTexture(&r->data, (unsigned short)server_id.v.data, 
		(unsigned short)new_texnum.v.data, flags.v.data);

	return ret_val.int_val;
}

int C_MoveSectorBSP(int object_id, local_var_type *local_vars,
	int num_normal_parms, parm_node normal_parm_array[],
	int num_name_parms, parm_node name_parm_array[])
{
	val_type ret_val, room_val, server_id, animation, height, speed;
	room_node *r;

	ret_val.v.tag = TAG_INT;
	ret_val.v.data = false;

	room_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
		normal_parm_array[0].value);
	server_id = RetrieveValue(object_id, local_vars, normal_parm_array[1].type,
		normal_parm_array[1].value);
	animation = RetrieveValue(object_id, local_vars, normal_parm_array[2].type,
		normal_parm_array[2].value);
	height = RetrieveValue(object_id, local_vars, normal_parm_array[3].type,
		normal_parm_array[3].value);
	speed = RetrieveValue(object_id, local_vars, normal_parm_array[4].type,
		normal_parm_array[4].value);

	if (room_val.v.tag != TAG_ROOM_DATA)
	{
		bprintf("C_MoveSectorBSP can't use non room %i,%i\n",
			room_val.v.tag, room_val.v.data);
		return ret_val.int_val;
	}

	if (server_id.v.tag != TAG_INT)
	{
		bprintf("C_MoveSectorBSP serverid can't use non int %i,%i\n",
			server_id.v.tag, server_id.v.data);
		return ret_val.int_val;
	}

	if (animation.v.tag != TAG_INT)
	{
		bprintf("C_MoveSectorBSP animation can't use non int %i,%i\n",
			animation.v.tag, animation.v.data);
		return ret_val.int_val;
	}

	if (height.v.tag != TAG_INT)
	{
		bprintf("C_MoveSectorBSP height can't use non int %i,%i\n",
			height.v.tag, height.v.data);
		return ret_val.int_val;
	}

	if (speed.v.tag != TAG_INT)
	{
		bprintf("C_MoveSectorBSP speed can't use non int %i,%i\n",
			speed.v.tag, speed.v.data);
		return ret_val.int_val;
	}

	r = GetRoomDataByID(room_val.v.data);
	if (r == NULL)
	{
		bprintf("C_MoveSectorBSP can't find room %i\n", room_val.v.data);
		return ret_val.int_val;
	}

	bool is_floor = (animation.v.data == ANIMATE_FLOOR_LIFT);
	float fheight = FINENESSKODTOROO((float)height.v.data);
	float fspeed = 0.0f; // todo, but always instant anyways atm

	BSPMoveSector(&r->data, (unsigned int)server_id.v.data, is_floor, fheight, fspeed);

	return ret_val.int_val;
}

int C_BlockerAddBSP(int object_id, local_var_type *local_vars,
	int num_normal_parms, parm_node normal_parm_array[],
	int num_name_parms, parm_node name_parm_array[])
{
	val_type ret_val, room_val, obj_val;
	val_type row, col, finerow, finecol;
	room_node *r;

	ret_val.v.tag = TAG_INT;
	ret_val.v.data = false;

	room_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
		normal_parm_array[0].value);
	obj_val = RetrieveValue(object_id, local_vars, normal_parm_array[1].type,
		normal_parm_array[1].value);
	row = RetrieveValue(object_id, local_vars, normal_parm_array[2].type,
		normal_parm_array[2].value);
	col = RetrieveValue(object_id, local_vars, normal_parm_array[3].type,
		normal_parm_array[3].value);
	finerow = RetrieveValue(object_id, local_vars, normal_parm_array[4].type,
		normal_parm_array[4].value);
	finecol = RetrieveValue(object_id, local_vars, normal_parm_array[5].type,
		normal_parm_array[5].value);

	if (room_val.v.tag != TAG_ROOM_DATA)
	{
		bprintf("C_BlockerAddBSP can't use non room %i,%i\n",
			room_val.v.tag, room_val.v.data);
		return ret_val.int_val;
	}

	if (obj_val.v.tag != TAG_OBJECT)
	{
		bprintf("C_BlockerAddBSP can't use non obj %i,%i\n",
			obj_val.v.tag, obj_val.v.data);
		return ret_val.int_val;
	}

	if (row.v.tag != TAG_INT)
	{
		bprintf("C_BlockerAddBSP row can't use non int %i,%i\n",
			row.v.tag, row.v.data);
		return ret_val.int_val;
	}

	if (col.v.tag != TAG_INT)
	{
		bprintf("C_BlockerAddBSP col can't use non int %i,%i\n",
			col.v.tag, col.v.data);
		return ret_val.int_val;
	}

	if (finerow.v.tag != TAG_INT)
	{
		bprintf("C_BlockerAddBSP finerow can't use non int %i,%i\n",
			finerow.v.tag, finerow.v.data);
		return ret_val.int_val;
	}

	if (finecol.v.tag != TAG_INT)
	{
		bprintf("C_BlockerAddBSP finecol can't use non int %i,%i\n",
			finecol.v.tag, finecol.v.data);
		return ret_val.int_val;
	}

	r = GetRoomDataByID(room_val.v.data);
	if (r == NULL)
	{
		bprintf("C_BlockerAddBSP can't find room %i\n", room_val.v.data);
		return ret_val.int_val;
	}

	V2 p;
	p.X = GRIDCOORDTOROO(col.v.data, finecol.v.data);
	p.Y = GRIDCOORDTOROO(row.v.data, finerow.v.data);

	// query
	ret_val.v.data = BSPBlockerAdd(&r->data, obj_val.v.data, &p);

	return ret_val.int_val;
}

int C_BlockerMoveBSP(int object_id, local_var_type *local_vars,
	int num_normal_parms, parm_node normal_parm_array[],
	int num_name_parms, parm_node name_parm_array[])
{
	val_type ret_val, room_val, obj_val;
	val_type row, col, finerow, finecol;
	room_node *r;

	ret_val.v.tag = TAG_INT;
	ret_val.v.data = false;

	room_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
		normal_parm_array[0].value);
	obj_val = RetrieveValue(object_id, local_vars, normal_parm_array[1].type,
		normal_parm_array[1].value);
	row = RetrieveValue(object_id, local_vars, normal_parm_array[2].type,
		normal_parm_array[2].value);
	col = RetrieveValue(object_id, local_vars, normal_parm_array[3].type,
		normal_parm_array[3].value);
	finerow = RetrieveValue(object_id, local_vars, normal_parm_array[4].type,
		normal_parm_array[4].value);
	finecol = RetrieveValue(object_id, local_vars, normal_parm_array[5].type,
		normal_parm_array[5].value);

	if (room_val.v.tag != TAG_ROOM_DATA)
	{
		bprintf("C_BlockerMoveBSP can't use non room %i,%i\n",
			room_val.v.tag, room_val.v.data);
		return ret_val.int_val;
	}

	if (obj_val.v.tag != TAG_OBJECT)
	{
		bprintf("C_BlockerMoveBSP can't use non obj %i,%i\n",
			obj_val.v.tag, obj_val.v.data);
		return ret_val.int_val;
	}

	if (row.v.tag != TAG_INT)
	{
		bprintf("C_BlockerMoveBSP row can't use non int %i,%i\n",
			row.v.tag, row.v.data);
		return ret_val.int_val;
	}

	if (col.v.tag != TAG_INT)
	{
		bprintf("C_BlockerMoveBSP col can't use non int %i,%i\n",
			col.v.tag, col.v.data);
		return ret_val.int_val;
	}

	if (finerow.v.tag != TAG_INT)
	{
		bprintf("C_BlockerMoveBSP finerow can't use non int %i,%i\n",
			finerow.v.tag, finerow.v.data);
		return ret_val.int_val;
	}

	if (finecol.v.tag != TAG_INT)
	{
		bprintf("C_BlockerMoveBSP finecol can't use non int %i,%i\n",
			finecol.v.tag, finecol.v.data);
		return ret_val.int_val;
	}

	r = GetRoomDataByID(room_val.v.data);
	if (r == NULL)
	{
		bprintf("C_BlockerMoveBSP can't find room %i\n", room_val.v.data);
		return ret_val.int_val;
	}

	V2 p;
	p.X = GRIDCOORDTOROO(col.v.data, finecol.v.data);
	p.Y = GRIDCOORDTOROO(row.v.data, finerow.v.data);

	// query
	ret_val.v.data = BSPBlockerMove(&r->data, obj_val.v.data, &p);

	return ret_val.int_val;
}

int C_BlockerRemoveBSP(int object_id, local_var_type *local_vars,
	int num_normal_parms, parm_node normal_parm_array[],
	int num_name_parms, parm_node name_parm_array[])
{
	val_type ret_val, room_val, obj_val;
	room_node *r;

	ret_val.v.tag = TAG_INT;
	ret_val.v.data = false;

	room_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
		normal_parm_array[0].value);
	obj_val = RetrieveValue(object_id, local_vars, normal_parm_array[1].type,
		normal_parm_array[1].value);
	
	if (room_val.v.tag != TAG_ROOM_DATA)
	{
		bprintf("C_BlockerRemoveBSP can't use non room %i,%i\n",
			room_val.v.tag, room_val.v.data);
		return ret_val.int_val;
	}

	if (obj_val.v.tag != TAG_OBJECT)
	{
		bprintf("C_BlockerRemoveBSP can't use non obj %i,%i\n",
			obj_val.v.tag, obj_val.v.data);
		return ret_val.int_val;
	}

	r = GetRoomDataByID(room_val.v.data);
	if (r == NULL)
	{
		bprintf("C_BlockerRemoveBSP can't find room %i\n", room_val.v.data);
		return ret_val.int_val;
	}

	// query
	ret_val.v.data = BSPBlockerRemove(&r->data, obj_val.v.data);

	return ret_val.int_val;
}

int C_BlockerClearBSP(int object_id, local_var_type *local_vars,
	int num_normal_parms, parm_node normal_parm_array[],
	int num_name_parms, parm_node name_parm_array[])
{
	val_type ret_val, room_val;
	room_node *r;

	ret_val.v.tag = TAG_INT;
	ret_val.v.data = true;

	room_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
		normal_parm_array[0].value);
	
	if (room_val.v.tag != TAG_ROOM_DATA)
	{
		bprintf("C_BlockerClearBSP can't use non room %i,%i\n",
			room_val.v.tag, room_val.v.data);
		return ret_val.int_val;
	}

	r = GetRoomDataByID(room_val.v.data);
	if (r == NULL)
	{
		bprintf("C_BlockerClearBSP can't find room %i\n", room_val.v.data);
		return ret_val.int_val;
	}

	// query
	BSPBlockerClear(&r->data);

	return ret_val.int_val;
}

int C_GetRandomPointBSP(int object_id, local_var_type *local_vars,
	int num_normal_parms, parm_node normal_parm_array[],
	int num_name_parms, parm_node name_parm_array[])
{
	val_type ret_val, room_val, maxattempts_val;
	val_type row, col, finerow, finecol;
	room_node *r;

	// in case it fails
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = false;

	room_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
		normal_parm_array[0].value);

	maxattempts_val = RetrieveValue(object_id, local_vars, normal_parm_array[1].type,
		normal_parm_array[1].value);

	row = RetrieveValue(object_id, local_vars, normal_parm_array[2].type,
		normal_parm_array[2].value);
	col = RetrieveValue(object_id, local_vars, normal_parm_array[3].type,
		normal_parm_array[3].value);
	finerow = RetrieveValue(object_id, local_vars, normal_parm_array[4].type,
		normal_parm_array[4].value);
	finecol = RetrieveValue(object_id, local_vars, normal_parm_array[5].type,
		normal_parm_array[5].value);

	if (room_val.v.tag != TAG_ROOM_DATA)
	{
		bprintf("C_GetRandomPointBSP can't use non room %i,%i\n",
			room_val.v.tag, room_val.v.data);
		return ret_val.int_val;
	}

	if (maxattempts_val.v.tag != TAG_INT)
	{
		bprintf("C_GetRandomPointBSP maxattempts can't use non int %i,%i\n",
			maxattempts_val.v.tag, maxattempts_val.v.data);
		return ret_val.int_val;
	}

	if (row.v.tag != TAG_INT)
	{
		bprintf("C_GetRandomPointBSP row source can't use non int %i,%i\n",
			row.v.tag, row.v.data);
		return ret_val.int_val;
	}

	if (col.v.tag != TAG_INT)
	{
		bprintf("C_GetRandomPointBSP col source can't use non int %i,%i\n",
			col.v.tag, col.v.data);
		return ret_val.int_val;
	}

	if (finerow.v.tag != TAG_INT)
	{
		bprintf("C_GetRandomPointBSP finerow source can't use non int %i,%i\n",
			finerow.v.tag, finerow.v.data);
		return ret_val.int_val;
	}

	if (finecol.v.tag != TAG_INT)
	{
		bprintf("C_GetRandomPointBSP finecol source can't use non int %i,%i\n",
			finecol.v.tag, finecol.v.data);
		return ret_val.int_val;
	}

	r = GetRoomDataByID(room_val.v.data);
	if (r == NULL)
	{
		bprintf("C_GetRandomPointBSP can't find room %i\n", room_val.v.data);
		return ret_val.int_val;
	}

	V2 p;
	bool ok = BSPGetRandomPoint(&r->data, maxattempts_val.v.data, &p);

	if (ok)
	{
		// set output vars
		local_vars->locals[finecol.v.data].v.tag = TAG_INT;
		local_vars->locals[finecol.v.data].v.data = ROOCOORDTOGRIDFINE(p.X);

		local_vars->locals[finerow.v.data].v.tag = TAG_INT;
		local_vars->locals[finerow.v.data].v.data = ROOCOORDTOGRIDFINE(p.Y);

		local_vars->locals[col.v.data].v.tag = TAG_INT;
		local_vars->locals[col.v.data].v.data = ROOCOORDTOGRIDBIG(p.X);

		local_vars->locals[row.v.data].v.tag = TAG_INT;
		local_vars->locals[row.v.data].v.data = ROOCOORDTOGRIDBIG(p.Y);

		// mark succeeded
		ret_val.v.data = true;	
	}

	return ret_val.int_val;
}

int C_GetStepTowardsBSP(int object_id, local_var_type *local_vars,
	int num_normal_parms, parm_node normal_parm_array[],
	int num_name_parms, parm_node name_parm_array[])
{
	val_type ret_val, room_val;
	val_type row_source, col_source, finerow_source, finecol_source;
	val_type row_dest, col_dest, finerow_dest, finecol_dest;
	val_type state_flags, objectid;
	room_node *r;

	// in case it fails
	ret_val.int_val = NIL;

	room_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
		normal_parm_array[0].value);
	row_source = RetrieveValue(object_id, local_vars, normal_parm_array[1].type,
		normal_parm_array[1].value);
	col_source = RetrieveValue(object_id, local_vars, normal_parm_array[2].type,
		normal_parm_array[2].value);
	finerow_source = RetrieveValue(object_id, local_vars, normal_parm_array[3].type,
		normal_parm_array[3].value);
	finecol_source = RetrieveValue(object_id, local_vars, normal_parm_array[4].type,
		normal_parm_array[4].value);

	row_dest = RetrieveValue(object_id, local_vars, normal_parm_array[5].type,
		normal_parm_array[5].value);
	col_dest = RetrieveValue(object_id, local_vars, normal_parm_array[6].type,
		normal_parm_array[6].value);
	finerow_dest = RetrieveValue(object_id, local_vars, normal_parm_array[7].type,
		normal_parm_array[7].value);
	finecol_dest = RetrieveValue(object_id, local_vars, normal_parm_array[8].type,
		normal_parm_array[8].value);

	state_flags = RetrieveValue(object_id, local_vars, normal_parm_array[9].type,
		normal_parm_array[9].value);
	objectid = RetrieveValue(object_id, local_vars, normal_parm_array[10].type,
		normal_parm_array[10].value);

	if (room_val.v.tag != TAG_ROOM_DATA)
	{
		bprintf("C_GetStepTowardsBSP can't use non room %i,%i\n",
			room_val.v.tag, room_val.v.data);
		return ret_val.int_val;
	}

	if (row_source.v.tag != TAG_INT)
	{
		bprintf("C_GetStepTowardsBSP row source can't use non int %i,%i\n",
			row_source.v.tag, row_source.v.data);
		return ret_val.int_val;
	}

	if (col_source.v.tag != TAG_INT)
	{
		bprintf("C_GetStepTowardsBSP col source can't use non int %i,%i\n",
			col_source.v.tag, col_source.v.data);
		return ret_val.int_val;
	}

	if (finerow_source.v.tag != TAG_INT)
	{
		bprintf("C_GetStepTowardsBSP finerow source can't use non int %i,%i\n",
			finerow_source.v.tag, finerow_source.v.data);
		return ret_val.int_val;
	}

	if (finecol_source.v.tag != TAG_INT)
	{
		bprintf("C_GetStepTowardsBSP finecol source can't use non int %i,%i\n",
			finecol_source.v.tag, finecol_source.v.data);
		return ret_val.int_val;
	}

	if (row_dest.v.tag != TAG_INT)
	{
		bprintf("C_GetStepTowardsBSP row dest can't use non int %i,%i\n",
			row_dest.v.tag, row_dest.v.data);
		return ret_val.int_val;
	}

	if (col_dest.v.tag != TAG_INT)
	{
		bprintf("C_GetStepTowardsBSP col dest can't use non int %i,%i\n",
			col_dest.v.tag, col_dest.v.data);
		return ret_val.int_val;
	}

	if (finerow_dest.v.tag != TAG_INT)
	{
		bprintf("C_GetStepTowardsBSP finerow dest can't use non int %i,%i\n",
			finerow_dest.v.tag, finerow_dest.v.data);
		return ret_val.int_val;
	}

	if (finecol_dest.v.tag != TAG_INT)
	{
		bprintf("C_GetStepTowardsBSP finecol dest can't use non int %i,%i\n",
			finecol_dest.v.tag, finecol_dest.v.data);
		return ret_val.int_val;
	}

	if (state_flags.v.tag != TAG_INT)
	{
		bprintf("C_GetStepTowardsBSP state_flags can't use non int %i,%i\n",
			state_flags.v.tag, state_flags.v.data);
		return ret_val.int_val;
	}

	if (objectid.v.tag != TAG_OBJECT)
	{
		bprintf("C_GetStepTowardsBSP objectid can't use non obj %i,%i\n",
			objectid.v.tag, objectid.v.data);
		return ret_val.int_val;
	}

	r = GetRoomDataByID(room_val.v.data);
	if (r == NULL)
	{
		bprintf("C_GetStepTowardsBSP can't find room %i\n", room_val.v.data);
		return ret_val.int_val;
	}

	V2 s;
	s.X = GRIDCOORDTOROO(col_source.v.data, finecol_source.v.data);
	s.Y = GRIDCOORDTOROO(row_source.v.data, finerow_source.v.data);

	V2 e;
   e.X = GRIDCOORDTOROO(local_vars->locals[col_dest.v.data].v.data, local_vars->locals[finecol_dest.v.data].v.data);
   e.Y = GRIDCOORDTOROO(local_vars->locals[row_dest.v.data].v.data, local_vars->locals[finerow_dest.v.data].v.data);

	V2 p;
	unsigned int flags = (unsigned int)state_flags.v.data;
	bool ok = BSPGetStepTowards(&r->data, &s, &e, &p, &flags, objectid.v.data);

	if (ok)
	{
		ret_val.v.tag = TAG_INT;
		ret_val.v.data = flags;

		local_vars->locals[finecol_dest.v.data].v.tag = TAG_INT;
		local_vars->locals[finecol_dest.v.data].v.data = ROOCOORDTOGRIDFINE(p.X);

		local_vars->locals[finerow_dest.v.data].v.tag = TAG_INT;
		local_vars->locals[finerow_dest.v.data].v.data = ROOCOORDTOGRIDFINE(p.Y);

		local_vars->locals[col_dest.v.data].v.tag = TAG_INT;
		local_vars->locals[col_dest.v.data].v.data = ROOCOORDTOGRIDBIG(p.X);

		local_vars->locals[row_dest.v.data].v.tag = TAG_INT;
		local_vars->locals[row_dest.v.data].v.data = ROOCOORDTOGRIDBIG(p.Y);
	}

	return ret_val.int_val;
}

/*
 * C_AppendListElem: takes a list and an item to be appended to the list,
 *    appends the item to the end of the list. Returns the original list
 *    with the item appended to the end.
 */
int C_AppendListElem(int object_id,local_var_type *local_vars,
         int num_normal_parms,parm_node normal_parm_array[],
         int num_name_parms,parm_node name_parm_array[])
{
   val_type source_val, list_val, ret_val;
   
   source_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
      normal_parm_array[0].value);
   list_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
      normal_parm_array[1].value);

   if (list_val.v.tag != TAG_LIST)
   {
      if (list_val.v.tag != TAG_NIL)
      {
         bprintf("C_AppendListElem object %i can't add to non-list %i,%i\n",
            object_id,list_val.v.tag,list_val.v.data);
         return list_val.int_val;
      }
   }

   ret_val.v.tag = TAG_LIST;
   ret_val.v.data = AppendListElem(source_val,list_val);
   return ret_val.int_val;
}

int C_Cons(int object_id,local_var_type *local_vars,
         int num_normal_parms,parm_node normal_parm_array[],
         int num_name_parms,parm_node name_parm_array[])
{
   val_type source_val,dest_val,ret_val;

   source_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
      normal_parm_array[0].value);
   dest_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
      normal_parm_array[1].value);

   if (dest_val.v.tag != TAG_LIST)
   {
      if (dest_val.v.tag != TAG_NIL)
      {
         bprintf("C_Cons object %i can't add to non-list %i,%i\n",
            object_id,dest_val.v.tag,dest_val.v.data);
         return dest_val.int_val;
      }
   }

   ret_val.v.tag = TAG_LIST;
   ret_val.v.data = Cons(source_val,dest_val);
   return ret_val.int_val;
}

int C_First(int object_id,local_var_type *local_vars,
			int num_normal_parms,parm_node normal_parm_array[],
			int num_name_parms,parm_node name_parm_array[])
{
	val_type list_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_First object %i can't take First of a non-list %i,%i\n",
			object_id,list_val.v.tag,list_val.v.data);
		return NIL;
	}
	if (!IsListNodeByID(list_val.v.data))
	{
		bprintf("C_First object %i can't take First of an invalid list %i,%i\n",
			object_id,list_val.v.tag,list_val.v.data);
		return NIL;
	}
	return First(list_val.v.data);
}

int C_Rest(int object_id,local_var_type *local_vars,
		   int num_normal_parms,parm_node normal_parm_array[],
		   int num_name_parms,parm_node name_parm_array[])
{
	val_type list_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_Rest object %i can't take Rest of a non-list %i,%i\n",
			object_id,list_val.v.tag,list_val.v.data);
		return NIL;
	}
	if (!IsListNodeByID(list_val.v.data))
	{
		bprintf("C_Rest object %i can't take Rest of an invalid list %i,%i\n",
			object_id,list_val.v.tag,list_val.v.data);
		return NIL;
	}
	return Rest(list_val.v.data);
}

int C_Length(int object_id,local_var_type *local_vars,
			 int num_normal_parms,parm_node normal_parm_array[],
			 int num_name_parms,parm_node name_parm_array[])
{
	val_type list_val,ret_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	
	if (list_val.v.tag == TAG_NIL)
	{
		ret_val.v.tag = TAG_INT;
		ret_val.v.data = 0;
		return ret_val.int_val;
	}
	
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_Length object %i can't take Length of a non-list %i,%i\n",
			object_id,list_val.v.tag,list_val.v.data);
		return NIL;
	}
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = Length(list_val.v.data);
	return ret_val.int_val;
}

int C_Last(int object_id,local_var_type *local_vars,
         int num_normal_parms,parm_node normal_parm_array[],
         int num_name_parms,parm_node name_parm_array[])
{
   val_type list_val;

   list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
                            normal_parm_array[0].value);

   if (list_val.v.tag != TAG_LIST)
   {
      bprintf("C_Last object %i can't get last element of a non-list %i,%i\n",
         object_id,list_val.v.tag,list_val.v.data);
      return NIL;
   }

   return Last(list_val.v.data);
}

int C_Nth(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[])
{
	val_type n_val,list_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_Nth object %i can't take Nth of a non-list %i,%i\n",
			object_id,list_val.v.tag,list_val.v.data);
		return NIL;
	}
	if (!IsListNodeByID(list_val.v.data))
	{
		bprintf("C_Nth object %i can't take Nth of an invalid list %i,%i\n",
			object_id,list_val.v.tag,list_val.v.data);
		return NIL;
	}
	
	n_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	if (n_val.v.tag != TAG_INT)
	{
		bprintf("C_Nth can't take Nth with n = non-int %i,%i\n",
			n_val.v.tag,n_val.v.data);
		return NIL;
	}
	
	return Nth(n_val.v.data,list_val.v.data);
}

/*
 * C_IsListMatch:  takes two lists, checks the values of each element
 *    in the lists and returns TRUE if the elements are identical, FALSE
 *    otherwise. Elements are identical if they have the same int_val
 *    (tag AND data type) except in the case of TAG_LIST, in which case
 *    the list contents must be identical but the list node number does not
 *    need to be.
 */
int C_IsListMatch(int object_id,local_var_type *local_vars,
            int num_normal_parms,parm_node normal_parm_array[],
            int num_name_parms,parm_node name_parm_array[])
{
   val_type ret_val, list_one, list_two;
   ret_val.v.tag = TAG_INT;
   ret_val.v.data = False;

   list_one = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
      normal_parm_array[0].value);
   if (list_one.v.tag != TAG_LIST)
   {
      bprintf("C_IsListMatch object %i can't check non-list one %i,%i\n",
         object_id, list_one.v.tag, list_one.v.data);
      return ret_val.int_val;
   }

   list_two = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
      normal_parm_array[1].value);
   if (list_two.v.tag != TAG_LIST)
   {
      bprintf("C_IsListMatch object %i can't check non-list two %i,%i\n",
         object_id, list_two.v.tag, list_two.v.data);
      return ret_val.int_val;
   }

   ret_val.v.data = IsListMatch(list_one.v.data, list_two.v.data);
   return ret_val.int_val;
}

int C_List(int object_id,local_var_type *local_vars,
		   int num_normal_parms,parm_node normal_parm_array[],
		   int num_name_parms,parm_node name_parm_array[])
{
	val_type temp,ret_val;
	int i;
	
	if (num_normal_parms == 0)
		return NIL;
	
	ret_val.int_val = NIL;
	for (i=num_normal_parms-1;i>=0;i--)
	{
		extern list_node *list_nodes;
		temp = RetrieveValue(object_id,local_vars,normal_parm_array[i].type,
			normal_parm_array[i].value);
		ret_val.v.data = Cons(temp,ret_val);
		ret_val.v.tag = TAG_LIST; /* do this AFTER the cons call or DIE */
	}
	return ret_val.int_val;
}

int C_IsList(int object_id,local_var_type *local_vars,
			 int num_normal_parms,parm_node normal_parm_array[],
			 int num_name_parms,parm_node name_parm_array[])
{
	val_type var_check,ret_val;
	
	var_check = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
			     normal_parm_array[0].value);
	
	ret_val.v.tag = TAG_INT;
	if (var_check.v.tag == TAG_LIST || var_check.v.tag == TAG_NIL)
		ret_val.v.data = True;
	else
		ret_val.v.data = False;
	
	return ret_val.int_val;
}

int C_SetFirst(int object_id,local_var_type *local_vars,
			   int num_normal_parms,parm_node normal_parm_array[],
			   int num_name_parms,parm_node name_parm_array[])
{
	val_type list_val,set_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_SetFirst object %i can't set elem of non-list %i,%i\n",
			object_id,list_val.v.tag,list_val.v.data);
		return NIL;
	}
	
	set_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	
	return SetFirst(list_val.v.data,set_val);
}

int C_SetNth(int object_id,local_var_type *local_vars,
			 int num_normal_parms,parm_node normal_parm_array[],
			 int num_name_parms,parm_node name_parm_array[])
{
	val_type n_val,list_val,set_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_SetFirst object %i can't set elem of non-list %i,%i\n",
			object_id,list_val.v.tag,list_val.v.data);
		return NIL;
	}
	n_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	if (n_val.v.tag != TAG_INT)
	{
		bprintf("C_SetNth object %i can't take Nth with n = non-int %i,%i\n",
			object_id,n_val.v.tag,n_val.v.data);
		return NIL;
	}
	
	set_val = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
		normal_parm_array[2].value);
	
	return SetNth(n_val.v.data,list_val.v.data,set_val);
}

/*
 * C_InsertListElem:  takes a list, a list position and one piece of data, adds
 *    the data at the given position. If the list position is larger than the
 *    list, it is added to the end. If list position 0 is sent, just returns
 *    the initial list, otherwise returns the altered list.
 */
int C_InsertListElem(int object_id,local_var_type *local_vars,
         int num_normal_parms,parm_node normal_parm_array[],
         int num_name_parms,parm_node name_parm_array[])
{
   val_type n_val,list_val,set_val, ret_val;

   list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
      normal_parm_array[0].value);
   if (list_val.v.tag != TAG_LIST)
   {
      if (list_val.v.tag != TAG_NIL)
      {
         bprintf("C_InsertListElem object %i can't add elem to non-list %i,%i\n",
            object_id,list_val.v.tag,list_val.v.data);
         return list_val.int_val;
      }
   }
   n_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
      normal_parm_array[1].value);
   if (n_val.v.tag != TAG_INT)
   {
      bprintf("C_InsertListElem object %i can't add elem with n = non-int %i, %i, returning list.\n",
         object_id,n_val.v.tag,n_val.v.data);
      return list_val.int_val;
   }

   set_val = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
      normal_parm_array[2].value);

   ret_val.v.tag = TAG_LIST;

   // Handle the case where the new element should be in the first position.
   // Should have called Cons to do this. Cons also adds to $ lists.
   if (n_val.v.data == 1 || list_val.v.tag == TAG_NIL)
      ret_val.v.data = Cons(set_val,list_val);
   else
      ret_val.v.data = InsertListElem(n_val.v.data,list_val.v.data,set_val);

   return ret_val.int_val;
}

/*
 * C_SwapListElem: takes a list and two integers representing elements
 *                 in the list, swaps the data in the two elements.
 *                 Returns NIL.
 */
int C_SwapListElem(int object_id,local_var_type *local_vars,
         int num_normal_parms,parm_node normal_parm_array[],
         int num_name_parms,parm_node name_parm_array[])
{
   val_type list_val, n_val, m_val;

   list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
      normal_parm_array[0].value);
   if (list_val.v.tag != TAG_LIST)
   {
      bprintf("C_SwapListElem object %i can't set elem of non-list %i,%i\n",
         object_id,list_val.v.tag,list_val.v.data);
      return NIL;
   }

   n_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
      normal_parm_array[1].value);
   if (n_val.v.tag != TAG_INT)
   {
      bprintf("C_SwapListElem object %i can't take Nth with n = non-int %i,%i\n",
         object_id,n_val.v.tag,n_val.v.data);
      return NIL;
   }

   m_val = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
      normal_parm_array[2].value);
   if (m_val.v.tag != TAG_INT)
   {
      bprintf("C_SwapListElem object %i can't take Nth with n = non-int %i,%i\n",
         object_id,m_val.v.tag,m_val.v.data);
      return NIL;
   }

   if (n_val.v.data == 0 || m_val.v.data == 0)
   {
      bprintf("C_SwapListElem object %i given invalid list element, elements are %i,%i\n",
         object_id,n_val.v.data,m_val.v.data);
      return NIL;
   }

   return SwapListElem(list_val.v.data,n_val.v.data,m_val.v.data);
}

int C_DelListElem(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type list_val,list_elem,ret_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_DelListElem object %i can't delete elem from non-list %i,%i\n",
			object_id,list_val.v.tag,list_val.v.data);
		return NIL;
	}
	
	list_elem = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
			     normal_parm_array[1].value);
	
	ret_val.int_val = DelListElem(list_val,list_elem);
	
	return ret_val.int_val;
}

int C_FindListElem(int object_id,local_var_type *local_vars,
				   int num_normal_parms,parm_node normal_parm_array[],
				   int num_name_parms,parm_node name_parm_array[])
{
	val_type list_val,list_elem,ret_val;
	
	list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	
	if (list_val.v.tag == TAG_NIL)
	{
		return NIL;
	}
	
	if (list_val.v.tag != TAG_LIST)
	{
		bprintf("C_FindListElem object %i can't find elem in non-list %i,%i\n",
			object_id,list_val.v.tag,list_val.v.data);
		return NIL;
	}
	
	list_elem = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
			     normal_parm_array[1].value);
	
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = FindListElem(list_val,list_elem);
	if (NIL == ret_val.int_val)
	{
		return NIL;
	}
	
	return ret_val.int_val;
}

/*
 * C_GetAllListNodesByClass: takes a list, a position and a class. Checks each
 *                sub-list in the parent list and if the class is present at
 *                the position, adds it to a new list. Returns the new list or
 *                NIL.
 */
int C_GetAllListNodesByClass(int object_id,local_var_type *local_vars,
         int num_normal_parms,parm_node normal_parm_array[],
         int num_name_parms,parm_node name_parm_array[])
{
   val_type list_val, class_val, pos_val;

   list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
                  normal_parm_array[0].value);

   if (list_val.v.tag != TAG_LIST)
   {
      if (list_val.v.tag == TAG_NIL)
         return NIL;
      bprintf("C_GetAllListNodesByClass object %i can't find elem in non-list %i,%i\n",
         object_id, list_val.v.tag, list_val.v.data);
      return NIL;
   }

   pos_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
               normal_parm_array[1].value);
   if (pos_val.v.tag != TAG_INT || pos_val.v.data < 1)
   {
      bprintf("C_GetAllListNodesByClass object %i can't use non-int position %i,%i\n",
         object_id, pos_val.v.tag, pos_val.v.data);
      return NIL;
   }

   class_val = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
                  normal_parm_array[2].value);

   if (class_val.v.tag != TAG_CLASS)
   {
      bprintf("C_GetAllListNodesByClass object %i can't use non-class %i,%i\n",
         object_id, class_val.v.tag, class_val.v.data);
      return NIL;
   }

   return GetAllListNodesByClass(list_val.v.data, pos_val.v.data, class_val.v.data);
}

/*
 * C_GetListNode: takes a list, a position and an object. Checks each
 *                sub-list in the parent list and returns the first list node
 *                containing the object at that position. Returns NIL if the
 *                object wasn't found in any sub-lists.
 */
int C_GetListNode(int object_id,local_var_type *local_vars,
         int num_normal_parms,parm_node normal_parm_array[],
         int num_name_parms,parm_node name_parm_array[])
{
   val_type list_val, list_elem, pos_val;

   list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
                  normal_parm_array[0].value);

   if (list_val.v.tag != TAG_LIST)
   {
      if (list_val.v.tag == TAG_NIL)
         return NIL;
      bprintf("C_GetListNode object %i can't find elem in non-list %i,%i\n",
         object_id, list_val.v.tag, list_val.v.data);
      return NIL;
   }

   pos_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
               normal_parm_array[1].value);
   if (pos_val.v.tag != TAG_INT || pos_val.v.data < 1)
   {
      bprintf("C_GetListNode object %i can't use non-int position %i,%i\n",
         object_id, pos_val.v.tag, pos_val.v.data);
      return NIL;
   }

   list_elem = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
                  normal_parm_array[2].value);

   if (list_elem.v.tag == TAG_NIL)
   {
      bprintf("C_GetListNode object %i can't find $ in list %i,%i\n",
         object_id, list_val.v.tag, list_val.v.data);
      return NIL;
   }

   return GetListNode(list_val, pos_val.v.data, list_elem);
}

/*
 * C_GetListElemByClass: takes a list and a class, returns the element of the
 *                        list with that class if found, NIL otherwise.
 */
int C_GetListElemByClass(int object_id,local_var_type *local_vars,
         int num_normal_parms,parm_node normal_parm_array[],
         int num_name_parms,parm_node name_parm_array[])
{
   val_type list_val, class_val;

   list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
                  normal_parm_array[0].value);

   if (list_val.v.tag != TAG_LIST)
   {
      if (list_val.v.tag == TAG_NIL)
         return NIL;
      bprintf("C_GetListElemByClass object %i can't get elem in non-list %i,%i\n",
         object_id, list_val.v.tag, list_val.v.data);
      return NIL;
   }

   class_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
                  normal_parm_array[1].value);
   if (class_val.v.tag != TAG_CLASS)
   {
      bprintf("C_GetListElemByClass object %i can't get non-class %i,%i\n",
         object_id, class_val.v.tag, class_val.v.data);
      return NIL;
   }

   return GetListElemByClass(list_val, class_val.v.data);
}

/*
 * C_ListCopy: takes a list, makes a copy and returns the copy.
 */
int C_ListCopy(int object_id,local_var_type *local_vars,
         int num_normal_parms,parm_node normal_parm_array[],
         int num_name_parms,parm_node name_parm_array[])
{
   val_type list_val, ret_val;

   list_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
                  normal_parm_array[0].value);

   if (list_val.v.tag != TAG_LIST)
   {
      if (list_val.v.tag == TAG_NIL)
         return NIL;
      bprintf("C_ListCopy object %i can't copy non-list %i,%i\n",
         object_id, list_val.v.tag, list_val.v.data);
      return NIL;
   }

   ret_val.v.data = ListCopy(list_val.v.data);
   ret_val.v.tag = TAG_LIST;

   return ret_val.int_val;
}

/*
 * C_GetTimeZoneOffset: returns the amount of seconds that must be added
 *                      to local time to equal UTC. Conversely, subtracting
 *                      this number from UTC (GetTime()) equals local time.
 */
int C_GetTimeZoneOffset(int object_id,local_var_type *local_vars,
            int num_normal_parms,parm_node normal_parm_array[],
            int num_name_parms,parm_node name_parm_array[])
{
   val_type ret_val;

   ret_val.v.tag = TAG_INT;
   ret_val.v.data = GetTimeZoneOffset();

   return ret_val.int_val;
}

int C_GetTime(int object_id,local_var_type *local_vars,
			  int num_normal_parms,parm_node normal_parm_array[],
			  int num_name_parms,parm_node name_parm_array[])
{
	val_type ret_val;
	
	ret_val.v.tag = TAG_INT;

    /*  We must subtract a number from the system time due to size
        limitations within the blakod.  Blakod uses 32 bit values,
        -4 bits for type and -1 bit for sign.  This leaves us with
        27 bits for value,  This only allows us to have 134M or so
        as a positive value.  Current system time is a bit larger
        than that.  So, we subtract off time to compensate.
    */

	ret_val.v.data = GetTime() - 1388534400L;    // Offset to January 2014
	
	return ret_val.int_val;
}

int C_GetTickCount(int object_id,local_var_type *local_vars,
			  int num_normal_parms,parm_node normal_parm_array[],
			  int num_name_parms,parm_node name_parm_array[])
{
	val_type ret_val;
	
	// GetMilliCount is from blakerv/time.c. 
	// Its return is in ms and with a precision of 1ms.
	// It also provides Windows & Linux implementations.
	UINT64 tick = GetMilliCount();
	
	// but tick is unsigned 64-bit integer
	// and blakserv integers are signed with only 28-bits
	// the high-bit is the sign at bit-index 27/31
	// recapitulate:
	// 0x00000000 = 0000 0000 0000 0000 0000 0000 0000 = 0
	// 0x07FFFFFF = 0111 1111 1111 1111 1111 1111 1111 = 134217727
	// 0x08000000 = 1000 0000 0000 0000 0000 0000 0000 = -134217728
	// 0x0FFFFFFF = 1111 1111 1111 1111 1111 1111 1111 = -1
	
	// convert:
	// 1) We grab the low 32-bits by casting to unsigned int (so next & can easily be done in 32-bit registers)
	// 2) We grab the value within the positive blakserv-integer mask by &
	// 3) This means our returned tick rolls over every 134217.728s (~37 hrs)
	// 4) Roll-Over means anything calculating the timespan of something before and after the roll-over
	//    will return a negative timespan (but only once).
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = (int)((unsigned int)tick & MAX_KOD_INT);
	
	return ret_val.int_val;
}


int C_Random(int object_id,local_var_type *local_vars,
			 int num_normal_parms,parm_node normal_parm_array[],
			 int num_name_parms,parm_node name_parm_array[])
{
	val_type low_bound,high_bound;
	val_type ret_val;
	int randomValue;
	
	low_bound = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
			     normal_parm_array[0].value);
	high_bound = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	if (low_bound.v.tag != TAG_INT || high_bound.v.tag != TAG_INT)
	{
		bprintf("C_Random got an invalid boundary %i,%i or %i,%i\n",
			low_bound.v.tag,low_bound.v.data,high_bound.v.tag,
			high_bound.v.data);
		return NIL;
	}
	if (low_bound.v.data > high_bound.v.data)
	{
		bprintf("C_Random got low > high boundary %i and %i\n",
			low_bound.v.data,high_bound.v.data);
		return NIL;
	}
	ret_val.v.tag = TAG_INT;
#if 0
	ret_val.v.data = low_bound.v.data + rand() % (high_bound.v.data -
		low_bound.v.data + 1);
#else
	// The rand() function returns number between 0 and 0x7fff (MAX_RAND)
	// we have to scale this to fit our range. -- call twice to fill all the bits
	// and mask to a 28 bit positive kod integer
	randomValue = MAX_KOD_INT & ((rand() << 15) + rand());
	ret_val.v.data = low_bound.v.data + randomValue % (high_bound.v.data - 
		low_bound.v.data + 1);
#endif
	return ret_val.int_val;
	
}

int C_Abs(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[])
{
	val_type int_val,ret_val;
	
	int_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (int_val.v.tag != TAG_INT)
	{
		bprintf("C_Abs can't use %i,%i\n",int_val.v.tag,int_val.v.data);
		return NIL;
	}
	
	ret_val.v.tag = TAG_INT;
	if (int_val.v.data & (1 << 27))
		ret_val.v.data = -int_val.v.data;
	else
		ret_val.v.data = int_val.v.data;
	
	return ret_val.int_val;  
}

int C_Sqrt(int object_id,local_var_type *local_vars,
		   int num_normal_parms,parm_node normal_parm_array[],
		   int num_name_parms,parm_node name_parm_array[])
{
	val_type int_val,ret_val;
	
	int_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (int_val.v.tag != TAG_INT)
	{
		bprintf("C_Sqrt can't use %i,%i\n",int_val.v.tag,int_val.v.data);
		return NIL;
	}
	if (int_val.v.data & (1 << 27))
	{
		bprintf("C_Sqrt result undefined for negative value\n");
		return NIL;
	}
	
	ret_val.v.tag = TAG_INT;
	ret_val.v.data = (int)sqrt((double)int_val.v.data);
	
	return ret_val.int_val;  
}

int C_Bound(int object_id,local_var_type *local_vars,
			int num_normal_parms,parm_node normal_parm_array[],
			int num_name_parms,parm_node name_parm_array[])
{
	val_type int_val,min_val,max_val;
	
	int_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (int_val.v.tag != TAG_INT)
	{
		bprintf("C_Bound can't use %i,%i\n",int_val.v.tag,int_val.v.data);
		return NIL;
	}
	
	min_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	if (min_val.v.tag != TAG_NIL)
	{
		if (min_val.v.tag != TAG_INT)
		{
			bprintf("C_Bound can't use min bound %i,%i\n",min_val.v.tag,min_val.v.data);
			return NIL;
		}
		if (int_val.v.data < min_val.v.data)
			int_val.v.data = min_val.v.data;
	}	 
	
	max_val = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
		normal_parm_array[2].value);
	if (max_val.v.tag != TAG_NIL)
	{
		if (max_val.v.tag != TAG_INT)
		{
			bprintf("C_Bound can't use max bound %i,%i\n",max_val.v.tag,max_val.v.data);
			return NIL;
		}
		if (int_val.v.data > max_val.v.data)
			int_val.v.data = max_val.v.data;
	}	 
	
	return int_val.int_val;
}

int C_CreateTable(int object_id,local_var_type *local_vars,
                  int num_normal_parms,parm_node normal_parm_array[],
                  int num_name_parms,parm_node name_parm_array[])
{
   val_type ret_val, size_val;

   if (num_normal_parms == 0)
      size_val.v.data = 73;
   else
   {
      size_val = RetrieveValue(object_id, local_vars, normal_parm_array[0].type,
         normal_parm_array[0].value);
      if (size_val.v.tag != TAG_INT)
      {
         bprintf("C_CreateTable can't use non-int %i,%i for size\n",
            size_val.v.tag, size_val.v.data);
         size_val.v.data = 73;
      }
   }

   ret_val.v.tag = TAG_TABLE;
   ret_val.v.data = CreateTable(size_val.v.data);

   return ret_val.int_val;
}

int C_AddTableEntry(int object_id,local_var_type *local_vars,
					int num_normal_parms,parm_node normal_parm_array[],
					int num_name_parms,parm_node name_parm_array[])
{
	val_type table_val,key_val,data_val;
	
	
	table_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (table_val.v.tag != TAG_TABLE)
	{
		bprintf("C_AddTableEntry can't use table id %i,%i\n",table_val.v.tag,table_val.v.data);
		return NIL;
	}
	
	key_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	
   // Can't use key value that might change. Strings are okay,
   // because the string itself is hashed.
   if (key_val.v.tag == TAG_OBJECT || key_val.v.tag == TAG_LIST
      || key_val.v.tag == TAG_TIMER || key_val.v.tag == TAG_TABLE
      || key_val.v.tag == TAG_CLASS)
   {
      bprintf("C_AddTableEntry can't use key id %i,%i\n",
         key_val.v.tag, key_val.v.data);
      return NIL;
   }

	data_val = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
		normal_parm_array[2].value);
	
	InsertTable(table_val.v.data,key_val,data_val);
	return NIL;
}

int C_GetTableEntry(int object_id,local_var_type *local_vars,
					int num_normal_parms,parm_node normal_parm_array[],
					int num_name_parms,parm_node name_parm_array[])
{
	val_type table_val,key_val,ret_val;
	
	
	table_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (table_val.v.tag != TAG_TABLE)
	{
		bprintf("C_GetTableEntry can't use table id %i,%i\n",table_val.v.tag,table_val.v.data);
		return NIL;
	}
	
	key_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	
	ret_val.int_val = GetTableEntry(table_val.v.data,key_val);
	return ret_val.int_val;
}

int C_DeleteTableEntry(int object_id,local_var_type *local_vars,
					   int num_normal_parms,parm_node normal_parm_array[],
					   int num_name_parms,parm_node name_parm_array[])
{
	val_type table_val,key_val;
	
	
	table_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (table_val.v.tag != TAG_TABLE)
	{
		bprintf("C_DeleteTableEntry can't use table id %i,%i\n",table_val.v.tag,table_val.v.data);
		return NIL;
	}
	
	key_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);
	
	DeleteTableEntry(table_val.v.data,key_val);
	return NIL;
}

int C_DeleteTable(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type table_val;
	
	table_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (table_val.v.tag != TAG_TABLE)
	{
		bprintf("C_DeleteTable can't use table id %i,%i\n",table_val.v.tag,table_val.v.data);
		return NIL;
	}
	bprintf("C_DeleteTable is deprecated, tables are deleted at GC.\n");
	//DeleteTable(table_val.v.data);
	return NIL;
}

int C_IsTable(int object_id,local_var_type *local_vars,
            int num_normal_parms,parm_node normal_parm_array[],
            int num_name_parms,parm_node name_parm_array[])
{
   val_type check_val, ret_val;

   ret_val.v.tag = TAG_INT;

   check_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
      normal_parm_array[0].value);

   if (check_val.v.tag == TAG_TABLE && GetTableByID(check_val.v.data))
      ret_val.v.data = True;
   else
      ret_val.v.data = False;

   return ret_val.int_val;
}

int C_RecycleUser(int object_id,local_var_type *local_vars,
				  int num_normal_parms,parm_node normal_parm_array[],
				  int num_name_parms,parm_node name_parm_array[])
{
	val_type object_val;
	object_node *o;
	user_node *old_user;
	user_node *new_user;
	
	object_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	if (object_val.v.tag != TAG_OBJECT)
	{
		bprintf("C_RecycleUser can't recycle non-object %i,%i\n",object_val.v.tag,object_val.v.data);
		return NIL;
	}
	
	o = GetObjectByID(object_val.v.data);
	if (o == NULL)
	{
		bprintf("C_RecycleUser can't find object %i\n",object_val.v.data);
		return NIL;
	}
	
	// Find the old user from the object that KOD gives us.
	old_user = GetUserByObjectID(o->object_id);
	if (old_user == NULL)
	{
		bprintf("C_RecycleUser can't find user which is object %i\n",object_val.v.data);
		return NIL;
	}
	
	// Create another user which matches the old one.
	new_user = CreateNewUser(old_user->account_id,o->class_id);
	//bprintf("C_RecycleUser made new user, got object %i\n",new_user->object_id);
	
	// Delete the old user/object.
	// KOD:  post(old_user_object,@Delete);
	//
	PostBlakodMessage(old_user->object_id,DELETE_MSG,0,NULL);
	DeleteUserByObjectID(old_user->object_id);
	//bprintf("C_RecycleUser deleted old user and object %i\n",old_user->object_id);
	
	object_val.v.tag = TAG_OBJECT;
	object_val.v.data = new_user->object_id;
	
	return object_val.int_val;
}

int C_IsObject(int object_id,local_var_type *local_vars,
			   int num_normal_parms,parm_node normal_parm_array[],
			   int num_name_parms,parm_node name_parm_array[])
{
	val_type var_check,ret_val;
	
	var_check = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
			     normal_parm_array[0].value);
	
	ret_val.v.tag = TAG_INT;
	if (var_check.v.tag == TAG_OBJECT && GetObjectByID(var_check.v.data))
		ret_val.v.data = True;
	else
		ret_val.v.data = False;
	
	return ret_val.int_val;
}

int C_MinigameNumberToString(int object_id,local_var_type *local_vars,
				int num_normal_parms,parm_node normal_parm_array[],
				int num_name_parms,parm_node name_parm_array[])
{
	val_type s1_val,s2_val;
	string_node *snod;
   int number;
   char newString[4];
   int index, iTemp;
	
	s1_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	
	if (s1_val.v.tag != TAG_STRING)
	{
		bprintf("C_MinigameNumberToString can't set non-string %i,%i\n",
			s1_val.v.tag,s1_val.v.data);
		return NIL;
	}
	
	snod = GetStringByID(s1_val.v.data);
	if (snod == NULL)
	{
		bprintf("C_MinigameNumberToString can't set invalid string %i,%i\n",
			s1_val.v.tag,s1_val.v.data);
		return NIL;
	}
	
	s2_val = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
		normal_parm_array[1].value);

	if (s2_val.v.tag != TAG_INT)
	{
		bprintf("C_MinigameNumberToString can't set from non-int %i,%i\n",
			s2_val.v.tag,s2_val.v.data);
		return NIL;
	}

   number = s1_val.v.data;

   index = 3;

   // This reverses the number on purpose, since the first four bits aren't a full number.
   while (index >= 0)
   {
      // Take off last byte off number.
      iTemp = number & 0xFF;
      number = number >> 8;

      newString[index] = (char)iTemp;

      index--;
   }

   SetString(snod, newString, 4);

	return NIL;
}

int C_MinigameStringToNumber(int object_id,local_var_type *local_vars,
				int num_normal_parms,parm_node normal_parm_array[],
				int num_name_parms,parm_node name_parm_array[])
{
 	val_type s1_val, ret_val;
	string_node *snod;
   int number;
   int index;

	s1_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
		normal_parm_array[0].value);
	
	if (s1_val.v.tag != TAG_STRING)
	{
		bprintf("C_MinigameNumberToString can't set non-string %i,%i\n",
			s1_val.v.tag,s1_val.v.data);
		return NIL;
	}
	
	snod = GetStringByID(s1_val.v.data);
	if (snod == NULL)
	{
		bprintf("C_MinigameNumberToString can't set invalid string %i,%i\n",
			s1_val.v.tag,s1_val.v.data);
		return NIL;
	}

   number = 0;

   index = 0;

   // Chew the string, spit out an int in reverse order.
   while (index < 4)
   {
      number = number << 8;
      number = number & snod->data[index];

      index = index + 1;
   }

	ret_val.v.tag = TAG_INT;
	ret_val.v.data = number;
	
	return ret_val.int_val;
}

int C_RecordStat(int object_id,local_var_type *local_vars,
				int num_normal_parms,parm_node normal_parm_array[],
				int num_name_parms,parm_node name_parm_array[])
{	
	val_type stat_type, stat1, stat2, stat3, stat4, stat5, stat6, stat7, stat8, stat9, stat10, stat11, stat12, stat13;
	resource_node *r_who_damaged, *r_who_attacker, *r_weapon, *r_victim, *r_killer, *r_room, *r_attack,
      *r_name, *r_home, *r_bind, *r_leader, *r_ghall;

   session_node *session;
   string_node *snod;
   char *c_guild_name, *c_guild_hall;

	// The first paramenter to RecordStat() should alwasy be a STAT_TYPE
	stat_type = RetrieveValue(object_id,local_vars,normal_parm_array[0].type, normal_parm_array[0].value);
	if (stat_type.v.tag != TAG_INT)
	{
		bprintf("STAT_TYPE expected in C_RecordStat() as first parameter");
		return NIL;
	}

	/*
	STAT_TYPE enum located in database.h, Also defined in blakston.khd to match between C code and Kod code.
	this switch statement should evaluate what kind of statistic is being passed, parse the remaining parameters
	and send them to the function in database.c that actually writes the data to the MySQL Database
	*/
	switch (stat_type.v.data)
	{
		case STAT_TOTALMONEY:
			if (num_normal_parms != 2)
			{
				bprintf("Wrong Number of Paramenters in C_RecordStat() STAT_TOTALMONEY");
				break;
			}
			
			stat1 = RetrieveValue(object_id,local_vars,normal_parm_array[1].type, normal_parm_array[1].value);
			
			if (stat1.v.tag != TAG_INT)
			{
				bprintf("Wrong Type of Parameter in C_RecordStat() STAT_TOTALMONEY");
				break;
			}
			else
			{
				MySQLRecordTotalMoney(stat1.v.data);
			}
			break;

		case STAT_MONEYCREATED:
			if (num_normal_parms != 2)
			{
				bprintf("Wrong Number of Paramenters in C_RecordStat() STAT_MONEYCREATED");
				break;
			}

			stat1 = RetrieveValue(object_id,local_vars,normal_parm_array[1].type, normal_parm_array[1].value);
			
			if (stat1.v.tag != TAG_INT)
			{
				bprintf("Wrong Type of Parameter in C_RecordStat() STAT_TOTALMONEY");
				break;
			}
			else
			{
				MySQLRecordMoneyCreated(stat1.v.data);				
			}
			break;

		case STAT_ASSESS_DAM:
			if (num_normal_parms != 8) 
			{
				bprintf("Wrong Number of Paramenters in C_RecordStat() STAT_ASSESS_DAM");
				break;
			}

			stat1 = RetrieveValue(object_id,local_vars,normal_parm_array[1].type, normal_parm_array[1].value);
			stat2 = RetrieveValue(object_id,local_vars,normal_parm_array[2].type, normal_parm_array[2].value);
			stat3 = RetrieveValue(object_id,local_vars,normal_parm_array[3].type, normal_parm_array[3].value);
			stat4 = RetrieveValue(object_id,local_vars,normal_parm_array[4].type, normal_parm_array[4].value);
			stat5 = RetrieveValue(object_id,local_vars,normal_parm_array[5].type, normal_parm_array[5].value);
			stat6 = RetrieveValue(object_id,local_vars,normal_parm_array[6].type, normal_parm_array[6].value);
			stat7 = RetrieveValue(object_id,local_vars,normal_parm_array[7].type, normal_parm_array[7].value);
			
			if (stat1.v.tag != TAG_RESOURCE || 
				stat2.v.tag != TAG_RESOURCE ||
				stat3.v.tag != TAG_INT ||
				stat4.v.tag != TAG_INT ||
				stat5.v.tag != TAG_INT ||
				stat6.v.tag != TAG_INT ||
				stat7.v.tag != TAG_RESOURCE)
			{
				bprintf("Wrong Type of Parameter in C_RecordStat() STAT_ASSESS_DAM");
				break;
			}
			else
			{	
				r_who_damaged = GetResourceByID(stat1.v.data);
				r_who_attacker = GetResourceByID(stat2.v.data);
				r_weapon = GetResourceByID(stat7.v.data);
				
				if (!r_who_damaged || !r_who_attacker || !r_weapon ||
					!r_who_damaged->resource_val[0] || !r_who_attacker->resource_val[0] || !r_weapon->resource_val[0])
				{
					bprintf("NULL string in C_RecordStat() for STAT_ASSESS_DAM");				
				}
				else
				{					
					MySQLRecordPlayerAssessDamage(
						r_who_damaged->resource_val[0], 
						r_who_attacker->resource_val[0], 
						stat3.v.data, stat4.v.data, stat5.v.data, stat6.v.data, 
						r_weapon->resource_val[0]);
				}
			}
			break;

      case STAT_PLAYERDEATH:
         if (num_normal_parms != 6)
         {
            bprintf("Wrong Number of Paramenters in C_RecordStat() STAT_PLAYERDEATH");
            break;
         }

         stat1 = RetrieveValue(object_id, local_vars, normal_parm_array[1].type, normal_parm_array[1].value);
         stat2 = RetrieveValue(object_id, local_vars, normal_parm_array[2].type, normal_parm_array[2].value);
         stat3 = RetrieveValue(object_id, local_vars, normal_parm_array[3].type, normal_parm_array[3].value);
         stat4 = RetrieveValue(object_id, local_vars, normal_parm_array[4].type, normal_parm_array[4].value);
         stat5 = RetrieveValue(object_id, local_vars, normal_parm_array[5].type, normal_parm_array[5].value);

         if (stat1.v.tag != TAG_RESOURCE ||
            stat2.v.tag != TAG_RESOURCE ||
            stat3.v.tag != TAG_RESOURCE ||
            stat4.v.tag != TAG_RESOURCE ||
            stat5.v.tag != TAG_INT)
         {
            bprintf("Wrong Type of Parameter in C_RecordStat() STAT_PLAYERDEATH");
            break;
         }
         else
         {
            r_victim = GetResourceByID(stat1.v.data);
            r_killer = GetResourceByID(stat2.v.data);
            r_room = GetResourceByID(stat3.v.data);
            r_attack = GetResourceByID(stat4.v.data);

            if (!r_victim || !r_killer || !r_room || !r_attack ||
               !r_victim->resource_val[0] || !r_killer->resource_val[0] || !r_room->resource_val[0] || !r_attack->resource_val[0])
            {
               bprintf("NULL string in C_RecordStat() for STAT_PLAYERDEATH");
            }
            else
            {

               MySQLRecordPlayerDeath(
                  r_victim->resource_val[0],
                  r_killer->resource_val[0],
                  r_room->resource_val[0],
                  r_attack->resource_val[0],
                  stat5.v.data);
            }
         }
         break;

      case STAT_PLAYER:
         if (num_normal_parms != 14)
         {
            bprintf("Wrong Number of Paramenters in C_RecordStat() STAT_PLAYER");
            break;
         }

         stat1 = RetrieveValue(object_id, local_vars, normal_parm_array[1].type, normal_parm_array[1].value);
         stat2 = RetrieveValue(object_id, local_vars, normal_parm_array[2].type, normal_parm_array[2].value);
         stat3 = RetrieveValue(object_id, local_vars, normal_parm_array[3].type, normal_parm_array[3].value);
         stat4 = RetrieveValue(object_id, local_vars, normal_parm_array[4].type, normal_parm_array[4].value);
         stat5 = RetrieveValue(object_id, local_vars, normal_parm_array[5].type, normal_parm_array[5].value);
         stat6 = RetrieveValue(object_id, local_vars, normal_parm_array[6].type, normal_parm_array[6].value);
         stat7 = RetrieveValue(object_id, local_vars, normal_parm_array[7].type, normal_parm_array[7].value);
         stat8 = RetrieveValue(object_id, local_vars, normal_parm_array[8].type, normal_parm_array[8].value);
         stat9 = RetrieveValue(object_id, local_vars, normal_parm_array[9].type, normal_parm_array[9].value);
         stat10 = RetrieveValue(object_id, local_vars, normal_parm_array[10].type, normal_parm_array[10].value);
         stat11 = RetrieveValue(object_id, local_vars, normal_parm_array[11].type, normal_parm_array[11].value);
         stat12 = RetrieveValue(object_id, local_vars, normal_parm_array[12].type, normal_parm_array[12].value);
         stat13 = RetrieveValue(object_id, local_vars, normal_parm_array[13].type, normal_parm_array[13].value);

         if (stat1.v.tag != TAG_SESSION)
         {
            bprintf("C_RecordStat can't use non-session %i,%i\n", stat1.v.tag, stat1.v.data);
            return NIL;
         }

         if (stat2.v.tag != TAG_RESOURCE ||
            stat3.v.tag != TAG_RESOURCE ||
            stat4.v.tag != TAG_RESOURCE ||
            stat6.v.tag != TAG_INT ||
            stat7.v.tag != TAG_INT ||
            stat8.v.tag != TAG_INT ||
            stat9.v.tag != TAG_INT ||
            stat10.v.tag != TAG_INT ||
            stat11.v.tag != TAG_INT ||
            stat12.v.tag != TAG_INT ||
            stat13.v.tag != TAG_INT)
         {

            bprintf("Wrong Type of Parameter in C_RecordStat() STAT_PLAYER");
            break;
         }
         else
         {
            if (stat5.v.tag != TAG_STRING)
            {
               c_guild_name = "";
            }
            else
            {
               snod = GetStringByID(stat5.v.data);
               if (snod == NULL)
               {
                  bprintf("C_RecordStat guild string is null");
                  break;
               }
               c_guild_name = snod->data;
            }

            session = GetSessionByID(stat1.v.data);
            r_name = GetResourceByID(stat2.v.data);
            r_home = GetResourceByID(stat3.v.data);
            r_bind = GetResourceByID(stat4.v.data);

            if (!session->account->account_id || !r_name || !r_home || !r_bind || !c_guild_name ||
               !r_name->resource_val[0] || !r_home->resource_val[0] || !r_bind->resource_val[0])
            {
               bprintf("NULL string in C_RecordStat() for STAT_PLAYER");
            }
            else
            {

               MySQLRecordPlayer(
                  session->account->account_id,
                  r_name->resource_val[0],
                  r_home->resource_val[0],
                  r_bind->resource_val[0],
                  c_guild_name,
                  stat6.v.data,
                  stat7.v.data,
                  stat8.v.data,
                  stat9.v.data,
                  stat10.v.data,
                  stat11.v.data,
                  stat12.v.data,
                  stat13.v.data);
            }
         }
         break;

      case STAT_PLAYERSUICIDE:
         if (num_normal_parms != 3)
         {
            bprintf("Wrong Number of Paramenters in C_RecordStat() STAT_PLAYERSUICIDE");
            break;
         }

         stat1 = RetrieveValue(object_id, local_vars, normal_parm_array[1].type, normal_parm_array[1].value);
         stat2 = RetrieveValue(object_id, local_vars, normal_parm_array[2].type, normal_parm_array[2].value);

         if (stat1.v.tag != TAG_SESSION)
         {
            bprintf("C_RecordStat STAT_PLAYERSUICIDE can't use non-session %i,%i\n", stat1.v.tag, stat1.v.data);
            return NIL;
         }

         if (stat2.v.tag != TAG_RESOURCE)
         {
            bprintf("Wrong Type of Parameter in C_RecordStat() STAT_PLAYERSUICIDE");
            break;
         }

         session = GetSessionByID(stat1.v.data);
         r_name = GetResourceByID(stat2.v.data);

         if (!session->account->account_id || !r_name || !r_name->resource_val[0])
         {
            bprintf("NULL string in C_RecordStat() for STAT_PLAYERSUICIDE");
         }
         else
         {

            MySQLRecordPlayerSuicide(session->account->account_id, r_name->resource_val[0]);
         }

         break;

      case STAT_GUILD:
         if (num_normal_parms != 4)
         {
            bprintf("Wrong Number of Paramenters in C_RecordStat() STAT_GUILD");
            break;
         }

         stat1 = RetrieveValue(object_id, local_vars, normal_parm_array[1].type, normal_parm_array[1].value);
         stat2 = RetrieveValue(object_id, local_vars, normal_parm_array[2].type, normal_parm_array[2].value);
         stat3 = RetrieveValue(object_id, local_vars, normal_parm_array[3].type, normal_parm_array[3].value);

         if (stat1.v.tag != TAG_STRING ||
            stat2.v.tag != TAG_RESOURCE ||
            stat3.v.tag != TAG_RESOURCE)
         {
            bprintf("Wrong Type of Parameter in C_RecordStat() STAT_GUILD");
            return NIL;
         }

         snod = GetStringByID(stat1.v.data);
         if (snod == NULL)
         {
            bprintf("C_RecordStat STAT_GUILD, guild string is null");
            break;
         }


         c_guild_name = snod->data;
         r_leader = GetResourceByID(stat2.v.data);
         r_ghall = GetResourceByID(stat3.v.data);

         if (!r_ghall || !r_ghall->resource_val[0])
         {
            c_guild_hall = "";
         }
         else
         {
            c_guild_hall = r_ghall->resource_val[0];
         }

         if (!r_leader ||
            !r_leader->resource_val[0])
         {
            bprintf("NULL string in C_RecordStat() for STAT_GUILD");
         }
         else
         {

            MySQLRecordGuild(
               c_guild_name,
               r_leader->resource_val[0],
               c_guild_hall);
         }

         break;

      case STAT_GUILDDISBAND:
         if (num_normal_parms != 2)
         {
            bprintf("Wrong Number of Paramenters in C_RecordStat() STAT_GUILDDISBAND");
            break;
         }

         stat1 = RetrieveValue(object_id, local_vars, normal_parm_array[1].type, normal_parm_array[1].value);

         if (stat1.v.tag != TAG_STRING)
         {
            bprintf("Wrong Type of Parameter in C_RecordStat() STAT_GUILDDISBAND");
            return NIL;
         }

         snod = GetStringByID(stat1.v.data);
         if (snod == NULL)
         {
            bprintf("C_RecordStat STAT_GUILDDISBAND, guild name is null");
            break;
         }

         c_guild_name = snod->data;

         MySQLRecordGuildDisband(c_guild_name);

         break;

		default:
			bprintf("ERROR: Unknown stat_type (%d) in C_RecordStat",stat_type.v.data);
			break;
	}

	return NIL;
}

int C_GetSessionIP(int object_id,local_var_type *local_vars,
            int num_normal_parms,parm_node normal_parm_array[],
            int num_name_parms,parm_node name_parm_array[])
{
   val_type session_id, temp, ret_val;
   session_node* session = NULL;
   
   session_id = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
      normal_parm_array[0].value);
      
   
   if (session_id.v.tag != TAG_SESSION)
   {
      bprintf("C_GetSessionIP can't use non session %i,%i\n",session_id.v.tag,session_id.v.data);
      return NIL;
   }

   session = GetSessionByID(session_id.v.data);
   
   if (!session)
   {
      bprintf("C_GetSessionIP can't find session for %i,%i\n", session_id.v.tag, session_id.v.data);
      return NIL;
   }
   
   ret_val.int_val = NIL;
   temp.v.tag = TAG_INT;
   
   // reverse the order, because the address is stored in network order in in6_addr
   for (int i = sizeof(struct in6_addr) - 1; i >= 0; i--)
   {
      temp.v.data = session->conn.addr.u.Byte[i];
      ret_val.v.data = Cons(temp, ret_val);
      ret_val.v.tag = TAG_LIST;
   }

   return ret_val.int_val;
}

int C_SetClassVar(int object_id,local_var_type *local_vars,
            int num_normal_parms,parm_node normal_parm_array[],
            int num_name_parms,parm_node name_parm_array[])
{
   // Note that setting a class var is only temporary until the server restarts.
   class_node *c;
   object_node *o;
   val_type ret_val, class_val, data_str, var_name;
   int var_id;
   const char *pStrConst;

   ret_val.v.tag = TAG_INT;
   ret_val.v.data = False;

   class_val = RetrieveValue(object_id,local_vars,normal_parm_array[0].type,
                  normal_parm_array[0].value);
   var_name = RetrieveValue(object_id,local_vars,normal_parm_array[1].type,
                  normal_parm_array[1].value);
   data_str = RetrieveValue(object_id,local_vars,normal_parm_array[2].type,
                  normal_parm_array[2].value);

   if (class_val.v.tag == TAG_OBJECT)
   {
      o = GetObjectByID(class_val.v.data);
      if (o == NULL)
      {
         bprintf("C_SetClassVar can't find the class of object %i\n",
               class_val.v.data);
         return ret_val.int_val;
      }

      class_val.v.tag = TAG_CLASS;
      class_val.v.data = o->class_id;
   }

   if (class_val.v.tag != TAG_CLASS)
   {
      bprintf("C_SetClassVar can't look for non-class %i,%i\n",
         class_val.v.tag,class_val.v.data);
      return ret_val.int_val;
   }

   c = GetClassByID(class_val.v.data);
   if (c == NULL)
   {
      bprintf("C_SetClassVar cannot find class %i.\n",class_val.v.data);
      return ret_val.int_val;
   }

   if (var_name.v.tag != TAG_DEBUGSTR)
   {
      bprintf("C_SetClassVar passed bad class var string, tag %i.\n",
         var_name.v.tag);
      return ret_val.int_val;
   }

   kod_statistics *kstat = GetKodStats();
   class_node *c2 = GetClassByID(kstat->interpreting_class);
   if (c2 == NULL)
   {
      bprintf("C_SetClassVar can't find class %i, can't get debug str\n",
            kstat->interpreting_class);
      return ret_val.int_val;
   }

   pStrConst = GetClassDebugStr(c2,var_name.v.data);
   if (pStrConst == NULL)
   {
      bprintf("C_SetClassVar: GetClassDebugStr returned NULL\n");
      return ret_val.int_val;
   }

   var_id = GetClassVarIDByName(c, pStrConst);
   if (var_id == INVALID_CLASSVAR)
   {
      bprintf("C_SetClassVar cannot find classvar named %s in class %i.\n",
            pStrConst,class_val.v.data);
      return ret_val.int_val;
   }

   c->vars[var_id].val = data_str;
   ret_val.v.data = True;

   return ret_val.int_val;
}
