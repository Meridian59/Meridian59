// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * config.c:  This file deals with user configuration of the game, such as 
 *   graphics and sound settings.  The settings are grouped together in a 
 *   global variable called "config".  The settings are all stored in a private
 *   INI file.
 * XXX Should check for errors in these routines
 */

#include "client.h"

#define MAX_INTSTR 12     // Maximum # of digits in an integer

/* Miscellaneous game settings */
Config config;
Bool	gLargeArea;
char inihost[MAXHOST];

// Full pathname of INI file
static char ini_filename[MAX_PATH + FILENAME_MAX];
char *ini_file;  // Pointer to ini_filename

// If version doesn't match that in INI file, restore default colors and fonts (used to change
// color and font settings in old clients).
#define INI_VERSION 3

/* INI file entries */
static char misc_section[]   = "Miscellaneous";  /* Section of INI file for config stuff */
static char INISaveOnExit[]  = "SaveOnExit";
static char INIPlayMusic[]   = "PlayMusic";
static char INIPlaySound[]   = "PlaySound";
static char INIPlayLoopSounds[]   = "PlayLoopSounds";
static char INIPlayRandomSounds[]   = "PlayRandomSounds";
static char INITimeout[]     = "Timeout";
static char INIUserName[]    = "UserName";
static char INIAnimate[]     = "Animate";
static char INIArea[]        = "Area";
static char INIDownload[]    = "Download";
static char INIBrowser[]     = "Browser";
static char INIDefaultBrowser[] = "DefaultBrowser";
static char INIVersion[]     = "INIVersion";
static char INIGuest[]       = "Guest";
static char INIServerLow[]   = "ServerLow";
static char INIServerHigh[]  = "ServerHigh";
static char INIServerGuest[] = "ServerGuest";
static char INILastPass[]    = "Sentinel";
static char INISoundLibrary[] = "SoundLibrary";
static char INICacheBalance[] = "CacheBalance";
static char INIObjectCacheMin[] = "ObjectCacheMin";
static char INIGridCacheMin[] = "GridCacheMin";
static char INIMusicVolume[]  = "MusicVolume";
static char INISoundVolume[]  = "SoundVolume";

static char interface_section[]= "Interface";
static char INIDrawMap[]     = "DrawMap";
static char INIScrollLock[]  = "ScrollLock";
static char INITooltips[]    = "Tooltips";
static char INIInventory[]   = "InventoryNum";
static char INIAggressive[]  = "Aggressive";
static char INITempSafe[]    = "TempSafe";
static char INIGrouping[]    = "Grouping";
static char INIAutoLoot[]    = "AutoLoot";
static char INIAutoCombine[] = "AutoCombine";
static char INIReagentBag[]  = "ReagentBag";
static char INISpellPower[]  = "SpellPower";
static char INIBounce[]      = "Bounce";
static char INIToolbar[]     = "Toolbar";
static char INIPainFX[]      = "Pain";
static char INIWeatherFX[]   = "Weather";
static char INIOldProfane[]  = "AntiProfane";
static char INIIgnoreProfane[]= "IgnoreProfane";
static char INIAntiProfane[] = "ProfanityFilter";
static char INIExtraProfane[]= "ExtraProfaneSearch";
static char INILagbox[]      = "LatencyMeter";
static char INIHaloColor[]   = "HaloColor";
static char INIColorCodes[]  = "ColorCodes";
static char INIMapAnnotations[] = "MapAnnotations";
static char INILanguage[]    = "Language";
static char window_section[] = "Window";         /* Section in INI file for window info */
static char INILeft[]        = "NormalLeft";
static char INIRight[]       = "NormalRight";
static char INITop[]         = "NormalTop";
static char INIBottom[]      = "NormalBottom";
static char INIShow[]        = "Show";
static char INIMaxX[]        = "MaxX";
static char INIMaxY[]        = "MaxY";

static char comm_section[]   = "Comm";  /* Section for comm stuff in INI file */
static char INIPort[]        = "Port";
static char INIRedialDelay[] = "RedialDelay";
static char INIHostname[]    = "Hostname";
static char INISockPort[]    = "SocketPort";
static char INIServerNum[]   = "ServerNumber";
static char INIDomainFormat[] = "Domain";

