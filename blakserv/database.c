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

*/

#include "database.h"
#include <stdio.h>

#define SLEEPTIME				10
#define SLEEPTIMENOCON			1000
#define MAX_RECORD_QUEUE		1000
#define RECORD_ENQUEUE_TIMEOUT	60
#define RECORD_DEQUEUE_TIMEOUT	60
#define MAX_ITEMS_UNTIL_LOOP	10
#define SLEEPINIT				100

sql_queue queue			= {0, 0, 0, 0};
HANDLE hMySQLWorker		= 0;
sql_worker_state state	= STOPPED;
MYSQL* mysql			= 0;
char* host				= 0;
char* user				= 0;
char* password			= 0;
char* db				= 0;

#pragma region SQL
#define SQLQUERY_CREATETABLE_MONEYTOTAL										"\
	CREATE TABLE `player_money_total`										\
	(																		\
	  `idplayer_money_total`		INT(11) NOT NULL AUTO_INCREMENT,		\
	  `player_money_total_time`		DATETIME NOT NULL,						\
	  `player_money_total_amount`	INT(11) NOT NULL,						\
	  PRIMARY KEY (`idplayer_money_total`)									\
	)																		\
	ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=latin1;"

#define SQLQUERY_CREATETABLE_PLAYERLOGINS									"\
	CREATE TABLE `player_logins`											\
	(																		\
	  `idplayer_logins`					INT(11) NOT NULL AUTO_INCREMENT,	\
	  `player_logins_account_name`		VARCHAR(45) NOT NULL,				\
	  `player_logins_character_name`	VARCHAR(45) NOT NULL,				\
	  `player_logins_IP`				VARCHAR(45) NOT NULL,				\
	  `player_logins_time`				DATETIME NOT NULL,					\
	  PRIMARY KEY (`idplayer_logins`)										\
	)																		\
	ENGINE=InnoDB DEFAULT CHARSET=latin1;"

#define SQLQUERY_CREATETABLE_MONEYCREATED							"\
	CREATE TABLE `money_created`									\
	(																\
	  `idmoney_created`			INT(11) NOT NULL AUTO_INCREMENT,	\
	  `money_created_amount`	INT(11) NOT NULL,					\
	  `money_created_time`		DATETIME NOT NULL,					\
	  PRIMARY KEY (`idmoney_created`)								\
	)																\
	ENGINE=InnoDB DEFAULT CHARSET=latin1;"

#define SQLQUERY_CREATETABLE_PLAYERDAMAGED							"\
	CREATE TABLE `player_damaged`									\
	(																\
	  `idplayer_damaged`		INT(11) NOT NULL AUTO_INCREMENT,	\
	  `player_damaged_who`		VARCHAR(45) NOT NULL,				\
	  `player_damaged_attacker` VARCHAR(45) NOT NULL,				\
	  `player_damaged_aspell`	INT(11) NOT NULL,					\
	  `player_damaged_atype`	INT(11) NOT NULL,					\
	  `player_damaged_applied`	INT(11) NOT NULL,					\
	  `player_damaged_original` INT(11)	NOT NULL,					\
	  `player_damaged_weapon`	VARCHAR(45) NOT NULL,				\
	  `player_damaged_time`		DATETIME NOT NULL,					\
	  PRIMARY KEY (`idplayer_damaged`)								\
	)																\
	ENGINE=InnoDB DEFAULT CHARSET=latin1;"

#define SQLQUERY_CREATEPROC_MONEYTOTAL				"\
	CREATE PROCEDURE WriteTotalMoney(				\n\
	IN total_money INT(11))							\n\
	BEGIN											\n\
	  INSERT INTO `player_money_total`				\n\
      SET											\n\
		`player_money_total_amount` = total_money,	\n\
		`player_money_total_time` = now();			\n\
	END"

