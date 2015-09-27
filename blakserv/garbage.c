// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * garbage.c
 *

 This module performs garbage collection on the list, table, object,
 string and timer nodes.  The most complicated part is the list nodes,
 everything else isn't too complicated.  See the GarbageCollect()
 function below for a full description of how things work.

 */

#include "blakserv.h"

#define SERVER_MERGE_BASE		(0)

#define VISITED_LIST (1<<31)

#define UNREFERENCED -1
#define REFERENCED -2

/* local function prototypes */

void GarbageKickoffGamePick(session_node *s);
void GarbageWarnAdminSession(session_node *s);

/* list node garbage collection */
void ClearListNodeGarbageRef(list_node *l,int list_id);
void MarkObjectListNodesAndTables(object_node *o);
void MarkTableListNode(int table_id);
void MarkListNode(int list_id);
void RenumberListNode(list_node *l,int list_id);
void RenumberObjectContainerReferences(object_node *o);
void RenumberTableListNodeReferences(table_node *t, int table_id);
void RenumberListNodeReferences(val_type *vlist_ptr);
void CompactListNode(list_node *l,int list_id);

/* Table garbage collection */
void ClearTableGarbageRef(table_node *t,int table_id);
void RenumberTable(table_node *t, int table_id);
void RenumberListNodeTableReferences(list_node *l, int list_id);
void RenumberTableReferences(val_type *vtable_ptr);
void CompactTable(table_node *t, int table_id);
void DeleteUnreferencedTable(table_node *t, int table_id);

/* object garbage collection */
void ClearObjectGarbageRef(object_node *o);
void MarkUserObjectNodes(user_node *u);
void MarkObject(int object_id);
void MarkListNodeObject(int list_id);
void MarkTableObject(int list_id);
void DeleteUnreferencedObject(object_node *o);

void RenumberObject(object_node *o);
void RenumberBlockerObjects(room_node *r);
void RenumberObjectReferences(object_node *o);
void RenumberUserObjectReferences(user_node *u);
void RenumberSessionObjectReferences(session_node *s);
void RenumberTimerObjectReferences(timer_node *t);
void RenumberListNodeObjectReferences(list_node *l,int list_id);
void RenumberTableObjectReferences(table_node *l, int table_id);
Bool ResetObjectReference(val_type *vobject_ptr);
void CompactObject(object_node *o);

/* timer garbage collection (well, renumbering) */
void RenumberTimer(timer_node *t);
void ResetTimerReference(val_type *vtimer_ptr);
void CompactTimer(timer_node *t);

/* string garbage collection */
void ClearStringGarbageRef(string_node *snod,int string_id);
void MarkObjectStrings(object_node *o);
void MarkListNodeStrings(list_node *l,int list_id);
void MarkTableStrings(table_node *t, int table_id);
void MarkString(int string_id);
void RenumberString(string_node *snod,int string_id);
void ResetStringReference(val_type *vlist_ptr);
void CompactString(string_node *snod,int string_id);

// Combined timer and string GC
void RenumberObjectTimerStringReferences(object_node *o);
void RenumberListNodeTimerStringReferences(list_node *l, int list_id);
void RenumberTableTimerStringReferences(table_node *t, int table_id);

int next_renumber;
int next_timer_renumber;
int next_string_renumber;
int next_list_renumber;
int next_table_renumber;

