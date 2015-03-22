// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* blakres.c
*

  This module keeps an open hash of resources in memory.  Each
  resource has an id number, its text, and possibly its name
  from the kodbase.  The resources are kept in sorted order by resource
  number so that when changing a dynamic rsc it is clear which dynamic
  rsc file will be rebuilt, and what resources go in it.
  
	Dynamic resources are loaded when a game is loaded, and contain
	character names.  They can be created in admin mode.  They are all
	resources with value MIN_DYNAMIC_RSC and higher.  They do not have names,
	just values.
	
*/

#include "blakserv.h"

static resource_node **resources;
static int resources_table_size;

static int next_dynamic_rsc;
static sihash_type resource_name_map;

/* local function prototypes */
void DynamicResourceChangeNotify(session_node *s);


void InitResource(void)
{
	int i;

	resources_table_size = ConfigInt(MEMORY_SIZE_RESOURCE_HASH);
	resources = (resource_node **)AllocateMemory(MALLOC_ID_RESOURCE,
																resources_table_size*sizeof(resource_node *));
	for (i=0;i<resources_table_size;i++)
		resources[i] = NULL;

	next_dynamic_rsc = MIN_DYNAMIC_RSC;

	resource_name_map = CreateSIHash(ConfigInt(MEMORY_SIZE_RESOURCE_NAME_HASH));
}

void ResetResource(void)
{
	resource_node *r,*temp;
	int i;

	for (i=0;i<resources_table_size;i++)
	{
		r = resources[i];
		while (r != NULL)
		{
			temp = r->next;

			FreeMemory(MALLOC_ID_RESOURCE,r,sizeof(resource_node));

			r = temp;
		}
		resources[i] = NULL;
	}

	FreeSIHash(resource_name_map);
	resource_name_map = CreateSIHash(ConfigInt(MEMORY_SIZE_RESOURCE_NAME_HASH));
}

void AddResource(int id,const char *str_value)
{
	int hash_num;
	resource_node *new_node;

	/* str_value is not permanent so need to make a copy here!
		Comes fromloadrsc or adddynamicrsc */
	
	if (GetResourceByID(id) != NULL)
	{
		eprintf("AddResource can't add resource num %i because it already exists!\n",id);
		return;
	}
	
	/* Ok, also check for dynamic resources being loaded/created here, in that we have
	to keep track of the next dynamic resource # to use. */
	if (id >= MIN_DYNAMIC_RSC && id >= next_dynamic_rsc)
	{
		next_dynamic_rsc = id + 1;
		/* dprintf("setting next dyn rsc to %i\n",next_dynamic_rsc); */
	}
	
	new_node = (resource_node *)AllocateMemory(MALLOC_ID_RESOURCE,sizeof(resource_node));
	new_node->resource_id = id;
	new_node->resource_val = (char *)AllocateMemory(MALLOC_ID_RESOURCE,strlen(str_value)+1);
	strcpy(new_node->resource_val,str_value);
	
	new_node->resource_name = NULL;
	
	/* add to resources table */
	hash_num = new_node->resource_id % resources_table_size;
	new_node->next = resources[hash_num];
	resources[hash_num] = new_node;
}

void SetResourceName(int id,char *name)
{
   resource_node *r;

   r = GetResourceByID(id);
   if (r == NULL)
   {
      eprintf("SetResourceName can't find resource id %i\n",id);
      return;
   }

   r->resource_name = (char *)AllocateMemory(MALLOC_ID_KODBASE,strlen(name)+1);
   strcpy(r->resource_name,name);

	SIHashInsert(resource_name_map,name,id);
}

static resource_node *notify_r;

int AddDynamicResource(const char *str_value)
{
	int new_rsc_id;
	resource_node *r;
	
	new_rsc_id = next_dynamic_rsc;
	AddResource(new_rsc_id,str_value);
	
	r = GetResourceByID(new_rsc_id);
	if (r == NULL)
		eprintf("AddDynamicResource created resource %i, now can't get\n",new_rsc_id);
	else
	{
		/* now notify everyone in game */
		notify_r = r;
		ForEachSession(DynamicResourceChangeNotify);
	}
	
	return new_rsc_id;
}

void ChangeDynamicResourceStr(resource_node *r,char *str_value)
{
	ChangeDynamicResource(r,str_value,strlen(str_value));
}

void ChangeDynamicResource(resource_node *r,char *data,int len_data)
{
	if (r == NULL)
	{
		eprintf("ChangeDynamicResource got passed a null resource\n");
		return;
	}
	
	FreeMemory(MALLOC_ID_RESOURCE,r->resource_val,strlen(r->resource_val)+1);
	r->resource_val = (char *)AllocateMemory(MALLOC_ID_RESOURCE,len_data+1);
	memcpy(r->resource_val,data,len_data);
	r->resource_val[len_data] = 0; /* null terminate */
	
	/* now notify everyone in game */
	notify_r = r;
	ForEachSession(DynamicResourceChangeNotify);
}

void DynamicResourceChangeNotify(session_node *s)
{
	if (s->state == STATE_GAME)
	{
		AddByteToPacket(BP_CHANGE_RESOURCE);
		AddIntToPacket(notify_r->resource_id);
		AddStringToPacket(strlen(notify_r->resource_val),notify_r->resource_val);
		SendPacket(s->session_id);
	}
}

resource_node * GetResourceByID(int id)
{
	resource_node *r;

	r = resources[id % resources_table_size];
	while (r != NULL)
	{
		if (r->resource_id == id)
			return r;
		r = r->next;
	}

	return NULL;
}

Bool IsResourceByID(int id)
{
	resource_node *r = GetResourceByID(id);
	if (r == NULL)
		return False;
	return True;
}

resource_node * GetResourceByName(const char *resource_name)
{
	int resource_id;
	Bool found;

	found = SIHashFind(resource_name_map,resource_name,&resource_id);
	if (found)
		return GetResourceByID(resource_id);

	return NULL;
}

void ForEachResource(void (*callback_func)(resource_node *r))
{
	resource_node *r;
	int i;
	
	for (i=0;i<resources_table_size;i++)
	{
		r = resources[i];
		while (r != NULL)
		{
			if (r->resource_id < MIN_DYNAMIC_RSC)
				callback_func(r);
			r = r->next;
		}
	}
}

/* This is for saving dynamic resources */
void ForEachDynamicRsc(void (*callback_func)(resource_node *r))
{
	resource_node *r;
	int i;
	
	for (i=0;i<resources_table_size;i++)
	{
		r = resources[i];
		while (r != NULL)
		{
			if (r->resource_id >= MIN_DYNAMIC_RSC)
				callback_func(r);
			r = r->next;
		}
	}
}