#define SQLQUERY_CREATEPROC_MONEYCREATED		"\
	CREATE PROCEDURE WriteMoneyCreated(			\n\
	IN money_created INT(11))					\n\
	BEGIN										\n\
	  INSERT INTO `money_created`				\n\
      SET										\n\
		`money_created_amount` = money_created,	\n\
		`money_created_time` = now();			\n\
	END"

#define SQLQUERY_CREATEPROC_PLAYERLOGIN				"\
	CREATE PROCEDURE WritePlayerLogin(				\n\
	  IN account	VARCHAR(45),					\n\
	  IN charname	VARCHAR(45),					\n\
	  IN ip			VARCHAR(45))					\n\
	BEGIN											\n\
	  INSERT INTO `player_logins`					\n\
      SET											\n\
		`player_logins_account_name` = account,		\n\
		`player_logins_character_name` = charname,	\n\
		`player_logins_IP` = ip,					\n\
		`player_logins_time` = now();				\n\
	END"

#define SQLQUERY_CREATEPROC_PLAYERASSESSDAMAGE "\
	CREATE PROCEDURE WritePlayerAssessDamage(	\n\
	  IN who		VARCHAR(45),				\n\
	  IN attacker	VARCHAR(45),				\n\
	  IN aspell		INT(11),					\n\
	  IN atype		INT(11),					\n\
	  IN applied	INT(11),					\n\
	  IN original	INT(11),					\n\
	  IN weapon		VARCHAR(45))				\n\
	BEGIN										\n\
	  INSERT INTO `player_damaged`				\n\
      SET										\n\
		`player_damaged_who` = who,				\n\
		`player_damaged_attacker` = attacker,	\n\
		`player_damaged_aspell` = aspell,		\n\
		`player_damaged_atype` = atype,			\n\
		`player_damaged_applied` = applied,		\n\
		`player_damaged_original` = original,	\n\
		`player_damaged_weapon` = weapon,		\n\
		`player_damaged_time` = now();			\n\
	END"

#define SQLQUERY_CALL_WRITETOTALMONEY			"CALL WriteTotalMoney(?);"
#define SQLQUERY_CALL_WRITEMONEYCREATED			"CALL WriteMoneyCreated(?);"
#define SQLQUERY_CALL_WRITEPLAYERLOGIN			"CALL WritePlayerLogin(?,?,?);"
#define SQLQUERY_CALL_WRITEPLAYERASSESSDAMAGE	"CALL WritePlayerAssessDamage(?,?,?,?,?,?,?);"

#define SQLQUERY_DROPPROC_TOTALMONEY			"DROP PROCEDURE IF EXISTS WriteTotalMoney;"
#define SQLQUERY_DROPPROC_MONEYCREATED			"DROP PROCEDURE IF EXISTS WriteMoneyCreated;"
#define SQLQUERY_DROPPROC_PLAYERLOGIN			"DROP PROCEDURE IF EXISTS WritePlayerLogin;"
#define SQLQUERY_DROPPROC_PLAYERASSESSDAMAGE	"DROP PROCEDURE IF EXISTS WritePlayerAssessDamage;"
#pragma endregion

#pragma region Public
void MySQLInit(char* Host, char* User, char* Password, char* DB)
{	
	// don't init if already running or a parameter is nullptr
	if (state != STOPPED || !Host || !User || !Password || !DB)
		return;
	
	// set state to starting
	state = STARTING;

	// save connection info
	host		= _strdup(Host);
	user		= _strdup(User);
	password	= _strdup(Password);
	db			= _strdup(DB);

	// spawn workthread
	hMySQLWorker = (HANDLE) _beginthread(_MySQLWorker, 0, 0);

	// sleep a bit to let workthread get initialized
	// e.g. no one calls MySQLRecordXY right after Init
	Sleep(SLEEPINIT);
};

void MySQLEnd()
{
	// set state to stopping
	state = STOPPING;
};

