// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * merintr.c:  Main Meridian interface DLL.
 */

#include "client.h"
#include "merintr.h"
#include "skills.h"

HINSTANCE hInst;              // Handle of this DLL

ClientInfo *cinfo;         // Holds data passed from main client
Bool        exiting;

Bool		gbClassicKeys = FALSE;

static Bool HandleSpells(char *ptr,long len);
static Bool HandleAddSpell(char *ptr,long len);
static Bool HandleRemoveSpell(char *ptr,long len);
static Bool HandleSkills(char *ptr,long len);
static Bool HandleAddSkill(char *ptr,long len);
static Bool HandleRemoveSkill(char *ptr,long len);
static Bool HandleStat(char *ptr,long len);
static Bool HandleStatGroup(char *ptr,long len);
static Bool HandleStatGroups(char *ptr, long len);
static Bool HandleAddEnchantment(char *ptr, long len);
static Bool HandleRemoveEnchantment(char *ptr, long len);
static Bool HandleUserCommand(char *ptr, long len);
static Bool HandleGuildInfo(char *ptr, long len);
static Bool HandleGuildAsk(char *ptr, long len);
static Bool HandleGuildList(char *ptr, long len);
static Bool HandleGuildHalls(char *ptr, long len);
static Bool HandleGuildShield(char *ptr, long len);
static Bool HandleGuildShields(char *ptr, long len);
static Bool HandleLookPlayer(char *ptr, long len);
static Bool HandleSendQuit(char *ptr, long len);
static Bool HandleSpellSchools(char *ptr, long len);
static void CustomConfigInit(void);

// Server message handler table
static handler_struct handler_table[] = {
{ BP_SPELLS,             HandleSpells },
{ BP_SPELL_ADD,          HandleAddSpell },
{ BP_SPELL_REMOVE,       HandleRemoveSpell },
{ BP_SKILLS,             HandleSkills },
{ BP_SKILL_ADD,          HandleAddSkill },
{ BP_SKILL_REMOVE,       HandleRemoveSkill },
{ BP_STAT,               HandleStat },
{ BP_STAT_GROUP,         HandleStatGroup },
{ BP_STAT_GROUPS,        HandleStatGroups },
{ BP_ADD_ENCHANTMENT,    HandleAddEnchantment },
{ BP_REMOVE_ENCHANTMENT, HandleRemoveEnchantment },
{ BP_USERCOMMAND,        HandleUserCommand },
{ 0, NULL},
};

// Client message table
client_message msg_table[] = {
{ BP_SEND_SPELLS,          { PARAM_END }, },
{ BP_SEND_SKILLS,          { PARAM_END }, },
{ BP_SEND_STATS,           { PARAM_GROUP, PARAM_END }, },
{ BP_SEND_STAT_GROUPS,     { PARAM_END }, },
{ BP_SEND_ENCHANTMENTS,    { PARAM_BYTE, PARAM_END }, },
{ 0,                       { PARAM_END }, },
};

// Server user command message table
static handler_struct user_handler_table[] = {
{ UC_GUILDINFO,            HandleGuildInfo, },
{ UC_GUILD_ASK,            HandleGuildAsk, },
{ UC_GUILD_LIST,           HandleGuildList, },
{ UC_GUILD_HALLS,          HandleGuildHalls, },
{ UC_GUILD_SHIELD,         HandleGuildShield, },
{ UC_GUILD_SHIELDS,        HandleGuildShields, },
{ UC_LOOK_PLAYER,          HandleLookPlayer, },
{ UC_SEND_QUIT,            HandleSendQuit, },
{ UC_SPELL_SCHOOLS,        HandleSpellSchools, },
{ 0, NULL},
};

// Client user command message table
client_message user_msg_table[] = {
{ UC_REST,                 { PARAM_END }, },
{ UC_STAND,                { PARAM_END }, },
{ UC_SUICIDE,              { PARAM_END }, },
{ UC_SAFETY,               { PARAM_BYTE, PARAM_END }, },
{ UC_TEMPSAFE,             { PARAM_BYTE, PARAM_END }, },
{ UC_GROUPING,             { PARAM_BYTE, PARAM_END }, },
{ UC_AUTOLOOT,             { PARAM_BYTE, PARAM_END }, },
{ UC_AUTOCOMBINE,          { PARAM_BYTE, PARAM_END }, },
{ UC_REAGENTBAG,           { PARAM_BYTE, PARAM_END }, },
{ UC_SPELLPOWER,           { PARAM_BYTE, PARAM_END }, },
{ UC_REQ_GUILDINFO,        { PARAM_END }, },
{ UC_INVITE,               { PARAM_ID, PARAM_END }, },
{ UC_EXILE,                { PARAM_ID, PARAM_END }, },
{ UC_RENOUNCE,             { PARAM_END }, },
{ UC_ABDICATE,             { PARAM_ID, PARAM_END }, },
{ UC_VOTE,                 { PARAM_ID, PARAM_END }, },
{ UC_SET_RANK,             { PARAM_ID, PARAM_BYTE, PARAM_END }, },
{ UC_GUILD_CREATE,         { PARAM_STRING, PARAM_STRING, PARAM_STRING, PARAM_STRING, PARAM_STRING, 
				PARAM_STRING, PARAM_STRING, PARAM_STRING, PARAM_STRING,
				PARAM_STRING, PARAM_STRING, PARAM_BYTE, PARAM_END }, },
{ UC_GUILD_SHIELDS,        { PARAM_END }, },
{ UC_GUILD_SHIELD,         { PARAM_END }, },
{ UC_CLAIM_SHIELD,         { PARAM_BYTE, PARAM_BYTE, PARAM_BYTE, PARAM_BYTE, PARAM_END }, },
{ UC_DISBAND,              { PARAM_END }, },
{ UC_MAKE_ALLIANCE,        { PARAM_ID, PARAM_END }, },
{ UC_END_ALLIANCE,         { PARAM_ID, PARAM_END }, },
{ UC_MAKE_ENEMY,           { PARAM_ID, PARAM_END }, },
{ UC_END_ENEMY,            { PARAM_ID, PARAM_END }, },
{ UC_REQ_GUILD_LIST,       { PARAM_END }, },
{ UC_GUILD_RENT,           { PARAM_ID, PARAM_STRING, PARAM_END }, },
{ UC_ABANDON_GUILD_HALL,   { PARAM_END }, },
{ UC_GUILD_SET_PASSWORD,   { PARAM_STRING, PARAM_END }, },
{ UC_CHANGE_URL,           { PARAM_ID, PARAM_STRING, PARAM_END }, },
{ UC_WITHDRAW,             { PARAM_INT, PARAM_END }, },
{ UC_DEPOSIT,              { PARAM_INT, PARAM_END }, },
{ UC_BALANCE,              { PARAM_END }, },
{ UC_APPEAL,               { PARAM_STRING, PARAM_END }, },
{ 0,                       { PARAM_END, }, },    // Must end table this way
};

