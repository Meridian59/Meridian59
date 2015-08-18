// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * parsecli.h
 *
 */

#ifndef _PARSECLI_H
#define _PARSECLI_H

#define MAX_CLIENT_PARMS 20

#define DONE_PARM -1
#define INVALID_PARM -2
#define SYSTEM_TABLE_PARM -3
#define USERCOMMAND_TABLE_PARM -4

#define LIST_PARM ((MAX_TAG)+1) /* or greater */
#define LIST_RSC_PARM ((MAX_TAG)+1)
#define LIST_OBJ_PARM ((MAX_TAG)+2)
#define LIST_INT_PARM ((MAX_TAG)+3)

typedef struct
{
   int len_parm;
   int type_parm;
} client_parm;

#define CLIENT_COMMANDS_PER_TABLE 256
/* each table is indexed by the first byte of command (0-255), which is
   an arry of the parameters */
typedef struct
{
   int num_parms;
   unsigned int call_count;
   client_parm client_parms[MAX_CLIENT_PARMS+1];
} client_table_node;

typedef client_table_node client_table_type[CLIENT_COMMANDS_PER_TABLE];

#define SPROCKET_TABLE_USER 1
#define SPROCKET_TABLE_SYSTEM 2
#define SPROCKET_TABLE_USERCOMMAND 3

void InitParseClient(void);
void AllocateParseClientListNodes(void); /* call after garbage collecting */

void GameMessageCount(unsigned char message_type);

void ClientToBlakodUser(session_node *session,int msg_len,char *msg_data);

void LoadParseClientTables(void);
void ResetParseClientTables(void);

#endif
