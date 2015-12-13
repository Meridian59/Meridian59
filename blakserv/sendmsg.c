// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * sendmsg.c
 *

  This module interprets compiled Blakod.
  
*/

#include "blakserv.h"

/* global debugging and profiling information */

/* stuff to calculate messages & times */
int message_depth = 0;
/* stack has class and bkod ptr for each frame. For current frame, bkod_ptr
   is as of the beginning of the function call */
kod_stack_type stack[MAX_DEPTH];

kod_statistics kod_stat;      /* actual statistics */

char *bkod;
int num_interpreted = 0; /* number of instructions in this top level call */

int trace_session_id = INVALID_ID;

post_queue_type post_q;

// Structs for unary/binary op to read data quicker. Can't include the
// info byte as the alignment will be incorrect. Forcing alignment with
// the extra byte generates slower code.
typedef struct
{
   bkod_type dest;
   bkod_type source;
} unopdata_node;

typedef struct
{
   bkod_type dest;
   bkod_type source1;
   bkod_type source2;
} binopdata_node;

/* return values for InterpretAtMessage */
enum
{
   RETURN_NONE = 0,
   RETURN_PROPAGATE = 1,
   RETURN_NO_PROPAGATE = 2,
};

/* table of pointers to functions to call for ccode functions */

typedef int (*ccall_proc)(int object_id,local_var_type *local_vars,
                    int num_normal_parms,parm_node normal_parm_array[],
                    int num_name_parms,parm_node name_parm_array[]);

ccall_proc ccall_table[MAX_C_FUNCTION];

int done;

/* local function prototypes */
int InterpretAtMessage(int object_id,class_node* c,message_node* m,
                  int num_sent_parms,parm_node sent_parms[],
                  val_type *ret_val);
__inline void StoreValue(int object_id,local_var_type *local_vars,int data_type,int data,
                   val_type new_data);
void InterpretUnaryAssign(int object_id,local_var_type *local_vars,opcode_type opcode);
void InterpretBinaryAssign(int object_id,local_var_type *local_vars,opcode_type opcode);
void InterpretGoto(int object_id, local_var_type *local_vars, opcode_type opcode);
void InterpretCall(int object_id,local_var_type *local_vars,opcode_type opcode);

void InitProfiling(void)
{
   int i;

   if (done)
      return;

   kod_stat.num_interpreted = 0;
   kod_stat.num_interpreted_highest = 0;
   kod_stat.billions_interpreted = 0;
   kod_stat.num_messages = 0;
   kod_stat.num_top_level_messages = 0;
   kod_stat.system_start_time = GetTime();
   kod_stat.interpreting_time = 0.0;
   kod_stat.interpreting_time_highest = 0;
   kod_stat.interpreting_time_over_second = 0;
   kod_stat.interpreting_time_message_id = INVALID_ID;
   kod_stat.interpreting_time_object_id = INVALID_ID;
   kod_stat.interpreting_time_posts = 0;
   kod_stat.message_depth_highest = 0;
   kod_stat.interpreting_class = INVALID_CLASS;

   for (i = 0; i < MAX_C_FUNCTION; i++)
      kod_stat.c_count_untimed[i] = 0;
   for (i = 0; i < MAX_C_FUNCTION; i++)
      kod_stat.c_count_timed[i] = 0;
   for (i = 0; i < MAX_C_FUNCTION; i++)
      kod_stat.ccall_total_time[i] = 0;

   message_depth = 0;

   if (ConfigBool(DEBUG_TIME_CALLS))
      InitTimeProfiling();
   else
      kod_stat.debugtime = false;

   done = 1;
}

void InitTimeProfiling(void)
{
   kod_stat.debugtime = true;
}

void EndTimeProfiling(void)
{
   kod_stat.debugtime = false;
}