static char users_section[]  = "Users";  /* Section for dealing with other users */
static char INIDrawNames[]   = "DrawNames";
static char INIIgnoreAll[]   = "IgnoreAll";
static char ININoBroadcast[] = "NoBroadcast";
static char INIIgnoreList[]  = "IgnoreList";

static char special_section[] = "Special";  /* Section for hidden stuff in INI file */
static char INIDebug[]        = "Debug";
static char INISecurity[]     = "Security";
static char INITechnical[]    = "Technical";

static char INIShowFPS[] = "ShowFPS";
#ifndef NODPRINTFS
static char INIShowMapBlocking[]= "ShowMapBlocking";
static char INIShowUnseenWalls[] = "ShowUnseenWalls";
static char INIShowUnseenMonsters[] = "ShowUnseenMonsters";
static char INIAvoidDownloadAskDialog[] = "AvoidDownloadAskDialog";
static char INIMaxFPS[]	  = "MaxFPS";
static char INIClearCache[]	  = "ClearCache";
static char INIQuickStart[]   = "QuickStart";
#endif // NODPRINTFS

static int   DefaultRedialDelay   = 60;
static char  DefaultHostname[]    = "cheater";
static char  DefaultDomainFormat[] = "meridian%d.openmeridian.org"; // MUST have a %d in it somewhere.
static char  DefaultSockPortFormat[] = "59%.2d";
static int   DefaultServerNum     = 101;
static int   DefaultTimeout       = 1440; // 1 day in minutes (60*24)

/************************************************************************/
/* 
 * LoadSettings:  Load all user's settings from INI file.
 */
void LoadSettings(void)
{
   FontsCreate(False);
   ColorsCreate(False);
   CommLoadSettings();
   ConfigLoad();
   LoadProfaneTerms();

   // Restore defaults if they've changed from previous version
   if (config.ini_version != INI_VERSION)
   {
     ColorsRestoreDefaults();
     FontsRestoreDefaults();
     config.ini_version = INI_VERSION;
   }
}
/************************************************************************/
/*
 * SaveSettings:  Save all user's settings (fonts, etc.)
 */
void SaveSettings(void)
{
   WindowSettingsSave();
   FontsSave();
   ColorsSave();
   CommSaveSettings();
   ConfigSave();
   SaveProfaneTerms();
}

/****************************************************************************/
/*
 * ConfigInit:  Find name of INI file.  This function must be called before
 *   using the ini_file variable to read from or write to the INI file.
 */
void ConfigInit(void)
{
   char dir[MAX_PATH];

   //memset(&config,0,sizeof(config));
   GetGamePath( dir );
   sprintf(ini_filename, "%s%s", dir, GetString(hInst, IDS_INIFILE));
   ini_file = ini_filename;
}
/****************************************************************************/
/*
 * ConfigLoad:  Load user configuration from INI file.  If entries not present
 *   in INI file, assign defaults.
 */
