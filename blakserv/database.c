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

//MYSQL *mysqlcon;

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
	if(mysql_query(mysqlcon, "CREATE TABLE `meridian`.`player_strength` ( \
                              `idplayer_strength` int(11) NOT NULL AUTO_INCREMENT, \
                              `timestamp` timestamp NULL DEFAULT NULL, \
                              `player_name` varchar(45) DEFAULT NULL, \
                              `player_basemaxhp` varchar(45) DEFAULT NULL, \
                              `player_maxmana` varchar(45) DEFAULT NULL, \
                              PRIMARY KEY (`idplayer_strength`) \
                            ) ENGINE=InnoDB DEFAULT CHARSET=latin1;"))
	{
		dprintf("unable to create table player_strength");
		return;
	}
}

void MySQLRecordStatTotalMoney(int total_money)
{
	char buf[200];
	sprintf(buf,"INSERT INTO `meridian`.`playermoneytotal` \
				SET PlayerMoneyTotalAmount = %d, PlayerMoneyTotalTime = NOW()");
	if(mysql_query(mysqlcon, buf))
	{
		dprintf("Unable to record StatTotalMoney");
		return;
	}
	dprintf("Recorded StatTotalMoney");
}