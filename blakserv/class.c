// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* class.c
*

  This module keeps an open hash table of classes, based upon their id
  (stored in the .bof).  The size of the table is configurable, and a
  warning is given if there are ever more than two classes with one
  hash value, since this could slow down operation of the server.
  
	Each class has a bunch of data.  There is the class id and its
	superclass id, the number of properties this class has, the number of
	property default values it has and the actual values, its array of
	messages and number of messages, possibly its name from the kodbase,
	the filename the class is defined in (the .kod file), and possibly a
	list of names of its properties.
	
*/

#include "blakserv.h"

static class_node **classes;
static int classes_table_size;

static sihash_type class_name_map;


/* local function prototypes */
void SetOneClassVariables(class_node *c,class_node *setvar_class);
void SetOneClassPropertyNames(class_node *c);

void InitClass(void)
{
	int i;

	classes_table_size = ConfigInt(MEMORY_SIZE_CLASS_HASH);
	classes = (class_node **)AllocateMemory(MALLOC_ID_CLASS,
					   classes_table_size*sizeof(class_node *));
	for (i=0;i<classes_table_size;i++)
		classes[i] = NULL;

	class_name_map = CreateSIHash(ConfigInt(MEMORY_SIZE_CLASS_NAME_HASH));
}

void ResetClass(void)
{
	class_node *c,*temp;
	classvar_name_node *cv,*temp_cv;
	property_name_node *prop,*temp_prop;
	int i;
	
	for (i=0;i<classes_table_size;i++)
	{
		c = classes[i];
		while (c != NULL)
		{
			temp = c->next;
			if (c->num_prop_defaults != 0)
				FreeMemory(MALLOC_ID_CLASS,c->prop_default,
							  sizeof(prop_default_type)*c->num_prop_defaults);
			if (c->num_var_defaults != 0)
				FreeMemory(MALLOC_ID_CLASS,c->var_default,
							  sizeof(var_default_type)*c->num_var_defaults);
			if (c->num_vars != 0)
				FreeMemory(MALLOC_ID_CLASS,c->vars,sizeof(var_default_type)*c->num_vars);
			
			if (c->class_name != NULL)
				FreeMemory(MALLOC_ID_KODBASE,c->class_name,strlen(c->class_name)+1);

			prop = c->property_names_this;
			while (prop != NULL)
			{
				temp_prop = prop->next;
				FreeMemory(MALLOC_ID_KODBASE,prop->name,strlen(prop->name)+1);
				FreeMemory(MALLOC_ID_KODBASE,prop,sizeof(property_name_node));
				prop = temp_prop;
			}

			FreeSIHash(c->property_names);
			
			cv = c->classvar_names;
			while (cv != NULL)
			{
				temp_cv = cv->next;
				FreeMemory(MALLOC_ID_KODBASE,cv->name,strlen(cv->name)+1);
				FreeMemory(MALLOC_ID_KODBASE,cv,sizeof(classvar_name_node));
				cv = temp_cv;
			}
			
			FreeMemory(MALLOC_ID_CLASS,c,sizeof(class_node));
			c = temp;
		}
		classes[i] = NULL;
	}

	FreeSIHash(class_name_map);
	class_name_map = CreateSIHash(ConfigInt(MEMORY_SIZE_CLASS_NAME_HASH));
}