void InitBkodInterpret(void)
{
   int i;

   bkod = NULL;

   post_q.next = 0;
   post_q.last = 0;
   
   for (i=0;i<MAX_C_FUNCTION;i++)
      ccall_table[i] = C_Invalid;
   
   ccall_table[CREATEOBJECT] = C_CreateObject;
   ccall_table[ISCLASS] = C_IsClass; 
   ccall_table[GETCLASS] = C_GetClass;
   
   ccall_table[SENDMESSAGE] = C_SendMessage;
   ccall_table[POSTMESSAGE] = C_PostMessage;
   ccall_table[SENDLISTMSG] = C_SendListMessage;
   ccall_table[SENDLISTMSGBREAK] = C_SendListMessageBreak;
   ccall_table[SENDLISTMSGBYCLASS] = C_SendListMessageByClass;
   ccall_table[SENDLISTMSGBYCLASSBREAK] = C_SendListMessageByClassBreak;

   ccall_table[SAVEGAME] = C_SaveGame;
   ccall_table[LOADGAME] = C_LoadGame;

   ccall_table[ADDPACKET] = C_AddPacket;
   ccall_table[SENDPACKET] = C_SendPacket;
   ccall_table[SENDCOPYPACKET] = C_SendCopyPacket;
   ccall_table[CLEARPACKET] = C_ClearPacket;
   ccall_table[GODLOG] = C_GodLog;
   ccall_table[DEBUG] = C_Debug;
   ccall_table[GETINACTIVETIME] = C_GetInactiveTime;
   ccall_table[DUMPSTACK] = C_DumpStack;

   ccall_table[STRINGEQUAL] = C_StringEqual;
   ccall_table[STRINGCONTAIN] = C_StringContain;
   ccall_table[SETRESOURCE] = C_SetResource;
   ccall_table[PARSESTRING] = C_ParseString;
   ccall_table[SETSTRING] = C_SetString;
   ccall_table[APPENDTEMPSTRING] = C_AppendTempString;
   ccall_table[CLEARTEMPSTRING] = C_ClearTempString;
   ccall_table[GETTEMPSTRING] = C_GetTempString;
   ccall_table[CREATESTRING] = C_CreateString;
   ccall_table[ISSTRING] = C_IsString;
   ccall_table[STRINGSUBSTITUTE] = C_StringSubstitute;
   ccall_table[STRINGLENGTH] = C_StringLength;
   ccall_table[STRINGCONSISTSOF] = C_StringConsistsOf;
   
   ccall_table[CREATETIMER] = C_CreateTimer;
   ccall_table[DELETETIMER] = C_DeleteTimer;
   ccall_table[GETTIMEREMAINING] = C_GetTimeRemaining;
   ccall_table[ISTIMER] = C_IsTimer;
   ccall_table[CREATEROOMDATA] = C_LoadRoom;
   ccall_table[FREEROOM] = C_FreeRoom;
   ccall_table[ROOMDATA] = C_RoomData;
   ccall_table[LINEOFSIGHTVIEW] = C_LineOfSightView;
   ccall_table[LINEOFSIGHTBSP] = C_LineOfSightBSP;
   ccall_table[CANMOVEINROOMBSP] = C_CanMoveInRoomBSP;
   ccall_table[CHANGETEXTUREBSP] = C_ChangeTextureBSP;
   ccall_table[MOVESECTORBSP] = C_MoveSectorBSP;
   ccall_table[GETLOCATIONINFOBSP] = C_GetLocationInfoBSP;
   ccall_table[BLOCKERADDBSP] = C_BlockerAddBSP;
   ccall_table[BLOCKERMOVEBSP] = C_BlockerMoveBSP;
   ccall_table[BLOCKERREMOVEBSP] = C_BlockerRemoveBSP;
   ccall_table[BLOCKERCLEARBSP] = C_BlockerClearBSP;
   ccall_table[GETRANDOMPOINTBSP] = C_GetRandomPointBSP;
   ccall_table[GETSTEPTOWARDSBSP] = C_GetStepTowardsBSP;

   ccall_table[APPENDLISTELEM] = C_AppendListElem;
   ccall_table[CONS] = C_Cons;
   ccall_table[FIRST] = C_First;
   ccall_table[REST] = C_Rest;
   ccall_table[LENGTH] = C_Length;
   ccall_table[LAST] = C_Last;
   ccall_table[NTH] = C_Nth;
   ccall_table[MLIST] = C_List;
   ccall_table[ISLIST] = C_IsList;
   ccall_table[SETFIRST] = C_SetFirst;
   ccall_table[SETNTH] = C_SetNth;
   ccall_table[SWAPLISTELEM] = C_SwapListElem;
   ccall_table[INSERTLISTELEM] = C_InsertListElem;
   ccall_table[DELLISTELEM] = C_DelListElem;
   ccall_table[FINDLISTELEM] = C_FindListElem;
   ccall_table[ISLISTMATCH] = C_IsListMatch;
   ccall_table[GETLISTELEMBYCLASS] = C_GetListElemByClass;
   ccall_table[GETLISTNODE] = C_GetListNode;
   ccall_table[GETALLLISTNODESBYCLASS] = C_GetAllListNodesByClass;
   ccall_table[LISTCOPY] = C_ListCopy;

   ccall_table[GETTIMEZONEOFFSET] = C_GetTimeZoneOffset;
   ccall_table[GETTIME] = C_GetTime;
   ccall_table[GETTICKCOUNT] = C_GetTickCount;
   ccall_table[SETCLASSVAR] = C_SetClassVar;
   
   ccall_table[CREATETABLE] = C_CreateTable;
   ccall_table[ADDTABLEENTRY] = C_AddTableEntry;
   ccall_table[GETTABLEENTRY] = C_GetTableEntry;
   ccall_table[DELETETABLEENTRY] = C_DeleteTableEntry;
   ccall_table[DELETETABLE] = C_DeleteTable;
   ccall_table[ISTABLE] = C_IsTable;

   ccall_table[ISOBJECT] = C_IsObject;
   
   ccall_table[RECYCLEUSER] = C_RecycleUser;
   
   ccall_table[RANDOM] = C_Random;
   
   ccall_table[RECORDSTAT] = C_RecordStat;
   
   ccall_table[GETSESSIONIP] = C_GetSessionIP;
   
   ccall_table[ABS] = C_Abs;
   ccall_table[BOUND] = C_Bound;
   ccall_table[SQRT] = C_Sqrt;

   ccall_table[MINIGAMENUMBERTOSTRING] = C_MinigameNumberToString;
   ccall_table[MINIGAMESTRINGTONUMBER] = C_MinigameStringToNumber;
}

kod_statistics * GetKodStats()
{
   return &kod_stat;
}

/* this pointer only makes sense when interpreting (used by bprintf only) */

char * GetBkodPtr(void)
{
   return bkod;
}

/* used by object.c to see if creation of object should call
SendTopLevelBlakodMessage or SendBlakodMessage */
Bool IsInterpreting(void)
{
   return bkod != NULL;
}

