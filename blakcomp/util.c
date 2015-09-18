// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* util.c 
 * Utility procedures for Blakod compiler 
 */

#include "blakcomp.h"

/************************************************************************/
void *SafeMalloc(long bytes)
{
   void *temp = (void *) malloc(bytes);
   if (temp == NULL)
   {
      fprintf(stderr, "Out of memory!\n");
      exit(1);
   }

   return temp;
}
/************************************************************************/
void SafeFree(void *ptr)
{
   if (ptr == NULL)
   {
      fprintf(stderr, "Attempt to free null pointer!\n");
      return;
   }
   free(ptr);
}
/************************************************************************/
char *strtolower(char *s)
{
   char *p = s;
   while (*p)
   {
      *p = tolower(*p);
      p++;
   }
   return s;
}
/************************************************************************/
/* 
 * set_extension: Set newfile to filename with its extension set to the
 *    given string.
 */
void set_extension(char *newfile, const char *filename, const char *extension)
{
   char *ptr;

   strcpy(newfile, filename);

   ptr = strrchr(newfile, '\\');  /* Find last component of path */
   if (ptr == NULL)
      ptr = newfile;

   ptr = strchr(ptr, '.');
   if (ptr != NULL)
      *ptr = '\0';
   strcat(newfile, extension);
}
/************************************************************************/
/*
 * string_hash: return a number i s.t. 0 <= i < max based on given
 *      string.  This function was stolen from Bison.
 *      Max should be odd for best results.
 *  NOTE: It's case insensitive!!!
 */
int string_hash(const char *name, int max)
{
   const char *cp = name;
   int k = 0;
   while (*cp++)
   {
      char ch = tolower(*cp);
      k = ((k << 1) ^ (ch)) & 0x3fff;
   }
   
   return k % max;
}
/* List abstraction: use void pointers to point to data field. */
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
/* list_add_item: add a node with data field newndata to end of l.  If l is NULL, 
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
 *    the given list.  If the item is not in the list, nothing is done.
 *    Returns the new list, since we have to set it to NULL if there
 *    was only one element in the list.
 *    Note that the data associated with the list item is NOT freed.
 *    The compare function is used to see if two data elements are equal.
 *    It should return nonzero iff its two arguments are equal.
 */
list_type list_delete_item(list_type l, void *deldata, int (*compare)(void *, void *))
{
   list_type temp, prev = l;

   if (l == NULL) 
      return NULL;
   
   if ((*compare)(deldata, l->data))
   {
      /* Reset "last" pointer, if appropriate */
      if (l->next != NULL)
	 l->next->last = l->last;
      return l->next;
   }

   temp = l->next;
   while (temp != NULL)
   {
      if ((*compare)(deldata, temp->data))
      {
	 /* If we delete last item of list, reset "last" pointer */
	 if (temp->next == NULL)
	    l->last = prev;
	 prev->next = temp->next;
	 return l;
      }
      prev = temp;
      temp = temp->next;
   }
   /* If not found, just return original list */
   return l;
}
/************************************************************************/
/*
 * list_delete_first: delete the first node from the given list; return
 *    the newly modified list.
 */
list_type list_delete_first(list_type l)
{
   if (l == NULL || l->next == NULL)
      return NULL;

   l->next->last = l->last;
   return l->next;
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
 * list_last_item: Return the first item in the given list.
 */
void *list_last_item(list_type l)
{
   if (l == NULL)
      return NULL;

   return l->last->data;
}
/************************************************************************/
/*
 * list_find_item: Find and return the given item in the given list.  The
 *   compare function is used to determine if two items are equal.
 *   Returns NULL if item not found.
 */
void *list_find_item(list_type l, void *data, int (*compare)(void *, void *))
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
 * get_statement_line:  Return line number that should be associated with given statement.
 *   curline is current parser position, which should be at the end of the statement.
 */
int get_statement_line(stmt_type s, int curline)
{
   // Statements with subclauses should report the beginning of their block, not the end.
   // Subtract 1 from line of expression, because expression usually ends at a left brace
   // on the following line.
   switch (s->type)
   {
   case S_IF:
      return s->value.if_stmt_val->condition->lineno - 1;

   case S_FOR:
      return s->value.for_stmt_val->condition->lineno - 1;

   case S_WHILE:
      return s->value.while_stmt_val->condition->lineno - 1;

   default:
      return curline;
   }
}
