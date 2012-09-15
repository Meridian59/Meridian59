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

 This module performs garbage collection on the list, object, string,
 and timer nodes.  The most complicated part is the list nodes,
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
void MarkObjectListNodes(object_node *o);
void MarkListNode(int list_id);
void RenumberListNode(list_node *l,int list_id);
void RenumberObjectListNodeReferences(object_node *o);
void RenumberListNodeReferences(val_type *vlist_ptr);
void CompactListNode(list_node *l,int list_id);

/* object garbage collection */
void ClearObjectGarbageRef(object_node *o);
void MarkUserObjectNodes(user_node *u);
void MarkObject(int object_id);
void MarkListNodeObject(int list_id);
void DeleteUnreferencedObject(object_node *o);

void RenumberObject(object_node *o);
void RenumberObjectReferences(object_node *o);
void RenumberUserObjectReferences(user_node *u);
void RenumberSessionObjectReferences(session_node *s);
void RenumberTimerObjectReferences(timer_node *t);
void RenumberListNodeObjectReferences(list_node *l,int list_id);
Bool ResetObjectReference(val_type *vobject_ptr);
void CompactObject(object_node *o);

/* timer garbage collection (well, renumbering) */
void RenumberTimer(timer_node *t);
void RenumberObjectTimerReferences(object_node *o);
void RenumberListNodeTimerReferences(list_node *l,int list_id);
void ResetTimerReference(val_type *vtimer_ptr);
void CompactTimer(timer_node *t);

/* string garbage collection */
void ClearStringGarbageRef(string_node *snod,int string_id);
void MarkObjectStrings(object_node *o);
void MarkListNodeStrings(list_node *l,int list_id);
void MarkString(int string_id);
void RenumberString(string_node *snod,int string_id);
void RenumberObjectStringReferences(object_node *o);
void RenumberListNodeStringReferences(list_node *l,int list_id);
void ResetStringReference(val_type *vlist_ptr);
void CompactString(string_node *snod,int string_id);


int next_renumber;

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

   ResetTable(); /* tables are not GC'ed yet, so we gotta clear 'em. */

   /* first, garbage collect the list nodes */

   /* 
    * This is complicated, because there can be multiple references
    * to a list node out there.
    *
    * However, it's still O(number of list nodes + number of object nodes)
    *
    * first, mark all list nodes unreferenced.
    *  then, mark used list nodes referenced.
    *  then, go through each list node in increasing numerical order and
    *        set the garbage_ref to what its new list node id will be.
    *  then, go through each object & list node and change its
    *        list id to that list node's new list id.
    *  then, go through each list node in increasing numerical order and
    *        move it to its new list id spot.
    */

   ForEachListNode(ClearListNodeGarbageRef);
   ForEachObject(MarkObjectListNodes);
   
   next_renumber = SERVER_MERGE_BASE;
   
   ForEachListNode(RenumberListNode);
   ForEachObject(RenumberObjectListNodeReferences);
   ForEachListNode(CompactListNode);

   SetNumListNodes(next_renumber);
   
   /* now garbage collect the object nodes */

   /* 
    * This is complicated, because there are multiple references to 
    * objects out there.
    *
    * However, it's still O(number of list nodes + number of object nodes)
    *
    * First, go through every user and system and mark referenced objects.
    *  then, delete the unreferenced ones.
    *  then, go through each object in increasing numerical order and
    *        set the garbage_ref to what its new object id will be.
    *  then, go through each object, list node, user, session, and timer,
    *        and change its object id to that object's new object id.
    *  then, go through each object in increasing numerical order and
    *        move it to its new object id spot.
    */

   ForEachObject(ClearObjectGarbageRef);
   ForEachUser(MarkUserObjectNodes);
   MarkObject(GetSystemObjectID());
   ForEachObject(DeleteUnreferencedObject);

   next_renumber = SERVER_MERGE_BASE;

   ForEachObject(RenumberObject);
   ForEachObject(RenumberObjectReferences);
   ForEachListNode(RenumberListNodeObjectReferences);
   ForEachUser(RenumberUserObjectReferences);
   ForEachSession(RenumberSessionObjectReferences);
   ForEachTimer(RenumberTimerObjectReferences);
   ForEachObject(CompactObject);
   SetNumObjects(next_renumber);

   /* now renumber timers, good for saving, and prevents rollover,
    * since they are created and deleted all the time 
    */

   next_renumber = SERVER_MERGE_BASE;

   ForEachTimer(RenumberTimer);
   ForEachObject(RenumberObjectTimerReferences);
   ForEachListNode(RenumberListNodeTimerReferences);
   ForEachTimer(CompactTimer);
   SetNumTimers(next_renumber);

   /* now garbage collect the strings, just like list nodes */

   ForEachString(ClearStringGarbageRef);
   ForEachObject(MarkObjectStrings);
   ForEachListNode(MarkListNodeStrings);

   next_renumber = SERVER_MERGE_BASE;

   ForEachString(RenumberString);
   ForEachObject(RenumberObjectStringReferences);
   ForEachListNode(RenumberListNodeStringReferences);
   ForEachString(CompactString);
   SetNumStrings(next_renumber);
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

