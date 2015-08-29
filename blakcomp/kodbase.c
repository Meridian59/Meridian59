// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* kodbase.c -- routines for loading and saving the database */

#include "blakcomp.h"

extern SymbolTable st;
extern int numbuiltins;

/* local defines */

static const char *basefile = "kodbase.txt";

#define MAX_LINE 128

int build_superclasses(list_type classes);
int init_tables(int id_count, int res_count);
int load_add_class(char *class_name, int class_id, int superclass_id, char *superclass_name);
int load_add_message(char *message_name, int message_id);
int load_add_resource(char *resource_name, int resource_id);
int load_add_parameter(char *parm_name, int);
int load_add_property(char *prop_name, int);
int load_add_classvar(char *name, int classvar_id);
int load_add_external(char *name, int idnum, int type);

/* current_class holds pointer to class info of class we're currently reading in */
class_type current_class = NULL;
/* current_message holds pointer to info of message handler we're currently reading in */
message_header_type current_message = NULL;

static int kodbase_line;
void database_error(const char *fmt, ...)
{
   va_list marker;

   printf("%s(%d): ", basefile, kodbase_line);
   va_start(marker, fmt);
   simple_error(fmt, marker);
   va_end(marker);
/*
   vprintf(fmt, marker);
   printf("\n");
*/
}

void set_kodbase_filename(char *filename)
{
   basefile = filename;
}

/*
 * load_kodbase - reads the kodbase.txt file into memory, returns success/failure
 */