BOOL MySQLRecordTotalMoney(int total_money)
{
	BOOL					enqueued;
	sql_record_totalmoney*	record;
	sql_queue_node*			node;
	
	if (state == 0)
		return FALSE;

	// allocate
	record	= (sql_record_totalmoney*)malloc(sizeof(sql_record_totalmoney));
	node	= (sql_queue_node*)malloc(sizeof(sql_queue_node));
	
	// set values
	record->total_money = total_money;
	
	// attach to node
	node->type = STAT_TOTALMONEY;
	node->data = record;

	// try to enqueue
	enqueued = _MySQLEnqueue(node);

	// cleanup in case of fail
	if (!enqueued)
	{
		free(record);
		free(node);
	}

	return enqueued;
};

BOOL MySQLRecordMoneyCreated(int money_created)
{
	BOOL						enqueued;	
	sql_record_moneycreated*	record;
	sql_queue_node*				node;
	
	if (state == 0)
		return FALSE;

	// allocate
	record	= (sql_record_moneycreated*)malloc(sizeof(sql_record_moneycreated));
	node	= (sql_queue_node*)malloc(sizeof(sql_queue_node));
	
	// set values
	record->money_created = money_created;
	
	// attach to node
	node->type = STAT_MONEYCREATED;
	node->data = record;

	// try to enqueue
	enqueued = _MySQLEnqueue(node);

	// cleanup in case of fail
	if (!enqueued)
	{
		free(record);
		free(node);
	}

	return enqueued;
};

BOOL MySQLRecordPlayerLogin(char* account, char* character, char* ip)
{
	BOOL					enqueued;	
	sql_record_playerlogin* record;
	sql_queue_node*			node;

	if (state == 0 || !account || !character || !ip)
		return FALSE;

	// allocate
	record	= (sql_record_playerlogin*)malloc(sizeof(sql_record_playerlogin));
	node	= (sql_queue_node*)malloc(sizeof(sql_queue_node));

	// set values
	record->account		= _strdup(account);
	record->character	= _strdup(character);
	record->ip			= _strdup(ip);
	
	// attach to node
	node->type = STAT_PLAYERLOGIN;
	node->data = record;

	// try to enqueue
	enqueued = _MySQLEnqueue(node);

	// cleanup in case of fail
	if (!enqueued)
	{
		free(record->account);
		free(record->character);
		free(record->ip);

		free(record);
		free(node);
	}
	
	return enqueued;
};

BOOL MySQLRecordPlayerAssessDamage(char* who, char* attacker, int aspell, int atype, int applied, int original, char* weapon)
{
	BOOL							enqueued;	
	sql_record_playerassessdamage*	record;
	sql_queue_node*					node;
	
	if (state == 0 || !who || !attacker || !weapon)
		return FALSE;

	// allocate
	record	= (sql_record_playerassessdamage*)malloc(sizeof(sql_record_playerassessdamage));
	node	= (sql_queue_node*)malloc(sizeof(sql_queue_node));
	
	// set values
	record->who			= _strdup(who);
	record->attacker	= _strdup(attacker);
	record->aspell		= aspell;
	record->atype		= atype;
	record->applied		= applied;
	record->original	= original;
	record->weapon		= _strdup(weapon);

	// attach to node
	node->type = STAT_ASSESS_DAM;
	node->data = record;

	// try to enqueue
	enqueued = _MySQLEnqueue(node);

	// cleanup in case of fail
	if (!enqueued)
	{
		free(record->who);
		free(record->attacker);
		free(record->weapon);

		free(record);
		free(node);
	}
	
	return enqueued;
};
#pragma endregion