void GarbageCollect()
{
   /* anyone in game mode w/o a user can have stale data, so knock 'em out */
   ForEachSession(GarbageKickoffGamePick);

   ForEachSession(GarbageWarnAdminSession);

   UpdateSecurityRedbook();

   NewEpoch(); /* since object #'s, etc. change, we send a special byte
      in all messages saying which 'period' between GC's we are,
      so we can ignore messages from before this GC */

   if (GetKodStats())
      GetKodStats()->interpreting_time_object_id = INVALID_ID;

   // Tables now get GC'd, so don't reset them.
   //ResetTables();

   /* First, garbage collect the list nodes and tables */

   /* 
    * This is complicated, because there can be multiple references
    * to a list node/table out there.
    *
    * However, it's still O(num list nodes + num object nodes + num hash nodes)
    *
    * first, mark all list nodes and tables unreferenced.
    *  then, mark used list nodes and tables referenced.
    *  then, go through each list node and table in increasing numerical order and
    *        set the garbage_ref to what its new id will be.
    *  then, go through each object, list node and table and change container
    *        ids to the new id for that container
    *  then, go through each list node and table in increasing numerical order and
    *        move it to its new id spot.
    */

   // Clear garbage refs.
   ForEachListNode(ClearListNodeGarbageRef);
   ForEachTable(ClearTableGarbageRef);

   // Mark list nodes and tables.
   ForEachObject(MarkObjectListNodesAndTables);

   // Delete unreferenced tables.
   ForEachTable(DeleteUnreferencedTable);

   // Renumber lists.
   next_list_renumber = SERVER_MERGE_BASE;
   ForEachListNode(RenumberListNode);

   // Renumber tables.
   next_table_renumber = SERVER_MERGE_BASE;
   ForEachTable(RenumberTable);

   // Renumber object references to list nodes and tables.
   ForEachObject(RenumberObjectContainerReferences);
   // Renumber table references to list nodes.
   ForEachTable(RenumberTableListNodeReferences);
   // Renumber list node references to tables.
   ForEachListNode(RenumberListNodeTableReferences);

   // Compact list nodes.
   ForEachListNode(CompactListNode);
   SetNumListNodes(next_list_renumber);

   // Compact tables.
   ForEachTable(CompactTable);
   SetNumTables(next_table_renumber);

   // To avoid the rare case where we somehow end up with a list inside a table
   // referencing the table itself (or vice versa), go through each table again
   // and clear the garbage ref. When marking objects, tables will be marked
   // also to ensure they are traversed once only.
   ForEachTable(ClearTableGarbageRef);

   /* now garbage collect the object nodes */

   /* 
    * This is complicated, because there are multiple references to 
    * objects out there.
    *
    * However, it's still O(num list nodes + num object nodes + num hash nodes)
    *
    * First, go through every user and built-in objects and mark referenced objects.
    *  then, delete the unreferenced ones.
    *  then, go through each object in increasing numerical order and
    *        set the garbage_ref to what its new object id will be.
    *  then, go through each object, list node, table, user, session, and timer,
    *        and change its object id to that object's new object id.
    *  then, go through each object in increasing numerical order and
    *        move it to its new object id spot.
    */

   ForEachObject(ClearObjectGarbageRef);
   // Also clear string GC references now.
   ForEachString(ClearStringGarbageRef);

   ForEachUser(MarkUserObjectNodes);

   // Mark built-in objects so they don't get deleted.
   for (int i = 0; i <= MAX_BUILTIN_OBJECT; ++i)
      MarkObject(GetBuiltInObjectID(i));

   ForEachObject(DeleteUnreferencedObject);

   next_renumber = SERVER_MERGE_BASE;

   ForEachObject(RenumberObject);
   // Renumber object IDs in each room's blocker data.
   ForEachRoom(RenumberBlockerObjects);
   ForEachObject(RenumberObjectReferences); // Also mark strings here
   ForEachListNode(RenumberListNodeObjectReferences); // Also mark strings here
   ForEachTable(RenumberTableObjectReferences); // Also mark strings here
   ForEachUser(RenumberUserObjectReferences);
   ForEachSession(RenumberSessionObjectReferences);
   ForEachTimer(RenumberTimerObjectReferences);
   ForEachObject(CompactObject);
   SetNumObjects(next_renumber);

   // Combined timer and string GC, as references to both can be
   // renumbered at the same time.
   // Timer GC prevents rollover since they are created and deleted constantly.
   // String GC references previously cleared, and strings get marked during
   // object renumbering to save time.

   // Renumber timers and strings.
   next_timer_renumber = SERVER_MERGE_BASE;
   ForEachTimer(RenumberTimer);
   next_string_renumber = SERVER_MERGE_BASE;
   ForEachString(RenumberString);

   // Update references to timers and strings.
   ForEachObject(RenumberObjectTimerStringReferences);
   ForEachListNode(RenumberListNodeTimerStringReferences);
   ForEachTable(RenumberTableTimerStringReferences);

   // Compact timers and strings.
   ForEachTimer(CompactTimer);
   SetNumTimers(next_timer_renumber);
   ForEachString(CompactString);
   SetNumStrings(next_string_renumber);
}

