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

//Possible statistics that can be passed from blakod
enum
{
  STAT_TOTALMONEY;
};

void MySQLTest();
void MySQLInit();
int MySQLCheckSchema();
void MySQLCreateSchema();
void MySQLEnd();

#endif