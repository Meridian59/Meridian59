// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * parsecli.c
 *

  This module has a few functions that concern communication between
  the client and the Blakod.  
  
	This module handles parsing tables to parse client messages and send
	the messages to the blakod.
	
*/

#include "blakserv.h"

#define OBJECT_ERROR1 1
#define OBJECT_ERROR2 2
#define OBJECT_ERROR3 3
#define LENGTH_ERROR1 4
#define LENGTH_ERROR2 5
#define LENGTH_ERROR3 6
#define OBJECT_LIST_ERROR 7

client_table_node *user_table,*system_table,*usercommand_table;

/* stuff for client -> server */
val_type cli_list_nodes[MAX_CLIENT_PARMS];

/* local function prototypes */
void ParseClientSendBlakod(int session_id,int msg_len,unsigned char *msg_data,int object_id,
						   client_table_type command_table);

// Handles more common error messages for ParseClientSendBlakod,
// adds extra info to track down the error.
inline void ParseClientSendBlakodError(int err_type, int session_id, int data, int protocol, int parm)
{
   session_node *s = GetSessionByID(session_id);
   int client_version = 0, account_id = 0;

   if (s)
   {
      client_version = s->version_minor + 100 * s->version_major;
      if (s->account)
         account_id = s->account->account_id;
   }
   switch (err_type)
   {
   case OBJECT_ERROR1:
   case OBJECT_ERROR2:
   case OBJECT_ERROR3:
      eprintf("ParseClientSendBlakod %i got invalid obj ref %i, protocol %i, cli parm %i, cli vers %i, acct id %i\n",
         err_type, data, protocol, parm, client_version, account_id);
      break;
   case LENGTH_ERROR1:
   case LENGTH_ERROR2:
   case LENGTH_ERROR3:
      eprintf("ParseClientSendBlakod %i got invalid short message len %i, protocol %i, cli parm %i, cli vers %i, acct id %i\n",
         err_type - 3, data, protocol, parm, client_version, account_id);
      break;
   case OBJECT_LIST_ERROR:
      eprintf("ParseClientSendBlakod got invalid obj ref %i in a list, protocol %i, cli parm %i, cli vers %i, acct id %i\n",
         data, protocol, parm, client_version, account_id);
      break;
   }
}

void InitParseClient(void)
{
	LoadParseClientTables();
	AllocateParseClientListNodes();
}

void LoadParseClientTables(void)
{
	int i;

   InitSprocket();
	
	user_table = GetParseClientTable(SPROCKET_TABLE_USER);
	if (user_table == NULL)
		FatalError("ParseClientLoadTables can't get user table");
	
	for (i=0;i<CLIENT_COMMANDS_PER_TABLE;i++)
		user_table[i].call_count = 0;
	
	system_table = GetParseClientTable(SPROCKET_TABLE_SYSTEM);
	if (system_table == NULL)
		FatalError("ParseClientLoadTables can't get system table");
	
	for (i=0;i<CLIENT_COMMANDS_PER_TABLE;i++)
		system_table[i].call_count = 0;
	
	usercommand_table = GetParseClientTable(SPROCKET_TABLE_USERCOMMAND);
	if (usercommand_table == NULL)
		FatalError("ParseClientLoadTables can't get user command table");
	
	for (i=0;i<CLIENT_COMMANDS_PER_TABLE;i++)
		usercommand_table[i].call_count = 0;     
}

void ResetParseClientTables(void)
{
	user_table = NULL;
	system_table = NULL;
	usercommand_table = NULL;
}

void AllocateParseClientListNodes()
{
	val_type list_val,temp;
	int i;
	
	/* allocate constant list that we set the First values in for
    * client messages.
    */
	
	temp.int_val = NIL;
	list_val.int_val = NIL;
	for (i=MAX_CLIENT_PARMS-1;i>=0;i--)
	{
		list_val.v.data = Cons(temp,list_val);
		list_val.v.tag = TAG_LIST;
		cli_list_nodes[i] = list_val;
	}
}

