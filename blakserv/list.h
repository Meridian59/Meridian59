// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * list.h
 *
 */

#ifndef _LIST_H
#define _LIST_H

#define INIT_LIST_NODES (500000)

typedef struct
{
   val_type first;
   val_type rest;
   int garbage_ref;
} list_node;

void InitList(void);
void ResetList(void);
void ClearList(void);
int GetListNodesUsed(void);
Bool LoadList(int list_id,val_type first,val_type rest);
list_node * GetListNodeByID(int list_id);
Bool IsListNodeByID(int list_id);
blak_int First(int list_id);
blak_int Rest(int list_id);
int Cons(val_type source,val_type dest);
int Length(int list_id);
blak_int Nth(int n,int list_id);
int SetFirst(int list_id,val_type new_val);
int SetNth(int n,int list_id,val_type new_val);
int FindListElem(val_type list_id,val_type list_elem);
blak_int DelListElem(val_type list_id,val_type list_elem);
// Move element at index n to index m, both 1-based.
// m may be up to 1 more than list length (meaning move to the end of the list).
void MoveListElem(val_type list_id, val_type n, val_type m);

void ForEachListNode(void (*callback_func)(list_node *l,int list_id));
void MoveListNode(int dest_id,int source_id);
void SetNumListNodes(int new_num_nodes);



#endif