void TraceInfo(int session_id,char *class_name,int message_id,int num_parms,
               parm_node parms[])
{
   int i;
   val_type val;
   char trace_buf[BUFFER_SIZE];
   char *buf_ptr = trace_buf;
   int num_chars;
   int buf_len = 0;

   if (num_parms == 0)
      num_chars = sprintf(buf_ptr, "%-15s%-20s\n", class_name, GetNameByID(message_id));
   else
      num_chars = sprintf(buf_ptr, "%-15s%-20s ", class_name, GetNameByID(message_id));
   buf_len += num_chars;
   buf_ptr += num_chars;

   for (i = 0; i < num_parms; ++i)
   {
      val.int_val = parms[i].value;

      if (i == num_parms - 1)
         num_chars = sprintf(buf_ptr, "%s = %s %s\n", GetNameByID(parms[i].name_id),
                        GetTagName(val), GetDataName(val));
      else
         num_chars = sprintf(buf_ptr, "%s = %s %s, ", GetNameByID(parms[i].name_id),
                        GetTagName(val), GetDataName(val));
      buf_len += num_chars;
      buf_ptr += num_chars;

      // Flushing shouldn't be necessary here, but just in case.
      if (buf_len > BUFFER_SIZE * 0.9)
      {
         buf_ptr -= buf_len;
         SendSessionAdminText(session_id, buf_ptr);
         buf_len = 0;
      }
   }
   buf_ptr -= buf_len;
   SendSessionAdminText(session_id, buf_ptr);
}

void PostBlakodMessage(int object_id,int message_id,int num_parms,parm_node parms[])
{
   int i, new_next;

   new_next = (post_q.next + 1) % MAX_POST_QUEUE;
   if (new_next == post_q.last)
   {
      bprintf("PostBlakodMessage can't post MESSAGE %s (%i) to OBJECT %i; queue filled\n",
         GetNameByID(message_id),message_id,object_id);
      return;
   }
   post_q.data[post_q.next].object_id = object_id;
   post_q.data[post_q.next].message_id = message_id;
   post_q.data[post_q.next].num_parms = num_parms;
   for (i=0;i<num_parms;i++)
   {
      post_q.data[post_q.next].parms[i] = parms[i];
   }
   post_q.next = new_next;
}

/* returns the return value of the blakod */
int SendTopLevelBlakodMessage(int object_id,int message_id,int num_parms,parm_node parms[])
{
   int ret_val = 0;
   double start_time = 0;
   double interp_time = 0;
   int posts = 0;
   int accumulated_num_interpreted = 0;

   if (message_depth != 0)
   {
      eprintf("SendTopLevelBlakodMessage called with message_depth %i "
         "and message id %i\n", message_depth,message_id);
   }

   start_time = GetMicroCountDouble();
   kod_stat.num_top_level_messages++;
   trace_session_id = INVALID_ID;
   num_interpreted = 0;

   ret_val = SendBlakodMessage(object_id,message_id,num_parms,parms);

   while (post_q.next != post_q.last)
   {
      posts++;

      accumulated_num_interpreted += num_interpreted;
      num_interpreted = 0;

      if (accumulated_num_interpreted > 10 * MAX_BLAKOD_STATEMENTS)
      {
         bprintf("SendTopLevelBlakodMessage too many instructions in posted followups\n");
         
         dprintf("SendTopLevelBlakodMessage too many instructions in posted followups\n");
         dprintf("  OBJECT %i CLASS %s MESSAGE %s (%i) some followups are being aborted\n",
            object_id,
            GetClassByID(GetObjectByID(object_id)->class_id)->class_name,
            GetNameByID(message_id), message_id);
         
         break;
      }

      /* posted messages' return value is ignored */
      SendBlakodMessage(post_q.data[post_q.last].object_id,post_q.data[post_q.last].message_id,
         post_q.data[post_q.last].num_parms,post_q.data[post_q.last].parms);

      post_q.last = (post_q.last + 1) % MAX_POST_QUEUE;
   }

   interp_time = GetMicroCountDouble() - start_time;
   kod_stat.interpreting_time += interp_time;
   if (interp_time > kod_stat.interpreting_time_highest)
   {
      kod_stat.interpreting_time_highest = (int)interp_time;
      kod_stat.interpreting_time_message_id = message_id;
      kod_stat.interpreting_time_object_id = object_id;
      kod_stat.interpreting_time_posts = posts;
   }
   if (interp_time > 1000000.0)
   {
      kod_stat.interpreting_time_over_second++;
      kod_stat.interpreting_time_message_id = message_id;
      kod_stat.interpreting_time_object_id = object_id;
      kod_stat.interpreting_time_posts = posts;
   }

   if (num_interpreted > kod_stat.num_interpreted_highest)
      kod_stat.num_interpreted_highest = num_interpreted;
   
   kod_stat.num_interpreted += num_interpreted;
   if (kod_stat.num_interpreted > 1000000000L)
   {
      kod_stat.num_interpreted -= 1000000000L;
      kod_stat.billions_interpreted++;
   }

   if (message_depth != 0)
   {
      eprintf("SendTopLevelBlakodMessage returning with message_depth %i "
         "and message id %i\n", message_depth, message_id);
   }

   return ret_val;
}

typedef struct {
   int class_id;
   int message_id;
   int num_params;
   parm_node *parm;
} ClassMessage, *PClassMessage;

static ClassMessage classMsg;
static int numExecuted;

void SendClassMessage(object_node *object)
{
   class_node *c = GetClassByID(object->class_id);
   do
   {
      if (c->class_id == classMsg.class_id)
      {
         SendBlakodMessage(object->object_id,classMsg.message_id,
                           classMsg.num_params,classMsg.parm);
         numExecuted++;
         return;
      }
      c = c->super_ptr;
   } while (c != NULL);
}

int SendBlakodClassMessage(int class_id,int message_id,int num_params,parm_node parm[])
{
   numExecuted = 0;
   classMsg.class_id = class_id;
   classMsg.message_id = message_id;
   classMsg.num_params = num_params;
   classMsg.parm = parm;
   ForEachObject(SendClassMessage);
   return numExecuted;
}

