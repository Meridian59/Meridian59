// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * config.c
 *

 This module takes care of all the configurable parts of Blakserv,
 except for the Blakod.  It reads data from a file, or uses default
 values in the table defining all the configurable data.

 We startup the memory subsystem before this, even though it uses
 channels for error reporting.  Therefore, in here you MUST
 make sure to not pass a bad value in to AllocateMemory for the
 id, since it will crash since channels don't exist yet.  We
 need to do this because the dynamic configurable guys will
 be reallocated.

 Because configuration is done early, before channels, we can't use
 normal error reporting functions.  Therefore, use StartupPrintf()
 in here for error messages.

 It is necessary to allow some of these configurable things to change
 at runtime.  Since multiple threads access these guys, we can't
 just let anyone get strings that could be changed by another
 thread, so we have a cs to guard in that case.  Dynamic strings
 can only be grabbed by using a special function that locks
 dynamic config, and then released by another function.  Dynamic
 ints and bools are fine, since we lock and unlock here, only
 returning an int or bool.

*/

#include "blakserv.h"

#define MAX_CONFIG_LINE 200

typedef struct
{
   int config_id;
   bool is_dynamic;
   const char *config_name;
   int config_type;
   const char *default_str;
} config_table_type;

config_table_type config_table[] =
{
{ PATH_GROUP,             false, "[Path]",        CONFIG_GROUP, "" },
{ PATH_BOF,               false, "Bof",           CONFIG_PATH,  "." },
{ PATH_MEMMAP,            false, "Memmap",        CONFIG_PATH,  "." },
{ PATH_RSC,               false, "Rsc",           CONFIG_PATH,  "." },
{ PATH_ROOMS,             false, "Rooms",         CONFIG_PATH,  "." },
{ PATH_MOTD,              false, "Motd",          CONFIG_PATH,  "." },
{ PATH_CHANNEL,           false, "Channel",       CONFIG_PATH,  "." },
{ PATH_LOADSAVE,          false, "LoadSave",      CONFIG_PATH,  "." },
{ PATH_FORMS,             false, "Forms",         CONFIG_PATH,  "." },
{ PATH_KODBASE,           false, "Kodbase",       CONFIG_PATH,  "." },
{ PATH_PACKAGE_FILE,      false, "PackageFile",   CONFIG_PATH,  "." },

{ SOCKET_GROUP,           false, "[Socket]",      CONFIG_GROUP, "" },
{ SOCKET_PORT,            false, "Port",          CONFIG_INT,   "5959" },
{ SOCKET_MAINTENANCE_PORT,false, "MaintenancePort",CONFIG_INT,  "9998" },
{ SOCKET_MAINTENANCE_MASK,false, "MaintenanceMask",CONFIG_STR,  "208.192.72.0" },
{ SOCKET_DNS_LOOKUP,      true, "DNSLookup",     CONFIG_BOOL,  "No" },
{ SOCKET_NAGLE,           false, "Nagle",         CONFIG_BOOL,  "Yes" },
{ SOCKET_BLOCK_TIME,      true, "BlockTime",     CONFIG_INT,   "300" }, /* seconds */

{ CHANNEL_GROUP,          false, "[Channel]",     CONFIG_GROUP, "" },
{ CHANNEL_DEBUG_DISK,     false, "DebugDisk",     CONFIG_BOOL,  "No" },
{ CHANNEL_ERROR_DISK,     false, "ErrorDisk",     CONFIG_BOOL,  "No" },
{ CHANNEL_LOG_DISK,       false, "LogDisk",       CONFIG_BOOL,  "No" },
{ CHANNEL_FLUSH,          true, "Flush",         CONFIG_BOOL,  "No" },

{ LOGIN_GROUP,            false, "[Login]",       CONFIG_GROUP, "" },
{ LOGIN_MAX_ATTEMPTS,     false, "MaxAttempts",   CONFIG_INT,   "3" },
{ LOGIN_MIN_VERSION,      true, "MinVersion",    CONFIG_INT,   "0" },
{ LOGIN_OLD_VERSION_STR,  false, "OldVersionStr", CONFIG_STR,
     "The game software has been upgraded while you have been online. Logoff and "
     "then login again to automatically upgrade your software." },
{ LOGIN_INVALID_VERSION,  true, "InvalidVersion",CONFIG_INT,   "100" },
{ LOGIN_INVALID_VERSION_STR,false, "InvalidVersionStr",CONFIG_STR,
    "Your version of the game software is beta; you need to purchase the latest version." },
{ LOGIN_SUSPEND_STR,      false, "SuspendStr", CONFIG_STR,
     "Your account has been disabled temporarily. "
	 "Check your email to see if the administrator has sent you a message." },
{ LOGIN_MAX_PER_IP,       true, "MaxPerIPAddress", CONFIG_INT, "0" },
{ LOGIN_TOO_MANY_PER_IP_STR,false, "TooManyPerIPAddressStr", CONFIG_STR,
      "Too many logins from the same IP address." },

{ INACTIVE_GROUP,         false, "[Inactive]",    CONFIG_GROUP, "" },
{ INACTIVE_SYNCHED,       true, "Synched",       CONFIG_INT,   "10" }, /* minutes */
{ INACTIVE_TRANSFER,      true, "Transfer",      CONFIG_INT,   "2" }, /* minutes */
{ INACTIVE_SELECTCHAR,    true, "SelectChar",    CONFIG_INT,   "10" }, /* minutes */
{ INACTIVE_GAME,          true, "Game",          CONFIG_INT,   "30" }, /* seconds */
{ INACTIVE_MAINTENANCE,   true, "Maintenance",   CONFIG_INT,   "30" }, /* seconds */
{ INACTIVE_OVERRIDE,      true, "Override",      CONFIG_BOOL,  "Yes" },

{ MOTD_GROUP,             false, "[MessageOfTheDay]", CONFIG_GROUP, "" },
{ MOTD_DEFAULT,           false, "Default",       CONFIG_STR,   "" },

{ CREDIT_GROUP,           false, "[Credit]",      CONFIG_GROUP, "" },
{ CREDIT_DRAIN_AMOUNT,    false, "DrainAmount",   CONFIG_INT,   "-1" },
{ CREDIT_DRAIN_TIME,      false, "DrainTime",     CONFIG_INT,   "1" },
{ CREDIT_WARN1,           false, "Warn1",         CONFIG_INT,   "5" },
{ CREDIT_WARN2,           false, "Warn2",         CONFIG_INT,   "1" },
{ CREDIT_INIT,            false, "Initial",       CONFIG_INT,   "0" },
{ CREDIT_ADMIN,           true, "Admin",         CONFIG_INT,   "25" },

{ SESSION_GROUP,          false, "[Session]",     CONFIG_GROUP, "" },
{ SESSION_MAX_ACTIVE,     true, "MaxActive",     CONFIG_INT,   "300" },
{ SESSION_MAX_CONNECT,    false, "MaxConnect",    CONFIG_INT,   "300" },
{ SESSION_BUSY,           false, "Busy",          CONFIG_STR,
     "Too many people are logged on right now; please try again later." },

{ LOCK_GROUP,             false, "[Lock]",        CONFIG_GROUP, "" },
{ LOCK_DEFAULT,           false, "Default",       CONFIG_STR,
     "The game is temporarily closed for maintenance." },

{ MEMORY_GROUP,           false, "[Memory]",      CONFIG_GROUP, "" },
{ MEMORY_SIZE_CLASS_HASH, false, "SizeClassHash", CONFIG_INT,   "99971" },
{ MEMORY_SIZE_CLASS_NAME_HASH, false, "SizeClassNameHash", CONFIG_INT,   "99971" },
{ MEMORY_SIZE_RESOURCE_HASH, false, "SizeResourceHash", CONFIG_INT,"99971" },
{ MEMORY_SIZE_RESOURCE_NAME_HASH, false, "SizeResourceNameHash", CONFIG_INT,"99971" },
{ MEMORY_SIZE_PROPERTIES_NAME_HASH, false, "SizePropertiesNameHash", CONFIG_INT,   "499" },

{ AUTO_GROUP,             false, "[Auto]",        CONFIG_GROUP, "" },
{ AUTO_GARBAGE_TIME,      false, "GarbageTime",   CONFIG_INT,   "90", }, /* minutes */
{ AUTO_GARBAGE_PERIOD,    false, "GarbagePeriod", CONFIG_INT,   "180", }, /* minutes */
{ AUTO_SAVE_TIME,         false, "SaveTime",      CONFIG_INT,   "0", }, /* minutes */
{ AUTO_SAVE_PERIOD,       false, "SavePeriod",    CONFIG_INT,   "180", }, /* minutes */
{ AUTO_KOD_TIME,          false, "KodTime",       CONFIG_INT,   "0", },
{ AUTO_KOD_PERIOD,        false, "KodPeriod",     CONFIG_INT,   "5", },
{ AUTO_INTERFACE_UPDATE,  false, "InterfaceUpdate",CONFIG_INT,  "5", },
{ AUTO_TRANSMITTED_TIME,  false, "TransmittedTime",CONFIG_INT,  "0", },
{ AUTO_TRANSMITTED_PERIOD,false, "TransmittedPeriod",CONFIG_INT,"60", }, /* seconds */
{ AUTO_RESET_POOL_TIME,   false, "ResetPoolTime", CONFIG_INT,   "0", },
{ AUTO_RESET_POOL_PERIOD, false, "ResetPoolPeriod",CONFIG_INT,  "60", },
{ AUTO_REOPEN_CHANNELS_TIME, false, "ReopenChannelsTime", CONFIG_INT,   "0", },
{ AUTO_REOPEN_CHANNELS_PERIOD, false, "ReopenChannelsPeriod",CONFIG_INT,  "86400", },

{ EMAIL_GROUP,            false, "[Email]",       CONFIG_GROUP, "" },
{ EMAIL_LISTEN,           false, "Listen",        CONFIG_BOOL,  "No" },
{ EMAIL_PORT,             false, "Port",          CONFIG_INT,   "25" },
{ EMAIL_ACCOUNT_CREATE_NAME, false, "AccountCreateName", CONFIG_STR, "account-create" },
{ EMAIL_ACCOUNT_DELETE_NAME, false, "AccountDeleteName", CONFIG_STR, "account-delete" },
{ EMAIL_LOCAL_MACHINE_NAME, true, "LocalMachineName", CONFIG_STR, "unknown" },
{ EMAIL_HOST,			  false, "MailServer",    CONFIG_STR,	   "ms-camaro" },
{ EMAIL_NAME,			  false, "PageAddress",    CONFIG_STR,	   "nobody@dev.null" },

{ UPDATE_GROUP,           false, "[Update]",      CONFIG_GROUP, "" },
{ UPDATE_CLIENT_MACHINE,  true, "ClientMachine", CONFIG_STR,   "unknown" },
{ UPDATE_CLIENT_FILE,     true, "ClientFilename",CONFIG_STR,   "unknown" },
{ UPDATE_PACKAGE_MACHINE, true, "PackageMachine",CONFIG_STR,   "unknown" },
{ UPDATE_PACKAGE_PATH,    true, "PackagePath",   CONFIG_STR,   "unknown" },
{ UPDATE_DOWNLOAD_REASON, true, "DownloadReason",CONFIG_STR,
     "An update of Meridian files is available and required." },
{ UPDATE_DEMO_BUTTON,	  true, "DemoButton"    ,CONFIG_STR,   "http://meridian59.neardeathstudios.com/M59-Download-01.shtml" },

{ CONSOLE_GROUP,          false, "[Console]",     CONFIG_GROUP, "" },
{ CONSOLE_ADMINISTRATOR,  false, "Administrator", CONFIG_STR,   "Administrator" },
{ CONSOLE_CAPTION,        false, "Caption",       CONFIG_STR,   "BlakSton Server" },

{ RIGHTS_GROUP,           false, "[Rights]",      CONFIG_GROUP, "" },
{ RIGHTS_GOROOM,          true, "GoRoom",        CONFIG_INT,   "2" },
{ RIGHTS_GOROOMBYNUM,     true, "GoRoomByNum",   CONFIG_INT,   "1" },
{ RIGHTS_GOPLAYER,        true, "GoPlayer",      CONFIG_INT,   "2" },
{ RIGHTS_GETPLAYER,       true, "GetPlayer",     CONFIG_INT,   "1" },

{ CONSTANTS_GROUP,        false, "[Constants]",   CONFIG_GROUP, "" },
{ CONSTANTS_ENABLED,      false, "Enabled",       CONFIG_BOOL,  "No" },
{ CONSTANTS_FILENAME,     false, "Filename",      CONFIG_STR,   ".\blakston.khd" },

{ ADVERTISE_GROUP,        false, "[Advertise]",   CONFIG_GROUP, "" },
{ ADVERTISE_FILE1,        true, "File1",         CONFIG_STR,   "ad1.bmp" },
{ ADVERTISE_URL1,         true, "Url1",          CONFIG_STR,   "https://discord.gg/meridian59" },
{ ADVERTISE_FILE2,        true, "File2",         CONFIG_STR,   "ad2.bmp" },
{ ADVERTISE_URL2,         true, "Url2",          CONFIG_STR,   "https://www.meridian59.com" },

{ DEBUG_GROUP,            false, "[Debug]",       CONFIG_GROUP, "" },
{ DEBUG_CANMOVEINROOM,    true, "CanMoveInRoom", CONFIG_BOOL,  "No" },
{ DEBUG_HEAP,             true, "Heap",          CONFIG_BOOL,  "No" },
{ DEBUG_TRANSMITTED_BYTES,true, "TransmittedBytes",CONFIG_BOOL,"No" },
{ DEBUG_HASH,             true, "Hash",          CONFIG_BOOL,  "No" },
{ DEBUG_INITPROPERTIES,   true, "InitProperties",CONFIG_BOOL,  "No" },
{ DEBUG_INITLOCALS,       true, "InitLocals",    CONFIG_BOOL,  "No" },
{ DEBUG_UNINITIALIZED,    true, "Uninitialized", CONFIG_BOOL,  "No" },

{ SECURITY_GROUP,         false, "[Security]",    CONFIG_GROUP, "" },
{ SECURITY_LOG_SPOOFS,    true, "LogSpoofs",     CONFIG_BOOL,  "Yes" },
{ SECURITY_HANGUP_SPOOFS, true, "HangupSpoofs",  CONFIG_BOOL,  "Yes" },
{ SECURITY_REDBOOK_RSC,   true, "RedbookRsc",    CONFIG_STR,   "system_success_rsc" },
	/* RedbookRsc is dynamic, but changes only take effect on garbage collection */

{ SERVICE_GROUP,          false, "[Service]",     CONFIG_GROUP, "" },
{ SERVICE_ENABLED,        true, "Enabled",       CONFIG_BOOL,  "No" },
{ SERVICE_MACHINE,        true, "Machine",       CONFIG_STR,   "ftp.neardeathstudios.com" },
{ SERVICE_DIRECTORY,      true, "Directory",     CONFIG_STR,   "/private/m59/service" },
{ SERVICE_USERNAME,       true, "Username",      CONFIG_STR,   "m59ftp" },
{ SERVICE_PASSWORD,       true, "Password",      CONFIG_STR,   "b58Iz3xp" },

{ BLAKOD_GROUP,           false, "[Blakod]",      CONFIG_GROUP, "" },
{ BLAKOD_MAX_STATEMENTS,  true, "MaxStatements", CONFIG_INT,   "20000000" },

};

