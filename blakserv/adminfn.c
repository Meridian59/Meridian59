// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* adminfn.c
*

  This module has all the administrator functions.  These can be called from an
  account in admin mode (see admin.c) or from an account in game mode, if they
  are an administrator.

	The inputted command from admin or game mode should be sent to TryAdminCommand.
	Then, the results of the admin command are funneled through the aprintf function,
	which sends the text straight back to admin mode or encapsulates it into a game
	message.

	  The allowed commands are stored in nested tables that have function
	  pointers to the function to call and its parameters for a given
	  command.

		Admin mode is powerful, and much care has to be taken when doing stuff here
		because the session's state can change, and then our state data is no longer
		valid.

		  This is the largest source file in Blakserv.  It would be very nice if it
		  were shorter.

*/

#include "blakserv.h"

enum { N,S,I,R }; /* no more params, string param, int param, rest of line param */

enum
{
	A = 0x01,
		M = 0x02
}; /* admin mode can do vs. maintenance mode can do */

#define T True
#define F False

#define MAX_ADMIN_PARM 6
#define MAX_ADMIN_BLAK_PARM 10

typedef UINT64 admin_parm_type;

typedef struct admin_table_struct
{
	void (*admin_func)(int session_id, admin_parm_type parms[],
                      int num_blak_parm, parm_node blak_parms[]);
	int parm_type[MAX_ADMIN_PARM];
	Bool has_blak_parm;
	int permissions;
	struct admin_table_struct *sub_table;
	int len_sub_table;
	const char *admin_cmd;
	const char *help;
} admin_table_type;

void AdminSendBufferList(void);

void aprintf(const char *fmt,...);
void AdminBufferSend(char *buf,int len_buf);
void SendAdminBuffer(char *buf,int len_buf);

void DoAdminCommand(char *admin_command);
void AdminTable(int len_admin_table,admin_table_type command_table[],int session_id,
				char *command);
Bool AdminIsValidBlakParm(val_type check_val);

void AdminHelp(int session_id,int len_command_table,admin_table_type command_table[]);

void AdminGarbage(int session_id,admin_parm_type parms[],
                  int num_blak_parm,parm_node blak_parm[]);
void AdminSaveGame(int session_id,admin_parm_type parms[],
                   int num_blak_parm,parm_node blak_parm[]);
void AdminSaveConfiguration(int session_id,admin_parm_type parms[],
                            int num_blak_parm,parm_node blak_parm[]);
void AdminSaveOneConfigNode(config_node *c,const char *config_name,const char *default_str);
void AdminWho(int session_id,admin_parm_type parms[],
              int num_blak_parm,parm_node blak_parm[]);
void AdminWhoEachSession(session_node *s);
void AdminLock(int session_id,admin_parm_type parms[],
               int num_blak_parm,parm_node blak_parm[]);
void AdminUnlock(int session_id,admin_parm_type parms[],
                 int num_blak_parm,parm_node blak_parm[]);
void AdminMail(int session_id,admin_parm_type parms[],
               int num_blak_parm,parm_node blak_parm[]);
void AdminPage(int session_id,admin_parm_type parms[],
               int num_blak_parm,parm_node blak_parm[]);

void AdminShowStatus(int seFssion_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[]);
void AdminShowMemory(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[]);
void AdminShowCalled(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[]);
void AdminShowCalledClass(class_node *c);

void AdminShowObject(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[]);
void AdminShowObjects(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[]);
void AdminShowListNode(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[]);
void AdminShowList(int session_id,admin_parm_type parms[],
                   int num_blak_parm,parm_node blak_parm[]);
void AdminShowListParen(int session_id,int list_id,int new_start);
void AdminShowUsers(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[]);
void AdminShowUser(int session_id,admin_parm_type parms[],
                   int num_blak_parm,parm_node blak_parm[]);
void AdminShowUsage(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[]);
void AdminShowUserHeader(void);
void AdminShowOneUser(user_node *u);
void AdminShowAccounts(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[]);
void AdminShowAccount(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[]);
void AdminShowAccountHeader(void);
void AdminShowOneAccount(account_node *a);
void AdminShowResource(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[]);
void AdminPrintResource(resource_node *r);
void AdminShowDynamicResources(int session_id,admin_parm_type parms[],
                               int num_blak_parm,parm_node blak_parm[]);
void AdminShowTimers(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[]);
void AdminShowTimer(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[]);
void AdminShowTime(int session_id,admin_parm_type parms[],
                   int num_blak_parm,parm_node blak_parm[]);
void AdminShowOneTimer(timer_node *t);
void AdminShowConfiguration(int session_id,admin_parm_type parms[],
                            int num_blak_parm,parm_node blak_parm[]);
void AdminShowOneConfigNode(config_node *c,const char *config_name,const char *default_str);
void AdminShowString(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[]);
void AdminShowSysTimers(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[]);
void AdminShowEachSysTimer(systimer_node *st);
void AdminShowCalls(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[]);
void AdminShowMessage(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[]);
void AdminShowClass(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[]);
void AdminShowPackages(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[]);
void AdminShowOnePackage(dllist_node *dl);
void AdminShowConstant(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[]);
void AdminShowTransmitted(int session_id,admin_parm_type parms[],
                          int num_blak_parm,parm_node blak_parm[]);
void AdminShowTable(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[]);
void AdminShowName(int session_id,admin_parm_type parms[],
                   int num_blak_parm,parm_node blak_parm[]);
void AdminShowReferences(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[]);
void AdminShowReferencesEachObject(object_node *o);
void AdminShowReferencesEachList(int list_id);
void AdminShowExactInstances(int session_id, admin_parm_type parms[],
						int num_blak_parm, parm_node blak_parm[]);
void AdminShowInstances(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[]);
void AdminShowMatches(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[]);
void AdminShowProtocol(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[]);


void AdminSetClass(int session_id,admin_parm_type parms[],
                   int num_blak_parm,parm_node blak_parm[]);
void AdminSetObject(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[]);
void AdminSetAccountName(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[]);
void AdminSetAccountPassword(int session_id,admin_parm_type parms[],
                             int num_blak_parm,parm_node blak_parm[]);
void AdminSetAccountCredits(int session_id,admin_parm_type parms[],
                            int num_blak_parm,parm_node blak_parm[]);
void AdminSetAccountObject(int session_id,admin_parm_type parms[],
                           int num_blak_parm,parm_node blak_parm[]);
/*void AdminSetResource(int session_id,admin_parm_type parms[]);*/
void AdminSetConfigInt(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[]);
void AdminSetConfigBool(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[]);
void AdminSetConfigStr(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[]);

void AdminSuspendUser(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[]);
void AdminSuspendAccount(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[]);
void AdminUnsuspendUser(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[]);
void AdminUnsuspendAccount(int session_id,admin_parm_type parms[],
                           int num_blak_parm,parm_node blak_parm[]);

void AdminCreateAccount(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[]);
void AdminCreateAutomated(int session_id,admin_parm_type parms[],
                          int num_blak_parm,parm_node blak_parm[]);
void AdminRecreateAutomated(int session_id,admin_parm_type parms[],
                            int num_blak_parm,parm_node blak_parm[]);
void AdminCreateUser(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[]);
void AdminCreateAdmin(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[]);
void AdminCreateDM(int session_id,admin_parm_type parms[],
                   int num_blak_parm,parm_node blak_parm[]);
void AdminCreateObject(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[]);
void AdminCreateListNode(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[]);
void AdminCreateTimer(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[]);
void AdminCreateResource(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[]);

void AdminDeleteTimer(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[]);
void AdminDeleteAccount(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[]);
void AdminDeleteUser(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[]);
void AdminCheckUserLoggedOn(session_node *s);

void AdminSendObject(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[]);
void AdminSendUsers(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[]);
void AdminSendClass(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[]);

void AdminTraceOnMessage(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[]);
void AdminTraceOffMessage(int session_id,admin_parm_type parms[],
                          int num_blak_parm,parm_node blak_parm[]);

void AdminAddCredits(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[]);

void AdminKickoffAll(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[]);
void AdminKickoffEachSession(session_node *s);
void AdminKickoffAccount(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[]);

void AdminHangupAll(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[]);
void AdminHangupEachSession(session_node *s);
void AdminHangupUser(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[]);
void AdminHangupAccount(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[]);
void AdminHangupSession(int admin_session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[]);

void AdminBlockIP(int session_id,admin_parm_type parms[],
                  int num_blak_parm,parm_node blak_parm[]);

void AdminReloadSystem(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[]);
void AdminReloadGame(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[]);
void AdminReloadGameEachSession(session_node *s);
void AdminReloadMotd(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[]);
void AdminReloadPackages(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[]);

void AdminDisableSysTimer(int session_id,admin_parm_type parms[],
                          int num_blak_parm,parm_node blak_parm[]);

void AdminEnableSysTimer(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[]);

void AdminTerminateNoSave(int session_id,admin_parm_type parms[],
                          int num_blak_parm,parm_node blak_parm[]);
void AdminTerminateSave(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[]);

void AdminSay(int session_id,admin_parm_type parms[],
              int num_blak_parm,parm_node blak_parm[]);
void AdminSayEachAdminSession(session_node *s);

void AdminRead(int session_id,admin_parm_type parms[],
               int num_blak_parm,parm_node blak_parm[]);

void AdminMark(int session_id,admin_parm_type parms[],
               int num_blak_parm,parm_node blak_parm[]);

admin_table_type admin_show_table[] =
{
	{ AdminShowAccount,       {R,N}, F, A|M, NULL, 0, "account",
	"Show one account by account id or name" },
	{ AdminShowAccounts,      {N},   F, A, NULL, 0, "accounts",      "Show all accounts" },
	{ AdminShowObjects,       {I,N}, F, A|M, NULL, 0, "belong",       "Show objects belonging to id" },
	{ AdminShowCalled,        {I,N}, F, A, NULL, 0, "called",
     "Show top (int) called messages" },
	{ AdminShowCalls,         {I,N}, F, A, NULL, 0, "calls",         "Show top (int) C call counts" },
	{ AdminShowClass,         {S,N}, F,A|M, NULL, 0, "class",          "Show info about class" },
	{ AdminShowTime,          {N},   F, A|M, NULL, 0, "clock",        "Show current server time" },
	{ AdminShowConfiguration, {N},   F, A|M, NULL, 0, "configuration", "Show configuration values" },
	{ AdminShowConstant,      {S,N}, F,A|M, NULL, 0, "constant",       "Show value of admin constant" },
	{ AdminShowDynamicResources,{N}, F, A, NULL, 0, "dynamic",       "Show all dynamic resources" },
	{ AdminShowExactInstances,{S,N}, F, A, NULL, 0, "exactinstances", "Show all instances of class, excluding subclasses" },
	{ AdminShowInstances,     {S,N}, F, A, NULL, 0, "instances",     "Show all instances of class" },
	{ AdminShowList,          {I,N}, F, A|M, NULL, 0, "list",          "Traverse & show a list" },
	{ AdminShowListNode,      {I,N}, F, A|M, NULL, 0, "listnode",      "Show one list node by id" },
	{ AdminShowMatches,       {S,S,S,S,S,N}, F, A, NULL, 0, "matches",     "Show all instances of class which match criteria" },
	{ AdminShowMemory,        {N},   F, A|M, NULL, 0, "memory",        "Show system memory use" },
	{ AdminShowMessage,       {S,S,N},F,A|M, NULL, 0, "message",
	"Show info about class & message" },
	{ AdminShowName,          {R,N}, F, A|M, NULL, 0, "name",          "Show object of user name" },
	{ AdminShowObject,        {I,N}, F, A|M, NULL, 0, "object",        "Show one object by id" },
	{ AdminShowPackages,      {N},   F,A, NULL, 0, "packages",       "Show all packages loaded" },
	{ AdminShowProtocol,      {N},   F, A|M, NULL, 0, "protocol",      "Show protocol message counts" },
	{ AdminShowReferences,    {S,S,N}, F, A, NULL, 0, "references",
	"Show what objects or lists reference a particular data value" },
	{ AdminShowResource,      {S,N}, F, A|M, NULL, 0, "resource",
	"Show a resource by resource name" },
	{ AdminShowStatus,        {N},   F, A|M, NULL, 0, "status",        "Show system status" },
	{ AdminShowString,        {I,N}, F, A|M, NULL, 0, "string",        "Show one string by string id" },
	{ AdminShowSysTimers,     {N},   F, A, NULL, 0, "systimers",     "Show system timers" },
	{ AdminShowTable,         {I,N}, F, A|M, NULL, 0, "table",         "Show a hash table" },
	{ AdminShowTimer,         {I},   F, A|M, NULL, 0, "timer",        "Show one timer by id" },
	{ AdminShowTimers,        {N},   F, A, NULL, 0, "timers",        "Show all timers" },
	{ AdminShowTransmitted,   {N},   F,A, NULL, 0, "transmitted",
	"Show # of bytes transmitted in last minute" },
	{ AdminShowUsage,         {N},   F,A|M,NULL, 0, "usage",         "Show current usage" },
	{ AdminShowUser,          {R,N}, F, A|M, NULL, 0, "user",          "Show one user by name or object id" },
	{ AdminShowUsers,         {N},   F, A, NULL, 0, "users",         "Show all users" },
};
#define LEN_ADMIN_SHOW_TABLE (sizeof(admin_show_table)/sizeof(admin_table_type))

admin_table_type admin_setacco_table[] =
{
	{ AdminSetAccountCredits,  {I,I,N}, F, A, NULL, 0, "credits",
	"Set an account's number of credits--use Add Account Credit instead" },
	{ AdminSetAccountName,     {I,R,N}, F, A|M, NULL, 0, "name",
		"Set account name by account number and password" },
	{ AdminSetAccountObject,   {I,I,N}, F, A|M, NULL, 0, "object",
	"Set an object to be the game object for an account, i.e., a character" },
	{ AdminSetAccountPassword, {I,S,N}, F,A|M, NULL, 0, "password",
	"Set password by account number and password" },
};
#define LEN_ADMIN_SETACCO_TABLE (sizeof(admin_setacco_table)/sizeof(admin_table_type))

admin_table_type admin_suspend_table[] =
{
	{ AdminSuspendAccount,  {I,R,N}, F, A|M, NULL, 0, "account",
	"Suspends account by name or id for number of hours" },
	{ AdminSuspendUser,     {I,R,N}, F, A|M, NULL, 0, "user",
		"Suspends account by user name for number of hours" },
};
#define LEN_ADMIN_SUSPEND_TABLE (sizeof(admin_suspend_table)/sizeof(admin_table_type))

admin_table_type admin_unsuspend_table[] =
{
	{ AdminUnsuspendUser,     {R,N}, F, A|M, NULL, 0, "user",
		"Unsuspends account by user name immediately" },
	{ AdminUnsuspendAccount,  {R,N}, F, A|M, NULL, 0, "account",
	"Unsuspends account by name or id immediately" },
};
#define LEN_ADMIN_UNSUSPEND_TABLE (sizeof(admin_unsuspend_table)/sizeof(admin_table_type))

admin_table_type admin_setcfg_table[] =
{
	{ AdminSetConfigBool,     {S,S,S,N}, F, A|M, NULL, 0, "boolean",
	"Set a modifiable boolean configuration parameter by [group], name, yes/no" },
	{ AdminSetConfigInt,     {S,S,I,N}, F, A|M, NULL, 0, "integer",
		"Set a modifiable integer configuration parameter by [group], name, new integer value" },
	{ AdminSetConfigStr,      {S,S,R,N}, F, A|M, NULL, 0, "string",
	"Set a modifiable string configuration parameter by [group], name, new string value" },
};
#define LEN_ADMIN_SETCFG_TABLE (sizeof(admin_setcfg_table)/sizeof(admin_table_type))

admin_table_type admin_set_table[] =
{
	/*
	{ AdminSetResource,   {I,R,N},     F, A, NULL, 0, "resource",
	"Set a dynamic resource to have a different string/filename" },
	*/
	{ NULL, {N}, F, A, admin_setacco_table,  LEN_ADMIN_SETACCO_TABLE, "account",
	"Account subcommand" },
	{ AdminSetClass,      {S,S,S,S,N}, F, A|M, NULL, 0, "class", "Set classvar by name of class, name of var, and value" },
	{ NULL, {N}, F, A, admin_setcfg_table,   LEN_ADMIN_SETCFG_TABLE,  "config",
	"Config subcommand" },
	{ AdminSetObject,     {I,S,S,S,N}, F, A|M, NULL, 0, "object", "Set object by id single property" },
};
#define LEN_ADMIN_SET_TABLE (sizeof(admin_set_table)/sizeof(admin_table_type))

