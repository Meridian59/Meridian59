// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * time.c
 *

  This module has functions for getting time, either in seconds since
  like 1970 or milliseconds since Windows started.  They are NOT the
  same.  The only use of milliseconds is for relative times.  The seconds
  time (GetTime()) is good for recording when things happen, and there 
  are functions convert it to a string for you.
  
*/

#include "blakserv.h"

void InitTime()
{
/* I use the multimedia timers (set to accuracy of 1ms here) because
for millisecond timing because they are more accurate than 
	GetTickCount().
	
	timeBeginPeriod(1);
    */
}

int GetTime()
{
	return (int)time(NULL);
}

char * TimeStr(time_t time)
{
	struct tm *tm_time;
	static char s[80];
	char *time_format;
	
	if (time == 0)
		return "Never";
	
	tm_time = localtime(&time);
	
	if (tm_time == NULL)
		return "Invalid Time";
	
	if (tm_time->tm_mday < 10)
		time_format = "%b  %#d %Y %H:%M:%S";
	else
		time_format = "%b %#d %Y %H:%M:%S";
	
	if (strftime(s,sizeof(s),time_format,tm_time) == 0)
		return "Time string too long";
	
	return s;
}

char * ShortTimeStr(time_t time)
{
	struct tm *tm_time;
	static char s[80];
	char *time_format;
	
	if (time == 0)
		return "Never";
	
	tm_time = localtime(&time);
	
	if (tm_time->tm_mday < 10)
		time_format = "%b  %#d %H:%M";
	else
		time_format = "%b %#d %H:%M";
	
	if (strftime(s,sizeof(s),time_format,tm_time) == 0)
		return "Time string too long";
	
	return s;
}

char * FileTimeStr(time_t time)
{
	struct tm *tm_time;
	static char s[80];
	
	if (time == 0)
		return "Never";
	
	tm_time = localtime(&time);
	
	if (strftime(s,sizeof(s),"%y-%m-%d.%H-%M-%S",tm_time) == 0)
		return "TimeStringTooLong";
	
	return s;
}

char * RelativeTimeStr(int time)
{
	static char s[80];
	int amount;
	s[0] = 0;
	
	amount = time / (24*60*60);
	if (amount != 0)
		sprintf(s,"%i day%s ",amount,amount != 1 ? "s" : "");
	
	amount = (time / (60*60)) % 24;
	if (amount != 0)
		sprintf(s+strlen(s),"%i hour%s ",amount,amount != 1 ? "s" : "");
	
	amount = (time / 60) % 60;
	if (amount != 0)
		sprintf(s+strlen(s),"%i minute%s ",amount,amount != 1 ? "s" : "");
	
	amount = time % 60;
	if (amount != 0)
		sprintf(s+strlen(s),"%i second%s",amount,amount != 1 ? "s" : "");
	
	if (s[0] == 0)
		sprintf(s,"0 sec");
	
	return s;
}

UINT64 GetMilliCount()
{
	static LARGE_INTEGER frequency;
	LARGE_INTEGER now;

	if (frequency.QuadPart == 0)
		QueryPerformanceFrequency(&frequency);

	if (frequency.QuadPart == 0)
	{
		eprintf("GetMilliCount can't get frequency\n");
		return 0;
	}

	QueryPerformanceCounter(&now);
	return (now.QuadPart*1000)/frequency.QuadPart;

	/*
	// old version - simple and normal
	// return (int)timeGetTime();

	// force rollover for testing

	static int first = true;
	static unsigned int offset;
	static unsigned int prev_time = 0;
	unsigned int retval;
	unsigned int current_time = (int)timeGetTime();

	if (first)
	{
		first = false;
		offset = ~current_time - 20000;
	}

	retval = current_time + offset;

	if (retval < prev_time)
	{
		dprintf("ROLLOVER IN TIME %u %u\n",prev_time,retval);
	}
	prev_time = retval;

	return retval;
	*/
}