void ConfigLoad(void)
{
   int index = 0;
   char *start, *ptr;
   char ignore_string[MAX_IGNORE_LIST * (MAX_CHARNAME + 1) + 1];
   
   config.save_settings = GetConfigInt(misc_section, INISaveOnExit, True, ini_file);
   config.play_music    = GetConfigInt(misc_section, INIPlayMusic, True, ini_file);
   config.play_sound    = GetConfigInt(misc_section, INIPlaySound, True, ini_file);
   config.music_volume    = GetConfigInt(misc_section, INIMusicVolume, 100, ini_file);
   config.sound_volume    = GetConfigInt(misc_section, INISoundVolume, 100, ini_file);
   config.play_loop_sounds    = GetConfigInt(misc_section, INIPlayLoopSounds, True, ini_file);
   config.play_random_sounds    = GetConfigInt(misc_section, INIPlayRandomSounds, True, ini_file);
   config.large_area    = GetConfigInt(misc_section, INIArea, True, ini_file);
   gLargeArea = config.large_area;
   // Animation option removed 3/4/97 to fix movement bug
#ifndef NODPRINTFS
   config.animate       = GetConfigInt(misc_section, INIAnimate, True, ini_file);
#else
   config.animate       = True;
#endif
   config.ini_version   = GetConfigInt(misc_section, INIVersion, 0, ini_file);
   config.default_browser = GetConfigInt(misc_section, INIDefaultBrowser, True, ini_file);
   GetPrivateProfileString(misc_section, INIUserName, "", 
			   config.username, MAXUSERNAME, ini_file); 
   GetPrivateProfileString(misc_section, INIBrowser, "", 
			   config.browser, MAX_PATH, ini_file); 
   
   config.draw_names   = GetConfigInt(users_section, INIDrawNames, True, ini_file);
   config.ignore_all   = GetConfigInt(users_section, INIIgnoreAll, False, ini_file);
   config.no_broadcast = GetConfigInt(users_section, ININoBroadcast, False, ini_file);

   GetPrivateProfileString(users_section, INIIgnoreList, "", 
                           ignore_string, sizeof(ignore_string), ini_file);
   memset(config.ignore_list, 0, sizeof(config.ignore_list));
   // Parse ignore string: usernames separated by commas
   start = ignore_string;
   ptr = start;
   while (index < MAX_IGNORE_LIST && *ptr != 0) {
     if (*ptr == ',') {
       *ptr = 0;
       strncpy(config.ignore_list[index], start, MAX_CHARNAME);
       start = ptr + 1;
       ++index;
     }
     ++ptr;
   }
   
   config.scroll_lock  = GetConfigInt(interface_section, INIScrollLock, False, ini_file);
   config.drawmap      = GetConfigInt(interface_section, INIDrawMap, True, ini_file);
   config.tooltips     = GetConfigInt(interface_section, INITooltips, True, ini_file);
   config.inventory_num= GetConfigInt(interface_section, INIInventory, True, ini_file);
   config.aggressive   = GetConfigInt(interface_section, INIAggressive, False, ini_file);
   config.tempsafe     = GetConfigInt(interface_section, INITempSafe, True, ini_file);
   config.grouping     = GetConfigInt(interface_section, INIGrouping, True, ini_file);
   config.autoloot     = GetConfigInt(interface_section, INIAutoLoot, True, ini_file);
   config.autocombine  = GetConfigInt(interface_section, INIAutoCombine, True, ini_file);
   config.reagentbag   = GetConfigInt(interface_section, INIReagentBag, True, ini_file);
   config.spellpower   = GetConfigInt(interface_section, INISpellPower, True, ini_file);
   config.bounce       = GetConfigInt(interface_section, INIBounce, True, ini_file);
   config.toolbar      = GetConfigInt(interface_section, INIToolbar, True, ini_file);
   config.pain         = GetConfigInt(interface_section, INIPainFX, True, ini_file);
   config.weather      = GetConfigInt(interface_section, INIWeatherFX, True, ini_file);
   config.antiprofane  = GetConfigInt(interface_section, INIAntiProfane, True, ini_file);
   config.ignoreprofane = GetConfigInt(interface_section, INIIgnoreProfane, False, ini_file);
   config.extraprofane = GetConfigInt(interface_section, INIExtraProfane, False, ini_file);
   config.lagbox       = GetConfigInt(interface_section, INILagbox, True, ini_file);
   config.halocolor    = GetConfigInt(interface_section, INIHaloColor, 0, ini_file);
   config.colorcodes   = GetConfigInt(interface_section, INIColorCodes, True, ini_file);
   config.map_annotations = GetConfigInt(interface_section, INIMapAnnotations, True, ini_file);
   config.language     = GetConfigInt(interface_section, INILanguage, 0, ini_file);
   config.guest        = GetConfigInt(misc_section, INIGuest, False, ini_file);
   config.server_low   = GetConfigInt(misc_section, INIServerLow, 0, ini_file);
   config.server_high  = GetConfigInt(misc_section, INIServerHigh, 0, ini_file);
   config.server_guest = GetConfigInt(misc_section, INIServerGuest, 0, ini_file);
   config.lastPasswordChange = GetConfigInt(misc_section, INILastPass, 0, ini_file);

   /* charlie: 
		This works like this , the balance is a % between 10%-90% which controls how much of the memory
		goes to which cache, so a value of 70% means that 70% of the cache memory goes to the 
		object cache, the remaining 30% to the grid cache anything over 90% is clamped, as is
		anything below 10%
	*/

   config.CacheBalance   = GetConfigInt(misc_section, INICacheBalance,        70, ini_file);
   config.ObjectCacheMin = GetConfigInt(misc_section, INIObjectCacheMin, 6000000, ini_file);
   config.GridCacheMin = GetConfigInt(misc_section, INIGridCacheMin,   4000000, ini_file);

   if( config.CacheBalance < 10 ) config.CacheBalance = 10 ;
   if( config.CacheBalance > 90 ) config.CacheBalance = 90 ;

   config.soundLibrary = GetConfigInt(misc_section, INISoundLibrary, LIBRARY_MSS, ini_file);

#ifdef NODPRINTFS
   config.debug    = False;
   config.security = True;
   config.showMapBlocking = FALSE;
   config.showUnseenWalls = FALSE;
   config.showUnseenMonsters = FALSE;
   config.avoidDownloadAskDialog = FALSE;
   config.maxFPS = FALSE;
   config.clearCache = FALSE;
#else
   config.debug				= GetConfigInt(special_section, INIDebug, False, ini_file);
   config.security			= GetConfigInt(special_section, INISecurity, True, ini_file);
   config.showMapBlocking	= GetConfigInt(special_section, INIShowMapBlocking, 0, ini_file);
   config.showUnseenWalls	= GetConfigInt(special_section, INIShowUnseenWalls, 0, ini_file);
   config.showUnseenMonsters = GetConfigInt(special_section, INIShowUnseenMonsters, 0, ini_file);
   config.avoidDownloadAskDialog = GetConfigInt(special_section, INIAvoidDownloadAskDialog, 0, ini_file);
   config.maxFPS			= GetConfigInt(special_section, INIMaxFPS, 70, ini_file);
   config.clearCache		= GetConfigInt(special_section, INIClearCache, False, ini_file);
   //config.quickstart = GetConfigInt(special_section, INIQuickStart, 0, ini_file);
#endif
   config.showFPS = GetConfigInt(special_section, INIShowFPS, False, ini_file);
   config.timeout	= GetConfigInt(misc_section, INITimeout, DefaultTimeout, ini_file);
   config.technical = GetConfigInt(special_section, INITechnical, False, ini_file);

   TimeSettingsLoad();
}
/****************************************************************************/
void ConfigSave(void)
{
  int i;
  char ignore_string[MAX_IGNORE_LIST * (MAX_CHARNAME + 1) + 1];
  
   WriteConfigInt(misc_section, INISaveOnExit, config.save_settings, ini_file);
   WriteConfigInt(misc_section, INIPlayMusic, config.play_music, ini_file);
   WriteConfigInt(misc_section, INIPlaySound, config.play_sound, ini_file);
   WriteConfigInt(misc_section, INIMusicVolume, config.music_volume, ini_file);
   WriteConfigInt(misc_section, INISoundVolume, config.sound_volume, ini_file);
   WriteConfigInt(misc_section, INIPlayLoopSounds, config.play_loop_sounds, ini_file);
   WriteConfigInt(misc_section, INIPlayRandomSounds, config.play_random_sounds, ini_file);
   WriteConfigInt(misc_section, INITimeout, config.timeout, ini_file);
   WriteConfigInt(misc_section, INIArea, gLargeArea, ini_file);
   WriteConfigInt(misc_section, INIAnimate, config.animate, ini_file);
   WriteConfigInt(misc_section, INIVersion, config.ini_version, ini_file);
   WriteConfigInt(misc_section, INIDefaultBrowser, config.default_browser, ini_file);
   WritePrivateProfileString(misc_section, INIBrowser, config.browser, ini_file);

   WriteConfigInt(misc_section, INICacheBalance, config.CacheBalance, ini_file);
   WriteConfigInt(misc_section, INIObjectCacheMin, config.ObjectCacheMin, ini_file);
   WriteConfigInt(misc_section, INIGridCacheMin, config.GridCacheMin, ini_file);

   WriteConfigInt(users_section, INIDrawNames, config.draw_names, ini_file);
   WriteConfigInt(users_section, INIIgnoreAll, config.ignore_all, ini_file);
   WriteConfigInt(users_section, ININoBroadcast, config.no_broadcast, ini_file);

   // Build up comma-separated list of character names
   ignore_string[0] = 0;
   for (i = 0; i < MAX_IGNORE_LIST; ++i) {
     if (config.ignore_list[i][0] != 0) {
       strcat(ignore_string, config.ignore_list[i]);
       strcat(ignore_string, ",");
     }
   }
   WritePrivateProfileString(users_section, INIIgnoreList, ignore_string, ini_file);

   WritePrivateProfileString(misc_section, INIUserName, config.username, ini_file);

   WriteConfigInt(interface_section, INIScrollLock, config.scroll_lock, ini_file);
   WriteConfigInt(interface_section, INITooltips, config.tooltips, ini_file);
   WriteConfigInt(interface_section, INIInventory, config.inventory_num, ini_file);
   WriteConfigInt(interface_section, INIAggressive, config.aggressive, ini_file);
   WriteConfigInt(interface_section, INITempSafe, config.tempsafe, ini_file);
   WriteConfigInt(interface_section, INIGrouping, config.grouping, ini_file);
   WriteConfigInt(interface_section, INIAutoLoot, config.autoloot, ini_file);
   WriteConfigInt(interface_section, INIAutoCombine, config.autocombine, ini_file);
   WriteConfigInt(interface_section, INIReagentBag, config.reagentbag, ini_file);
   WriteConfigInt(interface_section, INISpellPower, config.spellpower, ini_file);
   WriteConfigInt(interface_section, INIBounce, config.bounce, ini_file);
   WriteConfigInt(interface_section, INIToolbar, config.toolbar, ini_file);
   WriteConfigInt(interface_section, INIDrawMap, config.drawmap, ini_file);

   WriteConfigInt(interface_section, INIPainFX, config.pain, ini_file);
   WriteConfigInt(interface_section, INIWeatherFX, config.weather, ini_file);
   WriteConfigInt(interface_section, INIAntiProfane, config.antiprofane, ini_file);
   WriteConfigInt(interface_section, INIIgnoreProfane, config.ignoreprofane, ini_file);
   WriteConfigInt(interface_section, INIExtraProfane, config.extraprofane, ini_file);
   WriteConfigInt(interface_section, INILagbox, config.lagbox, ini_file);
   WriteConfigInt(interface_section, INIHaloColor, config.halocolor, ini_file);
   WriteConfigInt(interface_section, INIColorCodes, config.colorcodes, ini_file);
   WriteConfigInt(interface_section, INIMapAnnotations, config.map_annotations, ini_file);
   WriteConfigInt(interface_section, INILanguage, config.language, ini_file);

   // Don't write out "guest" option; user can't set it

   WriteConfigInt(misc_section, INIServerLow, config.server_low, ini_file);
   WriteConfigInt(misc_section, INIServerHigh, config.server_high, ini_file);
   WriteConfigInt(misc_section, INIServerGuest, config.server_guest, ini_file);
   WriteConfigInt(misc_section, INILastPass, config.lastPasswordChange, ini_file);
   WriteConfigInt(special_section, INIShowFPS, config.showFPS, ini_file);
   // "Special" section options NOT saved, so that they're not normally visible (except FPS)

   WritePrivateProfileString(interface_section, INIOldProfane, NULL, ini_file); // remove old string
}
/************************************************************************/
void ConfigOverride(LPCTSTR pszCmdLine)
{
   int argp;
   for (argp=1; argp < __argc; argp++)
   {
      char* p = __argv[argp];
      if (p && (*p == '-' || *p == '/'))
      {
	 char ch;
	 p++;
	 ch = *p++;
	 if (*p == ':')
	    p++;
	 switch (ch)
	 {
	 case 'h':
	 case 'H':
	    debug(("/H: \"%s\"\n", p));
	    strcpy(config.comm.hostname, p);
	    break;

	 case 'p':
	 case 'P':
	    debug(("/P: \"%s\"\n", p));
	    if (0 == atoi(p))
	    {
	       debug(("  ignoring invalid port number\n"));
	       break;
	    }
	    config.comm.sockport = atoi(p);
	    break;

	 case 'u':
	 case 'U':
	    debug(("/U: \"%s\"\n", p));
	    strcpy(config.username, p);
	    break;

	 case 'w':
	 case 'W':
	    debug(("/W: option got something\n"));
	    strcpy(config.password, p);
	    break;

	 case 'q':
	 case 'Q':
	    debug(("/Q: will try to start quick\n"));
	    config.quickstart = TRUE;
	    break;
	 }
      }
   }
}
/************************************************************************/
void WindowSettingsSave(void)
{
   RECT *r;
   WINDOWPLACEMENT w;
   
   w.length = sizeof(WINDOWPLACEMENT);
   GetWindowPlacement(hMain, &w);

   r = &w.rcNormalPosition;

   w.ptMaxPosition.x = min(w.ptMaxPosition.x, - GetSystemMetrics(SM_CXFRAME));
   w.ptMaxPosition.y = min(w.ptMaxPosition.y, - GetSystemMetrics(SM_CYFRAME));

   WriteConfigInt(window_section, INILeft, r->left, ini_file);
   WriteConfigInt(window_section, INIRight, r->right, ini_file);
   WriteConfigInt(window_section, INITop, r->top, ini_file);
   WriteConfigInt(window_section, INIBottom, r->bottom, ini_file);
   WriteConfigInt(window_section, INIMaxX, w.ptMaxPosition.x, ini_file);
   WriteConfigInt(window_section, INIMaxY, w.ptMaxPosition.y, ini_file);
   WriteConfigInt(window_section, INIShow, w.showCmd, ini_file);
}
/************************************************************************/
void WindowSettingsLoad(WINDOWPLACEMENT *w)
{
   RECT *r = &w->rcNormalPosition;
   int def_x, def_y, def_width, def_height;

   // When no INI file exists, we need to give a default position for the maximized window.
   // We need to place the window so that its border is off the screen; this is the
   // default Windows behavior.
   def_x = - GetSystemMetrics(SM_CXFRAME);
   def_y = - GetSystemMetrics(SM_CYFRAME);

   // Try to make client window optimally sized, but also fit to screen
   def_width  = min(MAIN_DEF_WIDTH, GetSystemMetrics(SM_CXSCREEN));
   def_height = min(MAIN_DEF_HEIGHT, GetSystemMetrics(SM_CYSCREEN));

   r->left   = GetConfigInt(window_section, INILeft, MAIN_DEF_LEFT, ini_file);
   r->right  = GetConfigInt(window_section, INIRight, MAIN_DEF_LEFT + def_width, ini_file);
   r->top    = GetConfigInt(window_section, INITop, MAIN_DEF_TOP, ini_file);
   r->bottom = GetConfigInt(window_section, INIBottom, MAIN_DEF_TOP + def_height, ini_file);
   w->ptMaxPosition.x = GetConfigInt(window_section, INIMaxX, def_x, ini_file);
   w->ptMaxPosition.y = GetConfigInt(window_section, INIMaxY, def_y, ini_file);
   w->showCmd = GetConfigInt(window_section, INIShow, SW_SHOWNORMAL, ini_file);
}

