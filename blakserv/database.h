// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* database.h
*

 BASIC USAGE:
 --------------------------
  * You must not call any method marked with '_' prefix from outside database.c
  * You should call methods MySQLRecordXY to record data to DB
  * Do not start adding includes to blakserv.h or other M59 related headers/stuff

 STEPS TO ADD A RECORD:
 --------------------------
  1) Design a SQL table layout and add the SQL query text to section SQL
  2) Design a stored procedure and add the SQL query text to section SQL
  3) Add the queries from (1) and (2) to _MySQLVerifySchema()
  4) --- TEST IF THEY GET CREATED ---
  5) Add a sql_record_xy enum and typedef below (matching your table design)
     Note: Don't use anything except simple datatypes (int, float, ..) and char* as parameters.
  6) Add a new sql_recordtype below (same entry must go to kod\include\blakston.khd)
  7) Create a MySQLRecordXY method to enqueue this kind of record (see examples)
	 Note: You must _strdup any char* parameters
  8) Create a _MySQLWriteXY method to write this kind of record (see examples)
	 Note: You must free all _strdups here
  9) Add the _MySQLWriterXY case to the 'switch' in _MySQLWriteNode()

*/

#ifndef _DATABASE_H
#define _DATABASE_H

#include <Windows.h>
#include <mysql.h>
#include <process.h>

#pragma region Structs/Enums
typedef struct sql_queue_node sql_queue_node;
typedef struct sql_queue sql_queue;
typedef struct sql_record_totalmoney sql_record_totalmoney;
typedef struct sql_record_moneycreated sql_record_moneycreated;
typedef struct sql_record_playerlogin sql_record_playerlogin;
typedef struct sql_record_playerassessdamage sql_record_playerassessdamage;
typedef struct sql_record_playerdeath sql_record_playerdeath;
typedef struct sql_record_player sql_record_player;
typedef struct sql_record_playersuicide sql_record_playersuicide;
typedef struct sql_record_guild sql_record_guild;
typedef struct sql_record_guilddisband sql_record_guilddisband;
typedef enum sql_recordtype sql_recordtype;
typedef enum sql_worker_state sql_worker_state;

struct sql_queue_node
{
	sql_recordtype type;
	void* data;
	sql_queue_node* next;
};

struct sql_queue
{
	HANDLE mutex;
	int count;
	sql_queue_node* first;
	sql_queue_node* last;
};

struct sql_record_totalmoney
{
	int total_money;
};

struct sql_record_moneycreated
{
	int money_created;
};

struct sql_record_playerlogin
{
	char* account;
	char* character;
	char* ip;
};

struct sql_record_playerassessdamage
{
	char* who;
	char* attacker;
	int aspell;
	int atype;
	int applied;
	int original;
	char* weapon;
};

struct sql_record_playerdeath
{
   char* victim;
   char* killer;
   char* room;
   char* attack;
   int ispvp;
};

struct sql_record_player
{
   int account_id;
   char* name;
   char* home;
   char* bind;
   char* guild;
   int max_health;
   int max_mana;
   int might;
   int p_int;
   int myst;
   int stam;
   int aim;
   int agil;
};

struct sql_record_playersuicide
{
   int account_id;
   char* name;
};

struct sql_record_guild
{
   char* name;
   char* leader;
   char* hall;
};

struct sql_record_guilddisband
{
   char* name;
};

enum sql_recordtype
{
	STAT_TOTALMONEY		= 1,
	STAT_MONEYCREATED	= 2,
	STAT_PLAYERLOGIN	= 3,
	STAT_ASSESS_DAM		= 4,
   STAT_PLAYERDEATH = 5,
   STAT_PLAYER = 6,
   STAT_PLAYERSUICIDE = 7,
   STAT_GUILD = 8,
   STAT_GUILDDISBAND = 9
};

enum sql_worker_state
{
	STOPPED			= 0,
	STOPPING		= 1,
	STARTING		= 2,
	INITIALIZED		= 3,
	CONNECTED		= 4,
	SCHEMAVERIFIED	= 5
};
#pragma endregion

void MySQLInit(char* Host, char* User, char* Password, char* DB);
void MySQLEnd();
BOOL MySQLRecordTotalMoney(int total_money);
BOOL MySQLRecordMoneyCreated(int money_created);
BOOL MySQLRecordPlayerLogin(char* account, char* character, char* ip);
BOOL MySQLRecordPlayerAssessDamage(char* who, char* attacker, int aspell, int atype, int applied, int original, char* weapon);
BOOL MySQLRecordPlayerDeath(char* victim, char* killer, char* room, char* attack, int ispvp);
BOOL MySQLRecordPlayer(int account_id, char* name, char* home, char* bind, char* guild,
   int max_health, int max_mana, int might, int p_int, int myst,
   int stam, int agil, int aim);
BOOL MySQLRecordPlayerSuicide(int account_id, char* name);
BOOL MySQLRecordGuild(char* name, char* leader, char* hall);
BOOL MySQLRecordGuildDisband(char* name);

void __cdecl _MySQLWorker(void* Parameters);
void _MySQLVerifySchema();
BOOL _MySQLEnqueue(sql_queue_node* Node);
BOOL _MySQLDequeue(BOOL processNode);
void _MySQLCallProc(char* Name, MYSQL_BIND Parameters[]);
void _MySQLWriteNode(sql_queue_node* Node, BOOL ProcessNode);
void _MySQLWriteTotalMoney(sql_record_totalmoney* Data, BOOL ProcessNode);
void _MySQLWriteMoneyCreated(sql_record_moneycreated* Data, BOOL ProcessNode);
void _MySQLWritePlayerLogin(sql_record_playerlogin* Data, BOOL ProcessNode);
void _MySQLWritePlayerAssessDamage(sql_record_playerassessdamage* Data, BOOL ProcessNode);
void _MySQLWritePlayerDeath(sql_record_playerdeath* Data, BOOL ProcessNode);
void _MySQLWritePlayer(sql_record_player* Data, BOOL ProcessNode);
void _MySQLWritePlayerSuicide(sql_record_playersuicide* Data, BOOL ProcessNode);
void _MySQLWriteGuild(sql_record_guild* Data, BOOL ProcessNode);
void _MySQLWriteGuildDisband(sql_record_guilddisband* Data, BOOL ProcessNode);
#endif