enum
{
   LEN_CONFIG_TABLE = sizeof(config_table)/sizeof(config_table_type)
};

config_node configs[NUM_CONFIG_VALUES];

Mutex mutex_dynamic_config;


/* local function prototypes */
const char * AddConfig(int config_id,const char *config_data,int config_type,int is_dynamic);
void LockDynamicConfig(void);
void UnlockDynamicConfig(void);
int LoadConfigLine(char *line,int lineno,const char *filename,int current_group);


void InitConfig(void)
{
   int i;

   for (i=0;i<NUM_CONFIG_VALUES;i++)
   {
      configs[i].config_type = CONFIG_NONE;
      configs[i].config_str_value = NULL;
   }

   mutex_dynamic_config = MutexCreate();
}

void ResetConfig(void)
{

}

/* returns error string, NULL if ok */
const char * AddConfig(int config_id,const char *config_data,int config_type,int is_dynamic)
{
   config_node *c;
   size_t len;
   int num;
   struct stat file_stat;
   char s[MAX_CONFIG_LINE];

   c = GetConfigByID(config_id);
   if (c != NULL)
      return "config option listed more than once";

   c = &configs[config_id];

   c->is_dynamic = is_dynamic;

   strncpy(s,config_data, sizeof(s));

   switch (config_type)
   {
   case CONFIG_GROUP :
      break;

   case CONFIG_PATH :
      len = strlen(s);

      if (s[len-1] == '\\' || s[len-1] == '/')
		  s[len-1] = 0;

      if (stat(s,&file_stat) != 0 || !(file_stat.st_mode & S_IFDIR))
		  return "invalid path--not found";

#ifdef BLAK_PLATFORM_WINDOWS
      if (s[len-1] != ':')
		  strcat(s,"\\");
#elif BLAK_PLATFORM_LINUX
	  strcat(s,"/");
#endif
      c->config_str_value = (char *)AllocateMemory(MALLOC_ID_CONFIG,strlen(s)+1);
      strcpy(c->config_str_value,s);
      break;

   case CONFIG_INT :
      if (sscanf(s,"%i",&num) != 1)
	 return "invalid int";
      c->config_int_value = num;
      break;

   case CONFIG_BOOL :
      if (stricmp(s,"YES") != 0 && stricmp(s,"NO") != 0)
	 return "invalid bool";
      c->config_int_value = (stricmp(s,"YES") == 0);
      break;

   case CONFIG_STR :
      if (*s == '@')
      {
	 // If blakserv.cfg has line "Setting   <@setting.txt>",
	 // then Setting is set to the *contents* of the first line
	 // of the setting.txt file.  The config becomes non-dynamic,
	 // since SetConfigStr would need to either ignore settings.txt
	 // or write to it.

	 FILE* f = fopen(s+1, "rt");
	 if (f)
	 {
	    fgets(s, sizeof(s)-1, f);
	    strtok(s, "\r\n\x1A");
	    fclose(f);
	    c->is_dynamic = false;
	 }
      }
      c->config_str_value = (char *)AllocateMemory(MALLOC_ID_CONFIG,strlen(s)+1);
      strcpy(c->config_str_value,s);
      break;

   default :
      StartupPrintf("AddConfig can't handle type %i\n",config_type);
      break;
   }

   c->config_id = config_id;
   c->config_type = config_type;

   return NULL; /* no error string, everything is fine */
}

