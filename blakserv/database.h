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

 RULES:
 --------------------------
  * You must not call methods marked with '_' prefix from outside
  * Call methods MySQLRecordXY to record data to DB
  * Do not create MySQLRecordXY methods using anything else than simple datatypes and char*
  * Make sure you _strdup any outside string parameter you get before enqueue
  * Do not include blakserv.h or other M59 related headers/stuff

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
typedef enum sql_recordtype sql_recordtype;

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

enum sql_recordtype
{
  STAT_TOTALMONEY	= 1,
  STAT_MONEYCREATED = 2,
  STAT_PLAYERLOGIN	= 3,
  STAT_ASSESS_DAM	= 4
};
#pragma endregion

void MySQLInit(char* Host, char* User, char* Password, char* DB);
void MySQLEnd();
BOOL MySQLRecordTotalMoney(int total_money);
BOOL MySQLRecordMoneyCreated(int money_created);
BOOL MySQLRecordPlayerLogin(char* account, char* character, char* ip);
BOOL MySQLRecordPlayerAssessDamage(char* who, char* attacker, int aspell, int atype, int applied, int original, char* weapon);

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
#endif
