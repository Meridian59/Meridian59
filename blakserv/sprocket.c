// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * sprocket.c
 *

*/

#include "blakserv.h"

typedef struct
{
	unsigned char client_cmd;
	client_parm client_parms[MAX_CLIENT_PARMS+1];
} client_def_table_type;

client_def_table_type client_def_table[] = 
{ 
	{ BP_REQ_MOVE,             { {2, TAG_INT}, {2, TAG_INT}, {1, TAG_INT}, {4, TAG_OBJECT},
	{0, DONE_PARM} } },
	{ BP_SEND_OBJECT_CONTENTS, { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ BP_SAY_TO,               { {1, TAG_INT}, {0,TAG_TEMP_STRING}, {0, DONE_PARM} } },
	{ BP_SAY_GROUP,            { {2, LIST_OBJ_PARM}, {0,TAG_TEMP_STRING}, {0, DONE_PARM} } },
	{ BP_REQ_ATTACK,           { {1, TAG_INT}, {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ BP_REQ_USE,              { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ BP_REQ_UNUSE,            { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ BP_SEND_PLAYER,          { {0, DONE_PARM} } },
	{ BP_SEND_ROOM_CONTENTS,   { {0, DONE_PARM} } },
	{ BP_REQ_INVENTORY,        { {0, DONE_PARM} } },
	{ BP_REQ_TURN,             { {4, TAG_OBJECT}, {2, TAG_INT}, {0, DONE_PARM} } },
	{ BP_REQ_GET,              { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ BP_REQ_DROP,             { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ BP_REQ_INVENTORY_MOVE,   { {4, TAG_OBJECT}, {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ BP_REQ_PUT,              { {4, TAG_OBJECT}, {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ BP_REQ_LOOK,             { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ BP_SEND_MAIL,            { {2, LIST_OBJ_PARM}, {0, TAG_STRING}, {0, DONE_PARM} } },
	{ BP_REQ_GET_MAIL,         { {0, DONE_PARM} } },
	{ BP_DELETE_MAIL,          { {4, TAG_INT}, {0, DONE_PARM} } },
	{ BP_DELETE_NEWS,          { {2, TAG_INT}, {4, TAG_INT}, {0, DONE_PARM} } },
	{ BP_SEND_STATS,           { {1, TAG_INT}, {0, DONE_PARM} } },
	{ BP_SEND_STAT_GROUPS,     { {0, DONE_PARM} } },
	{ BP_REQ_HIDE,             { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ BP_REQ_OFFER,            { {4, TAG_OBJECT}, {2, LIST_OBJ_PARM}, {0, DONE_PARM} } },
	{ BP_REQ_DEPOSIT,          { {4, TAG_OBJECT}, {2, LIST_OBJ_PARM}, {0, DONE_PARM} } },
	{ BP_CANCEL_OFFER,         { {0, DONE_PARM} } },
	{ BP_REQ_COUNTEROFFER,     { {2, LIST_OBJ_PARM}, {0, DONE_PARM} } },
	{ BP_ACCEPT_OFFER,         { {0, DONE_PARM} } },
	{ BP_REQ_GO,               { {0, DONE_PARM} } },
	{ BP_SEND_PLAYERS,         { {0, DONE_PARM} } },
	{ BP_REQ_BUY,              { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ BP_REQ_BUY_ITEMS,        { {4, TAG_OBJECT}, {2, LIST_OBJ_PARM}, {0, DONE_PARM} } },
	{ BP_REQ_WITHDRAWAL,       { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ BP_REQ_WITHDRAWAL_ITEMS, { {4, TAG_OBJECT}, {2, LIST_OBJ_PARM}, {0, DONE_PARM} } },
	{ BP_REQ_SHOOT,            { {2, TAG_INT}, {2,TAG_INT}, {0, DONE_PARM} } },
	{ BP_REQ_APPLY,            { {4, TAG_OBJECT}, {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ BP_SEND_SPELLS,          { {0, DONE_PARM} } },
	{ BP_SEND_SKILLS,          { {0, DONE_PARM} } },
	{ BP_REQ_CAST,             { {4, TAG_OBJECT}, {2, LIST_OBJ_PARM}, {0, DONE_PARM} } },
	{ BP_REQ_ARTICLES,         { {2, TAG_INT}, {0, DONE_PARM} } },
	{ BP_REQ_ARTICLE,          { {2, TAG_INT}, {4, TAG_INT}, {0, DONE_PARM} } },
	{ BP_POST_ARTICLE,         { {2, TAG_INT}, {0, TAG_STRING}, {0, TAG_STRING}, {0, DONE_PARM} } },
	{ BP_REQ_ADMIN,            { {0, TAG_TEMP_STRING}, {0, DONE_PARM} } },
	{ BP_ACTION,               { {1, TAG_INT}, {0, DONE_PARM} } },
	{ BP_SAY_BLOCKED,          { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ BP_REQ_LOOKUP_NAMES,     { {2, TAG_INT}, {0, TAG_TEMP_STRING}, {0, DONE_PARM} } },
	{ BP_CHANGE_DESCRIPTION,   { {4, TAG_OBJECT}, {0, TAG_STRING}, {0, DONE_PARM} } },
	{ BP_REQ_QUIT,             { {0, DONE_PARM} } },
	
	{ BP_SYSTEM,               { {0, SYSTEM_TABLE_PARM} } },
	{ BP_USERCOMMAND,          { {0, USERCOMMAND_TABLE_PARM} } },
	
	{ BP_REQ_ACTIVATE,         { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ BP_SEND_ENCHANTMENTS,    { {1, TAG_INT}, {0, DONE_PARM} } },
	{ BP_CHANGED_STATS,               { {1, TAG_INT}, {1, TAG_INT}, {1, TAG_INT}, {1, TAG_INT},
                                       {1, TAG_INT}, {1, TAG_INT},{1, TAG_INT}, {1, TAG_INT},
                                       {1, TAG_INT}, {1, TAG_INT}, {1, TAG_INT}, {1, TAG_INT},
                                       {1, TAG_INT}, {0, DONE_PARM} } },
	
};
int num_client_msgs = sizeof(client_def_table)/sizeof(client_def_table_type);

/* the following are sent to system only, by prefixing with BP_SYSTEM (see game.c) */

client_def_table_type system_def_table[] = 
{ 
	{ BP_NEW_CHARINFO,         { {4, TAG_OBJECT}, {0, TAG_TEMP_STRING}, {0, TAG_STRING}, 
	{1, TAG_INT}, {2, LIST_RSC_PARM}, {1, TAG_INT}, {1, TAG_INT},
	{2, LIST_INT_PARM}, {2, LIST_INT_PARM}, {2, LIST_INT_PARM}, 
	{0, DONE_PARM} } },
	{ BP_SEND_CHARINFO,        { {0, DONE_PARM} } },
};
int num_system_msgs = sizeof(system_def_table)/sizeof(client_def_table_type);

client_def_table_type usercommand_def_table[] = 
{ 
	{ UC_REST,                  { {0, DONE_PARM} } },
	{ UC_STAND,                 { {0, DONE_PARM} } },
	{ UC_SUICIDE,               { {0, DONE_PARM} } },
	{ UC_REQ_PREFERENCES,       { {0, DONE_PARM} } },
	{ UC_REQ_GUILDINFO,         { {0, DONE_PARM} } },
	{ UC_SEND_PREFERENCES,      { {4, TAG_INT}, {0, DONE_PARM} } },
	{ UC_INVITE,                { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ UC_RENOUNCE,              { {0, DONE_PARM} } },
	{ UC_EXILE,                 { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ UC_ABDICATE,              { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ UC_VOTE,                  { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ UC_SET_RANK,              { {4, TAG_OBJECT}, {1, TAG_INT}, {0, DONE_PARM} } },
	{ UC_GUILD_CREATE,          { {0, TAG_STRING}, {0, TAG_STRING},
	{0, TAG_STRING}, {0, TAG_STRING}, {0, TAG_STRING},
	{0, TAG_STRING}, {0, TAG_STRING}, {0, TAG_STRING},
	{0, TAG_STRING}, {0, TAG_STRING}, {0, TAG_STRING},
	{1, TAG_INT}, {0, DONE_PARM} } },
	{ UC_GUILD_SHIELDS,         { {0, DONE_PARM} } },
	{ UC_GUILD_SHIELD,          { {0, DONE_PARM} } },
	{ UC_CLAIM_SHIELD,          { {1, TAG_INT}, {1, TAG_INT}, {1, TAG_INT}, {1, TAG_INT}, {0, DONE_PARM} } },
	{ UC_DISBAND,               { {0, DONE_PARM} } },
	{ UC_MAKE_ALLIANCE,         { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ UC_END_ALLIANCE,          { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ UC_MAKE_ENEMY,            { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ UC_END_ENEMY,             { {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ UC_REQ_GUILD_LIST,        { {0, DONE_PARM} } },
	{ UC_GUILD_RENT,            { {4, TAG_OBJECT}, {0, TAG_STRING}, {0, DONE_PARM} } },
	{ UC_ABANDON_GUILD_HALL,    { {0, DONE_PARM} } },
	{ UC_GUILD_SET_PASSWORD,    { {0, TAG_STRING}, {0, DONE_PARM} } },
	{ UC_CHANGE_URL,            { {4, TAG_OBJECT}, {0,TAG_STRING}, {0, DONE_PARM} } },
	{ UC_DEPOSIT,               { {4, TAG_INT}, {0, DONE_PARM} } },
	{ UC_BALANCE,               { {0, DONE_PARM} } },
	{ UC_WITHDRAW,              { {4, TAG_INT}, {0, DONE_PARM} } },
	{ UC_APPEAL,                { {0, TAG_STRING}, {0, DONE_PARM} } },
	{ UC_REQ_RESCUE,            { {0, DONE_PARM} } },
	{ UC_MINIGAME_STATE,        { {4, TAG_OBJECT}, {0, TAG_TEMP_STRING}, {0, DONE_PARM} } },
	{ UC_MINIGAME_RESET_PLAYERS,{ {4, TAG_OBJECT}, {0, DONE_PARM} } },
	{ UC_REQ_TIME,              { {0, DONE_PARM} } },
};
int num_usercommand_msgs = sizeof(usercommand_def_table)/sizeof(client_def_table_type);

static client_table_type user_table,system_table,usercommand_table;


void InitSprocket(void)
{
	int i,j;
	
	/* setup parse tables */
	
	for (i=0;i<CLIENT_COMMANDS_PER_TABLE;i++)
	{
		user_table[i].client_parms[0].type_parm = INVALID_PARM;
		system_table[i].client_parms[0].type_parm = INVALID_PARM;
		usercommand_table[i].client_parms[0].type_parm = INVALID_PARM;
	}
	
	for (i=0;i<num_client_msgs;i++)
		for (j=0;j<MAX_CLIENT_PARMS+1;j++)
		{
			if (client_def_table[i].client_parms[j].type_parm == DONE_PARM)
				user_table[client_def_table[i].client_cmd].num_parms = j;
			
			user_table[client_def_table[i].client_cmd].client_parms[j] =
				client_def_table[i].client_parms[j];
		}
		
		for (i=0;i<num_system_msgs;i++)
			for (j=0;j<MAX_CLIENT_PARMS+1;j++)
			{
				if (system_def_table[i].client_parms[j].type_parm == DONE_PARM)
					system_table[system_def_table[i].client_cmd].num_parms = j;
				
				system_table[system_def_table[i].client_cmd].client_parms[j] =
					system_def_table[i].client_parms[j];
			}
			
			for (i=0;i<num_usercommand_msgs;i++)
				for (j=0;j<MAX_CLIENT_PARMS+1;j++)
				{
					if (usercommand_def_table[i].client_parms[j].type_parm == DONE_PARM)
						usercommand_table[usercommand_def_table[i].client_cmd].num_parms = j;
					
					usercommand_table[usercommand_def_table[i].client_cmd].client_parms[j] =
						usercommand_def_table[i].client_parms[j];
				}
}

client_table_node *GetParseClientTable(int table_type)
{
	switch (table_type)
	{
		case SPROCKET_TABLE_USER :
			return user_table;
		case SPROCKET_TABLE_SYSTEM :
			return system_table;
		case SPROCKET_TABLE_USERCOMMAND :
			return usercommand_table;
	}
	
	return NULL;
}
