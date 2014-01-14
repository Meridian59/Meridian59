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
#define RECORD_ENQUEUE_TIMEOUT 500
#define RECORD_DEQUEUE_TIMEOUT 500

typedef struct record_node record_node;

struct record_node
{
	int type;
	void* data;
	struct record_node *next;
};

typedef struct record_queue_type record_queue_type;

struct record_queue_type
{
	HANDLE mutex;
	int count;
	record_node* first;
	record_node* last;
};

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


//Queue a record
bool EnqueueRecord(record_node * data)

#endif