/////////////////////////////////////////////////////////////////////////////

void GarbageKickoffGamePick(session_node *s)
{
   /* anyone in game mode w/o a user can have stale data, so knock 'em out */
   if (s && s->state == STATE_GAME && s->game->object_id == INVALID_OBJECT)
      SetSessionState(s,STATE_SYNCHED);
}

void GarbageWarnAdminSession(session_node *s)
{
   if (!s)
      return;

   if (s->state == STATE_ADMIN ||
       (s->state == STATE_GAME && s->account->type == ACCOUNT_ADMIN))
   {
      SendSessionAdminText(s->session_id,
         "*** GARBAGE COLLECTION: ALL REFERENCES REORDERED ***\n");
   }
}

/////////////////////////////////////////////////////////////////////////////
// Lists
/////////////////////////////////////////////////////////////////////////////

void ClearListNodeGarbageRef(list_node *l,int list_id)
{
   l->garbage_ref = UNREFERENCED;
}

void MarkObjectListNodesAndTables(object_node *o)
{
   int i;

   for (i=0;i<o->num_props;i++)
   {
      if (o->p[i].val.v.tag == TAG_LIST)
      {
         MarkListNode(o->p[i].val.v.data);
      }
      else if (o->p[i].val.v.tag == TAG_TABLE)
      {
         MarkTableListNode(o->p[i].val.v.data);
      }
   }
}

void MarkTableListNode(int table_id)
{
   hash_node *hn;
   table_node *t;

   t = GetTableByID(table_id);
   if (t == NULL)
   {
      eprintf("MarkTableListNode death by garbage collection\n");
      return;
   }

   if (t->garbage_ref == REFERENCED)
      return;

   t->garbage_ref = REFERENCED;

   for (int i = 0; i < t->size; ++i)
   {
      hn = t->table[i];
      while (hn != NULL)
      {
         if (hn->data_val.v.tag == TAG_LIST)
            MarkListNode(hn->data_val.v.data);
         else if (hn->data_val.v.tag == TAG_TABLE)
            MarkTableListNode(hn->data_val.v.data);
         hn = hn->next;
      }
   }
}

void MarkListNode(int list_id)
{
   list_node *l;

   while (1)
   {
      l = GetListNodeByID(list_id);
      if (l == NULL)
      {
         eprintf("MarkListNode death by garbage collection\n");
         return;
      }

      if (l->garbage_ref == REFERENCED)
         return;

      l->garbage_ref = REFERENCED;

      if (l->first.v.tag == TAG_LIST)
         MarkListNode(l->first.v.data);
      else if (l->first.v.tag == TAG_TABLE)
         MarkTableListNode(l->first.v.data);

      if (l->rest.v.tag != TAG_LIST)
         break;

      list_id = l->rest.v.data;
   }
}

void RenumberListNode(list_node *l,int list_id)
{
   if (l->garbage_ref == REFERENCED)
   {
      l->garbage_ref = next_list_renumber++;
   }
}

void RenumberObjectContainerReferences(object_node *o)
{
   int i;

   for (i=0;i<o->num_props;i++)
   {
      if (o->p[i].val.v.tag == TAG_LIST)
         RenumberListNodeReferences(&(o->p[i].val));
      else if (o->p[i].val.v.tag == TAG_TABLE)
         RenumberTableReferences(&(o->p[i].val));
   }
}

