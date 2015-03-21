// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * class.h
 *
 */

#ifndef _CLASS_H
#define _CLASS_H

typedef struct
{
   int id;
   val_type val;
} prop_default_type;

typedef struct
{
   int id;
   val_type val;
} var_default_type;

typedef struct class_struct
{
   int class_id;
   int super_id;

   message_node *messages;
   char *fname;
   char *class_name;

   int num_properties;
   int num_prop_defaults;
   int num_messages;
   prop_default_type *prop_default;

   int num_vars;
   int num_var_defaults;
   var_default_type *var_default;
   var_default_type *vars;

	property_name_node *property_names_this;
	sihash_type property_names;

   classvar_name_node *classvar_names;

   char *bof_base;
   bof_dstring *dstrs;
   bof_line_table *line_table;

   struct class_struct *super_ptr;

   struct class_struct *next; /* for open hash table linked list */
} class_node;

/* two functions from message.c that need class_node */
message_node *GetMessageByID(int class_id,int message_id,class_node **found_class);
message_node *GetMessageByName(int class_id,char *message_name,class_node **found_class);


/* the 629 is just a number to mult by to get reasonable hash results */
#define GetClassHashNum(a) (((a+10000)*629)%classes_table_size)



void InitClass(void);
void ResetClass(void);
void AddClass(int id,bof_class_header *class_data,char *fname,char *string_base,
	      bof_dstring *dstrs,bof_line_table *line_table,bof_class_props *props);
void SetClassName(int id,char *name);
void SetClassesSuperPtr(void);
void SetClassVariables(void);
void AddClassPropertyName(class_node *c,char *property_name,int property_id);
void SetClassPropertyNames();
class_node * GetClassByName(const char *class_name);
class_node * GetClassByID(int class_id);
const char * GetPropertyNameByID(class_node *c,int property_id);
int GetPropertyIDByName(class_node *c,const char *property_name);
char *GetClassVarNameByID(class_node *c,int classvar_id);
int GetClassVarIDByName(class_node *c,const char *classvar_name);

void ForEachClass(void (*callback_func)(class_node *c));
const char * GetClassDebugStr(class_node *c,int dstr_id);
int GetSourceLine(class_node *c,char *bkod_ptr);


#endif