/********************************************************************/
/*
 * CommLoadSettings:  Load comm parameters from INI file.
 *  If use_defaults is False, try to load fonts from INI file.
 *  Otherwise use default comm settings.
 */
void CommLoadSettings(void)
{
   config.comm.timeout = GetConfigInt(comm_section, INIRedialDelay, DefaultRedialDelay, ini_file);

   GetPrivateProfileString(comm_section, INIHostname, DefaultHostname, 
			   inihost, MAXHOST, ini_file);
   GetPrivateProfileString(comm_section, INIDomainFormat, DefaultDomainFormat, 
			   config.comm.domainformat, MAXHOST, ini_file);
   if (!strstr(config.comm.domainformat, "%d"))
      strcpy(config.comm.domainformat, DefaultDomainFormat);
   config.comm.server_num = GetConfigInt(comm_section, INIServerNum, DefaultServerNum, ini_file);

   // We read the DomainFormat INI value (which we don't write into the INI)
   // and we set the hostname from that and the specified server number.
   //
   // A Meridian Service Provider other than NDS can supply another
   // value for DomainFormat in either the code (DefaultDomainFormat)
   // or by setting Domain=msp%d.foobar.co.sw in the INI.
   // We require there to be one %d in the format somewhere.
   //
   ConfigSetServerNameByNumber(config.comm.server_num);

   // What's the default port number we should connect to?  This varies by server number.
   config.comm.sockport = GetConfigInt(comm_section, INISockPort, -1, ini_file);

   if (config.comm.sockport == -1)
   {
      // We have no socket number set in the .ini file.
      config.comm.constant_port = False;
      ConfigSetSocketPortByNumber(config.comm.server_num);
   }
   else
   {
      config.comm.constant_port = True;
   }

}
/********************************************************************/
/*
 * CommSaveSettings:  Save comm settings to INI file.
 */