config_node * GetConfigByID(int config_id)
{
   if (config_id < 0 || config_id >= NUM_CONFIG_VALUES)
   {
      StartupPrintf("GetConfigByID got invalid request for id %i\n",config_id);
      return NULL;
   }

   if (configs[config_id].config_type == CONFIG_NONE)
      return NULL;
   else
      return &configs[config_id];
}

void LockDynamicConfig(void)
{
   MutexAcquire(mutex_dynamic_config);
}

void UnlockDynamicConfig(void)
{
   MutexRelease(mutex_dynamic_config);
}

void ForEachConfigNode(void (*callback_func)(config_node *c,const char *config_name,const char *default_str))
{
   int i,config_id;

   for (i=0;i<LEN_CONFIG_TABLE;i++)
   {
      config_id = config_table[i].config_id;
      callback_func(&configs[config_id],config_table[i].config_name,
		    config_table[i].default_str);
   }
}

char * ConfigStr(int config_id)
{
   config_node *c;

   /* this should be changed to NOT call startupprintf */

   c = GetConfigByID(config_id);
   if (c == NULL)
   {
      StartupPrintf("ConfigStr can't find id %i\n",config_id);
      return NULL;
   }
   if (c->config_type != CONFIG_STR && c->config_type != CONFIG_PATH)
   {
      StartupPrintf("ConfigStr found id %i is not str\n",config_id);
      return NULL;
   }

   if (c->is_dynamic)
   {
      StartupPrintf("ConfigStr found id %i is dynamic\n",config_id);
      return NULL;
   }

   return c->config_str_value;
}

