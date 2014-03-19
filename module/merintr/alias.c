// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * alias.c:  Handle hotkey aliases.  These are stored in the INI file and modified
 *   via a dialog.
 *
 * The user can choose whether an alias is a self-contained command, or whether the alias
 * is just added to the text box, and the user can continue typing.  This option is saved
 * to the INI file by adding a ~ character to the end of alias strings that are just
 * added to the text box.
 */

#include "client.h"
#include "merintr.h"

#define MAX_VERBLEN  10     // Maximum length of a command verb to alias
#define MAX_ALIASLEN 100    // Maximum length of an alias command
#define NUM_ALIASES  12     // Number of aliases

typedef struct
{
   WORD  key;       // Key to assign alias to
   char  text[MAX_ALIASLEN + 1];      // Text command for alias
   Bool  cr;        // True iff alias is a self-contained command (add CR to end)
} HotkeyAlias;

typedef struct
{
   char  verb[MAX_VERBLEN + 1];      // Text command for varb to alias
   char  text[MAX_ALIASLEN + 1];      // Text command for alias
} VerbAlias;

static HotkeyAlias aliases[] =
{
   { VK_F1,   "help",     True, },
   { VK_F2,   "rest",     True, },
   { VK_F3,   "stand",    True, },
   { VK_F4,   "neutral",  True, },
   { VK_F5,   "happy",    True, },
   { VK_F6,   "sad",      True, },
   { VK_F7,   "wry",      True, },
   { VK_F8,   "wave",     True, },
   { VK_F9,   "point",    True, },
   { VK_F10,  "addgroup", True, },
   { VK_F11,  "mail",     True, },
   { VK_F12,  "quit",     True, },
};

static VerbAlias* _apVerbAliases = NULL;
static int _nVerbAliases = 0;
static int _nAllocated = 0;
#define CHUNKSIZE 10

static char _szDefaultVerbAliases[] =
   "chuckle=emote chuckles.\0"
   "giggle=emote giggles.\0"
   "hail=tellguild Hail Guildmembers!\0"
   "laugh=emote laughs.\0"
   "laughat=emote laughs at ~~!\0"
   "smile=emote smiles.\0"
   "smileat=emote smiles at ~~.\0"
   "wink=emote winks.\0"
   "winkat=emote winks at ~~.\0"
   "\0";

static char alias_section[] = "Aliases";   // Section for aliases in INI file
static char command_section[] = "CommandAliases";   // Section for aliases in INI file

extern keymap interface_key_table[];
extern keymap inventory_key_table[];
extern keymap	gCustomKeys[];

static HWND hAliasDialog = NULL;
static HWND hAliasDialog1 = NULL;
static HWND hAliasDialog2 = NULL;

static BOOL CALLBACK AliasDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
static BOOL CALLBACK VerbAliasDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

extern Bool	gbClassicKeys;
extern player_info *GetPlayer(void);

/****************************************************************************/
/*
 * AliasInit:  Read aliases from INI file and set them up.
 */
void AliasInit(void)
{
   int i, len;
   char temp[10];
   char	fullSection[255];
   char	destName[128];
   char	*srcName;
   WORD command;
   player_info	*playerInfo;

   destName[0] = '\0';

   playerInfo = GetPlayerInfo();
   
   // This was incorrectly being called before the player's resource was known,
   // giving an unknown resource dialog box every time the game was entered.
   // Unfortunately, players' aliases were then written to disk under with
   // the default "<Unknown>" player name encoded in the INI preferences file.
   // In order not to lose everyone's saved aliases, we have to emulate that
   // old behavior.
   if (cinfo->player->name_res == 0)
      srcName = "<Unknown>";
   else
      srcName = LookupNameRsc(cinfo->player->name_res);
   len = strlen(srcName);

   for (i = 0; i < len; i++)
   {
	   itoa(srcName[i], temp, 10);
	   strcat(destName, temp);
   }

   strcpy(fullSection, alias_section);
   strcat(fullSection, destName);

   // Hot-Key Aliases
   for (i = 0; i < NUM_ALIASES; i++)
   {
      // Read function key aliases
      sprintf(temp, "F%d", i + 1);
      //GetPrivateProfileString(alias_section, temp, aliases[i].text,
	  GetPrivateProfileString(fullSection, temp, aliases[i].text,
      aliases[i].text, MAX_ALIASLEN, cinfo->ini_file);

      // Check for CR
      len = strlen(aliases[i].text);
      if (len > 0 && aliases[i].text[len - 1] == '~')
      {
	 command = A_TEXTINSERT;
	 aliases[i].text[len - 1] = 0;
	 aliases[i].cr = False;
      }
      else 
      {
	 command = A_TEXTCOMMAND;
	 aliases[i].cr = True;
      }

	  if (gbClassicKeys)
		AliasSetKey(interface_key_table, aliases[i].key, KEY_NONE, command, aliases[i].text);
	  else
		AliasSetKey(gCustomKeys, aliases[i].key, KEY_NONE, command, aliases[i].text);

      AliasSetKey(inventory_key_table, aliases[i].key, KEY_NONE, command, aliases[i].text);
   }
}