#pragma region Internal
void __cdecl _MySQLWorker(void* parameters)
{
	my_bool reconnect		= 1;
	int		processedcount	= 0;

	/******************************************
	/*            Initialization
	/******************************************/
		
	// init the queue
	queue.mutex = CreateMutex(NULL, FALSE, NULL);
	queue.count = 0;
	queue.first = 0;
	queue.last = 0;

	// init mysql
	mysql = mysql_init(NULL);
	
	if (!mysql)
		return;
	
	// enable auto-reconnect
	mysql_options(mysql, MYSQL_OPT_RECONNECT, &reconnect);

	// check if MySQLEnd was somehow already called and switched state to STOPPING
	// we still expect STARTING here and if so go to INITIALIZED
	if (state == STARTING)
		state = INITIALIZED;

	/******************************************
	/*              Work loop
	/******************************************/
	while(state != STOPPING)
	{	
		/**** Something went wrong with mysql ****/
		if (!mysql)		
			state = STOPPING;
				
		/****** No initial connection yet *****/
		else if (state == INITIALIZED)
		{
			// try connect
			if (mysql == mysql_real_connect(mysql, host, user, password, db, 0, NULL, 0))
				state = CONNECTED;

			// not successful, sleep
			else			
				Sleep(SLEEPTIMENOCON);			
		}

		/****** At least was connected once *****/
		else if (state >= CONNECTED)
		{
			// verify connection is still up
			// this will auto-reconnect
			if (mysql_ping(mysql) == 0)
			{
				// schema not yet verified/created
				if (state == CONNECTED)
					_MySQLVerifySchema();

				// fully running
				else if (state >= SCHEMAVERIFIED)
				{
					// reset counter for processed items
					processedcount = 0;

					// process up to limit pending items
					while(_MySQLDequeue(TRUE))
					{
						processedcount++;

						// reached limit, do more next loop
						// to get updated state etc.
						if (processedcount >= MAX_ITEMS_UNTIL_LOOP)
							break;						
					}
					
					// defaultsleep if none processed
					if (processedcount == 0)
						Sleep(SLEEPTIME);					
				}
			}
			else
				Sleep(SLEEPTIMENOCON);	
		}
	}

	/******************************************
	/*              Shutdown
	/******************************************/

	// close mysql
	if (mysql)
		mysql_close(mysql);

	// free local connection strings
	if (host)
		free(host);
	
	if (user)
		free(user);
	
	if (password)
		free(password);
	
	if (db)
		free(db);

	// clear queue
	while(_MySQLDequeue(FALSE));

	// delete mutex
	if (queue.mutex)
		CloseHandle(queue.mutex);

	// mark stopped
	state = STOPPED;

	// finalize thread
	_endthread();
};

void _MySQLVerifySchema()
{
	if (!mysql || !state)
		return;

	// create table (won't do it if exist)
	mysql_query(mysql, SQLQUERY_CREATETABLE_MONEYTOTAL);
	mysql_query(mysql, SQLQUERY_CREATETABLE_MONEYCREATED);
	mysql_query(mysql, SQLQUERY_CREATETABLE_PLAYERLOGINS);
	mysql_query(mysql, SQLQUERY_CREATETABLE_PLAYERDAMAGED);

	// drop procedures
	mysql_query(mysql, SQLQUERY_DROPPROC_TOTALMONEY);
	mysql_query(mysql, SQLQUERY_DROPPROC_MONEYCREATED);
	mysql_query(mysql, SQLQUERY_DROPPROC_PLAYERLOGIN);
	mysql_query(mysql, SQLQUERY_DROPPROC_PLAYERASSESSDAMAGE);
	
	// recreate them
	mysql_query(mysql, SQLQUERY_CREATEPROC_MONEYTOTAL);
	mysql_query(mysql, SQLQUERY_CREATEPROC_MONEYCREATED);
	mysql_query(mysql, SQLQUERY_CREATEPROC_PLAYERLOGIN);
	mysql_query(mysql, SQLQUERY_CREATEPROC_PLAYERASSESSDAMAGE);

	// set state to schema verified
	state = SCHEMAVERIFIED;
};

