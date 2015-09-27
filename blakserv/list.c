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

int AppendListElem(val_type source, val_type list_val)
{
   int list_id, new_list_id, n = 0, temp_list_id = -1;
   list_node *l, *new_node;

   if (list_val.v.tag == TAG_NIL)
      return Cons(source,list_val);
   list_id = list_val.v.data;

   // If we're appending a list, get the ID now in case memory is reallocated.
   if (source.v.tag == TAG_LIST)
      temp_list_id = source.v.data;

   // Allocate first, so a resize doesn't clobber list references.
   new_list_id = AllocateListNode();
   new_node = GetListNodeByID(new_list_id);
   if (!new_node)
   {
      bprintf("AppendListElem couldn't create list node, returning list %i", list_id);
      return list_id;
   }

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

   new_node->rest.int_val = NIL;

   if (temp_list_id >= 0)
   {
      new_node->first.v.tag = TAG_LIST;
      new_node->first.v.data = temp_list_id;
   }
   else
      new_node->first.int_val = source.int_val;

   l->rest.v.data = new_list_id;
   l->rest.v.tag = TAG_LIST;

   return list_id;
}

int Cons(val_type source,val_type dest)
{
   int list_id, source_id = -1, dest_id = -1;
   list_node *new_node;

   if (source.v.tag == TAG_LIST)
      source_id = source.v.data;
   if (dest.v.tag == TAG_LIST)
      dest_id = dest.v.data;

   list_id = AllocateListNode();
   new_node = GetListNodeByID(list_id);
   if (!new_node)
   {
      bprintf("Cons couldn't create new list node!");
      return NIL;
   }

   if (source_id >= 0)
   {
      new_node->first.v.tag = TAG_LIST;
      new_node->first.v.data = source_id;
   }
   else
      new_node->first.int_val = source.int_val;
   if (dest_id >= 0)
   {
      new_node->rest.v.tag = TAG_LIST;
      new_node->rest.v.data = dest_id;
   }
   else
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

int GetAllListNodesByClass(int list_id, int position, int class_id)
{
   list_node *l;
   object_node *o;
   class_node *c;
   int new_list_id = -1, l_list_id;
   val_type nil_val, obj_val, first_val, rest_val;
   nil_val.int_val = NIL;
   first_val.v.tag = TAG_LIST;
   rest_val.v.tag = TAG_LIST;

   l = GetListNodeByID(list_id);
   if (!l)
   {
      bprintf("GetAllListNodesByClass got invalid list %i.\n", list_id);
      return NIL;
   }

   if (l->first.v.tag != TAG_LIST)
   {
      bprintf("GetAllListNodesByClass got invalid sub-list %i %i.\n",
         l->first.v.tag, l->first.v.data);
   }
   else
   {
      if (position == 1)
         obj_val.int_val = First(l->first.v.data);
      else
         obj_val.int_val = Nth(position, l->first.v.data);

      if (obj_val.v.tag == TAG_OBJECT)
      {
         o = GetObjectByID(obj_val.v.data);
         if (o == NULL)
         {
            bprintf("GetAllListNodesByClass can't find object %i\n",
               obj_val.v.data);
            return NIL;
         }
         c = GetClassByID(o->class_id);
         if (c == NULL)
         {
            bprintf("GetAllListNodesByClass can't find class %i, DIE totally\n",
               o->class_id);
            FlushDefaultChannels();
            return NIL;
         }
         do
         {
            if (c->class_id == class_id)
            {
               first_val.v.data = ListCopy(l->first.v.data);
               new_list_id = Cons(first_val, nil_val);
               break;
            }
            c = c->super_ptr;
         } while (c != NULL);

         // Just made an allocation, get list_node again.
         l = &list_nodes[list_id];
      }
   }

   while (l && l->rest.v.data != NIL)
   {
      // This may allocate a node, so save list id.
      l_list_id = l->rest.v.data;
      l = GetListNodeByID(l->rest.v.data);

      if (l->first.v.tag != TAG_LIST)
      {
         bprintf("GetAllListNodesByClass got invalid sub-list %i %i.\n",
            l->first.v.tag, l->first.v.data);
      }
      else
      {
         if (position == 1)
            obj_val.int_val = First(l->first.v.data);
         else
            obj_val.int_val = Nth(position, l->first.v.data);

         if (obj_val.v.tag == TAG_OBJECT)
         {
            o = GetObjectByID(obj_val.v.data);
            if (o == NULL)
            {
               bprintf("GetAllListNodesByClass can't find object %i\n",
                  obj_val.v.data);
               return NIL;
            }
            c = GetClassByID(o->class_id);
            if (c == NULL)
            {
               bprintf("GetAllListNodesByClass can't find class %i, DIE totally\n",
                  o->class_id);
               FlushDefaultChannels();
               return NIL;
            }

            do
            {
               if (c->class_id == class_id)
               {
                  if (new_list_id < 0)
                  {
                     first_val.v.data = ListCopy(l->first.v.data);
                     new_list_id = Cons(first_val, nil_val);
                  }
                  else
                  {
                     first_val.v.data = ListCopy(l->first.v.data);
                     rest_val.v.data = new_list_id;
                     new_list_id = Cons(first_val, rest_val);
                  }
                  break;
               }
               c = c->super_ptr;
            } while (c != NULL);
            // Just made an allocation, get list_node again.
            l = &list_nodes[l_list_id];
         }
      }
   }

   first_val.v.data = new_list_id;
   return new_list_id >= 0 ? first_val.int_val : nil_val.int_val;
}

// Works like FindListElem, but compares the class of the object.
int GetListElemByClass(val_type list_id, int class_id)
{
   list_node *l;
   object_node *o;
   class_node *c;

   l = GetListNodeByID(list_id.v.data);
   if (!l)
   {
      bprintf("GetListElemByClass got invalid list.\n");
      return NIL;
   }

   if (l->first.v.tag == TAG_OBJECT)
   {
      o = GetObjectByID(l->first.v.data);
      if (o == NULL)
      {
         bprintf("GetListElemByClass can't find object %i\n",l->first.v.data);
         return NIL;
      }
      c = GetClassByID(o->class_id);
      if (c == NULL)
      {
         bprintf("GetListElemByClass can't find class %i, DIE totally\n",
            o->class_id);
         FlushDefaultChannels();
         return NIL;
      }
      do
      {
         if (c->class_id == class_id)
            return l->first.int_val;
         c = c->super_ptr;
      } while (c != NULL);
   }

   while (l && l->rest.v.data != NIL)
   {
      l = GetListNodeByID(l->rest.v.data);
      if (l->first.v.tag == TAG_OBJECT)
      {
         o = GetObjectByID(l->first.v.data);
         if (o == NULL)
         {
            bprintf("GetListElemByClass can't find object %i\n",l->first.v.data);

            return NIL;
         }
         c = GetClassByID(o->class_id);
         if (c == NULL)
         {
            bprintf("GetListElemByClass can't find class %i, DIE totally\n",
               o->class_id);
            FlushDefaultChannels();
            return NIL;
         }
         do
         {
            if (c->class_id == class_id)
               return l->first.int_val;
            c = c->super_ptr;
         } while (c != NULL);
      }
   }

   return NIL;
}

// Returns the list node that contains list_elem in the position given.
int GetListNode(val_type list_id, int position, val_type list_elem)
{
   list_node *l;

   l = GetListNodeByID(list_id.v.data);
   if (!l)
   {
      bprintf("GetListNode got invalid list.\n");
      return NIL;
   }

   if (l->first.v.tag != TAG_LIST)
   {
      bprintf("GetListNode called on invalid list.\n");
      return NIL;
   }

   if (position == 1)
   {
      if (First(l->first.v.data) == list_elem.int_val)
         return l->first.int_val;
   }
   else if (Nth(position, l->first.v.data) == list_elem.int_val)
      return l->first.int_val;

   while (l && l->rest.v.data != NIL)
   {
      l = GetListNodeByID(l->rest.v.data);

      if (l->first.v.tag != TAG_LIST)
      {
         bprintf("GetListNode has invalid list.\n");
         return NIL;
      }
      if (position == 1)
      {
         if (First(l->first.v.data) == list_elem.int_val)
            return l->first.int_val;
      }
      else if (Nth(position, l->first.v.data) == list_elem.int_val)
         return l->first.int_val;
   }

   // Not found, return NIL.
   return NIL;
}

// ListCopy makes a lot of list allocations, and if a list allocation causes
// the list_nodes memory to be resized, all existing list_node references
// here are invalidated. As a result, this function is structured so that
// no old list_nodes are accessed after an allocation or recursive ListCopy
// call. This adds overhead, but is still the fastest safe way to copy a list.
int ListCopy(int list_id)
{
   list_node *l, *new_node, *new_next;
   int new_list_id, first_list_id, rest_list_id, l_list_id;
   int temp;

   // Allocate first so a resize doesn't clobber references.
   new_list_id = first_list_id = AllocateListNode();

   l = GetListNodeByID(list_id);
   if (!l)
   {
      bprintf("ListCopy got invalid list %i.\n",list_id);
      return NIL;
   }

   if (l->first.v.tag == TAG_LIST)
   {
      // Copy to temp, in case ListCopy resizes memory.
      temp = ListCopy(l->first.v.data);

      // After a ListCopy call, any existing list reference may be invalid.
      // Access list_nodes directly, list node already confirmed to exist.
      l = &list_nodes[list_id];

      new_node = GetListNodeByID(new_list_id);
      if (!new_node)
      {
         bprintf("ListCopy couldn't allocate new node! %i\n",new_list_id);
         return NIL;
      }
      new_node->first.v.data = temp;
      new_node->first.v.tag = TAG_LIST;
   }
   else
   {
      new_node = GetListNodeByID(new_list_id);
      if (!new_node)
      {
         bprintf("ListCopy couldn't allocate new node! %i\n",new_list_id);
         return NIL;
      }
      new_node->first.int_val = l->first.int_val;
   }

   // Get next list id for accessing after allocation.
   l_list_id = l->rest.v.data;

   while (l && l->rest.v.data != NIL)
   {
      // Allocate first so a resize doesn't clobber references.
      rest_list_id = AllocateListNode();

      l = GetListNodeByID(l_list_id);
      if (!l)
      {
         bprintf("ListCopy got invalid list node %i %i\n",
            l->rest.v.tag, l->rest.v.data);
         return NIL;
      }

      if (l->first.v.tag == TAG_LIST)
      {
         // Copy to temp, in case ListCopy resizes memory.
         temp = ListCopy(l->first.v.data);
         // After a ListCopy call, any existing list reference may be invalid.
         // Access list_nodes directly, list node already confirmed to exist.
         l = &list_nodes[l_list_id];

         // Get new list node.
         new_next = GetListNodeByID(rest_list_id);
         if (!new_next)
         {
            bprintf("ListCopy couldn't allocate new node! %i\n",
               new_list_id);
            return NIL;
         }
         // Access list_nodes directly, list node already confirmed to exist.
         new_node = &list_nodes[first_list_id];
         new_node->rest.v.tag = TAG_LIST;
         new_node->rest.v.data = rest_list_id;
         new_node = new_next;

         new_node->first.v.data = temp;
         new_node->first.v.tag = TAG_LIST;
      }
      else
      {
         // Get new list node.
         new_next = GetListNodeByID(rest_list_id);
         if (!new_next)
         {
            bprintf("ListCopy couldn't allocate new node! %i\n",
               new_list_id);
            return NIL;
         }
         // Access list_nodes directly, list node already confirmed to exist.
         new_node = &list_nodes[first_list_id];
         new_node->rest.v.tag = TAG_LIST;
         new_node->rest.v.data = rest_list_id;
         new_node = new_next;

         new_node->first.int_val = l->first.int_val;
      }

      // Save list node refs, so we can get them next iteration after the allocation.
      first_list_id = rest_list_id;
      l_list_id = l->rest.v.data;
   }
   new_node->rest.int_val = NIL;

   return new_list_id;
}

int InsertListElem(int n,int list_id,val_type new_val)
{
   int new_list_id, temp_list_id = -1;
   list_node *l, *prev = NULL, *new_node;

   if (n == 0)
   {
      bprintf("InsertListElem given invalid list element %i, returning old list\n",
         n);
      return list_id;
   }

   // If the element we're adding is a list, get the list node now before
   // allocating another node, in case memory is resized.
   if (new_val.v.tag == TAG_LIST)
      temp_list_id = new_val.v.data;
   // Allocate first, so a resize doesn't clobber list references.
   new_list_id = AllocateListNode();
   new_node = GetListNodeByID(new_list_id);
   if (!new_node)
   {
      bprintf("InsertListElem couldn't allocate new node! %i\n",
         new_list_id);
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
         new_node->rest.int_val = NIL;

         // Use temp id if element we're adding is a list, in case allocate
         // resized memory.
         if (temp_list_id >= 0)
         {
            new_node->first.v.tag = TAG_LIST;
            new_node->first.v.data = temp_list_id;
         }
         else
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

   // This node is inserted in position of the existing one, so use its rest data.
   // Points to the old node.
   new_node->rest.v.data = prev->rest.v.data;
   new_node->rest.v.tag = TAG_LIST;

   // Use temp id if element we're adding is a list, in case allocate
   // resized memory.
   if (temp_list_id >= 0)
   {
      new_node->first.v.tag = TAG_LIST;
      new_node->first.v.data = temp_list_id;
   }
   else
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

// Next 6 functions deal with sending a message to objects in a list.

// ret_false is True if we return on a single false return from SendBlakodMessage.
int SendListMessage(int list_id, bool ret_false, int message_id,
                    int num_parms, parm_node parms[])
{
   list_node *l;
   int obj_id = INVALID_OBJECT, return_int = True;
   val_type ret_val;

   l = GetListNodeByID(list_id);
   if (!l)
   {
      bprintf("SendListMessage can't find list node %i\n", list_id);
      return return_int;
   }

   if (l->first.v.tag == TAG_OBJECT)
   {
      ret_val.int_val = SendBlakodMessage(l->first.v.data, message_id, num_parms, parms);
      if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
      {
         if (ret_false)
            return False;
         return_int = False;
      }
   }
   /* Sending to built-in objects in a list disabled for now.
   else if (l->first.v.tag == TAG_INT)
   {
      // Can send to built-in objects using constants.
      obj_id = GetBuiltInObjectID(l->first.v.data);
      if (obj_id > INVALID_OBJECT)
      {
         ret_val.int_val = SendBlakodMessage(obj_id, message_id, num_parms, parms);
         if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
         {
            if (ret_false)
               return False;
            return_int = False;
         }
      }
   }*/

   while (l && l->rest.v.tag != TAG_NIL)
   {
      l = GetListNodeByID(l->rest.v.data);
      if (!l)
      {
         bprintf("SendListMessage got invalid list node somewhere in list %i.\n",
            list_id);
         return return_int;
      }

      if (l->first.v.tag == TAG_OBJECT)
      {
         ret_val.int_val = SendBlakodMessage(l->first.v.data, message_id, num_parms, parms);
         if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
         {
            if (ret_false)
               return False;
            return_int = False;
         }
      }
      /* Sending to built-in objects in a list disabled for now.
      else if (l->first.v.tag == TAG_INT)
      {
         // Can send to built-in objects using constants.
         obj_id = GetBuiltInObjectID(l->first.v.data);
         if (obj_id > INVALID_OBJECT)
         {
            ret_val.int_val = SendBlakodMessage(obj_id, message_id, num_parms, parms);
            if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
            {
               if (ret_false)
                  return False;
               return_int = False;
            }
         }
      }*/
   }

   return return_int;
}

// ret_false is True if we return on a single false return from SendBlakodMessage.
int SendFirstListMessage(int list_id, bool ret_false, int message_id,
                         int num_parms, parm_node parms[])
{
   list_node *l, *first;
   int return_int = True, obj_id = INVALID_OBJECT;
   val_type ret_val;

   l = GetListNodeByID(list_id);
   if (!l)
   {
      bprintf("SendFirstListMessage can't find list node %i\n", list_id);
      return return_int;
   }

   if (l->first.v.tag != TAG_LIST)
      bprintf("SendFirstListMessage called on invalid sublist %i %i.\n",
         l->first.v.tag, l->first.v.data);
   else
   {
      first = GetListNodeByID(l->first.v.data);
      if (!first)
         bprintf("SendFirstListMessage can't find sub-list node %i %i\n",
            l->first.v.tag, l->first.v.data);
      else if (first->first.v.tag == TAG_OBJECT)
      {
         ret_val.int_val = SendBlakodMessage(first->first.v.data, message_id, num_parms, parms);
         if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
         {
            if (ret_false)
               return False;
            return_int = False;
         }
      }
      /* Sending to built-in objects in a list disabled for now.
      else if (first->first.v.tag == TAG_INT)
      {
         // Can send to built-in objects using constants.
         obj_id = GetBuiltInObjectID(first->first.v.data);
         if (obj_id > INVALID_OBJECT)
         {
            ret_val.int_val = SendBlakodMessage(obj_id, message_id, num_parms, parms);
            if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
            {
               if (ret_false)
                  return False;
               return_int = False;
            }
         }
      }*/
   }

   while (l && l->rest.v.tag != TAG_NIL)
   {
      l = GetListNodeByID(l->rest.v.data);
      if (!l)
      {
         bprintf("SendFirstListMessage got invalid node somewhere in list %i.\n",
            list_id);
         return return_int;
      }

      if (l->first.v.tag != TAG_LIST)
         bprintf("SendFirstListMessage called on invalid sublist %i %i.\n",
            l->first.v.tag, l->first.v.data);
      else
      {
         first = GetListNodeByID(l->first.v.data);
         if (!first)
         {
            bprintf("SendFirstListMessage can't find sub-list node %i %i\n",
               l->first.v.tag, l->first.v.data);
            continue;
         }

         if (first->first.v.tag == TAG_OBJECT)
         {
            ret_val.int_val = SendBlakodMessage(first->first.v.data, message_id, num_parms, parms);
            if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
            {
               if (ret_false)
                  return False;
               return_int = False;
            }
         }
         /* Sending to built-in objects in a list disabled for now.
         else if (first->first.v.tag == TAG_INT)
         {
            // Can send to built-in objects using constants.
            obj_id = GetBuiltInObjectID(first->first.v.data);
            if (obj_id > INVALID_OBJECT)
            {
               ret_val.int_val = SendBlakodMessage(obj_id, message_id, num_parms, parms);
               if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
               {
                  if (ret_false)
                     return False;
                  return_int = False;
               }
            }
         }*/
      }
   }

   return return_int;
}

// ret_false is True if we return on a single false return from SendBlakodMessage.
int SendNthListMessage(int list_id, int position, bool ret_false,
                       int message_id, int num_parms, parm_node parms[])
{
   list_node *l;
   int obj_id = INVALID_OBJECT, return_int = True;
   val_type obj_val, ret_val;

   l = GetListNodeByID(list_id);
   if (!l)
   {
      bprintf("SendNthListMessage can't find list node %i\n", list_id);
      return return_int;
   }

   if (l->first.v.tag != TAG_LIST)
      bprintf("SendNthListMessage called on invalid sublist %i %i.\n",
         l->first.v.tag, l->first.v.data);
   else
   {
      obj_val.int_val = Nth(position, l->first.v.data);
      if (obj_val.v.tag == TAG_OBJECT)
      {
         ret_val.int_val = SendBlakodMessage(obj_val.v.data, message_id, num_parms, parms);
         if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
         {
            if (ret_false)
               return False;
            return_int = False;
         }
      }
      /* Sending to built-in objects in a list disabled for now.
      else if (obj_val.v.tag == TAG_INT)
      {
         // Can send to built-in objects using constants.
         obj_id = GetBuiltInObjectID(obj_val.v.data);
         if (obj_id > INVALID_OBJECT)
         {
            ret_val.int_val = SendBlakodMessage(obj_id, message_id, num_parms, parms);
            if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
            {
               if (ret_false)
                  return False;
               return_int = False;
            }
         }
      }*/
   }

   while (l && l->rest.v.tag != TAG_NIL)
   {
      l = GetListNodeByID(l->rest.v.data);
      if (!l)
      {
         bprintf("SendNthListMessage got invalid list node somewhere in list %i\n",
            list_id);
         return return_int;
      }
      if (l->first.v.tag != TAG_LIST)
      {
         bprintf("SendNthListMessage called on invalid sub-list %i %i.\n",
            l->first.v.tag, l->first.v.data);
         continue;
      }

      obj_val.int_val = Nth(position, l->first.v.data);
      if (obj_val.v.tag == TAG_OBJECT)
      {
         ret_val.int_val = SendBlakodMessage(obj_val.v.data, message_id, num_parms, parms);
         if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
         {
            if (ret_false)
               return False;
            return_int = False;
         }
      }
      /* Sending to built-in objects in a list disabled for now.
      else if (obj_val.v.tag == TAG_INT)
      {
         // Can send to built-in objects using constants.
         obj_id = GetBuiltInObjectID(obj_val.v.data);
         if (obj_id > INVALID_OBJECT)
         {
            ret_val.int_val = SendBlakodMessage(obj_id, message_id, num_parms, parms);
            if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
            {
               if (ret_false)
                  return False;
               return_int = False;
            }
         }
      }*/
   }

   return return_int;
}

// ret_false is True if we return on a single false return from SendBlakodMessage.
int SendListMessageByClass(int list_id, int class_id, bool ret_false,
                           int message_id, int num_parms, parm_node parms[])
{
   list_node *l;
   int obj_id = INVALID_OBJECT, return_int = True;
   val_type ret_val;
   object_node *o = NULL;
   class_node *c;

   l = GetListNodeByID(list_id);
   if (!l)
   {
      bprintf("SendListMessageByClass can't find list node %i\n", list_id);
      return return_int;
   }

   if (l->first.v.tag == TAG_OBJECT)
      o = GetObjectByID(l->first.v.data);
   /* Sending to built-in objects in a list disabled for now.
   else if (l->first.v.tag == TAG_INT)
   {
      // Can send to built-in objects using constants.
      obj_id = GetBuiltInObjectID(l->first.v.data);
      if (obj_id > INVALID_OBJECT)
         o = GetObjectByID(obj_id);
   }*/

   if (o != NULL)
   {
      c = GetClassByID(o->class_id);
      if (c == NULL)
      {
         bprintf("SendListMessageByClass can't find class %i, DIE totally\n",
            o->class_id);
         FlushDefaultChannels();
         return return_int;
      }
      do
      {
         if (c->class_id == class_id)
         {
            ret_val.int_val = SendBlakodMessage(o->object_id, message_id, num_parms, parms);
            o = NULL;
            if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
            {
               if (ret_false)
                  return False;
               return_int = False;
            }
            break;
         }
         c = c->super_ptr;
      } while (c != NULL);
   }

   while (l && l->rest.v.tag != TAG_NIL)
   {
      l = GetListNodeByID(l->rest.v.data);
      if (!l)
      {
         bprintf("SendListMessageByClass got invalid list node in list %i\n",
            list_id);
         return return_int;
      }

      if (l->first.v.tag == TAG_OBJECT)
         o = GetObjectByID(l->first.v.data);
      /* Sending to built-in objects in a list disabled for now.
      else if (l->first.v.tag == TAG_INT)
      {
         // Can send to built-in objects using constants.
         obj_id = GetBuiltInObjectID(l->first.v.data);
         if (obj_id > INVALID_OBJECT)
            o = GetObjectByID(obj_id);
      }*/

      if (o == NULL)
         continue;

      c = GetClassByID(o->class_id);
      if (c == NULL)
      {
         bprintf("SendListMessageByClass can't find class %i, DIE totally\n",
            o->class_id);
         FlushDefaultChannels();
         return return_int;
      }
      do
      {
         if (c->class_id == class_id)
         {
            ret_val.int_val = SendBlakodMessage(o->object_id, message_id, num_parms, parms);
            o = NULL;
            if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
            {
               if (ret_false)
                  return False;
               return_int = False;
            }
            break;
         }
         c = c->super_ptr;
      } while (c != NULL);
   }

   return return_int;
}

// ret_false is True if we return on a single false return from SendBlakodMessage.
int SendFirstListMessageByClass(int list_id, int class_id, bool ret_false,
                                int message_id, int num_parms, parm_node parms[])
{
   list_node *l, *first;
   int obj_id = INVALID_OBJECT, return_int = True;
   val_type ret_val;
   object_node *o = NULL;
   class_node *c;

   l = GetListNodeByID(list_id);
   if (!l)
   {
      bprintf("SendFirstListMessageByClass can't find list node %i\n", list_id);
      return return_int;
   }

   if (l->first.v.tag != TAG_LIST)
   {
      bprintf("SendFirstListMessageByClass called on invalid sub-list %i %i.\n",
         l->first.v.tag, l->first.v.data);
   }
   else
   {
      first = GetListNodeByID(l->first.v.data);
      if (!first)
      {
         bprintf("SendFirstListMessageByClass can't find list node %i\n",
            l->first.v.tag, l->first.v.data);
      }
      else if (first->first.v.tag == TAG_OBJECT)
         o = GetObjectByID(first->first.v.data);
      /* Sending to built-in objects in a list disabled for now.
      else if (first->first.v.tag == TAG_INT)
      {
         // Can send to built-in objects using constants.
         obj_id = GetBuiltInObjectID(first->first.v.data);
         if (obj_id > INVALID_OBJECT)
            o = GetObjectByID(obj_id);
      }*/

      if (o != NULL)
      {
         c = GetClassByID(o->class_id);
         if (c == NULL)
         {
            bprintf("SendFirstListMessageByClass can't find class %i, DIE totally\n",
               o->class_id);
            FlushDefaultChannels();
            return return_int;
         }
         do
         {
            if (c->class_id == class_id)
            {
               ret_val.int_val = SendBlakodMessage(o->object_id, message_id, num_parms, parms);
               o = NULL;
               if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
               {
                  if (ret_false)
                     return False;
                  return_int = False;
               }
               break;
            }
            c = c->super_ptr;
         } while (c != NULL);
      }
   }

   while (l && l->rest.v.tag != TAG_NIL)
   {
      l = GetListNodeByID(l->rest.v.data);
      if (!l)
      {
         bprintf("SendFirstListMessageByClass got invalid list node in list %i\n",
            list_id);
         return return_int;
      }

      if (l->first.v.tag != TAG_LIST)
      {
         bprintf("SendFirstListMessageByClass called on invalid sub-list %i %i.\n",
            l->first.v.tag, l->first.v.data);
         continue;
      }

      first = GetListNodeByID(l->first.v.data);
      if (!first)
      {
         bprintf("SendFirstListMessageByClass can't find list node %i %i\n",
            l->first.v.tag, l->first.v.data);
         continue;
      }

      if (first->first.v.tag == TAG_OBJECT)
         o = GetObjectByID(first->first.v.data);
      /* Sending to built-in objects in a list disabled for now.
      else if (first->first.v.tag == TAG_INT)
      {
         // Can send to built-in objects using constants.
         obj_id = GetBuiltInObjectID(first->first.v.data);
         if (obj_id > INVALID_OBJECT)
            o = GetObjectByID(obj_id);
      }*/

      if (o == NULL)
         continue;

      c = GetClassByID(o->class_id);
      if (c == NULL)
      {
         bprintf("SendFirstListMessageByClass can't find class %i, DIE totally\n",
            o->class_id);
         FlushDefaultChannels();
         return return_int;
      }
      do
      {
         if (c->class_id == class_id)
         {
            ret_val.int_val = SendBlakodMessage(o->object_id, message_id, num_parms, parms);
            o = NULL;
            if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
            {
               if (ret_false)
                  return False;
               return_int = False;
            }
            break;
         }
         c = c->super_ptr;
      } while (c != NULL);
   }

   return return_int;
}

// ret_false is True if we return on a single false return from SendBlakodMessage.
int SendNthListMessageByClass(int list_id, int position, int class_id, bool ret_false,
                              int message_id, int num_parms, parm_node parms[])
{
   list_node *l;
   int obj_id = INVALID_OBJECT, return_int = True;
   val_type ret_val, obj_val;
   object_node *o = NULL;
   class_node *c;

   l = GetListNodeByID(list_id);
   if (!l)
   {
      bprintf("SendNthListMessageByClass can't find list node %i\n", list_id);
      return return_int;
   }

   if (l->first.v.tag != TAG_LIST)
   {
      bprintf("SendNthListMessageByClass called on invalid list %i %i.\n",
         l->first.v.tag, l->first.v.data);
   }
   else
   {
      obj_val.int_val = Nth(position, l->first.v.data);

      if (obj_val.v.tag == TAG_OBJECT)
         o = GetObjectByID(obj_val.v.data);
      /* Sending to built-in objects in a list disabled for now.
      else if (obj_val.v.tag == TAG_INT)
      {
         // Can send to built-in objects using constants.
         obj_id = GetBuiltInObjectID(obj_val.v.data);
         if (obj_id > INVALID_OBJECT)
            o = GetObjectByID(obj_id);
      }*/

      if (o != NULL)
      {
         c = GetClassByID(o->class_id);
         if (c == NULL)
         {
            bprintf("SendNthListMessageByClass can't find class %i, DIE totally\n",
               o->class_id);
            FlushDefaultChannels();
            return return_int;
         }
         do
         {
            if (c->class_id == class_id)
            {
               ret_val.int_val = SendBlakodMessage(o->object_id, message_id, num_parms, parms);
               o = NULL;
               if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
               {
                  if (ret_false)
                     return False;
                  return_int = False;
               }
               break;
            }
            c = c->super_ptr;
         } while (c != NULL);
      }
   }

   while (l && l->rest.v.tag != TAG_NIL)
   {
      l = GetListNodeByID(l->rest.v.data);
      if (!l)
      {
         bprintf("SendNthListMessageByClass got invalid list node in list %i\n",
            list_id);
         return return_int;
      }

      if (l->first.v.tag != TAG_LIST)
      {
         bprintf("SendNthListMessageByClass called on invalid list.\n");
         continue;
      }

      obj_val.int_val = Nth(position, l->first.v.data);

      if (obj_val.v.tag == TAG_OBJECT)
         o = GetObjectByID(obj_val.v.data);
      /* Sending to built-in objects in a list disabled for now.
      else if (obj_val.v.tag == TAG_INT)
      {
         // Can send to built-in objects using constants.
         obj_id = GetBuiltInObjectID(obj_val.v.data);
         if (obj_id > INVALID_OBJECT)
            o = GetObjectByID(obj_id);
      }*/

      if (o == NULL)
         continue;

      c = GetClassByID(o->class_id);
      if (c == NULL)
      {
         bprintf("SendNthListMessageByClass can't find class %i, DIE totally\n",
            o->class_id);
         FlushDefaultChannels();
         return return_int;
      }
      do
      {
         if (c->class_id == class_id)
         {
            ret_val.int_val = SendBlakodMessage(o->object_id, message_id, num_parms, parms);
            o = NULL;
            if (ret_val.v.tag == TAG_INT && ret_val.v.data == False)
            {
               if (ret_false)
                  return False;
               return_int = False;
            }
            break;
         }
         c = c->super_ptr;
      } while (c != NULL);
   }

   return return_int;
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