/* returns the return value of the blakod */
int SendBlakodMessage(int object_id,int message_id,int num_parms,parm_node parms[])
{
   object_node *o;
   class_node *c,*propagate_class;
   message_node *m;
   val_type message_ret;
   
   int prev_interpreting_class;
   char *prev_bkod;

   int propagate_depth = 0;

   prev_bkod = bkod;
   prev_interpreting_class = kod_stat.interpreting_class;

   o = GetObjectByID(object_id);
   if (o == NULL)
   {
      bprintf("SendBlakodMessage can't find OBJECT %i\n",object_id);
      return NIL;
   }

   c = GetClassByID(o->class_id);
   if (c == NULL)
   {
      eprintf("SendBlakodMessage OBJECT %i can't find CLASS %i\n",
         object_id,o->class_id);
      return NIL;
   }

   m = GetMessageByID(c->class_id,message_id,&c);

   if (m == NULL)
   {
      bprintf("SendBlakodMessage CLASS %s (%i) OBJECT %i can't find a handler for MESSAGE %s (%i)\n",
         c->class_name,c->class_id,object_id,GetNameByID(message_id),message_id);
      return NIL;
   }

   kod_stat.num_messages++;
   stack[message_depth].class_id = c->class_id;
   stack[message_depth].message_id = m->message_id;
   stack[message_depth].propagate_depth = 0;
   stack[message_depth].num_parms = num_parms;
   memcpy(stack[message_depth].parms,parms,num_parms*sizeof(parm_node));
   stack[message_depth].bkod_ptr = bkod;
   if (message_depth > 0)
      stack[message_depth-1].bkod_ptr = prev_bkod;
   message_depth++;
   if (message_depth > kod_stat.message_depth_highest)
      kod_stat.message_depth_highest = message_depth;

   if (message_depth >= MAX_DEPTH)
   {
      bprintf("SendBlakodMessage sending to CLASS %s (%i), depth is %i, aborted!\n",
         c->class_name,c->class_id,message_depth);
      
      kod_stat.interpreting_class = prev_interpreting_class;
      message_depth--;
      bkod = prev_bkod;

      return NIL;
   }

   if (m->trace_session_id != INVALID_ID)
   {
      trace_session_id = m->trace_session_id;
      m->trace_session_id = INVALID_ID;
   }

   if (trace_session_id != INVALID_ID)
      TraceInfo(trace_session_id,c->class_name,m->message_id,num_parms,parms);

   kod_stat.interpreting_class = c->class_id;

   bkod = m->handler;

   propagate_depth = 1;

   while (InterpretAtMessage(object_id,c,m,num_parms,parms,&message_ret) 
      == RETURN_PROPAGATE)
   {
      propagate_class = m->propagate_class;
      m = m->propagate_message;

      if (m == NULL)
      {
         bprintf("SendBlakodMessage can't propagate MESSAGE %s (%i) in CLASS %s (%i)\n",
            GetNameByID(message_id),message_id,c->class_name,c->class_id);
         message_depth -= propagate_depth;
         kod_stat.interpreting_class = prev_interpreting_class;
         bkod = prev_bkod;
         return NIL;
      }

      if (propagate_class == NULL)
      {
         bprintf("SendBlakodMessage can't find class to propagate to, from "
            "MESSAGE %s (%i) in CLASS %s (%i)\n",GetNameByID(message_id),
            message_id,c->class_name,c->class_id);
         message_depth -= propagate_depth;
         kod_stat.interpreting_class = prev_interpreting_class;
         bkod = prev_bkod;
         return NIL;
      }

      c = propagate_class;

      if (m->trace_session_id != INVALID_ID)
      {
         trace_session_id = m->trace_session_id;
         m->trace_session_id = INVALID_ID;
      }

      if (trace_session_id != INVALID_ID)
         TraceInfo(trace_session_id,"(propagate)",m->message_id,num_parms,parms);

      kod_stat.interpreting_class = c->class_id;

      stack[message_depth-1].bkod_ptr = bkod;

      stack[message_depth].class_id = c->class_id;
      stack[message_depth].message_id = m->message_id;
      stack[message_depth].propagate_depth = propagate_depth;
      stack[message_depth].num_parms = num_parms;
      memcpy(stack[message_depth].parms,parms,num_parms*sizeof(parm_node));
      stack[message_depth].bkod_ptr = m->handler;
      message_depth++;
      propagate_depth++;

      bkod = m->handler;
   }

   message_depth -= propagate_depth;
   kod_stat.interpreting_class = prev_interpreting_class;
   bkod = prev_bkod;

   return message_ret.int_val;
}

/* interpret code below here */

#define get_byte() (*bkod++)

__inline unsigned int get_int()
{
   bkod += 4;
   return *((unsigned int *)(bkod-4));
}

/* before calling this, you MUST set bkod to point to valid bkod. */

