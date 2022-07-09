// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* util.h
 * Header file for util.c
 */

#ifndef _UTIL_H
#define _UTIL_H

/* list_type is used as a generic list anywhere in the parser */
typedef struct _node {
   void *data;
   struct _node *last;  /* Points to last node in list */
   struct _node *next;
} *list_type, list_struct;

void *SafeMalloc(long bytes);
void SafeFree(void *ptr);

char *strtolower(char *);
int string_hash(const char *name, int max);
void set_extension(char *newfile, const char *filename, const char *extension);

list_type list_create(void *newdata);
list_type list_add_item(list_type l, void *newdata);
list_type list_append(list_type l1, list_type l2);
list_type list_delete_item(list_type l, void *deldata, int (*compare)(void *, void *));
list_type list_delete_first(list_type l);
void     *list_first_item(list_type l);
void     *list_last_item(list_type l);
void     *list_find_item(list_type l, void *deldata, int (*compare)(void *, void *));
list_type list_delete(list_type l);
list_type list_destroy(list_type l);
int       list_length(list_type l);

#endif /* #ifndef _UTIL_H */
