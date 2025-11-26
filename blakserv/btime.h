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
time_t GetTime();
std::string TimeStr(time_t time);
std::string ShortTimeStr(time_t time);
std::string FileTimeStr(time_t time);
std::string RelativeTimeStr(time_t time);
UINT64 GetMilliCount();

#endif

