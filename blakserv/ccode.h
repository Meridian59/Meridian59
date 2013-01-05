// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * ccode.h
 *
 */

#ifndef _CCODE_H
#define _CCODE_H

int C_Invalid(int object_id,local_var_type *local_vars,
	      int num_normal_parms,parm_node normal_parm_array[],
	      int num_name_parms,parm_node name_parm_array[]);

int C_AddPacket(int object_id,local_var_type *local_vars,
		int num_normal_parms,parm_node normal_parm_array[],
		int num_name_parms,parm_node name_parm_array[]);

int C_SendPacket(int object_id,local_var_type *local_vars,
		 int num_normal_parms,parm_node normal_parm_array[],
		 int num_name_parms,parm_node name_parm_array[]);

int C_SendCopyPacket(int object_id,local_var_type *local_vars,
		     int num_normal_parms,parm_node normal_parm_array[],
		     int num_name_parms,parm_node name_parm_array[]);

int C_ClearPacket(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[]);

int C_Debug(int object_id,local_var_type *local_vars,
	    int num_normal_parms,parm_node normal_parm_array[],
	    int num_name_parms,parm_node name_parm_array[]);

int C_GetInactiveTime(int object_id,local_var_type *local_vars,
		      int num_normal_parms,parm_node normal_parm_array[],
		      int num_name_parms,parm_node name_parm_array[]);

int C_DumpStack(int object_id,local_var_type *local_vars,
		      int num_normal_parms,parm_node normal_parm_array[],
		      int num_name_parms,parm_node name_parm_array[]);

int C_SendMessage(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[]);

int C_PostMessage(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[]);

int C_CreateObject(int object_id,local_var_type *local_vars,
		   int num_normal_parms,parm_node normal_parm_array[],
		   int num_name_parms,parm_node name_parm_array[]);

int C_IsClass(int object_id,local_var_type *local_vars,
	      int num_normal_parms,parm_node normal_parm_array[],
	      int num_name_parms,parm_node name_parm_array[]);

int C_GetClass(int object_id,local_var_type *local_vars,
	       int num_normal_parms,parm_node normal_parm_array[],
	       int num_name_parms,parm_node name_parm_array[]);

int C_StringEqual(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[]);

int C_StringSubstitute(int object_id,local_var_type *local_vars,
		    int num_normal_parms,parm_node normal_parm_array[],
		    int num_name_parms,parm_node name_parm_array[]);

int C_StringContain(int object_id,local_var_type *local_vars,
		    int num_normal_parms,parm_node normal_parm_array[],
		    int num_name_parms,parm_node name_parm_array[]);

int C_SetResource(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[]);

int C_ParseString(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[]);

int C_SetString(int object_id,local_var_type *local_vars,
		int num_normal_parms,parm_node normal_parm_array[],
		int num_name_parms,parm_node name_parm_array[]);

int C_AppendTempString(int object_id,local_var_type *local_vars,
		int num_normal_parms,parm_node normal_parm_array[],
		int num_name_parms,parm_node name_parm_array[]);

int C_ClearTempString(int object_id,local_var_type *local_vars,
		int num_normal_parms,parm_node normal_parm_array[],
		int num_name_parms,parm_node name_parm_array[]);

int C_GetTempString(int object_id,local_var_type *local_vars,
		int num_normal_parms,parm_node normal_parm_array[],
		int num_name_parms,parm_node name_parm_array[]);

int C_CreateString(int object_id,local_var_type *local_vars,
		   int num_normal_parms,parm_node normal_parm_array[],
		   int num_name_parms,parm_node name_parm_array[]);

int C_StringLength(int object_id,local_var_type *local_vars,
		   int num_normal_parms,parm_node normal_parm_array[],
		   int num_name_parms,parm_node name_parm_array[]);

int C_StringConsistsOf(int object_id,local_var_type *local_vars,
                       int num_normal_parms,parm_node normal_parm_array[],
                       int num_name_parms,parm_node name_parm_array[]);

int C_CreateTimer(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[]);

int C_DeleteTimer(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[]);

int C_GetTimeRemaining(int object_id,local_var_type *local_vars,
		       int num_normal_parms,parm_node normal_parm_array[],
		       int num_name_parms,parm_node name_parm_array[]);

int C_LoadRoom(int object_id,local_var_type *local_vars,
	       int num_normal_parms,parm_node normal_parm_array[],
	       int num_name_parms,parm_node name_parm_array[]);