void RenumberTableListNodeReferences(table_node *t, int table_id)
{
   hash_node *hn;

   for (int i = 0; i < t->size; ++i)
   {
      hn = t->table[i];
      while (hn != NULL)
      {
         if (hn->data_val.v.tag == TAG_LIST)
            RenumberListNodeReferences(&(hn->data_val));
         hn = hn->next;
      }
   }
}

void RenumberListNodeReferences(val_type *vlist_ptr)
{
   list_node *l;

  begin:
   l = GetListNodeByID(vlist_ptr->v.data);
   if (l == NULL)
   {
      eprintf("RenumberListNodeReferences death by garbage collection\n");
      return;
   }

   if (l->garbage_ref == REFERENCED || l->garbage_ref == UNREFERENCED)
   {
      eprintf("RenumberListNodeReferences unrenumbered list node %i\n",
         vlist_ptr->v.data);
      return;
   }

   /* this VISITED_LIST bit is around because suppose there are two lists
    * each containing a third list.  The first reference to list 3 will
    * move its first and rest things, but the second should not.
    */

   vlist_ptr->v.data = l->garbage_ref & ~VISITED_LIST; /* has the new list node id */

   if (!(l->garbage_ref & VISITED_LIST))
   {
      l->garbage_ref = l->garbage_ref | VISITED_LIST;
      
      if (l->first.v.tag == TAG_LIST)
         RenumberListNodeReferences(&(l->first));
      if (l->rest.v.tag == TAG_LIST)
      {
         vlist_ptr = &(l->rest);
         goto begin;
      }
   }
}

void CompactListNode(list_node *l,int list_id)
{
   if (l->garbage_ref != UNREFERENCED)
      MoveListNode(l->garbage_ref & ~VISITED_LIST,list_id);
}

/////////////////////////////////////////////////////////////////////////////
// Tables
/////////////////////////////////////////////////////////////////////////////

void ClearTableGarbageRef(table_node *t,int table_id)
{
   t->garbage_ref = UNREFERENCED;
}

// Sets the new table id for each referenced table.
void RenumberTable(table_node *t,int table_id)
{
   if (t->garbage_ref == REFERENCED)
   {
      t->garbage_ref = next_table_renumber++;
   }
}

// Iterate through list nodes, renumber tables.
void RenumberListNodeTableReferences(list_node *l, int list_id)
{
   if (l->first.v.tag == TAG_TABLE)
      RenumberTableReferences(&(l->first));
   if (l->rest.v.tag == TAG_TABLE)
      RenumberTableReferences(&(l->rest));
}

// Picks the correct new ID for a table.
void RenumberTableReferences(val_type *vtable_ptr)
{
   table_node *t;
   hash_node *hn;

   t = GetTableByID(vtable_ptr->v.data);
   if (t == NULL)
   {
      eprintf("RenumberTableReferences death by garbage collection\n");
      return;
   }

   if (t->garbage_ref == REFERENCED || t->garbage_ref == UNREFERENCED)
   {
      eprintf("RenumberTableReferences unrenumbered table %i\n",
         vtable_ptr->v.data);
      return;
   }

   /* this VISITED_LIST bit is around because suppose there are two tables
    * each containing a third table.  The first reference to table 3 will
    * move its data, but the second should not.
    */

   vtable_ptr->v.data = t->garbage_ref & ~VISITED_LIST; /* has the new table id */

   if (!(t->garbage_ref & VISITED_LIST))
   {
      t->garbage_ref = t->garbage_ref | VISITED_LIST;
      for (int i = 0; i < t->size; ++i)
      {
         hn = t->table[i];
         while (hn != NULL)
         {
            if (hn->data_val.v.tag == TAG_TABLE)
               RenumberTableReferences(&(hn->data_val));
            hn = hn->next;
         }
      }
   }
}

