// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * modules.c:  Handles add-on modules (DLLs).
 *
 * The first few functions deal with loading and initializing modules.  We keep a list
 * of all loaded modules in the module_list variable.
 *
 * Modules modify the behavior of the client by intercepting events, such as key presses
 * or messages arriving from the server.  When a module is loaded, it fills in a
 * structure that tells which events it wants to receive.
 *
 * The main function is ModuleEvent.  Various functions in the client call ModuleEvent
 * when an event happens.  They pass in the event type, and a variable list of arguments.
 * ModuleEvent looks through all the loaded modules, to see if any is interested in the
 * event.  If so, ModuleEvent calls a helper function that unpacks the variable
 * arguments, and passes them on to the event handler in the module.  The event handler's
 * return value indicates whether the event should be passed on to other modules.
 *
 * Since the modules are stored in decreasing order of priority, higher priority
 * modules get to process events first.  No order is defined among modules
 * with the same priority.
 *
 * Each module must have a GetModuleInfo function that is called just after the module is loaded.
 * This function fills in some fields in a structure.  The module may optionally have a
 * ModuleExit function that is called when the client leaves the game state, or just before
 * the client unloads the DLL.
 *
 * A module should respond to a call to its ModuleExit procedure by posting a BK_MODULEUNLOAD
 * message to the main client window, with its unique module id in lParam of the message.
 * This allows dialogs in the module to exit completely before the module is freed.
 *
 * Each module has a unique module ID number.  Modules with IDs of 100 or greater only
 * receive events if we are in STATE_GAME; this keeps game DLLs from getting spurious events after
 * the game has been exited.
 */

#include <crtdbg.h>

#include "client.h"

#define GAME_MODULE_MIN 100           // Modules with ID >= 100 don't get non-game messages

static list_type module_list = NULL;  /* List of ModuleInfo structs for all loaded modules,
				       * stored in decreasing order of priority */
static ClientInfo client_info;

static char module_dir[] = "resource\\";

static bool first_load;               // true when FIRST DLL is loaded

extern room_type   current_room;
extern HWND        hCurrentDlg;
extern list_type   current_users;
extern player_info player;
extern HPALETTE    hPal;
extern bool        map;
extern BYTE        light_palettes[NUM_PALETTES][NUM_COLORS];
extern DWORD       latency;

/* local function prototypes */
static int  CompareModulesByPriority(void *m1, void *m2);
static bool CompareModulesById(void *m1, void *m2);
static void ModuleExitByInfo(ModuleInfo *info);
/******************************************************************************/
/*
 * ModulesInit:  Initialize private data; called when we enter game mode.
 */
void ModulesInit(void)
{
   first_load = true;

   // Initialize client data sent to modules
   client_info.hMain         = hMain;
   client_info.config        = &config;
   client_info.current_room  = &current_room;
   client_info.player        = &player;
   client_info.hCurrentDlg   = &hCurrentDlg;
   client_info.hInst         = hInst;
   client_info.ini_file      = ini_file;
   client_info.current_users = &current_users;
   client_info.main_menu     = GetMenu(hMain);
   client_info.effects       = &effects;
   client_info.hPal          = hPal;
   client_info.hToolTips     = NULL;        // NULL until it's created
   client_info.szAppName     = szAppName;
   client_info.map           = &map;
   client_info.first_load    = &first_load;
   client_info.light_palettes = &light_palettes;
   client_info.latency       = &latency;
}
/******************************************************************************/
/*
 * ModulesClose:  Free private data; called when DLLs are to be unloaded.
 */
void ModulesClose(void)
{
   list_type l;
   ModuleInfo *info;

   for (l = module_list; l != NULL; l = l->next)
   {
      info = (ModuleInfo *) (l->data);
      FreeLibrary(info->handle);
   }
   module_list = list_destroy(module_list);
}
/******************************************************************************/
/*
 * ModulesEnterGame:  Set game data after STATE_GAME is entered.
 */
void ModulesEnterGame(void)
{
   client_info.hToolTips     = TooltipGetControl();
}
/******************************************************************************/
/*
 * CompareModulesByPriority:  Util procedure for sorting modules in order of priority.
 */
int CompareModulesByPriority(void *m1, void *m2)
{
   return ((ModuleInfo *) m1)->priority - ((ModuleInfo *) m2)->priority;
}
/******************************************************************************/
/*
 * CompareModulesById:  Return true iff modules have same module id.
 */
