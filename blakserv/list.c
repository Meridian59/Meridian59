// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* list.c
*

  This module maintains a dynamically sized array with the list nodes
  used by the Blakod.  They are like LISP list nodes, keeping values in
  two fields, first and rest.
  
*/

#include "blakserv.h"

list_node *list_nodes;
int num_nodes,max_nodes;

/* local function prototypes */
int AllocateListNode(void);

void InitList(void)
{
	num_nodes = 0;
	max_nodes = INIT_LIST_NODES;
	list_nodes = (list_node *)AllocateMemory(MALLOC_ID_LIST,max_nodes*sizeof(list_node));
}

void ResetList(void)
{
	ClearList();
}

/* ClearList
* 
* Need this because when loading game, if there is an error, reset anything
* that was already set.
*
*/
void ClearList(void)
{
	int old_nodes;
	
	old_nodes = max_nodes;
	
	num_nodes = 0;
	max_nodes = INIT_LIST_NODES;
	list_nodes = (list_node *)
		ResizeMemory(MALLOC_ID_LIST,list_nodes,old_nodes*sizeof(list_node),
		max_nodes*sizeof(list_node));
}

int GetListNodesUsed(void)
{
	return num_nodes;
}

int AllocateListNode(void)
{
	int old_nodes;
	
	if (num_nodes == max_nodes)
	{
		old_nodes = max_nodes;
		max_nodes = max_nodes + ( INIT_LIST_NODES/2) ;
		
		list_nodes = (list_node *)
			ResizeMemory(MALLOC_ID_LIST,list_nodes,old_nodes*sizeof(list_node),
			max_nodes*sizeof(list_node));      
		lprintf("AllocateListNode resized to %i list nodes\n",max_nodes);
	}
	return num_nodes++;
}

Bool LoadList(int list_id,val_type first,val_type rest)
{
	if (AllocateListNode() != list_id)
	{
		eprintf("LoadList didn't make list id %i\n",list_id);
		return False;
	}
	
	list_nodes[list_id].first = first;
	list_nodes[list_id].rest = rest;
	
	return True;
}

list_node *GetListNodeByID(int list_id)
{
	if (list_id < 0 || list_id >= num_nodes)
	{
		eprintf("GetListNodeByID can't retrieve invalid list node %i\n",list_id);
		return NULL;
	}
	return &list_nodes[list_id];
}

Bool IsListNodeByID(int list_id)
{
	if (list_id < 0 || list_id >= num_nodes)
		return False;
	
	return True;
}

int First(int list_id)
{
	list_node *l;
	
	l = GetListNodeByID(list_id);
	return (l? l->first.int_val : NIL);
}

int Rest(int list_id)
{
	list_node *l;
	
	l = GetListNodeByID(list_id);
	return (l? l->rest.int_val : NIL);
}

int AppendListElem(val_type source,val_type list_val)
{
   int list_id, new_list_id, n = 0;
   list_node *l, *new_node;

   list_id = list_val.v.data;

   if (list_val.v.tag == TAG_NIL)
      return Cons(source,list_val);

   l = GetListNodeByID(list_id);
   if (!l)
   {
      bprintf("AppendListElem couldn't find list node %i, returning list",list_id);
      return list_id;
   }

   while (l && l->rest.v.tag != TAG_NIL)
   {
      l = GetListNodeByID(l->rest.v.data);
      n++;
   }

   if (n > 500)
      bprintf("Warning, AppendListElem adding to large list, length %i",n);

   new_list_id = AllocateListNode();
   new_node = GetListNodeByID(new_list_id);
   if (!new_node)
   {
      bprintf("AppendListElem couldn't create list node, returning list %i",list_id);
      return list_id;
   }

   new_node->rest.int_val = NIL;
   new_node->first.int_val = source.int_val;
   l->rest.v.data = new_list_id;
   l->rest.v.tag = TAG_LIST;

   return list_id;
}

int Cons(val_type source,val_type dest)
{
   int list_id;
   list_node *new_node;

   /*   bprintf("Allocing list node #%i\n",num_nodes); */

   list_id = AllocateListNode();
   new_node = GetListNodeByID(list_id);
   if (!new_node)
   {
      bprintf("Cons couldn't create new list node!");
      return NIL;
   }

   new_node->first.int_val = source.int_val;
   new_node->rest.int_val = dest.int_val;
   return list_id;
}

int Length(int list_id)
{
	int len_so_far;
	list_node *l;
	
	l = GetListNodeByID(list_id);
	if (!l)
		return 0;
	
	len_so_far = 1;
	while (l && l->rest.v.tag != TAG_NIL)
	{
		len_so_far++;
		l = GetListNodeByID(l->rest.v.data);
	}
	return len_so_far;
}