// Moves tables to new position.
void CompactTable(table_node *t, int table_id)
{
   if (t->garbage_ref != UNREFERENCED)
      MoveTable(t->garbage_ref & ~VISITED_LIST, table_id);
}

// Delete tables that aren't referenced anywhere.
void DeleteUnreferencedTable(table_node *t, int table_id)
{
   if (t->garbage_ref == UNREFERENCED)
      DeleteTable(table_id);
}

/////////////////////////////////////////////////////////////////////////////
// Objects
/////////////////////////////////////////////////////////////////////////////

void ClearObjectGarbageRef(object_node *o)
{
   o->garbage_ref = UNREFERENCED;
}

void MarkUserObjectNodes(user_node *u)
{
   MarkObject(u->object_id);
}

void MarkObject(int object_id)
{
   int i;

   object_node *o;

   o = GetObjectByID(object_id);
   if (o == NULL)
   {
      eprintf("MarkObject death by garbage collection\n");
      return;
   }

   if (o->garbage_ref == REFERENCED)
      return;

   o->garbage_ref = REFERENCED;

   for (i=0;i<o->num_props;i++)
   {
      if (o->p[i].val.v.tag == TAG_OBJECT)
         MarkObject(o->p[i].val.v.data);
      else if (o->p[i].val.v.tag == TAG_LIST)
         MarkListNodeObject(o->p[i].val.v.data);
      else if (o->p[i].val.v.tag == TAG_TABLE)
         MarkTableObject(o->p[i].val.v.data);
   }
}

void MarkListNodeObject(int list_id)
{
   list_node *l;

   while (1)
   {
      l = GetListNodeByID(list_id);
      if (l == NULL)
      {
         eprintf("MarkListNodeObject death by garbage collection\n");
         return;
      }

      if (l->first.v.tag == TAG_OBJECT)
         MarkObject(l->first.v.data);
      else if (l->first.v.tag == TAG_LIST)
         MarkListNodeObject(l->first.v.data);
      else if (l->first.v.tag == TAG_TABLE)
         MarkTableObject(l->first.v.data);

      if (l->rest.v.tag == TAG_LIST)
         list_id = l->rest.v.data;
      else if (l->rest.v.tag == TAG_OBJECT)
         MarkObject(l->rest.v.data);
      else if (l->rest.v.tag == TAG_TABLE)
         MarkTableObject(l->rest.v.data);
      else
         break;
   }
}

void MarkTableObject(int table_id)
{
   table_node *t;
   hash_node *hn;

   t = GetTableByID(table_id);
   if (t == NULL)
   {
      eprintf("MarkTableObject death by garbage collection\n");
      return;
   }

   // Mark the table, in case we have a list inside a table inside a list
   // (i.e. the same list) or table inside a list inside a table. This will
   // ensure all objects get marked without causing stack overflow.
   if (t->garbage_ref == REFERENCED)
      return;

   t->garbage_ref = REFERENCED;

   for (int i = 0; i < t->size; ++i)
   {
      hn = t->table[i];
      while (hn != NULL)
      {
         if (hn->data_val.v.tag == TAG_OBJECT)
            MarkObject(hn->data_val.v.data);
         else if (hn->data_val.v.tag == TAG_TABLE)
            MarkTableObject(hn->data_val.v.data);
         else if (hn->data_val.v.tag == TAG_LIST)
            MarkListNodeObject(hn->data_val.v.data);
         hn = hn->next;
      }
   }
}

void DeleteUnreferencedObject(object_node *o)
{
   if (o->garbage_ref == UNREFERENCED)
      DeleteBlakodObject(o->object_id);
}

void RenumberObject(object_node *o)
{
   o->garbage_ref = next_renumber++;
}

void RenumberBlockerObjects(room_node *r)
{
   Blocker *b;

   b = r->data.Blocker;
   while (b)
   {
      b->ObjectID = GetObjectByID(b->ObjectID)->garbage_ref;
      b = b->Next;
   }
}