bool CompareModulesById(void *m1, void *m2)
{
   return ((ModuleInfo *) m1)->module_id == ((ModuleInfo *) m2)->module_id;
}
/******************************************************************************/
/*
 * ModuleLoadByRsc:  Load the module with filename given by the name_rsc resource.
 *   Return true iff successful.
 */
bool ModuleLoadByRsc(ID name_rsc)
{
  char *filename;
  char full_filename[MAX_PATH + FILENAME_MAX];
  char working_dir[MAX_PATH];
  ModuleInfo *info;
  list_type l;
  
  /* Make sure module not already loaded */
  for (l = module_list; l != NULL; l = l->next)
  {
    ModuleInfo *info = (ModuleInfo *) (l->data);
    if (info->rsc == name_rsc)
    {
      debug(("Module with resource #%d already loaded\n", name_rsc));
      return false;
    }
  }
  
  filename = LookupRsc(name_rsc);
  if (filename == NULL)
  {
    debug(("Couldn't find module filename for resource %d\n", name_rsc));
    return false;
  }
  
  // LoadLibrary can find libraries from places you don't want them.  We
  // specify the full path.
  GetWorkingDirectory(working_dir, MAX_PATH);
  snprintf(full_filename, sizeof(full_filename), "%s\\%s%s", working_dir, module_dir, filename);
  
  info = ModuleLoadByName(full_filename);
  if (info == NULL)
    return false;
  
  info->rsc = name_rsc;
  return true;
}
/******************************************************************************/
/*
 * ModuleLoadByName:  Load the module with the given filename.
 *   Returns pointer to new ModuleInfo if successful, or NULL on failure.
 */
ModuleInfo * ModuleLoadByName(char *filename)
{
   UINT old_mode;
   HINSTANCE hModule;
   ModuleProc lpfn;
   ModuleInfo *info;

   debug(("Loading DLL %s\n", filename));

   // Tell Windows not to display error message box if not found.
   old_mode = SetErrorMode(SEM_NOOPENFILEERRORBOX);

   hModule = LoadLibrary(filename);
   SetErrorMode(old_mode);

   if (hModule == NULL)
   {
      debug(("Unable to load DLL %s.  Reason = %d\n", filename, GetLastError()));
      return NULL;
   }

   lpfn = (ModuleProc) GetProcAddress(hModule, (LPCSTR) MAKELONG(MODULE_GETMODULEINFO, 0));
   if (lpfn == NULL)
   {
      debug(("GetProcAddress failed on DLL %s for function GetModuleInfo.\n", filename));
      return NULL;
   }

   info = (ModuleInfo *) SafeMalloc(sizeof(ModuleInfo));
   memset(info, 0, sizeof(ModuleInfo));  // Zero out in case module forgets to set something

   info->handle = hModule;
   (*lpfn)(info, &client_info);    // Ask module to fill in fields of info

   module_list = list_add_sorted_item(module_list, info, CompareModulesByPriority);

   debug(("Successfully loaded DLL %s, handle = %08x\n", filename, hModule));
   first_load = false;
   return info;
}
/******************************************************************************/
/*
 * ModuleExitByRsc:   Tell the module with the given resource filename to exit,
 *   if it is present.
 *   Returns true iff module is present.
 */
bool ModuleExitByRsc(ID name_rsc)
{
   list_type l;
   ModuleInfo *info;

   for (l = module_list; l != NULL; l = l->next)
   {
      info = (ModuleInfo *) (l->data);

      if (info->rsc == name_rsc)
      {
	 ModuleExitByInfo(info);
	 return true;
      }
   }
   return false;
}
/******************************************************************************/
/*
 * ModuleExitById:   Tell the module with the given module id to exit,
 *   if it is present.
 *   Returns true iff module is present.
 */
bool ModuleExitById(int module_id)
{
   list_type l;
   ModuleInfo *info;

   for (l = module_list; l != NULL; l = l->next)
   {
      info = (ModuleInfo *) (l->data);

      if (info->module_id == module_id)
      {
	 ModuleExitByInfo(info);
	 return true;
      }
   }
   return false;
}
/******************************************************************************/
/*
 * ModuleExitByInfo:   Tell the module with the given info structure to exit.
 */
void ModuleExitByInfo(ModuleInfo *info)
{
  ModuleExitProc lpfn;

  lpfn = (ModuleExitProc) GetProcAddress(info->handle, (LPCSTR) MAKELONG(MODULE_EXIT, 0));
  if (lpfn != NULL)
    (*lpfn)();
}
/******************************************************************************/
/*
 * ModuleUnloadById:   Unload the module with the given unique module id,
 *   if it is present.
 *   Returns true iff module is present.
 *
 *   Important:  Do not unload a module if it may have a dialog up.  The dialog
 *   procedure will be removed; however the dialog might still receive messages,
 *   causing a crash.  Instead, use ModuleExitByRsc.  A module with a dialog should
 *   post a message to the main window to remove itself after its dialogs are destroyed.
 */
