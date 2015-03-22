// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * kodbase.c
 *
 
 This module takes care of loading the kodbase file.  It adds symbolic
 names to classes, messages, properties, parameters, and messages.
 Message and parameter names are held in a special linked list in
 nameid.c.  Class and property names are held in the class, and
 resource names are held in the resource.
 
 */

#include "blakserv.h"

#define MAX_LINE 128

class_node *current_class;

/* local function prototypes */

void LoadKodbaseClass(char *class_name,int class_id);
void LoadKodbaseProperty(char *prop_name,int property_id);
void LoadKodbaseClassVariable(char *classvar_name,int classvar_id);
void LoadKodbaseMessage(char *message_name,int message_id);
void LoadKodbaseParameter(char *parm_name,int parm_id);
void LoadKodbaseResource(char *resource_name,int resource_id);

void LoadKodbase(void)
{
   FILE *kodbase;
   int lineno;
   char line[MAX_LINE+1];
   char *type_char,*t1,*t2,*t3,*t4;
   char load_name[MAX_PATH+FILENAME_MAX];

   sprintf(load_name,"%s%s",ConfigStr(PATH_KODBASE),KODBASE_FILE);

   if ((kodbase = fopen(load_name,"rt")) == NULL)
   {
      eprintf("LoadKodBase can't open %s!!!\n",load_name);
      return;
   }

   lineno = 0;
   while (fgets(line,MAX_LINE,kodbase))
   {
      lineno++;

      type_char = strtok(line," \t");
      if (type_char == NULL || strlen(type_char) != 1)
      {
	 eprintf("LoadKodBase line %i Bad type character!!!\n",lineno);
	 fclose(kodbase);
	 return;
      }

      t1 = strtok(NULL," \t\n");
      t2 = strtok(NULL," \t\n");
      t3 = strtok(NULL," \t\n");
      t4 = strtok(NULL," \t\n");

      switch (type_char[0])
      {
      case 'T' : /* ignore title line */
	 break;

      case 'C' :
	 if (t1 == NULL || t2 == NULL || atoi(t2) == 0 || t3 == NULL 
	     || (atoi(t3) != 0 && t4 == NULL))
	 {
	    eprintf("LoadKodBase line %i Bad class entry!!!\n",lineno);
	    fclose(kodbase);
	    return;
	 }
	 
	 LoadKodbaseClass(t1,atoi(t2));
	 
	 break;
	 
      case 'M' : 
	 if (t1 == NULL || t2 == NULL || atoi(t2) == 0)
	 {
	    eprintf("LoadKodBase line %i Bad message entry!!!\n",lineno);
	    fclose(kodbase);
	    return;
	 }
         LoadKodbaseMessage(t1,atoi(t2));
	 break;
	 
      case 'P' : 
	 if (t1 == NULL || t2 == NULL || atoi(t2) == 0)
	 {
	    eprintf("LoadKodBase line %i Bad parameter entry!!!\n",lineno);
	    fclose(kodbase);
	    return;
	 }
         LoadKodbaseParameter(t1,atoi(t2));
	 break;
	 
      case 'R' :
	 if (t1 == NULL || t2 == NULL || atoi(t2) == 0)
	 {
	    eprintf("LoadKodBase line %i Bad resource entry!!!\n",lineno);
	    fclose(kodbase);
	    return;
	 }
         LoadKodbaseResource(t1,atoi(t2));
	 break;
	 
      case 'Y' :
	 if (t1 == NULL || t2 == NULL)
	 {
	    eprintf("LoadKodBase line %i Bad property entry!!!\n",lineno);
	    fclose(kodbase);
	    return;
	 }
         LoadKodbaseProperty(t1,atoi(t2));
	 break;
	 
      case 'V' :
	 if (t1 == NULL || t2 == NULL)
	 {
	    eprintf("LoadKodBase line %i Bad classvar entry!!!\n",lineno);
	    fclose(kodbase);
	    return;
	 }
         LoadKodbaseClassVariable(t1,atoi(t2));
	 break;
	 
      default:
	 eprintf("LoadKodBase line %i Unknown type character %c\n",lineno,type_char[0]);
	 break;
      }
   }
   
   fclose(kodbase);
	
	SetClassPropertyNames();
}

void LoadKodbaseClass(char *class_name,int class_id)
{
   current_class = GetClassByID(class_id);
   if (current_class == NULL)
   {
      eprintf("LoadKodbaseClass can't find class id %i\n",class_id);
      return;
   }

	SetClassName(class_id,class_name);
}

void LoadKodbaseProperty(char *prop_name,int property_id)
{
   if (current_class == NULL)
   {
      eprintf("LoadKodbaseProperty has no current class (%s:%d)\n",
              prop_name, property_id);
      return;
   }

/*
   dprintf("class id %i has prop %i named %s\n",current_class->class_id,
           new_prop->id,new_prop->name);
*/
	AddClassPropertyName(current_class,prop_name,property_id);
   
   return;
}

void LoadKodbaseClassVariable(char *classvar_name,int classvar_id)
{
   classvar_name_node *new_classvar;

   if (current_class == NULL)
   {
      eprintf("LoadKodbaseClassVariable has no current class (%s:%d)\n",
              classvar_name, classvar_id);
      return;
   }

   new_classvar = (classvar_name_node *)AllocateMemory(MALLOC_ID_KODBASE,
						       sizeof(classvar_name_node));
   new_classvar->id = classvar_id;
   new_classvar->name = (char *)AllocateMemory(MALLOC_ID_KODBASE,strlen(classvar_name)+1);
   strcpy(new_classvar->name,classvar_name);

/*
   dprintf("class id %i has classvar %i named %s\n",current_class->class_id,
           new_classvar->id,new_classvar->name);
*/
   
   new_classvar->next = current_class->classvar_names;
   current_class->classvar_names = new_classvar;
   
   return;
}

void LoadKodbaseMessage(char *message_name,int message_id)
{
   CreateNameID(message_name,message_id);
}

void LoadKodbaseParameter(char *parm_name,int parm_id)
{
   CreateNameID(parm_name,parm_id);
}

void LoadKodbaseResource(char *resource_name,int resource_id)
{
	SetResourceName(resource_id,resource_name);
}
