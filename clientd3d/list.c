// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * list.c:  Implement a linked-list abstraction using void pointers.
 *   This implementation could do without the "last" pointer, and it
 *   might be better to maintain allocation of list nodes ourselves.
 */

#include "client.h"
#include <vector>

/************************************************************************/
/* List abstraction: use void pointers to point to data field. 
 * "last" pointer of first node points to last node; others are meaningless.
 */
/************************************************************************/
/* list_create: create and return new list with given data
 */
list_type list_create(void *newdata)
{
   list_type l = (list_type) SafeMalloc(sizeof(list_struct));
   l->data = newdata;
   l->last = l;
   l->next = NULL;
   return l;
}
/************************************************************************/
/* list_add_item: add a node with data field newdata to end of l.  If l is NULL, 
 *              creates a new list and returns it.
 */
list_type list_add_item(list_type l, void *newdata)
{
   if (l == NULL)
      l = list_create(newdata);
   else
   {
      list_type newnode = (list_type) SafeMalloc(sizeof(list_struct));
      newnode->data = newdata;
      newnode->next = NULL;
      l->last->next = newnode;
      l->last = newnode;
   }
   return l;
}
/************************************************************************/
/* list_add_first: add a node with data field newdata to front of l.  If l is NULL, 
 *              creates a new list and returns it.
 */
list_type list_add_first(list_type l, void *newdata)
{
   if (l == NULL)
      l = list_create(newdata);
   else
   {
      list_type newnode = (list_type) SafeMalloc(sizeof(list_struct));
      newnode->data = newdata;
      newnode->next = l;
      newnode->last = l->last;
      l = newnode;
   }
   return l;
}
/************************************************************************/
/* 
 * list_append: Return the result of appending l2 to the end of l1.
 *    l1 is modified; l2 is not.
 */
list_type list_append(list_type l1, list_type l2)
{
   if (l1 == NULL)
      return l2;

   if (l2 == NULL)
      return l1;

   l1->last->next = l2;
   l1->last = l2->last;
   return l1;
}
/************************************************************************/
/*
 * list_delete_item: delete the node with the given data element from
 *    the given list and free the list node.  If the item is not in the list, nothing is done.
 *    Returns the new list, since we have to set it to NULL if there
 *    was only one element in the list.
 *    Note that the data associated with the list item itself is NOT freed.
 *    The compare function is used to see if two data elements are equal.
 *    It should return nonzero iff its two arguments are equal.
 */
list_type list_delete_item(list_type l, void *deldata, CompareProc compare)
{
   list_type temp, prev = l, next;

   if (l == NULL) 
      return NULL;
   
   if ((*compare)(deldata, l->data))
   {
      /* Reset "last" pointer, if appropriate */
      if (l->next != NULL)
	 l->next->last = l->last;
      next = l->next;
      SafeFree(l);
      return next;
   }

   for (temp = l->next; temp != NULL; temp = temp->next)
   {
      if ((*compare)(deldata, temp->data))
      {
	 /* If we delete last item of list, reset "last" pointer */
	 if (temp->next == NULL)
	    l->last = prev;
	 prev->next = temp->next;
	 SafeFree(temp);
	 return l;
      }
      prev = temp;
   }
   /* If not found, just return original list */
   return l;
}
/************************************************************************/
/*
 * list_delete_first: delete the first node from the given list; return
 *    the newly modified list.  First list node is freed.
 */
list_type list_delete_first(list_type l)
{
   list_type next;

   if (l == NULL)
      return NULL;

   if (l->next == NULL)
   {
      SafeFree(l);
      return NULL;
   }

   l->next->last = l->last;
   next = l->next;
   SafeFree(l);
   return next;
}
/************************************************************************/
/*
 * list_delete_last: delete the last node from the given list; return
 *    the newly modified list.  Last list node is freed.
 */
list_type list_delete_last(list_type l)
{
   list_type prev, temp;

   if (l == NULL)
      return NULL;

   if (l->next == NULL)
   {
      SafeFree(l);
      return NULL;
   }
   
   prev = l;
   temp = l->next;
   while (temp->next != NULL)
   {
      prev = temp;
      temp = temp->next;
   }
   
   l->last = prev;
   prev->next = NULL;
   SafeFree(temp);
   return l;
}
/************************************************************************/
/*
 * list_first_item: Return the first item in the given list.
 */
void *list_first_item(list_type l)
{
   if (l == NULL)
      return NULL;

   return l->data;
}
/************************************************************************/
/*
 * list_last_item: Return the last item in the given list.
 */
void *list_last_item(list_type l)
{
   if (l == NULL)
      return NULL;

   return l->last->data;
}
/************************************************************************/
/*
 * list_nth_item: Return the nth item in the given list, or NULL if
 *   n > list_length(l) - 1.  Start counting from 0.
 */