int load_kodbase(void)
{
   FILE *kodbase;
   char line[MAX_LINE+1];
   char *type_char, *t1, *t2, *t3, *t4;

   /* If file is missing, give warning but continue, since this is normal
    * if no files have been compiled yet.
    */
   if ((kodbase = fopen(basefile, "rt")) == NULL)
   {
      simple_warning("Unable to open database file %s", basefile);
      return True;
   }

   kodbase_line = 0;
   while (fgets(line, MAX_LINE+1, kodbase))
   {
      kodbase_line++;

      type_char = strtok(line," \t");
      if (type_char == NULL || strlen(type_char) != 1)
      {
	 database_error("Bad type character");
	 fclose(kodbase);
	 return False;
      }

      t1 = strtok(NULL," \t\n");
      t2 = strtok(NULL," \t\n");
      t3 = strtok(NULL," \t\n");
      t4 = strtok(NULL," \t\n");

      switch (type_char[0])
      {
      case 'T' : 
	 if (t2 == NULL || !init_tables(atoi(t1), atoi(t2)))
	 {
	    database_error("Bad type character");
	    fclose(kodbase);
	    return False;
	 }
	 break;

      case 'C' :
	 if (t3 == NULL)
	 {
	    database_error("Bad class entry");
	    fclose(kodbase);
	    return False;
	 }
	 
	 /* Error if missing superclass name */
	 if ((atoi(t3) != 0 && t4 == NULL) || !load_add_class(t1,atoi(t2),atoi(t3), t4))
	 {
	    database_error("Bad class entry");
	    fclose(kodbase);
	    return False;
	 }
	 break;

      case 'M' :
	 if (t2 == NULL || !load_add_message(t1,atoi(t2)))
	 {
	    database_error("Bad message entry");
	    fclose(kodbase);
	    return False;
	 }
	 break;

      case 'R':
	 if (t2 == NULL || !load_add_resource(t1,atoi(t2)))
	 {
	    database_error("Bad resource entry");
	    fclose(kodbase);
	    return False;
	 }
	 break;

      case 'P' :
	 if (t2 == NULL || !load_add_parameter(t1, atoi(t2)))
	 {
	    database_error("Bad parameter entry");
	    fclose(kodbase);
	    return False;
	 }
	 break;

      case 'Y' :
	 if (t2 == NULL || !load_add_property(t1, atoi(t2)))
	 {
	    database_error("Bad property entry");
	    fclose(kodbase);
	    return False;
	 }
	 break;

      case 'V' :
	 if (t2 == NULL || !load_add_classvar(t1, atoi(t2)))
	 {
	    database_error("Bad classvar entry");
	    fclose(kodbase);
	    return False;
	 }
	 break;

      case 'c':
	 if (t2 == NULL || !load_add_external(t1, atoi(t2), I_CLASS))
	 {
	    database_error("Bad external class entry");
	    fclose(kodbase);
	    return False;
	 }
	 break;

      case 'm':
	 if (t2 == NULL || !load_add_external(t1, atoi(t2), I_MESSAGE))
	 {
	    database_error("Bad external message entry");
	    fclose(kodbase);
	    return False;
	 }
	 break;

      case 'p':
	 if (t2 == NULL || !load_add_external(t1, atoi(t2), I_PARAMETER))
	 {
	    database_error("Bad external parameter entry");
	    fclose(kodbase);
	    return False;
	 }
	 break;

      default:
	 database_error("Unknown type character %c", type_char[0]);
      }
   }
   /* Now that everone's read in, make pointers to superclasses */
   if (!build_superclasses(st.classes))
   {
      fclose(kodbase);
      return False;
   }
   
   fclose(kodbase);
   return True;
}
/**********************************************************************/
int init_tables(int id_count, int res_count)
{
   st.maxid = id_count;
   st.maxresources = res_count;
   st.classes = NULL;
   return True;
}
/**********************************************************************/
/* Return False on error, True on success */
int load_add_class(char *class_name, int class_id, int superclass_id, char *superclass_name)
{
   /* Build up a class data structure for the new class. */
   id_type id = (id_type)SafeMalloc(sizeof(id_struct));
   id_type temp_id = (id_type)SafeMalloc(sizeof(id_struct));
   class_type c = (class_type)SafeMalloc(sizeof(class_struct));

   // Adding new built-in object types will render existing kodbase.txt files
   // incompatible. This isn't a problem as a pre-existing kodbase.txt is only
   // required for reloading a live server, so a new one can be made. Check
   // for built-in class name/ID mismatches here and instruct the user to
   // delete kodbase.txt if this check fails.

   extern id_struct BuiltinIds[];
   if ((strcmp(BuiltinIds[SETTINGS_CLASS].name, class_name) == 0
         && class_id != SETTINGS_CLASS)
      || (strcmp(BuiltinIds[REALTIME_CLASS].name, class_name) == 0
         && class_id != REALTIME_CLASS)
      || (strcmp(BuiltinIds[EVENTENGINE_CLASS].name, class_name) == 0
         && class_id != EVENTENGINE_CLASS))
   {
      database_error("Incompatible kodbase.txt. Delete the file and recompile.");
      return False;
   }

   id->name = strdup(class_name);
   id->idnum = class_id;
   id->type = I_CLASS;
   id->source = DBASE;

   c->class_id = id;
   c->properties = c->messages = c->resources = c->classvars = NULL;
   c->is_new = False;  /* Don't generate code for this class */
   /* Store superclass id # in pointer for now.  Id # will be converted to pointer
    * when build_superclasses below is called. */
   c->superclass = (class_type) superclass_id;

   /* Add to list of classes that have been read in */
   st.classes = list_add_item(st.classes, (void *) c);

   current_class = c;
   st.curclass = class_id;
   current_message = NULL;

   /* Call table_insert instead of add_identifier so that our id # from
    * the database file is preserved. 
    */
   if (table_insert(st.globalvars, (void *) id, id_hash, id_compare) == 0)
      return True;
   else return False;
}
/**********************************************************************/
int load_add_message(char *message_name, int message_id)
{
   id_type id = (id_type) SafeMalloc(sizeof(id_struct));
   message_handler_type m = (message_handler_type) SafeMalloc(sizeof(message_handler_struct));
   message_header_type h = (message_header_type) SafeMalloc(sizeof(message_header_struct));

   id->name = strdup(message_name);
   id->idnum = message_id;
   id->type = I_MESSAGE;
   id->ownernum = st.curclass;
   id->source = DBASE;

   h->message_id = id;
   h->params = NULL;

   m->header = h;
   m->locals = NULL;
   m->body = NULL;

   /* Add message to message list of current class */
   if (current_class == NULL)
      simple_error("Message appears outside of class in database file");
   else
      current_class->messages = list_add_item(current_class->messages, (void *) m);

   current_message = h;
   st.curmessage = message_id;
   /* OK if message already in table; just ignore return value */
   table_insert(st.globalvars, (void *) id, id_hash, id_compare);
   return True;
}
/**********************************************************************/
int load_add_resource(char *resource_name, int resource_id)
{
   id_type id = (id_type) SafeMalloc(sizeof(id_struct));
   resource_type r = (resource_type) SafeMalloc(sizeof(resource_struct));
   
   id->name = strdup(resource_name);
   id->idnum = resource_id;
   id->type = I_RESOURCE;
   id->ownernum = st.curclass;
   id->source = DBASE;

   r->lhs = id;
   // Have to load in resources from *rsc files
   for (int i = 0; i < sizeof(r->resource) / sizeof(r->resource[i]); i++)
   {
      r->resource[i] = NULL;
   }

   /* Add resource to resource list of current class */
   if (current_class == NULL)
      database_error("Resource appears outside of class in database file");
   current_class->resources = list_add_item(current_class->resources, (void *) r);

   /* OK if parameter already in table; just ignore return value */
   table_insert(st.globalvars, (void *) id, id_hash, id_compare);
   return True;
}
/**********************************************************************/
int load_add_parameter(char *parm_name, int parm_id)
{
   id_type id = (id_type) SafeMalloc(sizeof(id_struct));
   param_type p = (param_type) SafeMalloc(sizeof(param_struct));

   id->name = strdup(parm_name);
   id->idnum = parm_id;
   id->type = I_PARAMETER;
   id->ownernum = st.curmessage;
   id->source = DBASE;

   p->lhs = id;
   p->rhs = NULL;

   if (current_message == NULL)
      simple_error("Message appears outside of class in database file");
   else
      current_message->params = list_add_item(current_message->params, (void *) p);

   /* OK if parameter already in table; just ignore return value */
   table_insert(st.globalvars, (void *) id, id_hash, id_compare);
   return True;
}
/**********************************************************************/
int load_add_property(char *prop_name, int property_id)
{
   id_type id = (id_type) SafeMalloc(sizeof(id_struct));
   property_type p = (property_type) SafeMalloc(sizeof(property_struct));

   id->name = strdup(prop_name);
   id->idnum = property_id;
   id->type = I_PROPERTY;
   id->ownernum = st.curclass;
   id->source = DBASE;

   /* Make fake property to insert into property list */
   p->id = id;
   p->rhs = NULL;

   /* Add property to property list of current class */
   if (current_class == NULL)
      simple_error("Property appears outside of class in database file");
   current_class->properties = list_add_item(current_class->properties, (void *) p);

   return True;
}
/**********************************************************************/
int load_add_classvar(char *name, int classvar_id)
{
   id_type id = (id_type) SafeMalloc(sizeof(id_struct));
   classvar_type c = (classvar_type) SafeMalloc(sizeof(classvar_struct));

   id->name = strdup(name);
   id->idnum = classvar_id;
   id->type = I_CLASSVAR;
   id->ownernum = st.curclass;
   id->source = DBASE;

   /* Make fake classvar to insert into list */
   c->id = id;
   c->rhs = NULL;

   /* Add to list of current class */
   if (current_class == NULL)
      simple_error("Classvar appears outside of class in database file");
   current_class->classvars = list_add_item(current_class->classvars, (void *) c);

   return True;
}
/**********************************************************************/
int load_add_external(char *name, int idnum, int type)
{
   id_type id = (id_type) SafeMalloc(sizeof(id_struct));

   /* Add unresolved externals to list of missing vars */
   id->name = strdup(name);
   id->type = I_MISSING;
   id->idnum = idnum;
   id->source = type;

   if (table_insert(st.missingvars, (void *) id, id_hash, id_compare) == 0)
      return True;
   return False;
}
/**********************************************************************/
/*
 * build_superclasses: After all classes have been read in from database,
 *   turn superclass id #s into pointers to the superclasses themselves.
 *   We store the superclass id #s in the superclass pointers as they
 *   are read in load_add_class above; here we find the actual pointers.
 */