BOOL _MySQLEnqueue(sql_queue_node* Node)
{	
	BOOL enqueued = FALSE;

	if (!Node)
		return enqueued;

	// make sure it has no successor set
	Node->next = 0;

	// try to lock for multithreaded access
	if (WaitForSingleObject(queue.mutex, RECORD_ENQUEUE_TIMEOUT) == WAIT_OBJECT_0)
	{
		// got space in queue left
		if (queue.count < MAX_RECORD_QUEUE)
		{
			// empty queue
			if (queue.count == 0 ||
				queue.first == NULL ||
				queue.last == NULL)
			{
				queue.first = Node;
				queue.last = Node;
				queue.count = 1;
			}

			// not empty
			else
			{
				queue.last->next = Node;
				queue.last = Node;
				queue.count++;
			}

			enqueued = TRUE;
		}
		
		// release lock
		ReleaseMutex(queue.mutex);		
	}

	return enqueued;
};

BOOL _MySQLDequeue(BOOL processNode)
{
	BOOL			dequeued	= FALSE;
	sql_queue_node*	node		= 0;
			
	// try to lock for multithreaded access
	if (WaitForSingleObject(queue.mutex, RECORD_ENQUEUE_TIMEOUT) == WAIT_OBJECT_0)
	{
		// dequeue from the beginning (FIFO)
		if (queue.first)
		{
			// get first element to process
			node = queue.first;
			
			// update queue, set first item to the next one
			queue.first = node->next;
			queue.count--;

			// save that we dequeued a node
			dequeued = TRUE;			
		}
		
		// release lock
		ReleaseMutex(queue.mutex);		
	}
	
	// if we dequeued one, process it
	if (node)
	{
		// process node
		_MySQLWriteNode(node, processNode);

		// free memory of processed record and node
		free(node->data);
		free(node);			
	}

	return dequeued;
};

void _MySQLCallProc(char* ProcName, MYSQL_BIND Parameters[])
{
	MYSQL_STMT*	stmt;
	int status;

	if (!ProcName || !Parameters)
		return;

	// init statement
	stmt = mysql_stmt_init(mysql);
	
	if (!stmt)	
	  return;
	
	// execute stored procedure
	status = mysql_stmt_prepare(stmt, ProcName, strlen(ProcName));	
	status = mysql_stmt_bind_param(stmt, Parameters);
	status = mysql_stmt_execute(stmt);

	// close statement
	mysql_stmt_close(stmt);
};

void _MySQLWriteNode(sql_queue_node* Node, BOOL ProcessNode)
{
	if (!Node || !Node->data)
		return;

	switch(Node->type)
	{
		case STAT_TOTALMONEY:
			_MySQLWriteTotalMoney((sql_record_totalmoney*)Node->data, ProcessNode);
			break;

		case STAT_MONEYCREATED:
			_MySQLWriteMoneyCreated((sql_record_moneycreated*)Node->data, ProcessNode);
			break;

		case STAT_PLAYERLOGIN:
			_MySQLWritePlayerLogin((sql_record_playerlogin*)Node->data, ProcessNode);
			break;

		case STAT_ASSESS_DAM:
			_MySQLWritePlayerAssessDamage((sql_record_playerassessdamage*)Node->data, ProcessNode);
			break;
	}
};

void _MySQLWriteTotalMoney(sql_record_totalmoney* Data, BOOL ProcessNode)
{
	MYSQL_BIND params[1];
	
	// really write it, or just free mem at end?
	if (ProcessNode)
	{
		// allocate parameters
		memset(params, 0, sizeof (params));

		// set parameter 0
		params[0].buffer_type = MYSQL_TYPE_LONG;
		params[0].buffer = &Data->total_money;
		params[0].length = 0;
		params[0].is_null = 0;
	
		// call stored procedure
		_MySQLCallProc(SQLQUERY_CALL_WRITETOTALMONEY, params);
	}
};

