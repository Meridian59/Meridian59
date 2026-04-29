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
#include <vector>
#include <numeric>
#include <algorithm>

#define PING_DELAY           5000       // # of milliseconds between pings

DWORD latency = 0;                      // Latest estimate of server latency, in milliseconds

// Session ping tracking
static std::vector<int> session_ping;
static int ping_color_counts[3] = {0, 0, 0};                      // Latest estimate of server latency, in milliseconds

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

   Lagbox_Update(latency);

   // Track session ping
   if (latency <= 250)
      ping_color_counts[0]++; // green
   else if (latency <= 1000)
      ping_color_counts[1]++; // orange
   else
      ping_color_counts[2]++; // red

   if (session_ping.size() >= 36000)
      session_ping.erase(session_ping.begin());
   session_ping.push_back((int)latency);
}

void PingFlushPerfReport(void)
{
   if (!session_ping.empty())
   {
      double sum = std::accumulate(session_ping.begin(), session_ping.end(), 0.0);
      config.last_avg_ping = (int)(sum / session_ping.size());

      std::vector<int> sorted = session_ping;
      std::sort(sorted.begin(), sorted.end());

      // Best ping
      config.last_min_ping = sorted.front();

      // Worst 1% ping
      int high_count = std::max(1, (int)(sorted.size() / 100));
      double high_sum = std::accumulate(sorted.end() - high_count, sorted.end(), 0.0);
      config.last_high_ping = (int)(high_sum / high_count);

      // Overall color
      if (ping_color_counts[0] >= ping_color_counts[1] && ping_color_counts[0] >= ping_color_counts[2])
         config.last_ping_color = 0;
      else if (ping_color_counts[1] >= ping_color_counts[0] && ping_color_counts[1] >= ping_color_counts[2])
         config.last_ping_color = 1;
      else
         config.last_ping_color = 2;

      ConfigSave();
   }

   session_ping.clear();
   ping_color_counts[0] = 0;
   ping_color_counts[1] = 0;
   ping_color_counts[2] = 0;
}
/****************************************************************************/
/*
 * PingTimerProc:  Send a ping if necessary.  This function should be called occasionally
 *   to see if we need to send a ping.
 */
void CALLBACK PingTimerProc(HWND hwnd, UINT msg, UINT_PTR timer, DWORD dwTime)
{
   DWORD now;

   now = timeGetTime();

   if (now - last_sent_time >= PING_DELAY - 200 || last_sent_time == 0)
   {
      RequestGamePing();
      last_sent_time = now;
   }
}

