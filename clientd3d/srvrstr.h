// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * srvrstr.h:  Header file for srvrstr.c
 */

#ifndef _SRVRSTR_H
#define _SRVRSTR_H

// Text style codes
enum {
   STYLE_NORMAL    = 0,
   STYLE_ITALIC    = 1,
   STYLE_BOLD      = 2,
   STYLE_UNDERLINE = 4,

   STYLE_RESET     = 3,
};

#define MAXQPARAMS 25  /* Maximum # of %q parameters in a server message when fully expanded */

typedef struct {
   int fieldPos[MAXQPARAMS];
   int size[MAXQPARAMS];
   int bytePos[MAXQPARAMS];
} PosArray;

M59EXPORT Bool CheckServerMessage(char** message, char **params, long *len, ID fmt_id);
M59EXPORT int CheckMessageOrder(char **param_ptr, long *len, ID fmt_id);
M59EXPORT void DisplayServerMessage(char *message, COLORREF start_color, BYTE start_style);
M59EXPORT void DisplayMessage(char *message, COLORREF start_color, BYTE start_style);


#endif /* #ifndef _SRVRSTR_H */