int ConfigInt(int config_id)
{
   config_node *c;
   int ret_val;

   c = GetConfigByID(config_id);
   if (c == NULL)
   {
      StartupPrintf("ConfigInt can't find id %i\n",config_id);
      return 0;
   }
   if (c->config_type != CONFIG_INT)
   {
      StartupPrintf("ConfigInt found id %i is not int\n",config_id);
      return 0;
   }

   if (c->is_dynamic)
      LockDynamicConfig();

   ret_val = c->config_int_value;

   if (c->is_dynamic)
      UnlockDynamicConfig();

   return ret_val;
}

bool ConfigBool(int config_id)
{
   config_node *c;
   bool ret_val;

   c = GetConfigByID(config_id);
   if (c == NULL)
   {
      StartupPrintf("ConfigBool can't find id %i\n",config_id);
      return false;
   }
   if (c->config_type != CONFIG_BOOL)
   {
      StartupPrintf("ConfigBool found id %i is not bool\n",config_id);
      return false;
   }

   if (c->is_dynamic)
      LockDynamicConfig();

   ret_val = c->config_int_value;

   if (c->is_dynamic)
      UnlockDynamicConfig();

   return ret_val;
}

/* LockConfigStr
   This is for getting strings that are dynamic.  You must call UnlockConfigStr
   when you're done, unless we fail and return NULL */
