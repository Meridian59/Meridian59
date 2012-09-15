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

// Command handling procedures
void CommandGoRoom(char *args);
void CommandGoPlayer(char *args);
void CommandGetPlayer(char *args);
void CommandShowGChannel(char *args);
void CommandShowMap(char *args);
void CommandHideMap(char *args);
void CommandResetData(char *args);
void CommandDM(char *args);
void CommandEcho(char *args);
void CommandCrowds(char *args);

#endif /* #ifndef _COMMAND_H */