int C_RoomData(int object_id,local_var_type *local_vars,
	       int num_normal_parms,parm_node normal_parm_array[],
	       int num_name_parms,parm_node name_parm_array[]);

int C_CanMoveInRoom(int object_id,local_var_type *local_vars,
		    int num_normal_parms,parm_node normal_parm_array[],
		    int num_name_parms,parm_node name_parm_array[]);

int C_CanMoveInRoomFine(int object_id,local_var_type *local_vars,
		    int num_normal_parms,parm_node normal_parm_array[],
		    int num_name_parms,parm_node name_parm_array[]);

int C_Cons(int object_id,local_var_type *local_vars,
	   int num_normal_parms,parm_node normal_parm_array[],
	   int num_name_parms,parm_node name_parm_array[]);

int C_First(int object_id,local_var_type *local_vars,
	    int num_normal_parms,parm_node normal_parm_array[],
	    int num_name_parms,parm_node name_parm_array[]);

int C_Rest(int object_id,local_var_type *local_vars,
	   int num_normal_parms,parm_node normal_parm_array[],
	   int num_name_parms,parm_node name_parm_array[]);

int C_Length(int object_id,local_var_type *local_vars,
	     int num_normal_parms,parm_node normal_parm_array[],
	     int num_name_parms,parm_node name_parm_array[]);

int C_Nth(int object_id,local_var_type *local_vars,
	  int num_normal_parms,parm_node normal_parm_array[],
	  int num_name_parms,parm_node name_parm_array[]);

int C_List(int object_id,local_var_type *local_vars,
	   int num_normal_parms,parm_node normal_parm_array[],
	   int num_name_parms,parm_node name_parm_array[]);

int C_IsList(int object_id,local_var_type *local_vars,
	     int num_normal_parms,parm_node normal_parm_array[],
	     int num_name_parms,parm_node name_parm_array[]);

int C_SetFirst(int object_id,local_var_type *local_vars,
	       int num_normal_parms,parm_node normal_parm_array[],
	       int num_name_parms,parm_node name_parm_array[]);

int C_SetNth(int object_id,local_var_type *local_vars,
	     int num_normal_parms,parm_node normal_parm_array[],
	     int num_name_parms,parm_node name_parm_array[]);

int C_FindListElem(int object_id,local_var_type *local_vars,
		   int num_normal_parms,parm_node normal_parm_array[],
		   int num_name_parms,parm_node name_parm_array[]);

int C_DelListElem(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[]);

int C_GetTime(int object_id,local_var_type *local_vars,
	      int num_normal_parms,parm_node normal_parm_array[],
	      int num_name_parms,parm_node name_parm_array[]);

int C_Random(int object_id,local_var_type *local_vars,
	     int num_normal_parms,parm_node normal_parm_array[],
	     int num_name_parms,parm_node name_parm_array[]);

int C_Abs(int object_id,local_var_type *local_vars,
	  int num_normal_parms,parm_node normal_parm_array[],
	  int num_name_parms,parm_node name_parm_array[]);

int C_Sqrt(int object_id,local_var_type *local_vars,
	   int num_normal_parms,parm_node normal_parm_array[],
	   int num_name_parms,parm_node name_parm_array[]);

int C_Bound(int object_id,local_var_type *local_vars,
	    int num_normal_parms,parm_node normal_parm_array[],
	    int num_name_parms,parm_node name_parm_array[]);

int C_CreateTable(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[]);

int C_AddTableEntry(int object_id,local_var_type *local_vars,
		       int num_normal_parms,parm_node normal_parm_array[],
		       int num_name_parms,parm_node name_parm_array[]);

int C_GetTableEntry(int object_id,local_var_type *local_vars,
		    int num_normal_parms,parm_node normal_parm_array[],
		    int num_name_parms,parm_node name_parm_array[]);

int C_DeleteTableEntry(int object_id,local_var_type *local_vars,
		       int num_normal_parms,parm_node normal_parm_array[],
		       int num_name_parms,parm_node name_parm_array[]);

int C_DeleteTable(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[]);

int C_RecycleUser(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[]);

int C_IsObject(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[]);

int C_MinigameNumberToString(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[]);

int C_MinigameStringToNumber(int object_id,local_var_type *local_vars,
		  int num_normal_parms,parm_node normal_parm_array[],
		  int num_name_parms,parm_node name_parm_array[]);


void FuzzyCollapseString(char* pTarget, char* pSource, int len);
bool FuzzyBufferEqual(char *s1,int len1,char *s2,int len2);
bool FuzzyBufferContain(char *s1,int len1,char *s2,int len2);

#endif