char * LockConfigStr(int config_id)
{
   config_node *c;

   c = GetConfigByID(config_id);
   if (c == NULL)
   {
      eprintf("LockConfigStr can't find id %i\n",config_id);
      return NULL;
   }
   if (c->config_type != CONFIG_STR && c->config_type != CONFIG_PATH)
   {
      eprintf("LockConfigStr found id %i is not str\n",config_id);
      return NULL;
   }

   if (!c->is_dynamic)
   {
      //eprintf("LockConfigStr found id %i is not dynamic\n",config_id);
      //return NULL;
   }

   LockDynamicConfig();

   return c->config_str_value;
}

void UnlockConfigStr(void)
{
   UnlockDynamicConfig();
}

int GetConfigIDByGroupAndName(char *group,char *name)
{
   int i;
   bool in_group;

   in_group = false; /* if we've already found the correct group node */
   for (i=0;i<LEN_CONFIG_TABLE;i++)
   {
      if (config_table[i].config_type == CONFIG_GROUP)
      {
	 if (in_group)
	    break;

	 if (!stricmp(group,config_table[i].config_name))
	    in_group = true;
	 continue;
      }

      /* it's not a group, it's a real config node */

      if (in_group == false)
	 continue;

      if (!stricmp(name,config_table[i].config_name))
      {
	 return config_table[i].config_id;
      }
   }
   return INVALID_CONFIG;
}