admin_table_type admin_create_table[] =
{
	{ AdminCreateAccount, {S,S,S,N}, F,A,NULL, 0, "account",
		"Create account by type (user/admin/dm), name, password" },
	{ AdminCreateAdmin,   {I,N},   F, A, NULL, 0, "admin",   "Create admin object by account id" },
	{ AdminCreateAutomated,{S,S,N},F,A|M,NULL, 0, "automated",
	"Create account and user by name, password" },
	{ AdminCreateDM,      {I,N},   F, A|M, NULL, 0, "dm",      "Create DM object by account id" },
	{ AdminCreateListNode,{S,S,S,S,N},F, A, NULL, 0, "listnode","Create list node" },
	{ AdminCreateObject,  {S,N},   T, A, NULL, 0, "object",  "Create object by class name and parms" },
	{ AdminCreateResource,{R,N},   F, A, NULL, 0, "resource","Create resource string" },
	{ AdminCreateTimer,   {I,S,I,N},F,A, NULL, 0, "timer","Create timer for obj id, message, milli" },
	{ AdminCreateUser,    {I,N},   F, A|M, NULL, 0, "user",    "Create user object by account id" },
};
#define LEN_ADMIN_CREATE_TABLE (sizeof(admin_create_table)/sizeof(admin_table_type))

admin_table_type admin_delete_table[] =
{
	{ AdminDeleteAccount, {I,N}, F,A|M,NULL, 0, "account","Delete account & user by ID" },
	{ AdminDeleteTimer,   {I,N}, F, A, NULL, 0, "timer",  "Delete timer by ID" },
	{ AdminDeleteUser,    {I,N}, F, A|M, NULL, 0, "user",   "Delete user by object ID" },
};
#define LEN_ADMIN_DELETE_TABLE (sizeof(admin_delete_table)/sizeof(admin_table_type))

admin_table_type admin_send_table[] =
{
	{ AdminSendObject,    {I,S,N}, T, A|M, NULL, 0, "object", "Send object by ID a message" },
	{ AdminSendUsers,     {R,N},   F, A|M, NULL, 0, "users",  "Send logged in people a system message" },
	{ AdminSendClass,     {S,S,N}, T, A|M, NULL, 0, "class",  "Send all objects of class a message" },
};
#define LEN_ADMIN_SEND_TABLE (sizeof(admin_send_table)/sizeof(admin_table_type))

admin_table_type admin_trace_table[] =
{
	{ AdminTraceOffMessage, {S,S,N}, F, A, NULL, 0, "off",
	"Stop tracing message by class & msg names " },
	{ AdminTraceOnMessage,  {S,S,N}, F, A, NULL, 0, "on",  "Trace message by class & message names " },
};
#define LEN_ADMIN_TRACE_TABLE (sizeof(admin_trace_table)/sizeof(admin_table_type))

admin_table_type admin_add_table[] =
{
	{ AdminAddCredits,   {I,I,N}, F, A, NULL, 0, "credits",
		"Add credits by account number and credits" },
};
#define LEN_ADMIN_ADD_TABLE (sizeof(admin_add_table)/sizeof(admin_table_type))

admin_table_type admin_kickoff_table[] =
{
	{ AdminKickoffAccount, {I,N},  F, A, NULL, 0, "account", "Kick one account out of the game" },
	{ AdminKickoffAll,     {N},    F, A, NULL, 0, "all", "Kick all users out of the game" },
};
#define LEN_ADMIN_KICKOFF_TABLE (sizeof(admin_kickoff_table)/sizeof(admin_table_type))

admin_table_type admin_hangup_table[] =
{
	{ AdminHangupAccount,  {R,N},  F, A|M, NULL, 0, "account", "Hangup one account" },
	{ AdminHangupAll,      {N},    F, A|M, NULL, 0, "all", "Hangup all users" },
	{ AdminBlockIP,        {R,N},  F, A|M, NULL, 0, "ip", "Block an IP address (temporarily)" },
	{ AdminHangupSession,  {I,N},  F, A, NULL, 0, "session", "Hangup one session" },
	{ AdminHangupUser,     {R,N},  F, A|M, NULL, 0, "user", "Hangup one user" },
};
#define LEN_ADMIN_HANGUP_TABLE (sizeof(admin_hangup_table)/sizeof(admin_table_type))

admin_table_type admin_reload_table[] =
{
	{ AdminReloadGame,     {I,N}, F, A|M, NULL, 0, "game",   "Reload game from any save time (0 for last)" },
	{ AdminReloadMotd,     {N},   F, A|M, NULL, 0, "motd",   "Reload message of the day from file" },
	{ AdminReloadPackages, {N},   F, A|M, NULL, 0, "packages","Rescan upload directory for packages" },
	{ AdminReloadSystem,   {N},   F, A|M, NULL, 0, "system", "Save game and reload all kod, motd" },
};
#define LEN_ADMIN_RELOAD_TABLE (sizeof(admin_reload_table)/sizeof(admin_table_type))

admin_table_type admin_recreate_table[] =
{
	{ AdminRecreateAutomated,{I,S,S,N},F,A|M,NULL, 0, "automated",
		"Create specific account and user by name, password" },
};
#define LEN_ADMIN_RECREATE_TABLE (sizeof(admin_recreate_table)/sizeof(admin_recreate_table))

admin_table_type admin_disable_table[] =
{
	{ AdminDisableSysTimer,{I,N}, F, A, NULL, 0, "systimer","Disable a system timer" },
};
#define LEN_ADMIN_DISABLE_TABLE (sizeof(admin_disable_table)/sizeof(admin_table_type))

admin_table_type admin_enable_table[] =
{
	{ AdminEnableSysTimer,{I,N}, F, A, NULL, 0, "systimer","Enable a system timer" },
};
#define LEN_ADMIN_ENABLE_TABLE (sizeof(admin_enable_table)/sizeof(admin_table_type))

admin_table_type admin_terminate_table[] =
{
	{ AdminTerminateNoSave,{N},   F, A|M, NULL, 0, "nosave", "Shut down the server without saving" },
	{ AdminTerminateSave,  {N},   F, A|M, NULL, 0, "save",   "Save game and shut down the server" },
};
#define LEN_ADMIN_TERMINATE_TABLE (sizeof(admin_terminate_table)/sizeof(admin_table_type))

admin_table_type admin_save_table[] =
{
	{ AdminSaveConfiguration,{N},F, A|M, NULL, 0, "configuration","Save blakserv.cfg" },
	{ AdminSaveGame,      {N},   F, A|M, NULL, 0, "game",    "Save game (will garbage collect first)" },
};
#define LEN_ADMIN_SAVE_TABLE (sizeof(admin_save_table)/sizeof(admin_table_type))

admin_table_type admin_main_table[] =
{
	{ NULL, {N}, F, A, admin_add_table,    LEN_ADMIN_ADD_TABLE,    "add",    "Add subcommand" },
	{ NULL, {N}, F, A, admin_create_table, LEN_ADMIN_CREATE_TABLE, "create", "Create subcommand" },
	{ NULL, {N}, F, A, admin_delete_table, LEN_ADMIN_DELETE_TABLE, "delete", "Delete subcommand" },
	{ NULL, {N}, F, A, admin_disable_table,LEN_ADMIN_DISABLE_TABLE,"disable", "Disable subcommand" },
	{ NULL, {N}, F, A, admin_enable_table, LEN_ADMIN_ENABLE_TABLE, "enable", "Enable subcommand" },
	{ AdminGarbage,       {N},   F, A, NULL, 0, "garbage",   "Garbage collect" },
	{ NULL, {N}, F, A, admin_hangup_table, LEN_ADMIN_HANGUP_TABLE, "hangup", "Hangup subcommand" },
	{ AdminLock,          {R,N}, F, A|M, NULL, 0, "lock",      "Lock the game" },
	{ NULL, {N}, F, A, admin_kickoff_table,LEN_ADMIN_KICKOFF_TABLE,"kickoff","Kickoff subcommand" },
	{ AdminMail,          {N},   F, A, NULL, 0, "mail",      "Read administrator mail" },
	{ AdminMark,          {N},   F, A|M, NULL, 0, "mark",      "Mark all channel logs with a dashed line" },
	{ AdminPage,          {N},   F, A, NULL, 0, "page",      "Page the console" },
	{ AdminRead,          {S,N}, F, A|M, NULL, 0, "read",      "Read admin commands from a file, echoes everything" },
	{ NULL, {N}, F, A, admin_recreate_table,LEN_ADMIN_RECREATE_TABLE, "recreate", "Recreate subcommand" },
	{ NULL, {N}, F, A, admin_reload_table, LEN_ADMIN_RELOAD_TABLE, "reload", "Reload subcommand" },
	{ NULL, {N}, F, A, admin_save_table,   LEN_ADMIN_SAVE_TABLE,   "save",   "Save subcommand" },
	{ AdminSay,           {R,N}, F, A|M, NULL, 0, "say",       "Say text to all admins logged in" },
	{ NULL, {N}, F, A, admin_send_table,   LEN_ADMIN_SEND_TABLE,   "send",   "Send subcommand" },
	{ NULL, {N}, F, A, admin_set_table,    LEN_ADMIN_SET_TABLE,    "set",    "Set subcommand" },
	{ NULL, {N}, F, A, admin_show_table,   LEN_ADMIN_SHOW_TABLE,   "show",   "Show subcommand" },
	{ NULL, {N}, F, A, admin_suspend_table, LEN_ADMIN_SUSPEND_TABLE,"suspend", "Suspend subcommand" },
	{ NULL, {N}, F, A, admin_terminate_table,LEN_ADMIN_TERMINATE_TABLE,"terminate",
	"Terminate subcommand" },
	{ NULL, {N}, F, A, admin_trace_table,  LEN_ADMIN_TRACE_TABLE,  "trace",  "Trace subcommand" },
	{ AdminUnlock,        {N},   F, A|M, NULL, 0, "unlock",    "Unlock the game" },
	{ NULL, {N}, F, A, admin_unsuspend_table, LEN_ADMIN_UNSUSPEND_TABLE,"unsuspend", "Unsuspend subcommand" },
	{ AdminWho,           {N},   F, A|M, NULL, 0, "who",       "Show every account logged on" },
};

#define LEN_ADMIN_MAIN_TABLE (sizeof(admin_main_table)/sizeof(admin_table_type))

int admin_session_id; /* set by TryAdminCommand each time */
static buffer_node *blist; /* same */

void aprintf(const char *fmt,...)
{
	char s[BUFFER_SIZE];
	va_list marker;

	va_start(marker,fmt);
	vsnprintf(s,sizeof(s),fmt,marker);
	va_end(marker);

	TermConvertBuffer(s,sizeof(s)); /* makes \n's into CR/LF pairs for edit boxes */

	AdminBufferSend(s, (int) strlen(s));
}

char *to_lowercase(char *s)
{
   char* p = s;
   while ((*p = tolower(*p))) p++;
   return s;
}

void AdminBufferSend(char *buf,int len_buf)
{
	blist = AddToBufferList(blist,buf,len_buf);
}

void AdminSendBufferList(void)
{
	session_node *s;
	buffer_node *bn;
	unsigned short len;

	s = GetSessionByID(admin_session_id);
	if (s == NULL)
		return; /* must have errored on earlier write and been hung up */

	if (s->state == STATE_GAME)
	{
		/* put in length now... kind of a hack */
		len = 0;
		bn = blist;

		while (bn != NULL)
		{
			len += bn->len_buf;
			bn = bn->next;
		}
		bn = GetBuffer();
		AddByteToBufferList(bn,(unsigned char)BP_ADMIN);
		AddToBufferList(bn,&len,2);
		bn->next = blist;
		blist = bn;

	}

	if (blist != NULL)
	{
		SecurePacketBufferList(admin_session_id,blist);
		SendClientBufferList(admin_session_id,blist);
	}

	blist = NULL;
}

/* SendAdminBuffer
Sends a block o' bytes to admin_session_id, but not using buffer pool.  Used
only for say and trace. */
void SendAdminBuffer(char *buf,int len_buf)
{
	session_node *session;

	if (len_buf > BUFFER_SIZE)
	{
		eprintf("SendAdminBuffer sent only first %lu bytes of requested buffer,\n",BUFFER_SIZE);
		len_buf = BUFFER_SIZE;
	}
	else if (len_buf < 0 || !buf)
	{
		eprintf("SendAdminBuffer got invalid buffer\n");
		return;
	}

	session = GetSessionByID(admin_session_id);
	if (session == NULL)
		return; /* must have errored on earlier write and been hung up */

	switch (session->state)
	{
	case STATE_ADMIN :
		SendClient(admin_session_id,buf,(unsigned short)len_buf);
		break;
	case STATE_GAME :
		AddByteToPacket(BP_ADMIN);
		AddStringToPacket(len_buf,buf);
		SendPacket(admin_session_id);
		break;
	default :
		eprintf("SendAdminBuffer called, SESSION %lu state %lu is not admin or game\n",
			session->session_id,session->state);
	}
}

/* SendSessionAdminText
This can be called from any module to asynchronously send
admin text. Currently only used for trace info and say. */
void SendSessionAdminText(int session_id,const char *fmt,...)
{
	int prev_admin_session_id;

	char s[BUFFER_SIZE];
	va_list marker;

	va_start(marker,fmt);
	vsnprintf(s,sizeof(s),fmt,marker);
	va_end(marker);

	prev_admin_session_id = admin_session_id;

	admin_session_id = session_id;

	TermConvertBuffer(s,sizeof(s)); /* makes \n's into CR/LF pairs for edit boxes */
	SendAdminBuffer(s, (int) strlen(s));

	admin_session_id = prev_admin_session_id;
}

void TryAdminCommand(int session_id,char *admin_command)
{
	session_node *s;

	s = GetSessionByID(session_id);
	if (s == NULL)
	{
		eprintf("TryAdminCommand got invalid SESSION %lu\n",session_id);
		return;
	}

	admin_session_id = session_id;

	if (blist != NULL)
		eprintf("TryAdminCommand entered with blist = %08x!\n",blist);

	blist = NULL;

	if (s->account == NULL)
	{
		/* dprintf(" Maintenance %s\n",admin_command); */
	}
	else
		dprintf(" %2i %s\n",s->account->account_id,admin_command);

	DoAdminCommand(admin_command);
}

void DoAdminCommand(char *admin_command)
{
	char *cmd;

	cmd = strtok(admin_command," \t\n");

	if (cmd != NULL)
	{
		AdminTable(LEN_ADMIN_MAIN_TABLE,admin_main_table,admin_session_id,cmd);
		AdminSendBufferList();
	}

}