void CmdAliasInit(void)
{
	// Command Aliases
    char* pVerb;
    char* pCommand;
    int nAllocated = 1024;
    char* pSection = (char *) SafeMalloc(nAllocated);
    
    while (pSection)
    {
       int nReturned;
       
       // Fills buffer with "verb=command\0verb=command\0\0".
       nReturned = GetPrivateProfileSection(
          command_section,
          pSection, nAllocated,
          cinfo->ini_file);
       
       // Indicates buffer was big enough.
       if (nReturned && nReturned < nAllocated-2)
          break;
       
       SafeFree(pSection);
       pSection = NULL;
       
       // Buffer needs to grow.
       if (nReturned)
       {
          nAllocated = nAllocated * 4 / 3;
          pSection = (char *) SafeMalloc(nAllocated);
       }
    }
    
    pVerb = pSection;
    if (!pSection || !pSection[0])
       pVerb = _szDefaultVerbAliases;
    
    while (*pVerb)
    {
       pCommand = strtok(pVerb, "=");
       pCommand = strtok(NULL, "");
       AddVerbAlias(pVerb, pCommand);
       pVerb = pCommand+strlen(pCommand)+1;
    }
    
    if (pSection)
       SafeFree(pSection);
}
/****************************************************************************/
/*
 * AliasExit:  Clean up alias dialog(s).
 */
void AliasExit(void)
{
   if (hAliasDialog != NULL)
      SendMessage(hAliasDialog, WM_COMMAND, IDCANCEL, 0);
   if (hAliasDialog1 != NULL)
      SendMessage(hAliasDialog1, WM_COMMAND, IDCANCEL, 0);
   if (hAliasDialog2 != NULL)
      SendMessage(hAliasDialog2, WM_COMMAND, IDCANCEL, 0);
}
/****************************************************************************/
/*
 * AliasSave:  Save aliases to INI file.
 */
void AliasSave(void)
{
   int i, len;
   char temp[10], text[MAX_ALIASLEN + 1];
   char	fullSection[255];
   char	destName[128];
   char	*srcName;
   player_info	*playerInfo;

   destName[0] = '\0';

   playerInfo = GetPlayerInfo();
   srcName = LookupNameRsc(cinfo->player->name_res);
   len = strlen(srcName);

   for (i = 0; i < len; i++)
   {
	   itoa(srcName[i], temp, 10);
	   strcat(destName, temp);
   }

   strcpy(fullSection, alias_section);
   strcat(fullSection, destName);

   // Save the hotkey aliases.
   for (i=0; i < NUM_ALIASES; i++)
   {
      // Save function key aliases
      sprintf(temp, "F%d", i + 1);

      if (aliases[i].cr)
	 strcpy(text, aliases[i].text);
      else
	 sprintf(text, "%s~", aliases[i].text);

      WritePrivateProfileString(fullSection, temp, text, cinfo->ini_file);
   }

   strcpy(fullSection, command_section);
   strcat(fullSection, destName);

   // Save the command aliases.
   if (_nVerbAliases)
   {
      WritePrivateProfileSection(command_section, "\0\0\0", cinfo->ini_file);
      for (i = 0; i < _nVerbAliases; i++)
      {
	 if (_apVerbAliases[i].verb[0])
	 {
	    WritePrivateProfileString(command_section,
	       _apVerbAliases[i].verb,
	       _apVerbAliases[i].text,
	       cinfo->ini_file);
	 }
      }
   }
}
/****************************************************************************/
/*
 * AliasSetKey:  Set up given alias to given virtual key code in given key table.
 */