int build_superclasses(list_type classes)
{
   list_type temp, l = classes;
   int superclass_idnum;

   while (l != NULL)
   {
      class_type c = (class_type) l->data;
      if ( (int) c->superclass != NO_SUPERCLASS)
      {
	 superclass_idnum = (int) c->superclass;
	 /* Search through classes looking for parent */
	 temp = classes;
	 while (temp != NULL)
	 {
	    class_type parent = (class_type) temp->data;
	    if (parent->class_id->idnum == superclass_idnum)
	    {
	       c->superclass = parent;
	       break;
	    }
	    temp = temp->next;
	 }
      }
      l = l->next;
   }
   return True;
}
/**********************************************************************/
/* 
 * save_externals:  Write out unresolved external references, given in 
 *    a list.  Returns # of unresolved externals.
 */
int save_externals(FILE *kodbase, list_type l)
{
   id_type id;
   int num = 0;
   char tag;
   
   for ( ; l != NULL; l = l->next)
   {
      id = (id_type) l->data;
      
      /* Don't write out identifiers that are built-in to the compiler */
      if (id->idnum < IDBASE)
	 continue;

      switch(id->source)
      {
      case I_CLASS:
	 tag = 'c';
	 break;

      case I_MESSAGE:
	 tag = 'm';
	 break;

      case I_PARAMETER:
	 tag = 'p';
	 break;
      }
      fprintf(kodbase, "%c %s %d\n", tag, id->name, id->idnum);
      num++;
   }
   return num;
}
/**********************************************************************/
/*
 * save_class_list: Write out database information for a list of classes.
 */