void AdminTable(int len_command_table,admin_table_type command_table[],int session_id,
				char *command)
{
	int mode_type,i,index,num_parms,num,num_blak_parm;
	char *parm_str,*prev_tok;
	admin_parm_type admin_parm[MAX_ADMIN_PARM];
	parm_node blak_parm[MAX_ADMIN_BLAK_PARM];
	val_type blak_val;
	session_node *s;

	s = GetSessionByID(session_id);
	if (s == NULL)
	{
     eprintf("AdminTable got invalid SESSION %lu\n",session_id);
     return;
	}
	if (command == NULL || !stricmp(command,"HELP") || !stricmp(command,"?"))
	{
			AdminHelp(session_id,len_command_table,command_table);
      return;
	}

	index = -1;
	for (i=0;i<len_command_table;i++)
	{
		to_lowercase(command);
		if (strstr(command_table[i].admin_cmd,command)
			== command_table[i].admin_cmd)
		{
			if (stricmp(command_table[i].admin_cmd,command) == 0)
         {
            // Exact match; stop looking
            index = i;
            break;
         }

         if (index != -1)
         {
            aprintf("Ambiguous command; try 'help'.\n");
            return;
			}
			index = i;
		}
	}

	if (index == -1)
	{
		aprintf("Unknown command; try 'help'.\n");
		return;
	}

	if (command_table[index].admin_func == NULL)
	{
		/* we've got a subtable */

		AdminTable(command_table[index].len_sub_table,
			command_table[index].sub_table,session_id,
			strtok(NULL," \t\n"));
		return;
	}


	/* we've got a function call.  check permissions */

	mode_type = 0;
	if (s->state == STATE_MAINTENANCE)
		mode_type |= M;
	if (s->state == STATE_ADMIN || (s->state == STATE_GAME &&
		(s->account->type == ACCOUNT_DM ||
				    s->account->type == ACCOUNT_ADMIN)))
					mode_type |= A;

	if ((command_table[index].permissions & mode_type) == 0)
	{
		aprintf("You do not have access to this command.\n");
		return;
	}

	/* check parameters */

	for (i=0;i<MAX_ADMIN_PARM;i++)
	{
		if (command_table[index].parm_type[i] == N)
			break;
	}
	num_parms = i;

	prev_tok = command;

	i = 0;
	while (i < num_parms)
	{
		if (command_table[index].parm_type[i] != R) /* for rest of line char (R), don't mess it up*/
		{
			parm_str = strtok(NULL," \t\n");
			if (parm_str == NULL)
			{
				aprintf("Missing parameter %lu.\n",i+1);
				return;
			}
			prev_tok = parm_str;
		}
		switch (command_table[index].parm_type[i])
		{
		case S :
			admin_parm[i] = (admin_parm_type)parm_str;
			break;
		case I :
			if (sscanf(parm_str,"%d",&num) != 1)
			{
				aprintf("Parameter %d should be an int, not '%s'.\n",
					i+1,parm_str);
				return;
			}
			admin_parm[i] = (admin_parm_type)num;
			break;
		case R :
			/* remember how strtok works to see why this works */
			admin_parm[i] = (admin_parm_type) (prev_tok + strlen(prev_tok) + 1);
			/* now make sure no more params */
			prev_tok = NULL;
			break;
		}
		i++;
	}

	if (command_table[index].has_blak_parm)
	{
		i = 0;
		while ((parm_str = strtok(NULL," \t\n")) != NULL)
		{
			blak_parm[i].type = CONSTANT;
			blak_parm[i].name_id = GetIDByName(parm_str);
			if (blak_parm[i].name_id == INVALID_ID)
			{
				aprintf("'%s' is not a valid parameter.\n",parm_str);
				return;
			}

			parm_str = strtok(NULL," \t\n");
			if (parm_str == NULL)
			{
				aprintf("Blakod parameter %lu needs tag and value.\n",i+1);
				return;
			}
			num = GetTagNum(parm_str);
			if (num == INVALID_TAG)
			{
				aprintf("Blakod parameter %lu has invalid tag.\n",i+1);
				return;
			}
			blak_val.v.tag = num;

			if( num == TAG_TEMP_STRING )	// Added string ("quote") support -- AJM
			{
				char *text;
				parm_str = strtok( NULL, "" );	// Get rest of line for "quote" type parameter
				if (parm_str == NULL)
				{
					aprintf("Blakod parameter %lu needs value\n",i+1);
					return;
				}

				text = (char *) parm_str;
				SetTempString( text, (int) strlen( text ) );  // Copies to global temp_str

				// we need to set type, name_id, and value
				//	type is CONSTANT (set above)
				//	name_id is the blakod message parameter name (also set above)
				//	value is the union of blak_val.v.tag (set to TAG_TEMP_STRING) and blak_val.v.data
				blak_val.v.data = 0;		/* doesn't matter for TAG_TEMP_STRING */
				blak_parm[i].value = blak_val.int_val;

				i++;	// increment to keep count correct
				break;	// bail out of the parse loop
			}
			else
			{
				parm_str = strtok(NULL," \t\n");
				if (parm_str == NULL)
				{
					aprintf("Blakod parameter %lu needs value.\n",i+1);
					return;
				}
				if (0 == stricmp("SELF", parm_str) && blak_val.v.tag == TAG_OBJECT &&
					s && s->state == STATE_GAME && s->game && s->game->object_id != INVALID_OBJECT)
				{
					num = s->game->object_id;
				}
				else if (LookupAdminConstant(parm_str,&num) == False)
				{
					bool negate = false;

					// INT parameters may have a negative number and still be legal
					if (blak_val.v.tag == TAG_INT && *parm_str == '-')
					{
						negate = true;
						parm_str++;
					}

					num = GetDataNum(blak_val.v.tag,parm_str);
					if (num == INVALID_DATA)
					{
						aprintf("Blakod parameter %lu has invalid data.\n",i+1);
						return;
					}

					if (negate)
						num = -num;
				}
				blak_val.v.data = num;

				if (AdminIsValidBlakParm(blak_val) == False)
				{
					aprintf("Blakod parameter %lu references invalid data.\n",i+1);
					return;
				}

				blak_parm[i].value = blak_val.int_val;
				i++;
			}

		}	// end while( next parm != NULL )
		num_blak_parm = i;

	}	// end if has blakparm

	if (prev_tok != NULL && strtok(NULL," \t\n") != NULL)
	{
		aprintf("Too many parameters, command ignored.\n");
		return;
	}
	if (command_table[index].has_blak_parm)
		command_table[index].admin_func(session_id,admin_parm,num_blak_parm,blak_parm);
	else
		command_table[index].admin_func(session_id,admin_parm, 0, blak_parm);
}

Bool AdminIsValidBlakParm(val_type check_val)
{
	switch (check_val.v.tag)
	{
	case TAG_OBJECT :
		return IsObjectByID(check_val.v.data);
	case TAG_STRING :
		return IsStringByID(check_val.v.data);
	case TAG_LIST :
		return IsListNodeByID(check_val.v.data);
	case TAG_RESOURCE :
		return IsResourceByID(check_val.v.data);
	case TAG_NIL :
		return check_val.v.data == 0;
	case TAG_TEMP_STRING :
		return True;		// "quote" type parm: Assume any (non-null) string is valid
	}

	return True;
}

void AdminHelp(int session_id,int len_command_table,admin_table_type command_table[])
{
	int i,j;
	Bool done_parm;

	for (i=0;i<len_command_table;i++)
	{
		aprintf("%-10s ",command_table[i].admin_cmd);

		done_parm = False;
		for (j=0;j<MAX_ADMIN_PARM;j++)
		{
			if (command_table[i].parm_type[j] == N)
				done_parm = True;
			if (done_parm)
				aprintf(" ");
			else
				switch (command_table[i].parm_type[j])
			{
		case S : aprintf("S"); break;
		case I : aprintf("I"); break;
		case R : aprintf("R"); break;
			}
		}
		if (command_table[i].has_blak_parm)
			aprintf(" P");
		else
			aprintf("  ");
		aprintf(" %s\n",command_table[i].help);
	}
}

void AdminTerminateNoSave(int session_id,admin_parm_type parms[],
                          int num_blak_parm,parm_node blak_parm[])
{
	aprintf("Terminating server. All connections, "
		"including yours, about to be lost...\n");

	SetQuit();
}

void AdminTerminateSave(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[])
{
	lprintf("AdminTerminateSave saving\n");

	aprintf("Garbage collecting and saving game... ");

	SendBlakodBeginSystemEvent(SYSEVENT_SAVE);
	GarbageCollect();
	SaveAll();
	AllocateParseClientListNodes(); /* it needs a list to send to users */
	SendBlakodEndSystemEvent(SYSEVENT_SAVE);

	aprintf("done.\n");

	aprintf("Terminating server. All connections, "
		"including yours, about to be lost\n");

	SetQuit();
}

void AdminGarbage(int session_id,admin_parm_type parms[],
                  int num_blak_parm,parm_node blak_parm[])
{
	lprintf("AdminGarbage garbage collecting\n");

	PauseTimers();
	aprintf("Garbage collecting... ");
	AdminSendBufferList();

	SendBlakodBeginSystemEvent(SYSEVENT_GARBAGE);

	GarbageCollect();
	AllocateParseClientListNodes(); /* it needs a list to send to users */
	SendBlakodEndSystemEvent(SYSEVENT_GARBAGE);
	aprintf("done.\n");
	UnpauseTimers();

	ResetBufferPool();
}

void AdminSaveGame(int session_id,admin_parm_type parms[],
                   int num_blak_parm,parm_node blak_parm[])
{
	INT64 save_time;

	lprintf("AdminSave saving\n");

	PauseTimers();
	aprintf("Garbage collecting and saving game... ");
	AdminSendBufferList();

	SendBlakodBeginSystemEvent(SYSEVENT_SAVE);

	GarbageCollect();
	save_time = SaveAll();
	AllocateParseClientListNodes(); /* it needs a list to send to users */

	SendBlakodEndSystemEvent(SYSEVENT_SAVE);

	aprintf("done.  Save time is (%lli).\n", save_time);
	UnpauseTimers();
}

/* data for ForEachConfigNode */
static FILE *configfile;
void AdminSaveConfiguration(int session_id,admin_parm_type parms[],
                            int num_blak_parm,parm_node blak_parm[])
{

	if ((configfile = fopen(CONFIG_FILE,"wt")) == NULL)
	{
		eprintf("AdminSaveConfiguration can't open %s to save config.\n",CONFIG_FILE);
		return;
	}

	fprintf(configfile,"# %s\n",BlakServLongVersionString());
	fprintf(configfile,"# Configuration file automatically generated at %s\n",
		TimeStr(GetTime()));
	fprintf(configfile,"# -------------------------------------------\n");


	ForEachConfigNode(AdminSaveOneConfigNode);
	fclose(configfile);

	aprintf("Configuration saved.\n");
}

void AdminSaveOneConfigNode(config_node *c,const char *config_name,const char *default_str)
{
	/* print out non-default config data */
	switch (c->config_type)
	{
	case CONFIG_GROUP :
		fprintf(configfile,"\n");
		fprintf(configfile,"%-20s ",config_name);
		fprintf(configfile,"\n");
		break;
	case CONFIG_STR :
		if (strcmp(c->config_str_value,default_str) != 0)
		{
			fprintf(configfile,"%-20s ",config_name);
			if (strchr(c->config_str_value,' ') != NULL)
				fprintf(configfile,"<%s>\n",c->config_str_value);
			else
				fprintf(configfile,"%s\n",c->config_str_value);
		}
		break;
	case CONFIG_INT :
		if (atoi(default_str) != c->config_int_value)
		{
			fprintf(configfile,"%-20s ",config_name);
			fprintf(configfile,"%i\n",c->config_int_value);
		}
		break;
	case CONFIG_PATH :
		if (strcmp(c->config_str_value,default_str) != 0)
		{
			fprintf(configfile,"%-20s ",config_name);
			fprintf(configfile,"%s\n",c->config_str_value);
		}
		break;
	case CONFIG_BOOL :
		if ((stricmp(default_str,"Yes") == 0) ^ c->config_int_value)
		{
			fprintf(configfile,"%-20s ",config_name);
			if (c->config_int_value != False)
				fprintf(configfile,"Yes\n");
			else
				fprintf(configfile,"No\n");
		}
		break;
	default :
		fprintf(configfile,"%-20s ",config_name);
		fprintf(configfile,"# Unknown type\n");
		break;
	}
}

static int admin_who_count;
void AdminWho(int session_id,admin_parm_type parms[],
              int num_blak_parm,parm_node blak_parm[])
{
	aprintf("\n");
	aprintf("Name                Act Ver Sess Port               Where\n");
	aprintf("--------------------------------------------------------"
		"--------------------\n");

	admin_who_count = 0;
	ForEachSession(AdminWhoEachSession);
}

void AdminWhoEachSession(session_node *s)
{
	const char *str;

	if (s->conn.type == CONN_CONSOLE)
		return;

	if (s->account != NULL)
		str = s->account->name;
	else
		str = "?";

	aprintf("%-18.18s ",str);

	if (s->account != NULL)
		aprintf("%4i",s->account->account_id);
	else
		aprintf("    ");

	if (s->blak_client)
		aprintf("%4i",s->version_major*100+s->version_minor);
	else
		aprintf(" %-3s","No");

	aprintf(" %4i ",s->session_id);
	aprintf("%-18.18s ",s->conn.name);

	aprintf("%s",GetStateName(s));

	if (s->state == STATE_GAME)
	{
		val_type name_val;
		resource_node *r;

		if (s->game->object_id != INVALID_OBJECT)
		{
			aprintf(" - ");

			name_val.int_val = SendTopLevelBlakodMessage(s->game->object_id,USER_NAME_MSG,0,NULL);
			if (name_val.v.tag == TAG_RESOURCE)
			{
				r = GetResourceByID(name_val.v.data);
				if (r == NULL)
					aprintf("Invalid resource id %i",name_val.v.data);
				else
					aprintf("%s",r->resource_val);
			}
			else
				aprintf("Non-resource %i,%i",name_val.v.tag,name_val.v.data);
			aprintf(" (%i)",s->game->object_id);
		}
	}
	aprintf("\n");
	admin_who_count++;
	if ((admin_who_count % 40) == 0)
		AdminSendBufferList();

}

void AdminLock(int session_id,admin_parm_type parms[],
               int num_blak_parm,parm_node blak_parm[])
{
	char *ptr;
	char *lockstr;

	char *text;
	text = (char *)parms[0];

	lockstr = ConfigStr(LOCK_DEFAULT);

	ptr = text;
	while (*ptr != 0)
	{
		if (*ptr != ' ' && *ptr != '\n' && *ptr != '\t')
		{
			lockstr = text;
			break;
		}
		ptr++;
	}
	aprintf("Locking game <%s>.\n",lockstr);
	lprintf("AdminLock locking game (%s).\n",lockstr);
	SetGameLock(lockstr);
	InterfaceUpdate();
}

void AdminUnlock(int session_id,admin_parm_type parms[],
                 int num_blak_parm,parm_node blak_parm[])
{
	if (!IsGameLocked())
	{
		aprintf("Game isn't locked.\n");
		return;
	}

	aprintf("Unlocking game.\n");
	lprintf("AdminUnlock unlocking game.\n");
	SetGameUnlock();
	InterfaceUpdate();
}

void AdminMail(int session_id,admin_parm_type parms[],
               int num_blak_parm,parm_node blak_parm[])
{
	enum { MAIL_BUFSIZE = 400 }; /* because we aprintf this buffer, keep small */

	int infile, numread;
	char loadname[MAX_PATH+FILENAME_MAX];
	char buf[MAIL_BUFSIZE + 2];

	sprintf(loadname, "%s%s",ConfigStr(PATH_FORMS),NOTE_FILE);

	if ((infile = open(loadname, O_RDONLY | O_TEXT)) == -1)
	{
		aprintf("Couldn't open mail file.\n");
		return;
	}

	buf[MAIL_BUFSIZE+1] = 0;
	do
	{
		numread = read(infile, buf, MAIL_BUFSIZE);
		buf[numread] = 0;
		aprintf("%s",buf);
	} while (numread > 0);

	close(infile);
}

void AdminPage(int session_id,admin_parm_type parms[],
               int num_blak_parm,parm_node blak_parm[])
{
	session_node *s;

	s = GetSessionByID(session_id);
	if (s == NULL)
		eprintf("AdminPage admin SESSION %i has no session!",session_id);
	else
		if (s->account != NULL)
			lprintf("AdminPage %s paged the console\n",s->account->name);

		InterfaceSignalConsole();
}


void AdminShowStatus(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[])
{
	kod_statistics *kstat;
	object_node *o;
	class_node *c;
	const char *m;
	INT64 now = GetTime();

	aprintf("System Status -----------------------------\n");

	aprintf("%s on %s\n",BlakServLongVersionString(),LockConfigStr(EMAIL_LOCAL_MACHINE_NAME));
	UnlockConfigStr();

	kstat = GetKodStats();

	aprintf("Current time is %s\n",TimeStr(now));
	aprintf("System started at %s (up for %s = %lli seconds)\n",
		TimeStr(kstat->system_start_time),
          RelativeTimeStr((int) (now - kstat->system_start_time)),
		now - kstat->system_start_time);

	aprintf("----\n");
	aprintf("Interpreted %i.%09i billion total instructions in %lli seconds\n",
		kstat->billions_interpreted,kstat->num_interpreted,
		kstat->interpreting_time/1000);
	aprintf("Handled %i top level messages, total %i messages\n",
		kstat->num_top_level_messages, kstat->num_messages);
	aprintf("Deepest message call stack is %i calls from top level\n",kstat->message_depth_highest);
	aprintf("Most instructions on one top level message is %i instructions\n",kstat->num_interpreted_highest);
	aprintf("Number of top level messages over 1000 milliseconds is %i\n",kstat->interpreting_time_over_second);
	aprintf("Longest time on one top level message is %i milliseconds\n",kstat->interpreting_time_highest);

	if (kstat->interpreting_time_object_id != INVALID_ID)
	{
		o = GetObjectByID(kstat->interpreting_time_object_id);
		c = o? GetClassByID(o->class_id) : NULL;
		m = GetNameByID(kstat->interpreting_time_message_id);
		aprintf("Most recent slow top level message is OBJECT %i CLASS %s MESSAGE %s\n",
			kstat->interpreting_time_object_id,
			(char*)(c? c->class_name : "(unknown)"),
			(char*)(m? m : "(unknown)"));
		aprintf("Most recent slow top level message includes %i posted followups\n",
			kstat->interpreting_time_posts);
	}

	aprintf("----\n");
	aprintf("Active accounts: %i\n",GetActiveAccountCount());
	aprintf("Next account number is %i\n",GetNextAccountID());
	aprintf("Clients on port %i, maintenance on port %i\n",
		ConfigInt(SOCKET_PORT),
		ConfigInt(SOCKET_MAINTENANCE_PORT));
	aprintf("There are %i sessions logged on\n", GetUsedSessions());

	aprintf("----\n");
	aprintf("Used %i list nodes\n",GetListNodesUsed());
	aprintf("Used %i object nodes\n",GetObjectsUsed());
	aprintf("Used %i string nodes\n",GetStringsUsed());
	aprintf("Watching %i active timers\n",GetNumActiveTimers());

	if (IsGameLocked())
		aprintf("The game is LOCKED (%s)\n",GetGameLockedReason());

	aprintf("-------------------------------------------\n");
}

