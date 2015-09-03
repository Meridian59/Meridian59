// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * command.h:  Header file for command.c
 */

#ifndef _COMMAND_H
#define _COMMAND_H

Bool TellGroup(char *name, char *message);

// Command handling procedures
void CommandSay(char *args);
void CommandYell(char *args);
void CommandBroadcast(char *args);
void CommandEmote(char *args);
void CommandQuit(char *args);
void CommandHel(char *args);
void CommandHelp(char *args);
void CommandTell(char *args);
void CommandWho(char *args);
void CommandGroupAdd(char *args);
void CommandGroupDelete(char *args);
void CommandGroupNew(char *args);
void CommandBuy(char *args);
void CommandDrop(char *args);
void CommandGet(char *args);
void CommandLook(char *args);
void CommandOffer(char *args);
void CommandCast(char *args);
void CommandWave(char *args);
void CommandPoint(char *args);
void CommandDance(char *args);
void CommandRest(char *args);
void CommandStand(char *args);
void CommandSuicide(char *args);
void CommandNeutral(char *args);
void CommandHappy(char *args);
void CommandSad(char *args);
void CommandWry(char *args);
void CommandMap(char *args);
void CommandPut(char *args);
void CommandActivate(char *args);
void CommandSuicid(char *args);
void CommandGuild(char *args);
void CommandPassword(char *args);
void CommandDeposit(char *args);
void CommandWithdraw(char *args);
void CommandBalance(char *args);
void CommandGroup(char *args);
void CommandAppeal(char *args);
void CommandTellGuild(char *args);
void CommandSafetyOn(char *args);
void CommandSafetyOff(char *args);
void CommandTempSafeOn(char *args);
void CommandTempSafeOff(char *args);
void CommandGroupingOn(char *args);
void CommandGroupingOff(char *args);
void CommandAutoLootOn(char *args);
void CommandAutoLootOff(char *args);
void CommandAutoCombineOn(char *args);
void CommandAutoCombineOff(char *args);
void CommandReagentBagOn(char *args);
void CommandReagentBagOff(char *args);
void CommandSpellPowerOn(char *args);
void CommandSpellPowerOff(char *args);

#endif /* #ifndef _COMMAND_H */
