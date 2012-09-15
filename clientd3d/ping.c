// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * ping.c:  Periodically send a "ping" message to the server, and measure the round
 *   trip time.  We present this information to the user as a measure of latency.
 */

#include "client.h"

#define PING_DELAY           5000       // # of milliseconds between pings

DWORD latency = 0;                      // Latest estimate of server latency, in milliseconds

static DWORD last_sent_time;    // Last time we sent a ping message to the server
static int timer_id;            // id of ping timer, 0 if none
/****************************************************************************/
/*
 * PingTimerStart:  Start timer to periodically check if a ping is needed.
 */
void PingTimerStart(void)
{
   timer_id = SetTimer(NULL, 0, (UINT) PING_DELAY, PingTimerProc);
}
/****************************************************************************/
/*
 * PingTimerAbort:  Start timer to periodically check if a ping is needed.
 */
void PingTimerAbort(void)
{
   if (timer_id != 0)
   {
      KillTimer(NULL, timer_id);
      timer_id = 0;
   }
}
/****************************************************************************/
/*
 * PingSentMessage:  Called when we send a message to the server; set last
 *   message time.
 */
void PingSentMessage(void)
{
   // In the old method of sending pings, we would send a ping after 20 seconds
   // of no other messages being sent.  Thus we needed to record the time whenever
   // a message was sent.
   //   last_msg_time = timeGetTime();
}
/****************************************************************************/
/*
 * PingGotReply:  We got a reply to our last BP_PING message.
 */
void PingGotReply(void)
{
   DWORD now;

   now = timeGetTime();
   latency = now - last_sent_time;

   //XXX
   //REVIEW: Should send out a ModuleEvent so any module can notice a new value.
   Lagbox_Update(latency);
}
/****************************************************************************/
/*
 * PingTimerProc:  Send a ping if necessary.  This function should be called occasionally
 *   to see if we need to send a ping.
 */
void CALLBACK PingTimerProc(HWND hwnd, UINT msg, UINT timer, DWORD dwTime)
{
   DWORD now;

   now = timeGetTime();

   if (now - last_sent_time >= PING_DELAY - 200 || last_sent_time == 0)
   {
      RequestGamePing();
      last_sent_time = now;
   }
}