void AdminShowMemory(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[])
{
	memory_statistics *mstat;

	aprintf("System Memory -----------------------------\n");

	mstat = GetMemoryStats();

	size_t total = 0;

	aprintf("%s\n",TimeStr(GetTime()));
	for (int i=0;i<GetNumMemoryStats();i++)
	{
		aprintf("%-20s %8lu\n",GetMemoryStatName(i),mstat->allocated[i]);
		total += mstat->allocated[i];
	}
	aprintf("%-20s %4lu MB\n","-- Total",total/1024/1024);

	aprintf("-------------------------------------------\n");
}

static int show_messages_ignore_count;
static int show_messages_ignore_id;
static int show_messages_count;
static int show_messages_message_id;
static class_node * show_messages_class;
void AdminShowCalled(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[])
{
	int i;

	int num_show;
	num_show = (int)parms[0];

	num_show = std::max(1,num_show);
	num_show = std::min(500,num_show);

	aprintf("%4s %-22s %-22s %s\n","Rank","Class","Message","Count");

	show_messages_ignore_count = -1;
	show_messages_ignore_id = -1;
	for (i=0;i<num_show;i++)
	{
		show_messages_count = -1;
		ForEachClass(AdminShowCalledClass);
		if (show_messages_count <= 0)
			break;
		aprintf("%3i. %-22s %-22s %i\n",i+1,
			(show_messages_class == NULL)?("Unknown"):show_messages_class->class_name,
			GetNameByID(show_messages_message_id),show_messages_count);
		show_messages_ignore_count = show_messages_count;
		show_messages_ignore_id = show_messages_message_id;
	}

}

void AdminShowCalledClass(class_node *c)
{
	int i;

	for (i=0;i<c->num_messages;i++)
		if ((show_messages_ignore_count == -1 ||
			(c->messages[i].called_count < show_messages_ignore_count ||
			(c->messages[i].called_count == show_messages_ignore_count &&
			c->messages[i].message_id > show_messages_ignore_id))))
			if (c->messages[i].called_count > show_messages_count ||
				(c->messages[i].called_count == show_messages_count &&
				c->messages[i].message_id < show_messages_message_id))
			{
				show_messages_count = c->messages[i].called_count;
				show_messages_message_id = c->messages[i].message_id;
				show_messages_class = c;
			}
}

void AdminShowObjects(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[])
{
	object_node *o;
	class_node *c;
	int object_id;
	int howManyObjects ;
	int propId,propObjId ;
	int i;
	admin_parm_type lparm[MAX_ADMIN_PARM];

	object_id = (int)parms[0];

	if( IsObjectByID( object_id ) == False )
	{
		aprintf("Invalid object id %i (or it has been deleted).\n",
			object_id);
		return;
	}

	howManyObjects = GetObjectsUsed();

	aprintf("%d owns the following objects - \n",object_id);

	for(i=0;i<howManyObjects;i++) {
		/* attempt to get an object */
		o = GetObjectByIDQuietly( i ) ;
		if ( o != NULL ) {
			/* get the class for the object */
			c = GetClassByID(o->class_id);
			if( c!= NULL ) {
				/* attempt to get poOwner */
				propId = GetPropertyIDByName(c,"poOwner");
				if( propId != INVALID_PROPERTY ) {
					/* valid object, class and has poOwner property */
					propObjId = atol( GetDataName(o->p[ propId ].val) );
					if(propObjId == object_id ) {
						lparm[0] = (admin_parm_type) i;
						AdminShowObject(session_id,lparm, 0, NULL);
					}
				}
			}
		}
	}

}

void AdminShowObject(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[])
{
	object_node *o;
	class_node *c;
	int i;
	const char *prop_name;
	char buf[200];

	int object_id;
	object_id = (int)parms[0];

	if (IsObjectByID(object_id) == False)
	{
		aprintf("Invalid object id %i (or it has been deleted).\n",
			object_id);
		return;
	}

	o = GetObjectByID(object_id);
	if (o == NULL)
	{
		aprintf("Invalid object id %i (or it has been deleted).\n",
			object_id);
		return;
	}

	c = GetClassByID(o->class_id);
	if (c == NULL) /* can't ever be, charlie: yes it can..*/
	{
		aprintf("OBJECT %i has invalid class!\n",o->object_id);
		return;
	}

	aprintf(":< OBJECT %i is CLASS %s\n",o->object_id,c->class_name);
	/* = is ok because self is not counted */
	for (i=0;i<=c->num_properties;i++)
	{
		if (i == 0)
			prop_name = "self";
		else
			prop_name = GetPropertyNameByID(c,o->p[i].id);
		if (prop_name == NULL)
			sprintf(buf,": #%-19i",o->p[i].id);
		else
			sprintf(buf,": %-20s",prop_name);
		aprintf("%s = %s %s\n",buf,GetTagName(o->p[i].val),
			GetDataName(o->p[i].val));
	}
	aprintf(":>\n");
	return;
}

void AdminShowListNode(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[])
{
	list_node *l;

	int list_id;
	list_id = (int)parms[0];

	l = GetListNodeByID(list_id);
	if (l == NULL)
	{
		aprintf("Invalid list node id %i (or it has been deleted).\n",
			list_id);
		return;
	}
	aprintf(":< first = %s %s\n",GetTagName(l->first),
		GetDataName(l->first));
	aprintf(":  rest = %s %s\n",GetTagName(l->rest),
		GetDataName(l->rest));
	aprintf(":>\n");
}

void AdminShowList(int session_id,admin_parm_type parms[],
                   int num_blak_parm,parm_node blak_parm[])
{
	int list_id;
	list_id = (int)parms[0];

	aprintf(":<\n");
	AdminShowListParen(session_id,list_id,True);
	aprintf(":>\n");
}

void AdminShowListParen(int session_id,int list_id,int new_start)
{
	list_node *l;

	l = GetListNodeByID(list_id);
	if (l == NULL)
	{
		aprintf("Invalid list node id %i (or it has been deleted).\n",
			list_id);
		return;
	}

	if (new_start)
		aprintf(": [\n");

	if (l->first.v.tag == TAG_LIST)
		AdminShowListParen(session_id,l->first.v.data,True);
	else
	{
		//if (!new_start)
		//aprintf(", ");
		aprintf(": %s %s\n",GetTagName(l->first),
			GetDataName(l->first));
	}

	if (l->rest.v.tag == TAG_LIST)
		AdminShowListParen(session_id,l->rest.v.data,False);
	else
		if (l->rest.v.tag != TAG_NIL)
		{
			aprintf(": .\n");
			aprintf(": %s %s\n",GetTagName(l->rest),
					  GetDataName(l->rest));
		}

	if (new_start)
		aprintf(": ]\n");
}

void AdminShowUsers(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[])
{
	AdminShowUserHeader();
	ForEachUser(AdminShowOneUser);
}

void AdminShowUserHeader(void)
{
	aprintf("%-5s%-8s%-8s%s\n","Acct","Object","Class","Name");
}

void AdminShowOneUser(user_node *u)
{
	val_type name_val;
	resource_node *r;
	object_node *o;
	class_node *c;
	char ch = 'U';

	if (!u)
		return;

	o = GetObjectByID(u->object_id);
	if (!o)
		return;

	c = GetClassByID(o->class_id);
	if (!c)
		return;

	aprintf("%4i %7i %-7s ",u->account_id,u->object_id,c->class_name);

	name_val.int_val = SendTopLevelBlakodMessage(u->object_id,USER_NAME_MSG,0,NULL);
	if (name_val.v.tag == TAG_RESOURCE)
	{
		r = GetResourceByID(name_val.v.data);
		if (r == NULL)
			aprintf("Invalid resource id %i.",name_val.v.data);
		else
			aprintf("%s",r->resource_val);
	}
	else
		aprintf("Non-resource %i,%i.",name_val.v.tag,name_val.v.data);
	aprintf("\n");
}

void AdminShowUser(int session_id,admin_parm_type parms[],
                   int num_blak_parm,parm_node blak_parm[])
{
	user_node *u;
	int id;
	char *ptr;
	Bool is_by_number;
	char *arg_str;

	arg_str = (char *)parms[0];
	is_by_number = True;

	ptr = arg_str;
	while (*ptr != 0)
	{
		if (*ptr < '0' || *ptr > '9')
			is_by_number = False;
		ptr++;
	}

	if (is_by_number)
	{
		sscanf(arg_str,"%i",&id);
		u = GetUserByObjectID(id);
	}
	else
	{
		u = GetUserByName(arg_str);
	}

	if (u == NULL)
	{
		aprintf("Cannot find user %s.\n",arg_str);
		return;
	}

	AdminShowUserHeader();
	AdminShowOneUser(u);
}

void AdminShowUsage(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[])
{
	int s = GetUsedSessions();

	aprintf(":< sessions %i\n",s);
	aprintf(":>\n");
}

void AdminShowAccounts(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[])
{
	AdminShowAccountHeader();
	ForEachAccount(AdminShowOneAccount);
}

void AdminShowAccount(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[])
{
	account_node *a;
	int account_id;
	char *ptr;
	Bool is_account_number;

	char *account_str;
	account_str = (char *)parms[0];

	is_account_number = True;

	ptr = account_str;
	while (*ptr != 0)
	{
		if (*ptr < '0' || *ptr > '9')
			is_account_number = False;
		ptr++;
	}

	if (is_account_number)
	{
		sscanf(account_str,"%i",&account_id);
		a = GetAccountByID(account_id);
	}
	else
		a = GetAccountByName(account_str);

	if (a == NULL)
	{
		aprintf("Cannot find account %s.\n",account_str);
		return;
	}


	AdminShowAccountHeader();
	AdminShowOneAccount(a);

	AdminShowUserHeader();
	ForEachUserByAccountID(AdminShowOneUser,a->account_id);

}

void AdminShowAccountHeader()
{
	aprintf("%-6s%-23s%-10s%-8s%-30s\n","Acct","Name","Suspended","Credits",
		"Last login");
}

void AdminShowOneAccount(account_node *a)
{
	char ch = ' ';
	static const char* types = " ADG"; // see enum ACCOUNT_* in account.h
   char buff[9];

	if (a->type >= 0 && a->type <= (int)strlen(types))
		ch = types[a->type];

   // Check the suspend time.  We don't print a negative time.
   if (a->suspend_time <= GetTime())
   {
      a->suspend_time = 0;
   }

   if (a->suspend_time > 0)
   {
      // Print suspended time left in hours.
      sprintf(buff,"%7.1fh",(a->suspend_time - GetTime())/(60.0*60.0));
   }
   else
   {
	   buff[0] = 0;
   }

	aprintf("%4i%c %-24s%8s %4i.%02i %-30s\n",a->account_id,ch,a->name,
        buff, a->credits/100,a->credits%100,TimeStr(a->last_login_time));
}

void AdminShowResource(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[])
{
	int rsc_id;
	resource_node *r;

	char *name_resource;
	name_resource = (char *)parms[0];

	/* try name_resource as a string or as an int, for handiness */

	r = GetResourceByName(name_resource);
	if (r == NULL)
	{
		if (sscanf(name_resource,"%i",&rsc_id) != 1)
		{
			aprintf("There is no resource named %s.\n",name_resource);
			return;
		}

		r = GetResourceByID(rsc_id);
		if (r == NULL)
		{
			aprintf("There is no resource with id %s.\n",name_resource);
			return;
		}
	}
	aprintf("%-7s %s\n","ID","Name = Value");
	AdminPrintResource(r);
}

void AdminPrintResource(resource_node *r)
{
	if (r == NULL)
		eprintf("AdminPrintResource got passed NULL\n");
	else
	{
		aprintf("%-7i %s = %s\n",r->resource_id,
			r->resource_name == NULL ? "(dynamic)" : r->resource_name,r->resource_val);
	}
}

void AdminShowDynamicResources(int session_id,admin_parm_type parms[],
                               int num_blak_parm,parm_node blak_parm[])
{
	aprintf("%-7s %s\n","ID","Name = Value");
	ForEachDynamicRsc(AdminPrintResource);
}

void AdminShowTimers(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[])
{
	aprintf("%-7s%-14s%-8s%-20s\n","Timer","Remaining ms","Object",
		"Message");
	ForEachTimer(AdminShowOneTimer);
}

void AdminShowTimer(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[])
{
	timer_node *t;

	int timer_id;
	timer_id = (int)parms[0];

	t = GetTimerByID(timer_id);
	if (t == NULL)
	{
		aprintf("Cannot find timer %i.\n",timer_id);
		return;
	}

	aprintf("%-7s%-14s%-8s%-20s\n","Timer","Remaining ms","Object",
		"Message");
	AdminShowOneTimer(t);
}

void AdminShowOneTimer(timer_node *t)
{
	int expire_time;
	object_node *o;

	o = GetObjectByID(t->object_id);
	if (o == NULL)
	{
		aprintf("Timer has invalid object %i?\n",t->object_id);
		return;
	}

	expire_time = (int)(t->time - GetMilliCount());

	aprintf("%5i  %-14u%-8i%-20s\n",t->timer_id,expire_time,
		t->object_id,GetNameByID(t->message_id));
}

void AdminShowTime(int session_id,admin_parm_type parms[],
                   int num_blak_parm,parm_node blak_parm[])
{
	INT64 now = GetTime();

	aprintf("Current server clock reads %lli (%s).\n", now, TimeStr(now));
}

void AdminShowConfiguration(int session_id,admin_parm_type parms[],
                            int num_blak_parm,parm_node blak_parm[])
{
	ForEachConfigNode(AdminShowOneConfigNode);
}

void AdminShowOneConfigNode(config_node *c,const char *config_name,const char *default_str)
{
	if (c->config_type == CONFIG_GROUP)
		aprintf("\n");

	aprintf("%-20s ",config_name);
	switch (c->config_type)
	{
	case CONFIG_GROUP :
		aprintf("\n");
		break;
	case CONFIG_STR :
		if (strchr(c->config_str_value,' ') != NULL)
			aprintf("<%s>\n",c->config_str_value);
		else
			aprintf("%s\n",c->config_str_value);
		break;
	case CONFIG_INT :
		aprintf("%i\n",c->config_int_value);
		break;
	case CONFIG_PATH :
		aprintf("%s\n",c->config_str_value);
		break;
	case CONFIG_BOOL :
		if (c->config_int_value != False)
			aprintf("Yes\n");
		else
			aprintf("No\n");
		break;
	default :
		aprintf("# Unknown type\n");
		break;
	}
}

void AdminShowString(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[])
{
	string_node *snod;

	int string_id;
	string_id = (int)parms[0];

	if (IsStringByID(string_id) == False)
	{
		aprintf("Cannot find string %i.\n",string_id);
		return;
	}

	snod = GetStringByID(string_id);
	if (snod == NULL)
	{
		aprintf("Cannot find string %i.\n",string_id);
		return;
	}

	aprintf("String %i\n",string_id);
	aprintf("-------------------------------------------\n");
	/* not null-terminated */
	AdminBufferSend(snod->data,snod->len_data);
	aprintf("\n-------------------------------------------\n");
}

void AdminShowSysTimers(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[])
{
	aprintf("%s %-18s %-15s %-15s %-22s\n","#","System Timer type","Period","On the",
		"Next time");
	ForEachSysTimer(AdminShowEachSysTimer);
}

void AdminShowEachSysTimer(systimer_node *st)
{
  const char *s;

	switch (st->systimer_type)
	{
	case SYST_GARBAGE : s = "Garbage collect"; break;
	case SYST_SAVE : s = "Save game"; break;
	case SYST_BLAKOD_HOUR : s = "New Blakod hour"; break;
	case SYST_INTERFACE_UPDATE : s = "Update interface"; break;
	case SYST_RESET_TRANSMITTED : s = "Reset transmit count"; break;
	case SYST_RESET_POOL : s = "Reset buffer pool"; break;
	default : s = "Unknown"; break;
	}
	aprintf("%i %-18s %-15s ",st->systimer_type,s,RelativeTimeStr(st->period));
	aprintf("%-15s ",RelativeTimeStr(st->time));

	if (st->enabled)
		aprintf("%-22s",TimeStr(st->next_time_activate));
	else
		aprintf("Disabled");
	aprintf("\n");
}

