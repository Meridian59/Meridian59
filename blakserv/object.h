// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * object.h
 *
 */

#ifndef _OBJECT_H
#define _OBJECT_H

#define INIT_OBJECTS 1000000

typedef struct
{
   int id;
   val_type val;
} prop_type;

typedef struct
{
   int object_id;
   int class_id;
   Bool deleted;
   int garbage_ref;
   int num_props; /* used by garbage collect */
   prop_type *p;
} object_node;

void InitObject(void);
void ResetObject(void);
void ClearObject(void);
int GetObjectsUsed(void);
int CreateObject(int class_id,int num_parms,parm_node parms[]);
Bool LoadObject(int object_id,char *class_name);
void DeleteBlakodObject(int object_id);
object_node * GetObjectByID(int object_id);
object_node * GetObjectByIDQuietly(int object_id);
Bool IsObjectByID(int object_id);
object_node * GetObjectByIDEvenDeleted(int object_id);
Bool SetObjectPropertyByName(int object_id,char *prop_name,val_type val);

void ForEachObject(void (*callback_func)(object_node *o));
void MoveObject(int dest_id,int source_id);
void SetNumObjects(int new_num_objects);

#endif