void AliasSetKey(KeyTable table, WORD vk_key, WORD flags, WORD command, void *data)
{
   int index;
   WORD table_flags;

   for (index = 0; table[index].vk_code != 0; index++)
   {
      if (table[index].vk_code != vk_key)
	 continue;

      table_flags = table[index].flags;
      
      if (table_flags == KEY_ANY || (table_flags & flags))
      {
	 table[index].command = command;
	 table[index].data    = data;
      }
   }
}
/****************************************************************************/
/*
 * RemoveVerbAlias.
 */
void RemoveVerbAlias(char* pszVerb)
{
   int i;

   for (i = 0; i < _nVerbAliases; i++)
   {
      if (_apVerbAliases[i].verb[0])
      {
	 if (0 == stricmp(pszVerb, _apVerbAliases[i].verb))
	 {
	    _apVerbAliases[i].verb[0] = '\0';
	 }
      }
   }
}
/****************************************************************************/
/*
 * FreeVerbAliases:  Free all command alias memory.
 */
void FreeVerbAliases()
{
   if (_apVerbAliases)
   {
      SafeFree(_apVerbAliases);
      _apVerbAliases = NULL;
      _nVerbAliases = 0;
      _nAllocated = 0;
   }
}
/****************************************************************************/
/*
 * AddVerbAlias:  Set up given alias to given command verb.
 */
BOOL AddVerbAlias(char* pszVerb, char* pszCommand)
{
   VerbAlias* pAlias = NULL;
   int i;

   if (!_apVerbAliases)
   {
      _nVerbAliases = 0;
      _nAllocated = CHUNKSIZE;
      _apVerbAliases = (VerbAlias *) SafeMalloc(_nAllocated * sizeof(VerbAlias));
      memset(_apVerbAliases, 0, _nAllocated * sizeof(VerbAlias));
   }

   if (!pszVerb || !*pszVerb)
      return FALSE;

   // Unique verbs only.
   RemoveVerbAlias(pszVerb);

   // Find a free structure if there is one.
   //
   pAlias = NULL;
   for (i = 0; i < _nVerbAliases; i++)
   {
      if (!_apVerbAliases[i].verb[0])
      {
	 pAlias = &_apVerbAliases[i];
	 break;
      }
   }
   if (!pAlias)
   {
      if (_nVerbAliases < _nAllocated)
      {
	 pAlias = &_apVerbAliases[_nVerbAliases];
	 _nVerbAliases++;
      }
   }

   // SafeReallocate another CHUNKSIZE verb structures if necessary.
   //
   if (!pAlias)
   {
      pAlias = (VerbAlias *)
         SafeRealloc(_apVerbAliases, (_nAllocated+CHUNKSIZE)*sizeof(VerbAlias));
      if (pAlias)
      {
         _apVerbAliases = pAlias;
         
         for (i = _nAllocated; i < _nAllocated+CHUNKSIZE; i++)
            memset(&_apVerbAliases[i], 0, sizeof(*_apVerbAliases));
         pAlias = _apVerbAliases + _nVerbAliases;
         
         _nAllocated += CHUNKSIZE;
         _nVerbAliases++;
      }
   }
   if (!pAlias)
      return FALSE;
   
   // Prepare the verb/pattern structure.
   //
   strncpy(pAlias->verb, pszVerb, MAX_VERBLEN);
   pAlias->verb[MAX_VERBLEN] = '\0';

   strncpy(pAlias->text, pszCommand, MAX_ALIASLEN);
   pAlias->text[MAX_ALIASLEN] = '\0';

   return TRUE;
}
/****************************************************************************/
/*
 * ParseVerbAlias:  User typed this.  Reissue a command if it was an alias.
 */
