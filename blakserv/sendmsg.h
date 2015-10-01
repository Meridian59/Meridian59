// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * sendmsg.h
 *
 */

#ifndef _SENDMSG_H
#define _SENDMSG_H

#define MAX_C_PARMS 40
#define MAX_NAME_PARMS 45
#define MAX_LOCALS 50
#define MAX_BLAKOD_STATEMENTS 20000000
/* the c function id is 1 byte */
#define MAX_C_FUNCTION 256

typedef struct
{
   int num_locals;
   val_type locals[MAX_LOCALS];
} local_var_type;

typedef struct
{
	int class_id;
	int message_id;
	int propagate_depth;
	int num_parms;
   parm_node parms[MAX_NAME_PARMS];
	char *bkod_ptr;
} kod_stack_type;

typedef struct
{
   int num_interpreted;
   int num_interpreted_highest;
   int billions_interpreted;
   int num_messages;
   int num_top_level_messages;
   int system_start_time;

   double interpreting_time;
   int interpreting_time_highest;
   int interpreting_time_over_second;
   int interpreting_time_message_id;
   int interpreting_time_object_id;
   int interpreting_time_posts;
   int message_depth_highest;

   /* while interpreting stuff, this is valid */
   int interpreting_class;

   double frequency;
   double ccall_total_time[MAX_C_FUNCTION];
   /* the number of calls to each C function */
   int c_count_untimed[MAX_C_FUNCTION];
   int c_count_timed[MAX_C_FUNCTION];
} kod_statistics;

/* stuff for PostMessage queue */

#define MAX_POST_QUEUE 4000

typedef struct
{
   int object_id;
   int message_id;
   int num_parms;
   parm_node parms[MAX_NAME_PARMS];
} post_node;

typedef struct
{
   int next;
   int last;
   post_node data[MAX_POST_QUEUE];
} post_queue_type;

void InitProfiling(void);
void InitTimeProfiling(void);
void EndTimeProfiling(void);
void InitBkodInterpret(void);
kod_statistics * GetKodStats(void);
char * GetBkodPtr(void);
Bool IsInterpreting(void);

void PostBlakodMessage(int object_id,int message_id,int num_parms,parm_node parms[]);

int SendTopLevelBlakodMessage(int object_id,int message_id,int num_parms,parm_node parms[]);
int SendBlakodMessage(int object_id,int message_id,int num_parms,parm_node parms[]);
int SendBlakodClassMessage(int class_id,int message_id,int num_params,parm_node parm[]);
char *BlakodDebugInfo(void);
char *BlakodStackInfo(void);

/* this function used in sendmsg.c and ccode.c, but called all the time! */

val_type __inline RetrieveValue(int object_id,local_var_type *local_vars,int data_type,int data)
{
   object_node *o;
   class_node *c;
   val_type ret_val;

   /* This function is called very very often, so the switch has been
    * optimized away to return the value immediately. */

   if (data_type == LOCAL_VAR)
      return *(val_type *)&local_vars->locals[data].int_val;

   if (data_type == PROPERTY)
   {
      o = GetObjectByID(object_id);
      if (o == NULL)
      {
         eprintf("[%s] RetrieveValue can't find OBJECT %i\n",BlakodDebugInfo(),
            object_id);
         ret_val.int_val = NIL;
         return ret_val;
      }
      return *(val_type *)&o->p[data].val.int_val; 
   }

   if (data_type == CONSTANT)
      return *(val_type *)&data;

   if (data_type == CLASS_VAR)
   {
      o = GetObjectByID(object_id);
      if (o == NULL)
      {
         eprintf("[%s] RetrieveValue class var can't find OBJECT %i\n",
            BlakodDebugInfo(),object_id);
         ret_val.int_val = NIL;
         return ret_val;
      }

      c = GetClassByID(o->class_id);
      if (c == NULL)
      {
         eprintf("[%s] RetrieveValue can't find CLASS %i for OBJECT %i\n",
            BlakodDebugInfo(),o->class_id,object_id);
         ret_val.int_val = NIL;
         return ret_val;
      }
      if (data >= c->num_vars || data < 0)
      {
         eprintf("[%s] RetrieveValue can't retrieve invalid class var %i in OBJECT %i CLASS %s (%i)\n",
            BlakodDebugInfo(),data,object_id,c->class_name,c->class_id);
         ret_val.int_val = NIL;
         return ret_val;
      }
      ret_val = c->vars[data].val;
      if (ret_val.v.tag == TAG_OVERRIDE)
      {
         /* if it's a class var, but overridden in this class by property */
         return *(val_type *)&o->p[ret_val.v.data].val.int_val;
      }
      return ret_val;
   }

   bprintf("[%s] RetrieveValue can't identify type %i\n",
      BlakodDebugInfo(),data_type);

   ret_val.int_val = NIL;
   return ret_val;
}

#endif