keymap interface_key_table[] = {
{ VK_UP,          KEY_NONE | KEY_ALT,   A_FORWARD },
{ VK_NUMPAD8,     KEY_NONE | KEY_ALT,   A_FORWARD },
{ VK_DOWN,        KEY_NONE | KEY_ALT,   A_BACKWARD },
{ VK_NUMPAD2,     KEY_NONE | KEY_ALT,   A_BACKWARD },

{ VK_UP,          KEY_SHIFT | KEY_ALT,   A_FORWARD },
{ VK_NUMPAD8,     KEY_SHIFT | KEY_ALT,   A_FORWARD },
{ VK_DOWN,        KEY_SHIFT | KEY_ALT,   A_BACKWARD },
{ VK_NUMPAD2,     KEY_SHIFT | KEY_ALT,   A_BACKWARD },

{ VK_UP,          KEY_SHIFT | KEY_ALT | KEY_CTL,   A_FORWARD },
{ VK_NUMPAD8,     KEY_SHIFT | KEY_ALT | KEY_CTL,   A_FORWARD },
{ VK_DOWN,        KEY_SHIFT | KEY_ALT | KEY_CTL,   A_BACKWARD },
{ VK_NUMPAD2,     KEY_SHIFT | KEY_ALT | KEY_CTL,   A_BACKWARD },

{ VK_UP,          KEY_ALT | KEY_CTL,   A_FORWARD },
{ VK_NUMPAD8,     KEY_ALT | KEY_CTL,   A_FORWARD },
{ VK_DOWN,        KEY_ALT | KEY_CTL,   A_BACKWARD },
{ VK_NUMPAD2,     KEY_ALT | KEY_CTL,   A_BACKWARD },

{ VK_UP,          KEY_SHIFT | KEY_CTL,            A_FORWARDFAST },
{ VK_NUMPAD8,     KEY_SHIFT | KEY_CTL,            A_FORWARDFAST },
{ VK_DOWN,        KEY_SHIFT | KEY_CTL,            A_BACKWARDFAST },
{ VK_NUMPAD2,     KEY_SHIFT | KEY_CTL,            A_BACKWARDFAST },

{ VK_UP,          KEY_SHIFT,            A_FORWARDFAST },
{ VK_NUMPAD8,     KEY_SHIFT,            A_FORWARDFAST },
{ VK_DOWN,        KEY_SHIFT,            A_BACKWARDFAST },
{ VK_NUMPAD2,     KEY_SHIFT,            A_BACKWARDFAST },

{ VK_LEFT,        KEY_ALT,              A_SLIDELEFT },
{ VK_RIGHT,       KEY_ALT,              A_SLIDERIGHT }, 
{ VK_NUMPAD4,     KEY_ALT,              A_SLIDELEFT },
{ VK_NUMPAD6,     KEY_ALT,              A_SLIDERIGHT }, 

{ VK_LEFT,        KEY_ALT | KEY_CTL,              A_SLIDELEFT },
{ VK_RIGHT,       KEY_ALT | KEY_CTL,              A_SLIDERIGHT }, 
{ VK_NUMPAD4,     KEY_ALT | KEY_CTL,              A_SLIDELEFT },
{ VK_NUMPAD6,     KEY_ALT | KEY_CTL,              A_SLIDERIGHT }, 

{ VK_LEFT,        KEY_ALT | KEY_SHIFT,              A_SLIDELEFT },
{ VK_RIGHT,       KEY_ALT | KEY_SHIFT,              A_SLIDERIGHT }, 
{ VK_NUMPAD4,     KEY_ALT | KEY_SHIFT,              A_SLIDELEFT },
{ VK_NUMPAD6,     KEY_ALT | KEY_SHIFT,              A_SLIDERIGHT }, 

{ VK_LEFT,        KEY_ALT | KEY_SHIFT | KEY_CTL,              A_SLIDELEFT },
{ VK_RIGHT,       KEY_ALT | KEY_SHIFT | KEY_CTL,              A_SLIDERIGHT }, 
{ VK_NUMPAD4,     KEY_ALT | KEY_SHIFT | KEY_CTL,              A_SLIDELEFT },
{ VK_NUMPAD6,     KEY_ALT | KEY_SHIFT | KEY_CTL,              A_SLIDERIGHT }, 

{ VK_LEFT,        KEY_NONE | KEY_CTL,   A_TURNLEFT },
{ VK_RIGHT,       KEY_NONE | KEY_CTL,   A_TURNRIGHT }, 
{ VK_NUMPAD4,     KEY_NONE | KEY_CTL,   A_TURNLEFT },
{ VK_NUMPAD6,     KEY_NONE | KEY_CTL,   A_TURNRIGHT }, 

{ VK_LEFT,        KEY_SHIFT | KEY_CTL,   A_TURNLEFT },
{ VK_RIGHT,       KEY_SHIFT | KEY_CTL,   A_TURNRIGHT }, 
{ VK_NUMPAD4,     KEY_SHIFT | KEY_CTL,   A_TURNLEFT },
{ VK_NUMPAD6,     KEY_SHIFT | KEY_CTL,   A_TURNRIGHT }, 

{ VK_LEFT,        KEY_SHIFT,            A_TURNFASTLEFT },
{ VK_RIGHT,       KEY_SHIFT,            A_TURNFASTRIGHT }, 
{ VK_NUMPAD4,     KEY_SHIFT,            A_TURNFASTLEFT },
{ VK_NUMPAD6,     KEY_SHIFT,            A_TURNFASTRIGHT }, 

{ VK_PRIOR,       KEY_ANY,              A_LOOKUP },
{ VK_NUMPAD9,     KEY_ANY,              A_LOOKUP },

{ VK_NEXT,        KEY_ANY,              A_LOOKDOWN },
{ VK_NUMPAD3,     KEY_ANY,              A_LOOKDOWN },

{ VK_END,         KEY_ANY,              A_FLIP },
{ VK_NUMPAD1,     KEY_ANY,              A_FLIP },

{ VK_CLEAR,       KEY_ANY,              A_LOOKSTRAIGHT },
{ VK_HOME,        KEY_ANY,              A_LOOKSTRAIGHT },
{ VK_NUMPAD5,     KEY_ANY,              A_LOOKSTRAIGHT },

{ VK_CONTROL,     KEY_ANY,              A_ATTACKCLOSEST },

{ VK_RBUTTON,     KEY_NONE,             A_LOOKMOUSE },
{ VK_LBUTTON,     KEY_NONE,             A_STARTDRAG },
{ VK_LBUTTON,     KEY_ALT,              A_ATTACK },

{ VK_TAB,         KEY_NONE,             A_TABFWD,   (void *) IDC_MAIN },
{ VK_TAB,         KEY_SHIFT,            A_TABBACK,  (void *) IDC_MAIN },
{ VK_ESCAPE,      KEY_ANY,              A_TARGETCLEAR }, // Does an A_GOTOMAIN as well.

{ VK_LBRACKET,   KEY_NONE,              A_TARGETPREVIOUS }, // No idea why it's this value instead of '['. xxx
{ VK_RBRACKET,   KEY_NONE,              A_TARGETNEXT },     // ']'
{ VK_BACKSLASH,  KEY_NONE,              A_TARGETSELF },

{ 'A',            KEY_NONE,             A_TEXTINSERT, "a" },
{ 'B',            KEY_NONE,             A_TEXTINSERT, "b" },
{ 'C',            KEY_NONE,             A_TEXTINSERT, "c" },
{ 'D',            KEY_NONE,             A_TEXTINSERT, "d" },
{ 'E',            KEY_NONE,             A_TEXTINSERT, "e" },
{ 'F',            KEY_NONE,             A_TEXTINSERT, "f" },
{ 'G',            KEY_NONE,             A_TEXTINSERT, "g" },
{ 'H',            KEY_NONE,             A_TEXTINSERT, "h" },
{ 'I',            KEY_NONE,             A_TEXTINSERT, "i" },
{ 'J',            KEY_NONE,             A_TEXTINSERT, "j" },
{ 'K',            KEY_NONE,             A_TEXTINSERT, "k" },
{ 'L',            KEY_NONE,             A_TEXTINSERT, "l" },
{ 'M',            KEY_NONE,             A_TEXTINSERT, "m" },
{ 'N',            KEY_NONE,             A_TEXTINSERT, "n" },
{ 'O',            KEY_NONE,             A_TEXTINSERT, "o" },
{ 'P',            KEY_NONE,             A_TEXTINSERT, "p" },
{ 'Q',            KEY_NONE,             A_TEXTINSERT, "q" },
{ 'R',            KEY_NONE,             A_TEXTINSERT, "r" },
{ 'S',            KEY_NONE,             A_TEXTINSERT, "s" },
{ 'T',            KEY_NONE,             A_TEXTINSERT, "t" },
{ 'U',            KEY_NONE,             A_TEXTINSERT, "u" },
{ 'V',            KEY_NONE,             A_TEXTINSERT, "v" },
{ 'W',            KEY_NONE,             A_TEXTINSERT, "w" },
{ 'X',            KEY_NONE,             A_TEXTINSERT, "x" },
{ 'Y',            KEY_NONE,             A_TEXTINSERT, "y" },
{ 'Z',            KEY_NONE,             A_TEXTINSERT, "z" },

{ VK_F1,          KEY_NONE,             A_TEXTCOMMAND, },  // These keys filled in by aliases
{ VK_F2,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F3,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F4,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F5,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F6,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F7,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F8,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F9,          KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F10,         KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F11,         KEY_NONE,             A_TEXTCOMMAND, },
{ VK_F12,         KEY_NONE,             A_TEXTCOMMAND, },

{ VK_SPACE,       KEY_ANY,              A_GO },
{ VK_RETURN,      KEY_ANY,              A_LOOK },
{ VK_SINGLEQUOTE, KEY_ANY,              A_TEXTINSERT,   "say " },
{ '1',            KEY_SHIFT,            A_TEXTINSERT,   "yell " },
{ '3',            KEY_SHIFT,            A_TEXTINSERT,   "who" },
{ VK_SEMICOLON,   KEY_ANY,              A_TEXTINSERT,   "emote " },
{ VK_PERIOD,      KEY_ANY,              A_TEXTINSERT,   "tell " },
{ VK_SLASH,       KEY_SHIFT,            A_TEXTINSERT,   "help" },

{ VK_ADD,         KEY_ANY,              A_MAPZOOMIN },
{ VK_SUBTRACT,    KEY_ANY,              A_MAPZOOMOUT },

{ 0, 0, 0},   // Must end table this way
};

/* Keys in GAME_SELECT state */
keymap select_key_table[] = {
{ VK_LBUTTON,     KEY_NONE,             A_SELECT,      0 },
{ VK_RETURN,      KEY_ANY,              A_SELECT,      0 },
{ VK_ESCAPE,      KEY_ANY,              A_ENDSELECT },
{ VK_UP,          KEY_ANY,              A_CURSORUP },
{ VK_DOWN,        KEY_ANY,              A_CURSORDOWN },
{ VK_LEFT,        KEY_ANY,              A_CURSORLEFT },
{ VK_RIGHT,       KEY_ANY,              A_CURSORRIGHT }, 
{ VK_PRIOR,       KEY_ANY,              A_CURSORUPRIGHT }, 
{ VK_HOME,        KEY_ANY,              A_CURSORUPLEFT }, 
{ VK_NEXT,        KEY_ANY,              A_CURSORDOWNRIGHT }, 
{ VK_END,         KEY_ANY,              A_CURSORDOWNLEFT }, 
{ VK_NUMPAD8,     KEY_ANY,              A_CURSORUP },
{ VK_NUMPAD2,     KEY_ANY,              A_CURSORDOWN },
{ VK_NUMPAD4,     KEY_ANY,              A_CURSORLEFT },
{ VK_NUMPAD6,     KEY_ANY,              A_CURSORRIGHT }, 
{ VK_NUMPAD9,     KEY_ANY,              A_CURSORUPRIGHT }, 
{ VK_NUMPAD7,     KEY_ANY,              A_CURSORUPLEFT }, 
{ VK_NUMPAD3,     KEY_ANY,              A_CURSORDOWNRIGHT }, 
{ VK_NUMPAD1,     KEY_ANY,              A_CURSORDOWNLEFT }, 

{ 0, 0, 0},   // Must end table this way
};

