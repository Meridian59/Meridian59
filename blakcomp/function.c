// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* 
 * function.c
 * "Function prototypes" for built in C-language functions 
 */
#include "blakcomp.h"
#include "bkod.h"

/*
 * The function_type table below defines signatures for all the built-in
 * C functions.  Each argument to one of these functions can be an
 * expression, a class name, a message name, a setting of the form a=b,
 * or a variable number of expressions or settings.  
 * Using this table lets us handle all function calls identically in the 
 * grammar.
 */

/* Use "ANONE" at end of line to indicate end of function header.
 * ASETTINGS and AEXPRESSIONS should only appear at the end of a list; they should
 * only be followed by ANONE.
 */
function_type Functions[] = {
{"Send",		SENDMESSAGE,   	 AEXPRESSION,	AEXPRESSION, 	ASETTINGS, ANONE},
{"Create",	        CREATEOBJECT,	 AEXPRESSION,	ASETTINGS,	ANONE},
{"Cons",		CONS,		 AEXPRESSION,	AEXPRESSION,	ANONE},
{"First",		FIRST,		 AEXPRESSION,	ANONE},
{"Rest",		REST,		 AEXPRESSION,	ANONE},
{"Length", 		LENGTH,		 AEXPRESSION,	ANONE},
{"List", 		LIST,		 AEXPRESSIONS,	ANONE},
{"Nth",			NTH,		 AEXPRESSION,	AEXPRESSION,	ANONE},
{"SetFirst",            SETFIRST,        AEXPRESSION,   AEXPRESSION,    ANONE},
{"SetNth",              SETNTH,          AEXPRESSION,   AEXPRESSION,    AEXPRESSION, ANONE},
{"DelListElem",         DELLISTELEM,     AEXPRESSION,   AEXPRESSION,    ANONE},
{"FindListElem",         FINDLISTELEM,     AEXPRESSION,   AEXPRESSION,    ANONE},
{"MoveListElem",       MOVELISTELEM,     AEXPRESSION,   AEXPRESSION, AEXPRESSION, ANONE},
{"Random",		RANDOM,		 AEXPRESSION,	AEXPRESSION,	ANONE},
{"AddPacket",           ADDPACKET,       AEXPRESSIONS,  ANONE},
{"SendPacket",          SENDPACKET,      AEXPRESSION,   ANONE},
{"SendCopyPacket",      SENDCOPYPACKET,  AEXPRESSION,   ANONE},
{"ClearPacket",         CLEARPACKET,     ANONE},
{"Debug",		DEBUG,		 AEXPRESSIONS,	ANONE},
{"GetInactiveTime",     GETINACTIVETIME, AEXPRESSION,   ANONE},
{"DumpStack", DUMPSTACK, ANONE},
{"StringEqual",		STRINGEQUAL,	 AEXPRESSION, 	AEXPRESSION,	ANONE},
{"StringContain",	STRINGCONTAIN,	 AEXPRESSION, 	AEXPRESSION,	ANONE},
{"StringSubstitute",	STRINGSUBSTITUTE,AEXPRESSION, 	AEXPRESSION,	AEXPRESSION,	ANONE},
{"StringLength",        STRINGLENGTH,    AEXPRESSION,   ANONE},
{"StringConsistsOf",    STRINGCONSISTSOF,AEXPRESSION,   AEXPRESSION,    ANONE},
{"CreateTimer",         CREATETIMER,     AEXPRESSION,   AEXPRESSION,    AEXPRESSION, ANONE},
{"DeleteTimer",         DELETETIMER,	 AEXPRESSION,   ANONE},
{"IsList",              ISLIST,          AEXPRESSION,   ANONE},
{"IsClass",             ISCLASS,         AEXPRESSION,   AEXPRESSION,    ANONE},
{"RoomData",            ROOMDATA,        AEXPRESSION,   ANONE},
{"LoadRoom",            CREATEROOMDATA,  AEXPRESSION,   ANONE},
{"GetClass",            GETCLASS,        AEXPRESSION,   ANONE},
{"GetTime",             GETTIME,         ANONE},
{"GetTickCount",        GETTICKCOUNT,    ANONE},
{"CanMoveInRoom",       CANMOVEINROOM,   AEXPRESSION,   AEXPRESSION,AEXPRESSION,
    AEXPRESSION,AEXPRESSION,ANONE},
{"CanMoveInRoomFine",CANMOVEINROOMFINE,AEXPRESSION,AEXPRESSION,AEXPRESSION,
    AEXPRESSION,AEXPRESSION,ANONE},
{"SetResource",         SETRESOURCE,     AEXPRESSION,   AEXPRESSION,  ANONE},
{"Post",		POSTMESSAGE,   	 AEXPRESSION,	AEXPRESSION, 	ASETTINGS, ANONE},
{"Abs",                 ABS,             AEXPRESSION,   ANONE},
{"Sqrt",                SQRT,            AEXPRESSION,   ANONE},
{"ParseString",		PARSESTRING,   	 AEXPRESSION,   AEXPRESSION,	AEXPRESSION, ANONE},
{"CreateTable",         CREATETABLE,     ANONE},
{"AddTableEntry",       ADDTABLEENTRY,   AEXPRESSION,   AEXPRESSION,    AEXPRESSION, ANONE},
{"GetTableEntry",       GETTABLEENTRY,   AEXPRESSION,   AEXPRESSION, ANONE},
{"DeleteTableEntry",    DELETETABLEENTRY,AEXPRESSION,   AEXPRESSION, ANONE},
{"DeleteTable",         DELETETABLE,     AEXPRESSION,   ANONE},
{"Bound",               BOUND,           AEXPRESSION,   AEXPRESSION, AEXPRESSION, ANONE},
{"GetTimeRemaining",    GETTIMEREMAINING,AEXPRESSION,   ANONE},
{"SetString",           SETSTRING,       AEXPRESSION,   AEXPRESSION, ANONE},
{"AppendTempString",    APPENDTEMPSTRING,AEXPRESSION,   ANONE},
{"ClearTempString",     CLEARTEMPSTRING, ANONE},
{"GetTempString",       GETTEMPSTRING,   ANONE},
{"CreateString",        CREATESTRING,    ANONE},
{"IsObject",            ISOBJECT,        AEXPRESSION,   ANONE},
{"RecycleUser",         RECYCLEUSER,     AEXPRESSION,   ANONE},
{"MinigameNumberToString", MINIGAMENUMBERTOSTRING,     AEXPRESSION,   AEXPRESSION,   ANONE},
{"MinigameStringToNumber", MINIGAMESTRINGTONUMBER,     AEXPRESSION,   ANONE},
   };

