// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * merintr.h:  Header file for merintr.c
 */

#ifndef _MERINTR_H
#define _MERINTR_H

#include "merintrc.h"
#include "mermain.h"
#include "spells.h"
#include "command.h"
#include "groups.h"
#include "userarea.h"
#include "alias.h"
#include "stats.h"
#include "statbtn.h"
#include "statcach.h"
#include "statlist.h"
#include "statnum.h"
#include "statmain.h"
#include "inventry.h"
#include "enchant.h"
#include "guild.h"
#include "actions.h"
#include "drawint.h"
#include "groupdlg.h"
#include "language.h"

#define MODULE_ID 100    // Unique module id #

#define A_CAST          (A_MODULE + 200)
#define A_CASTSPELL     (A_MODULE + 201)
#define A_LOOKINVENTORY (A_MODULE + 202)    // Examine item in inventory

// Window messages
#define BK_CREATED      (WM_USER + 100)

// Sending messages to server
#define RequestSkills()		   ToServer(BP_SEND_SKILLS, msg_table)
#define RequestSpells()            ToServer(BP_SEND_SPELLS, msg_table)
#define RequestStats(group)        ToServer(BP_SEND_STATS, msg_table, group)
#define RequestStatGroups()        ToServer(BP_SEND_STAT_GROUPS, msg_table)
#define RequestEnchantments(type)  ToServer(BP_SEND_ENCHANTMENTS, msg_table, type)

#define RequestRest()              ToServer(BP_USERCOMMAND, user_msg_table, UC_REST)
#define RequestStand()             ToServer(BP_USERCOMMAND, user_msg_table, UC_STAND)
#define RequestSuicide()           ToServer(BP_USERCOMMAND, user_msg_table, UC_SUICIDE)
#define SendPreferences(val)       ToServer(BP_USERCOMMAND, user_msg_table, UC_SEND_PREFERENCES, val)
#define RequestGuildInfo()         ToServer(BP_USERCOMMAND, user_msg_table, UC_REQ_GUILDINFO)
#define RequestInvite(obj)         ToServer(BP_USERCOMMAND, user_msg_table, UC_INVITE, obj)
#define RequestRenounce()          ToServer(BP_USERCOMMAND, user_msg_table, UC_RENOUNCE)
#define RequestExile(obj)          ToServer(BP_USERCOMMAND, user_msg_table, UC_EXILE, obj)
#define RequestAbdicate(obj)       ToServer(BP_USERCOMMAND, user_msg_table, UC_ABDICATE, obj)
#define RequestVote(obj)           ToServer(BP_USERCOMMAND, user_msg_table, UC_VOTE, obj)
#define RequestSetRank(obj, rank)  ToServer(BP_USERCOMMAND, user_msg_table, UC_SET_RANK, obj, rank)

#define RequestCreateGuild(name, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, secret) \
                                   ToServer(BP_USERCOMMAND, user_msg_table, UC_GUILD_CREATE, \
								   name, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, secret)

#define RequestGuildShieldSamples() ToServer(BP_USERCOMMAND, user_msg_table, UC_GUILD_SHIELDS)

#define RequestGuildShield()       ToServer(BP_USERCOMMAND, user_msg_table, UC_GUILD_SHIELD)

#define RequestFindGuildShield(colorA, colorB, pattern) \
                                   ToServer(BP_USERCOMMAND, user_msg_table, UC_CLAIM_SHIELD, \
								   colorA, colorB, pattern, FALSE)

#define RequestClaimGuildShield(colorA, colorB, pattern) \
                                   ToServer(BP_USERCOMMAND, user_msg_table, UC_CLAIM_SHIELD, \
								   colorA, colorB, pattern, TRUE)

#define RequestDisband()           ToServer(BP_USERCOMMAND, user_msg_table, UC_DISBAND)
#define RequestAlly(obj)           ToServer(BP_USERCOMMAND, user_msg_table, UC_MAKE_ALLIANCE, obj)
#define RequestEndAlly(obj)        ToServer(BP_USERCOMMAND, user_msg_table, UC_END_ALLIANCE, obj)
#define RequestEnemy(obj)          ToServer(BP_USERCOMMAND, user_msg_table, UC_MAKE_ENEMY, obj)
#define RequestEndEnemy(obj)       ToServer(BP_USERCOMMAND, user_msg_table, UC_END_ENEMY, obj)
#define RequestGuildList()         ToServer(BP_USERCOMMAND, user_msg_table, UC_REQ_GUILD_LIST)
#define RequestGuildRent(obj, s)   ToServer(BP_USERCOMMAND, user_msg_table, UC_GUILD_RENT, obj, s)
#define RequestAbandonHall()       ToServer(BP_USERCOMMAND, user_msg_table, UC_ABANDON_GUILD_HALL)
#define RequestGuildPassword(s)    ToServer(BP_USERCOMMAND, user_msg_table, UC_GUILD_SET_PASSWORD, s)
#define RequestWithdrawMoney(amount)    ToServer(BP_USERCOMMAND, user_msg_table, UC_WITHDRAW, amount)
#define RequestDepositMoney(amount)     ToServer(BP_USERCOMMAND, user_msg_table, UC_DEPOSIT, amount)
#define RequestBalance()           ToServer(BP_USERCOMMAND, user_msg_table, UC_BALANCE)
#define RequestAppeal(s)           ToServer(BP_USERCOMMAND, user_msg_table, UC_APPEAL, s)
#define RequestPreferences()       ToServer(BP_USERCOMMAND, user_msg_table, UC_REQ_PREFERENCES)

extern client_message msg_table[];
extern client_message user_msg_table[];

extern ClientInfo *cinfo;         // Holds data passed from main client
extern Bool exiting;              // True when module is exiting and should be unloaded

extern HINSTANCE hInst;  // module handle

extern player_info *GetPlayer(void);

#endif /* #ifndef _MERINTR_H */
