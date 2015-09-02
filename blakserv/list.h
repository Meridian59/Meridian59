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

#define INIT_LIST_NODES (10000000)

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
int First(int list_id);
int Rest(int list_id);
int AppendListElem(val_type source,val_type list_val);
int Cons(val_type source,val_type dest);
int Length(int list_id);
int Nth(int n,int list_id);
int IsListMatch(int list_one_id, int list_two_id);
int Last(int list_id);
int SetFirst(int list_id,val_type new_val);
int SetNth(int n,int list_id,val_type new_val);
int SwapListElem(int list_id,int elem_one,int elem_two);
int FindListElem(val_type list_id,val_type list_elem);
int InsertListElem(int n,int list_id,val_type new_val);
int DelListElem(val_type list_id,val_type list_elem);

void ForEachListNode(void (*callback_func)(list_node *l,int list_id));
void MoveListNode(int dest_id,int source_id);
void SetNumListNodes(int new_num_nodes);



#endif
