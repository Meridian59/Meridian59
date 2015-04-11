// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * list.h:  Header file for list.c
 */

#ifndef _LIST_H
#define _LIST_H

/* list_type is used as a generic list */
typedef struct _node {
   void *data;
   struct _node *last;  /* Points to last node in list */
   struct _node *next;
} *list_type, list_struct;

/* procedure to compare two list entries:  return nonzero iff entries are equal */
typedef Bool (*CompareProc)(void *, void *);

/* procedure to compare two list entries:  return zero iff entries are equal,
 *  < 0 if first entry is smaller, > 0 if first entry is larger.
 * (This will cause the list to be sorted in increasing order */
typedef int (*SortProc)(void *, void *);


M59EXPORT list_type list_create(void *newdata);
M59EXPORT list_type list_add_item(list_type l, void *newdata);
M59EXPORT list_type list_add_first(list_type l, void *newdata);
M59EXPORT list_type list_append(list_type l1, list_type l2);
M59EXPORT list_type list_delete_item(list_type l, void *deldata, CompareProc compare);
M59EXPORT list_type list_delete_first(list_type l);
M59EXPORT list_type list_delete_last(list_type l);
M59EXPORT void     *list_first_item(list_type l);
M59EXPORT void     *list_last_item(list_type l);
M59EXPORT void     *list_nth_item(list_type l, int n);
M59EXPORT void     *list_find_item(list_type l, void *deldata, CompareProc compare);
M59EXPORT int       list_get_position(list_type l, void *data, CompareProc compare);
M59EXPORT list_type list_delete(list_type l);
M59EXPORT list_type list_destroy(list_type l);
M59EXPORT int       list_length(list_type l);
M59EXPORT list_type list_move_to_front(list_type l, void *data, CompareProc compare);
M59EXPORT list_type list_move_to_nth(list_type l, void *data1, void *data2, CompareProc compare);
M59EXPORT list_type list_add_sorted_item(list_type l, void *newdata, SortProc compare);

#endif /* #ifndef _LIST_H */