void RenumberObjectReferences(object_node *o)
{
   int i;

   for (i=0;i<o->num_props;i++)
   {
      if (o->p[i].val.v.tag == TAG_OBJECT)
      {
         if (ResetObjectReference(&(o->p[i].val)) == False)
         {
            eprintf("RenumberObjectReferences got object death in object %i\n",
               o->object_id);
         }
      }
      else if (o->p[i].val.v.tag == TAG_STRING)
         MarkString(o->p[i].val.v.data);
   }
}

void RenumberListNodeObjectReferences(list_node *l,int list_id)
{
   if (l->first.v.tag == TAG_OBJECT)
   {
      if (ResetObjectReference(&(l->first)) == False)
         eprintf("RenumberListNodesReferences got object death in list node %i first\n",
            list_id);
   }
   else if (l->first.v.tag == TAG_STRING)
      MarkString(l->first.v.data);

   if (l->rest.v.tag == TAG_OBJECT)
   {
      if (ResetObjectReference(&(l->rest)) == False)
         eprintf("RenumberListNodesReferences got object death in list node %i first\n",
            list_id);
   }
   else if (l->rest.v.tag == TAG_STRING)
      MarkString(l->rest.v.data);
}

void RenumberTableObjectReferences(table_node *t, int table_id)
{
   hash_node *hn;

   for (int i = 0; i < t->size; ++i)
   {
      hn = t->table[i];
      while (hn != NULL)
      {
         if (hn->data_val.v.tag == TAG_OBJECT)
         {
            if (ResetObjectReference(&(hn->data_val)) == False)
               eprintf("RenumberTableObjectReferences got object death in table %i key %i\n",
                  table_id, i);
         }
         else if (hn->data_val.v.tag == TAG_STRING)
            MarkString(hn->data_val.v.data);
         hn = hn->next;
      }
   }
}

void RenumberUserObjectReferences(user_node *u)
{
   object_node *o;

   o = GetObjectByID(u->object_id);
   if (o == NULL)
   {
      eprintf("RenumberUserObjectReferences death by garbage collection\n");
      return;
   }
   u->object_id = o->garbage_ref; /* has the new object id */
}

void RenumberSessionObjectReferences(session_node *s)
{
   object_node *o;

   if (s->state != STATE_GAME)
      return;

   if (s->game->object_id == INVALID_OBJECT)
      return; /* they're in char selection, no problem */

   o = GetObjectByID(s->game->object_id);
   if (o == NULL)
   {
      eprintf("RenumberSessionObjectReferences death by garbage collection\n");
      return;
   }
   s->game->object_id = o->garbage_ref; /* has the new object id */
}

void RenumberTimerObjectReferences(timer_node *t)
{
   object_node *o;

   o = GetObjectByID(t->object_id);
   if (o == NULL)
   {
      eprintf("RenumberTimerObjectReferences death by garbage collection, message %s\n",
         GetNameByID(t->message_id));

      return;
   }
   t->object_id = o->garbage_ref; /* has the new object id */
}


Bool ResetObjectReference(val_type *vobject_ptr)
{
   object_node *o;

   o = GetObjectByID(vobject_ptr->v.data);
   if (o == NULL)
   {
      eprintf("ResetObjectReference death by garbage collection\n");
      return False;
   }

   vobject_ptr->v.data = o->garbage_ref; /* has the new object id */
   return True;
}

void CompactObject(object_node *o)
{
   MoveObject(o->garbage_ref,o->object_id);
}

/////////////////////////////////////////////////////////////////////////////
// Timers and strings
/////////////////////////////////////////////////////////////////////////////

void RenumberObjectTimerStringReferences(object_node *o)
{
   int i;

   for (i = 0; i<o->num_props; i++)
   {
      if (o->p[i].val.v.tag == TAG_TIMER)
         ResetTimerReference(&(o->p[i].val));
      else if (o->p[i].val.v.tag == TAG_STRING)
         ResetStringReference(&(o->p[i].val));
   }
}