/* Keys in waiting states, so that user can still quit game */
static keymap default_key_table[] = {
{ 'Q',          KEY_NONE,  A_QUIT },
{ 0, 0, 0},   // Must end table this way
};

static TypedCommand commands[] = {
{ "say",                CommandSay, },
{ "sagen",              CommandSay, },
{ "broadcast",          CommandBroadcast, },
{ "mitteilen",          CommandBroadcast, },
{ "emote",              CommandEmote, },
{ "ego",                CommandEmote, },
{ "who",                CommandWho, },
{ "wer",                CommandWho, },
{ "quit",               CommandQuit, },
{ "beenden",            CommandQuit, },
{ "tell",               CommandTell, },
{ "telepathie",         CommandTell, },
{ "hel",                CommandHel, },
{ "hilf",               CommandHel, },
{ "help",               CommandHelp, },
{ "hilfe",              CommandHelp, },
{ "use",                CommandActivate, },
{ "benutzen",           CommandActivate, },
{ "get",                CommandGet, },
{ "addgroup",           CommandGroupAdd, },
{ "gruppehinzu",        CommandGroupAdd, },
{ "agroup",             CommandGroupAdd, },
{ "gruppeneu",          CommandGroupAdd, },
{ "put",                CommandPut, },
{ "ablegen",            CommandPut, }, 
{ "delgroup",           CommandGroupDelete, },
{ "gruppelöschen",      CommandGroupDelete, }, 
{ "dgroup",             CommandGroupDelete, },
{ "gruppelöschen",      CommandGroupDelete, }, 
{ "newgroup",           CommandGroupNew, },
{ "neuegruppe",         CommandGroupNew, },
{ "ngroup",             CommandGroupNew, },
{ "neuegruppe",         CommandGroupNew, },
{ "buy",                CommandBuy, },
{ "kaufen",             CommandBuy, },
{ "drop",               CommandDrop, },
{ "wegwerfen",          CommandDrop, },
{ "nehmen",             CommandGet, },
{ "look",               CommandLook, },
{ "schauen",            CommandLook, },
{ "offer",              CommandOffer, },
{ "anbieten",           CommandOffer, },
{ "cast",               CommandCast, },
{ "zaubern",            CommandCast, },
{ "map",                CommandMap, },
{ "karte",              CommandMap, },
{ "wave",               CommandWave, },
{ "winken",             CommandWave, },
{ "point",              CommandPoint, },
{ "deuten",             CommandPoint, },
{ "dance",              CommandDance, },
{ "tanzen",             CommandDance, },
{ "alias",              CommandAlias, },
{ "befehle",            CommandAlias, },
{ "cmdalias",           CommandVerbAlias, },
{ "kurzbefehle",        CommandVerbAlias, },
{ "rest",               CommandRest, },
{ "rasten",             CommandRest, },
{ "yell",               CommandYell, },
{ "rufen",              CommandYell, },
{ "stand",              CommandStand, },
{ "aufstehen",          CommandStand, },
{ "suicid",             CommandSuicid, },
{ "haraki",             CommandSuicid, },
{ "suicide",            CommandSuicide, },
{ "harakiri",           CommandSuicide, },
{ "neutral",            CommandNeutral, },
{ "happy",              CommandHappy, },
{ "glücklich",          CommandHappy, },
{ "sad",                CommandSad, },
{ "traurig",            CommandSad, },
{ "wry",                CommandWry, },
{ "grimmig",            CommandWry, },
{ "guild",              CommandGuild, },
{ "gilde",              CommandGuild, },
{ "password",           CommandPassword, },
{ "passwort",           CommandPassword, },
{ "withdraw",           CommandWithdraw, },
{ "abheben",            CommandWithdraw, },
{ "deposit",            CommandDeposit, },
{ "einzahlen",          CommandDeposit, },
{ "balance",            CommandBalance, },
{ "kontostand",         CommandBalance, },
{ "group",              CommandGroup, },
{ "gruppen",            CommandGroup, },
{ "appeal",             CommandAppeal, },
{ "aufrufen",           CommandAppeal, },
{ "tellguild",          CommandTellGuild, },
{ "telgilde",           CommandTellGuild, },
{ "tguild",             CommandTellGuild, },
{ "tgilde",             CommandTellGuild, },
{ "safety on",          CommandSafetyOn, },
{ "sicherheit an",      CommandSafetyOn, },
{ "safety off",         CommandSafetyOff, },
{ "sicherheit aus",     CommandSafetyOff, },
{ "tempsafe on",        CommandTempSafeOn, },
{ "tempsicherheit an",  CommandTempSafeOn, },
{ "tempsafe off",       CommandTempSafeOff, },
{ "tempsicherheit aus", CommandTempSafeOff, },
{ "grouping on",        CommandGroupingOn, },
{ "gruppenbildung an",  CommandGroupingOn, },
{ "grouping off",       CommandGroupingOff, },
{ "gruppenbildung aus", CommandGroupingOff, },
{ "autoloot on",        CommandAutoLootOn, },
{ "autoloot an",        CommandAutoLootOn, },
{ "autoloot off",       CommandAutoLootOff, },
{ "autoloot aus",       CommandAutoLootOff, },
{ "autocombine on",     CommandAutoCombineOn, },
{ "autocombine an",     CommandAutoCombineOn, },
{ "autocombine off",    CommandAutoCombineOff, },
{ "autocombine aus",    CommandAutoCombineOff, },
{ "reagentbag on",      CommandReagentBagOn, },
{ "reagentbag an",      CommandReagentBagOn, },
{ "reagentbag off",     CommandReagentBagOff, },
{ "reagentbag aus",     CommandReagentBagOff, },
{ "spellpower on",      CommandSpellPowerOn, },
{ "spellpower an",      CommandSpellPowerOn, },
{ "spellpower off",     CommandSpellPowerOff, },
{ "spellpower aus",     CommandSpellPowerOff, },
{ NULL,          NULL},    // Must end table this way
};

typedef struct action_label
{
	char	label[32];
	int		action;
	void	*data;
} action_label;

static ascii_key	gAsciiKeyMap[] =
{
	{"mouse0",	VK_LBUTTON},
	{"mouse1",	VK_RBUTTON},
	{"mouse2",	VK_MBUTTON},
	{"enter",	VK_RETURN},
	{"return",	VK_RETURN},
	{"esc",		VK_ESCAPE},
	{"alt",		VK_MENU},
	{"ctrl",	VK_CONTROL},
	{"shift",	VK_SHIFT},
	{"space",	VK_SPACE},
	{"`",		VK_BACKQUOTE},
	{"'",		VK_SINGLEQUOTE},
	{";",		VK_SEMICOLON},
	{".",		VK_PERIOD},
	{",",		VK_COMMA},
	{"/",		VK_SLASH},
	{"[",		VK_LBRACKET},
	{"]",		VK_RBRACKET},
	{"\\",		VK_BACKSLASH},
	{"home",	VK_HOME},
	{"end",		VK_END},
	{"insert",	VK_INSERT},
	{"delete",	VK_DELETE},
	{"pageup",	VK_PRIOR},
	{"pagedown",VK_NEXT},
	{"up",		VK_UP},
	{"down",	VK_DOWN},
	{"left",	VK_LEFT},
	{"right",	VK_RIGHT},
	{"tab",		VK_TAB},
	{"numpad0",	VK_NUMPAD0},
	{"numpad1",	VK_NUMPAD1},
	{"numpad2",	VK_NUMPAD2},
	{"numpad3",	VK_NUMPAD3},
	{"numpad4",	VK_NUMPAD4},
	{"numpad5",	VK_NUMPAD5},
	{"numpad6",	VK_NUMPAD6},
	{"numpad7",	VK_NUMPAD7},
	{"numpad8",	VK_NUMPAD8},
	{"numpad9",	VK_NUMPAD9},
	{"multiply",VK_MULTIPLY},
	{"divide",	VK_DIVIDE},
	{"add",		VK_ADD},
	{"subtract",VK_SUBTRACT},
	{"decimal",	VK_DECIMAL},
	{"separator",VK_SEPARATOR},
	{"f1",		VK_F1},
	{"f2",		VK_F2},
	{"f3",		VK_F3},
	{"f4",		VK_F4},
	{"f5",		VK_F5},
	{"f6",		VK_F6},
	{"f7",		VK_F7},
	{"f8",		VK_F8},
	{"f9",		VK_F9},
	{"f10",		VK_F10},
	{"f11",		VK_F11},
	{"f12",		VK_F12},
	{"\0",		0},
};