void AdminShowCalls(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[])
{
	int i,count,ignore_val,max_index;
	kod_statistics *kstat;
	char c_name[50];

	int num_show;
	num_show = (int)parms[0];

	kstat = GetKodStats();

	aprintf("%4s %-15s %s\n","Rank","Function","Count");

	ignore_val = -1;
	for (count=0;count<num_show;count++)
	{
		max_index = -1;
		for (i=0;i<MAX_C_FUNCTION;i++)
		{
			if (ignore_val == -1 || kstat->c_count[i] < ignore_val)
			{
				if (max_index == -1 || kstat->c_count[i] > kstat->c_count[max_index])
					max_index = i;
			}
		}
		ignore_val = kstat->c_count[max_index];

		if (ignore_val == 0)
			break;

		switch (max_index)
		{
		case CREATEOBJECT : strcpy(c_name, "CreateObject"); break;
		case ISCLASS : strcpy(c_name, "IsClass"); break;
		case GETCLASS : strcpy(c_name, "GetClass"); break;
		case SENDMESSAGE : strcpy(c_name, "Send"); break;
		case POSTMESSAGE : strcpy(c_name, "Post"); break;
		case DEBUG : strcpy(c_name, "Debug"); break;
		case ADDPACKET : strcpy(c_name, "AddPacket"); break;
		case SENDPACKET : strcpy(c_name, "SendPacket"); break;
		case SENDCOPYPACKET : strcpy(c_name, "SendCopyPacket"); break;
		case CLEARPACKET : strcpy(c_name, "ClearPacket"); break;
		case GETINACTIVETIME : strcpy(c_name, "GetInactiveTime"); break;
		case STRINGEQUAL : strcpy(c_name, "StringEqual"); break;
		case STRINGCONTAIN : strcpy(c_name, "StringContain"); break;
		case SETRESOURCE : strcpy(c_name, "SetResource"); break;
		case PARSESTRING : strcpy(c_name, "ParseString"); break;
		case SETSTRING : strcpy(c_name, "SetString"); break;
		case CREATESTRING : strcpy(c_name, "CreateString"); break;
		case STRINGSUBSTITUTE : strcpy(c_name, "StringSubstitute"); break;
		case APPENDTEMPSTRING : strcpy(c_name, "AppendTempString"); break;
		case CLEARTEMPSTRING : strcpy(c_name, "ClearTempString"); break;
		case GETTEMPSTRING : strcpy(c_name, "GetTempString"); break;
		case STRINGLENGTH : strcpy(c_name, "StringLength"); break;
		case STRINGCONSISTSOF : strcpy(c_name, "StringConsistsOf"); break;
		case CREATETIMER : strcpy(c_name, "CreateTimer"); break;
		case DELETETIMER : strcpy(c_name, "DeleteTimer"); break;
		case GETTIMEREMAINING : strcpy(c_name, "GetTimeRemaining"); break;
		case CREATEROOMDATA : strcpy(c_name, "CreateRoomData"); break;
		case ROOMDATA : strcpy(c_name, "RoomData"); break;
		case CANMOVEINROOM : strcpy(c_name, "CanMoveInRoom"); break;
		case CANMOVEINROOMFINE : strcpy(c_name, "CanMoveInRoomFine"); break;
		case MINIGAMENUMBERTOSTRING : strcpy(c_name, "MinigameNumberToString"); break;
		case MINIGAMESTRINGTONUMBER : strcpy(c_name, "MinigameStringToNumber"); break;
		case CONS : strcpy(c_name, "Cons"); break;
		case FIRST : strcpy(c_name, "First"); break;
		case REST : strcpy(c_name, "Rest"); break;
		case LENGTH : strcpy(c_name, "Length"); break;
		case NTH : strcpy(c_name, "Nth"); break;
		case LIST : strcpy(c_name, "List"); break;
		case ISLIST : strcpy(c_name, "IsList"); break;
		case SETFIRST : strcpy(c_name, "SetFirst"); break;
		case SETNTH : strcpy(c_name, "SetNth"); break;
		case DELLISTELEM : strcpy(c_name, "DelListElem"); break;
		case FINDLISTELEM : strcpy(c_name, "FindListElem"); break;
		case MOVELISTELEM : strcpy(c_name, "MoveListElem"); break;
		case GETTIME : strcpy(c_name, "GetTime"); break;
		case GETTICKCOUNT : strcpy(c_name, "GetTickCount"); break;
		case ABS : strcpy(c_name, "Abs"); break;
		case BOUND : strcpy(c_name, "Bound"); break;
		case SQRT : strcpy(c_name, "Sqrt"); break;
		case CREATETABLE : strcpy(c_name, "CreateTable"); break;
		case ADDTABLEENTRY : strcpy(c_name, "AddTableEntry"); break;
		case GETTABLEENTRY : strcpy(c_name, "GetTableEntry"); break;
		case DELETETABLEENTRY : strcpy(c_name, "DeleteTableEntry"); break;
		case DELETETABLE : strcpy(c_name, "DeleteTable"); break;
		case ISOBJECT : strcpy(c_name, "IsObject"); break;
		case RECYCLEUSER : strcpy(c_name, "RecycleUser"); break;
		case RANDOM : strcpy(c_name, "Random"); break;

		default :
			sprintf(c_name,"Unknown (%i)",max_index);
			break;
		}

		aprintf("%3i. %-15s %i\n",count+1,c_name,kstat->c_count[max_index]);
	}
}

void AdminShowMessage(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[])
{
	class_node *c,*found_class;
	message_node *m;
	char *bkod_ptr;
	int i,num_parms;
	int parm_id;
	val_type parm_init_value;

	char *class_str;
	char *message_str;
	class_str = (char *)parms[0];
	message_str = (char *)parms[1];

	c = GetClassByName(class_str);
	if (c == NULL)
	{
		c = GetClassByID( atol( class_str ) );
		if (c == NULL)
		{
			aprintf("Cannot find class %s.\n",class_str);
			return;
		}
	}

	m = GetMessageByName(c->class_id,message_str,&found_class);
	if (m == NULL)
	{
		m = GetMessageByID(c->class_id,atol( message_str ),&found_class);
		if (m == NULL)
		{
			aprintf("Cannot find message %s in CLASS %i.\n",message_str,c->class_id);
			return;
		}
	}

	aprintf("--------------------------------------------------------------\n");
	aprintf("CLASS %i : %s\n",c->class_id,c->class_name);
	aprintf("MESSAGE %i : %s",m->message_id,GetNameByID(m->message_id));
	if (c != found_class)
		aprintf(" (handled by CLASS %s)",found_class->class_name);
	aprintf("\n");
	aprintf("--------------------------------------------------------------\n");
	aprintf("  Parameters:\n");
	/* we need to read the first few bytes from the bkod to get the parms */
	bkod_ptr = m->handler;
	/* # of local vars at this byte */
	bkod_ptr++;
	num_parms = *bkod_ptr;
	bkod_ptr++;

	for (i=0;i<num_parms;i++)
	{
		parm_id = *((int *)bkod_ptr);
		bkod_ptr += 4;
		parm_init_value.int_val = *((int *)bkod_ptr);
		bkod_ptr += 4;

		aprintf("  %-20s %s %s\n",GetNameByID(parm_id),GetTagName(parm_init_value),
			GetDataName(parm_init_value));
	}

	if (num_parms == 0)
		aprintf("  (none)\n");

	aprintf("--------------------------------------------------------------\n");
	if (m->dstr_id != INVALID_DSTR)
		aprintf("%s\n",GetClassDebugStr(found_class,m->dstr_id));
	else
		aprintf("No description\n");
	aprintf("--------------------------------------------------------------\n");
}

void AdminShowClass(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[])
{
	int i;
	class_node *c;
	char *classvar_name;
	char buf[200];

	char *class_str;
	class_str = (char *)parms[0];

	c = GetClassByName(class_str);
	if (c == NULL)
	{
		c = GetClassByID(atoi( class_str ) );
		if(c == NULL ) {
			aprintf("Cannot find class %s.\n",class_str);
			return;
		}
	}

	if (c->super_ptr == NULL)
		aprintf(":< CLASS %s (%i) %s\n",c->class_name,c->class_id,c->fname);
	else
		aprintf(":< CLASS %s (%i) %s is %s\n",c->class_name,c->class_id,c->fname,c->super_ptr->class_name);

	for (i=0;i<c->num_vars;i++)
	{
		classvar_name = GetClassVarNameByID(c,i);
		if (classvar_name == NULL)
			sprintf(buf,": VAR #%-19i",i);
		else
			sprintf(buf,": VAR %-20s",classvar_name);
		aprintf("%s = %s %s\n",buf,GetTagName(c->vars[i].val),
				  GetDataName(c->vars[i].val));
	}

   for (i=0;i<c->num_messages;i++)
      aprintf(": MSG %s\n",GetNameByID(c->messages[i].message_id));

	aprintf(":>\n");

}

void AdminShowExactInstances(int session_id, admin_parm_type parms[],
	int num_blak_parm, parm_node blak_parm[])
{
	int i, m;
	class_node* c;
	extern object_node* objects;
	extern int num_objects;

	char* class_str;
	class_str = (char*)parms[0];

	c = GetClassByName(class_str);
	if (c == NULL)
	{
		aprintf("Cannot find CLASS %s.\n", class_str);
		return;
	}

	aprintf(":< instances (excluding subclasses) of CLASS %s (%i)\n:", c->class_name, c->class_id);
	m = 0;
	for (i = 0; i < num_objects; i++)
	{
		class_node* wc = GetClassByID(objects[i].class_id);
		if (wc == c)
		{
			aprintf(" OBJECT %i", i);
			m++;
		}
	}
	aprintf("\n: %i total", m);
	aprintf("\n:>\n");
}

void AdminShowInstances(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[])
{
	int i, m;
	class_node *c;
	extern object_node* objects;
	extern int num_objects;

	char *class_str;
	class_str = (char *)parms[0];

	c = GetClassByName(class_str);
	if (c == NULL)
	{
		aprintf("Cannot find CLASS %s.\n",class_str);
		return;
	}

	aprintf(":< instances of CLASS %s (%i)\n:",c->class_name,c->class_id);
	m = 0;
	for (i = 0; i < num_objects; i++)
	{
		class_node* wc = GetClassByID(objects[i].class_id);
		while (wc)
		{
			if (wc == c)
			{
				aprintf(" OBJECT %i", i);
				m++;
				break;
			}
			wc = wc->super_ptr;
		}
	}
	aprintf("\n: %i total", m);
	aprintf("\n:>\n");
}

void AdminShowMatches(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[])
{
	int i, m;
	class_node *c;
	val_type match;
	char* class_str;
	char* property_str;
	char* relation_str;
	char* tag_str;
	char* data_str;
	char* walk;
	int tag_int;
	int data_int;
	int property_id;
	extern object_node* objects;
	extern int num_objects;
	enum { none=0,isequal=1,isgreater=2,isless=4,sametag=8,difftag=16 };
	int matchtype;

	class_str = (char *)parms[0];
	property_str = (char *)parms[1];
	relation_str = (char *)parms[2];
	tag_str = (char *)parms[3];
	data_str = (char *)parms[4];

	c = GetClassByName(class_str);
	if (c == NULL)
	{
		aprintf("Cannot find CLASS %s.\n",class_str);
		return;
	}

	property_id = GetPropertyIDByName(c,property_str);
	if (property_id == INVALID_PROPERTY)
	{
		aprintf("Property %s doesn't exist (at least for CLASS %s (%i)).\n",
			property_str,c->class_name,c->class_id);
		return;
	}

	matchtype = 0;
	walk = relation_str;
	while (walk && *walk)
	{
		switch (*walk++)
		{
		case '<': matchtype |= isless | sametag; break;
		case '=': matchtype |= isequal | sametag; break;
		case '>': matchtype |= isgreater | sametag; break;
		case '*': matchtype |= isless | isequal | isgreater | sametag; break;
		case '#': matchtype |= isless | isequal | isgreater | difftag; break;
		default:
			aprintf("Relationship not recognized.  Use <, =, >, *, #, or combination.\n");
			return;
		}
	}

	tag_int = GetTagNum(tag_str);
	if (tag_int == INVALID_TAG)
	{
		aprintf("'%s' is not a tag.\n",tag_str);
		return;
	}

	if (LookupAdminConstant(data_str,&data_int) == False)
	{
		bool negate = false;

		// INT properties may have a negative number and still be legal
		if (tag_int == TAG_INT && *data_str == '-')
		{
			negate = true;
			data_str++;
		}

		data_int = GetDataNum(tag_int,data_str);
		if (data_int == INVALID_DATA)
		{
			aprintf("'%s' is not valid data.\n",data_str);
			return;
		}

		if (negate)
			data_int = -data_int;
	}

	match.v.tag = tag_int;
	match.v.data = data_int;

	aprintf(":< matching instances of CLASS %s (%i) %s %s %s %s\n",
		c->class_name,c->class_id, property_str, relation_str,
		GetTagName(match), GetDataName(match));

	m = 0;
	for (i = 0; i < num_objects; i++)
	{
		class_node* wc = GetClassByID(objects[i].class_id);
		class_node* thisc = wc;
		while (wc)
		{
			if (wc == c)
			{
				val_type thisv;
				int thismatch;

#if 0
				property_id = GetPropertyIDByName(thisc,property_str);
				if (property_id == INVALID_PROPERTY)
				{
					aprintf("Property %s doesn't exist (at least for CLASS %s (%i)).\n",
						property_str,thisc->class_name,thisc->class_id);
					break;
				}
#endif

				// This object's property's value.
				thisv = objects[i].p[property_id].val;

				// Compare it to the match value.
				thismatch = 0;
				if (thisv.v.tag == match.v.tag)
					thismatch |= sametag;
				else
					thismatch |= difftag;
				if (thisv.v.tag == TAG_INT)
				{
					if ((int)thisv.v.data > (int)match.v.data) thismatch |= isgreater;
					if ((int)thisv.v.data < (int)match.v.data) thismatch |= isless;
				}
				else
				{
					if ((unsigned int)thisv.v.data > (unsigned int)match.v.data) thismatch |= isgreater;
					if ((unsigned int)thisv.v.data < (unsigned int)match.v.data) thismatch |= isless;
				}
				if (thisv.v.data == match.v.data) thismatch |= isequal;

				// If it compares favorably according to the relationship requested, show it.
				if ((thismatch &  (sametag|difftag)) == (matchtype &  (sametag|difftag)) &&
					(thismatch & ~(sametag|difftag)) &  (matchtype & ~(sametag|difftag)))
				{
					aprintf(": OBJECT %i CLASS %s (%i) %s = %s %s\n",
						i, thisc->class_name, thisc->class_id, property_str,
						GetTagName(thisv), GetDataName(thisv));
					m++;
				}

				break;
			}
			wc = wc->super_ptr;
		}
	}
	aprintf(": %i total\n", m);
	aprintf(":>\n");
}

void AdminShowPackages(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[])
{
	aprintf("%-35s %-4s %-9s\n","Filename","Type","Date/Time/Sequence");
	ForEachDLlist(AdminShowOnePackage);
}

void AdminShowOnePackage(dllist_node *dl)
{
	aprintf("%-35s %-4i %-9i\n",dl->fname,dl->file_type,dl->last_mod_time);
}

void AdminShowConstant(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[])
{
	int value;

	char *name;
	name = (char *)parms[0];

	if (LookupAdminConstant(name,&value))
		aprintf("%s = %i\n",name,value);
	else
		aprintf("There is no value for %s\n",name);
}

void AdminShowTransmitted(int session_id,admin_parm_type parms[],
                          int num_blak_parm,parm_node blak_parm[])
{
	aprintf("In most recent transmission period, server has transmitted %i bytes.\n",
		GetTransmittedBytes());
}

void AdminShowTable(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[])
{
	table_node *tn;
	hash_node *hn;
	int i;

	int table_id;
	table_id = (int)parms[0];

	tn = GetTableByID(table_id);
	if (tn == NULL)
	{
		aprintf("Cannot find table %i.\n",table_id);
		return;
	}

	aprintf("Table %i (size %i)\n",tn->table_id,tn->size);
	aprintf("----------------------------------------------------------------------\n");
	for (i=0;i<tn->size;i++)
	{
		hn = tn->table[i];
		if (hn != NULL)
		{
			aprintf("hash %5i : ",i);
			while (hn != NULL)
			{
				aprintf("(key %s %s ",GetTagName(hn->key_val),GetDataName(hn->key_val));
				aprintf("val %s %s) ",GetTagName(hn->data_val),GetDataName(hn->data_val));
				hn = hn->next;
			}
			aprintf("\n");
		}
	}

}

void AdminShowName(int session_id,admin_parm_type parms[],
                   int num_blak_parm,parm_node blak_parm[])
{
	char *username = (char *)parms[0];
	user_node * u = GetUserByName(username);

	if (!u)
	{
		aprintf("Cannot find user with name %s.\n",username);
		return;
	}

	aprintf(":< object %i\n",u->object_id);
	aprintf(":>\n");
}

