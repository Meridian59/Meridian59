// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* savegame.c
*

  This module saves game information to the file, so it can be loaded
  in by loadgame.c at some future time.
  
*/

#include "blakserv.h"

typedef struct
{
	unsigned int data:28;
	unsigned int tag:4;
} unsigned_type;

FILE *savefile;

#define SaveGameWrite(buf,len) \
{ \
	if (fwrite(buf,len,1,savefile) != 1) \
	eprintf("File %s Line %i error writing to file!\n",__FILE__,__LINE__); \
} 

#define SaveGameWriteByte(byte) \
{ \
	char ch; \
	ch = byte; \
	if (fwrite(&ch,1,1,savefile) != 1) \
	eprintf("File %s Line %i error writing to file!\n",__FILE__,__LINE__); \
} 

#define SaveGameWriteInt(num) \
{ \
	int temp; \
	temp = num; \
	if (fwrite(&temp,4,1,savefile) != 1) \
	eprintf("File %s Line %i error writing to file!\n",__FILE__,__LINE__); \
} 

#define SaveGameWriteString(s) \
{ \
	unsigned short len_s; \
	if (s == NULL) \
{ \
	eprintf("SaveGameWriteString line %i can't save NULL string, invalid saved game\n", \
	__LINE__); \
	len_s = 0; \
	SaveGameWrite(&len_s,2); \
	return; \
} \
	len_s = strlen(s); \
	SaveGameWrite(&len_s,2); \
	SaveGameWrite(s,len_s); \
}



/* local function prototypes */

void SaveClasses(void);
void SaveEachClass(class_node *c);
void SaveResources(void);
void SaveEachResource(resource_node *r);
void SaveSystem(void);
void SaveObjects(void);
void SaveEachObject(object_node *o);
void SaveListNodes(void);
void SaveEachListNode(list_node *l,int list_id);
void SaveTimers(void);
void SaveEachTimer(timer_node *t);
void SaveUsers(void);
void SaveEachUser(user_node *u);
void SaveGameCountEachDynamicRsc(resource_node *r);

const char *GetTagShortName(val_type val);

Bool SaveGame(char *filename)
{
	savefile = fopen(filename,"wb");
	if (savefile == NULL)
	{
		eprintf("SaveGame can't open %s to save everything!!!\n",filename);
		return False;
	}
	
	SaveClasses();
	SaveResources();
	SaveSystem();
	SaveObjects();
	SaveListNodes();
	SaveTimers(); 
	SaveUsers();
	
	fclose(savefile);
	
	return True;
}

void SaveClasses(void)
{
	ForEachClass(SaveEachClass);
}

void SaveEachClass(class_node *c)
{
	int i;
	
	if ((GetClassByID(c->class_id) != c) ||
		(c->super_id != NO_SUPERCLASS && GetClassByID(c->super_id) == NULL) ||
		(c->class_name == NULL))
	{
		eprintf("SaveEachClass is not saving invalid class %i\n",c->class_id);
		return;
	}
	
	SaveGameWriteByte(SAVE_GAME_CLASS);
	SaveGameWriteInt(c->class_id);
	SaveGameWriteString(c->class_name);
	SaveGameWriteInt(c->num_properties);
	
	for (i=1;i<=c->num_properties;i++)
	{
		const char *s = GetPropertyNameByID(c,i);
		if (!s)
		{
			eprintf("SaveEachClass cannot find property name for index %i in %s\n",i,c->fname);
			continue;
		}
		SaveGameWriteString(s);
	}
}

void SaveResources(void)
{
	ForEachResource(SaveEachResource);
}

void SaveEachResource(resource_node *r)
{
	if ((r->resource_name == NULL))
	{
		eprintf("SaveEachResource is not saving invalid resource %i\n",r->resource_id);
		return;
	}
	
	SaveGameWriteByte(SAVE_GAME_RESOURCE);
	SaveGameWriteInt(r->resource_id);
	SaveGameWriteString(r->resource_name);
}

void SaveSystem(void)
{
	SaveGameWriteByte(SAVE_GAME_SYSTEM);
	SaveGameWriteInt(GetSystemObjectID());
}

void SaveObjects(void)
{
	ForEachObject(SaveEachObject);
}

void SaveEachObject(object_node *o)
{
	int i;
	class_node *c;
	
	c = GetClassByID(o->class_id);
	if (c == NULL)
	{
		eprintf("SaveEachObject can't get class %i\n",o->class_id);
		return;
	}
	
	SaveGameWriteByte(SAVE_GAME_OBJECT);
	SaveGameWriteInt(o->object_id);
	SaveGameWriteInt(o->class_id);
	SaveGameWriteInt(c->num_properties);
	
	/* remember, don't save self (p[0]) because no name for it!
    * loadall will set self for us, fortunately.
    */
	/* equal to num_properties is ok, because self = prop 0 */
	for (i=1;i<=c->num_properties;i++)
		SaveGameWriteInt(o->p[i].val.int_val);
}

void SaveListNodes(void)
{
	SaveGameWriteByte(SAVE_GAME_LIST_NODES);
	SaveGameWriteInt(GetListNodesUsed());
	ForEachListNode(SaveEachListNode);
}

void SaveEachListNode(list_node *l,int list_id)
{
	SaveGameWriteInt(l->first.int_val);
	SaveGameWriteInt(l->rest.int_val);
}

void SaveTimers(void)
{
	ForEachTimer(SaveEachTimer);
}

void SaveEachTimer(timer_node *t)
{
	int save_time;
	object_node *o;
	const char *s;
	
	o = GetObjectByID(t->object_id);
	if (o == NULL)
	{
		eprintf("SaveEachTimer can't get object %i\n",t->object_id);
		return;
	}
	
	s = GetNameByID(t->message_id);
	if (!s)
	{
		eprintf("SaveEachTimer is not saving invalid timer %i\n",t->object_id);
		return;
	}
	
	SaveGameWriteByte(SAVE_GAME_TIMER);
	SaveGameWriteInt(t->timer_id);
	SaveGameWriteInt(t->object_id);
	SaveGameWriteString(GetNameByID(t->message_id));
	
	save_time = (int)(t->time - GetMilliCount());
	if (save_time < 0)
		save_time = 0;
	SaveGameWriteInt(save_time);
}

void SaveUsers(void)
{
	ForEachUser(SaveEachUser);
}

void SaveEachUser(user_node *u)
{
	SaveGameWriteByte(SAVE_GAME_USER);
	SaveGameWriteInt(u->account_id);
	SaveGameWriteInt(u->object_id);
}

/* used in saved game to save space */
const char *GetTagShortName(val_type val)
{
	switch (val.v.tag)
	{
		case TAG_NIL : return "$";
		case TAG_INT : return "I";
		case TAG_OBJECT : return "O";
		case TAG_LIST : return "L";
		case TAG_RESOURCE : return "R";
	}

	return GetTagName(val);
}