// this actionlabel table has to match up exactly with the keymap table below.  don't blame me.
static action_label	gActionLabels[] =
{
	// movement
	{"forward",			A_FORWARD,			NULL},
	{"backward",		A_BACKWARD,			NULL},
	{"left",			A_TURNLEFT,			NULL},
	{"right",			A_TURNRIGHT,		NULL},
	{"slideleft",		A_SLIDELEFT,		NULL},
	{"slideright",		A_SLIDERIGHT,		NULL},
	{"forward",			A_FORWARDFAST,		NULL},
	{"backward",		A_BACKWARDFAST,		NULL},
	{"left",			A_TURNFASTLEFT,		NULL},
	{"right",			A_TURNFASTRIGHT,	NULL},
	{"slideleft",		A_SLIDELEFTFAST,	NULL},
	{"slideright",		A_SLIDERIGHTFAST,	NULL},
	{"lookup",			A_LOOKUP,			NULL},
	{"lookdown",		A_LOOKDOWN,			NULL},
	{"mouselooktoggle",	A_MOUSELOOK,		NULL},

	// commands
	{"attack",			A_ATTACKCLOSEST,	NULL},
	{"open",			A_GO,				NULL},
	{"flip",			A_FLIP,				NULL},
	{"lookstraight",	A_LOOKSTRAIGHT,		NULL},
	{"offer",			A_OFFER,			NULL},
	{"buy",				A_BUY,				NULL},
	{"deposit",			A_DEPOSIT,			NULL},
	{"withdraw",		A_WITHDRAW,			NULL},
	{"mousetarget",		A_ATTACK,			NULL},
	{"targetnext",		A_TARGETNEXT,		NULL},
	{"targetprevious",	A_TARGETPREVIOUS,	NULL},
	{"targetclear",		A_TARGETCLEAR,		NULL},
	{"targetself",		A_TARGETSELF,		NULL},
	{"look",			A_LOOK,				NULL},
	{"pickup",			A_PICKUP,			NULL},
	{"map",				A_MAP,				NULL},
	{"examine",			A_LOOKMOUSE,		NULL},
	{"textcommand",		A_TEXTCOMMAND,		NULL},
	{"mapzoomin",		A_MAPZOOMIN,		NULL},
	{"mapzoomout",		A_MAPZOOMOUT,		NULL},
	{"tabforward",		A_TABFWD,			(void *)IDC_MAIN},
	{"tabbackward",		A_TABBACK,			(void *)IDC_MAIN},

	// chat
	{"chat",			A_GOTOSAY,			NULL},
	{"say",				A_TEXTINSERT,		"say "},
	{"tell",			A_TEXTINSERT,		"tell "},
	{"yell",			A_TEXTINSERT,		"yell "},
	{"broadcast",		A_TEXTINSERT,		"broadcast "},
	{"emote",			A_TEXTINSERT,		"emote "},
	{"who",				A_WHO,				NULL},

	// end
	{"\0", -1, NULL},
};

// this keymap table has to match up exactly with the prior actionlabel table.  don't blame me.
keymap	gCustomKeys[] =
{
	// movement
	{(WORD)-1,				(WORD)-1,				A_FORWARD,			NULL},
	{(WORD)-1,				(WORD)-1,				A_BACKWARD,			NULL},
	{(WORD)-1,				(WORD)-1,				A_TURNLEFT,			NULL},
	{(WORD)-1,				(WORD)-1,				A_TURNRIGHT,		NULL},
	{(WORD)-1,				(WORD)-1,				A_SLIDELEFT,		NULL},
	{(WORD)-1,				(WORD)-1,				A_SLIDERIGHT,		NULL},
	{(WORD)-1,				(WORD)-1,				A_FORWARDFAST,		NULL},
	{(WORD)-1,				(WORD)-1,				A_BACKWARDFAST,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TURNFASTLEFT,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TURNFASTRIGHT,	NULL},
	{(WORD)-1,				(WORD)-1,				A_SLIDELEFTFAST,	NULL},
	{(WORD)-1,				(WORD)-1,				A_SLIDERIGHTFAST,	NULL},
	{(WORD)-1,				(WORD)-1,				A_LOOKUP,			NULL},
	{(WORD)-1,				(WORD)-1,				A_LOOKDOWN,			NULL},
	{(WORD)-1,				(WORD)-1,				A_MOUSELOOK,		NULL},

	// commands
	{(WORD)-1,				(WORD)-1,				A_ATTACKCLOSEST,	NULL},
	{(WORD)-1,				(WORD)-1,				A_GO,				NULL},
	{(WORD)-1,				(WORD)-1,				A_FLIP,				NULL},
	{(WORD)-1,				(WORD)-1,				A_LOOKSTRAIGHT,		NULL},
	{(WORD)-1,				(WORD)-1,				A_OFFER,			NULL},
	{(WORD)-1,				(WORD)-1,				A_BUY,				NULL},
	{(WORD)-1,				(WORD)-1,				A_DEPOSIT,			NULL},
	{(WORD)-1,				(WORD)-1,				A_WITHDRAW,			NULL},
	{(WORD)-1,				(WORD)-1,				A_ATTACK,			NULL},
	{(WORD)-1,				(WORD)-1,				A_TARGETNEXT,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TARGETPREVIOUS,	NULL},
	{(WORD)-1,				(WORD)-1,				A_TARGETCLEAR,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TARGETSELF,		NULL},
	{(WORD)-1,				(WORD)-1,				A_LOOK,				NULL},
	{(WORD)-1,				(WORD)-1,				A_PICKUP,			NULL},
	{(WORD)-1,				(WORD)-1,				A_MAP,				NULL},
	{(WORD)-1,				(WORD)-1,				A_LOOKMOUSE,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TEXTCOMMAND,		NULL},
	{(WORD)-1,				(WORD)-1,				A_MAPZOOMIN,		NULL},
	{(WORD)-1,				(WORD)-1,				A_MAPZOOMOUT,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TABFWD,			(void *)IDC_MAIN},
	{(WORD)-1,				(WORD)-1,				A_TABBACK,			(void *)IDC_MAIN},

	// chat
	{(WORD)-1,				(WORD)-1,				A_GOTOSAY,			NULL},
	{(WORD)-1,				(WORD)-1,				A_TEXTINSERT,		"say "},
	{(WORD)-1,				(WORD)-1,				A_TEXTINSERT,		"tell "},
	{(WORD)-1,				(WORD)-1,				A_TEXTINSERT,		"yell "},
	{(WORD)-1,				(WORD)-1,				A_TEXTINSERT,		"broadcast "},
	{(WORD)-1,				(WORD)-1,				A_TEXTINSERT,		"emote "},
	{(WORD)-1,				(WORD)-1,				A_WHO,				NULL},

	// aliases (fkey hotkeys)
	{(WORD)-1,				(WORD)-1,				A_TEXTCOMMAND,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TEXTCOMMAND,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TEXTCOMMAND,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TEXTCOMMAND,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TEXTCOMMAND,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TEXTCOMMAND,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TEXTCOMMAND,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TEXTCOMMAND,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TEXTCOMMAND,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TEXTCOMMAND,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TEXTCOMMAND,		NULL},
	{(WORD)-1,				(WORD)-1,				A_TEXTCOMMAND,		NULL},

	// end
	{0, 0, 0},
};

keymap gQuickChatTable[] = {
{ 'A',            KEY_NONE,             A_TEXTINSERT, "a" },
{ 'B',            KEY_NONE,             A_TEXTINSERT, "b" },
{ 'C',            KEY_NONE,             A_TEXTINSERT, "c" },
{ 'D',            KEY_NONE,             A_TEXTINSERT, "d" },
{ 'E',            KEY_NONE,             A_TEXTINSERT, "e" },
{ 'F',            KEY_NONE,             A_TEXTINSERT, "f" },
{ 'G',            KEY_NONE,             A_TEXTINSERT, "g" },
{ 'H',            KEY_NONE,             A_TEXTINSERT, "h" },
{ 'I',            KEY_NONE,             A_TEXTINSERT, "i" },
{ 'J',            KEY_NONE,             A_TEXTINSERT, "j" },
{ 'K',            KEY_NONE,             A_TEXTINSERT, "k" },
{ 'L',            KEY_NONE,             A_TEXTINSERT, "l" },
{ 'M',            KEY_NONE,             A_TEXTINSERT, "m" },
{ 'N',            KEY_NONE,             A_TEXTINSERT, "n" },
{ 'O',            KEY_NONE,             A_TEXTINSERT, "o" },
{ 'P',            KEY_NONE,             A_TEXTINSERT, "p" },
{ 'Q',            KEY_NONE,             A_TEXTINSERT, "q" },
{ 'R',            KEY_NONE,             A_TEXTINSERT, "r" },
{ 'S',            KEY_NONE,             A_TEXTINSERT, "s" },
{ 'T',            KEY_NONE,             A_TEXTINSERT, "t" },
{ 'U',            KEY_NONE,             A_TEXTINSERT, "u" },
{ 'V',            KEY_NONE,             A_TEXTINSERT, "v" },
{ 'W',            KEY_NONE,             A_TEXTINSERT, "w" },
{ 'X',            KEY_NONE,             A_TEXTINSERT, "x" },
{ 'Y',            KEY_NONE,             A_TEXTINSERT, "y" },
{ 'Z',            KEY_NONE,             A_TEXTINSERT, "z" },
{ 0, 0, 0},   // Must end table this way
};