/* AddClass
*
* This adds a new class to the class hash table.
*/
void AddClass(int id,bof_class_header *class_data,char *fname,char *bof_base,
			  bof_dstring *dstrs,bof_line_table *line_table,bof_class_props *props)
{
	int i,hash_num;
	class_node *new_node;
   bof_list_elem *classvar_values, *prop_values;

	new_node = (class_node *)AllocateMemory(MALLOC_ID_CLASS,sizeof(class_node));
	memset(new_node, 0, sizeof(class_node));
	
	new_node->class_id = id;
	new_node->super_id = class_data->superclass;
	new_node->num_properties = props->num_properties;
	new_node->num_prop_defaults = props->num_default_prop_vals;
	new_node->messages = NULL;
	new_node->num_messages = 0;
	if (new_node->num_prop_defaults != 0)
	{
		new_node->prop_default = (prop_default_type *)
			AllocateMemory(MALLOC_ID_CLASS,sizeof(prop_default_type)*new_node->num_prop_defaults);
	}
	else
		new_node->prop_default = NULL;

   prop_values = &props->prop_values;
	for (i=0;i<new_node->num_prop_defaults;i++)
	{
		new_node->prop_default[i].id = prop_values[i].id;
		new_node->prop_default[i].val.int_val = prop_values[i].offset; 
	}
	new_node->fname = fname;
	new_node->class_name = NULL;
	new_node->property_names = NULL;
	new_node->property_names_this = NULL;
	new_node->classvar_names = NULL;
	new_node->super_ptr = NULL;
	new_node->bof_base = bof_base; /* is really pointer to file in memory,
	which is the base from which the dstrs are */
	new_node->dstrs = dstrs;
	new_node->line_table = line_table; /* table of source line number info */
	
	new_node->num_var_defaults = class_data->num_default_classvar_vals;
	new_node->num_vars = class_data->num_classvars;
	
	if (new_node->num_var_defaults != 0)
	{
		/* dprintf("defaults %i\n",new_node->num_var_defaults); */
		new_node->var_default = (var_default_type *)
			AllocateMemory(MALLOC_ID_CLASS,sizeof(var_default_type)*new_node->num_var_defaults);
		
	}
	else
		new_node->var_default = NULL;
	
   classvar_values = &class_data->classvar_values;
	for (i=0;i<new_node->num_var_defaults;i++)
	{
		new_node->var_default[i].id = classvar_values[i].id;
		new_node->var_default[i].val.int_val = classvar_values[i].offset; 
	}
	
	new_node->vars = NULL;
	
	/* add to class table */
	hash_num = GetClassHashNum(new_node->class_id);
	new_node->next = classes[hash_num];
	classes[hash_num] = new_node;
}

void SetClassName(int id,char *name)
{
	class_node *current_class;

   current_class = GetClassByID(id);
   if (current_class == NULL)
   {
      eprintf("SetClassName can't find class id %i\n",id);
      return;
   }

   current_class->class_name = (char *)AllocateMemory(MALLOC_ID_KODBASE,strlen(name)+1);
   strcpy(current_class->class_name,name);

	SIHashInsert(class_name_map,name,id);
}

/* SetClassesSuperPtr
*
* After calling addclass for all classes, call this to setup our class
* hierarchy parent pointers 
*
*/
void SetClassesSuperPtr(void)
{
	class_node *c,*super_ptr;
	int i;
	int length; 

	for (i=0;i<classes_table_size;i++)
	{
		c = classes[i];
		length = 0; 
		while (c != NULL)
		{
			length++; 
			if (c->super_id != NO_SUPERCLASS)
			{
				super_ptr = GetClassByID(c->super_id);
				if (super_ptr == NULL)
					eprintf("SetClassesSuperPtr found class %i with invalid parent id %i! "
					"[possibly obsolete bof]\n",c->class_id, c->super_id);
				else
					c->super_ptr = super_ptr;
			}
			c = c->next;
		}

		if (length > 1)
			eprintf("Fatal: Class hash bin %3i has length %i; Increase SizeClassHash in [Memory] in blakserv.cfg\n",i,length);

	}
}

void SetClassVariables(void)
{
	class_node *c;
	int i;
	
	for (i=0;i<classes_table_size;i++)
	{
		c = classes[i];
		while (c != NULL)
		{
			if (c->num_vars > 0)
			{
				/* dprintf("allocating %i, %i\n",sizeof(var_default_type),c->num_vars); */
				c->vars = (var_default_type *)
					AllocateMemory(MALLOC_ID_CLASS,sizeof(var_default_type)*c->num_vars);
				SetOneClassVariables(c,c);
			}
			
			c = c->next;
		}
	}
}

void SetOneClassVariables(class_node *c,class_node *setvar_class)
{
	int i;
	
	/* set the class variables of class c, from the top of its ancestors down
	to its own defaults */
	
	if (setvar_class->super_ptr != NULL)
		SetOneClassVariables(c,setvar_class->super_ptr);
	
	for (i=0;i<setvar_class->num_var_defaults;i++)
	{
		if (setvar_class->var_default[i].id >= c->num_vars)
		{
			eprintf("SetOneClassVariables can't set invalid class var %i (num %i) of "
				"class %i\n",setvar_class->var_default[i].id,c->num_vars,c->class_id);
		}
		else
		{
			c->vars[setvar_class->var_default[i].id].id = setvar_class->var_default[i].id;
			c->vars[setvar_class->var_default[i].id].val.int_val =
				setvar_class->var_default[i].val.int_val;
		}
	}
	
}

