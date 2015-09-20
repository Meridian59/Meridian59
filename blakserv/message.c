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
 Messages are stored and accessed by hashed message ID for speed.

 */

#include "blakserv.h"

/* local function prototypes */
void ResetMessageClass(class_node *c);
void SetEachClassMessagesPropagate(class_node *c);

static int max_messages;
static int hash_collisions;

void InitMessage()
{
   // Keep track of some stats.
   max_messages = 0;
   hash_collisions = 0;
}

void ResetMessage()
{
   ForEachClass(ResetMessageClass);
   max_messages = 0;
   hash_collisions = 0;
}

void ResetMessageClass(class_node *c)
{
   message_node *m, *temp;

   if (!c->num_messages)
      return;

   for (int i = 0; i < MESSAGE_TABLE_SIZE; ++i)
   {
      m = c->messages[i];
      while (m != NULL)
      {
         temp = m->next;
         FreeMemory(MALLOC_ID_MESSAGE, m, sizeof(message_node));
         m = temp;
      }
      c->messages[i] = NULL;
   }

   FreeMemory(MALLOC_ID_MESSAGE, c->messages, MESSAGE_TABLE_SIZE * sizeof(message_node*));
   c->messages = NULL;
   c->num_messages = 0;
}

void SetClassNumMessages(int class_id, int num_messages)
{
   class_node *c;

   c = GetClassByID(class_id);
   if (c == NULL)
   {
      eprintf("SetClassNumMessages can't find class %i\n",class_id);
      return;
   }

   if (!num_messages)
      return;

   // Track highest number of messages in a class.
   if (num_messages > max_messages)
      max_messages = num_messages;

   c->num_messages = num_messages;

   c->messages = (message_node **)AllocateMemoryCalloc(MALLOC_ID_MESSAGE,
      MESSAGE_TABLE_SIZE, sizeof(message_node*));

   // Zero allocated memory so we don't need to set any defaults.
}

void AddMessage(int class_id,int count,int message_id,char *offset,int dstr_id)
{
   class_node *c;
   message_node *m;
   int hash_num;
   /* count which message in the table we are setting */

   c = GetClassByID(class_id);
   if (c == NULL)
   {
      eprintf("AddMessage can't find class %i\n",class_id);
      return;
   }

   m = (message_node *)AllocateMemory(MALLOC_ID_MESSAGE, sizeof(message_node));

   m->message_id = message_id;
   m->handler = offset;
   m->dstr_id = dstr_id;
   m->trace_session_id = INVALID_ID;
   m->timed_call_count = 0;
   m->untimed_call_count = 0;
   m->total_call_time = 0.0;

   m->next = NULL;

   hash_num = GetMessageHashNum(m->message_id);
   m->next = c->messages[hash_num];

   // Keep track of collisions.
   if (m->next != NULL)
      hash_collisions++;

   c->messages[hash_num] = m;
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
   message_node *m;

   /* if no superclass, then there is no propagate message */
   if (c->super_ptr == NULL)
      return;
   if (!c->num_messages)
      return;
   /* there is a parent, so look for a propagate message */
   for (int i = 0; i < MESSAGE_TABLE_SIZE; ++i)
   {
      m = c->messages[i];
      while (m != NULL)
      {
         m->propagate_message =
            GetMessageByID(c->super_ptr->class_id, m->message_id,
            &m->propagate_class);
         m = m->next;
      }
   }
}

message_node *GetMessageByID(int class_id, int message_id, class_node **found_class)
{
   class_node *c;
   message_node *m;

   c = GetClassByID(class_id);

   if (c == NULL)
   {
      eprintf("GetMessageByID can't find class %i\n", class_id);
      return NULL;
   }

   do
   {
      if (c->num_messages)
      {
         m = c->messages[GetMessageHashNum(message_id)];

         while (m != NULL)
         {
            if (m->message_id == message_id)
            {
               if (found_class != NULL)
                  *found_class = c;
               return m;
            }
            m = m->next;
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

// Returns the largest number of messages in a class.
int GetHighestMessageCount()
{
   return max_messages;
}

// Returns the number of message hash collisions when loading game data.
int GetNumMessageHashCollisions()
{
   return hash_collisions;
}
