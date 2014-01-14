// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* database.c
*

  MySQL!
	
*/

#include "blakserv.h"

MYSQL *mysqlcon;

HANDLE hConsumer;

record_queue_type * record_q;

bool connected = false;
bool enabled = false;
bool consume = true;


void CreateRecordQueue()
{
	record_q = (record_queue_type*)malloc(sizeof(record_queue_type));
	record_q->count = 0;
	record_q->first = NULL;
	record_q->last = NULL;

	record_q->mutex = CreateMutex(NULL,FALSE,NULL);

	if (record_q->mutex == NULL)
	{
		dprintf("Mutex poop");
	}
}

bool EnqueueRecord(record_node * data)
{
	bool wasQueued = false;

	DWORD dwWaitResult = WaitForSingleObject(record_q->mutex,RECORD_ENQUEUE_TIMEOUT);

	if (dwWaitResult == WAIT_OBJECT_0)
	{
		__try
		{
			if (record_q->count < MAX_RECORD_QUEUE)
			{
				if (record_q->count == 0 ||
					record_q->first == NULL ||
					record_q->last == NULL)
				{
					record_q->first = data;
					record_q->last = data;
					record_q->count = 1;
					wasQueued = true;
				}
				else
				{
					record_q->last->next = data;
					record_q->last = data;
					record_q->count++;
					wasQueued = true;
				}
			}
		}
		__finally
		{
			ReleaseMutex(record_q->mutex);
		}
	}
	return wasQueued;
}


bool DequeueRecord(record_node * data)
{
	bool wasDequeued = false;

	DWORD dwWaitResult = WaitForSingleObject(record_q->mutex,RECORD_DEQUEUE_TIMEOUT);

	if (dwWaitResult == WAIT_OBJECT_0)
	{
		__try 
		{
			// dequeue from the beginning (FIFO)
			if (record_q->count > 0 && record_q->first != NULL)
			{
				wasDequeued = true;

				// set data values
				data->type = record_q->first->type;
				data->data = record_q->first->data;

				// save pointer to old element for cleanup
				record_node* old = record_q->first;

				// set first item to the next one
				record_q->first = old->next;
				record_q->count--;

				// free memory of queueitem
				free(old);
			}
		}
		__finally 
		{
			// unlock
			ReleaseMutex(record_q->mutex);
		}
	}

	// return
	return wasDequeued; 
}


void __cdecl ConsumerThread(void *unused)
{
	dprintf("ConsumerThread()");
	// holds the one we're processing per step
	record_node* node = (record_node*)malloc(sizeof(record_node));
	node->type = 0;
	node->data = NULL;
	node->next = NULL;

	// this needs a proper shutdown condition when blakserv also ends
	// to shut down the consumer thread also
	while (consume)
	{
		// handle all pending
		while (DequeueRecord(node))
		{
			// cast depending on type
			switch(node->type)
			{
			case STAT_ASSESS_DAM:
				{
					PlayerAssessDamageRecord* data = (PlayerAssessDamageRecord*)node->data;
					MySQLRecordPlayerAssessDamage(
						data->res_who_damaged,
						data->res_who_attacker,
						data->aspell,
						data->atype,
						data->damage_applied,
						data->damage_original,
						data->res_weapon
						);
					free(data);
				}
				break;

			case STAT_MONEYCREATED:
				{
					MoneyCreatedRecord* data = (MoneyCreatedRecord*)node->data;
					MySQLRecordStatMoneyCreated(data->money_created);
					free(data);
				}
				break;
				
			case STAT_TOTALMONEY:
				{
					TotalMoneyRecord* data = (TotalMoneyRecord*)node->data;
					MySQLRecordStatTotalMoney(data->total_money);
					free(data);
				}
				break;
			}			
		}
		
		// sleep
		Sleep(1);
	}
	_endthread(); //Only reached after MySQLEnd()
}


void MySQLTest()
{
	dprintf("MySQL client version: %s\n", mysql_get_client_info());
}