void save_class_list(FILE *kodbase, list_type c)
{
   list_type m, pr, p, r, cv;
   class_type each_class;
   classvar_type classvar;
   property_type property;
   message_handler_type message;
   param_type parameter;
   resource_type resource;
   
   for ( ; c != NULL; c = c->next)
   {
      each_class = (class_type) (c->data);
      /* Print out class info */
      if (each_class->superclass == NULL)
         fprintf(kodbase, "C %s %d %d\n", each_class->class_id->name,
                 each_class->class_id->idnum,
                 NO_SUPERCLASS);
      else fprintf(kodbase, "C %s %d %d %s\n", each_class->class_id->name,
                   each_class->class_id->idnum,
                   each_class->superclass->class_id->idnum,
                   each_class->superclass->class_id->name);
      
      /* Loop over all resources */
      for (r = each_class->resources; r != NULL; r = r->next)
      {
         resource = (resource_type) r->data;
         fprintf(kodbase, "R\t%s %d\n", resource->lhs->name, resource->lhs->idnum);
      }
      
      /* Loop over all classvars */
      for (cv = each_class->classvars; cv != NULL; cv = cv->next)
      {
         classvar = (classvar_type) (cv->data);
         fprintf(kodbase, "V\t%s %d\n", classvar->id->name, classvar->id->idnum);
      }
      
      /* Loop over all properties */
      for (pr = each_class->properties; pr != NULL; pr = pr->next)
      {
         property = (property_type) (pr->data);
         fprintf(kodbase, "Y\t%s %d\n", property->id->name, property->id->idnum);
      }
      
      /* Loop over all message handlers */
      for (m = each_class->messages; m != NULL; m = m->next)
      {
         message = (message_handler_type) (m->data);
         fprintf(kodbase, "M\t%s %d\n", message->header->message_id->name, 
                 message->header->message_id->idnum);
         
         /* Loop over all parameters */
         for (p = message->header->params; p != NULL; p = p->next)
         {
            parameter = (param_type) (p->data);
            fprintf(kodbase, "P\t\t%s %d\n", parameter->lhs->name, parameter->lhs->idnum);
         }
      }
   }
}
/**********************************************************************/
/*
 * save_kodbase: Writes out the new database file based on class list
 *              passed in from parser.  Returns True iff successful.
 */
int save_kodbase()
{
   FILE *kodbase;
   int numexternals;
   list_type external_list;

   if ((kodbase = fopen(basefile, "wt")) == NULL)
   {
      simple_error("Unable to open database file %s", basefile);
      return False;
   }
  
   /* First, write out status line of # of identifiers & resources */
   fprintf(kodbase, "T %d %d\n", st.maxid, st.maxresources);

   /* Loop over all classes, and write them out */
   save_class_list(kodbase, st.classes);

   /* Write out unresolved externals */
   external_list = table_get_all(st.missingvars);
   numexternals = save_externals(kodbase, external_list);
   
   /*if (numexternals != 0)
      simple_warning("%d unresolved externals", numexternals);*/

   fclose(kodbase);
   return True;
}
