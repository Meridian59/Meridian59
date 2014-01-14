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

  MySQL!
	
*/

#ifndef _DATABASE_H
#define _DATABASE_H

#define MAX_RECORD_QUEUE 100

typedef struct 
{
	int type;
	char *data;
} record_node;

typedef struct
{
	int next;
	int last;
	record_node data[MAX_RECORD_QUEUE];
} record_queue_type;

typedef struct
{
	int res_who_damaged;
	int res_who_attacker;
	int aspell;
	int atype;
	int damage_applied;
	int damage_original;
	int res_weapon;
}PlayerAssessDamageRecord;

void MySQLTest();
void MySQLInit();
int MySQLCheckSchema();
void MySQLCreateSchema();
void MySQLEnd();
void MySQLRecordStatTotalMoney(int total_money);
void MySQLRecordPlayerLogin(session_node *s);
void MySQLRecordStatMoneyCreated(int money_created);
void MySQLRecordPlayerAssessDamage(int res_who_damaged, int res_who_attacker, int aspell, int atype, int damage_applied, int damage_original, int res_weapon);

#endif