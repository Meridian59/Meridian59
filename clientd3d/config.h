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

#ifdef __cplusplus
extern "C" {
#endif

// Communication settings
typedef struct {
   WORD  timeout;                 /* # of seconds to wait before redialing */
   
   char  hostname[MAXHOST + 1];   /* Hostname of server for socket connections */
   int   sockport;                /* Port server listens on for socket connections */
   Bool  constant_port;           /* Is the port constant, or does it change per server? */
   int   server_num;              /* Server number to connect to (influences hostname) */
   char  domainformat[MAXHOST + 1];
} CommSettings;

// Structure to hold user configurations
typedef struct {
   Bool save_settings;           /* Save settings on exit? */
   Bool play_music;              /* Does user want to hear music? */
   Bool play_sound;              /* Does user want to hear sound? */
   Bool large_area;              /* Drawing area size--> 0 = small, nonzero = large */
   int  timeout;                 /* Period of logoff timer */
   char username[MAXUSERNAME+1]; /* User's last login name */
   char password[MAXPASSWORD+1]; /* User's last password (not saved to INI file) */

   CommSettings comm;            /* Communication settings */

   char browser[MAX_PATH + 1];   /* Full path to user's browser program */
   Bool default_browser;         /* True when browser location was retrieved from registry */

   Bool animate;                 /* Should we draw animations? */
   int  download_time;           /* Time of last successful download */
   Bool auto_connect;            /* Connect immediately upon starting program? */
   Bool debug;                   /* Display debugging window? */
   Bool security;                /* Use room security? */
   int  ini_version;             /* INI version number; restore defaults if it doesn't match */

   Bool draw_names;              /* Draw names over players? */
   Bool ignore_all;              /* Ignore EVERYTHING said? */
   Bool no_broadcast;            /* Ignore all broadcasts? */
   char ignore_list[MAX_IGNORE_LIST][MAX_CHARNAME + 1]; /* Usernames to ignore */

   Bool scroll_lock;             /* Don't scroll main edit box if scrolled back */
   Bool tooltips;                /* Display tooltips? */
   Bool inventory_num;           /* Display amounts for number items in inventory? */
   Bool aggressive;              /* Allowed to attack other players? */
   Bool tempsafe;                /* Do we get a temporary angel when we die? */
   Bool grouping;                /* Do we join building groups? */
   Bool autoloot;                /* Automatically loot stackable items? */
   Bool autocombine;             /* Autocombine spell items? */
   Bool reagentbag;              /* Automatically stash items into reagent bag? */
   Bool spellpower;              /* Display spellpower for spells we cast? */
   Bool bounce;                  /* Display player "bouncing" animation? */
   Bool toolbar;                 /* Display toolbar? */

   Bool pain;                    /* Display pain effect on hits? */
   Bool weather;                 /* Display weather effects? */
   Bool technical;               /* Show technical info such as the connected server number? */
   Bool quickstart;              /* Try to answer all questions with defaults until playing. */
   Bool antiprofane;             /* Kill annoying incoming profanity. */
   Bool guest;                   /* Automatically log in as "guest"? */
   int  server_low, server_high; /* Closed interval of legal server numbers for guest logins */
   int  halocolor;               // 0 = red, 1 = blue, 2 = green
   int  language;                // 0 = English, 1 = German, 2 = Korean

   Bool lagbox;                  /* Display lag meter? */
   Bool ignoreprofane;           /* Kill messages including any profanity. */
   Bool extraprofane;            /* Really search hard for possible hidden profanity. */
   int  server_guest;            /* Server to try first for guest logins */
   Bool play_loop_sounds;
   Bool play_random_sounds;
   Bool showMapBlocking;
   Bool showFPS;
   Bool showUnseenWalls;
   Bool showUnseenMonsters;
   Bool avoidDownloadAskDialog;
   int  maxFPS;		 /* Slow machine down for rendering to this frames per second */
   Bool drawmap;
   Bool clearCache;

   Bool colorcodes;
   int lastPasswordChange;

   int soundLibrary;
   Bool rosterbmps;         // unused, should be removed
   int CacheBalance;			 /* controls the balance between the object and grid caches */
   int ObjectCacheMin;			 /* minimum size of the object cache */
   int GridCacheMin;			 /* minimum size of the grid cache */

   // stuff for new client
   BOOL	bAlwaysRun;
   BOOL bAttackOnTarget;
   BOOL	bQuickChat;
   BOOL bInvertMouse;
   BOOL bDynamicLighting;
   int	mouselookXScale;
   int	mouselookYScale;

   Bool map_annotations;       /* Display annotations on map? */

   int sound_volume;           // 0 - 100
   int music_volume;           // 0 - 100
} Config;

void ConfigInit(void);
void ConfigOverride(LPCTSTR pszCmdLine);

void LoadSettings(void);
void SaveSettings(void);

void ConfigLoad(void);
void ConfigSave(void);
void ConfigMenuLaunch(void);

void WindowSettingsSave(void);
void WindowSettingsLoad(WINDOWPLACEMENT *w);

void TimeSettingsLoad(void);
void TimeSettingsSave(int download_time);

M59EXPORT int GetConfigInt(char *section, char *key, int default_value, char *fname);
BOOL WritePrivateProfileInt(char *section, char *key, int value, char *fname);
M59EXPORT BOOL WriteConfigInt(char *section, char *key, int value, char *fname);

void ConfigSetServerNameByNumber(int num);
void ConfigSetSocketPortByNumber(int num);

#define LIBRARY_NIL 0
#define LIBRARY_MSS 1
#define LIBRARY_MIX 2

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _CONFIG_H */