void SetConfigInt(int config_id,int new_value)
{
   config_node *c;

   c = GetConfigByID(config_id);
   if (c == NULL)
   {
      eprintf("SetConfigInt can't find id %i\n",config_id);
      return;
   }
   if (c->config_type != CONFIG_INT)
   {
      eprintf("SetConfigInt found id %i is not int\n",config_id);
      return;
   }

   if (c->is_dynamic == false)
   {
      eprintf("SetConfigInt found id %i is not dynamic\n",config_id);
      return;
   }

   LockDynamicConfig();

   c->config_int_value = new_value;

   UnlockDynamicConfig();
}

void SetConfigBool(int config_id,bool new_value)
{
   config_node *c;

   c = GetConfigByID(config_id);
   if (c == NULL)
   {
      eprintf("SetConfigBool can't find id %i\n",config_id);
      return;
   }
   if (c->config_type != CONFIG_BOOL)
   {
      eprintf("SetConfigBool found id %i is not bool\n",config_id);
      return;
   }

   if (c->is_dynamic == false)
   {
      eprintf("SetConfigBool found id %i is not dynamic\n",config_id);
      return;
   }

   LockDynamicConfig();

   c->config_int_value = (int) new_value;

   UnlockDynamicConfig();
}

void SetConfigStr(int config_id,char *new_value)
{
   config_node *c;

   c = GetConfigByID(config_id);
   if (c == NULL)
   {
      eprintf("SetConfigStr can't find id %i\n",config_id);
      return;
   }
   if (c->config_type != CONFIG_STR)
   {
      eprintf("SetConfigStr found id %i is not str\n",config_id);
      return;
   }

   if (c->is_dynamic == false)
   {
      eprintf("SetConfigStr found id %i is not dynamic\n",config_id);
      return;
   }

   LockDynamicConfig();

   FreeMemory(MALLOC_ID_CONFIG,c->config_str_value,strlen(c->config_str_value)+1);
   c->config_str_value = (char *)AllocateMemory(MALLOC_ID_CONFIG,strlen(new_value)+1);
   strcpy(c->config_str_value,new_value);

   UnlockDynamicConfig();
}