/* local function prototypes */
static spell       *ExtractNewSpell(char **ptr);
static Bool         ExtractStatistic(char **ptr, Statistic *s);
/****************************************************************************/
BOOL WINAPI DllMain(HINSTANCE hModule, DWORD reason, LPVOID reserved)
{
   switch (reason)
   {
   case DLL_PROCESS_ATTACH:
      hInst = hModule;
      break;

   case DLL_PROCESS_DETACH:
      break;
   }
   return TRUE;
}
/****************************************************************************/
void WINAPI GetModuleInfo(ModuleInfo *info, ClientInfo *client_info)
{
   info->event_mask = EVENT_SERVERMSG | EVENT_USERACTION | EVENT_FONTCHANGED |
      EVENT_STATECHANGED | EVENT_MENUITEM | EVENT_TEXTCOMMAND | EVENT_RESIZE |
	 EVENT_USERCHANGED | EVENT_REDRAW | EVENT_DRAWITEM | EVENT_COLORCHANGED |
	    EVENT_RESETDATA | EVENT_INVENTORY | EVENT_ANIMATE | EVENT_MOUSECLICK |
	       EVENT_SETCURSOR | EVENT_NEWROOM | EVENT_CONFIGCHANGED;
   info->priority   = PRIORITY_NORMAL;
   info->module_id  = MODULE_ID;
   cinfo = client_info;    // Save client info for our use later

   exiting = False;

   // Add game key tables
   //KeyAddTable(GAME_PLAY, interface_key_table);
   CustomConfigInit();

   KeyAddTable(GAME_SELECT, select_key_table);
   KeyAddTable(GAME_INVALID, default_key_table);

   InterfaceInit();
}
/****************************************************************************/
void WINAPI ModuleExit(void)
{
   KeyRemoveTable(GAME_PLAY, interface_key_table);
   KeyRemoveTable(GAME_SELECT, select_key_table);
   KeyRemoveTable(GAME_INVALID, default_key_table);

   FreeVerbAliases();

   exiting = True;
   InterfaceExit();
}

/* event handlers */
/****************************************************************************/
/*
 * EVENT_SERVERMSG
 */
/****************************************************************************/
Bool WINAPI EventServerMessage(char *message, long len)
{
   Bool retval;

   retval = LookupMessage(message, len, handler_table);

   // If we handle message, don't pass it on to anyone else
   if (retval == True)
     return False;

   return True;    // Allow other modules to get other messages
}

void CustomConfigInit(void)
{
	int		i, curKey, curAction, fKey, bQuickChat, value;
	WORD	flags, runKey;
	char	string0[255];
	char	string1[255];
	char	*pChar1;
	char	keys[] = "keys";
	char	config[] = "config";
	char	file[] = "./config.ini";
	FILE	*pFile;

	i = curKey = curAction = fKey = 0;
	bQuickChat = FALSE;

	GetPrivateProfileString(config, "initialized", "error\n", string0,
		255, file);

	strupr(string0);
	if (0 != strcmp(string0, "TRUE"))
	// if config file doesn't exist, create a default one
//	if (NULL == (pFile = fopen("./config.ini", "r")))
	{
		pFile = fopen("./config.ini", "w");

		fprintf(pFile, "Configuration file for Meridian 59\n\n\n");

		fprintf(pFile, "This file enables the new custom configuration settings and key bindings.  The [config] section contains the following options:\n\n");

		fprintf(pFile, "initialized:  Set this to false (or delete this file) to have Meridian create a default config.ini file for you\n");
		fprintf(pFile, "classickeybindings:  Set this to true to enable key bindings identical to the old Meridian client (later key bindings are ignored)\n");
		fprintf(pFile, "invertmouse:  Set to true to invert mouselook's y-axis\n");
		fprintf(pFile, "mouselookxscale/mouselookyscale:  Sets the mouse sensitivity.  Accepts any value from 1 to 30\n");
		fprintf(pFile, "quickchat:  Set to true to have all alphabetic keys go instantly to chat (any later key binding using an alphabetic key will be ignored)\n");
		fprintf(pFile, "alwaysrun:  Set to true to avoid having to hold the run key (default: shift) to run\n");
		fprintf(pFile, "dynamiclighting:  Set to true to enable dynamic lighting.  Setting this to false may increase performance on some systems\n\n");
		fprintf(pFile, "softwarerenderer:  Set to true to force the client to run in software graphics mode\n\n");
		fprintf(pFile, "rendererfailedonce:  NOT USER EDITABLE.  The client will set this to true when it determines that your hardware/driver cannot run Meridian in Direct3D.  This will prevent any further error messages until you either delete this file or change video cards/drivers\n\n");

		fprintf(pFile, "The [key] section represents your key bindings.  It uses the following format:\n\n");

		fprintf(pFile, "<action>=<key>[+<modifier>]\n\n");

		fprintf(pFile, "Where <key> can be any of the following keys:\n\n");

		fprintf(pFile, "a-z, mouse0-mouse5, pageup, pagedown, home, end, delete, enter, space, up, down, left, right, esc, tab, ',', ';', '.', ''', '[', or ']' \n\n");
		fprintf(pFile, "And the optional <modifier> can be: shift, alt, ctrl, or any\n\n");
		fprintf(pFile, "Use 'any' for keys that you would like to work inclusively with all other actions.  Some good examples of this are the 'attack' and 'mouselooktoggle' actions\n\n");

		fclose(pFile);

		WritePrivateProfileString("config", "initialized", "true", "./config.ini");
		WritePrivateProfileString("config", "classickeybindings", "false", "./config.ini");
		WritePrivateProfileString("config", "invertmouse", "false", "./config.ini");
		WritePrivateProfileString("config", "mouselookxscale", "15", "./config.ini");
		WritePrivateProfileString("config", "mouselookyscale", "9", "./config.ini");
		WritePrivateProfileString("config", "quickchat", "false", "./config.ini");
		WritePrivateProfileString("config", "alwaysrun", "true", "./config.ini");
		WritePrivateProfileString("config", "attackontarget", "false", "./config.ini");
		WritePrivateProfileString("config", "dynamiclighting", "true", "./config.ini");
		WritePrivateProfileString("config", "softwarerenderer", "false", "./config.ini");

		WritePrivateProfileString("keys", "forward", "w", "./config.ini");
		WritePrivateProfileString("keys", "backward", "s", "./config.ini");
		WritePrivateProfileString("keys", "left", "left", "./config.ini");
		WritePrivateProfileString("keys", "right", "right", "./config.ini");
		WritePrivateProfileString("keys", "slideleft", "a", "./config.ini");
		WritePrivateProfileString("keys", "slideright", "d", "./config.ini");
		WritePrivateProfileString("keys", "run/walk", "shift", "./config.ini");
		WritePrivateProfileString("keys", "lookup", "pageup", "./config.ini");

		WritePrivateProfileString("keys", "lookdown", "pagedown", "./config.ini");
		WritePrivateProfileString("keys", "attack", "e+any", "./config.ini");
		WritePrivateProfileString("keys", "open", "space", "./config.ini");
		WritePrivateProfileString("keys", "say", "f", "./config.ini");
		WritePrivateProfileString("keys", "chat", "enter", "./config.ini");
		WritePrivateProfileString("keys", "tell", "t", "./config.ini");
		WritePrivateProfileString("keys", "yell", "y", "./config.ini");
		WritePrivateProfileString("keys", "broadcast", "b", "./config.ini");

		WritePrivateProfileString("keys", "lookstraight", "home", "./config.ini");
		WritePrivateProfileString("keys", "flip", "end", "./config.ini");
		WritePrivateProfileString("keys", "who", "w+ctrl", "./config.ini");
		WritePrivateProfileString("keys", "look", "l", "./config.ini");
		WritePrivateProfileString("keys", "pickup", "g", "./config.ini");
		WritePrivateProfileString("keys", "offer", "o+ctrl", "./config.ini");
		WritePrivateProfileString("keys", "buy", "b+ctrl", "./config.ini");
		WritePrivateProfileString("keys", "deposit", "d+shift", "./config.ini");

		WritePrivateProfileString("keys", "withdraw", "w+shift", "./config.ini");
		WritePrivateProfileString("keys", "targetnext", "]", "./config.ini");
		WritePrivateProfileString("keys", "targetprevious", "[", "./config.ini");
		WritePrivateProfileString("keys", "emote", ";", "./config.ini");
		WritePrivateProfileString("keys", "map", "m+shift", "./config.ini");
		WritePrivateProfileString("keys", "examine", "mouse1+any", "./config.ini");
		WritePrivateProfileString("keys", "mouselooktoggle", "c+any", "./config.ini");
		WritePrivateProfileString("keys", "mousetarget", "mouse0", "./config.ini");

		WritePrivateProfileString("keys", "targetclear", "esc", "./config.ini");
		WritePrivateProfileString("keys", "targetself", "q", "./config.ini");
		WritePrivateProfileString("keys", "tabforward", "tab", "./config.ini");
		WritePrivateProfileString("keys", "tabbackward", "tab+shift", "./config.ini");
		WritePrivateProfileString("keys", "mapzoomin", "add", "./config.ini");
		WritePrivateProfileString("keys", "mapzoomout", "subtract", "./config.ini");
	}

	// now check config settings

	// attack on target
	GetPrivateProfileString(config, "attackontarget", "error\n", string0,
		255, file);

	strupr(string0);
	if (0 == strcmp(string0, "TRUE"))
		cinfo->config->bAttackOnTarget = TRUE;
	else
		cinfo->config->bAttackOnTarget = FALSE;

	// always run
	GetPrivateProfileString(config, "alwaysrun", "error\n", string0,
		255, file);

	strupr(string0);
	if (0 == strcmp(string0, "TRUE"))
		cinfo->config->bAlwaysRun = TRUE;
	else
		cinfo->config->bAlwaysRun = FALSE;

	// mouselook x scale
	GetPrivateProfileString(config, "mouselookxscale", "error\n", string0,
		255, file);

	value = atoi(string0);
	cinfo->config->mouselookXScale = min(30, max(1, value));

	// mouselook y scale
	GetPrivateProfileString(config, "mouselookyscale", "error\n", string0,
		255, file);

	value = atoi(string0);
	cinfo->config->mouselookYScale = min(30, max(1, value));

	// mouselook invert
	GetPrivateProfileString(config, "invertmouse", "error\n", string0,
		255, file);

	strupr(string0);
	if (0 == strcmp(string0, "TRUE"))
		cinfo->config->bInvertMouse = TRUE;
	else
	{
		cinfo->config->bInvertMouse = FALSE;
		cinfo->config->mouselookYScale = -cinfo->config->mouselookYScale;
	}

	// dynamic lighting
	GetPrivateProfileString(config, "dynamiclighting", "error\n", string0,
		255, file);

	strupr(string0);
	if (0 == strcmp(string0, "TRUE"))
		cinfo->config->bDynamicLighting = TRUE;
	else
		cinfo->config->bDynamicLighting = FALSE;

	// now key bindings
	// first check to see if they want classic controls
	// quick chat
	GetPrivateProfileString(config, "classickeybindings", "error\n", string0,
		255, file);

	strupr(string0);
	if (0 == strcmp(string0, "TRUE"))
	{
		// if yes, use default key table and we're done
		KeyAddTable(GAME_PLAY, interface_key_table);
		gbClassicKeys = TRUE;
	}
	else
	{
		GetPrivateProfileString(config, "quickchat", "error\n", string0,
			255, file);

		strupr(string0);
		if (0 == strcmp(string0, "TRUE"))
		{
			KeyAddTable(GAME_PLAY, gQuickChatTable);
			bQuickChat = TRUE;
		}

		// find out which key is bound to "run"
		GetPrivateProfileString(keys, "run/walk", "error\n", string0,
			255, file);

		if (strcmp(string0, "error\n") == 0)
			strcpy(string0, "shift");

		if (0 == strcmp(string0, "shift"))
			runKey = KEY_SHIFT;

		if (0 == strcmp(string0, "alt"))
			runKey = KEY_ALT;

		if (0 == strcmp(string0, "ctrl"))
			runKey = KEY_CTL;

		for (curAction = 0; gActionLabels[curAction].action != -1; curAction++, curKey++)
		{
			GetPrivateProfileString(keys, gActionLabels[curAction].label, "error\n", string0,
				255, file);

			if (strcmp(string0, "error\n") == 0)
				continue;

			// check to see if this is a multi-key bind
			if (pChar1 = strpbrk(string0, "+"))
			{
				strcpy(string1, &pChar1[1]);
				*pChar1 = '\0';

				if (0 == strcmp(string1, "shift"))
					flags = KEY_SHIFT;

				if (0 == strcmp(string1, "alt"))
					flags = KEY_ALT;

				if (0 == strcmp(string1, "ctrl"))
					flags = KEY_CTL;

				if (0 == strcmp(string1, "any"))
					flags = KEY_ANY;
			}
			else
				flags = KEY_NONE;

			if //((strlen(string0) == 1) && 
				(((string0[0] >= 'a') && (string0[0] <= 'z')) &&
				((string0[1] == '+') || (string0[1] == '\0')))
				
			{
				if (bQuickChat)
					continue;

				strupr(string0);
				gCustomKeys[curKey].vk_code = string0[0];
				gCustomKeys[curKey].flags = flags;
			}
			else
			{
				for (i = 0; gAsciiKeyMap[i].asciiName[0] != '\0'; i++)
				{
					if (strcmp(string0, gAsciiKeyMap[i].asciiName) == 0)
					{
						gCustomKeys[curKey].vk_code = gAsciiKeyMap[i].vkCode;
						gCustomKeys[curKey].flags = flags;
					}
				}
			}

			if (IsMoveFastAction(gActionLabels[curAction].action) ||
				IsTurnFastAction(gActionLabels[curAction].action))
			{
				if (cinfo->config->bAlwaysRun)
					gCustomKeys[curKey].flags = flags;
				else
					gCustomKeys[curKey].flags = (gCustomKeys[curKey].flags & (~KEY_NONE)) | runKey;
			}
			else if (IsMoveAction(gActionLabels[curAction].action) ||
					IsTurnAction(gActionLabels[curAction].action))
			{
				if (cinfo->config->bAlwaysRun)
					gCustomKeys[curKey].flags = (gCustomKeys[curKey].flags & (~KEY_NONE)) | runKey;
				else
					gCustomKeys[curKey].flags = flags;
			}
		}

		for (i = 0, fKey = VK_F1; i < 12; i++, curKey++, fKey++)
		{
			gCustomKeys[curKey].vk_code = fKey;
			gCustomKeys[curKey].flags = KEY_ANY;
		}

		KeyAddTable(GAME_PLAY, gCustomKeys);
	}
}
/********************************************************************/
/* 
 * ExtractNewSpell: Similar to ExtractNewObject, but extracts a spell
 */
