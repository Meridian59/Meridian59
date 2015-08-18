// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * string.h
 *
 */

#ifndef _BSTRING_H
#define _BSTRING_H

#define INIT_STRING_NODES 50000
#define LEN_TEMP_STRING LEN_MAX_CLIENT_MSG

typedef struct
{
   char *data;
   int len_data;
   int garbage_ref;
} string_node;

void InitString(void);
void ResetString(void);
int GetStringsUsed(void);
string_node * GetStringByID(int string_id);
Bool IsStringByID(int string_id);
int CreateString(const char *new_str);
int CreateStringWithLen(const char *buf,int len);
Bool LoadBlakodString(FILE *f,int len_str,int string_id);
void ForEachString(void (*callback_func)(string_node *snod,int string_id));
void FreeString(int string_id);
void MoveStringNode(int dest_id,int source_id);
void SetNumStrings(int new_num_strings);
int GetNumStrings(void);

void SetString(string_node *snod,char *buf,int len);

void SetTempString(char *buf,int len);
void ClearTempString(void);
void AppendTempString(const char *buf, int len);
void AppendNumToTempString(int iNum);
string_node * GetTempString(void);

#endif
