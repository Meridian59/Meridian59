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
	size_t len_s; \
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
void SaveGameVersion(void);
void SaveClasses(void);
void SaveEachClass(class_node *c);
void SaveResources(void);
void SaveEachResource(resource_node *r);
void SaveBuiltInObjects(void);
void SaveObjects(void);
void SaveEachObject(object_node *o);
void SaveListNodes(void);
void SaveEachListNode(list_node *l,int list_id);
void SaveTables(void);
void SaveEachTable(table_node *t,int table_id);
void SaveTimers(void);
void SaveEachTimer(timer_node *t);
void SaveUsers(void);
void SaveEachUser(user_node *u);

// Functions for writing to the save game buffer.
void SaveGameCopyByteBuffer(char byte_buffer);
void SaveGameCopyIntBuffer(int int_buffer);
void SaveGameCopyStringBuffer(const char *string_buffer);
// Used to flush the buffer and write to file.
void SaveGameFlushBuffer();

// buffer is used for buffering data to write at one time, vs writing multiple
// times with small amount of data.
static char *buffer;

// buffer_position is the current position of buffer we're writing to, and the
// length that will be written to file if flushed.
static int buffer_position;

// buffer_size is the size of the currently allocated memory for buffer.
static int buffer_size;

// buffer_warning_size is used to check if we need to flush the buffer. Equal
// to 90% buffer_size.
static int buffer_warning_size;

Bool SaveGame(char *filename)
{
   savefile = fopen(filename,"wb");
   if (savefile == NULL)
   {
      eprintf("SaveGame can't open %s to save everything!!!\n",filename);
      return False;
   }

   // Buffer for buffering data to call fwrite() once. Used to speed up
   // saving for objects, lists, tables and users.
   buffer_position = 0;
   buffer = (char *) AllocateMemory(MALLOC_ID_SAVE_GAME, SAVEGAME_BUFFER);
   buffer_size = SAVEGAME_BUFFER;
   buffer_warning_size = (SAVEGAME_BUFFER / 10) * 9;

   SaveGameVersion();
   SaveClasses();
   SaveResources();
   SaveBuiltInObjects();
   SaveObjects();
   SaveListNodes();
   SaveTables();
   SaveTimers(); 
   SaveUsers();

   if (buffer_position > 0)
      SaveGameFlushBuffer();

   fclose(savefile);

   // Free buffer memory.
   FreeMemory(MALLOC_ID_SAVE_GAME, buffer, buffer_size);

   return True;
}

// Write 4 bytes to buffer.
void SaveGameCopyIntBuffer(int int_buffer)
{
   memcpy(&(buffer[buffer_position]), &int_buffer, 4);
   buffer_position += 4;

   // Flush buffer at 90%.
   if (buffer_position > buffer_warning_size)
      SaveGameFlushBuffer();
}

// Write 1 byte to buffer.
void SaveGameCopyByteBuffer(char byte_buffer)
{
   memcpy(&(buffer[buffer_position]), &byte_buffer, 1);
   buffer_position++;

   // Flush buffer at 90%.
   if (buffer_position > buffer_warning_size)
      SaveGameFlushBuffer();
}

// Write a string to buffer.
void SaveGameCopyStringBuffer(const char *string_buffer)
{
   size_t len_s;

   if (string_buffer == NULL)
   {
      eprintf("SaveGameCopyStringBuffer line %i can't save NULL string, invalid saved game\n", \
         __LINE__);
      len_s = 0;
      memcpy(&(buffer[buffer_position]), &len_s, 2);
      buffer_position += 2;
   }
   else
   {
      len_s = strlen(string_buffer);
      memcpy(&(buffer[buffer_position]), &len_s, 2);
      buffer_position += 2;
      memcpy(&(buffer[buffer_position]), string_buffer, len_s);
      buffer_position += len_s;
   }

   // Flush buffer at 90%.
   if (buffer_position > buffer_warning_size)
      SaveGameFlushBuffer();
}

// Write buffer to save game file, reset buffer position.
void SaveGameFlushBuffer()
{
   if (fwrite(buffer, buffer_position, 1, savefile) != 1)
      eprintf("File %s Line %i error writing to file!\n", __FILE__, __LINE__);
   buffer_position = 0;
}