spell *ExtractNewSpell(char **ptr)
{
   spell *s = (spell *) SafeMalloc(sizeof(spell));
   ZeroMemory(s,sizeof(spell));
   ExtractObject(ptr, &s->obj);
   Extract(ptr, &s->num_targets, 1);
   Extract(ptr, &s->school, 1);
   s->school -= 1;  // Convert to 0 based
   return s;
}  
/********************************************************************/
/* 
 * ExtractNewSkill: Similar to ExtractNewObject, but extracts a skill
 */
skill *ExtractNewSkill(char **ptr)
{
   skill *s = (skill *) SafeMalloc(sizeof(skill));
   ZeroMemory(s,sizeof(skill));
   ExtractObject(ptr, &s->obj);
   return s;
}  
/********************************************************************/
/*
 * ExtractStatistic: Fill s with data fom ptr, and increment ptr appropriately.
 *   Return True on success.
 */
Bool ExtractStatistic(char **ptr, Statistic *s)
{
   Extract(ptr, &s->num, SIZE_STAT_NUM);
   Extract(ptr, &s->name_res, SIZE_ID);
   Extract(ptr, &s->type, 1);
   switch (s->type)
   {
   case STATS_NUMERIC:
      Extract(ptr, &s->numeric.tag,   SIZE_STAT_TYPE);
      Extract(ptr, &s->numeric.value, SIZE_ID);
      
      if (s->numeric.tag == STAT_INT)
      {
	 Extract(ptr, &s->numeric.min, SIZE_ID);
	 Extract(ptr, &s->numeric.max, SIZE_ID);
	 Extract(ptr, &s->numeric.current_max, SIZE_ID);
      }
      break;
      
   case STATS_LIST:
      Extract(ptr, &s->list.id, SIZE_ID);
      Extract(ptr, &s->list.value, SIZE_ID);
      Extract(ptr, &s->list.icon, SIZE_ID);
      break;
      
   default:
      debug(("ExtractStatistic got unknown stat type %d\n", (int) s->type));
      return False;
   }
   return True;
}
/********************************************************************/
Bool HandleSpells(char *ptr,long len)
{
   WORD list_len, i;
   list_type list = NULL;
   char *start;

   if (len < SIZE_LIST_LEN)
      return FALSE;

   Extract(&ptr, &list_len, SIZE_LIST_LEN);
   len -= SIZE_LIST_LEN;

   start = ptr;
   for (i=0; i < list_len; i++)
      list = list_add_item(list, ExtractNewSpell(&ptr));
   len -= (ptr - start);
   if (len != 0)
   {
      ObjectListDestroy(list);
      return False;
   }   
   SetSpells(list);
   return True;
}
/********************************************************************/
Bool HandleAddSpell(char *ptr, long len)
{
   spell *sp;
   char *start = ptr;

   sp = ExtractNewSpell(&ptr);
   
   len -= (ptr - start);
   if (len != 0)
   {
      SafeFree(sp);
      return False;
   }
   AddSpell(sp);   
   return True;
}
/********************************************************************/
Bool HandleRemoveSpell(char *ptr, long len)
{
   ID spell_id;

   if (len != SIZE_ID)
      return False;
   Extract(&ptr, &spell_id, SIZE_ID);   
   RemoveSpell(spell_id);
   
   return True;
}
/********************************************************************/
Bool HandleSkills(char *ptr,long len)
{
   WORD list_len, i;
   list_type list = NULL;
   char *start;

   if (len < SIZE_LIST_LEN)
      return FALSE;

   Extract(&ptr, &list_len, SIZE_LIST_LEN);
   len -= SIZE_LIST_LEN;

   start = ptr;
   for (i=0; i < list_len; i++)
      list = list_add_item(list, ExtractNewSkill(&ptr));
   len -= (ptr - start);
   if (len != 0)
   {
      ObjectListDestroy(list);
      return False;
   }   
   SetSkills(list);
   return True;
}
/********************************************************************/
Bool HandleAddSkill(char *ptr, long len)
{
   skill *sp;
   char *start = ptr;

   sp = ExtractNewSkill(&ptr);
   
   len -= (ptr - start);
   if (len != 0)
   {
      SafeFree(sp);
      return False;
   }
   AddSkill(sp);   
   return True;
}
/********************************************************************/
Bool HandleRemoveSkill(char *ptr, long len)
{
   ID skill_id;

   if (len != SIZE_ID)
      return False;
   Extract(&ptr, &skill_id, SIZE_ID);   
   RemoveSkill(skill_id);
   
   return True;
}
/********************************************************************/
Bool HandleStat(char *ptr, long len)
{
   BYTE group;
   Statistic s;
   char *start = ptr;

   Extract(&ptr, &group, SIZE_GROUP);
   if (ExtractStatistic(&ptr, &s) == False)
      return False;

   len -= (ptr - start);
   if (len != 0)
      return False;
   
   StatChange(group, &s);
   return True;
}
/********************************************************************/
Bool HandleStatGroup(char *ptr, long len)
{
   list_type stat_list = NULL;
   BYTE group, list_len;
   int i;
   char *start = ptr;

   if (len < SIZE_GROUP + SIZE_NUM_STATS)
      return False;

   Extract(&ptr, &group, SIZE_GROUP);
   Extract(&ptr, &list_len, SIZE_NUM_STATS);

   for (i=0; i < list_len; i++)
   {
      Statistic *s = (Statistic *) SafeMalloc(sizeof(Statistic));

      if (ExtractStatistic(&ptr, s) == False)
      {
	 list_destroy(stat_list);
	 return False;
      }
      stat_list = list_add_item(stat_list, s);
   }

   len -= (ptr - start);
   if (len != 0)
   {
      list_destroy(stat_list);
      return False;
   }

   StatsReceiveGroup(group, stat_list);
   return True;
}
/********************************************************************/
Bool HandleStatGroups(char *ptr, long len)
{
   char *start = ptr;
   BYTE num_groups;
   ID *names;
   int i;

   Extract(&ptr, &num_groups, 1);
   names = (ID *) SafeMalloc(num_groups * SIZE_ID);
   for (i=0; i < num_groups; i++)
      Extract(&ptr, &names[i], SIZE_ID);
   
   len -= (ptr - start);
   if (len != 0)
      return False;

   StatsGroupsInfo(num_groups, names);
   return True;
}
/********************************************************************/
Bool HandleAddEnchantment(char *ptr, long len)
{
   BYTE type;
   object_node *obj;
   char *start = ptr;

   Extract(&ptr, &type, 1);
   obj = ExtractNewObject(&ptr);

   len -= (ptr - start);
   if (len != 0)
   {
      ObjectDestroy(obj);
      return False;
   }

   EnchantmentAdd(type, obj);
   return True;
}
/********************************************************************/
Bool HandleRemoveEnchantment(char *ptr, long len)
{
   BYTE type;
   ID obj_id;
   char *start = ptr;
   
   Extract(&ptr, &type, 1);
   Extract(&ptr, &obj_id, SIZE_ID);

   len -= (ptr - start);
   if (len != 0)
      return False;

   EnchantmentRemove(type, obj_id);
   return True;
}
/********************************************************************/
Bool HandleUserCommand(char *ptr, long len)
{
   BYTE type;
   int index;
   Bool success;

   UserAreaRedraw();

   Extract(&ptr, &type, SIZE_TYPE);

   // Dispatch on user command code
   index = 0;
   while (user_handler_table[index].msg_type != 0)
   {
      if (user_handler_table[index].msg_type == type)
      {
	 if (user_handler_table[index].handler != NULL)
	 {                           
	    /* Don't count type byte in length for handler */
	    success = (*user_handler_table[index].handler)(ptr, len - SIZE_TYPE);
	    if (!success)
	    {
	       debug(("Error in user command message of type %d from server\n", type));
	       return False;
	    }
	    return True;
	 }
	 break;
      }
      index++;
   }
   return False;   // Pass on to other modules
}
/********************************************************************/
Bool HandleGuildInfo(char *ptr, long len)
{
   char *start = ptr;
   GuildConfigDialogStruct info;
   int i;

   len = ExtractString(&ptr, len, info.name, MAX_GUILD_NAME);
   Extract(&ptr, &info.has_password, 1);
   len -= 1;
   if (info.has_password)
     len = ExtractString(&ptr, len, info.password, MAX_GUILD_NAME);

   Extract(&ptr, &info.flags, 4);
   len -= 4;

   // Get player's guild id
   Extract(&ptr, &info.guild_id, SIZE_ID);
   len -= SIZE_ID;

   // Get rank names
   for (i=0; i < NUM_GUILD_RANKS; i++)
   {
      len = ExtractString(&ptr, len, info.male_ranks[i], MAX_RANK_LENGTH);
      len = ExtractString(&ptr, len, info.female_ranks[i], MAX_RANK_LENGTH);
   }

   // Get current vote
   Extract(&ptr, &info.current_vote, SIZE_ID);
   len -= SIZE_ID;

   Extract(&ptr, &info.num_users, 2);
   len -= 2;

   if (info.num_users > MAX_GUILD_USERS)
      return False;

   for (i = 0; i < info.num_users; i++)
   {
      GuildMember *member = &info.members[i];
      Extract(&ptr, &member->id, SIZE_ID);
      len -= SIZE_ID;
      len = ExtractString(&ptr, len, member->name, MAXUSERNAME);
      if (len == -1)
	 return False;

      Extract(&ptr, &member->rank, 1);
      Extract(&ptr, &member->gender, 1);
      len -= 2;
   }

   if (len != 0)
      return False;

   GuildShieldInit();		//	Initialize Guild Shield page.

   GuildConfigInitInfo(&info);
   return True;
}
/********************************************************************/
Bool HandleGuildAsk(char *ptr, long len)
{
  int cost1, cost2;
  
  if (len != 8)
    return False;

  Extract(&ptr, &cost1, 4);
  Extract(&ptr, &cost2, 4);

  GuildCreate(cost1, cost2);
  return True;
}
/********************************************************************/
Bool HandleGuildList(char *ptr, long len)
{
   list_type guild_list;
   IDList other_guilds[4];
   WORD num_guilds;
   int i, j;
   ID id;
   
   Extract(&ptr, &num_guilds, 2);
   len -= 2;
   guild_list = NULL;
   for (j=0; j < num_guilds; j++)
   {
      GuildInfo *info = (GuildInfo *) SafeMalloc(sizeof(GuildInfo));
      
      Extract(&ptr, &info->id, SIZE_ID);
      len -= SIZE_ID;
      len = ExtractString(&ptr, len, info->name, MAX_GUILD_NAME);
      guild_list = list_add_item(guild_list, info);
   }

   for (i=0; i < 4; i++)
   {
      Extract(&ptr, &num_guilds, 2);
      len -= 2;
      other_guilds[i] = NULL;
      for (j=0; j < num_guilds; j++)
      {
	 Extract(&ptr, &id, SIZE_ID);
	 len -= SIZE_ID;
	 other_guilds[i] = IDListAdd(other_guilds[i], id);
      }
   }

   if (len != 0)
   {
      list_destroy(guild_list);
      for (i=0; i < 4; i++)
	 IDListDelete(other_guilds[i]);
      return False;
   }
   
   GuildGotList(guild_list, other_guilds[0], other_guilds[1], other_guilds[2], other_guilds[3]);

   return True;
}
/********************************************************************/
Bool HandleGuildShield(char *ptr, long len)
{
   char name[MAX_GUILD_NAME + 1];  // Name of guild
   ID id;
   BYTE color1;
   BYTE color2;
   BYTE pattern;

   Extract(&ptr, &id, SIZE_ID);
   len -= SIZE_ID;

   len = ExtractString(&ptr, len, name, MAX_GUILD_NAME);

   Extract(&ptr, &color1, 1);
   Extract(&ptr, &color2, 1);
   Extract(&ptr, &pattern, 1);
   len -= 3;

   if (len != 0)
      return False;

   GuildGotShield(id, name, color1, color2, pattern);

   return True;
}
/********************************************************************/
Bool HandleGuildShields(char *ptr, long len)
{
   list_type shield_list;
   WORD num_guildshields;
   int j;

   Extract(&ptr, &num_guildshields, 2);
   len -= 2;
   shield_list = NULL;
   for (j=0; j < num_guildshields; j++)
   {
      ID* pIconRsc = (ID*)SafeMalloc(sizeof(ID));
      Extract(&ptr, pIconRsc, SIZE_ID);
      len -= SIZE_ID;
      shield_list = list_add_item(shield_list, pIconRsc);
   }

   if (len != 0)
   {
      list_destroy(shield_list);
      return False;
   }

   GuildGotShields(shield_list);

   return True;
}
/********************************************************************/
Bool HandleGuildHalls(char *ptr, long len)
{
   char *start = ptr;
   WORD num_halls;
   int i;
   GuildHall *halls;

   Extract(&ptr, &num_halls, 2);

   halls = (GuildHall *) SafeMalloc(num_halls * sizeof(GuildHall));

   for (i=0; i < num_halls; i++)
   {
      Extract(&ptr, &halls[i].id, SIZE_ID);
      Extract(&ptr, &halls[i].name_res, SIZE_ID);
      Extract(&ptr, &halls[i].cost, 4);
      Extract(&ptr, &halls[i].rent, 4);
   }

   len -= (ptr - start);
   if (len != 0)
      return False;

   GuildHallsReceived(num_halls, halls);

   return True;
}
/********************************************************************/
Bool HandleLookPlayer(char *ptr, long len)
{
   char description[MAXMESSAGE], fixed_string[MAXMESSAGE], url[MAX_URL + 1];
   char* desc = description;
   char* fixed = fixed_string;
   ID resource_id;
   BYTE flags;
   char *start = ptr;
   object_node obj;

   ExtractObject(&ptr, &obj);
   Extract(&ptr, &flags, 1);
   Extract(&ptr, &resource_id, SIZE_ID);

   len -= (ptr - start);

   // Player bio desc (editable section).
   /* Remove format string id # & other ids from length */
   if (!CheckServerMessage(&desc, &ptr, &len, resource_id))
      return False;

   // Get fixed string (extra info). This used to be sent as one string,
   // now we handle it as resources.
   //ExtractString(&ptr, len, fixed, MAXMESSAGE);
   //debug(("got \"%s\" for fixed\n", fixed));
   Extract(&ptr, &resource_id, SIZE_ID);
   len -= SIZE_ID;

   // See if we need to reorder the message.
   if (CheckMessageOrder(&ptr, &len, resource_id) < 0)
      return False;
   /* Remove format string id # & other ids from length */
   if (!CheckServerMessage(&fixed, &ptr, &len, resource_id))
      return False;

   // Get URL
   ExtractString(&ptr, len, url, MAX_URL);
   len -= (SIZE_STRING_LEN + strlen(url));
   if (len != 0)
      return False;

   DisplayDescription(&obj, flags, desc, fixed, url);
   ObjectDestroy(&obj);
   return True;
}
/********************************************************************/
Bool HandleSendQuit(char *ptr, long len)
{
  if (len != 0)
    return False;
  RequestQuit();
  return True;
}
/********************************************************************/
Bool HandleSpellSchools(char *ptr, long len)
{
  BYTE num;
  int i;
  ID *schools;
  char *start = ptr;
  
  Extract(&ptr, &num, 1);
  schools = (ID *) SafeMalloc(num * SIZE_ID);
  for (i=0; i < num; i++)
     Extract(&ptr, &schools[i], SIZE_ID);

  SpellsGotSchools(num, schools);

  len -= (ptr - start);
  if (len != 0)
    return False;
  
  return True;
}
/****************************************************************************/
/*
 * EVENT_MOUSECLICK
 */
