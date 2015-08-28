// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * time.h
 *
 */

#ifndef _BTIME_H
#define _BTIME_H

void InitTime();
int GetTime();
int GetTimeZoneOffset();

const char * TimeStr(time_t time);
const char * ShortTimeStr(time_t time);
const char * FileTimeStr(time_t time);
const char * RelativeTimeStr(int time);
UINT64 GetMilliCount();
double GetMicroCountDouble();

#endif