void MySQLCheckConnection()
{
	if (!enabled)
		return;

	if (connected)
	{
		if (mysql_ping(mysqlcon) == 0)
		{
			connected = true;
			return;
		}
		else
		{
			connected = false;
			dprintf("MySQL connection dropped. Attempting Reconnect.");
			MySQLInit();
		}
	}
	else
		MySQLInit();

}

void MySQLInit()
{
	if (ConfigBool(MYSQL_ENABLED) == False)
		return;
	enabled = true;

	CreateRecordQueue();
	hConsumer = (HANDLE) _beginthread(ConsumerThread,0,0);

	mysqlcon = mysql_init(NULL);

	if (mysqlcon == NULL)
	{
		dprintf("%s\n", mysql_error(mysqlcon));
		return;
	}

	if (mysql_real_connect(mysqlcon, ConfigStr(MYSQL_HOST), ConfigStr(MYSQL_USERNAME), ConfigStr(MYSQL_PASSWORD), NULL, 0, NULL, 0) == NULL) 
    {
		dprintf("Error connecting: %s\n", mysql_error(mysqlcon));
		dprintf("host: %s user: %s pass: %s", ConfigStr(MYSQL_HOST), ConfigStr(MYSQL_USERNAME), ConfigStr(MYSQL_PASSWORD));
		mysql_close(mysqlcon);
		return;
    } 
	else
	{
		dprintf("connected to mysql host: %s user: %s", ConfigStr(MYSQL_HOST),ConfigStr(MYSQL_USERNAME) );
	}

	//Check for valid schema here...

	char buf[100];
	sprintf(buf, "USE %s", ConfigStr(MYSQL_DB));
	if(mysql_query(mysqlcon, buf))
	{
		dprintf("Error selecting database: %s\n", mysql_error(mysqlcon));
		return;
	}
	else
	{
		dprintf("connected to mysql database: %s", ConfigStr(MYSQL_DB));
	}
	connected = true;
    return;
}

int  MySQLCheckSchema()
{
	//SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = 'DBName'
	//CREATE DATABASE IF NOT EXISTS DBName;
	//SELECT table_name FROM information_schema.tables WHERE table_schema = 'databasename' AND table_name = 'tablename';
    return 0;
}

void MySQLEnd()
{
	//mysql_close(mysqlcon);
	//return;
	consume = false;
	return;
}

void MySQLCreateSchema()
{
	//void MySQLRecordStatTotalMoney(int total_money):
	if(mysql_query(mysqlcon, "CREATE TABLE `player_money_total` ( \
							 `idplayer_money_total` int(11) NOT NULL AUTO_INCREMENT, \
							 `player_money_total_time` datetime NOT NULL, \
							 `player_money_total_amount` int(11) NOT NULL, \
							 PRIMARY KEY (`idPlayerMoneyTotal`) \
							 ) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=latin1;"))
	{

		dprintf("unable to create table playermoneytotal");
	}
	
	//MySQLRecordPlayerLogin(session_node *s)
	if(mysql_query(mysqlcon, "CREATE TABLE `player_logins` ( \
							 `idplayer_logins` int(11) NOT NULL AUTO_INCREMENT, \
							 `player_logins_account_name` varchar(45) NOT NULL, \
							 `player_logins_character_name` varchar(45) NOT NULL, \
							 `player_logins_IP` varchar(45) NOT NULL, \
							 `player_logins_time` datetime NOT NULL, \
							 PRIMARY KEY (`idplayer_logins`) \
							 ) ENGINE=InnoDB DEFAULT CHARSET=latin1; "))
	{
		dprintf("unable to create table player_logins", mysql_error(mysqlcon));
	}

	if(mysql_query(mysqlcon, "CREATE TABLE `money_created` ( \
							 `idmoney_created` int(11) NOT NULL AUTO_INCREMENT, \
							 `money_created_amount` int(11) NOT NULL, \
							 `money_created_time` datetime NOT NULL, \
							 PRIMARY KEY (`idmoney_created`) \
							 ) ENGINE=InnoDB DEFAULT CHARSET=latin1;"))
	{
		dprintf("unable to create table money_created", mysql_error(mysqlcon));
	}

	if(mysql_query(mysqlcon, "CREATE TABLE `player_damaged` ( \
							 `idplayer_damaged` int(11) NOT NULL AUTO_INCREMENT, \
							 `player_damaged_who` varchar(45) NOT NULL, \
							 `player_damaged_attacker` varchar(45) NOT NULL, \
							 `player_damaged_aspell` int(11) NOT NULL, \
							 `player_damaged_atype` int(11) NOT NULL, \
							 `player_damaged_applied` int(11) NOT NULL, \
							 `player_damaged_original` int(11) NOT NULL, \
							 `player_damaged_weapon` varchar(45) NOT NULL, \
							 `player_damaged_time` datetime NOT NULL, \
							 PRIMARY KEY (`idplayer_damaged`) \
							 ) ENGINE=InnoDB DEFAULT CHARSET=latin1;"))
	{
		dprintf("unable to create table player_damaged", mysql_error(mysqlcon));
	}
}

