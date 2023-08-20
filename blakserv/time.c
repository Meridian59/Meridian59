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
}

time_t GetTime()
{
	return time(NULL);
}

const char * TimeStr(time_t time)
{
	struct tm *tm_time;
	static char s[80];
	const char *time_format;
	
	if (time == 0)
		return "Never";
	
	tm_time = localtime(&time);
	
	if (tm_time == NULL)
		return "Invalid Time";
	
  time_format = "%b %d %Y %H:%M:%S";
	
	if (strftime(s,sizeof(s),time_format,tm_time) == 0)
		return "Time string too long";
	
	return s;
}

const char * ShortTimeStr(time_t time)
{
	struct tm *tm_time;
	static char s[80];
	const char *time_format;
	
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

const char * FileTimeStr(time_t time)
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

const char * RelativeTimeStr(time_t time)
{
	static char s[80];
	int amount;
	s[0] = 0;
	
	amount = (int) (time / (24*60*60));
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
#ifdef BLAK_PLATFORM_WINDOWS
   
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

#else

   struct timeval tv;
   gettimeofday(&tv, NULL);
   
   double time_in_ms = tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;

   return (UINT64) time_in_ms;
   
#endif
}

