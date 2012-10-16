//----------------------------------------------------------------------------
// File: 
//
// Desc: 
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _CLIENT_ID_H
#define _CLIENT_ID_H




//-----------------------------------------------------------------------------
// Name: 
// Desc: Client IDs are 32-bit values that refer to a particular Client. They are 
//       broken up into two bitfields, one of which can be used into an index 
//       of a list of Client 'slots', the other bitfield is a "uniqueness" value 
//       that is incremented each time a new Client is created. Hence, although
//       the same slot may be reused by different Clients are different times, 
//       it's possible to distinguish between the two by comparing uniqueness 
//       values (you can just compare the whole 32-bit id).
//-----------------------------------------------------------------------------
typedef DWORD   CLClientID;

#define CLIENT_SLOT_BITS 13
#define MAX_CLIENTS      (1<<CLIENT_SLOT_BITS)
#define CLIENT_SLOT_MASK (MAX_CLIENTS-1)




#endif