int Nth(int n,int list_id)
{
	int i;
	list_node *l;
	
	l = GetListNodeByID(list_id);
	for (i=1;i<n;i++)
	{
		if (!l)
		{
			bprintf("Nth found invalid list node somewhere in list %i\n",
				list_id);
			return NIL;
		}
		if (l->rest.v.tag != TAG_LIST)
		{
			bprintf("Nth can't go past end of list %i,%i\n",
				l->rest.v.tag,l->rest.v.data);
			return NIL;
		}
		l = GetListNodeByID(l->rest.v.data);
	}
	return (l? l->first.int_val : NIL);
}

int IsListMatch(int list_one_id, int list_two_id)
{
   list_node *l1, *l2;

   l1 = GetListNodeByID(list_one_id);
   l2 = GetListNodeByID(list_two_id);

   if (!l1)
   {
      bprintf("IsListMatch had invalid list node at start of list one\n");
      return False;
   }
   if (!l2)
   {
      bprintf("IsListMatch had invalid list node at start of list two\n");
      return False;
   }

   // If the element is a list, check its contents, else compare int_val.
   if (l1->first.v.tag == TAG_LIST && l2->first.v.tag == TAG_LIST)
   {
      if (!IsListMatch(l1->first.v.data,l2->first.v.data))
         return False;
   }
   else if (l1->first.int_val != l2->first.int_val)
      return False;

   // Iterate through lists and check each node.
   while (l1->rest.v.tag != TAG_NIL && l2->rest.v.tag != TAG_NIL)
   {
      l1 = GetListNodeByID(l1->rest.v.data);
      l2 = GetListNodeByID(l2->rest.v.data);

      if (!l1)
      {
         bprintf("IsListMatch had invalid list node somewhere in list one\n");
         return False;
      }

      if (!l2)
      {
         bprintf("IsListMatch had invalid list node somewhere in list two\n");
         return False;
      }

      // If the element is a list, check its contents, else compare int_val.
      if (l1->first.v.tag == TAG_LIST && l2->first.v.tag == TAG_LIST)
      {
         if (!IsListMatch(l1->first.v.data,l2->first.v.data))
            return False;
      }
      else if (l1->first.int_val != l2->first.int_val)
         return False;
   }

   // Make sure we're at end of both lists.
   if (l1->rest.v.tag != TAG_NIL || l2->rest.v.tag != TAG_NIL)
      return False;

   return True;
}

int Last(int list_id)
{
   list_node *l;

   l = GetListNodeByID(list_id);
   if (!l)
   {
      bprintf("Last given invalid list ID %i\n", list_id);
      return NIL;
   }

   while (l && l->rest.v.tag != TAG_NIL)
   {
      l = GetListNodeByID(l->rest.v.data);
   }

   if (!l)
   {
      bprintf("Last found invalid list node somewhere in list %i\n",
         list_id);
      return NIL;
   }

   return l->first.int_val;
}

int SetFirst(int list_id,val_type new_val)
{
	list_node *l;
	
	l = GetListNodeByID(list_id);
	if (l)
		l->first = new_val;
	
	return NIL;
}

int SetNth(int n,int list_id,val_type new_val)
{
	int i;
	list_node *l;
	
	l = GetListNodeByID(list_id);
	for (i=1;i<n;i++)
	{
		if (!l)
		{
			bprintf("SetNth found invalid list node somewhere in list %i\n",
				list_id);
			return NIL;
		}
		if (l->rest.v.tag != TAG_LIST)
		{
			bprintf("SetNth can't go past end o' list %i,%i\n",
				l->rest.v.tag,l->rest.v.data);
			return NIL;
		}
		l = GetListNodeByID(l->rest.v.data);
	}
	
	if (l)
		l->first = new_val;
	
	return NIL;
}

int SwapListElem(int list_id,int elem_one,int elem_two)
{
   list_node *l, *list_node_one, *list_node_two;
   val_type temp;

   l = GetListNodeByID(list_id);

   // Set each of the list nodes to be swapped to the first list node initially.
   list_node_one = l;
   list_node_two = l;

   // Start i at 2, since n or m = 1 will be handled by the initialisation.
   for (int i = 2; i <= elem_one || i <= elem_two; i++)
   {
      if (!l)
      {
         bprintf("SwapListElem found invalid list node somewhere in list %i\n",
            list_id);
         return NIL;
      }
      if (l->rest.v.tag != TAG_LIST)
      {
         bprintf("SwapListElem can't go past end of list %i,%i\n",
            l->rest.v.tag,l->rest.v.data);
         return NIL;
      }

      l = GetListNodeByID(l->rest.v.data);

      if (i == elem_one)
      {
         list_node_one = l;
      }
      if (i == elem_two)
      {
         list_node_two = l;
      }
   }

   temp = list_node_two->first;
   list_node_two->first = list_node_one->first;
   list_node_one->first = temp;

   return NIL;
}