void SaveGameVersion(void)
{
   SaveGameCopyByteBuffer(SAVE_GAME_VERSION);
   SaveGameCopyIntBuffer(SAVEGAME_VERS);
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

   SaveGameCopyByteBuffer(SAVE_GAME_CLASS);
   SaveGameCopyIntBuffer(c->class_id);
   SaveGameCopyStringBuffer(c->class_name);
   SaveGameCopyIntBuffer(c->num_properties);

   for (i=1;i<=c->num_properties;i++)
   {
      const char *s = GetPropertyNameByID(c,i);
      if (!s)
      {
         eprintf("SaveEachClass cannot find property name for index %i in %s\n",i,c->fname);
         continue;
      }
      SaveGameCopyStringBuffer(s);
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

   SaveGameCopyByteBuffer(SAVE_GAME_RESOURCE);
   SaveGameCopyIntBuffer(r->resource_id);
   SaveGameCopyStringBuffer(r->resource_name);
}

// Write the save game type (SAVE_GAME_BUILTINOBJ), followed by the number
// of built-in objects, then each object constant followed by the object ID.
void SaveBuiltInObjects(void)
{
   SaveGameCopyByteBuffer(SAVE_GAME_BUILTINOBJ);
   SaveGameCopyIntBuffer(NUM_BUILTIN_OBJECTS);
   SaveGameCopyIntBuffer(SYSTEM_OBJECT);
   SaveGameCopyIntBuffer(GetSystemObjectID());
   SaveGameCopyIntBuffer(SETTINGS_OBJECT);
   SaveGameCopyIntBuffer(GetSettingsObjectID());
   SaveGameCopyIntBuffer(REALTIME_OBJECT);
   SaveGameCopyIntBuffer(GetRealTimeObjectID());
   SaveGameCopyIntBuffer(EVENTENGINE_OBJECT);
   SaveGameCopyIntBuffer(GetEventEngineObjectID());
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

   SaveGameCopyByteBuffer(SAVE_GAME_OBJECT);
   SaveGameCopyIntBuffer(o->object_id);
   SaveGameCopyIntBuffer(o->class_id);
   SaveGameCopyIntBuffer(c->num_properties);

   /* remember, don't save self (p[0]) because no name for it!
    * loadall will set self for us, fortunately.
    */
   /* equal to num_properties is ok, because self = prop 0 */
   for (i = 1; i <= c->num_properties; ++i)
      SaveGameCopyIntBuffer(o->p[i].val.int_val);
}

void SaveListNodes(void)
{
   SaveGameCopyByteBuffer(SAVE_GAME_LIST_NODES);
   SaveGameCopyIntBuffer(GetListNodesUsed());
   ForEachListNode(SaveEachListNode);
}

void SaveEachListNode(list_node *l,int list_id)
{
   SaveGameCopyIntBuffer(l->first.int_val);
   SaveGameCopyIntBuffer(l->rest.int_val);
}

void SaveTables(void)
{
   SaveGameCopyByteBuffer(SAVE_GAME_TABLES);
   SaveGameCopyIntBuffer(GetTablesUsed());
   ForEachTable(SaveEachTable);
}

void SaveEachTable(table_node *t,int table_id)
{
   hash_node *hn;

   SaveGameCopyIntBuffer(t->size);
   SaveGameCopyIntBuffer(t->num_entries);

   for (int i = 0; i < t->size; ++i)
   {
      hn = t->table[i];
      while (hn != NULL)
      {
         SaveGameCopyIntBuffer(hn->key_val.int_val);
         SaveGameCopyIntBuffer(hn->data_val.int_val);
         hn = hn->next;
      }
   }
}

void SaveTimers(void)
{
   ForEachTimer(SaveEachTimer);
}

void SaveEachTimer(timer_node *t)
{
   int save_time;
   object_node *o;
   char *s;

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

   SaveGameCopyByteBuffer(SAVE_GAME_TIMER);
   SaveGameCopyIntBuffer(t->timer_id);
   SaveGameCopyIntBuffer(t->object_id);
   SaveGameCopyStringBuffer(GetNameByID(t->message_id));

   save_time = (int)(t->time - GetMilliCount());
   if (save_time < 0)
      save_time = 0;
   SaveGameCopyIntBuffer(save_time);
}

void SaveUsers(void)
{
   ForEachUser(SaveEachUser);
}

void SaveEachUser(user_node *u)
{
   SaveGameCopyByteBuffer(SAVE_GAME_USER);
   SaveGameCopyIntBuffer(u->account_id);
   SaveGameCopyIntBuffer(u->object_id);
}