void AddClassPropertyName(class_node *c,char *property_name,int property_id)
{
   property_name_node *new_prop;

   new_prop = (property_name_node *)AllocateMemory(MALLOC_ID_KODBASE,sizeof(property_name_node));
   new_prop->id = property_id;
   new_prop->name = (char *)AllocateMemory(MALLOC_ID_KODBASE,strlen(property_name)+1);
   strcpy(new_prop->name,property_name);

 /*
    dprintf("class id %i has prop %i named %s\n",c->class_id,
            new_prop->id,new_prop->name);
 */
   new_prop->next = c->property_names_this;
   c->property_names_this = new_prop;
}

void SetClassPropertyNames()
{
	class_node *c;
	int i;
	
	for (i=0;i<classes_table_size;i++)
	{
		c = classes[i];
		while (c != NULL)
		{
			c->property_names = CreateSIHash(ConfigInt(MEMORY_SIZE_PROPERTIES_NAME_HASH));
			SetOneClassPropertyNames(c);
			c = c->next;
		}
	}
}

void SetOneClassPropertyNames(class_node *c)
{
	property_name_node *prop;
	class_node *ancestor;
	ancestor = c;
	while (ancestor != NULL)
	{
		prop = ancestor->property_names_this;
		while (prop != NULL)
		{
			// if we have this property name already, don't need to add again
			int value;
			if (SIHashFind(c->property_names,prop->name,&value) == False)
				SIHashInsert(c->property_names,prop->name,prop->id);
			prop = prop->next;
		}
		ancestor = ancestor->super_ptr;
	}
}

class_node * GetClassByID(int class_id)
{
	class_node *c;
	
	c = classes[GetClassHashNum(class_id)];
	while (c != NULL)
	{
		if (c->class_id == class_id)
			return c;
		c = c->next;
	}
	
	return NULL;
}

class_node * GetClassByName(const char *class_name)
{
	int class_id;
	Bool found;

	found = SIHashFind(class_name_map,class_name,&class_id);
	if (found)
		return GetClassByID(class_id);

	return NULL;
}

const char * GetPropertyNameByID(class_node *c,int property_id)
{
	return SIHashFindByValue(c->property_names,property_id);
}

int GetPropertyIDByName(class_node *c,const char *property_name)
{
	int id;

	if (c == NULL)
		return INVALID_PROPERTY;

	if (SIHashFind(c->property_names,property_name,&id) == True)
		return id;

	// check ancestor classes
	if (c->super_ptr != NULL)
		return GetPropertyIDByName(c->super_ptr,property_name);

	return INVALID_PROPERTY;
}

int GetClassVarIDByName(class_node *c, const char *name)
{
	classvar_name_node *cv;

	if (c == NULL || name == NULL)
		return INVALID_CLASSVAR;

	cv = c->classvar_names;
	while (cv != NULL)
	{
		if (stricmp(cv->name, name) == 0)
			return cv->id;
		cv = cv->next;
	}
	
	if (c->super_ptr == NULL)
      return INVALID_CLASSVAR;
   
   return GetClassVarIDByName(c->super_ptr, name);
}


char * GetClassVarNameByID(class_node *c,int classvar_id)
{
	classvar_name_node *cv;
	
	if (c == NULL)
		return NULL;
	
	cv = c->classvar_names;
	while (cv != NULL)
	{
		if (cv->id == classvar_id)
			return cv->name;
		cv = cv->next;
	}
	
	if (c->super_ptr != NULL)
		return GetClassVarNameByID(c->super_ptr,classvar_id);
	else
		return NULL;
}

void ForEachClass(void (*callback_func)(class_node *c))
{
	class_node *c;
	int i;
	
	for (i=0;i<classes_table_size;i++)
	{
		c = classes[i];
		while (c != NULL)
		{
			callback_func(c);
			c = c->next;
		}
	}
}

const char * GetClassDebugStr(class_node *c,int dstr_id)
{
	if (dstr_id > c->dstrs->num_strings)
	{
		eprintf("GetClassDebugStr got invalid dstr id, %i %i\n",c->class_id,dstr_id);
		return "Invalid DStr";
	}
	return c->bof_base + ((int *) &c->dstrs->string_offsets)[dstr_id];
}

int GetSourceLine(class_node *c,char *bkod_ptr)
{
	bof_line_entry *bline;
	int prev_line;
	int i;
	
	if (c->line_table == NULL)
		return 0;
	
	/* search for first line starting at or after current spot, then print previous
	line, because chances are we advanced past beginning of instruction */
	
	prev_line = 0;
	
	bline = &c->line_table->entries;
	for (i=0;i<c->line_table->num_line_entries;i++)
	{
		if (bline->file_offset + c->bof_base > bkod_ptr)
			return prev_line;
		prev_line = bline->line_number;
		bline++;
		
	}

	return 0;
}