int FindListElem(val_type list_id,val_type list_elem)
{
	int i;
	list_node *l;
	
	l = GetListNodeByID(list_id.v.data);
	if (!l)
		return NIL;
	
	if (l->first.int_val == list_elem.int_val)
	{
		return 1;
	}
	
	i = 1;
	while (l && l->rest.v.data != NIL && l->first.int_val != list_elem.int_val)
	{
		l = GetListNodeByID(l->rest.v.data);
		i++;
	}
	if (l && l->first.int_val == list_elem.int_val)
	{
		return i;
	}
	
	return NIL;
}

int InsertListElem(int n,int list_id,val_type new_val)
{
   int new_list_id;
   list_node *l, *prev = NULL, *new_node;

   if (n  == 0)
   {
      bprintf("InsertListElem given invalid list element %i, returning old list\n",
         n);
      return list_id;
   }

   l = GetListNodeByID(list_id);

   // Start at i = 2, n = 1 handled already.
   for (int i = 2; i <= n; i++)
   {
      if (!l)
      {
         bprintf("InsertListElem found invalid list node somewhere in list %i\n",
            list_id);
         return list_id;
      }
      if (l->rest.v.tag != TAG_LIST)
      {
         // Add the new value to the end of the list.
         new_list_id = AllocateListNode();
         new_node = GetListNodeByID(new_list_id);
         if (!new_node)
         {
            bprintf("InsertListElem couldn't allocate new node! %i\n",
               new_list_id);
            return list_id;
         }
         new_node->rest.int_val = NIL;
         new_node->first.int_val = new_val.int_val;
         // Previous node points to this one.
         l->rest.v.tag = TAG_LIST;
         l->rest.v.data = new_list_id;
         return list_id;
      }
      prev = l;
      l = GetListNodeByID(l->rest.v.data);
   }

   if (!l || !prev)
   {
      bprintf("InsertListElem found invalid list node somewhere in list %i\n",
         list_id);
      return list_id;
   }

   new_list_id = AllocateListNode();
   new_node = GetListNodeByID(new_list_id);

   if (!new_node)
   {
      bprintf("InsertListElem couldn't allocate new node! %i\n",
         new_list_id);
      return list_id;
   }

   // This node is inserted in position of the existing one, so use its rest data.
   // Points to the old node.
   new_node->rest.v.data = prev->rest.v.data;
   new_node->rest.v.tag = TAG_LIST;
   new_node->first.int_val = new_val.int_val;
   // Previous node points to this one.
   prev->rest.v.data = new_list_id;

   return list_id;
}

int DelListElem(val_type list_id,val_type list_elem)
{
	list_node *l,*prev;
	
	l = GetListNodeByID(list_id.v.data);
	if (!l)
		return NIL;
	
	prev = l;
	if (l->first.int_val == list_elem.int_val)
	{
		return l->rest.int_val;
	}
	
	while (l && l->rest.v.data != NIL && l->first.int_val != list_elem.int_val)
	{
		prev = l;
		l = GetListNodeByID(l->rest.v.data);
	}
	if (l && l->first.int_val == list_elem.int_val)
	{
		prev->rest = l->rest;
		return list_id.int_val;
	}
	
	bprintf("DelListElem can't find elem %i,%i in list id %i\n",
		list_elem.v.tag,list_elem.v.data,list_id.v.data);
	
	return list_id.int_val;
}

void ForEachListNode(void (*callback_func)(list_node *l,int list_id))
{
	int i;
	
	for (i=0;i<num_nodes;i++)
		callback_func(&list_nodes[i],i);
}

/* these functions are for garbage collecting */

void MoveListNode(int dest_id,int source_id)
{
	list_node *source,*dest;
	
	source = GetListNodeByID(source_id);
	if (source == NULL)
	{
		eprintf("MoveListNode can't find source %i, total death end game\n",
			source_id);
		return;
	}
	
	dest = GetListNodeByID(dest_id);
	if (dest == NULL)
	{
		eprintf("MoveListNode can't find dest %i, total death end game\n",
			dest_id);
		return;
	}
	dest->first = source->first;
	dest->rest = source->rest;
	dest->garbage_ref = source->garbage_ref;
}

void SetNumListNodes(int new_num_nodes)
{
	num_nodes = new_num_nodes;
}