static val_type admin_show_references_value;
static const char *admin_show_references_tag_str;
static const char *admin_show_references_data_str;
static int admin_show_references_count;
void AdminShowReferences(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[])
{
	int tag_int,data_int;

	admin_show_references_tag_str = (char *)parms[0];
	admin_show_references_data_str = (char *)parms[1];

	tag_int = GetTagNum(admin_show_references_tag_str);
	if (tag_int == INVALID_TAG)
	{
		aprintf("'%s' is not a tag.\n",admin_show_references_tag_str);
		return;
	}
	if (tag_int == TAG_NIL)
	{
		/* Showing thousands of instances of NIL would be overwhelming. */
		aprintf("Cannot show references to NIL.\n");
		return;
	}

	if (LookupAdminConstant(admin_show_references_data_str,&data_int) == False)
	{
		bool negate = false;

		// INT properties may have a negative number and still be legal
		if (tag_int == TAG_INT && *admin_show_references_data_str == '-')
		{
			negate = true;
			admin_show_references_data_str++;
		}

		data_int = GetDataNum(tag_int,admin_show_references_data_str);
		if (data_int == INVALID_DATA)
		{
			aprintf("'%s' is not valid data.\n",admin_show_references_data_str);
			return;
		}

		if (negate)
			data_int = -data_int;
	}

	admin_show_references_value.v.tag = tag_int;
	admin_show_references_value.v.data = data_int;

	admin_show_references_tag_str = GetTagName(admin_show_references_value);
	admin_show_references_data_str = GetDataName(admin_show_references_value);

	admin_show_references_count = 0;

	aprintf(":< references to %s %s\n",admin_show_references_tag_str,admin_show_references_data_str);

	ForEachObject(AdminShowReferencesEachObject);

	aprintf(":>\n");
}

static int admin_show_references_current_object;
static const char * admin_show_references_current_prop;
static class_node* admin_show_references_current_class;
void AdminShowReferencesEachObject(object_node *o)
{
	int i;

	admin_show_references_current_class = GetClassByID(o->class_id);
	admin_show_references_current_object = o->object_id;

	for (i=0;i<o->num_props;i++)
	{
		if (i == 0)
			admin_show_references_current_prop = "self";
		else
			admin_show_references_current_prop =
			GetPropertyNameByID(admin_show_references_current_class,o->p[i].id);

		if (o->p[i].val.int_val == admin_show_references_value.int_val)
		{
			admin_show_references_count++;
			aprintf(": OBJECT %i CLASS %s %s = %s %s\n",
				admin_show_references_current_object,
				admin_show_references_current_class->class_name,
				admin_show_references_current_prop,
				admin_show_references_tag_str,
				admin_show_references_data_str);
		}
		else if (o->p[i].val.v.tag == TAG_LIST)
		{
			AdminShowReferencesEachList(o->p[i].val.v.data);
		}
	}
}

void AdminShowReferencesEachList(int list_id)
{
	list_node *l;

	for(;;)
	{
		l = GetListNodeByID(list_id);
		if (l == NULL)
		{
			eprintf("AdminShowReferencesEachList can't get LIST %i\n",list_id);
			return;
		}

		if (l->first.int_val == admin_show_references_value.int_val)
			aprintf(": OBJECT %i CLASS %s %s = LIST containing %s %s\n",
			admin_show_references_current_object,
			admin_show_references_current_class->class_name,
			admin_show_references_current_prop,
			admin_show_references_tag_str,
			admin_show_references_data_str);

		if (l->first.v.tag == TAG_LIST)
			AdminShowReferencesEachList(l->first.v.data);

		if (l->rest.v.tag != TAG_LIST)
			break;

		list_id = l->rest.v.data;
	}
}

/* in parsecli.c */
extern client_table_node *user_table,*system_table,*usercommand_table;

void AdminShowProtocol(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[])
{
	int i;

#define QT (CLIENT_COMMANDS_PER_TABLE/4)

	aprintf("Regular commands\n");

	for (i=0;i<CLIENT_COMMANDS_PER_TABLE/4;i++)
		aprintf("%3i %10u | %3i %10u | %3i %10u | %3i %10u\n",
		i,user_table[i].call_count,i+QT,user_table[i+QT].call_count,
		i+2*QT,user_table[i+2*QT].call_count,i+3*QT,user_table[i+3*QT].call_count);

	AdminSendBufferList();

	aprintf("\nSystem commands\n");

	for (i=0;i<CLIENT_COMMANDS_PER_TABLE/4;i++)
		aprintf("%3i %10u | %3i %10u | %3i %10u | %3i %10u\n",
		i,system_table[i].call_count,i+QT,system_table[i+QT].call_count,
		i+2*QT,system_table[i+2*QT].call_count,i+3*QT,system_table[i+3*QT].call_count);

	AdminSendBufferList();

	aprintf("\nUser commands\n");

	for (i=0;i<CLIENT_COMMANDS_PER_TABLE/4;i++)
		aprintf("%3i %10u | %3i %10u | %3i %10u | %3i %10u\n",
		i,usercommand_table[i].call_count,i+QT,usercommand_table[i+QT].call_count,
		i+2*QT,usercommand_table[i+2*QT].call_count,
		i+3*QT,usercommand_table[i+3*QT].call_count);
}

void AdminSetClass(int session_id,admin_parm_type parms[],
                   int num_blak_parm,parm_node blak_parm[])
{
   // Note that setting a class var is only temporary until the server restarts.
	class_node *c;
	char *class_name, *var_name, *tag_str, *data_str;
   int var_id, tag_int, data_int;
	val_type val;

	class_name = (char *)parms[0];
	var_name = (char *) parms[1];
	tag_str = (char *) parms[2];
	data_str = (char *) parms[3];

	c = GetClassByName(class_name);
	if (c == NULL)
	{
		aprintf("Cannot find class named %s.\n",class_name);
		return;
	}

   var_id = GetClassVarIDByName(c, var_name);
   if (var_id == INVALID_CLASSVAR)
   {
		aprintf("Cannot find classvar named %s in class %s.\n", var_name, class_name);
		return;
   }

   tag_int = GetTagNum(tag_str);
	if (tag_int == INVALID_TAG)
	{
		aprintf("'%s' is not a tag.\n", tag_str);
		return;
	}

	if (LookupAdminConstant(data_str,&data_int) == False)
	{
		bool negate = false;

		// INT properties may have a negative number and still be legal
		if (tag_int == TAG_INT && *data_str == '-')
		{
			negate = true;
			data_str++;
		}

		data_int = GetDataNum(tag_int,data_str);
		if (data_int == INVALID_DATA)
		{
			aprintf("'%s' is not valid data.\n",data_str);
			return;
		}

		if (negate)
			data_int = -data_int;
	}

   val.v.tag = tag_int;
	val.v.data = data_int;

   c->vars[var_id].val = val;
}

void AdminSetObject(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[])
{
	object_node *o;
	class_node *c;
	int property_id,tag_int,data_int;
	val_type val;
	int object_id;
	char *property_str,*tag_str,*data_str;

	object_id = (int)parms[0];
	property_str = (char *)parms[1];
	tag_str = (char *)parms[2];
	data_str = (char *)parms[3];

	o = GetObjectByID(object_id);
	if (o == NULL)
	{
		aprintf("Invalid object id %i (or it has been deleted).\n",
			object_id);
		return;
	}

	c = GetClassByID(o->class_id);
	if (c == NULL) /* can't ever be */
	{
		aprintf("OBJECT %i has invalid class!\n",o->object_id);
		return;
	}

	property_id = GetPropertyIDByName(c,property_str);
	if (property_id == INVALID_PROPERTY)
	{
		aprintf("Property %s doesn't exist (at least for CLASS %s (%i)).\n",
			property_str,c->class_name,c->class_id);
		return;
	}

	tag_int = GetTagNum(tag_str);
	if (tag_int == INVALID_TAG)
	{
		aprintf("'%s' is not a tag.\n",tag_str);
		return;
	}

	if (LookupAdminConstant(data_str,&data_int) == False)
	{
		bool negate = false;

		// INT properties may have a negative number and still be legal
		if (tag_int == TAG_INT && *data_str == '-')
		{
			negate = true;
			data_str++;
		}

		data_int = GetDataNum(tag_int,data_str);
		if (data_int == INVALID_DATA)
		{
			aprintf("'%s' is not valid data.\n",data_str);
			return;
		}

		if (negate)
			data_int = -data_int;
	}

	val.v.tag = tag_int;
	val.v.data = data_int;

	o->p[property_id].val = val;
}

void AdminSetAccountName(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[])
{
	account_node *a;

	int account_id;
	char *name;
	account_id = (int)parms[0];
	name = (char *)parms[1];
	if (!name || !*name)
	{
		aprintf("Missing parameter 2: new account name.\n");
		return;
	}

	if (strchr(name, ':'))
	{
		aprintf("Account names cannot contain the character ':'.\n");
		return;
	}

	a = GetAccountByID(account_id);
	if (a == NULL)
	{
		aprintf("Cannot find account %i.\n",account_id);
		return;
	}
	lprintf("AdminSetAccountName changing name of account %i from %s to %s\n",
		a->account_id,a->name,name);
	aprintf("Changing name of account %i from '%s' to '%s'.\n",
		a->account_id,a->name,name);
	SetAccountName(a,name);
}

void AdminSetAccountPassword(int session_id,admin_parm_type parms[],
                             int num_blak_parm,parm_node blak_parm[])
{
	account_node *a;

	int account_id;
	char *password;
	account_id = (int)parms[0];
	password = (char *)parms[1];

	a = GetAccountByID(account_id);
	if (a == NULL)
	{
		aprintf("Cannot find account %i.\n",account_id);
		return;
	}
	lprintf("AdminSetAccountPassword setting password for %s\n",a->name);
	SetAccountPassword(a,password);

	aprintf("Set password for account %i (%s).\n",a->account_id,a->name);
}

void AdminSetAccountCredits(int session_id,admin_parm_type parms[],
                            int num_blak_parm,parm_node blak_parm[])
{
	account_node *a;

	int account_id,credits;
	account_id = (int)parms[0];
	credits = (int)parms[1];

	a = GetAccountByID(account_id);
	if (a == NULL)
	{
		aprintf("Cannot find account %i.\n",account_id);
		return;
	}
	lprintf("AdminSetAccountCredits setting account %i to have %i credits\n",account_id,credits);
	a->credits = 100*credits + 5;
}

void AdminSetAccountObject(int session_id,admin_parm_type parms[],
                           int num_blak_parm,parm_node blak_parm[])
{
	int account_id,object_id;
	user_node *u;

	account_id = (int)parms[0];
	object_id = (int)parms[1];

	if (GetObjectByID(object_id) == NULL)
	{
		aprintf("Object %i does not exist.\n",object_id);
		return;
	}

	if (GetAccountByID(account_id) == NULL)
	{
		aprintf("Cannot find account %i.\n",account_id);
		return;
	}

	u = GetUserByObjectID(object_id);
	if (u == NULL)
	{
		aprintf("warning: Object %i is not a known user object; results may not be what you expect.\n",object_id);
	}
	else
	{
		aprintf("Removing user object %i from the old account %i.\n", u->object_id, u->account_id);
		DeleteUserByObjectID(u->object_id); /* deletes user node but not object */
		u = NULL;
	}

	if (AssociateUser(account_id,object_id) == False) /* creates a user node for it */
	{
		aprintf("Error assocating new account and object.\n");
	}
	else
	{
		aprintf("Associated account %i with object %i as a user.\n", account_id, object_id);
	}
}

/*
void AdminSetResource(int session_id,admin_parm_type parms[])
{
resource_node *r;

  int rsc_id;
  char *str_value;
  rsc_id = (int)parms[0];
  str_value = (char *)parms[1];

	r = GetResourceByID(rsc_id);
	if (r == NULL)
	{
	aprintf("Resource ID %i does not exist.\n",rsc_id);
	return;
	}

	  if (r->resource_id < MIN_DYNAMIC_RSC)
	  {
      aprintf("Resource %i is not a dynamic resource.\n",
	  r->resource_id);
      return;
	  }

		ChangeDynamicResourceStr(r,str_value);

		  AdminPrintResource(r);
		  }
*/
void AdminSetConfigInt(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[])
{
	int config_id;
	config_node *c;

	char *group;
	char *name;
	int new_value;
	group = (char *)parms[0];
	name = (char *)parms[1];
	new_value = (int)parms[2];

	config_id = GetConfigIDByGroupAndName(group,name);
	if (config_id == INVALID_CONFIG)
	{
		aprintf("Unable to find configure group %s name %s.\n",group,name);
		return;
	}

	c = GetConfigByID(config_id);
	if (c == NULL)
	{
		aprintf("Bizarre--got config id %i, but no config node exists.\n",config_id);
		return;
	}

	if (c->is_dynamic == False)
	{
		aprintf("This configure option is not settable at runtime.\n");
		return;
	}

	if (c->config_type != CONFIG_INT)
	{
		aprintf("This configure option is not an integer.\n");
		return;
	}


	SetConfigInt(c->config_id,new_value);

	aprintf("Configure option group %s name %s is now set to %i.\n",
		group,name,new_value);

}

void AdminSetConfigBool(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[])
{
	int config_id;
	config_node *c;

	char *group;
	char *name;
	char *new_value;
	group = (char *)parms[0];
	name = (char *)parms[1];
	new_value = (char *)parms[2];

	config_id = GetConfigIDByGroupAndName(group,name);
	if (config_id == INVALID_CONFIG)
	{
		aprintf("Unable to find configure group %s name %s.\n",group,name);
		return;
	}

	c = GetConfigByID(config_id);
	if (c == NULL)
	{
		aprintf("Bizarre--got config id %i, but no config node exists.\n",config_id);
		return;
	}

	if (c->is_dynamic == False)
	{
		aprintf("This configure option is not settable at runtime.\n");
		return;
	}

	if (c->config_type != CONFIG_BOOL)
	{
		aprintf("This configure option is not boolean.\n");
		return;
	}


	if (stricmp(new_value,"YES") != 0 && stricmp(new_value,"NO") != 0)
	{
		aprintf("Boolean configuration options must be 'yes' or 'no'\n");
		return;
	}


	if (stricmp(new_value,"YES") == 0)
		SetConfigBool(c->config_id,True);
	else
		SetConfigBool(c->config_id,False);


	aprintf("Configure option group %s name %s is now set to '%s'.\n",
		group,name,new_value);
}

void AdminSetConfigStr(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[])
{
	int config_id;
	config_node *c;

	char *group;
	char *name;
	char *new_value;
	group = (char *)parms[0];
	name = (char *)parms[1];
	new_value = (char *)parms[2];

	config_id = GetConfigIDByGroupAndName(group,name);
	if (config_id == INVALID_CONFIG)
	{
		aprintf("Unable to find configure group %s name %s.\n",group,name);
		return;
	}

	c = GetConfigByID(config_id);
	if (c == NULL)
	{
		aprintf("Bizarre--got config id %i, but no config node exists.\n",config_id);
		return;
	}

	if (c->is_dynamic == False)
	{
		aprintf("This configure option is not settable at runtime.\n");
		return;
	}

	if (c->config_type != CONFIG_STR)
	{
		aprintf("This configure option is not a string.\n");
		return;
	}


	SetConfigStr(c->config_id,new_value);

	aprintf("Configure option group %s name %s is now set to %s.\n",
		group,name,new_value);

}

void AdminSuspendUser(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[])
{
	user_node *u;
	account_node *a;
	int hours;
	int id;
	char *arg_str;
	Bool is_by_number;
	//char *ptr;

	hours = (int)parms[0];

	arg_str = (char *)parms[1];
	if (!arg_str || !*arg_str)
	{
		aprintf("Missing parameter 2: user name or number.\n");
		return;
	}

	is_by_number = False;
	//   is_by_number = True;

	//   ptr = arg_str;
	//   while (*ptr != 0)
	//   {
	//      if (*ptr < '0' || *ptr > '9')
	//	 is_by_number = False;
	//      ptr++;
	//   }

	if (is_by_number)
	{
		sscanf(arg_str,"%i",&id);
		u = GetUserByObjectID(id);
	}
	else
	{
		u = GetUserByName(arg_str);
	}

	if (u == NULL)
	{
		aprintf("Cannot find user %s.\n",arg_str);
		return;
	}

	a = GetAccountByID(u->account_id);

	if (a == NULL)
	{
		aprintf("Cannot find account for user %s.\n",arg_str);
		return;
	}

	if (!SuspendAccountRelative(a, hours))
	{
		aprintf("Suspension of account %i (%s) failed.\n",
			a->account_id, a->name);
		return;
	}

	if (a->suspend_time <= 0)
	{
		aprintf("Account %i (%s) is unsuspended.\n",
			a->account_id, a->name);
	}
	else
	{
		aprintf("Account %i (%s) is suspended until %s.\n",
			a->account_id, a->name, TimeStr(a->suspend_time));
	}
}