void *list_nth_item(list_type l, int n)
{
   int i;

   for (i = 0; i < n; i++)
   {
      if (l == NULL)
	 return NULL;
      l = l->next;
   }

   if (l == NULL)
      return NULL;
   return l->data;
}
/************************************************************************/
/*
 * list_find_item: Find and return the given item in the given list.  The
 *   compare function is used to determine if two items are equal.
 *   Returns NULL if item not found.
 */
void *list_find_item(list_type l, void *data, CompareProc compare)
{
   while (l != NULL)
   {
      if ( (*compare)(data, l->data))
	 return l->data;
      l = l->next;
   }
   return NULL;
}
/************************************************************************/
/*
 * list_get_position: Find and return the position of an item in the given list.
 *   The compare function is used to determine if two items are equal.
 *   Returns -1 if item not found.
 */
int list_get_position(list_type l, void *data, CompareProc compare)
{
   int n = 0;

   while (l != NULL)
   {
      if ( (*compare)(data, l->data))
         return n;
      l = l->next;
      n++;
   }
   return -1;
}
/************************************************************************/
/* list_length: return length of given list.
 */
int list_length(list_type l)
{
   int length = 0;

   while (l != NULL)
   {
      length++;
      l = l->next;
   }
   return length;
}
/************************************************************************/
/* list_delete: Free the memory associated with a list, and return NULL.
 *    One would typically do l = list_delete(l) to delete l.
 *    Note that the data associated with each list item is NOT freed.
 */
list_type list_delete(list_type l)
{
   list_type next;

   while (l != NULL)
   {
      next = l->next;
      SafeFree(l);
      l = next;
   }
   return NULL;
}
/************************************************************************/
/* list_destroy: Free the memory associated with a list AND its elements,
 *    and return NULL.
 *    Note that the data associated with each list item IS freed.
 */
list_type list_destroy(list_type l)
{
   list_type next;

   while (l != NULL)
   {
      next = l->next;
      SafeFree(l->data);
      SafeFree(l);
      l = next;
   }
   return NULL;
}
/************************************************************************/
/*
 * list_move_to_front:  Find the given item in the given list, and
 *   move it to the front of the list.
 *   Returns the new list, NOT the given item.
 */
list_type list_move_to_front(list_type l, void *data, CompareProc compare)
{
   list_type temp, prev;

   if (l == NULL)
      return NULL;

   /* Check first node */
   if ( (*compare)(data, l->data))
   {
      return l;
   }

   prev = l;
   temp = l->next;

   while (temp != NULL)
   {
      if ( (*compare)(data, temp->data))
      {
	 /* Move node to front of list */
	 prev->next = temp->next;
	 temp->next = l;

	 /* Check if we're moving last node */
	 if (l->last == temp)
	    temp->last = prev;
	 else temp->last = l->last;

	 return temp;
      }
      prev = temp;
      temp = temp->next;
   }
   return l;
}
/************************************************************************/
/*
 * list_move_item:  Given a list and source and dest indices, which must
 *   be in range, move the item at the source index to just before the dest index.
 */
list_type list_move_item(list_type l, int source_index, int dest_index)
{
   // Anything to do?
   if (source_index == dest_index)
      return l;

   // Copy all list data into a vector
   std::vector<void *> list_contents;
   list_type node = l;
   while (node != NULL)
   {
     list_contents.push_back(node->data);
     node = node->next;
   }
   
   // Do the move on the vector
   list_contents.insert(list_contents.begin() + dest_index, list_contents[source_index]);
   int pos_to_erase = source_index + (source_index > dest_index ? 1 : 0);
   list_contents.erase(list_contents.begin() + pos_to_erase);
   
   // Copy the vector back to the list
   node = l;
   for (auto data : list_contents)
   {
     node->data = data;
     node = node->next;
   }

   return l;
}
/************************************************************************/
/*
 * list_add_sorted_item:  Add given item to list in increasing sorted order.
 *   Requires that list is already sorted.
 */
list_type list_add_sorted_item(list_type l, void *newdata, SortProc compare)
{
   list_type temp, newnode, prev;

   if (l == NULL)
      return list_create(newdata);

   newnode = (list_type) SafeMalloc(sizeof(list_struct));
   newnode->data = newdata;
   newnode->next = NULL;

   /* Check if less than first item */
   if ((*compare)(newdata, l->data) < 0)
   {
      newnode->next = l;
      newnode->last = l->last;

      return newnode;
   }

   prev = l;
   for (temp = l->next; temp != NULL; temp = temp->next)
   {
      if ((*compare)(newdata, temp->data) < 0)
	 break;
      prev = temp;
   }

   prev->next = newnode;
   newnode->next = temp;

   /* Check if added to end of list */
   if (temp == NULL)
      l->last = newnode;

   return l;
      
}