/* returns either RETURN_PROPAGATE or RETURN_NO_PROPAGATE.  If no propagate,
* then the return value in ret_val is good.
*/
int InterpretAtMessage(int object_id,class_node* c,message_node* m,
                  int num_sent_parms,
                  parm_node sent_parms[],val_type *ret_val)
{
   int parm_id;
   int i, j;
   double startTime;
   val_type parm_init_value;
   local_var_type local_vars;
   char num_locals, num_parms;
   Bool found_parm;

   // Time messages.
   if (kod_stat.debugtime)
      startTime = GetMicroCountDouble();

   num_locals = get_byte();
   num_parms = get_byte();

   local_vars.num_locals = num_locals + num_parms;

   if (local_vars.num_locals > MAX_LOCALS)
   {
      dprintf("InterpretAtMessage found too many locals and parms for OBJECT %i CLASS %s MESSAGE %s (%s) aborting and returning NIL\n",
         object_id,
         c? c->class_name : "(unknown)",
         m? GetNameByID(m->message_id) : "(unknown)",
         BlakodDebugInfo());
      (*ret_val).int_val = NIL;
      return RETURN_NO_PROPAGATE;
   }

   /* both table and call parms are sorted */

   j = 0;
   i = 0;

   for (;i<num_parms;i++)
   {
      parm_id = get_int(); /* match this with parameters */
      parm_init_value.int_val = get_int();

      /* look if we have a value for this parm */
      found_parm = False;
      j = 0;         /* don't assume sorted for now */
      while (j < num_sent_parms)
      {
         if (sent_parms[j].name_id == parm_id)
         {
         /* assuming no RetrieveValue needed here, since InterpretCall
            does that for us */
            local_vars.locals[i].int_val = sent_parms[j].value;
            found_parm = True;
            j++;
            break;
         }
         j++;
      }

      if (!found_parm)
         local_vars.locals[i].int_val = parm_init_value.int_val;
   }

   // Init all non-parm locals to NIL
   for (;i < local_vars.num_locals; ++i)
      local_vars.locals[i].int_val = NIL;

   for(;;)         /* returns when gets a blakod return */
   {
      /* infinite loop check */
      if (++num_interpreted > MAX_BLAKOD_STATEMENTS)
      {
         bprintf("InterpretAtMessage interpreted too many instructions--infinite loop?\n");

         dprintf("Infinite loop at depth %i\n", message_depth);
         dprintf("  OBJECT %i CLASS %s MESSAGE %s (%s) aborting and returning NIL\n",
            object_id,
            c? c->class_name : "(unknown)",
            m? GetNameByID(m->message_id) : "(unknown)",
            BlakodDebugInfo());

         dprintf("  Local variables:\n");
         for (i=0;i<local_vars.num_locals;i++)
         {
            dprintf("  %3i : %s %5i\n", i,
               GetTagName(local_vars.locals[i]),
               local_vars.locals[i].v.data);
         }

         (*ret_val).int_val = NIL;
         return RETURN_NO_PROPAGATE;
      }

      // Get the opcode for this operation
      opcode_type *opcode = (opcode_type *)bkod++;

      /* use continues instead of breaks here since there is nothing
      after the switch, for efficiency */

      switch (opcode->command)
      {
         case UNARY_ASSIGN : 
            InterpretUnaryAssign(object_id,&local_vars,*opcode);
            continue;
         case BINARY_ASSIGN : 
            InterpretBinaryAssign(object_id, &local_vars, *opcode);
            continue;
         case GOTO : 
            InterpretGoto(object_id, &local_vars, *opcode);
            continue;
         case CALL : 
            InterpretCall(object_id, &local_vars, *opcode);
            continue;
         case RETURN :
            if (kod_stat.debugtime)
            {
               m->total_call_time += (GetMicroCountDouble() - startTime);
               m->timed_call_count++;
            }
            else
            {
               m->untimed_call_count++;
            }
            if (opcode->dest == PROPAGATE)
               return RETURN_PROPAGATE;
            else
            {
               int data;
               data = get_int();
               *ret_val = RetrieveValue(object_id, &local_vars, opcode->source1, data);
               return RETURN_NO_PROPAGATE;
            }
            /* can't get here */
               continue;
         default : 
            bprintf("InterpretAtMessage found INVALID OPCODE command %i.  die.\n",
               opcode->command);
            FlushDefaultChannels();
            continue;
      }
   }
}

/* RetrieveValue used to be here, but is inline, and used in ccode.c too, so it's
in sendmsg.h now */

__inline void StoreValue(int object_id, local_var_type *local_vars,
                         int data_type, int data, val_type new_data)
{
   object_node *o;

   switch (data_type)
   {
   case LOCAL_VAR : 
      if (data < 0 || data >= local_vars->num_locals)
      {
         eprintf("[%s] StoreValue can't write to illegal local var %i\n",
            BlakodDebugInfo(),data);
         return;
      }
      local_vars->locals[data].int_val = new_data.int_val;
      break;

   case PROPERTY : 
      o = GetObjectByID(object_id);
      if (o == NULL)
      {
         eprintf("[%s] StoreValue can't find object %i\n",
            BlakodDebugInfo(),object_id);
         return;
      }
      // num_props includes self, so the max property is stored at [num_props - 1]
      if (data < 0 || data >= o->num_props)
      {
         eprintf("[%s] StoreValue can't write to illegal property %i (max %i)\n",
            BlakodDebugInfo(), data, o->num_props - 1);
         return;
      }
      o->p[data].val.int_val = new_data.int_val; 
      break;

   default :
      eprintf("[%s] StoreValue can't identify type %i\n",
         BlakodDebugInfo(),data_type); 
      break;
   }
}