void RenumberListNodeTimerStringReferences(list_node *l, int list_id)
{
   if (l->first.v.tag == TAG_TIMER)
      ResetTimerReference(&(l->first));
   else if (l->first.v.tag == TAG_STRING)
      ResetStringReference(&(l->first));

   if (l->rest.v.tag == TAG_TIMER)
      ResetTimerReference(&(l->rest));
   else if (l->rest.v.tag == TAG_STRING)
      ResetStringReference(&(l->rest));
}

void RenumberTableTimerStringReferences(table_node *t, int table_id)
{
   hash_node *hn;

   for (int i = 0; i < t->size; ++i)
   {
      hn = t->table[i];
      while (hn != NULL)
      {
         if (hn->data_val.v.tag == TAG_TIMER)
            ResetTimerReference(&(hn->data_val));
         else if (hn->data_val.v.tag == TAG_STRING)
            ResetStringReference(&(hn->data_val));
         hn = hn->next;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////
// Timers
/////////////////////////////////////////////////////////////////////////////

void RenumberTimer(timer_node *t)
{
   t->garbage_ref = next_timer_renumber++;
}

void ResetTimerReference(val_type *vtimer_ptr)
{
   timer_node *t;

   t = GetTimerByID(vtimer_ptr->v.data);
   if (t == NULL)
   {
      eprintf("ResetTimerReference found a reference to non-existent timer %i\n",
         vtimer_ptr->v.data);
      return;
   }

   vtimer_ptr->v.data = t->garbage_ref; /* has the new timer id */
}

void CompactTimer(timer_node *t)
{
   t->timer_id = t->garbage_ref;
}

/////////////////////////////////////////////////////////////////////////////
// Strings
/////////////////////////////////////////////////////////////////////////////

void ClearStringGarbageRef(string_node *snod,int string_id)
{
   snod->garbage_ref = UNREFERENCED;
}

void MarkObjectStrings(object_node *o)
{
   int i;

   for (i=0;i<o->num_props;i++)
   {
      if (o->p[i].val.v.tag == TAG_STRING)
         MarkString(o->p[i].val.v.data);
   }
}

void MarkListNodeStrings(list_node *l,int list_id)
{
   if (l->first.v.tag == TAG_STRING)
      MarkString(l->first.v.data);
   if (l->rest.v.tag == TAG_STRING)
      MarkString(l->rest.v.data);
}

void MarkTableStrings(table_node *t, int table_id)
{
   hash_node *hn;

   for (int i = 0; i < t->size; ++i)
   {
      hn = t->table[i];
      while (hn != NULL)
      {
         if (hn->data_val.v.tag == TAG_STRING)
            MarkString(hn->data_val.v.data);
         hn = hn->next;
      }
   }
}

void MarkString(int string_id)
{
   string_node *snod;

   snod = GetStringByID(string_id);
   if (snod == NULL)
   {
      eprintf("MarkString death by garbage collection\n");
      return;
   }
   snod->garbage_ref = REFERENCED;
}

void RenumberString(string_node *snod,int string_id)
{
   if (snod->garbage_ref == REFERENCED)
   {
      snod->garbage_ref = next_string_renumber++;
   }
}

void ResetStringReference(val_type *vlist_ptr)
{
   string_node *snod;

   snod = GetStringByID(vlist_ptr->v.data);
   if (snod == NULL)
   {
      eprintf("ResetStringReference death by garbage collection\n");
      return;
   }

   if (snod->garbage_ref == REFERENCED || snod->garbage_ref == UNREFERENCED)
   {
      eprintf("RenumberStringReferences unrenumbered string node %i\n",
         vlist_ptr->v.data);
      return;
   }

   vlist_ptr->v.data = snod->garbage_ref; /* has the new list node id */
}

void CompactString(string_node *snod,int string_id)
{
   if (snod->garbage_ref == UNREFERENCED)
      FreeString(string_id);
   else
      MoveStringNode(snod->garbage_ref,string_id);
}
