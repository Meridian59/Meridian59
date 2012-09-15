// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * parse.h:  Header file for parse.c
 */

#ifndef _PARSE_H
#define _PARSE_H

#define MAX_COMMAND_ARGS 10    // Maximum # of required arguments to a command

// Command handler takes string user typed
typedef void (*CommandProc)(char *args);

typedef struct {
   char *name;         // String user types to invoke command
   CommandProc proc;   // Procedure to call when command found
} TypedCommand;

BOOL ParseGotText(char *str);
M59EXPORT Bool ParseCommand(char *str, TypedCommand *commands);

#endif /* #ifndef _PARSE_H */