void InterpretUnaryAssign(int object_id,local_var_type *local_vars,opcode_type opcode)
{
   char info = get_byte();
   unopdata_node *opnode = (unopdata_node*)((bkod += sizeof(unopdata_node))
      - sizeof(unopdata_node));

   val_type source_data = RetrieveValue(object_id, local_vars, opcode.source1, opnode->source);

   switch (info)
   {
   case NOT : 
      if (source_data.v.tag != TAG_INT)
      {
         bprintf("InterpretUnaryAssign can't not non-int %i,%i\n",
            source_data.v.tag,source_data.v.data);
         break;
      }
      source_data.v.data = !source_data.v.data;
      break;
   case NEGATE :
      if (source_data.v.tag != TAG_INT)
      {
         bprintf("InterpretUnaryAssign can't negate non-int %i,%i\n",
            source_data.v.tag,source_data.v.data);
         break;
      }
      source_data.v.data = -source_data.v.data;
      break;
   case NONE :
      break;
   case BITWISE_NOT :
      if (source_data.v.tag != TAG_INT)
      {
         bprintf("InterpretUnaryAssign can't bitwise not non-int %i,%i\n",
            source_data.v.tag,source_data.v.data);
         break;
      }
      source_data.v.data = ~source_data.v.data;
      break;
   case POST_INCREMENT:
      if (source_data.v.tag != TAG_INT)
      {
         bprintf("InterpretUnaryAssign can't post-increment non-int %i,%i\n",
            source_data.v.tag,source_data.v.data);
         break;
      }
      if (opnode->source != opnode->dest)
         StoreValue(object_id, local_vars, opcode.dest, opnode->dest, source_data);
      ++source_data.v.data;
      StoreValue(object_id, local_vars, opcode.dest, opnode->source, source_data);
      return;
   case PRE_INCREMENT:
      if (source_data.v.tag != TAG_INT)
      {
         bprintf("InterpretUnaryAssign can't pre-increment non-int %i,%i\n",
            source_data.v.tag, source_data.v.data);
         break;
      }
      ++source_data.v.data;
      if (opnode->source != opnode->dest)
         StoreValue(object_id, local_vars, opcode.dest, opnode->source, source_data);
      break;
   case POST_DECREMENT :
      if (source_data.v.tag != TAG_INT)
      {
         bprintf("InterpretUnaryAssign can't post-decrement non-int %i,%i\n",
            source_data.v.tag,source_data.v.data);
         break;
      }
      if (opnode->source != opnode->dest)
         StoreValue(object_id, local_vars, opcode.dest, opnode->dest, source_data);
      --source_data.v.data;
      StoreValue(object_id, local_vars, opcode.dest, opnode->source, source_data);
      return;
   case PRE_DECREMENT:
      if (source_data.v.tag != TAG_INT)
      {
         bprintf("InterpretUnaryAssign can't pre-decrement non-int %i,%i\n",
            source_data.v.tag, source_data.v.data);
         break;
      }
      --source_data.v.data;
      if (opnode->source != opnode->dest)
         StoreValue(object_id, local_vars, opcode.dest, opnode->source, source_data);
      break;
   default :
      bprintf("InterpretUnaryAssign can't perform unary op %i\n",info);
      break;
   }

   StoreValue(object_id, local_vars, opcode.dest, opnode->dest, source_data);
}

