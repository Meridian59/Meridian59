// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * charstat.c:  Handle statistics tab page.
 */

#include "client.h"
#include "char.h"

typedef struct {
   int min;        // Minimum value of stat
   int max;        // Maximum value of stat
   int val;        // Current value of stat
   int cost;       // Cost of increasing stat by one point
   HWND hGraph;    // Handle of graph control for stat
} Stat;

static Stat stats[] = {
{ 1,   50,   25,   1,   NULL },       // Might
{ 1,   50,   25,   1,   NULL },       // Intellect
{ 1,   50,   25,   1,   NULL },       // Stamina
{ 1,   50,   25,   1,   NULL },       // Agility
{ 1,   50,   25,   1,   NULL },       // Mysticism
{ 1,   50,   25,   1,   NULL },       // Aim
};

#define NUM_STATS (sizeof stats / sizeof stats[0])

static int suggested_stats[][NUM_STATS] = {
   { 40, 10, 50, 40, 10, 50 },  // Pure Fighter (Weaponcraft/Kraanan)
   { 40, 25, 50, 15, 35, 35 },  // Shal'ille/Qor Warrior
   { 40, 35, 50, 25, 15, 35 },  // Riija Warrior
   { 40, 50, 45, 15, 45, 5  },  // Pure mage
};

static int  stat_points = STAT_POINTS_INITIAL;   // # of stat points remaining
static HWND hPoints;                             // Handle of "points left" graph control

static Bool controls_created = False;     // True after graph controls have been created

static WNDPROC lpfnDefGraphProc;  /* Default graph control window procedure */

static Stat *CharFindControl(HWND hwnd);
static void CharStatsInit(HWND hDlg);
static long CALLBACK StatGraphProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void CharStatsGraphChanging(HWND hDlg, WPARAM wParam, LPARAM lParam);
static void CharStatsCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
static void SetStatSliders(HWND hDlg, int *values);
/********************************************************************/
BOOL CALLBACK CharStatsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
   case WM_INITDIALOG:
      CharStatsInit(hDlg);
      break;
      
      HANDLE_MSG(hDlg, WM_COMMAND, CharStatsCommand);
      
   case WM_NOTIFY:
      switch (((LPNMHDR) lParam)->code)
      {
      case PSN_SETACTIVE:
         SendMessage(hPoints, GRPH_POSSET, 0, stat_points);
         break;
      }
      return TRUE;      
      
   case GRPHN_POSCHANGING:
      CharStatsGraphChanging(hDlg, wParam, lParam);
      break;
   }
   return FALSE;
}
/********************************************************************/
void CharStatsInit(HWND hDlg)
{
   int i;
   HWND hGraph;

   controls_created = False;
   stat_points = STAT_POINTS_INITIAL;
   // Initialize graph controls
   for (i=0; i < NUM_CHAR_STATS; i++)
   {
      hGraph = GetDlgItem(hDlg, IDC_CHAR_GRAPH1 + i);
      
      lpfnDefGraphProc = (WNDPROC) GetWindowLong(hGraph, GWL_WNDPROC);
      
      SendMessage(hGraph, GRPH_COLORSET, GRAPHCOLOR_BAR, GetColor(COLOR_BAR1));
      SendMessage(hGraph, GRPH_COLORSET, GRAPHCOLOR_BKGND, GetColor(COLOR_BAR3));
      SendMessage(hGraph, GRPH_COLORSET, GRAPHCOLOR_SLIDERBKGND, 
                  GetSysColor(COLOR_BTNFACE));  // CTL3D color

      SendMessage(hGraph, GRPH_RANGESET, stats[i].min, stats[i].max);
      SendMessage(hGraph, GRPH_POSSET, 0, stats[i].val);

      stats[i].hGraph = hGraph;

      SubclassWindow(hGraph, StatGraphProc); 
   }

   hPoints = GetDlgItem(hDlg, IDC_POINTSLEFT);
   SendMessage(hPoints, GRPH_COLORSET, GRAPHCOLOR_BAR, GetColor(COLOR_BAR2));
   SendMessage(hPoints, GRPH_COLORSET, GRAPHCOLOR_BKGND, GetColor(COLOR_BAR3));
   SendMessage(hPoints, GRPH_RANGESET, 0, STAT_POINTS_INITIAL);

   controls_created = True;
}
/********************************************************************/
/*
 * CharStatGraphChanging:  Handle GRAPHN_POSCHANGING message from graph control.
 */
