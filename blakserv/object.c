// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * object.c
 *

 This module maintains a dynamically sized array with the Blakod
 objects. 

 */

#include "blakserv.h"

object_node *objects;
int num_objects,max_objects;

/* local function prototypes */
void SetObjectProperties(int object_id,class_node *c);

void InitObject()
{
   num_objects = 0;
   max_objects = INIT_OBJECTS;
   objects = (object_node *)AllocateMemory(MALLOC_ID_OBJECT,max_objects*sizeof(object_node));
}

void ResetObject()
{
   int i,old_objects;
   class_node *c;

   for (i=0;i<num_objects;i++)
   {
      if (!objects[i].deleted)
      {
	 c = GetClassByID(objects[i].class_id);
	 if (c == NULL)
	 {
	    eprintf("ResetObject can't find class id %i\n",objects[i].class_id);
	    return;
	 }

	 FreeMemory(MALLOC_ID_OBJECT_PROPERTIES,objects[i].p,
		    sizeof(prop_type)*(1+c->num_properties));
      }
   }
   old_objects = max_objects;
   num_objects = 0;  
   max_objects = INIT_OBJECTS;
   objects = (object_node *)
      ResizeMemory(MALLOC_ID_OBJECT,objects,old_objects*sizeof(object_node),
		   max_objects*sizeof(object_node));
}

/* ClearObject
 *
 * This is to clear away objects loaded in when there is an error loading
 * the game.
 */
void ClearObject()
{
   int old_objects;

   old_objects = max_objects;
   num_objects = 0;
   max_objects = INIT_OBJECTS;
   objects = (object_node *)
      ResizeMemory(MALLOC_ID_OBJECT,objects,old_objects*sizeof(object_node),
		   max_objects*sizeof(object_node));
}

int GetObjectsUsed()
{
   return num_objects;
}

int AllocateObject(int class_id)
{
   int old_objects;
   class_node *c;

   c = GetClassByID(class_id);
   if (c == NULL)
   {
      eprintf("AllocateObject can't find class id %i\n",class_id);
      return INVALID_OBJECT;
   }

   if (num_objects == max_objects)
   {
      old_objects = max_objects;
      max_objects = max_objects * 2;
      objects = (object_node *)
	 ResizeMemory(MALLOC_ID_OBJECT,objects,old_objects*sizeof(object_node),
		      max_objects*sizeof(object_node));
      lprintf("AllocateObject resized to %i objects\n",max_objects);
   }

   objects[num_objects].object_id = num_objects;
   objects[num_objects].class_id = class_id;
   objects[num_objects].deleted = False;
   objects[num_objects].num_props = 1 + c->num_properties;
   objects[num_objects].p = (prop_type *)AllocateMemory(MALLOC_ID_OBJECT_PROPERTIES,
							sizeof(prop_type)*(1+c->num_properties));

   return num_objects++;
}

/* charlie:  i dont want the error logs spammed by the object search routines */

object_node * GetObjectByIDQuietly(int object_id)
{
   if (object_id < 0 || object_id >= num_objects)
   {
      return NULL;
   }
   if (objects[object_id].deleted)
   {
      class_node* c;
      c = GetClassByID(objects[object_id].class_id);
      return NULL;
   }

   return &objects[object_id];
}

object_node * GetObjectByID(int object_id)
{
   if (object_id < 0 || object_id >= num_objects)
   {
      eprintf("GetObjectByID can't retrieve invalid object %i\n",object_id);
      return NULL;
   }
   if (objects[object_id].deleted)
   {
      class_node* c;
      c = GetClassByID(objects[object_id].class_id);
      if (c)
	 eprintf("GetObjectByID can't retrieve deleted OBJECT %i which was CLASS %s\n",object_id,c->class_name);
      else
	 eprintf("GetObjectByID can't retrieve deleted OBJECT %i, unknown or invalid class\n",object_id);
      return NULL;
   }
   return &objects[object_id];
}

Bool IsObjectByID(int object_id)
{
   if (object_id < 0 || object_id >= num_objects || objects[object_id].deleted)
      return False;

   return True;
}

object_node * GetObjectByIDEvenDeleted(int object_id)
{
   /* this is needed for garbage collection to reuse deleted ones */
   if (object_id < 0 || object_id >= num_objects)
   {
      eprintf("GetObjectByID can't retrieve invalid OBJECT %i\n",object_id);
      return NULL;
   }
   return &objects[object_id];
}

int CreateObject(int class_id,int num_parms,parm_node parms[])
{
   int new_object_id;
   class_node *c;

   new_object_id = AllocateObject(class_id);

   if (new_object_id == INVALID_OBJECT)
      return INVALID_OBJECT;
   
   /* set self = prop 0 */
   objects[new_object_id].p[0].id = 0; 
   objects[new_object_id].p[0].val.v.tag = TAG_OBJECT; 
   objects[new_object_id].p[0].val.v.data = new_object_id;

   c = GetClassByID(class_id);
   if (c == NULL) /* can't ever be, because AllocateObject checks */
   {
      eprintf("CreateObject can't find class id %i\n",class_id);
      return INVALID_OBJECT;
   }

   SetObjectProperties(new_object_id,c);

   /* might not be top level message, since can be called from blakod.  If it
      really IS a top level message, then it better not post anything, since
      post messages won't be handled */
   if (IsInterpreting())
      SendBlakodMessage(new_object_id,CONSTRUCTOR_MSG,num_parms,parms);
   else
      SendTopLevelBlakodMessage(new_object_id,CONSTRUCTOR_MSG,num_parms,parms);

   return new_object_id;
}

