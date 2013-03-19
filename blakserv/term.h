// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * term.h
 *
 */

#ifndef _TERM_H
#define _TERM_H

void __cdecl cprintf(int session_id,const char *fmt,...);
void TermConvertBuffer(char *s,int len_s);

char * GetTagName(val_type val);
char * GetDataName(val_type val);

int GetTagNum(char *tag_str);
int GetDataNum(int tag_val,char *data_str);

Bool BlakMoveFile(char *source,char *dest);

#endif
