// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* loadgame.c
*

  This module loads the game data from a file.  This file is a binary file.
  It used to be a text file, with comment lines started with a pound sign (#).
  Data lines start with a tag name, either "SYSTEM", "OBJECT", "PROP", "LIST",
  "TIMER", or "USER" which indicates what type of data is being stored on that
  line.
  
*/

#include <assert.h>

#include "blakserv.h"

#define MAX_SAVE_LINE 200

typedef struct load_game_prop_struct
{
	int namelen;
	char *prop_name;
} load_game_prop_node;

typedef struct load_game_class_struct
{
	int class_old_id;
	char *class_name;
	
	int num_props;
	load_game_prop_node *props; /* array of property names */
	
	struct load_game_class_struct *next;
} load_game_class_node;

typedef struct loaded_file_struct
{
   char fname[MAX_PATH+FILENAME_MAX];
   FILE *file;
} loaded_file_node;
loaded_file_node loadfile;

int current_object_id;
int current_object_class_id;

// hash table of classes by name
int load_game_classes_table_size;
load_game_class_node **load_game_classes;

ishash_type load_game_resources;


#define LoadGameRead(buf,len) \
{ \
   if (fread(buf, 1, len, loadfile.file) != len) \
   { \
	   eprintf("File %s Line %i not enough bytes to read\n",__FILE__,__LINE__); \
      return False; \
   } \
} 

#define LoadGameReadInt(buf) LoadGameRead(buf,4)
#define LoadGameReadChar(buf) LoadGameRead(buf,1)

#define LoadGameReadString(buf,max_len) \
{ \
	unsigned short len; \
   if (fread(&len, 1, 2, loadfile.file) != 2) \
   { \
      eprintf("File %s Line %i not enough bytes to read\n",__FILE__,__LINE__); \
      return False; \
   } \
\
	if (len > max_len-1) \
   { \
      eprintf("File %s Line %i string too long (%i >= %i)\n",__FILE__,__LINE__,len,max_len); \
      return False; \
   } \
\
   if (fread(buf, 1, len, loadfile.file) != len) \
   { \
      eprintf("File %s Line %i not enough bytes to read\n",__FILE__,__LINE__); \
      return False; \
   } \
	buf[len] = 0; \
}


/* local function prototypes */

Bool LoadGameOpen(char *fname);
Bool LoadGameParse(char *filename);
void LoadGameClose(void);
Bool LoadGameVersion(void);
Bool LoadGameBuiltInObjects();

Bool LoadGameObject(void);
Bool LoadGameListNodes(void);
Bool LoadGameTables(void);
Bool LoadGameTimer(void);
Bool LoadGameUser(void);
Bool LoadGameClass(void);
void LoadAddPropertyName(load_game_class_node *lgc,int prop_old_id,char *prop_name);
Bool LoadGameResource(void);

load_game_class_node * CreateLoadGameClass(int class_old_id,char *class_name,int num_props);
void CreateLoadGameResource(int resource_old_id,char *resource_name);

load_game_class_node * GetLoadGameClassByID(int class_old_id);
char * GetLoadGamePropertyNameByID(load_game_class_node *lgc,int prop_old_id);
const char * GetLoadGameResourceByID(int resource_old_id);

void LoadGameTranslateVal(val_type *pval);

// Save game version number.
int savegame_version;

Bool LoadGame(char *filename)
{
	Bool ret_val;
	load_game_class_node *lgc,*tempc;
	int i,j;
	UINT64 start_time;
	UINT64 end_time;
	
	start_time = GetMilliCount();
	dprintf("LoadGame starting\n");

	load_game_classes_table_size = ConfigInt(MEMORY_SIZE_CLASS_HASH);
	load_game_classes = (load_game_class_node **)AllocateMemory(MALLOC_ID_LOAD_GAME,
																	  load_game_classes_table_size*sizeof(load_game_class_node *));
	for (i=0;i<load_game_classes_table_size;i++)
		load_game_classes[i] = NULL;

	load_game_resources = CreateISHash(ConfigInt(MEMORY_SIZE_RESOURCE_NAME_HASH));
	current_object_id = INVALID_OBJECT;
	current_object_class_id = INVALID_OBJECT;
	
	if (LoadGameOpen(filename) == False)
	{
		eprintf("LoadGame can't open %s to load the game, using default!\n",
			filename);
		return False;
	}
	savegame_version = 0;
	ret_val = LoadGameParse(filename);
	
	LoadGameClose();
	
	/* now free load game memory */
	
	for (i=0;i<load_game_classes_table_size;i++)
	{
		lgc = load_game_classes[i];
		while (lgc != NULL)
		{
			for (j=0;j<lgc->num_props;j++)
			{
					/* if the game crashes here, its likely that you've got a bad .bof
						and the saved games corrupted, check the lgc for the name of the bof to check */
					FreeMemory(MALLOC_ID_LOAD_GAME,lgc->props[j].prop_name,
								  lgc->props[j].namelen+1);
			}
			if (lgc->num_props > 0)
				FreeMemory(MALLOC_ID_LOAD_GAME,lgc->props,lgc->num_props*sizeof(load_game_prop_node));
			FreeMemory(MALLOC_ID_LOAD_GAME,lgc->class_name,strlen(lgc->class_name)+1);
		
			tempc = lgc->next;
			FreeMemory(MALLOC_ID_LOAD_GAME,lgc,sizeof(load_game_class_node));
			lgc = tempc;
		}
	}
	
	FreeMemory(MALLOC_ID_LOAD_GAME,load_game_classes,load_game_classes_table_size*sizeof(load_game_class_node *));
	load_game_classes = NULL;

	FreeISHash(load_game_resources);
	load_game_resources = NULL;

	end_time = GetMilliCount();
	dprintf("LoadGame exiting LoadGame %u seconds\n",(unsigned int)(end_time-start_time)/1000);
	
	return ret_val;
}

Bool LoadGameOpen(char *fname)
{
   loadfile.file = fopen(fname, "rb");
	return loadfile.file != NULL;
}

void LoadGameClose(void)
{
	fclose(loadfile.file);
}

Bool LoadGameParse(char *filename)
{
	char cmd;
	
	while (true)
	{
      if (fread(&cmd, 1, 1, loadfile.file) != 1)
      {
         if (feof(loadfile.file))
            return True;

         eprintf("File %s Line %i couldn't read type\n",
                 __FILE__,__LINE__);
         return False;
      }

      //      dprintf("load game %i\n",cmd);
		switch (cmd)
		{
		case SAVE_GAME_CLASS :
			if (!LoadGameClass())
				return False;
			break;
		case SAVE_GAME_RESOURCE :
			if (!LoadGameResource())
				return False;
			break;
		case SAVE_GAME_BUILTINOBJ :
			if (!LoadGameBuiltInObjects())
				return False;
			break;
		case SAVE_GAME_VERSION :
			if (!LoadGameVersion())
				return False;
			break;
		case SAVE_GAME_OBJECT :
			if (!LoadGameObject())
				return False;
			break;
		case SAVE_GAME_LIST_NODES :
			if (!LoadGameListNodes())
				return False;
			break;
		case SAVE_GAME_TABLES :
			if (!LoadGameTables())
				return False;
			break;
		case SAVE_GAME_TIMER :
			if (!LoadGameTimer())
				return False;
			break;
		case SAVE_GAME_USER :
			if (!LoadGameUser())
				return False;
			break;
		default :
			eprintf("LoadGameFile found invalid command byte %u at offset %i in %s\n",
                 cmd,ftell(loadfile.file),filename);
			return False;
		}
	}
	
	return True;
}

Bool LoadGameVersion(void)
{
   int temp;

   LoadGameReadInt(&temp);
   savegame_version = temp;

   return True;
}

Bool LoadGameBuiltInObjects()
{
   int obj_id, num_builtins, obj_constant;

   // Old save games just had system built-in object.
   if (savegame_version == 0)
   {
      LoadGameReadInt(&obj_id);
      SetSystemObjectID(obj_id);
      return True;
   }

   // Save game version 1 handles any number of built-ins.
   if (savegame_version == 1)
   {
      LoadGameReadInt(&num_builtins);
      for (int i = 0; i < num_builtins; ++i)
      {
         LoadGameReadInt(&obj_constant);
         LoadGameReadInt(&obj_id);
         SetBuiltInObjectID(obj_constant, obj_id);
      }
   }

   return True;
}

Bool LoadGameObject(void)
{
	int object_id,class_old_id,num_props,i;
	val_type prop_val;
	char *property_str;
	load_game_class_node *lgc;
	
	LoadGameReadInt(&object_id);
	LoadGameReadInt(&class_old_id);
	LoadGameReadInt(&num_props);
	
	lgc = GetLoadGameClassByID(class_old_id);
	if (lgc == NULL)
	{
		eprintf("LoadGameObject found object %i class id %i without class name\n",object_id,
			class_old_id);
		return False;
	}

	if (!LoadObject(object_id,lgc->class_name))
	{
		eprintf("LoadGameObject can't load object %i\n",object_id);
		return False;
	}
	current_object_id = object_id;

	for (i=1;i<=num_props;i++)
	{
		LoadGameReadInt(&prop_val);
		// eprintf("loading object %i\n",object_id);

		LoadGameTranslateVal(&prop_val);
		property_str = GetLoadGamePropertyNameByID(lgc,i);
		if (property_str == NULL)
		{
			eprintf("LoadGameObject found object %i class %s property %i without name\n",
				object_id,lgc->class_name,i);
			return False;
		}

		if (!SetObjectPropertyByName(current_object_id,property_str,prop_val))
		{
			//eprintf("LoadGameObject object %i class %s property %s can't set object property\n",
			//	object_id,lgc->class_name,property_str);
			// it's usually ok, property just eliminated in new kod
		}
	}
	
	return True;
}

Bool LoadGameListNodes(void)
{
	int num_list_nodes,i;
	val_type first_val,rest_val;
	
	LoadGameReadInt(&num_list_nodes);
	
	for (i=0;i<num_list_nodes;i++)
	{
		LoadGameReadInt(&first_val.int_val);
		LoadGameReadInt(&rest_val.int_val);
		
		LoadGameTranslateVal(&first_val);
		LoadGameTranslateVal(&rest_val);
		
		if (!LoadList(i,first_val,rest_val))
		{
			eprintf("LoadGameList can't set list node %i\n",i);
			return False;
		}
	}
	
	return True;
}

Bool LoadGameTables(void)
{
   int num_tables, size, num_entries, table_id;
   val_type key_val, data_val;
   table_node *t;

   LoadGameReadInt(&num_tables);

   for (int i = 0; i < num_tables; ++i)
   {
      LoadGameReadInt(&size);
      LoadGameReadInt(&num_entries);

      table_id = CreateTable(size);
      t = GetTableByID(table_id);
      if (t == NULL)
      {
         eprintf("LoadGameTables can't set table %i\n",i);
         return False;
      }

      if (table_id != i)
      {
         eprintf("LoadGameTables got incorrect table id %i, expected %i\n",
            table_id, i);
         return False;
      }

      if (t->size != size)
      {
         eprintf("LoadGameTables got invalid table size %i, expected %i\n",
            t->size, size);
         return False;
      }

      for (int j = 0; j < num_entries; ++j)
      {
         LoadGameReadInt(&key_val.int_val);
         LoadGameReadInt(&data_val.int_val);

         LoadGameTranslateVal(&key_val);
         LoadGameTranslateVal(&data_val);

         InsertTable(i, key_val, data_val);
      }
   }

   return True;
}

Bool LoadGameTimer(void)
{
	int timer_id,object_id,milliseconds;
	char buf[100];
	
	LoadGameReadInt(&timer_id);
	LoadGameReadInt(&object_id);
	LoadGameReadString(buf,sizeof(buf));
	LoadGameReadInt(&milliseconds);
	
	if (!LoadTimer(timer_id,object_id,buf,milliseconds))
	{
		eprintf("LoadGameTimer can't set timer %i\n",timer_id);
		/* still ok */
	}
	return True;
}

Bool LoadGameUser(void)
{   
	int object_id,account_id;
	
	LoadGameReadInt(&account_id);
	LoadGameReadInt(&object_id);
	
	LoadUser(account_id,object_id);
	return True;
}

Bool LoadGameClass(void)
{
	int class_old_id,num_props,i;
	char buf[100];
	load_game_class_node *lgc;
	
	LoadGameReadInt(&class_old_id);
	LoadGameReadString(buf,sizeof(buf));
	LoadGameReadInt(&num_props);
	
	lgc = CreateLoadGameClass(class_old_id,buf,num_props);
	
	for (i=1;i<=num_props;i++)
	{
      LoadGameReadString(buf, sizeof(buf));
		LoadAddPropertyName(lgc,i,buf);
	}   
	
	return True;
}


void LoadAddPropertyName(load_game_class_node *lgc,int prop_old_id,char *prop_name)
{
	load_game_prop_node *lgp;
	
	lgp = &lgc->props[prop_old_id-1];
	lgp->namelen = strlen( prop_name ) ;
	
	assert( lgp->namelen );

	lgp->prop_name = (char *)AllocateMemory(MALLOC_ID_LOAD_GAME,
														 lgp->namelen+1);
	strcpy(lgp->prop_name,prop_name);
}

Bool LoadGameResource(void)
{
	int resource_old_id;
	char buf[100];
	
	LoadGameReadInt(&resource_old_id);
	LoadGameReadString(buf,sizeof(buf));
	
	CreateLoadGameResource(resource_old_id,buf);
	return True;
}


load_game_class_node * CreateLoadGameClass(int class_old_id,char *class_name,int num_props)
{
	load_game_class_node *lgc;
	unsigned int hash_value;
	
	lgc = (load_game_class_node *)AllocateMemory(MALLOC_ID_LOAD_GAME,
		sizeof(load_game_class_node));
	
	lgc->class_old_id = class_old_id;
	lgc->class_name = (char *)AllocateMemory(MALLOC_ID_LOAD_GAME,
		strlen(class_name)+1);
	strcpy(lgc->class_name,class_name);
	lgc->num_props = num_props;
	lgc->props = NULL;
	if (num_props > 0)
		lgc->props = (load_game_prop_node *)AllocateMemory(MALLOC_ID_LOAD_GAME,
																			num_props*sizeof(load_game_prop_node));    

	hash_value = class_old_id % load_game_classes_table_size;
	lgc->next = load_game_classes[hash_value];
	load_game_classes[hash_value] = lgc;
	
	return lgc;
}

void CreateLoadGameResource(int resource_old_id,char *resource_name)
{
	ISHashInsert(load_game_resources,resource_old_id,resource_name);
}

load_game_class_node * GetLoadGameClassByID(int class_old_id)
{
	load_game_class_node *lgc;
	unsigned int hash_value;
	
	hash_value = class_old_id % load_game_classes_table_size;
	lgc = load_game_classes[hash_value];
	while (lgc != NULL)
	{
		if (lgc->class_old_id == class_old_id)
			return lgc;
		lgc = lgc->next;
	}
	return NULL;
}

char * GetLoadGamePropertyNameByID(load_game_class_node *lgc,int prop_old_id)
{
	if (prop_old_id < 1 || prop_old_id > lgc->num_props)
		return NULL;
	
	return lgc->props[prop_old_id-1].prop_name;
}

const char * GetLoadGameResourceByID(int resource_old_id)
{
	return ISHashFind(load_game_resources,resource_old_id);
}


void LoadGameTranslateVal(val_type *pval)
{
	load_game_class_node *lgc;
	class_node *c;
	const char *resource_name;
	resource_node *r;
	
	switch (pval->v.tag)
	{
	case TAG_NIL :
		pval->v.data = 0;
		break;
		
	case TAG_TEMP_STRING :
		eprintf("LoadGameTranslateVal found saved TAG_TEMP_STRING, converting to NIL\n");
		pval->v.tag = TAG_NIL;
		pval->v.data = 0;
		break;
		
	case TAG_CLASS :
		lgc = GetLoadGameClassByID(pval->v.data);
		if (lgc == NULL)
		{
			eprintf("LoadGameTranslateVal unable to get class %i\n",pval->v.data);
			break;
		}
		c = GetClassByName(lgc->class_name);
		if (c == NULL)
		{
			eprintf("LoadGameTranslateVal unable to lookup loaded class %s\n",
				lgc->class_name);
			break;
		}
		pval->v.data = c->class_id;
		
		break;
		
	case TAG_RESOURCE :
		if (pval->v.data >= MIN_DYNAMIC_RSC)
			break;

		resource_name = GetLoadGameResourceByID(pval->v.data);
		if (resource_name == NULL)
		{
			eprintf("LoadGameTranslateVal unable to get resource %i\n",pval->v.data);
			break;
		}

		r = GetResourceByName(resource_name);
		if (r == NULL)
		{
			eprintf("LoadGameTranslateVal unable to lookup loaded resource %s\n",
					  resource_name);
			break;
		}

		pval->v.data = r->resource_id;
	}
}