Bool LoadObject(int object_id,char *class_name)
{
   class_node *c;

   c = GetClassByName(class_name);
   if (c == NULL) 
   {
      eprintf("LoadObject can't find class name %s\n",class_name);
      return False;
   }

   if (AllocateObject(c->class_id) != object_id)
   {
      eprintf("LoadObject didn't make object id %i\n",object_id);
      return False;
   }

   /* set self = prop 0 */
   objects[object_id].p[0].id = 0;
   objects[object_id].p[0].val.v.tag = TAG_OBJECT; 
   objects[object_id].p[0].val.v.data = object_id;

   // Set any built-in object ID here, in case we're loading an old
   // save game with a blakod object that needs to be saved as a built-in.
   if (c->class_id <= MAX_BUILTIN_CLASS)
      SetBuiltInObjectIDByClass(c->class_id, object_id);

   /* if no kod changed, then setting the properties shouldn't be
    * necessary.  however, who knows.
    */
   SetObjectProperties(object_id,c);

   return True;
}

Bool SetObjectPropertyByName(int object_id,char *prop_name,val_type val)
{
   object_node *o;
   class_node *c;
   int property_id;
   
   o = GetObjectByID(object_id);
   if (o == NULL)
   {
      eprintf("SetObjectPropertyByName can't find object %i\n",object_id);
      return False;
   }

   c = GetClassByID(o->class_id);
   if (c == NULL) 
   {
      eprintf("SetObjectPropertyByName can't find class %i\n",
	      o->class_id);
      return False;
   }

   property_id = GetPropertyIDByName(c,prop_name);
   if (property_id == INVALID_PROPERTY)
   {
      eprintf("SetObjectPropertyByName can't find property %s in class %s (%i)\n",
	      prop_name, c->class_name, c->class_id);
      return False;
   }

   if (o->num_props <= property_id)
   {
      eprintf("SetObjectPropertyByName property index/id %i not in object %i class %s (%i)\n",
	      property_id,object_id,c->class_name,c->class_id);
      return False;
   }
   
   if (o->p[property_id].id != property_id)
   {
      eprintf("SetObjectPropertyByName property index/id mismatch %i %i\n",
	      property_id,o->p[property_id].id);
      return False;
   }

   o->p[property_id].val = val;
   return True;
}

void SetObjectProperties(int object_id,class_node *c)
{
   int i;

   if (c->super_ptr != NULL)
      SetObjectProperties(object_id,c->super_ptr);

   for (i=0;i<c->num_prop_defaults;i++)
   {
      if (c->prop_default[i].id >= objects[object_id].num_props)
      {
	 eprintf("SetObjectProperties can't set invalid property %i of "
		 "OBJECT %i CLASS %s (%i)\n",c->prop_default[i].id,object_id,c->class_name,c->class_id);
      }
      else
      {
	 objects[object_id].p[c->prop_default[i].id].id = c->prop_default[i].id;
	 objects[object_id].p[c->prop_default[i].id].val.int_val =
	    c->prop_default[i].val.int_val;
      }
   }
}

void DeleteBlakodObject(int object_id)
{
   class_node *c;
   object_node *o;

   o = GetObjectByID(object_id);
   if (o == NULL)
   {
      eprintf("DeleteBlakodObject can't get an object by ID %i!\n",object_id);
      return;
   }

   c = GetClassByID(o->class_id);
   if (c == NULL)
   {
      eprintf("DeleteBlakodObject can't find class %i\n",o->class_id);
      return;
   }

   /* now remove object */

   FreeMemory(MALLOC_ID_OBJECT_PROPERTIES,o->p,sizeof(prop_type)*(1+c->num_properties));
   o->deleted = True;
}   

void ForEachObject(void (*callback_func)(object_node *o))
{
   int i;

   for (i=0;i<num_objects;i++)
      if (!objects[i].deleted)
	 callback_func(&objects[i]);
}

/* these functions are for garbage collecting */

void MoveObject(int dest_id,int source_id)
{
   object_node *source,*dest;

   source = GetObjectByID(source_id);
   if (source == NULL)
   {
      eprintf("MoveObject can't find source %i, total death end game\n",
	      source_id);
      FlushDefaultChannels();
      return;
   }

   dest = GetObjectByIDEvenDeleted(dest_id);
   if (dest == NULL)
   {
      eprintf("MoveListNode can't find dest %i, total death end game\n",
	      dest_id);
      FlushDefaultChannels();
      return;
   }

   /* don't change the dest id here--it is set to array index, correctly */
   dest->class_id = source->class_id;
   dest->deleted = source->deleted;
   dest->garbage_ref = source->garbage_ref;
   dest->num_props = source->num_props;
   dest->p = source->p;

   // If this is a built-in object, set the new object ID.
   if (source->class_id <= MAX_BUILTIN_CLASS)
      SetBuiltInObjectIDByClass(source->class_id, dest_id);
}

void SetNumObjects(int new_num_objects)
{
   num_objects = new_num_objects;
}

/*
void debugobjects(int session_id)
{
   int i;

   for (i=0;i<num_objects;i++)
   {
      dprintf("%5i",i);
      if (objects[i].deleted)
	 dprintf("%5i\n",-1);
      else
	 dprintf("%5i\n",objects[i].class_id);
   }
   dprintf("\n");
}
*/