void AdminSuspendAccount(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[])
{
	account_node *a;
	int hours;
	int id;
	char *arg_str;
	Bool is_by_number;
	char *ptr;

	hours = (int)parms[0];

	arg_str = (char *)parms[1];
	if (!arg_str || !*arg_str)
	{
		aprintf("Missing parameter 2: account name or number.\n");
		return;
	}

	is_by_number = True;

	ptr = arg_str;
	while (*ptr != 0)
	{
		if (*ptr < '0' || *ptr > '9')
			is_by_number = False;
		ptr++;
	}

	if (is_by_number)
	{
		sscanf(arg_str,"%i",&id);
		a = GetAccountByID(id);
	}
	else
	{
		a = GetAccountByName(arg_str);
	}

	if (a == NULL)
	{
		aprintf("Cannot find account %s.\n",arg_str);
		return;
	}

	if (!SuspendAccountRelative(a, hours))
	{
		aprintf("Suspension of account %i (%s) failed.\n",
			a->account_id, a->name);
		return;
	}

	if (a->suspend_time <= 0)
	{
		aprintf("Account %i (%s) is unsuspended.\n",
			a->account_id, a->name);
	}
	else
	{
		aprintf("Account %i (%s) is suspended until %s.\n",
			a->account_id, a->name, TimeStr(a->suspend_time));
	}
}

void AdminUnsuspendUser(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[])
{
	user_node *u;
	account_node *a;
	int id;
	char *arg_str;
	Bool is_by_number;
	//char *ptr;

	arg_str = (char *)parms[0];
	if (!arg_str || !*arg_str)
	{
		aprintf("Missing parameter 1: user name or number.\n");
		return;
	}

	is_by_number = False;
	//   is_by_number = True;

	//   ptr = arg_str;
	//   while (*ptr != 0)
	//   {
	//      if (*ptr < '0' || *ptr > '9')
	//	 is_by_number = False;
	//      ptr++;
	//   }

	if (is_by_number)
	{
		sscanf(arg_str,"%i",&id);
		u = GetUserByObjectID(id);
	}
	else
	{
		u = GetUserByName(arg_str);
	}

	if (u == NULL)
	{
		aprintf("Cannot find user %s.\n",arg_str);
		return;
	}

	a = GetAccountByID(u->account_id);

	if (a == NULL)
	{
		aprintf("Cannot find account for user %s.\n",arg_str);
		return;
	}

	if (!SuspendAccountAbsolute(a, 0) || a->suspend_time > 0)
	{
		aprintf("Unsuspension of account %i (%s) failed.\n",
			a->account_id, a->name);
		return;
	}

	aprintf("Account %i (%s) is unsuspended.\n",
		a->account_id, a->name);
}

void AdminUnsuspendAccount(int session_id,admin_parm_type parms[],
                           int num_blak_parm,parm_node blak_parm[])
{
	account_node *a;
	int id;
	char *arg_str;
	Bool is_by_number;
	char *ptr;

	arg_str = (char *)parms[0];
	if (!arg_str || !*arg_str)
	{
		aprintf("Missing parameter 1: account name or number.\n");
		return;
	}

	is_by_number = True;

	ptr = arg_str;
	while (*ptr != 0)
	{
		if (*ptr < '0' || *ptr > '9')
			is_by_number = False;
		ptr++;
	}

	if (is_by_number)
	{
		sscanf(arg_str,"%i",&id);
		a = GetAccountByID(id);
	}
	else
	{
		a = GetAccountByName(arg_str);
	}

	if (a == NULL)
	{
		aprintf("Cannot find account %s.\n",arg_str);
		return;
	}

	if (!SuspendAccountAbsolute(a, 0) || a->suspend_time > 0)
	{
		aprintf("Unsuspension of account %i (%s) failed.\n",
			a->account_id, a->name);
		return;
	}

	aprintf("Account %i (%s) is unsuspended.\n",
		a->account_id, a->name);
}

void AdminCreateAccount(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[])
{
	int account_id;

	char *name,*password,*type;
	type = (char *)parms[0];
	name = (char *)parms[1];
	password = (char *)parms[2];

	if (0 == stricmp(type, "AUTO"))
	{
		// Tried "CREATE ACCOUNT AUTO blah blah" for the command "CREATE AUTO blah blah".
		AdminCreateAutomated(session_id,&parms[1], 0, NULL);
		return;
	}

	if (!name || !*name)
	{
		aprintf("Account name must be at least one character.\n");
		return;
	}
	if (strchr(name, ':'))
	{
		aprintf("Account names cannot contain the character ':'.\n");
		return;
	}

	switch (toupper(type[0]))
	{
	case 'A':
		if (CreateAccount(name,password,ACCOUNT_ADMIN,&account_id) == False)
		{
			aprintf("Account name %s already exists\n",name);
			return;
		}
		break;

	case 'D':
		if (CreateAccount(name,password,ACCOUNT_DM,&account_id) == False)
		{
			aprintf("Account name %s already exists\n",name);
			return;
		}
		break;

	default :
		if (CreateAccount(name,password,ACCOUNT_NORMAL,&account_id) == False)
		{
			aprintf("Account name %s already exists\n",name);
			return;
		}

	}

	aprintf("Created ACCOUNT %i.\n",account_id);
}

void AdminCreateAutomated(int session_id,admin_parm_type parms[],
                          int num_blak_parm,parm_node blak_parm[])
{
	/* create account and 1 user for it */

	int account_id;
	user_node *u;

	char *name,*password;

	name = (char *)parms[0];
	password = (char *)parms[1];

	if (CreateAccount(name,password,ACCOUNT_NORMAL,&account_id) == False)
	{
		aprintf("Account name %s already exists\n",name);
		return;
	}

	aprintf("Created account %i.\n",account_id);

	u = CreateNewUser(account_id,USER_CLASS);
	if (u == NULL)
	{
		aprintf("Cannot find just created user for account %i!\n",
			account_id);
		return;
	}

	AdminShowOneUser(u);

}

void AdminRecreateAutomated(int session_id,admin_parm_type parms[],
                            int num_blak_parm,parm_node blak_parm[])
{
	/* create account and 1 user for it */

	int account_id, acct;
	user_node *u;

	char *name,*password;

	account_id = (int)parms[0];
	name = (char *)parms[1];
	password = (char *)parms[2];

	acct = RecreateAccountSecurePassword(account_id,name,password,ACCOUNT_NORMAL);
	if (acct >= 0)
	{
		aprintf("Created account %i.\n",acct);
	}
	else
	{
		aprintf("Cannot recreate account %i; may already exist.\n", account_id);
		return;
	}

	u = CreateNewUser(account_id,USER_CLASS);
	if (u == NULL)
	{
		aprintf("Cannot find just created user for account %i!\n",
			account_id);
		return;
	}
	AdminShowOneUser(u);

}

void AdminCreateUser(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[])
{
	user_node *u;

	int account_id;
	account_id = (int)parms[0];

	u = CreateNewUser(account_id,USER_CLASS);
	if (u == NULL)
	{
		aprintf("Cannot find just created user for account %i!\n",
			account_id);
		return;
	}
	AdminShowUserHeader();
	AdminShowOneUser(u);
}

void AdminCreateAdmin(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[])
{
	user_node *u;

	int account_id;
	account_id = (int)parms[0];

	u = CreateNewUser(account_id,ADMIN_CLASS);
	if (u == NULL)
	{
		aprintf("Cannot find just created user for account %i!\n",
			account_id);
		return;
	}
	AdminShowUserHeader();
	AdminShowOneUser(u);
}

void AdminCreateDM(int session_id,admin_parm_type parms[],
                   int num_blak_parm,parm_node blak_parm[])
{
	user_node *u;

	int account_id;
	account_id = (int)parms[0];

	u = CreateNewUser(account_id,DM_CLASS);
	if (u == NULL)
	{
		aprintf("Cannot find just created user for account %i!\n",
			account_id);
		return;
	}
	AdminShowUserHeader();
	AdminShowOneUser(u);
}


void AdminCreateObject(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[])
{
	class_node *c;
	int new_object_id;
	val_type system_id_const;

	char *class_name;
	class_name = (char *)parms[0];


	c = GetClassByName(class_name);
	if (c == NULL)
	{
		aprintf("Cannot find class named %s.\n",class_name);
		return;
	}

	/* add system_id = the system id for convenience */

	system_id_const.v.tag = TAG_OBJECT;
	system_id_const.v.data = GetSystemObjectID();

	blak_parm[num_blak_parm].type = CONSTANT;
	blak_parm[num_blak_parm].value = system_id_const.int_val;
	blak_parm[num_blak_parm].name_id = SYSTEM_PARM;
	num_blak_parm++;

	new_object_id = CreateObject(c->class_id,num_blak_parm,blak_parm);
	aprintf("Created object %i.\n",new_object_id);
}

void AdminCreateListNode(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[])
{
	int num,list_id;
	val_type first_val,rest_val;

	char *first_tag,*first_data,*rest_tag,*rest_data;
	first_tag = (char *)parms[0];
	first_data = (char *)parms[1];
	rest_tag = (char *)parms[2];
	rest_data = (char *)parms[3];

	num = GetTagNum(first_tag);
	if (num == INVALID_TAG)
	{
		aprintf("First tag invalid.\n");
		return;
	}
	first_val.v.tag = num;

	num = GetDataNum(first_val.v.tag,first_data);
	if (num == INVALID_DATA)
	{
		aprintf("First data invalid.\n");
		return;
	}
	first_val.v.data = num;

	num = GetTagNum(rest_tag);
	if (num == INVALID_TAG)
	{
		aprintf("Rest tag invalid.\n");
		return;
	}
	rest_val.v.tag = num;

	num = GetDataNum(rest_val.v.tag,rest_data);
	if (num == INVALID_DATA)
	{
		aprintf("Rest data invalid.\n");
		return;
	}
	rest_val.v.data = num;

	list_id = Cons(first_val,rest_val);
	aprintf("Created list node %i.\n",list_id);

}

void AdminCreateTimer(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[])
{
	int message_id,timer_id;

	int object_id,milliseconds;
	char *message_name;
	object_id = (int)parms[0];
	message_name = (char *)parms[1];
	milliseconds = (int)parms[2];

	message_id = GetIDByName(message_name);
	if (message_id == INVALID_ID)
	{
		aprintf("Cannot find message name %s.\n",message_name);
		return;
	}

	timer_id = CreateTimer(object_id,message_id,milliseconds);
	aprintf("Created timer %i.\n",timer_id);
}

void AdminCreateResource(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[])
{
	int rsc_id;
	resource_node *r;

	char *resource_value;
	resource_value = (char *)parms[0];

	rsc_id = AddDynamicResource(resource_value);
	r = GetResourceByID(rsc_id);
	if (r == NULL)
	{
		aprintf("Error creating resource.\n");
		return;
	}
	AdminPrintResource(r);
}

void AdminDeleteTimer(int session_id,admin_parm_type parms[],
                      int num_blak_parm,parm_node blak_parm[])
{
	timer_node *t;

	int timer_id;
	timer_id = (int)parms[0];

	t = GetTimerByID(timer_id);
	if (t == NULL)
	{
		aprintf("Timer %i does not exist.\n",timer_id);
		return;
	}
	AdminShowOneTimer(t);

	DeleteTimer(timer_id);
	aprintf("This timer has been deleted.\n");

}

void AdminDeleteAccount(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[])
{
	account_node *a;

	int account_id;
	account_id = (int)parms[0];

	a = GetAccountByID(account_id);
	if (a == NULL)
	{
		aprintf("Account %i does not exist.\n",account_id);
		return;
	}

	if (a->account_id == admin_session_id)
	{
		aprintf("You can't delete your own account.\n");
		return;
	}

	aprintf("Account %i will be deleted.\n",a->account_id);

   // XXX Need a replacement for this on Linux
#ifdef BLAK_PLATFORM_WINDOWS
	PostThreadMessage(main_thread_id,WM_BLAK_MAIN_DELETE_ACCOUNT,0,a->account_id);
#endif
}

void AdminDeleteEachUserObject(user_node *u)
{
	/* delete u->object_id */

	val_type ret_val;

	ret_val.int_val = SendTopLevelBlakodMessage(u->object_id,DELETE_MSG,0,NULL);
}

static int admin_check_user;
static Bool admin_user_is_logged_in;
void AdminDeleteUser(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[])
{
	user_node *u;

	int object_id;
	object_id = (int)parms[0];

	u = GetUserByObjectID(object_id);
	if (u == NULL)
	{
		aprintf("Cannot find user with object id %i.\n",object_id);
		return;
	}

	admin_check_user = u->object_id;
	admin_user_is_logged_in = False;
	ForEachSession(AdminCheckUserLoggedOn);

	if (admin_user_is_logged_in)
	{
		aprintf("Someone is logged in as that user; you must kick them off before deleting them\n");
		return;
	}

	AdminDeleteEachUserObject(u);
	aprintf("Deleting the %i user.\n",DeleteUserByObjectID(u->object_id));
}

void AdminCheckUserLoggedOn(session_node *s)
{
	if (s && s->state == STATE_GAME && s->game && s->game->object_id == admin_check_user)
		admin_user_is_logged_in = True;
}

void AdminSendObject(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[])
{
	int message_id;
	val_type blak_val;
	object_node *o;
	message_node *m;
	const char* tag;
	const char* data;

	int object_id;
	const char *message_name;
	object_id = (int)parms[0];
	message_name = (char *)parms[1];

	DoneLoadAccounts();

	/* need to sort blak_parm */

	o = GetObjectByID(object_id);
	if (o == NULL)
	{
		aprintf("Invalid object id %i (or it has been deleted).\n",
			object_id);
		return;
	}

	message_id = GetIDByName(message_name);
	if (message_id == INVALID_ID)
	{
		aprintf("Cannot find message '%s'.\n",message_name);
		return;
	}

	m = GetMessageByID(o->class_id,message_id,NULL);
	if (m == NULL)
	{
		aprintf("OBJECT %i can't handle MESSAGE %i %s.\n",
			object_id,message_id,message_name);
		return;
	}
	data = GetNameByID(m->message_id);
	if (data)
		message_name = data;

		/* Send the message and handle any posted messages spawned, also.
    */
	blak_val.int_val = SendTopLevelBlakodMessage(object_id,message_id,num_blak_parm,blak_parm);

	/* Note that o may be invalid from here if we needed to resize our object array
    * mid-message.  Messages that could create a ton of objects could do that, such
	* as Meridian's RecreateAll() message.
	*/

	tag = GetTagName(blak_val);
	data = GetDataName(blak_val);
	aprintf(":< return from OBJECT %i MESSAGE %s (%i)\n", object_id,message_name,m->message_id);
	aprintf(": %s %s\n",(tag? tag : "UNKNOWN"),(data? data : "UNKNOWN"));

	if (tag && data)
	{

		if (blak_val.v.tag == TAG_STRING || blak_val.v.tag == TAG_TEMP_STRING)
		{
			string_node* snod =
				(blak_val.v.tag == TAG_STRING)?
				GetStringByID(blak_val.v.data) :
			GetTempString();
			int len;
			if (snod && snod->len_data)
			{
			  len = std::min(snod->len_data, 60);
			  aprintf(":   == \"");
			  AdminBufferSend(snod->data,len);
			  if (len < snod->len_data)
			    aprintf("...");
			  aprintf("\"\n");
			}
		}
		else if (blak_val.v.tag == TAG_RESOURCE)
		{
			resource_node* rnod = GetResourceByID(blak_val.v.data);
			if (rnod && rnod->resource_val && *rnod->resource_val)
			{
        int len = std::min((int) strlen(rnod->resource_val), 60);
			  aprintf(":   == \"");
			  AdminBufferSend(rnod->resource_val, len);
			  if (len < (int)strlen(rnod->resource_val))
			    aprintf("...");
			  aprintf("\"\n");
			}
		}
		else if (blak_val.v.tag == TAG_OBJECT)
		{
			object_node* onod = GetObjectByID(blak_val.v.data);
			if (onod)
			{
				class_node* cnod = GetClassByID(onod->class_id);
				aprintf(":   is CLASS %s (%i)\n", (cnod? cnod->class_name : "<invalid>"), (onod->class_id));
			}
		}

	}

	aprintf(":>\n");
}

void AdminSendUsers(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm_unused[])
{
	val_type str_val;
	parm_node blak_parm[1];

	char *text;
	text = (char *)parms[0];

	SetTempString(text, (int) strlen(text));
	str_val.v.tag = TAG_TEMP_STRING;
	str_val.v.data = 0;		/* doesn't matter for TAG_TEMP_STRING */

	blak_parm[0].type = CONSTANT;
	blak_parm[0].value = str_val.int_val;
	blak_parm[0].name_id = STRING_PARM;

	SendTopLevelBlakodMessage(GetSystemObjectID(),SYSTEM_STRING_MSG,1,blak_parm);
	aprintf("Sent to gamers: '%s'.\n",text);
}