/****************************************************************************/
Bool WINAPI EventMouseClick(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   if (fDoubleClick)
      PerformAction(A_ACTIVATEMOUSE, NULL);

   // When button pressed, do both setting focus and selection (default is focus only)
   if (GameGetState() == GAME_SELECT)
   {
      SetFocus(cinfo->hMain);
      PerformAction(A_SELECT, NULL);
      return False;
   }

   return True;
}
/****************************************************************************/
/*
 * EVENT_USERACTION
 */
/****************************************************************************/
Bool WINAPI EventUserAction(int action, void *action_data)
{
   return InterfaceAction(action, action_data);
}
/****************************************************************************/
/*
 * EVENT_FONTCHANGED
 */
/****************************************************************************/
Bool WINAPI EventFontChanged(WORD font_id, LOGFONT *font)
{
   InterfaceFontChanged(font_id, font);
   return True;
}
/****************************************************************************/
/*
 * EVENT_COLORCHANGED
 */
/****************************************************************************/
Bool WINAPI EventColorChanged(WORD color_id, COLORREF color)
{
   InterfaceColorChanged(color_id, color);
   return True;
}
/****************************************************************************/
/*
 * EVENT_STATECHANGED
 */
/****************************************************************************/
Bool WINAPI EventStateChanged(int old_state, int new_state)
{
   if (new_state == GAME_INVALID)
   {
   }
   return True;
}
/****************************************************************************/
/*
 * EVENT_MENUITEM
 */