void InterpretBinaryAssign(int object_id,local_var_type *local_vars,opcode_type opcode)
{
   val_type source1_data,source2_data;
   char info = get_byte();
   binopdata_node *opnode = (binopdata_node*)((bkod += sizeof(binopdata_node))
      - sizeof(binopdata_node));

   source1_data = RetrieveValue(object_id,local_vars,opcode.source1, opnode->source1);
   source2_data = RetrieveValue(object_id, local_vars, opcode.source2, opnode->source2);

   /*
   if (source1_data.v.tag != source2_data.v.tag)
   bprintf("InterpretBinaryAssign is operating on 2 diff types!\n");
   */
   switch (info)
   {
   case ADD : 
      if (source1_data.v.tag != TAG_INT || source2_data.v.tag != TAG_INT)
      {
         bprintf("InterpretBinaryAssign can't add 2 vars %i,%i and %i,%i\n",
            source1_data.v.tag,source1_data.v.data,
            source2_data.v.tag,source2_data.v.data);
         break;
      }
      source1_data.v.data += source2_data.v.data;
      break;
   case SUBTRACT :
      if (source1_data.v.tag != TAG_INT || source2_data.v.tag != TAG_INT)
      {
         bprintf("InterpretBinaryAssign can't sub 2 vars %i,%i and %i,%i\n",
            source1_data.v.tag,source1_data.v.data,
            source2_data.v.tag,source2_data.v.data);
         break;
      }
      source1_data.v.data -= source2_data.v.data;
      break;
   case MULTIPLY :
      if (source1_data.v.tag != TAG_INT || source2_data.v.tag != TAG_INT)
      {
         bprintf("InterpretBinaryAssign can't mult 2 vars %i,%i and %i,%i\n",
            source1_data.v.tag,source1_data.v.data,
            source2_data.v.tag,source2_data.v.data);
         break;
      }
      source1_data.v.data *= source2_data.v.data;
      break;
   case DIV :
      if (source1_data.v.tag != TAG_INT || source2_data.v.tag != TAG_INT)
      {
         bprintf("InterpretBinaryAssign can't div 2 vars %i,%i and %i,%i\n",
            source1_data.v.tag,source1_data.v.data,
            source2_data.v.tag,source2_data.v.data);
         break;
      }
      if (source2_data.v.data == 0)
      {
         bprintf("InterpretBinaryAssign can't div by 0\n");
         break;
      }
      source1_data.v.data /= source2_data.v.data;
      break;
   case MOD :
      if (source1_data.v.tag != TAG_INT || source2_data.v.tag != TAG_INT)
      {
         bprintf("InterpretBinaryAssign can't mod 2 vars %i,%i and %i,%i\n",
            source1_data.v.tag,source1_data.v.data,
            source2_data.v.tag,source2_data.v.data);
         break;
      }
      if (source2_data.v.data == 0)
      {
         bprintf("InterpretBinaryAssign can't mod 0\n");
         break;
      }
      source1_data.v.data = abs(source1_data.v.data % source2_data.v.data);
      break;
   case AND :
      if (source1_data.v.tag != TAG_INT || source2_data.v.tag != TAG_INT)
      {
         bprintf("InterpretBinaryAssign can't and 2 vars %i,%i and %i,%i\n",
            source1_data.v.tag,source1_data.v.data,
            source2_data.v.tag,source2_data.v.data);
         break;
      }
      source1_data.v.data = source1_data.v.data && source2_data.v.data;
      break;
   case OR :
      if (source1_data.v.tag != TAG_INT || source2_data.v.tag != TAG_INT)
      {
         bprintf("InterpretBinaryAssign can't or 2 vars %i,%i and %i,%i\n",
            source1_data.v.tag,source1_data.v.data,
            source2_data.v.tag,source2_data.v.data);
         break;
      }
      source1_data.v.data = source1_data.v.data || source2_data.v.data;
      break;
   case EQUAL :
#if 0
      // disabled:  used to be only TAG_NIL vs TAG_X, or TAG_X vs TAG_X is legal
      // now:  TAG_X vs TAG_Y is legal, and returns FALSE for equal
      
      if (source1_data.v.tag != source2_data.v.tag &&
         source1_data.v.tag != TAG_NIL && source2_data.v.tag != TAG_NIL)
      {
         bprintf("InterpretBinaryAssign can't = 2 vars %i,%i and %i,%i\n",
            source1_data.v.tag,source1_data.v.data,
            source2_data.v.tag,source2_data.v.data);
         break;
      }
#endif
      if (source1_data.v.tag != source2_data.v.tag)
         source1_data.v.data = False;
      else
         source1_data.v.data = source1_data.v.data == source2_data.v.data;
      source1_data.v.tag = TAG_INT;
      break;

   case NOT_EQUAL :
#if 0
      // disabled:  used to be only TAG_NIL vs TAG_X, or TAG_X vs TAG_X is legal
      // now:  TAG_X vs TAG_Y is legal, and returns TRUE for not equal
      
      if (source1_data.v.tag != source2_data.v.tag &&
         source1_data.v.tag != TAG_NIL && source2_data.v.tag != TAG_NIL)
      {
         bprintf("InterpretBinaryAssign can't <> 2 vars %i,%i and %i,%i\n",
            source1_data.v.tag,source1_data.v.data,
            source2_data.v.tag,source2_data.v.data);
         break;
      }
#endif
      
      if (source1_data.v.tag != source2_data.v.tag)
         source1_data.v.data = True; 
      else
         source1_data.v.data = source1_data.v.data != source2_data.v.data;
      source1_data.v.tag = TAG_INT;
      break;
   case LESS_THAN :
      if (source1_data.v.tag != TAG_INT || source2_data.v.tag != TAG_INT)
      {
         bprintf("InterpretBinaryAssign can't < 2 vars %i,%i and %i,%i\n",
            source1_data.v.tag,source1_data.v.data,
            source2_data.v.tag,source2_data.v.data);
         break;
      }
      source1_data.v.data = source1_data.v.data < source2_data.v.data;
      break;
   case GREATER_THAN :
      if (source1_data.v.tag != TAG_INT || source2_data.v.tag != TAG_INT)
      {
         bprintf("InterpretBinaryAssign can't > 2 vars %i,%i and %i,%i\n",
            source1_data.v.tag,source1_data.v.data,
            source2_data.v.tag,source2_data.v.data);
         break;
      }
      source1_data.v.data = source1_data.v.data > source2_data.v.data;
      break;
   case LESS_EQUAL :
      if (source1_data.v.tag != TAG_INT || source2_data.v.tag != TAG_INT)
      {
         bprintf("InterpretBinaryAssign can't <= 2 vars %i,%i and %i,%i\n",
            source1_data.v.tag,source1_data.v.data,
            source2_data.v.tag,source2_data.v.data);
         break;
      }
      source1_data.v.data = source1_data.v.data <= source2_data.v.data;
      break;
   case GREATER_EQUAL :
      if (source1_data.v.tag != TAG_INT || source2_data.v.tag != TAG_INT)
      {
         bprintf("InterpretBinaryAssign can't >= 2 vars %i,%i and %i,%i\n",
            source1_data.v.tag,source1_data.v.data,
            source2_data.v.tag,source2_data.v.data);
         break;
      }
      source1_data.v.data = source1_data.v.data >= source2_data.v.data;
      break;
   case BITWISE_AND :
      if (source1_data.v.tag != TAG_INT || source2_data.v.tag != TAG_INT)
      {
         bprintf("InterpretBinaryAssign can't and 2 vars %i,%i and %i,%i\n",
            source1_data.v.tag,source1_data.v.data,
            source2_data.v.tag,source2_data.v.data);
         break;
      }
      source1_data.v.data = source1_data.v.data & source2_data.v.data;
      break;
   case BITWISE_OR :
      if (source1_data.v.tag != TAG_INT || source2_data.v.tag != TAG_INT)
      {
         bprintf("InterpretBinaryAssign can't or 2 vars %i,%i and %i,%i\n",
            source1_data.v.tag,source1_data.v.data,
            source2_data.v.tag,source2_data.v.data);
         break;
      }
      source1_data.v.data = source1_data.v.data | source2_data.v.data;
      break;
   default :
      bprintf("InterpretBinaryAssign can't perform binary op %i\n", info);
      break;
   }

   StoreValue(object_id, local_vars, opcode.dest, opnode->dest, source1_data);
}

void InterpretGoto(int object_id, local_var_type *local_vars, opcode_type opcode)
{
   int dest_addr;
   int var_check;
   val_type check_data;

   /* we've read one byte of instruction so far */
   char *inst_start = bkod - 1;

   /* This function is called often, so the switch has been
   * optimized away to return the value immediately. */

   dest_addr = get_int();

   /* unconditional gotos have source2 bits set--otherwise, it's a goto
   only if the source1 bits have a non-zero var */
   if (opcode.source2 == GOTO_UNCONDITIONAL)
   {
      bkod = inst_start + dest_addr;
      return;
   }

   var_check = get_int();
   check_data = RetrieveValue(object_id,local_vars,opcode.source1,var_check);
   if ((opcode.dest == GOTO_IF_TRUE && check_data.v.data != 0) ||
      (opcode.dest == GOTO_IF_FALSE && check_data.v.data == 0))
      bkod = inst_start + dest_addr;
}