void MySQLRecordStatTotalMoney(int total_money)
{
	MySQLCheckConnection();
	if (!connected | !enabled)
		return;

	char buf[200];
	sprintf(buf,"INSERT INTO `player_money_total` \
				SET player_money_total_amount = %d, \
				player_money_total_time = NOW()",total_money);

	

	if(mysql_query(mysqlcon, buf))
	{
		dprintf("Unable to record StatTotalMoney (%s)", mysql_error(mysqlcon));
		return;
	}
}

void MySQLRecordStatMoneyCreated(int money_created)
{
	MySQLCheckConnection();
	if (!connected | !enabled)
		return;

	char buf[200];
	sprintf(buf,"INSERT INTO `money_created` \
				SET money_created_amount = %d, \
				money_created_time = NOW()",money_created);

	if(mysql_query(mysqlcon, buf))
	{
		dprintf("Unable to record StatMoneyCreated (%s)", mysql_error(mysqlcon));
		return;
	}
}

void MySQLRecordPlayerLogin(session_node *s)
{
	MySQLCheckConnection();
	if (!connected | !enabled)
		return;

	//Log of characters, accounts, ips
	val_type name_val;
    resource_node *r;
	char buf[1000];
	
    name_val.int_val = SendTopLevelBlakodMessage(s->game->object_id,USER_NAME_MSG,0,NULL);
    r = GetResourceByID(name_val.v.data);
    //dprintf("Account %s (%d) logged in with character %s (%d) from ip %s",s->account->name,s->account->account_id,r->resource_val,s->game->object_id,s->conn.name);

	sprintf(buf,"INSERT INTO `player_logins` \
				SET player_logins_account_name = '%s', \
				player_logins_character_name = '%s', \
				player_logins_IP = '%s', \
				player_logins_time = NOW()",s->account->name,r->resource_val,s->conn.name);
	if(mysql_query(mysqlcon, buf))
	{
		dprintf("Unable to record StatPlayerLogin (%s)", mysql_error(mysqlcon));
		return;
	}
}

void MySQLRecordPlayerAssessDamage(int res_who_damaged, int res_who_attacker, int aspell, int atype, int damage_applied, int damage_original, int res_weapon)
{
	MySQLCheckConnection();
	if (!connected | !enabled)
		return;

	char buf[1200];
	resource_node *r_who_damaged, *r_who_attacker, *r_weapon;


	r_who_damaged = GetResourceByID(res_who_damaged);
	r_who_attacker = GetResourceByID(res_who_attacker);
	r_weapon = GetResourceByID(res_weapon);

	sprintf(buf,"INSERT INTO `player_damaged` SET player_damaged_who = '%s', player_damaged_attacker = '%s', player_damaged_aspell = %d, player_damaged_atype = %d, player_damaged_applied = %d, player_damaged_original = %d, player_damaged_weapon = '%s', player_damaged_time = NOW()",
		r_who_damaged->resource_val, r_who_attacker->resource_val, aspell, atype, damage_applied, damage_original, r_weapon->resource_val);
	if(mysql_query(mysqlcon,buf))
	{
		dprintf("Unable to record StatPlayerAssessDamage (%s)", mysql_error(mysqlcon));
		return;
	}
}