/****************************************************************************/
Bool WINAPI EventMenuItem(int id)
{
   // Check for spell menu items
   if (id >= ID_SPELL && id < ID_SPELL + MAX_SPELLS)
   {
      MenuSpellChosen(id);
      return False;  // Don't pass this menu item on
   }

   // Check for action menu items
   if (id >= ID_ACTION && id < ID_ACTION + MAX_ACTIONS)
   {
      MenuActionChosen(id);
      return False;  // Don't pass this menu item on
   }

   // Check for language menu items
   if (id >= ID_LANGUAGE && id < ID_LANGUAGE + MAX_LANGUAGE_ID)
   {
      MenuLanguageChosen(id);
      return False;
   }

   return True;
}
/****************************************************************************/
/*
 * EVENT_TEXTCOMMAND
 */
/****************************************************************************/
Bool WINAPI EventTextCommand(char *str)
{
   static BOOL bRecursing = FALSE;
   BOOL bReturn = TRUE;

   // Parse command as one of the real commands.
   if (ParseCommand(str, commands))
      return FALSE;

   // Parse command as verb alias.
   // Don't let an alias find another alias.
   //
   if (!bRecursing)
   {
      bRecursing = TRUE;

      bReturn = !ParseVerbAlias(str);

      bRecursing = FALSE;
   }

   // Return FALSE if we handled it and it shouldn't be further handled.
   //
   return bReturn;
}
/****************************************************************************/
/*
 * EVENT_RESIZE
 */
/****************************************************************************/
Bool WINAPI EventResize(int xsize, int ysize, AREA *view)
{
   InterfaceResizeModule(xsize, ysize, view);
   return True;
}
/****************************************************************************/
/*
 * EVENT_USERCHANGED
 */
/****************************************************************************/
Bool WINAPI EventUserChanged(void)
{
   InterfaceUserChanged();
   return True;
}
/****************************************************************************/
/*
 * EVENT_REDRAW
 */
/****************************************************************************/
Bool WINAPI EventRedraw(HDC hdc)
{
   InterfaceRedrawModule(hdc);
   return True;
}
/****************************************************************************/
/*
 * EVENT_DRAWITEM
 */
/****************************************************************************/
Bool WINAPI EventDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis)
{
   return InterfaceDrawItem(hwnd, lpdis);
}
/****************************************************************************/
/*
 * EVENT_RESETDATA
 */
/****************************************************************************/
Bool WINAPI EventResetData(void)
{
   InterfaceResetData();
   return True;
}
/****************************************************************************/
/*
 * EVENT_INVENTORY
 */
/****************************************************************************/
Bool WINAPI EventInventory(int command, void *data)
{
   switch (command)
   {
   case INVENTORY_SET:
      DisplayInventory(cinfo->player->inventory);
      StatsShowGroup( False );
      ShowInventory( True );
      DisplayInventoryAsStatGroup( (BYTE)STATS_INVENTORY );
      break;

   case INVENTORY_ADD:
      InventoryAddItem((object_node *) data);

      //DisplayInventory(cinfo->player->inventory);
      StatsShowGroup( False );
      ShowInventory( True );
      DisplayInventoryAsStatGroup( (BYTE)STATS_INVENTORY );

      break;

   case INVENTORY_REMOVE:
      InventoryRemoveItem((ID) data);
      break;

   case INVENTORY_USE:
      DisplaySetUsing((ID) data, True);
      break;

   case INVENTORY_UNUSE:
      DisplaySetUsing((ID) data, False);
      break;

   case INVENTORY_CHANGE:
      InventoryChangeItem((object_node *) data);
      break;

   case INVENTORY_USELIST:
      DisplayUsing((list_type) data);
      break;
   }
   return True;
}
/****************************************************************************/
/*
 * EVENT_ANIMATE
 */
/****************************************************************************/
Bool WINAPI EventAnimate(int dt)
{
   room_contents_node *r;

   if (cinfo->config->animate)
   {
      AnimateInventory(dt);
      AnimateEnchantments(dt);

      // If self is invisible, redraw self view to make it shimmer
      r = GetRoomObjectById(cinfo->player->id);
      if (r != NULL && r->obj.drawingtype == DRAWFX_INVISIBLE)
	 UserAreaRedraw();
   }
   return True;
}
/****************************************************************************/
/*
 * EVENT_SETCURSOR
 */
/****************************************************************************/
Bool WINAPI EventSetCursor(HCURSOR cursor)
{
   if (GameGetState() == GAME_PLAY && InventoryMouseCaptured())
   {
      SetMainCursor(LoadCursor(cinfo->hInst, MAKEINTRESOURCE(IDC_DROPCURSOR)));
      return False;
   }
   return True;
}
/****************************************************************************/
/*
 * EVENT_NEWROOM
 */
/****************************************************************************/
Bool WINAPI EventNewRoom(void)
{
   InterfaceNewRoom();
   return True;
}
/****************************************************************************/
/*
 * EVENT_CONFIGCHANGED
 */
/****************************************************************************/
Bool WINAPI EventConfigChanged(void)
{
   InterfaceConfigChanged();
   return True;
}

extern player_info *GetPlayerInfo(void);

player_info *GetPlayer(void)
{
   return GetPlayerInfo();
}
