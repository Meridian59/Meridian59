// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * message.c
 *

 This module has functions to take care of a table of messages in a
 class.  The messages are read in by loadkod.c, which sets up the
 messages for a class based on the message table in the .bof file.
 The table is in the same order as the table in the .bof file.  Linear
 searches are performed to find messages.

 */

#include "blakserv.h"

/* local function prototypes */
void ResetMessageClass(class_node *c);
void SetEachClassMessagesPropagate(class_node *c);

void InitMessage()
{

}

void ResetMessage()
{
   ForEachClass(ResetMessageClass);
}

void ResetMessageClass(class_node *c)
{
   if (c->num_messages == 0)
      return;

   FreeMemory(MALLOC_ID_MESSAGE,c->messages,c->num_messages*sizeof(message_node));
   c->messages = NULL;
   c->num_messages = 0;
}

void SetClassNumMessages(int class_id,int num_messages)
{
   class_node *c;
   int i;

   c = GetClassByID(class_id);
   if (c == NULL)
   {
      eprintf("SetClassNumMessages can't find class %i\n",class_id);
      return;
   }

   if (num_messages == 0)
      return;

   c->messages = (message_node *)AllocateMemory(MALLOC_ID_MESSAGE,
						num_messages*sizeof(message_node));
   c->num_messages = num_messages;

   for (i=0;i<c->num_messages;i++)
   {
      c->messages[i].message_id = 0;
      c->messages[i].handler = 0;
      c->messages[i].dstr_id = INVALID_DSTR;
      c->messages[i].trace_session_id = INVALID_ID;
      c->messages[i].timed_call_count = 0;
      c->messages[i].untimed_call_count = 0;
      c->messages[i].total_call_time = 0.0;
      c->messages[i].propagate_message = NULL;
      c->messages[i].propagate_class = NULL;
   }  
}

void AddMessage(int class_id,int count,int message_id,char *offset,int dstr_id)
{
   class_node *c;

   /* count which message in the table we are setting */

   c = GetClassByID(class_id);
   if (c == NULL)
   {
      eprintf("AddMessage can't find class %i\n",class_id);
      return;
   }

   c->messages[count].message_id = message_id;
   c->messages[count].handler = offset;
   c->messages[count].dstr_id = dstr_id;
   c->messages[count].trace_session_id = INVALID_ID;
   c->messages[count].timed_call_count = 0;
   c->messages[count].untimed_call_count = 0;
   c->messages[count].total_call_time = 0.0;
}

/* SetMessagesPropagate
   This function goes through every class and calculates for each message
   a pointer to the executable bkod if propagated from the current one. */

void SetMessagesPropagate()
{
   ForEachClass(SetEachClassMessagesPropagate);
}

void SetEachClassMessagesPropagate(class_node *c)
{
   int i;

   /* if no superclass, then there is no propagate message */
   if (c->super_ptr == NULL)
      return;

   /* there is a parent, so look for a propagate message */
   for (i=0;i<c->num_messages;i++)
   {
      c->messages[i].propagate_message = 
	 GetMessageByID(c->super_ptr->class_id,c->messages[i].message_id,
			&c->messages[i].propagate_class);
      /*
      if (c->messages[i].propagate_message != NULL)
      {
	 dprintf("%i %i 0x%x\n",c->class_id,c->messages[i].message_id,&c->messages[i].propagate_class);
      }
      */
   }

}

message_node *GetMessageByID(int class_id,int message_id,class_node **found_class)
{
   class_node *c;
   message_node *m;
   int i;

   c = GetClassByID(class_id);

   if (c == NULL)
   {
      eprintf("GetMessageByID can't find class %i\n",class_id);
      return NULL;
   }
   
   do
   {
      m = c->messages;

      for (i=0;i<c->num_messages;i++)
      {
	 if (m[i].message_id == message_id)
	 {
	    if (found_class != NULL)
	       *found_class = c;
	    return &m[i];
	 }
      }
      c = c->super_ptr;
   } while (c != NULL);

   return NULL;
}

message_node *GetMessageByName(int class_id,char *message_name,class_node **found_class)
{
   int message_id;

   message_id = GetIDByName(message_name);
   if (message_id == INVALID_ID)
   {
      eprintf("GetMessageByName can't find name %s\n",message_name);
      return NULL;
   }
   return GetMessageByID(class_id,message_id,found_class);
}