void CommSaveSettings(void)
{
   WriteConfigInt(comm_section, INIServerNum, config.comm.server_num, ini_file);
}
/********************************************************************/
/* 
 * TimeSettingsLoad:  Load last download time from INI file
 */
void TimeSettingsLoad(void)
{
   config.download_time = GetConfigInt(misc_section, INIDownload, 0, ini_file);
}

int GetTimeSettings(void)
{
 return GetConfigInt(misc_section, INIDownload, 0, ini_file);

}
/********************************************************************/
/* 
 * TimeSettingsSave:  Set last download time to given value and save
 *   to INI file.
 */
void TimeSettingsSave(int download_time)
{
   config.download_time = download_time;
   WriteConfigInt(misc_section, INIDownload, config.download_time, ini_file);
}

/* 32 bit replacements for 16-bit Windows functions */
/********************************************************************/
/*
 * GetConfigInt:  32-bit version of GetPrivateProfileInt
 */
int GetConfigInt(char *section, char *key, int default_value, char *fname)
{
   char buf[MAX_INTSTR], temp[MAX_INTSTR];

   sprintf(temp, "%d", default_value);

   if (GetPrivateProfileString(section, key, temp, buf, MAX_INTSTR, fname) == 0)
      return default_value;
   
   return atoi(buf);
}
/********************************************************************/
/*
 * WriteConfigInt:  32-bit version of WritePrivateProfileInt
 */
