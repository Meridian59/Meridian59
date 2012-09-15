// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * sort.c:  Sort parameter lists in message handler headers and calls.
 *    These are sorted to improve server performance.
 */

#include "blakcomp.h"

static list_type InsertionSort(list_type list, CompareProc compare);

/************************************************************************/
int CompareParameters(void *param1, void *param2)
{
   param_type p1 = (param_type) param1;
   param_type p2 = (param_type) param2;

   return (p1->lhs->idnum < p2->lhs->idnum);
}
/************************************************************************/
int CompareArguments(void *arg1, void *arg2)
{
   arg_type a1 = (arg_type) arg1;
   arg_type a2 = (arg_type) arg2;

   if (a1->type != ARG_SETTING || a2->type != ARG_SETTING)
   {
      simple_error("Internal error--bad argument type in CompareArguments");
      return 0;
   }

   return (a1->value.setting_val->id->idnum < a2->value.setting_val->id->idnum);
}
/************************************************************************/
int CompareMessageHandlers(void *hand1, void *hand2)
{
   message_handler_type h1 = (message_handler_type) hand1;
   message_handler_type h2 = (message_handler_type) hand2;

   return (h1->header->message_id->idnum < h2->header->message_id->idnum);
}
/************************************************************************/
/* 
 * SortParameterList: Sort the given list in place, and return it.
 *   params must be a list of param_type; that is, a list of message
 *   handler parameters.
 */
list_type SortParameterList(list_type params)
{
   return InsertionSort(params, CompareParameters);
}
/************************************************************************/
/* 
 * SortArgumentList: Sort the given list in place, and return it.
 *   args must be a list of arg_type; that is, a list of arguments to a 
 *   function call.
 */
list_type SortArgumentList(list_type args)
{
   list_type ptr, prev;
   arg_type arg;

   /* Only sort the settings--leave other arguments as they are */
   prev = NULL;
   for (ptr = args; ptr != NULL; prev = ptr, ptr = ptr->next)
   {
      arg = (arg_type) ptr->data;
      if (arg->type == ARG_SETTING)
	 break;
   }

   /* If no settings, don't sort */
   if (ptr == NULL)
      return args;

   /* Sort settings and reattach to end of list */
   ptr = InsertionSort(ptr, CompareArguments);

   if (prev != NULL)
      prev->next = ptr;
   else args = ptr;

   /* Fix up "last" pointer */
   args->last = ptr->last;
      
   return args;
}
/************************************************************************/
/*
 * SortMessageHandlerList: Sort the given list in place, and return it.
 *   handlers must be a list of message_handler_type; that is, a list of 
 *   a class's message handlers.
 */
list_type SortMessageHandlerList(list_type handlers)
{
   return InsertionSort(handlers, CompareMessageHandlers);
}
/************************************************************************/
/*
 * InsertionSort:  Perform an in-place insertion sort on the given list,
 *   and return it.
 *   compare must return nonzero if its first argument is less than
 *    its second argument for the list to be sorted in increasing order.
 *
 */
list_type InsertionSort(list_type list, CompareProc compare)
{
   list_type item, next_item, ptr, prev;

   if (list == NULL)
   {
      return list;
   }

   if (list->next == NULL)
   {
      list->last = list;
      return list;
   }

   item = list->next;
   list->next = NULL;

   for (; item != NULL; item = next_item)
   {
      next_item = item->next;

      /* Find place to insert this element */
      prev = NULL;
      for (ptr = list; ptr != NULL; prev = ptr, ptr = ptr->next)
      {
	 if ((*compare)(item->data, ptr->data))
	 {
	    /* Insert before current element */
	    item->next = ptr;
	    if (prev == NULL)
	       list = item;
	    else prev->next = item;
	    break;
	 }
      }
      
      /* If largest item, add to end */
      if (ptr == NULL)
      {
	 prev->next = item;
	 item->next = NULL;
      }      
   }

   /* Reset "last" pointer */
   for (ptr = list; ptr->next != NULL; ptr = ptr->next)
      ;
   list->last = ptr;

   return list;
}
