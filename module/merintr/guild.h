// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * guild.h:  Header file for guild.c
 */

#ifndef _GUILD_H
#define _GUILD_H

#define MAX_GUILD_USERS 400              // Max # of users in a guild
#define MAX_GUILD_NAME  30               // Max length of guild name

#define NUM_GUILD_RANKS 5                // Number of player ranks in a guild

#define MAX_RANK_LENGTH 20               // Max length of a rank name

// Guild command flags
#define GC_INVITE         0x00000001      // Invite
#define GC_EXILE          0x00000002      // Exile guild member
#define GC_RENOUNCE       0x00000004      // Renounce guild ties
#define GC_VOTE           0x00000020      // Vote for guild member
#define GC_ABDICATE       0x00000040      // Abdicate guildmaster position
#define GC_MAKE_ALLIANCE  0x00000100      // Make alliance with another guild
#define GC_END_ALLIANCE   0x00000200      // End guild alliance
#define GC_DECLARE_ENEMY  0x00000400      // Declare another guild as enemy
#define GC_END_ENEMY      0x00000800      // Make peace with enemy guild
#define GC_SET_RANK       0x00001000      // Set guild member's rank
#define GC_DISBAND        0x00002000      // Disband (destroy) guild
#define GC_ABANDON        0x00004000      // Abandon guild hall

// Constants for guild member genders
#define GUILD_MALE   1
#define GUILD_FEMALE 2


// Info on each guild member
typedef struct {
   ID id;                     // Object id of guild member
   char name[MAXUSERNAME];    // String name
   BYTE rank;                 // Rank (1 = lowest)
   BYTE gender;               // Gender (GUILD_MALE or GUILD_FEMALE)
} GuildMember;

typedef struct {
   char name[MAX_GUILD_NAME]; // Name of guild
   Bool has_password;             // True if guild has a password
   char password[MAX_GUILD_NAME]; // Guild password
   int  flags;
   ID   guild_id;             // Guild to which player belongs
   WORD num_users;
   ID   current_vote;         // Person currently voting for
   GuildMember members[MAX_GUILD_USERS];
   char male_ranks[NUM_GUILD_RANKS][MAX_RANK_LENGTH];    // Rank names for males
   char female_ranks[NUM_GUILD_RANKS][MAX_RANK_LENGTH];  // Rank names for females
   int  rank;                 // Player's rank
} GuildConfigDialogStruct;

// Info about someone else's guild
typedef struct {
   ID id;                          // Object id of guild
   char name[MAX_GUILD_NAME + 1];  // Name of guild
} GuildInfo;

// Info about a guild for sale
typedef struct {
   ID  id;             // Object id of guild hall
   ID  name_res;       // Name resource of guild hall
   int cost;           // Cost of purchasing guild hall
   int rent;           // Daily rent of hall
} GuildHall;

void GuildConfigInitInfo(GuildConfigDialogStruct *info);
void GuildCreate(int cost1, int cost2);
void GuildGotList(list_type guilds, IDList ally_guilds, IDList enemy_guilds, IDList other_allies,
		  IDList other_enemies);
void GuildGotShield(ID idGuild, char* pszGuildName, BYTE color1, BYTE color2, BYTE pattern);
void GuildGotShields(list_type shields);
void GuildResetData(void);
void AbortGuildDialogs(void);

void GuildHallsReceived(WORD num_halls, GuildHall *halls);

BOOL CALLBACK GuildInviteDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL CALLBACK GuildMemberDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL CALLBACK GuildAllyDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL CALLBACK GuildMasterDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
BOOL CALLBACK GuildShieldDialogProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
void GuildShieldDraw(void);
void GuildShieldControl(HWND hwndGraphic);
void GuildShieldInit();

#endif /* #ifndef _GUILD_H */