void _MySQLWriteMoneyCreated(sql_record_moneycreated* Data, BOOL ProcessNode)
{
	MYSQL_BIND params[1];
	
	// really write it, or just free mem at end?
	if (ProcessNode)
	{
		// allocate parameters
		memset(params, 0, sizeof (params));

		// set parameter 0
		params[0].buffer_type = MYSQL_TYPE_LONG;
		params[0].buffer = &Data->money_created;
		params[0].length = 0;
		params[0].is_null = 0;
	
		// call stored procedure
		_MySQLCallProc(SQLQUERY_CALL_WRITEMONEYCREATED, params);
	}
};

void _MySQLWritePlayerLogin(sql_record_playerlogin* Data, BOOL ProcessNode)
{
	MYSQL_BIND params[3];	
	unsigned long len_acc	= (unsigned long)strlen(Data->account);
	unsigned long len_char	= (unsigned long)strlen(Data->character);
	unsigned long len_ip	= (unsigned long)strlen(Data->ip);
	
	// really write it, or just free mem at end?
	if (ProcessNode)
	{
		// allocate parameters
		memset(params, 0, sizeof (params));
	
		// set parameter 0
		params[0].buffer_type = MYSQL_TYPE_STRING;
		params[0].buffer = Data->account;
		params[0].length = &len_acc;
		params[0].is_null = 0;
	
		// set parameter 1
		params[1].buffer_type = MYSQL_TYPE_STRING;
		params[1].buffer = Data->character;
		params[1].length = &len_char;
		params[1].is_null = 0;
	
		// set parameter 2
		params[2].buffer_type = MYSQL_TYPE_STRING;
		params[2].buffer = Data->ip;
		params[2].length = &len_ip;
		params[2].is_null = 0;

		// call stored procedure
		_MySQLCallProc(SQLQUERY_CALL_WRITEPLAYERLOGIN, params);
	}

	// internal strings cleanup
	free(Data->account);
	free(Data->character);
	free(Data->ip);
};

void _MySQLWritePlayerAssessDamage(sql_record_playerassessdamage* Data, BOOL ProcessNode)
{
	MYSQL_BIND params[7];	
	unsigned long len_who		= (unsigned long)strlen(Data->who);
	unsigned long len_attacker	= (unsigned long)strlen(Data->attacker);
	unsigned long len_weapon	= (unsigned long)strlen(Data->weapon);
	
	// really write it, or just free mem at end?
	if (ProcessNode)
	{
		// allocate parameters
		memset(params, 0, sizeof (params));
	
		// set parameter 0
		params[0].buffer_type = MYSQL_TYPE_STRING;
		params[0].buffer = Data->who;
		params[0].length = &len_who;
		params[0].is_null = 0;
	
		// set parameter 1
		params[1].buffer_type = MYSQL_TYPE_STRING;
		params[1].buffer = Data->attacker;
		params[1].length = &len_attacker;
		params[1].is_null = 0;
	
		// set parameter 3
		params[2].buffer_type = MYSQL_TYPE_LONG;
		params[2].buffer = &Data->aspell;
		params[2].length = 0;
		params[2].is_null = 0;

		// set parameter 3
		params[3].buffer_type = MYSQL_TYPE_LONG;
		params[3].buffer = &Data->atype;
		params[3].length = 0;
		params[3].is_null = 0;
	
		// set parameter 4
		params[4].buffer_type = MYSQL_TYPE_LONG;
		params[4].buffer = &Data->applied;
		params[4].length = 0;
		params[4].is_null = 0;
	
		// set parameter 5
		params[5].buffer_type = MYSQL_TYPE_LONG;
		params[5].buffer = &Data->original;
		params[5].length = 0;
		params[5].is_null = 0;

		// set parameter 6
		params[6].buffer_type = MYSQL_TYPE_STRING;
		params[6].buffer = Data->weapon;
		params[6].length = &len_weapon;
		params[6].is_null = 0;
	
		// call stored procedure
		_MySQLCallProc(SQLQUERY_CALL_WRITEPLAYERASSESSDAMAGE, params);
	}

	// internal strings cleanup
	free(Data->who);
	free(Data->attacker);
	free(Data->weapon);
};
#pragma endregion