BOOL WriteConfigInt(char *section, char *key, int value, char *fname)
{
   char buf[MAX_INTSTR];

   sprintf(buf, "%d", value);
   return WritePrivateProfileString(section, key, buf, fname);
}

/********************************************************************/
/*
 * ConfigSetServerNameByNumber:  Set server name given server number.
 */
void ConfigSetServerNameByNumber(int num)
{
   sprintf(config.comm.hostname, config.comm.domainformat, num);
}

/********************************************************************/
/*
 * ConfigSetServerPortByNumber:  Set socketport from server number.
 */
void ConfigSetSocketPortByNumber(int num)
{
   char buf[MAX_INTSTR];

   // Do not alter it if we have a constant port set.
   if (config.comm.constant_port)
   {
      return;
   }

   /* We try to connect to the socket of the form 59##, where ## is
   ** the last two numbers of the server number.  So, if the user
   ** specifies server 103, it will try to connect to port 5903 on
   ** the specified server.  This allows us to run multiple server 
   ** instances on a single machine.
   **
   ** There should be no conflicts for most server numbers, since
   ** ports 5900 to 5967 are currently unassigned. If conflicts do
   ** appear, just change DefaultSockPortFormat.
   */

   sprintf(buf, DefaultSockPortFormat, (num % 100));

   config.comm.sockport = atoi(buf);
}

void ConfigMenuLaunch(void)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	char command_line[MAX_CMDLINE];

	sprintf(command_line, "%s", "m59bind.exe");

	memset(&si, sizeof(si), 0);
	si.cb = sizeof(si);
	GetStartupInfo(&si); /* shouldn't need to do this.  very weird */

	if (!CreateProcess(NULL, command_line,
		NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
	{
		debug(("Failed running configuration menu program %s\n", command_line));

		ClientError(hInst, hMain, IDS_NOCONFIGMENUEXE, config.browser);
	}
}
