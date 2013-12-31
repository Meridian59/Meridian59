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

#include "blakserv.h"

MYSQL *mysqlcon;

void MySQLTest()
{
	dprintf("MySQL client version: %s\n", mysql_get_client_info());
}

void MySQLInit()
{
	mysqlcon = mysql_init(NULL);

	if (mysqlcon == NULL)
	{
		dprintf("%s\n", mysql_error(mysqlcon));
		return;
	}

	if (mysql_real_connect(mysqlcon, ConfigStr(MYSQL_HOST), ConfigStr(MYSQL_USERNAME), ConfigStr(MYSQL_PASSWORD), NULL, 0, NULL, 0) == NULL) 
    {
      dprintf("%s\n", mysql_error(mysqlcon));
	  dprintf("host: %s user: %s pass: %s", ConfigStr(MYSQL_HOST), ConfigStr(MYSQL_USERNAME), ConfigStr(MYSQL_PASSWORD));
      mysql_close(mysqlcon);
      return;
    } 
	else
	{
		dprintf("connected to mysql host: %s user: %s", ConfigStr(MYSQL_HOST),ConfigStr(MYSQL_USERNAME) );
	}
    return;
}

int MySQLCheckSchema()
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
}

void MySQLCreateSchema()
{
	//void MySQLRecordStatTotalMoney(int total_money):
	if(mysql_query(mysqlcon, "CREATE TABLE `meridian`.`playermoneytotal` ( \
							 `idPlayerMoneyTotal` int(11) NOT NULL AUTO_INCREMENT, \
							 `PlayerMoneyTotalTime` datetime NOT NULL, \
							 `PlayerMoneyTotalAmount` int(11) NOT NULL, \
							 PRIMARY KEY (`idPlayerMoneyTotal`) \
							 ) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=latin1;"))
	{

		dprintf("unable to create table playermoneytotal");
		return;
	}
	
	//MySQLRecordPlayerLogin(session_node *s)
	if(mysql_query(mysqlcon, "CREATE TABLE `player_logins` ( \
							 `idplayer_logins` int(11) NOT NULL AUTO_INCREMENT, \
							 `player_logins_account_name` varchar(45) NOT NULL, \
							 `player_logins_character_name` varchar(45) NOT NULL, \
							 `player_logins_IP` varchar(45) NOT NULL, \
							 PRIMARY KEY (`idplayer_logins`) \
							 ) ENGINE=InnoDB DEFAULT CHARSET=latin1; "))
	{
		dprintf("unable to create table player_logins");
		return;
	}

	if(mysql_query(mysqlcon, "CREATE TABLE `money_created` ( \
							 `idmoney_created` int(11) NOT NULL AUTO_INCREMENT, \
							 `money_created_amount` int(11) NOT NULL, \
							 `money_created_time` datetime NOT NULL, \
							 PRIMARY KEY (`idmoney_created`) \
							 ) ENGINE=InnoDB DEFAULT CHARSET=latin1;"))
	{
		dprintf("unable to create table money_created");
		return;
	}
}

void MySQLRecordStatTotalMoney(int total_money)
{
	char buf[200];
	sprintf(buf,"INSERT INTO `meridian`.`playermoneytotal` \
				SET PlayerMoneyTotalAmount = %d, \
				PlayerMoneyTotalTime = NOW()",total_money);
	if(mysql_query(mysqlcon, buf))
	{
		dprintf("Unable to record StatTotalMoney");
		return;
	}
}

void MySQLRecordStatMoneyCreated(int money_created)
{
	char buf[200];
	sprintf(buf,"INSERT INTO `meridian`.`money_created` \
				SET money_created_amount = %d, \
				money_created_time = NOW()",money_created);
	if(mysql_query(mysqlcon, buf))
	{
		dprintf("Unable to record StatMoneyCreated");
		return;
	}
	dprintf("Recorded StatMoneyCreated");
}

void MySQLRecordPlayerLogin(session_node *s)
{
	//Log of characters, accounts, ips
	val_type name_val;
    resource_node *r;
	char buf[1000];
   
    name_val.int_val = SendTopLevelBlakodMessage(s->game->object_id,USER_NAME_MSG,0,NULL);
    r = GetResourceByID(name_val.v.data);
    //dprintf("Account %s (%d) logged in with character %s (%d) from ip %s",s->account->name,s->account->account_id,r->resource_val,s->game->object_id,s->conn.name);

	sprintf(buf,"INSERT INTO `meridian`.`player_logins` \
				SET player_logins_account_name = '%s', \
				player_logins_character_name = '%s', \
				player_logins_IP = '%s'",s->account->name,r->resource_val,s->conn.name);
	if(mysql_query(mysqlcon, buf))
	{
		dprintf("Unable to record StatPlayerLogin");
		return;
	}
}