bool ModuleUnloadById(int module_id)
{
   list_type l;
   ModuleInfo *info;

   for (l = module_list; l != NULL; l = l->next)
   {
      info = (ModuleInfo *) (l->data);

      if (info->module_id == module_id)
      {
	 debug(("Unloading module %d\n", module_id));
	 FreeLibrary(info->handle);
	 module_list = list_delete_item(module_list, info, CompareModulesById);
	 SafeFree(info);
	 return true;
      }
   }
   debug(("ModuleUnloadById couldn't find module %d to unload\n", module_id));
   return false;
}
/******************************************************************************/
/*
 * ModulesExitGame:  Tell each module that we're leaving game mode.  This is called
 *   before we leave the current game state to go into GAME_NONE.
 */
void ModulesExitGame(void)
{
   list_type l;
   ModuleInfo *info;
   ModuleExitProc lpfn;

   for (l = module_list; l != NULL; l = l->next)
   {
     info = (ModuleInfo *) (l->data);
     
     lpfn = (ModuleExitProc) GetProcAddress(info->handle, (LPCSTR) MAKELONG(MODULE_EXIT, 0));
     if (lpfn != NULL)
       (*lpfn)();
   }
}

/******************************************************************************/
typedef bool (WINAPI *EventHandlerProc)(...);
typedef bool (*EventProc)(ModuleInfo *info, EventHandlerProc handler_proc, va_list v_list);

typedef bool (WINAPI *EventHandlerServerMsgProc)(char *, DWORD);
typedef bool (WINAPI *EventHandlerKeyProc)(HWND, UINT, BOOL, int, UINT);
typedef bool (WINAPI *EventHandlerActionProc)(int, void *);
typedef bool (WINAPI *EventHandlerMouseClickProc)(HWND, BOOL, int, int, UINT);
typedef bool (WINAPI *EventHandlerFontChangedProc)(WORD, LOGFONT *);
typedef bool (WINAPI *EventHandlerColorChangedProc)(WORD, COLORREF);
typedef bool (WINAPI *EventHandlerMenuItemProc)(int);
typedef bool (WINAPI *EventHandlerStateChangedProc)(int, int);
typedef bool (WINAPI *EventHandlerWindowMsgProc)(HWND, UINT, WPARAM, LPARAM);
typedef bool (WINAPI *EventHandlerAnimateProc)(int);
typedef bool (WINAPI *EventHandlerToolbarButtonProc)(Button *);
typedef bool (WINAPI *EventHandlerTextCommandProc)(char *);
typedef bool (WINAPI *EventHandlerResizeProc)(int, int, AREA *);
typedef bool (WINAPI *EventHandlerRedrawProc)(HDC);
typedef bool (WINAPI *EventHandlerDrawItemProc)(HWND, DRAWITEMSTRUCT *);
typedef bool (WINAPI *EventHandlerInventoryProc)(int, void *);
typedef bool (WINAPI *EventHandlerSetCursorProc)(HCURSOR);

