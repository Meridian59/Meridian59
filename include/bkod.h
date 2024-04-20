// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* bkod.h
 * header file for the bkod byte compiled format
 */

#ifndef _BKOD_H
#define _BKOD_H

/* special byte for message handler header */
enum
{
   HANDLER = 0,
   CONSTRUCTOR = 1,
   DESTRUCTOR = 2,
};

/* command bits of the opcode */
enum
{
   UNARY_ASSIGN = 0,
   BINARY_ASSIGN = 1,
   GOTO = 2,
   CALL = 3,
   RETURN = 4,
   DEBUG_LINE = 5,
};

/* info byte for unary assign */
enum
{
   NOT = 0,
   NEGATE = 1,
   NONE = 2,
   BITWISE_NOT = 3,
};

/* info byte for binary assign */
enum
{
   ADD = 0,
   SUBTRACT = 1,
   MULTIPLY = 2,
   DIV = 3,
   MOD = 4,
   AND = 5,
   OR = 6,
   EQUAL = 7,
   NOT_EQUAL = 8,
   LESS_THAN = 9,
   GREATER_THAN = 10,
   LESS_EQUAL = 11,
   GREATER_EQUAL = 12,
   BITWISE_AND = 13,
   BITWISE_OR = 14,
};

/* source1 & source2 bits of the opcode for assign and return, and dest bit of the opcode,
 * and some legal values for type byte of each parameter in a call
 */
enum 
{
   LOCAL_VAR = 0,
   PROPERTY = 1,
   CONSTANT = 2,
   CLASS_VAR = 3,
};

/* source1 bits of the opcode for goto */
enum
{
   GOTO_COND_LOCAL_VAR = 0,
   GOTO_COND_PROPERTY = 1,
   GOTO_COND_CONSTANT = 2,
   GOTO_COND_CLASS_VAR = 3,

   GOTO_UNCONDITIONAL = 1,    // source2 bits for unconditional goto
};

/* source1 bits of the opcode for call */
enum
{
   CALL_ASSIGN_LOCAL_VAR = 0,
   CALL_ASSIGN_PROPERTY = 1,
   CALL_NO_ASSIGN = 2,
};

/* dest bit of the opcode for return */
enum
{
   NO_PROPAGATE = 0,
   PROPAGATE = 1,
};

/* dest bit of the opcode for conditional gotos */
enum
{
   GOTO_IF_TRUE = 0,
   GOTO_IF_FALSE = 1,
};

/* function ID's */
enum
{
   CREATEOBJECT = 1,
   ISCLASS = 3,
   GETCLASS = 5,

   SENDMESSAGE = 11,
   POSTMESSAGE = 12,

   DEBUG = 22,
   ADDPACKET = 23,
   SENDPACKET = 24,
   SENDCOPYPACKET = 25,
   CLEARPACKET = 26,
   GETINACTIVETIME = 27,
	DUMPSTACK = 28,
   
   STRINGEQUAL = 31,
   STRINGCONTAIN = 32,
   SETRESOURCE = 33,
   PARSESTRING = 34,
   SETSTRING = 35,
   CREATESTRING = 36,
   STRINGSUBSTITUTE = 37,
   APPENDTEMPSTRING = 38,
   CLEARTEMPSTRING = 39,
   GETTEMPSTRING = 40,
   STRINGLENGTH = 43,
   STRINGCONSISTSOF = 44,

   CREATETIMER = 51,
   DELETETIMER = 52,
   GETTIMEREMAINING = 53,

   CREATEROOMDATA = 61,
   ROOMDATA = 63,
   CANMOVEINROOM = 64,
   CANMOVEINROOMFINE = 65,

   MINIGAMENUMBERTOSTRING = 71,
   MINIGAMESTRINGTONUMBER = 72,

   CONS = 101,
   FIRST = 102,
   REST = 103,
   LENGTH = 104,
   NTH = 105,
   LIST = 106,
   ISLIST = 107,
   SETFIRST = 108,
   SETNTH = 109,
   DELLISTELEM = 110,
   FINDLISTELEM = 111,
   MOVELISTELEM = 112,

   GETTIME = 120,
   GETTICKCOUNT = 121,

   ABS = 131,
   BOUND = 132,
   SQRT = 133,

   CREATETABLE = 141,
   ADDTABLEENTRY = 142,
   GETTABLEENTRY = 143,
   DELETETABLEENTRY = 144,
   DELETETABLE = 145,

   RECYCLEUSER = 151,

   ISOBJECT = 161,

   RANDOM = 201,
};

enum
{
   NIL = 0,
   TAG_NIL = 0,
   TAG_INT = 1,
   TAG_OBJECT = 2,
   TAG_LIST = 3,
   TAG_RESOURCE = 4,
   TAG_TIMER = 5,
   TAG_SESSION = 6,
   TAG_ROOM_DATA = 7,
   TAG_TEMP_STRING = 8,
   TAG_STRING = 9,
   TAG_CLASS = 10,
   TAG_MESSAGE = 11,
   TAG_DEBUGSTR = 12,
   TAG_OVERRIDE = 13,     // For overriding a class variable with a property
   TAG_RESERVED = 14,     // Reserved for future expansion
   TAG_INVALID = 15,
};

#define MAX_TAG 12
#define MAX_KOD_INT ((1<<27)-1)  // 28th bit is sign. 0x07ffffff == kod +134217727
#define MASK_KOD_INT ((1<<28)-1) // 28th bit is sign. 0x0fffffff == kod -1
#define MIN_KOD_INT (1<<27)      // 28th bit is sign. 0x08000000 == kod -134217728

typedef struct
{
   int data:28;
   unsigned int tag:4;
} constant_type;

/* opcode byte breakdown */
typedef struct
{
   unsigned int source2:2;
   unsigned int source1:2;
   unsigned int dest:1;
   unsigned int command:3;
} opcode_type;

#endif