void AdminSendClass(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[])
{
	int message_id;
	message_node *m;
	class_node *c;
	int executed;

	const char *class_name,*message_name;
	class_name = (char *)parms[0];
	message_name = (char *)parms[1];

	DoneLoadAccounts();

	/* need to sort blak_parm */

	message_name = (char *)parms[1];

	c = GetClassByName(class_name);
	if (c == NULL)
	{
		aprintf("Cannot find class '%s'.\n",class_name);
		return;
	}

	message_id = GetIDByName(message_name);
	if (message_id == INVALID_ID)
	{
		aprintf("Cannot find message '%s'.\n",message_name);
		return;
	}

	m = GetMessageByID(c->class_id,message_id,NULL);
	if (m == NULL)
	{
		aprintf("CLASS %i %s cannot handle MESSAGE %i %s.\n",
			c->class_id,c->class_name,message_id,message_name);
		return;
	}
	message_name = GetNameByID(message_id);

	executed = SendBlakodClassMessage(c->class_id,message_id,num_blak_parm,blak_parm);

	aprintf(":< %i instance(s) sent MESSAGE %i %s\n:>\n", executed, message_id, message_name);
}

void AdminTraceOnMessage(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[])
{
	class_node *c;
	int message_id;
	message_node *m;

	const char *class_name,*message_name;
	class_name = (char *)parms[0];
	message_name = (char *)parms[1];

	c = GetClassByName(class_name);
	if (c == NULL)
	{
		aprintf("Cannot find CLASS %s.\n",class_name);
		return;
	}

	message_id = GetIDByName(message_name);
	if (message_id == INVALID_ID)
	{
		aprintf("Cannot find MESSAGE %s.\n",message_name);
		return;
	}

	message_name = GetNameByID(message_id);
	m = GetMessageByID(c->class_id,message_id,NULL);
	if (m == NULL)
	{
		aprintf("Cannot find MESSAGE %s (%i) for CLASS %s (%i).\n",
			message_name,message_id,c->class_name,c->class_id);
		return;
	}

	m->trace_session_id = session_id;

}

void AdminTraceOffMessage(int session_id,admin_parm_type parms[],
                          int num_blak_parm,parm_node blak_parm[])
{
	class_node *c;
	int message_id;
	message_node *m;

	const char *class_name,*message_name;
	class_name = (char *)parms[0];
	message_name = (char *)parms[1];

	c = GetClassByName(class_name);
	if (c == NULL)
	{
		aprintf("Cannot find CLASS %s.\n",class_name);
		return;
	}

	message_id = GetIDByName(message_name);
	if (message_id == INVALID_ID)
	{
		aprintf("Cannot find MESSAGE %s.\n",message_name);
		return;
	}

	message_name = GetNameByID(message_id);
	m = GetMessageByID(c->class_id,message_id,NULL);
	if (m == NULL)
	{
		aprintf("Cannot find MESSAGE %s (%i) for CLASS %s (%i).\n",
			message_name,message_id,c->class_name,c->class_id);
		return;
	}

	m->trace_session_id = INVALID_ID;
}

void AdminAddCredits(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[])
{
	account_node *a;

	int account_id;
	int credits;
	account_id = (int)parms[0];
	credits = (int)parms[1];

	a = GetAccountByID(account_id);
	if (a == NULL)
	{
		aprintf("Cannot find ACCOUNT %i.\n",account_id);
		return;
	}
	lprintf("AdminAddAccount adding %i credits to ACCOUNT %i (%s)\n",credits,account_id,a->name);
	a->credits += 100*credits;
}

void AdminKickoffAll(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[])
{
	lprintf("AdminKickoffAll kicking everyone out of the game\n");
	ForEachSession(AdminKickoffEachSession);
}

void AdminKickoffEachSession(session_node *s)
{
	if (s->account == NULL)
		return;

	if (s->state != STATE_GAME)
		return;

	GameClientExit(s);
	SetSessionState(s,STATE_SYNCHED);
}

void AdminKickoffAccount(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[])
{
	account_node *a;
	session_node *kickoff_session;

	int account_id;
	account_id = (int)parms[0];

	a = GetAccountByID(account_id);
	if (a == NULL)
	{
		aprintf("Cannot find ACCOUNT %i.\n",account_id);
		return;
	}

	kickoff_session = GetSessionByAccount(a);

	if (kickoff_session == NULL)
	{
		aprintf("ACCOUNT %i (%s) is not logged on.\n",account_id,a->name);
		return;
	}
	if (kickoff_session->state != STATE_GAME)
	{
		aprintf("ACCOUNT %i (%s) is not in the game.\n",account_id,a->name);
		return;
	}

	lprintf("AdminKickoffAccount kicking ACCOUNT %i (%s) out of the game\n",account_id,a->name);
	GameClientExit(kickoff_session);
	SetSessionState(kickoff_session,STATE_SYNCHED);
}

void AdminHangupAll(int session_id,admin_parm_type parms[],
                    int num_blak_parm,parm_node blak_parm[])
{
	lprintf("AdminHangupAll hanging up everyone\n");
	ForEachSession(AdminHangupEachSession);
}

void AdminHangupEachSession(session_node *s)
{
	HangupSession(s);
}

void AdminHangupUser(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[])
{
	user_node *u;
	account_node *a;
	session_node *hangup_session;
	int id;
	char *arg_str;
	Bool is_by_number;
	char *ptr;

	arg_str = (char *)parms[0];
	is_by_number = True;

	ptr = arg_str;
	while (*ptr != 0)
	{
		if (*ptr < '0' || *ptr > '9')
			is_by_number = False;
		ptr++;
	}

	if (is_by_number)
	{
		sscanf(arg_str,"%i",&id);
		u = GetUserByObjectID(id);
	}
	else
	{
		u = GetUserByName(arg_str);
	}

	if (u == NULL)
	{
		aprintf("Cannot find user %s.\n",arg_str);
		return;
	}

	a = GetAccountByID(u->account_id);

	if (a == NULL)
	{
		aprintf("Cannot find account for user %s.\n",arg_str);
		return;
	}

	hangup_session = GetSessionByAccount(a);
	if (hangup_session == NULL)
	{
		aprintf("ACCOUNT %i (%s) is not logged in.\n",
			a->account_id, a->name);
		return;
	}

	if (session_id == hangup_session->session_id)
	{
		aprintf("Cannot hangup yourself.\n");
		return;
	}

	aprintf("ACCOUNT %i (%s) SESSION %i has been disconnected.\n",
		a->account_id, a->name, hangup_session->session_id);

	// Manually hanging up an account will block that IP address
	// from reconnecting for a short time (usually 5min).
	id = ConfigInt(SOCKET_BLOCK_TIME);
	AddBlock(id, &hangup_session->conn.addr);

	HangupSession(hangup_session);
}

extern block_node* FindBlock(struct in_addr* piaPeer);

/*
 * AdminBlockIP - Block an IP address from accessing this server
 */

void AdminBlockIP(int session_id,admin_parm_type parms[],
                  int num_blak_parm,parm_node blak_parm[])
{
	struct in_addr blocktoAdd;
	char *arg_str = (char *)parms[0];

	aprintf("This command will only affect specified IPs until the server reboots\n");

	if( ( blocktoAdd.s_addr = inet_addr( arg_str ) ) != -1 ) {
		if(FindBlock( &blocktoAdd ) == NULL )  {
			AddBlock(-1, &blocktoAdd);
			aprintf("IP %s blocked\n",inet_ntoa( blocktoAdd ) );
		} else {
			DeleteBlock( &blocktoAdd );
			aprintf("IP %s has been unblocked\n" ,inet_ntoa( blocktoAdd ) );
		}
	}  else {
		aprintf("Couldn`t build IP address bad format %s\n",arg_str );
	}
}

void AdminHangupAccount(int session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[])
{
	account_node *a;
	session_node *hangup_session;
	int id;
	char *arg_str;
	Bool is_by_number;
	char *ptr;

	arg_str = (char *)parms[0];
	is_by_number = True;

	ptr = arg_str;
	while (*ptr != 0)
	{
		if (*ptr < '0' || *ptr > '9')
			is_by_number = False;
		ptr++;
	}

	if (is_by_number)
	{
		sscanf(arg_str,"%i",&id);
		a = GetAccountByID(id);
	}
	else
	{
		a = GetAccountByName(arg_str);
	}

	if (a == NULL)
	{
		aprintf("Cannot find account %s.\n",arg_str);
		return;
	}

	hangup_session = GetSessionByAccount(a);
	if (hangup_session == NULL)
	{
		aprintf("ACCOUNT %i (%s) is not logged in.\n",a->account_id,a->name);
		return;
	}

	if (session_id == hangup_session->session_id)
	{
		aprintf("Cannot hangup yourself.\n");
		return;
	}

	aprintf("ACCOUNT %i (%s) SESSION %i has been disconnected.\n",
		a->account_id, a->name, hangup_session->session_id);

	// Manually hanging up an account will block that IP address
	// from reconnecting for a short time (usually 5min).
	id = ConfigInt(SOCKET_BLOCK_TIME);
	AddBlock(id, &hangup_session->conn.addr);

	HangupSession(hangup_session);
}

void AdminHangupSession(int admin_session_id,admin_parm_type parms[],
                        int num_blak_parm,parm_node blak_parm[])
{
	session_node *s;

	int session_id;
	session_id = (int)parms[0];

	s = GetSessionByID(session_id);
	if (s == NULL)
	{
		aprintf("Cannot find SESSION %i.\n",session_id);
		return;
	}

	if (admin_session_id == s->session_id)
	{
		aprintf("Cannot hangup on yourself.\n");
		return;
	}

	if (s->conn.type == CONN_CONSOLE)
	{
		aprintf("Cannot hangup the console.\n");
		return;
	}

	aprintf("SESSION %i has been disconnected.\n",s->session_id);

	HangupSession(s);
}

void AdminReloadSystem(int session_id,admin_parm_type parms[],
                       int num_blak_parm,parm_node blak_parm[])
{
	lprintf("AdminReloadSystem reloading system\n");

	PauseTimers();
	aprintf("Garbage collecting and saving game... ");

	SendBlakodBeginSystemEvent(SYSEVENT_RELOAD_SYSTEM);

	GarbageCollect();
	SaveAll();
	aprintf("done.\n");

	aprintf("Unloading game, kodbase, and .bof ... ");
	AdminSendBufferList();
	ResetAdminConstants();
	ResetUser();
	ResetString();
	ResetRoomData();
	ResetLoadMotd();
	ResetLoadBof();
	ResetDLlist();
	ResetNameID();
	ResetResource();
	ResetTimer();
	ResetList();
	ResetObject();
	ResetMessage();
	ResetClass();
	aprintf("done.\n");

	aprintf("Loading game, kodbase, and .bof ... ");
	AdminSendBufferList();

	LoadMotd();
	LoadBof();
	LoadRsc();

	LoadKodbase();

	UpdateSecurityRedbook();

	LoadAdminConstants();
	/* can't reload accounts because sessions have pointers to accounts */
	if (!LoadAllButAccount())
		eprintf("AdminReload couldn't load game.  You are dead.\n");

	AddBuiltInDLlist();

	AllocateParseClientListNodes(); /* it needs a list to send to users */
	SendBlakodEndSystemEvent(SYSEVENT_RELOAD_SYSTEM);

	aprintf("done.\n");

	UnpauseTimers();
}

/* stuff for foreachsession */
int accounts_in_game;
void AdminReloadGame(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[])
{
	int save_time;

	lprintf("AdminReloadGame\n");

	/* make sure no one in game */
	AdminHangupAll(session_id, parms, num_blak_parm, blak_parm);

	accounts_in_game = 0;
	ForEachSession(AdminReloadGameEachSession);
	if (accounts_in_game > 0)
	{
		aprintf("Cannot reload game because %i %s in it.\n",
			accounts_in_game,accounts_in_game == 1 ? "person is" : "people are");
		return;
	}

	aprintf("Unloading game... ");
	AdminSendBufferList();
	ResetRoomData();
	ResetUser();
	ResetString();
	ResetTimer();
	ResetList();
	ResetObject();
	aprintf("done.\n");
	AdminSendBufferList();

	save_time = (int)parms[0];
	if (save_time != 0)
	{
		lprintf("Game save time forced to (%i)\n", save_time);
		aprintf("Forcing game save time to (%i)... ", save_time);
		SaveControlFile(save_time);
		aprintf("done.\n");
	}

	aprintf("Loading game... ");
	AdminSendBufferList();

	/* can't reload accounts because sessions have pointers to accounts */
	if (!LoadAllButAccount())
		eprintf("AdminReload couldn't reload all, system dead\n");

	AllocateParseClientListNodes(); /* it needs a list to send to users */

	/* since it's an older saved game, tell Blakod that everyone's off */
	SendTopLevelBlakodMessage(GetSystemObjectID(),LOADED_GAME_MSG,0,NULL);

	aprintf("done.\n");
}

void AdminReloadGameEachSession(session_node *s)
{
	if (s->state == STATE_GAME)
		accounts_in_game++;
}

void AdminReloadMotd(int session_id,admin_parm_type parms[],
                     int num_blak_parm,parm_node blak_parm[])
{
	lprintf("AdminReloadMotd reloading message of the day\n");

	aprintf("Reloading motd... ");
	AdminSendBufferList();

	ResetLoadMotd();
	LoadMotd();

	aprintf("done.\n");
}

void AdminReloadPackages(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[])
{
	aprintf("Reloading packages... ");
	AdminSendBufferList();

	ResetDLlist();
	AddBuiltInDLlist();
	aprintf("done.\n");
}

void AdminDisableSysTimer(int session_id,admin_parm_type parms[],
                          int num_blak_parm,parm_node blak_parm[])
{

	int systimer_type;
	systimer_type = (int)parms[0];

	if (DisableSysTimer(systimer_type) == False)
		aprintf("Invalid systimer type %i.\n",systimer_type);
	else
		aprintf("Systimer disabled.\n");

}

void AdminEnableSysTimer(int session_id,admin_parm_type parms[],
                         int num_blak_parm,parm_node blak_parm[])
{

	int systimer_type;
	systimer_type = (int)parms[0];

	if (EnableSysTimer(systimer_type) == False)
		aprintf("Invalid systimer type %i.\n",systimer_type);
	else
		aprintf("Systimer enabled.\n");

}

static char *say_admin_text;
static int say_admin_session_id;
void AdminSay(int session_id,admin_parm_type parms[],
              int num_blak_parm,parm_node blak_parm[])
{
	char *text;
	text = (char *)parms[0];

	say_admin_text = text;
	say_admin_session_id = session_id;

	ForEachSession(AdminSayEachAdminSession);

	aprintf("Said.\n");
}

void AdminSayEachAdminSession(session_node *s)
{
	session_node *sender_session;
	const char* account;

	sender_session = GetSessionByID(say_admin_session_id);
	if (sender_session == NULL)
	{
		eprintf("AdminSayEachAdminSession no sender SESSION %i.\n",say_admin_session_id);
		return;
	}

	if (sender_session->session_id == s->session_id)
		return;

	if (s->state == STATE_ADMIN ||
		(s->state == STATE_GAME && s->account && s->account->type == ACCOUNT_ADMIN))
	{
		account = "Administrator";
		if (sender_session->account &&
			sender_session->account->name &&
			*sender_session->account->name)
		{
			account = sender_session->account->name;
		}

		SendSessionAdminText(s->session_id,
			"%s says, \"%s\"\n",
			account,say_admin_text);
	}
}

void AdminRead(int session_id,admin_parm_type parms[],
               int num_blak_parm,parm_node blak_parm[])
{
	FILE *fptr;
	char line[2000];
	char *ptr;
	static int admin_in_read = False;

	char *filename;
	filename = (char *)parms[0];

	if (admin_in_read)
	{
		aprintf("Recursive read or script command not allowed.\n");
		return;
	}

	fptr = fopen(filename,"rt");
	if (fptr == NULL)
	{
		aprintf("Error opening %s.\n",filename);
		return;
	}

	admin_in_read = True;

	while (fgets(line,sizeof(line)-1,fptr))
	{
		ptr = strtok(line,"\n");
		if (ptr)
		{
			while (*ptr == ' ' || *ptr == '\t')
				ptr++;

			if (*ptr)
			{
				aprintf(">> %s\n",ptr);
				DoAdminCommand(ptr);
				AdminSendBufferList();
			}
		}
	}

	fclose(fptr);

	admin_in_read = False;
}

void AdminMark(int session_id,admin_parm_type parms[],
               int num_blak_parm,parm_node blak_parm[])
{
	lprintf("-------------------------------------------------------------------------------------\n");
	dprintf("-------------------------------------------------------------------------------------\n");
	eprintf("-------------------------------------------------------------------------------------\n");
}