static bool ModuleEventServerMsg(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventKey(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventAction(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventMouseClick(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventFontChanged(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventColorChanged(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventMenuItem(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventModuleMsg(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventStateChanged(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventWindowMsg(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventAnimate(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventToolbarButton(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventTextCommand(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventResize(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventUserChanged(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventRedraw(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventDrawItem(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventResetData(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventInventory(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventSetCursor(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventNewRoom(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventSetFocus(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);
static bool ModuleEventConfigChanged(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list);


typedef struct {
   int       event_code;       // Bit that is set for this event
   int       ordinal;          // Ordinal # of event handler to call in module
   EventProc handler;          // Procedure to call to dispatch event
} event_entry, *EventTable;

static event_entry events[] = {
{ EVENT_SERVERMSG,    MODULE_SERVERMESSAGE,     ModuleEventServerMsg, },
{ EVENT_KEY,          MODULE_KEYPRESS,          ModuleEventKey, },
{ EVENT_USERACTION,   MODULE_USERACTION,        ModuleEventAction, },
{ EVENT_MOUSECLICK,   MODULE_MOUSECLICK,        ModuleEventMouseClick, },
{ EVENT_FONTCHANGED,  MODULE_FONTCHANGED,       ModuleEventFontChanged, },
{ EVENT_COLORCHANGED, MODULE_COLORCHANGED,      ModuleEventColorChanged, },
{ EVENT_MENUITEM,     MODULE_MENUITEM,          ModuleEventMenuItem, },
{ EVENT_MODULEMSG,    MODULE_MODULEMESSAGE,     ModuleEventModuleMsg, },
{ EVENT_STATECHANGED, MODULE_STATECHANGED,      ModuleEventStateChanged, },
{ EVENT_WINDOWMSG,    MODULE_WINDOWMSG,         ModuleEventWindowMsg, },
{ EVENT_ANIMATE,      MODULE_ANIMATE,           ModuleEventAnimate, },
{ EVENT_TOOLBUTTON,   MODULE_TOOLBUTTON,        ModuleEventToolbarButton, },
{ EVENT_TEXTCOMMAND,  MODULE_TEXTCOMMAND,       ModuleEventTextCommand, },
{ EVENT_RESIZE,       MODULE_RESIZE,            ModuleEventResize, },
{ EVENT_USERCHANGED,  MODULE_USERCHANGED,       ModuleEventUserChanged, },
{ EVENT_REDRAW,       MODULE_REDRAW,            ModuleEventRedraw, },
{ EVENT_DRAWITEM,     MODULE_DRAWITEM,          ModuleEventDrawItem, },
{ EVENT_RESETDATA,    MODULE_RESETDATA,         ModuleEventResetData, },
{ EVENT_INVENTORY,    MODULE_INVENTORY,         ModuleEventInventory, },
{ EVENT_SETCURSOR,    MODULE_SETCURSOR,         ModuleEventSetCursor, },
{ EVENT_NEWROOM,      MODULE_NEWROOM,           ModuleEventNewRoom, },
{ EVENT_SETFOCUS,     MODULE_SETFOCUS,          ModuleEventSetFocus, },
{ EVENT_CONFIGCHANGED,MODULE_CONFIGCHANGED,     ModuleEventConfigChanged, },
{ 0, 0, NULL},   // must end table this way
};

/******************************************************************************/
bool _cdecl ModuleEvent(int event, ...)
{
   list_type l;
   EventHandlerProc lpfn;
   bool retval;
   int index;
   va_list marker;

   for (index = 0; events[index].event_code != 0; index++)
   {
      if (events[index].event_code == event)
	 break;
   }

   if (events[index].event_code == 0 || events[index].handler == NULL)
   {
      debug(("ModuleEvent found unknown event type %d\n", event));
      return true;
   }

   // See if any modules want this message
   for (l = module_list; l != NULL; l = l->next)
   {
      ModuleInfo *info = (ModuleInfo *) (l->data);

      // Skip game modules if not in game mode
      if (info->module_id >= GAME_MODULE_MIN && state != STATE_GAME)
	 continue;

      if ((info->event_mask & event) != 0 && info->priority != 0)
      {
        lpfn = (EventHandlerProc) GetProcAddress(info->handle,
                                      (LPCSTR) MAKELONG(events[index].ordinal, 0));
	 if (lpfn != NULL)
	 {
	    va_start(marker, event);
	    retval = (*events[index].handler)(info, lpfn, marker);
	    va_end(marker);
	    if (retval == false)
	       return false;
	 }
	 else debug(("Module missing event handler for event %d\n", event));
      }
   }
   return true;
}

/* functions to unpack varargs list and pass on to module */
/******************************************************************************/
bool ModuleEventServerMsg(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   char *message = va_arg(v_list, char *);
   DWORD len     = va_arg(v_list, DWORD);

   return (*((EventHandlerServerMsgProc) module_proc))(message, len);
}
/******************************************************************************/
bool ModuleEventKey(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   HWND hwnd   = va_arg(v_list, HWND);
   UINT vk     = va_arg(v_list, UINT);
   BOOL fDown  = va_arg(v_list, BOOL);
   int cRepeat = va_arg(v_list, int);
   UINT flags  = va_arg(v_list, UINT);

   return (*((EventHandlerKeyProc) module_proc))(hwnd, vk, fDown, cRepeat, flags);
}
/******************************************************************************/
bool ModuleEventAction(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   int action = va_arg(v_list, int);
   void *data = va_arg(v_list, void *);

   return (*((EventHandlerActionProc) module_proc))(action, data);
}
/******************************************************************************/
bool ModuleEventMouseClick(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   HWND hwnd   = va_arg(v_list, HWND);
   BOOL fDoubleClick = va_arg(v_list, BOOL);
   int  x      = va_arg(v_list, int);
   int  y      = va_arg(v_list, int);
   UINT keyFlags = va_arg(v_list, UINT);

   return (*((EventHandlerMouseClickProc) module_proc))(hwnd, fDoubleClick, x, y, keyFlags);
}
/******************************************************************************/
bool ModuleEventFontChanged(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   WORD font_id  = va_arg(v_list, WORD);
   LOGFONT *font = va_arg(v_list, LOGFONT *);

   return (*((EventHandlerFontChangedProc) module_proc))(font_id, font);
}
/******************************************************************************/
bool ModuleEventColorChanged(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   WORD color_id  = va_arg(v_list, WORD);
   COLORREF color = va_arg(v_list, COLORREF);

   return (*((EventHandlerColorChangedProc) module_proc))(color_id, color);
}
/******************************************************************************/
bool ModuleEventMenuItem(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   int id  = va_arg(v_list, int);

   return (*((EventHandlerMenuItemProc) module_proc))(id);
}
/******************************************************************************/
bool ModuleEventModuleMsg(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   int module_id  = va_arg(v_list, int);
   va_list params = va_arg(v_list, va_list);

   // Only send message to target module
   if (module_id != info->module_id)
      return true;

   (*module_proc)(params);
   return false;
}
/******************************************************************************/
bool ModuleEventStateChanged(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   int old_state  = va_arg(v_list, int);
   int new_state  = va_arg(v_list, int);

   return (*((EventHandlerStateChangedProc) module_proc))(old_state, new_state);
}
/******************************************************************************/
bool ModuleEventWindowMsg(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   HWND   hwnd    = va_arg(v_list, HWND);
   UINT   message = va_arg(v_list, UINT);
   WPARAM wParam  = va_arg(v_list, WPARAM);
   LPARAM lParam  = va_arg(v_list, LPARAM);

   return (*((EventHandlerWindowMsgProc) module_proc))(hwnd, message, wParam, lParam);
}
/******************************************************************************/
bool ModuleEventAnimate(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   int dt  = va_arg(v_list, int);
   return (*((EventHandlerAnimateProc) module_proc))(dt);
}
/******************************************************************************/
bool ModuleEventToolbarButton(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   Button *b = va_arg(v_list, Button *);
   return (*((EventHandlerToolbarButtonProc) module_proc))(b);
}
/******************************************************************************/
bool ModuleEventTextCommand(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   char *str = va_arg(v_list, char *);
   return (*((EventHandlerTextCommandProc) module_proc))(str);
}
/******************************************************************************/
bool ModuleEventResize(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   int xsize  = va_arg(v_list, int);
   int ysize  = va_arg(v_list, int);
   AREA *view = va_arg(v_list, AREA *);

   return (*((EventHandlerResizeProc) module_proc))(xsize, ysize, view);
}
/******************************************************************************/
bool ModuleEventUserChanged(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   return (*module_proc)();
}
/******************************************************************************/
bool ModuleEventRedraw(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   HDC    hdc    = va_arg(v_list, HDC);

   return (*((EventHandlerRedrawProc) module_proc))(hdc);
}
/******************************************************************************/
bool ModuleEventDrawItem(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   HWND   hwnd    = va_arg(v_list, HWND);
   DRAWITEMSTRUCT *lpdis = va_arg(v_list, DRAWITEMSTRUCT *);
   return (*((EventHandlerDrawItemProc) module_proc))(hwnd, lpdis);
}
/******************************************************************************/
bool ModuleEventResetData(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   return (*module_proc)();
}
/******************************************************************************/
bool ModuleEventInventory(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   int command = va_arg(v_list, int);
   void *data  = va_arg(v_list, void *);

   return (*((EventHandlerInventoryProc) module_proc))(command, data);
}
/******************************************************************************/
bool ModuleEventSetCursor(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   HCURSOR cursor  = va_arg(v_list, HCURSOR);

   return (*((EventHandlerSetCursorProc) module_proc))(cursor);
}
/******************************************************************************/
bool ModuleEventNewRoom(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   return (*module_proc)();
}
/******************************************************************************/
bool ModuleEventSetFocus(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   return (*module_proc)();
}
/******************************************************************************/
bool ModuleEventConfigChanged(ModuleInfo *info, EventHandlerProc module_proc, va_list v_list)
{
   return (*module_proc)();
}