BOOL ParseVerbAlias(char* pInput)
{
   char accum[MAXSAY+1];
   char* pVerb;
   char* pArgs;
   int iMatch = -1;
   BOOL bTie = FALSE;
   BOOL bInvalid;
   int i;

   if (!pInput || !*pInput)
      return FALSE;

   pInput = strdup(pInput);
   if (!pInput)
      return FALSE;

   pVerb = pInput;
   while (*pVerb == ' ' || *pVerb == '\t')
      pVerb++;

   pArgs = strtok(pVerb, " \t\r\n");
   pArgs = strtok(NULL, "\r\n");
   if (!pArgs)
      pArgs = "";

   for (i = 0; i < _nVerbAliases; i++)
   {
      // if defined verb begins with typed verb,
      if (_apVerbAliases[i].verb[0])
      {
	 if (0 == strnicmp(_apVerbAliases[i].verb, pVerb, strlen(pVerb)))
	 {
	    if (iMatch >= 0)
	       bTie = TRUE;
	    iMatch = i;

	    if (0 == stricmp(_apVerbAliases[i].verb, pVerb))
	    {
	       bTie = FALSE;
	       break;
	    }
	 }
      }
   }

   if (iMatch < 0)
      return FALSE;

   if (bTie)
   {
      GameMessage(GetString(hInst, IDS_AMBIGUOUSALIAS));
      return TRUE;
   }

   // Find any ~~ in .text, replace it with pArgs.
   //
   pVerb = _apVerbAliases[iMatch].text;
   strcpy(accum, pVerb);
   {
      char* pBefore = pVerb;
      char* pAfter = strstr(pVerb, "~~");

      if (pAfter && (strlen(pVerb)+strlen(pArgs) < MAXSAY))
      {
	 strcpy(accum+(pAfter-pBefore), pArgs);
	 strcat(accum, pAfter+2);
	 pVerb = accum;
      }
   }

   bInvalid = ModuleEvent(EVENT_TEXTCOMMAND, accum);
   if (bInvalid)
      GameMessage(GetString(hInst, IDS_INVALIDALIAS));

   // Return TRUE if command found and handled.
   return TRUE;
}
/************************************************************************/
/*
 * VerbAlias: "alias" command
 */
void CommandAliasCommon(char *args, DLGPROC pfnAliasDialogProc, int idDialog)
{
   if (args && *args)
   {
      while (*args == ' ' || *args == '\t')
	 args++;
   }

   // Arguments given.  Try to define or remove a command alias.
   //
   if (args && *args)
   {
      char* pszVerb = args;
      char* pszCommand;
      pszCommand = strtok(args, " \t=\r\n");
      pszCommand = strtok(NULL, "\r\n");

      if (pszCommand && *pszCommand)
      {
	 AddVerbAlias(pszVerb, pszCommand);
	 GameMessage(GetString(hInst, IDS_CMDALIASDEFINED));
      }
      else
      {
	 RemoveVerbAlias(pszVerb);
	 GameMessage(GetString(hInst, IDS_CMDALIASREMOVED));
      }

      AliasSave();

      return;
   }

   // No arguments.  Do the default (hotkey) dialog.
   //
   if (IDOK == DialogBox(hInst, MAKEINTRESOURCE(idDialog),
                         cinfo->hMain, pfnAliasDialogProc))
   {
      AliasSave();
   }
}
void CommandVerbAlias(char *args)
{
   CommandAliasCommon(args, VerbAliasDialogProc, IDD_CMDALIAS);
}
void CommandAlias(char *args)
{
   CommandAliasCommon(args, AliasDialogProc, IDD_ALIAS);
}
/*****************************************************************************/
/*
 * AliasDialogProc:  Dialog procedure for alias dialog.
 */
