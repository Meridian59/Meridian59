// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * loadrsc.c: Load data files.
 *
 * Resource files (.rsc extension) contain strings and filenames that are references by
 * number in messages from the server.  We load all of these into a hash table.
 *
 * Resource files can also have .rsb extensions; these are meant to be used as "base"
 * resources that are overridden by smaller .rsc files.  First, all .rsb files are loaded
 * in alphabetical order, and then all .rsc files are loaded.
 *
 */

#include <crtdbg.h>

#include "client.h"

#define TABLE_SIZE (30000-1)

static Table *t;      /* Hash table of all resources loaded in */

static Bool available_languages[MAX_LANGUAGE_ID]; // 1 if language present.

static char resource_dir[] = "resource";
static char room_dir[] = "resource";
static char rsb_spec[] = "*.rsb";
static char rsc_spec[] = "*.rsc";

static Bool ignore_duplicates;  // Don't complain about duplicate rscs when True

/* local function prototypes */
static DWORD ResourceHash(void *resource, DWORD tablesize);
static Bool  ResourceCompare(void *r1, void *r2);
static DWORD IdHash(void *idnum, DWORD tablesize);
static Bool  IdResourceCompare(void *idnum, void *r1);
static void  FreeRsc(void *entry);
static bool  RscAddCallback(char *fname, int res, int lang_id, char *string);
static Bool  LoadRscFiles(char *filespec);
static Bool  LoadRscFilesSorted(char *filespec);
/******************************************************************************/
/*
* GetString:  Load and return resource string with given resource identifier.
*             String is loaded from given module's resources.
* NOTE:  Only use 5 strings from this function at a time; it uses a circular
*        buffer of static strings!!
*/
char *GetString(HINSTANCE hModule, ID idnum)
{
	static int index = 0;
	static char szLoadedString[5][MAXSTRINGLEN];
	
	index = (index + 1) % 5;
	
	szLoadedString[index][0] = 0; 
	LoadString (hModule, idnum, szLoadedString[index], MAXSTRINGLEN);
	return (LPSTR)szLoadedString[index];
}
/******************************************************************************/
DWORD IdHash(void *idnum, DWORD tablesize)
{
	return *((ID *) idnum) % tablesize;
}
/******************************************************************************/
Bool IdResourceCompare(void *idnum, void *r1)
{
	return (Bool) (*((ID *) idnum) == ((resource_type) r1)->idnum);
}
/******************************************************************************/
DWORD ResourceHash(void *resource, DWORD tablesize)
{
	return (((resource_type) resource)->idnum) % tablesize;
}
/******************************************************************************/
Bool ResourceCompare(void *r1, void *r2)
{
	return (Bool) (((resource_type) r1)->idnum == ((resource_type) r2)->idnum);
}
/******************************************************************************/
/*
* CompareFilenames:  Util procedure for sorting rsc filenames.
*/
int CompareFilenames(void *f1, void *f2)
{
	return stricmp((char *) f1, (char *) f2);
}
/******************************************************************************/
/*
* LoadResources: Load all resources into a newly allocated table.
*   Return True on success, False iff no resource files found.
*/
Bool LoadResources(void)
{
	Bool rsc_loaded, rsb_loaded;
	
	/* Initialize new table */
	t = table_create(TABLE_SIZE);
	
	ignore_duplicates = True;
	// Load combined rscs, then normal rscs
	rsb_loaded = LoadRscFilesSorted(rsb_spec);
	rsc_loaded = LoadRscFiles(rsc_spec);
	
	// Built-in resources for About box
	RscAddCallback("", ABOUT_RSC, 0, "aarmor.bgf");
	RscAddCallback("", ABOUT_RSC1, 0,"c1.bgf");
	RscAddCallback("", ABOUT_RSC2, 0, "c2.bgf");
	RscAddCallback("", ABOUT_RSC3, 0, "c3.bgf");
	RscAddCallback("", LAGBOXICON_RSC, 0, "ilagbox.bgf");
	RscAddCallback("", LAGBOXNAME_RSC, 0, "Latency");
	
	ignore_duplicates = False;
	
	if (!rsb_loaded && !rsc_loaded)
	{
		debug(("Couldn't load any resources!\n"));
		return False;
	}
	
	return True;
}
/******************************************************************************/
/*
* LoadRscFiles:  Load all the resource files with the given filespec.
*   Returns True iff any was loaded.
*/
Bool LoadRscFiles(char *filespec)
{
	HANDLE hFindFile;
	WIN32_FIND_DATA file_info;
	char file_load_path[MAX_PATH + FILENAME_MAX], game_path[MAX_PATH];
	
	GetGamePath( game_path );
	sprintf(file_load_path,"%s%s\\%s", game_path, resource_dir, filespec);
	
	hFindFile = FindFirstFile(file_load_path, &file_info);
	if (hFindFile == INVALID_HANDLE_VALUE)
		return False;
	
	for(;;)
	{
		// Skip directories
		if (!(file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			/* Add subdirectory name, using file_load_path as temporary */
			sprintf(file_load_path, "%s%s\\%s", game_path, resource_dir, file_info.cFileName);  
			_RPT1(_CRT_WARN,"Loading File : %s\n",file_load_path); 
			if (!RscFileLoad((char *) file_load_path, RscAddCallback))
				debug(("Can't load resource file %s\n", file_info.cFileName));
		}
		
		if (FindNextFile(hFindFile, &file_info) == FALSE)
			break;
	}
	
	FindClose(hFindFile);
	
	return True;
}
/******************************************************************************/
/*
* LoadRscFilesSorted:  Load all the resource files with the given filespec, in 
*   sorted filename order.
*   Returns True iff any was loaded.
*/
Bool LoadRscFilesSorted(char *filespec)
{
	HANDLE hFindFile;
	WIN32_FIND_DATA file_info;
	char file_load_path[MAX_PATH + FILENAME_MAX], game_path[MAX_PATH];
	list_type filenames, l;
	
	GetGamePath( game_path );
	sprintf(file_load_path,"%s%s\\%s", game_path, resource_dir, filespec);
	
	hFindFile = FindFirstFile(file_load_path, &file_info);
	if (hFindFile == INVALID_HANDLE_VALUE)
		return False;
	
	filenames = NULL;
	for(;;)
	{
		// Skip directories
		if (!(file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			filenames = list_add_sorted_item(filenames, strdup(file_info.cFileName), CompareFilenames);
		
		if (FindNextFile(hFindFile, &file_info) == FALSE)
			break;
	}
	
	for (l = filenames; l != NULL; l = l->next)
	{
		/* Add subdirectory name, using file_load_path as temporary */
		sprintf(file_load_path, "%s%s\\%s", game_path, resource_dir, (char *) (l->data));  
		
		_RPT1(_CRT_WARN,"Loading File : %s\n",file_load_path); 
		
		if (!RscFileLoad((char *) file_load_path, RscAddCallback))
			debug(("Can't load resource file %s\n", file_info.cFileName));
	}
	
	FindClose(hFindFile);
	
	list_destroy(filenames);
	return True;
}
/******************************************************************************/
void ChangeResource(ID res, char *value)
{
   debug(("Got new resource %d, value = %s\n", res, value));
   RscAddCallback("", res, 0, value);
}
/******************************************************************************/
/*
* FreeResources:  Free resource table.
*/
void FreeResources(void)
{
	table_destroy(t, FreeRsc);
}
/******************************************************************************/
/*
* RscAddCallback:  Called for each new resource that's loaded from a file.
*   Add given resource to table.
*/
bool RscAddCallback(char *fname, int res, int lang_id, char *string)
{
   resource_type r;

   // Check for existing resource first, since we might want to add
   // another language string to it.
   r = (resource_type) table_lookup(t, &res, IdHash, IdResourceCompare);
   if (r == NULL)
   {
      r = (resource_type) SafeMalloc(sizeof(resource_struct));
      r->idnum = res;

      for (int i = 0; i < MAX_LANGUAGE_ID; i++)
         r->resource[i] = NULL;
   }
   else
      table_delete_item(t, r, ResourceHash, ResourceCompare);

   // Shouldn't be able to compile blakod with multiple strings
   // of the same language type.
   if (r->resource[lang_id])
      SafeFree(r->resource[lang_id]);
   r->resource[lang_id] = (char *) SafeMalloc(strlen(string) + 1);
   strcpy(r->resource[lang_id], string);

   available_languages[lang_id] = True;

   // Replace the existing resource if we used it.
   table_insert(t, r, ResourceHash, ResourceCompare);

   return true;
}
Bool *GetAvailableLanguages(void)
{
   return available_languages;
}
/******************************************************************************/
/*
* LookupRsc: Return the string associated with the given resource id #, or NULL
*   if it's not in the table.
*/
char *LookupRsc(ID idnum)
{
   resource_type r;
   r = (resource_type) table_lookup(t, &idnum, IdHash, IdResourceCompare);
   if (r == NULL)
   {
      debug(("Missing resource %d\n", idnum));
      /* Ask if user wants to redownload */
      // We post this message to the main window, which then calls MissingResource.
      // This avoids problems with the animation timer going off with the redownload
      // dialog going off, which causes a crash for an unknown reason.
      PostMessage(hMain, BK_NORESOURCE, 0, 0);
      return NULL;
   }

   // Default to English.
   if (r->resource[config.language] == NULL)
      return r->resource[0];

   return r->resource[config.language];
}
/******************************************************************************/
/*
* LookupNameRsc: Return the string associated with the given resource id #, or 
*   "<Unknown>" if it's not in the table.  Bring up redownload dialog if not in table.
*/
char *LookupNameRsc(ID idnum)
{
	char *name = LookupRsc(idnum); 
	if (name == NULL)
		name = GetString(hInst, IDS_UNKNOWN);
	return name;
}
/******************************************************************************/
/*
* LookupRscRedbook: Return the string associated with the given resource id #, or
*   "<Unknown>" if it's not in the table.
*/
char *LookupRscRedbook(ID idnum)
{
   resource_type r;
   r = (resource_type)table_lookup(t, &idnum, IdHash, IdResourceCompare);
   if (r == NULL)
   {
      debug(("Missing resource %d\n", idnum));
      return GetString(hInst, IDS_UNKNOWN);
   }
   return r->resource[0];
}
/******************************************************************************/
/*
* LookupRscNoError: Return the string associated with the given resource id #, or 
*   "<Unknown>" if it's not in the table.
*/
char *LookupRscNoError(ID idnum)
{
	resource_type r;
	r = (resource_type) table_lookup(t, &idnum, IdHash, IdResourceCompare);
	if (r == NULL)
	{
		debug(("Missing resource %d\n", idnum));
		return GetString(hInst, IDS_UNKNOWN);
	}
   // Default to English.
   if (r->resource[config.language] == NULL)
      return r->resource[0];

   return r->resource[config.language];
}
/******************************************************************************/
/* 
* FreeRsc:  Free a resource.  This is a callback procedure passed to table_delete.
*/
void FreeRsc(void *entry)
{
   resource_type typecast_entry = (resource_type) entry;
   for (int i = 0; i < MAX_LANGUAGE_ID; i++)
      if (typecast_entry->resource[i])
         SafeFree(typecast_entry->resource[i]);

   SafeFree(typecast_entry);
}
/******************************************************************************/
/*
* MissingResource:  What to call if we find that client is missing a resource 
*   from a server message.  If user wants to, quit game, automatically redownload
*   new resources, and reenter game.
*/
void MissingResource(void)
{
   /* Always display the box, to alert us something is missing.
	// In release version, we are not doing updates, so don't show dialog.
#ifdef NODPRINTFS
	return;
#else*/
	/* Maximum of one of these dialogs at a time */
	static Bool dialog_up = False;
	
	if (dialog_up)
		return;
	
	dialog_up = True;
	if (!AreYouSure(hInst, hMain, YES_BUTTON, IDS_MISSINGRESOURCE))
		return;
	
	RequestQuit();
	dialog_up = False;
//#endif
}
/******************************************************************************/
/*
* LoadRoomFile:  Load the room description file given by fname into the 
*   given room structure.  Returns True iff successful.
*/
Bool LoadRoomFile(char *fname, room_type *r)
{
	char filename[MAX_PATH + FILENAME_MAX], game_path[MAX_PATH];
	
	GetGamePath( game_path );
	/* Add directory to filename */
	sprintf(filename, "%s%s\\%.*s", game_path, room_dir, FILENAME_MAX, fname);
	
	return BSPRooFileLoad(filename, r);
}

/******************************************************************************/
/*
* DeleteRscFiles:  Delete given list of resource files
*/
void DeleteRscFiles(list_type files)
{
	list_type l;
	char filename[MAX_PATH + FILENAME_MAX], game_path[MAX_PATH];
	char *fname;
	struct stat s;
	
	GetGamePath( game_path );
	for (l = files; l != NULL; l = l->next)
	{
		fname = (char *) l->data;
		
		sprintf(filename, "%s%s\\%.*s", game_path, resource_dir, FILENAME_MAX, fname);
		
		/* If file doesn't exist, we're ok */
		if (stat(filename, &s) != 0)
		{
			debug(("Couldn't find file %s to delete\n", filename));
			continue;
		}
		
		if (unlink(filename) != 0)
			ClientError(hInst, hMain, IDS_CANTDELETE, filename);
	}
}
/******************************************************************************/
/*
* DeleteAllRscFiles:  Delete all resource files in resource directory.
*   Also resets last download time to 0.
*/
void DeleteAllRscFiles(void)
{
	HANDLE hFindFile;
	WIN32_FIND_DATA file_info;
	char path[MAX_PATH + FILENAME_MAX], game_path[MAX_PATH];
	
	GetGamePath( game_path );
	// Reset last download time to never
	DownloadSetTime(0);
	
	debug(("Deleting all resource files\n"));
	sprintf(path, "%s%s\\*.*", game_path, resource_dir);
	
	hFindFile = FindFirstFile(path, &file_info);
	if (hFindFile == INVALID_HANDLE_VALUE)
		return;
	
	for(;;)
	{
		// Skip directories
		if (!(file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			sprintf(path, "%s%s\\%s", game_path, resource_dir, file_info.cFileName);  
			
			if (unlink(path) != 0)
				ClientError(hInst, hMain, IDS_CANTDELETE, path);
		}
		
		if (FindNextFile(hFindFile, &file_info) == FALSE)
			break;
	}
	FindClose(hFindFile);
}