void ClearListNodeGarbageRef(list_node *l,int list_id)
{
   l->garbage_ref = UNREFERENCED;
}

void MarkObjectListNodes(object_node *o)
{
   int i;

   for (i=0;i<o->num_props;i++)
   {
      if (o->p[i].val.v.tag == TAG_LIST)
      {
	 MarkListNode(o->p[i].val.v.data);
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
      
      l->garbage_ref = REFERENCED;
      
      if (l->first.v.tag == TAG_LIST)
	 MarkListNode(l->first.v.data);

      if (l->rest.v.tag != TAG_LIST)
	 break;

      list_id = l->rest.v.data;
   }
}

void RenumberListNode(list_node *l,int list_id)
{
   if (l->garbage_ref == REFERENCED)
   {
      l->garbage_ref = next_renumber++;
   }
}

void RenumberObjectListNodeReferences(object_node *o)
{
   int i;
   
   for (i=0;i<o->num_props;i++)
   {
      if (o->p[i].val.v.tag == TAG_LIST)
      {
	 RenumberListNodeReferences(&(o->p[i].val));
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
      if (o->p[i].val.v.tag == TAG_LIST)
	 MarkListNodeObject(o->p[i].val.v.data);
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
      if (l->rest.v.tag == TAG_OBJECT)
	 MarkObject(l->rest.v.data);
      
      if (l->first.v.tag == TAG_LIST)
	 MarkListNodeObject(l->first.v.data);
      if (l->rest.v.tag != TAG_LIST)
	 break;
      list_id = l->rest.v.data;
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
   
   if (l->rest.v.tag == TAG_OBJECT)
   {
      if (ResetObjectReference(&(l->rest)) == False)
	 eprintf("RenumberListNodesReferences got object death in list node %i first\n",
		 list_id);
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

void RenumberTimer(timer_node *t)
{
   t->garbage_ref = next_renumber++;
}

void RenumberObjectTimerReferences(object_node *o)
{
   int i;

   for (i=0;i<o->num_props;i++)
   {
      if (o->p[i].val.v.tag == TAG_TIMER)
	 ResetTimerReference(&(o->p[i].val));
   }
}

void RenumberListNodeTimerReferences(list_node *l,int list_id)
{
   if (l->first.v.tag == TAG_TIMER)
      ResetTimerReference(&(l->first));
   if (l->rest.v.tag == TAG_TIMER)
      ResetTimerReference(&(l->rest));
}

void ResetTimerReference(val_type *vtimer_ptr)
{
   timer_node *t;

   t = GetTimerByID(vtimer_ptr->v.data);
   if (t == NULL)
   {
      eprintf("ResetTimerReference found a reference to non-existent timer %i\n",vtimer_ptr->v.data);
      return;
   }

   vtimer_ptr->v.data = t->garbage_ref; /* has the new timer id */
}

void CompactTimer(timer_node *t)
{
   t->timer_id = t->garbage_ref;
}


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
      snod->garbage_ref = next_renumber++;
   }
}

void RenumberObjectStringReferences(object_node *o)
{
   int i;
   
   for (i=0;i<o->num_props;i++)
   {
      if (o->p[i].val.v.tag == TAG_STRING)
	 ResetStringReference(&(o->p[i].val));
   }
}

void RenumberListNodeStringReferences(list_node *l,int list_id)
{
   if (l->first.v.tag == TAG_STRING)
      ResetStringReference(&(l->first));
   if (l->rest.v.tag == TAG_STRING)
      ResetStringReference(&(l->rest));
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