BOOL CALLBACK AliasDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   int i;
   HWND hEdit;
   WORD command;

   switch (message)
   {
   case WM_INITDIALOG:
      // Show current aliases
      for (i=0; i < NUM_ALIASES; i++)
      {
	 hEdit = GetDlgItem(hDlg, IDC_ALIASF1 + i);
	 SetWindowText(hEdit, aliases[i].text);
	 Edit_LimitText(hEdit, MAX_ALIASLEN - 1);
	 SetWindowFont(hEdit, GetFont(FONT_LIST), FALSE);

	 CheckDlgButton(hDlg, IDC_CHECK_F1 + i, aliases[i].cr);
      }

      hAliasDialog = hDlg;
      hAliasDialog1 = hDlg;
      if (hAliasDialog2)
	 DestroyWindow(GetDlgItem(hDlg, IDC_TUNNEL));

      CenterWindow(hDlg, GetParent(hDlg));
      return TRUE;

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDOK:
	 // Read aliases
	 for (i=0; i < NUM_ALIASES; i++)
	 {
	    GetDlgItemText(hDlg, IDC_ALIASF1 + i, aliases[i].text, MAX_ALIASLEN - 1);
	    aliases[i].cr = IsDlgButtonChecked(hDlg, IDC_CHECK_F1 + i);
	    if (aliases[i].cr)
	       command = A_TEXTCOMMAND;
	    else command = A_TEXTINSERT;
	    AliasSetKey(interface_key_table, aliases[i].key, KEY_NONE, command, aliases[i].text);
	    AliasSetKey(inventory_key_table, aliases[i].key, KEY_NONE, command, aliases[i].text);
	 }

	 EndDialog(hDlg, IDOK);
	 return TRUE;

      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;

      case IDC_TUNNEL:
	 if (!hAliasDialog2 &&
	     IDOK == DialogBox(hInst, MAKEINTRESOURCE(IDD_CMDALIAS), hDlg, VerbAliasDialogProc))
	 {
	    AliasSave();
	    SetFocus(GetDlgItem(hDlg, IDOK));
	 }
      }
      break;

   case WM_DESTROY:
      hAliasDialog = NULL;
      hAliasDialog1 = NULL;
      return TRUE;
   }
   
   return FALSE;
}
/*****************************************************************************/
void ListView_ClearSelection(HWND hList)
{
   int nItems, i;

   nItems = ListView_GetItemCount(hList);
   for (i = 0; i < nItems; i++)
      ListView_SetItemState(hList, i, 0, LVIS_SELECTED);
}
void ListView_SetSelection(HWND hList, int index)
{
   int nItems, i;

   nItems = ListView_GetItemCount(hList);
   for (i = 0; i < nItems; i++)
      ListView_SetItemState(hList, i, (i == index)? LVIS_SELECTED : 0, LVIS_SELECTED);
}
int ListView_GetSelection(HWND hList)
{
   int nItems, i;

   nItems = ListView_GetItemCount(hList);
   for (i = 0; i < nItems; i++)
      if (ListView_GetItemState(hList, i, LVIS_SELECTED))
	 return i;

   return -1;
}
/*****************************************************************************/
/*
 * VerbAliasDialogProc:  Dialog procedure for command verb alias dialog.
 */