void LoadConfig(void)
{
   FILE *configfile;
   char line[MAX_CONFIG_LINE+1];
   int lineno,current_group,i,config_id;

   if ((configfile = fopen(CONFIG_FILE,"rt")) == NULL)
      StartupPrintf("LoadConfig can't open %s, using default configuration\n",CONFIG_FILE);
   else
   {
      lineno = 1;
      current_group = -1;
      while (fgets(line,MAX_CONFIG_LINE,configfile))
      {
	 current_group = LoadConfigLine(line,lineno,CONFIG_FILE,current_group);
	 lineno++;
      }
      fclose(configfile);
   }

   for (i=0;i<LEN_CONFIG_TABLE;i++)
   {
      config_id = config_table[i].config_id;
      if (GetConfigByID(config_id) == NULL)
	 AddConfig(config_id,config_table[i].default_str,
		   config_table[i].config_type,config_table[i].is_dynamic);
   }
}

int LoadConfigLine(char *line,int lineno,const char *filename,int current_group)
{
   char *first_str,*t1;
   int i;

   first_str = strtok(line,"= \t\n");
   if (first_str == NULL)
      return current_group;

   if (*first_str == '#')
      return current_group;

   if (*first_str == '[')
   {
      if (strchr(first_str,']') == NULL)
      {
	 StartupPrintf("LoadConfigLine found invalid grouping (no ]) %s (%i)\n",
		filename,lineno);
	 return current_group;
      }
      t1 = strtok(NULL,"= \t\n");
      if (t1 != NULL)
      {
	 StartupPrintf("LoadConfigLine found extra stuff after grouping %s (%i)\n",
		filename,lineno);
	 return current_group;
      }

      for (i=0;i<LEN_CONFIG_TABLE;i++)
	 if (config_table[i].config_type == CONFIG_GROUP &&
	     !stricmp(first_str,config_table[i].config_name))
	    return i;

      StartupPrintf("LoadConfig found nonexistent group %s %s (%i)\n",
	     first_str,filename,lineno);

      return current_group;
   }


   if (current_group < 0 || current_group >= LEN_CONFIG_TABLE)
   {
      StartupPrintf("LoadConfig found data before a group declared %s (%i)\n",
	     filename,lineno);
      return current_group;
   }

   t1 = strtok(NULL,"\n");	/* rest of the line */
   if (t1 == NULL)
   {
      StartupPrintf("LoadConfig error %s (%i): no data value\n",filename,lineno);
      return current_group;
   }

   while (*t1 != 0 && (*t1 == ' ' || *t1 == '\t'))
      t1++;			/* skip whitespace */

   if (*t1 == '<' && *(t1+strlen(t1)-1) == '>')
   {
      *(t1+strlen(t1)-1) = 0;
      t1++;
   }
   else
      t1 = strtok(t1," \t\n");

   for (i=current_group+1;i<LEN_CONFIG_TABLE;i++)
   {
      if (config_table[i].config_type == CONFIG_GROUP)
	 break;

      if (!stricmp(first_str,config_table[i].config_name))
      {
         const char *s = AddConfig(config_table[i].config_id,t1,config_table[i].config_type,
		       config_table[i].is_dynamic);
	 if (s != NULL)
	    StartupPrintf("LoadConfig error %s (%i): `%s' is %s\n",
		   filename,lineno,t1,s);
	 break;
      }
   }

   if (i == LEN_CONFIG_TABLE || config_table[i].config_type == CONFIG_GROUP)
      StartupPrintf("LoadConfig can't match value %s (%i)\n",filename,lineno);

   return current_group;
}