void GameMessageCount(unsigned char message_type)
{
	user_table[message_type].call_count++;
}

void ClientToBlakodUser(session_node *session,int msg_len,char *msg_data)
{
	ParseClientSendBlakod(session->session_id,msg_len,(unsigned char *) msg_data,
		session->game->object_id,user_table);
}

void ParseClientSendBlakod(int session_id,int msg_len,unsigned char *msg_data,int object_id,
						   client_table_type command_table)
{
	int i,j,msg_byte_offset,num_parms;
	val_type command,in_val,temp,session_id_const,type_const;
	parm_node parms[5];
	int list_index, table_type;
	unsigned short len_str;
	
	val_type number_stuff;
	Bool found_number_stuff;
	
	msg_byte_offset = 0;
	
	if (msg_len == 0)
	{
		eprintf("ParseClientSendBlakod got zero length message\n");
		return;
	}
	
	command.v.tag = TAG_INT;
	command.v.data = (int)(msg_data[msg_byte_offset]);
	msg_byte_offset++;
	
	table_type = 0; /* sent to Blakod, to identify subtables */
	
	/* check if redirected to different table */
	switch (command_table[command.v.data].client_parms[0].type_parm)
	{
	case SYSTEM_TABLE_PARM :
		command_table = system_table;
		object_id = GetSystemObjectID();
		
		command.v.tag = TAG_INT;
		command.v.data = (int)(msg_data[msg_byte_offset]);
		msg_byte_offset++;
		
		command_table[command.v.data].call_count++;
		break;
		
	case USERCOMMAND_TABLE_PARM :
		command_table = usercommand_table;
		
		command.v.tag = TAG_INT;
		command.v.data = (int)(msg_data[msg_byte_offset]);
		msg_byte_offset++;
		
		command_table[command.v.data].call_count++;
		
		table_type = 1;
		break;
		
	default :
		/* don't need to do anything, just parse the normal message */;
		/* counting was already done in game.c, so don't need to add here */
	}
	
	if (command_table[command.v.data].client_parms[0].type_parm == INVALID_PARM)
	{
		eprintf("ParseClientSendBlakod got invalid command %i\n",command.v.data);
		return;
	}
	
	found_number_stuff = False;
	i = command.v.data;
	
	list_index = MAX_CLIENT_PARMS-1-command_table[i].num_parms;
	
	/* put on command to beginning of list */
	SetFirst(cli_list_nodes[list_index++].v.data,command);
	
	/* now add the parms to list in forward order */
	for (j=0;j<command_table[i].num_parms;j++)
	{
		if (msg_byte_offset > msg_len - command_table[i].client_parms[j].len_parm)
		{
			ParseClientSendBlakodError(LENGTH_ERROR1, session_id, msg_len, i, j);
			return;
		}
		
		switch (command_table[i].client_parms[j].len_parm)
		{
		case 0 :
			
			/* this is a variable length field, 2 bytes for length */
			/* make it either a string or the temp string, based upon tag */
			
			switch (command_table[i].client_parms[j].type_parm)
			{
			case TAG_STRING :
				len_str = *(short *)(msg_data + msg_byte_offset);
				if (msg_byte_offset + 2 + len_str > msg_len)
				{
					eprintf("ParseCientSendBlakod got invalid string length %u\n",len_str);
					return;
				}
				/*
				dprintf("string is ok--compare is %u 2 %u (total %u) with %u\n",
						  msg_byte_offset,len_str,msg_byte_offset+2+len_str,msg_len);
				*/
				temp.v.data = CreateStringWithLen((char *) msg_data+msg_byte_offset+2,
					len_str);
				temp.v.tag = TAG_STRING;
				
				/*
				{ 
				int i;
				dprintf("string len %i: ",len_str);
				for (i=0;i<len_str;i++)
				dprintf("%c",*(msg_data+msg_byte_offset+2+i));
				dprintf("\n");
				}
				*/
				
				msg_byte_offset += 2 + len_str;
				SetFirst(cli_list_nodes[list_index++].v.data,temp);
				break;
				
			case TAG_TEMP_STRING :
				len_str = *(short *)(msg_data + msg_byte_offset);
				if (msg_byte_offset + 2 + len_str > msg_len)
				{
					eprintf("ParseCientSendBlakod got invalid temp string length %u\n",len_str);
					return;
				}
				/*
				dprintf("temp string is ok--compare is %u 2 %u (total %u) with %u\n",
						  msg_byte_offset,len_str,msg_byte_offset+2+len_str,msg_len);
				*/
				SetTempString((char *) msg_data+msg_byte_offset+2,len_str);
				temp.v.data = 0; /* doesn't matter, has tag TAG_TEMP_STRING */
				temp.v.tag = TAG_TEMP_STRING;
				
				msg_byte_offset += 2 + len_str;
				SetFirst(cli_list_nodes[list_index++].v.data,temp);
				break;
			}
			break;
			
			case 1 : 
				temp.v.data = (int)(*(char *)(msg_data+msg_byte_offset));
				temp.v.tag = command_table[i].client_parms[j].type_parm;
				if (temp.v.tag == TAG_OBJECT &&
					NULL == GetObjectByID(temp.v.data))
				{
					ParseClientSendBlakodError(OBJECT_ERROR1, session_id, temp.v.data, i, j);
					return;
				}
				
				msg_byte_offset += 1;
				SetFirst(cli_list_nodes[list_index++].v.data,temp);
				break;
				
			case 2 : 
				temp.v.data = (int)(*(short *)(msg_data+msg_byte_offset));
				
				if (command_table[i].client_parms[j].type_parm >= LIST_PARM)
				{
					int len_list,k;
					val_type list_val;
					
					msg_byte_offset += 2;
					
					len_list = temp.v.data;
					list_val.int_val = NIL;
					
					found_number_stuff = True;
					number_stuff.int_val = NIL;
					
					/* dprintf("got list length %i,type %i\n",len_list,
					command_table[i].client_parms[j].type_parm); */
					if (command_table[i].client_parms[j].type_parm == LIST_OBJ_PARM)
					{
					/* object lists will be in reverse order (number items make it
						painful to reverse it here) */
						for (k=0;k<len_list;k++)
						{
							if (msg_byte_offset > msg_len-4)
							{
								ParseClientSendBlakodError(LENGTH_ERROR2, session_id, msg_len, i, j);
								break;
							}

							in_val.int_val = (int)(*(int *)(msg_data+msg_byte_offset));
							msg_byte_offset += 4;
							
							temp.v.data = in_val.v.data;
							temp.v.tag = TAG_OBJECT;
							if (/* temp.v.tag == TAG_OBJECT && */
								NULL == GetObjectByID(temp.v.data))
							{
								ParseClientSendBlakodError(OBJECT_LIST_ERROR, session_id, temp.v.data, i, j);
								return;
							}
							
							list_val.v.data = Cons(temp,list_val);
							list_val.v.tag = TAG_LIST;
							
							if (in_val.v.tag == CLIENT_TAG_NUMBER)
							{
								/* grab the number */
								temp.v.data = (int)(*(int *)(msg_data+msg_byte_offset));
								temp.v.tag = TAG_INT;
								number_stuff.v.data = Cons(temp,number_stuff);
								number_stuff.v.tag = TAG_LIST;
								
								msg_byte_offset += 4;
							}
						}
					}
					else
					{
						/* resource and integer lists will be forwards (because 
							we read it in reverse here) */
						if (msg_byte_offset + len_list*4 > msg_len)
						{
							eprintf("ParseClientSendBlakod got invalid len_list, truncating\n");
							len_list = (msg_len-msg_byte_offset)/4;
						}

						for (k=0;k<len_list;k++)
						{
							in_val.int_val = (int)(*(int *)(msg_data + msg_byte_offset+(len_list-k-1)*4));
							temp.v.data = in_val.v.data;
							
							switch (command_table[i].client_parms[j].type_parm)
							{
							case LIST_RSC_PARM : temp.v.tag = TAG_RESOURCE; break;
							case LIST_INT_PARM : temp.v.tag = TAG_INT; break;
							}
							list_val.v.data = Cons(temp,list_val);
							list_val.v.tag = TAG_LIST;
						}
						msg_byte_offset += len_list*4;
					}
					
					temp.int_val = list_val.int_val;
				}
				else
				{
					temp.v.tag = command_table[i].client_parms[j].type_parm;
					msg_byte_offset += 2;
				}
				
				if (temp.v.tag == TAG_OBJECT &&
					NULL == GetObjectByID(temp.v.data))
				{
					ParseClientSendBlakodError(OBJECT_ERROR2, session_id, temp.v.data, i, j);
					return;
				}
				
				SetFirst(cli_list_nodes[list_index++].v.data,temp);
				break;
				
			case 4 : 
				in_val.int_val = (int)(*(int *)(msg_data+msg_byte_offset));
				temp.v.data = in_val.v.data;
				temp.v.tag = command_table[i].client_parms[j].type_parm;
				if (temp.v.tag == TAG_OBJECT &&
					NULL == GetObjectByID(temp.v.data))
				{
					ParseClientSendBlakodError(OBJECT_ERROR3, session_id, temp.v.data, i, j);
					return;
				}
				
				/* dprintf("taking out an int %i\n",temp.v.data); */
				msg_byte_offset += 4;
				SetFirst(cli_list_nodes[list_index++].v.data,temp);
				
				if (in_val.v.tag == CLIENT_TAG_NUMBER)
				{
					if (found_number_stuff)
						eprintf("ParseClientSendBlakod found more than one number parm\n");
					
					/* grab the number */
					found_number_stuff = True;
					number_stuff.v.data = (int)(*(int *)(msg_data+msg_byte_offset));
					number_stuff.v.tag = TAG_INT;
					msg_byte_offset += 4;
				}
				break;
      }
   }
   
   /* check for broken protocol
   */
   if (msg_byte_offset > msg_len)
   {
      ParseClientSendBlakodError(LENGTH_ERROR3, session_id, msg_len, i, j);
      return;
   }
   
   /* give the message to KOD:
   *
   * send(OBJECT object_id,&ReceiveClient,
   *      #client_msg=[cli_list_nodes],
   *    { #number_stuff=[number_stuff],           }
   *      #session_id=SESSION session_id,
   *      #type=INT 0/1);
   *
   */
   
   parms[0].type = CONSTANT;
   parms[0].value = cli_list_nodes[MAX_CLIENT_PARMS-1-command_table[i].num_parms].int_val;
   parms[0].name_id = CLIENT_PARM;
   
   session_id_const.v.tag = TAG_SESSION;
   session_id_const.v.data = session_id;
   
   parms[1].type = CONSTANT;
   parms[1].value = session_id_const.int_val;
   parms[1].name_id = SESSION_ID_PARM;
   
   type_const.v.tag = TAG_INT;
   type_const.v.data = table_type; /* 0 = system or user, 1 = usercommand */
   
   parms[2].type = CONSTANT;
   parms[2].value = type_const.int_val;
   parms[2].name_id = TYPE_PARM;
   
   num_parms = 3;
   if (found_number_stuff)
   {
	   parms[num_parms].type = CONSTANT;
	   parms[num_parms].value = number_stuff.int_val;
	   parms[num_parms].name_id = NUMBER_STUFF_PARM;
	   
	   num_parms++;
   }
   
   SendTopLevelBlakodMessage(object_id,RECEIVE_CLIENT_MSG,num_parms,parms);
}