void CharStatsGraphChanging(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
   Stat *s;
   int index, cost;

   /* Update points graph */
   s = CharFindControl((HWND) wParam);
   if (s == NULL || (HWND) wParam == hPoints)
      return;
   
   index = SendMessage((HWND) wParam, GRPH_POSGET, 0, 0);  // Get old value
   cost = s->cost * (lParam - index);
   
   s->val = lParam;
   
   // Don't change points when controls are being created
   if (controls_created)
      stat_points -= cost;
   SendMessage(hPoints, GRPH_POSSET, 0, stat_points);
}
/********************************************************************/
/*
 * CharStatsGetChoices:  Fill in stats with currently selected stat values.
 */
void CharStatsGetChoices(int *buf)
{
   int i;
   
   for (i=0; i < NUM_CHAR_STATS; i++)
      buf[i] = stats[i].val;
}
/********************************************************************/
/*
 * CharStatsGetPoints:  Return # of stat points remaining
 */
int CharStatsGetPoints(void)
{
  return stat_points;
}
/********************************************************************/
/* 
 * CharFindControl:  Return Stat structure for given graph control, or NULL if none. 
 */
Stat *CharFindControl(HWND hwnd)
{
   int i;

   for (i=0; i < NUM_CHAR_STATS; i++)
      if (stats[i].hGraph == hwnd)
         return &stats[i];
   
   return NULL;
}
/********************************************************************/
/*
 * StatGraphProc:  Subclassed window procedure for stat graph controls.
 */
long CALLBACK StatGraphProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   Stat *s;
   int new_pos, cur_pos;
   int cost;

   switch(msg)
   {
   case GRPH_POSSETUSER:
      /* See if user has enough points to make this change.  First, find stat */
      s = CharFindControl(hwnd);
      if (s == NULL)
      {
         debug(("Couldn't find graph control in list!\n"));
         return 0L;
      }
      
      cur_pos = SendMessage(hwnd, GRPH_POSGET, 0, 0);
      
      new_pos = lParam;
      cost = s->cost * (new_pos - cur_pos);
      
      if (cost > stat_points)
      {
         // Set value to greatest we can afford
         if (cost == 0)
            return 0;
         
         lParam = cur_pos + stat_points / s->cost;
         if (lParam == cur_pos)
            return 0;
      }
   }
   
   return CallWindowProc(lpfnDefGraphProc, hwnd, msg, wParam, lParam);
}
/********************************************************************/
/*
 * CharStatsCommand:  Handle WM_COMMAND messages.
 */
void CharStatsCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
   UserDidSomething();

   switch (id)
   {
   case IDC_WARRIOR:
      SetStatSliders(hwnd, suggested_stats[0]);
      break;

   case IDC_HYBRID:
      SetStatSliders(hwnd, suggested_stats[1]);
      break;
      
   case IDC_FAVORITES:
	   WebLaunchBrowser(GetString(hInst, IDS_FAVORITES));
      break; 
	  
   case IDC_MAGE:
      SetStatSliders(hwnd, suggested_stats[3]);
      break;

   default:
      CharTabPageCommand(hwnd, id, hwndCtl, codeNotify);
      return;
   }
}
/********************************************************************/
/*
 * SetStatSliders:  Set slider values to the given set.
 */
void SetStatSliders(HWND hDlg, int *values)
{
   int i;
   HWND hGraph;
   
   for (i = 0; i < NUM_STATS; ++i) {
      stats[i].val = values[i];
      hGraph = GetDlgItem(hDlg, IDC_CHAR_GRAPH1 + i);
      SendMessage(hGraph, GRPH_POSSET, 0, stats[i].val);
   }
   // Assume no stat points are left over
   SendMessage(hPoints, GRPH_POSSET, 0, stat_points);
}