BOOL CALLBACK VerbAliasDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
   switch (message)
   {
   case WM_INITDIALOG:

      SendDlgItemMessage(hDlg, IDC_VERB, EM_LIMITTEXT, MAX_VERBLEN, 0L);
      SendDlgItemMessage(hDlg, IDC_EXPANSION, EM_LIMITTEXT, MAX_ALIASLEN, 0L);

      // Prepare list of aliases currently defined.
      {
	 LV_COLUMN lvcol;
	 HWND hList;
	 int i;

	 // Prepare list of aliases.
	 hList = GetDlgItem(hDlg, IDC_COMMAND_LIST);
	 SetWindowFont(hList, GetFont(FONT_LIST), FALSE);
	 SetWindowLong(hList, GWL_STYLE, GetWindowLong(hList, GWL_STYLE) | LVS_SHOWSELALWAYS);

	 // Add column headings
	 lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
	 lvcol.pszText = GetString(hInst, IDS_AHEADER0);
	 lvcol.cx = 60;
	 ListView_InsertColumn(hList, 0, &lvcol);
	 lvcol.pszText = GetString(hInst, IDS_AHEADER1);
	 lvcol.cx = 220;
	 ListView_InsertColumn(hList, 1, &lvcol);

	 // Add aliases
	 for (i = 0; i < _nVerbAliases; i++)
	 {
	    if (_apVerbAliases[i].verb[0])
	    {
	       LV_ITEM lvitem;

	       // Add item for verb.
	       lvitem.mask = LVIF_TEXT;
	       lvitem.iItem = ListView_GetItemCount(hList);
	       lvitem.iSubItem = 0;
	       lvitem.pszText = _apVerbAliases[i].verb;
	       lvitem.iItem = ListView_InsertItem(hList, &lvitem);

	       // Add subitem for command.
	       lvitem.mask = LVIF_TEXT;
	       lvitem.iSubItem = 1;
	       lvitem.pszText = _apVerbAliases[i].text;
	       ListView_SetItem(hList, &lvitem);
	    }
	 }
      }

      // Dialog is up.
      hAliasDialog = hDlg;
      hAliasDialog2 = hDlg;
      if (hAliasDialog1)
	 DestroyWindow(GetDlgItem(hDlg, IDC_TUNNEL));

      CenterWindow(hDlg, GetParent(hDlg));
      return TRUE;

   case WM_NOTIFY:
      if (wParam != IDC_COMMAND_LIST)
      {
	 return TRUE;
      }
      else
      {
	 NM_LISTVIEW* pnm = (NM_LISTVIEW*)lParam;
	 LV_HITTESTINFO lvhit;
	 LV_KEYDOWN* plvkey;
	 HWND hList;
	 int i;

	 if (!pnm)
	    break;

	 // Prepare list of aliases.
	 hList = GetDlgItem(hDlg, IDC_COMMAND_LIST);

	 switch (pnm->hdr.code)
	 {
	 case NM_CLICK:
	    {
	       VerbAlias alias;
	       LV_ITEM lvitem;
	       HWND hList;

	       hList = GetDlgItem(hDlg, IDC_COMMAND_LIST);

	       // If you click anywhere on an item.
	       GetCursorPos(&lvhit.pt);
	       ScreenToClient(hList, &lvhit.pt);
	       lvhit.pt.x = 3;
	       i = ListView_HitTest(hList, &lvhit);
	       if (i < 0)
		  break;

	       // Get our current definition.
	       lvitem.mask = LVIF_TEXT;
	       lvitem.iItem = i;
	       lvitem.iSubItem = 0;
	       lvitem.pszText = alias.verb;
	       lvitem.cchTextMax = sizeof(alias.verb);
	       ListView_GetItem(hList, &lvitem);
	       lvitem.iSubItem = 1;
	       lvitem.pszText = alias.text;
	       lvitem.cchTextMax = sizeof(alias.text);
	       ListView_GetItem(hList, &lvitem);

	       // Put it up for editing.
	       SetDlgItemText(hDlg, IDC_VERB, alias.verb);
	       SetDlgItemText(hDlg, IDC_EXPANSION, alias.text);

	       // Select and focus the item in the list.
	       ListView_SetItemState(hList, i, 
				     LVIS_SELECTED | LVIS_FOCUSED,
				     LVIS_SELECTED | LVIS_FOCUSED);
	    }
	    break;

	 case LVN_KEYDOWN:
	    plvkey = (LV_KEYDOWN*)pnm;
	    if (plvkey->wVKey == VK_DELETE)
	    {
//	       for (i = 0; i < nItems; i++)
//		  if (is selected i)
//		     ListView_DeleteItem(hList, i);
	    }
	    break;
	 }
      }
      return TRUE;

   case WM_COMMAND:
      switch(GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDOK:
	 // Erase current set and redefine words in the dialog.
	 {
	    int i, nItems;
	    HWND hList;
	    hList = GetDlgItem(hDlg, IDC_COMMAND_LIST);

	    // Simulate a last click of 'Define' in case they forgot.
	    // Does no harm if they left the boxes empty after their
	    // last 'Define'.
	    //
	    ListView_ClearSelection(hList);
	    SendMessage(hDlg, WM_COMMAND,
	       (WPARAM)MAKELONG(IDC_DEFINE, BN_CLICKED),
	       (LPARAM)GetDlgItem(hDlg, IDC_DEFINE));

	    // Clear our real aliases and rebuild them from our listbox.
	    //
	    FreeVerbAliases();
	    nItems = ListView_GetItemCount(hList);
	    for (i = 0; i < nItems; i++)
	    {
	       VerbAlias alias;
	       LV_ITEM lvitem;

	       lvitem.mask = LVIF_TEXT;
	       lvitem.iItem = i;
	       lvitem.iSubItem = 0;
	       lvitem.pszText = alias.verb;
	       lvitem.cchTextMax = sizeof(alias.verb);
	       ListView_GetItem(hList, &lvitem);
	       lvitem.iSubItem = 1;
	       lvitem.pszText = alias.text;
	       lvitem.cchTextMax = sizeof(alias.text);
	       ListView_GetItem(hList, &lvitem);

	       AddVerbAlias(alias.verb, alias.text);
	    }
	 }
	 EndDialog(hDlg, IDOK);
	 return TRUE;

      case IDCANCEL:
	 EndDialog(hDlg, IDCANCEL);
	 return TRUE;

      case IDC_DEFINE:
	 {
	    VerbAlias alias;
	    LV_ITEM lvitem;
	    HWND hList;

	    hList = GetDlgItem(hDlg, IDC_COMMAND_LIST);

	    // Get our new definition.
	    GetDlgItemText(hDlg, IDC_VERB, alias.verb, sizeof(alias.verb));
	    GetDlgItemText(hDlg, IDC_EXPANSION, alias.text, sizeof(alias.text));
	    if (!alias.verb[0])
	    {
	       SetFocus(GetDlgItem(hDlg, IDC_VERB));
	       return TRUE;
	    }
	    if (!alias.text[0])
	    {
	       SetFocus(GetDlgItem(hDlg, IDC_EXPANSION));
	       return TRUE;
	    }

	    // Simulate a remove so we don't have to hunt and modify existing entry.
	    ListView_ClearSelection(hList);
	    SendMessage(hDlg, WM_COMMAND,
	       (WPARAM)MAKELONG(IDC_REMOVE, BN_CLICKED),
	       (LPARAM)GetDlgItem(hDlg, IDC_REMOVE));

	    // Add item for verb.
	    lvitem.mask = LVIF_TEXT;
	    lvitem.iItem = ListView_GetItemCount(hList);
	    lvitem.iSubItem = 0;
	    lvitem.pszText = alias.verb;
	    lvitem.iItem = ListView_InsertItem(hList, &lvitem);

	    // Add subitem for command.
	    lvitem.mask = LVIF_TEXT;
	    lvitem.iSubItem = 1;
	    lvitem.pszText = alias.text;
	    ListView_SetItem(hList, &lvitem);

	    // Clear the fields to try another.
	    SetDlgItemText(hDlg, IDC_VERB, "");
	    SetDlgItemText(hDlg, IDC_EXPANSION, "");
	    SetFocus(GetDlgItem(hDlg, IDC_VERB));
	 }
	 return TRUE;

      case IDC_REMOVE:
	 {
	    char verb[MAX_VERBLEN+1];
	    LV_FINDINFO lvfind;
	    HWND hList;
	    int i;

	    hList = GetDlgItem(hDlg, IDC_COMMAND_LIST);

	    // Get our new definition.
	    GetDlgItemText(hDlg, IDC_VERB, verb, sizeof(verb));
	    if (!verb[0])
	    {
	       LV_ITEM lvitem;

	       i = ListView_GetSelection(hList);
	       if (i < 0)
	       {
		  SetFocus(GetDlgItem(hDlg, IDC_VERB));
		  return TRUE;
	       }

	       lvitem.mask = LVIF_TEXT;
	       lvitem.iItem = i;
	       lvitem.iSubItem = 0;
	       lvitem.pszText = verb;
	       lvitem.cchTextMax = sizeof(verb);
	       ListView_GetItem(hList, &lvitem);
	       if (!verb[0])
	       {
		  SetFocus(GetDlgItem(hDlg, IDC_VERB));
		  return TRUE;
	       }
	    }

	    // Find item for verb.
	    lvfind.flags = LVFI_STRING | LVFI_WRAP; // not LVFI_PARTIAL
	    lvfind.psz = verb;
	    lvfind.lParam = 0;
	    i = ListView_FindItem(hList, -1, &lvfind);
	    if (i < 0)
	    {
	       SetFocus(GetDlgItem(hDlg, IDC_VERB));
	       Edit_SetSel(GetDlgItem(hDlg, IDC_VERB), 0, -1);
	       return TRUE;
	    }

	    // Remove item.
	    ListView_DeleteItem(hList, i);

	    // Clear the fields to try another.
	    SetDlgItemText(hDlg, IDC_VERB, "");
	    SetDlgItemText(hDlg, IDC_EXPANSION, "");
	    SetFocus(GetDlgItem(hDlg, IDC_VERB));
	 }
	 return TRUE;

      case IDC_TUNNEL:
	 if (!hAliasDialog1 &&
	     IDOK == DialogBox(hInst, MAKEINTRESOURCE(IDD_ALIAS), hDlg, AliasDialogProc))
	 {
	    AliasSave();
	    SetFocus(GetDlgItem(hDlg, IDOK));
	 }
	 return TRUE;
      }
      break;

   case WM_DESTROY:
      hAliasDialog = NULL;
      hAliasDialog2 = NULL;
      return TRUE;
   }
   
   return FALSE;
}