void InterpretCall(int object_id,local_var_type *local_vars,opcode_type opcode)
{
   parm_node normal_parm_array[MAX_C_PARMS], name_parm_array[MAX_NAME_PARMS]; 
   unsigned char info, num_normal_parms, num_name_parms, initial_type;
   int initial_value;
   val_type call_return, name_val;
   int assign_index;
   int i;

   info = get_byte(); /* get function id */

   switch(opcode.source1)
   {
   case CALL_NO_ASSIGN :
      break;
   case CALL_ASSIGN_LOCAL_VAR :
   case CALL_ASSIGN_PROPERTY :
      assign_index = get_int();
      break;
   }

   num_normal_parms = get_byte();

   if (num_normal_parms > MAX_C_PARMS)
   {
      bprintf("InterpretCall found a call w/ more than %i parms, DEATH\n",
         MAX_C_PARMS);
      FlushDefaultChannels();
      num_normal_parms = MAX_C_PARMS;
   }

   for (i=0;i<num_normal_parms;i++)
   {
      normal_parm_array[i].type = get_byte();
      normal_parm_array[i].value = get_int();
   }

   num_name_parms = get_byte();

   if (num_name_parms > MAX_NAME_PARMS)
   {
      bprintf("InterpretCall found a call w/ more than %i name parms, DEATH\n",
         MAX_NAME_PARMS);
      FlushDefaultChannels();
      num_name_parms = MAX_NAME_PARMS;
   }

   for (i=0;i<num_name_parms;i++)
   {
      name_parm_array[i].name_id = get_int();

      initial_type = get_byte();
      initial_value = get_int();

      /* translate to literal now, because won't have local vars
      if nested call to sendmessage again */

      /* maybe only need to do this in call to sendmessage and postmessage? */

      name_val = RetrieveValue(object_id,local_vars,initial_type,initial_value);

      name_parm_array[i].value = name_val.int_val;
   }

   // Time messages.
   if (kod_stat.debugtime)
   {
      double startTime = GetMicroCountDouble();
      /* increment timed count of the c function, for profiling info */
      kod_stat.c_count_timed[info]++;
      call_return.int_val = ccall_table[info](object_id, local_vars, num_normal_parms,
         normal_parm_array, num_name_parms, name_parm_array);
      kod_stat.ccall_total_time[info] += (GetMicroCountDouble() - startTime);
   }
   else
   {
      /* increment untimed count of the c function, for profiling info */
      kod_stat.c_count_untimed[info]++;
      call_return.int_val = ccall_table[info](object_id, local_vars, num_normal_parms,
         normal_parm_array, num_name_parms, name_parm_array);
   }

   switch(opcode.source1)
   {
      case CALL_NO_ASSIGN :
         break;
      case CALL_ASSIGN_LOCAL_VAR :
      case CALL_ASSIGN_PROPERTY :
         StoreValue(object_id,local_vars,opcode.source1,assign_index,call_return);
         break;
   }
}

char *BlakodDebugInfo()
{
   static char s[100];
   class_node *c;

   if (kod_stat.interpreting_class == INVALID_CLASS)
   {
      sprintf(s,"Server");
   }
   else
   {
      c = GetClassByID(kod_stat.interpreting_class);
      if (c == NULL)
         sprintf(s,"Invalid class %i",kod_stat.interpreting_class);
      else
         sprintf(s,"%s (%i)",c->fname,GetSourceLine(c,bkod));
   }
   return s;
}

char *BlakodStackInfo()
{
   static char buf[5000];
   class_node *c;
   int i;

   buf[0] = '\0';
   for (i=message_depth-1;i>=0;i--)
   {
      char s[1000];
      if (stack[i].class_id == INVALID_CLASS)
      {
         sprintf(s,"Server");
      }
      else
      {
         c = GetClassByID(stack[i].class_id);
         if (c == NULL)
            sprintf(s,"Invalid class %i",stack[i].class_id);
         else
         {
            char *bp;
            char *class_name;
            char buf2[200];
            char parms[800];
            int j;

            /* for current frame, stack[] has pointer at beginning of function;
               use current pointer instead */
            bp = stack[i].bkod_ptr;
            if (i == message_depth-1)
               bp = bkod;

            class_name = "(unknown)";
            if (c->class_name)
               class_name = c->class_name;
            /* use %.*s with a fixed string of pluses to get exactly one plus per
               propagate depth */
            sprintf(s,"%.*s%s::%s",stack[i].propagate_depth,"++++++++++++++++++++++",
               class_name,GetNameByID(stack[i].message_id));
            strcat(s,"(");
            parms[0] = '\0';
            for (j=0;j<stack[i].num_parms;j++)
            {
               val_type val;
               val.int_val = stack[i].parms[j].value;
               sprintf(buf2,"#%s=%s %s",GetNameByID(stack[i].parms[j].name_id),
                       GetTagName(val),GetDataName(val));
               if (j > 0)
                  strcat(parms,",");
               strcat(parms,buf2);
            }
            strcat(s,parms);
            strcat(s,")");
            sprintf(buf2," %s (%i)",c->fname,GetSourceLine(c,bp));
            strcat(s,buf2);
         }
      }
      if (i < message_depth-1)
         strcat(buf,"\n");
      strcat(buf,s);
      if (strlen(buf) > sizeof(buf) - 1000)
      {
         strcat(buf,"\n...and more");
         break;
      }
   }
   return buf;
}
