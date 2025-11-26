// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * config.h:  Header file for config.c
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#define MAXPHONE 30   /* Max # of digits in a phone # */
#define MAXINITSTR 50 /* Max length of modem init string */

#define MAXHOST 50    /* Max length of hostname */
#define MAXPORT 6     /* Max # of digits in port number */

#define MAX_IGNORE_LIST 1000  /* Max number of player names that can be ignored */

#define CONFIG_MAX_VOLUME 100  // Max value of sound / music volume settings

// Communication settings
typedef struct {
   WORD  timeout;                 /* # of seconds to wait before redialing */
   
   char  hostname[MAXHOST + 1];   /* Hostname of server for socket connections */
   int   sockport;                /* Port server listens on for socket connections */
   bool  constant_port;           /* Is the port constant, or does it change per server? */
   int   server_num;              /* Server number to connect to (influences hostname) */
   char  domainformat[MAXHOST + 1];
} CommSettings;

// Structure to hold user configurations
typedef struct {
   bool save_settings;           /* Save settings on exit? */
   bool play_music;              /* Does user want to hear music? */
   bool play_sound;              /* Does user want to hear sound? */
   int  timeout;                 /* Period of logoff timer */
   char username[MAXUSERNAME+1]; /* User's last login name */
   char password[MAXPASSWORD+1]; /* User's last password (not saved to INI file) */

   CommSettings comm;            /* Communication settings */

   char browser[MAX_PATH + 1];   /* Full path to user's browser program */
   bool default_browser;         /* True when browser location was retrieved from registry */

   int  download_time;           /* Time of last successful download */
   bool auto_connect;            /* Connect immediately upon starting program? */
   bool debug;                   /* Display debugging window? */
   bool security;                /* Use room security? */
   int  ini_version;             /* INI version number; restore defaults if it doesn't match */

   bool draw_names;              /* Draw names over players? */
   bool ignore_all;              /* Ignore EVERYTHING said? */
   bool no_broadcast;            /* Ignore all broadcasts? */
   char ignore_list[MAX_IGNORE_LIST][MAX_CHARNAME + 1]; /* Usernames to ignore */

   bool scroll_lock;             /* Don't scroll main edit box if scrolled back */
   bool tooltips;                /* Display tooltips? */
   bool inventory_num;           /* Display amounts for number items in inventory? */
   bool aggressive;              /* Allowed to attack other players? */
   bool bounce;                  /* Display player "bouncing" animation? */
   bool toolbar;                 /* Display toolbar? */

   bool pain;                    /* Display pain effect on hits? */
   bool weather;                 /* Display weather effects? */
   bool technical;               /* Show technical info such as the connected server number? */
   bool quickstart;              /* Try to answer all questions with defaults until playing. */
   bool antiprofane;             /* Kill annoying incoming profanity. */
   int	halocolor;					//	0 = red, 1 = blue, 2 = green

   bool lagbox;                  /* Display lag meter? */
   bool spinning_cube;           /* Display the classic spinning latency meter */
   bool ignoreprofane;           /* Kill messages including any profanity. */
   bool extraprofane;            /* Really search hard for possible hidden profanity. */
   bool play_loop_sounds;
   bool play_random_sounds;
   bool showMapBlocking;
   bool showFPS;
   bool showUnseenWalls;
   bool showUnseenMonsters;
   bool avoidDownloadAskDialog;
   int  maxFPS;		 /* Slow machine down for rendering to this frames per second */
   bool drawmap;
   bool clearCache;

   bool colorcodes;
   int lastPasswordChange;

   int soundLibrary;
   bool rosterbmps;         // unused, should be removed
   int CacheBalance;			 /* controls the balance between the object and grid caches */
   int ObjectCacheMin;			 /* minimum size of the object cache */
   int GridCacheMin;			 /* minimum size of the grid cache */

   // stuff for new client
   bool	bAlwaysRun;
   bool bAttackOnTarget;
   bool	bQuickChat;
   bool bInvertMouse;
   bool bDynamicLighting;
   int	mouselookXScale;
   int	mouselookYScale;

   bool map_annotations;       /* Display annotations on map? */

   int sound_volume;           // 0 - 100
   int music_volume;           // 0 - 100
   int ambient_volume;         // 0 - 100 (ambient sounds are looping sounds)

   int text_area_size;        /* As a percentage of the client height */

   int active_stat_group; // Which stat group (inventory, skills, spells or stats) is visible.

   // Lowers graphics performance settings for optimized resource usage. 
   // Reduces max FPS to 60, lowers background texture resolution, 
   // and switches D3D present mode to D3DPRESENT_INTERVAL_DEFAULT 
   // for more stable, power-efficient rendering.
   bool gpuEfficiency;
   bool show_inventory_rarity;   /* Add inventory item rarity symbols? */
} Config;

void ConfigInit(void);
void ConfigOverride(LPCTSTR pszCmdLine);

void LoadSettings(void);
void SaveSettings(void);

void ConfigLoad(void);
void ConfigSave(void);

void WindowSettingsSave(void);
void WindowSettingsLoad(WINDOWPLACEMENT *w);

void TimeSettingsLoad(void);
void TimeSettingsSave(int download_time);

M59EXPORT int GetConfigInt(char *section, char *key, int default_value, char *fname);
bool WritePrivateProfileInt(char *section, char *key, int value, char *fname);
M59EXPORT bool WriteConfigInt(char *section, char *key, int value, char *fname);

void ConfigSetServerNameByNumber(int num);
void ConfigSetSocketPortByNumber(int num);

// Is this the Steam version of the client?  (e.g. this has its own update system)
bool IsSteamVersion();
  
#define LIBRARY_NIL 0
#define LIBRARY_MSS 1
#define LIBRARY_MIX 2

#endif /* #ifndef _CONFIG_H */