int numfuncs = (sizeof(Functions)/sizeof(function_type));

/* 
 * BuiltinIds is a bunch of identifiers that are automatically put into the 
 * compiler's symbol tables before any input is read.  This assures that the
 * identifiers will have the id #s given in the table, and so the server will
 * be able to use these numbers to access the identifiers from C.
 * The id #s of builtin ids should be below IDBASE defined in blakcomp.h.
 */
id_struct BuiltinIds[] = {
{"self",          I_PROPERTY,   0,  0,   COMPILE},
{"user",          I_MISSING,    1,  0,   I_CLASS},
{"userlogon",     I_MISSING,    2,  0,   I_MESSAGE},
{"session_id",    I_MISSING,    3,  0,   I_PARAMETER},
{"system",        I_MISSING,    4,  0,   I_CLASS},
{"system_id",     I_MISSING,    5,  0,   I_PARAMETER},
{"receiveclient", I_MISSING,    6,  0,   I_MESSAGE},
{"client_msg",    I_MISSING,    7,  0,   I_PARAMETER},
{"garbagecollecting",I_MISSING, 8,  0,   I_MESSAGE},
{"loadedfromdisk",I_MISSING,    9,  0,   I_MESSAGE},
{"constructor",   I_MISSING,   10,  0,   I_MESSAGE},
{"destructor",    I_MISSING,   11,  0,   I_MESSAGE},
{"number_stuff",  I_MISSING,   12,  0,   I_PARAMETER},
{"garbagecollectingdone",I_MISSING, 13,  0,   I_MESSAGE},
{"string",        I_MISSING,   14,  0,   I_PARAMETER},
{"adminsystemmessage",I_MISSING, 15,  0, I_MESSAGE},
{"getusername",   I_MISSING,   16,  0,   I_MESSAGE},
{"getusericon",   I_MISSING,   17,  0,   I_MESSAGE},
{"sendcharinfo",  I_MISSING,   18,  0,   I_MESSAGE},
{"newhour",       I_MISSING,   19,  0,   I_MESSAGE},
{"guest",         I_MISSING,   20,  0,   I_CLASS},
{"name",    	  I_MISSING,   21,  0,   I_PARAMETER},
{"icon",    	  I_MISSING,   22,  0,   I_PARAMETER},
{"admin",         I_MISSING,   23,  0,   I_CLASS},
{"systemlogon",   I_MISSING,   24,  0,   I_MESSAGE},
{"finduserbyinternetmailname",I_MISSING,25,0,I_MESSAGE},
{"receiveinternetmail",I_MISSING,26,0,   I_MESSAGE},
{"perm_string",   I_MISSING,   27,  0,   I_PARAMETER},
{"isfirsttime",   I_MISSING,   28,  0,   I_MESSAGE},
{"delete",        I_MISSING,   29,  0,   I_MESSAGE},
{"timer",         I_MISSING,   30,  0,   I_PARAMETER},
{"type",          I_MISSING,   31,  0,   I_PARAMETER},
{"DM",            I_MISSING,   32,  0,   I_CLASS},
{"finduserbystring",I_MISSING, 33,  0,   I_MESSAGE},
{"creator",       I_MISSING,   34,  0,   I_CLASS},
};

int numbuiltins = (sizeof(BuiltinIds)/sizeof(id_struct));
