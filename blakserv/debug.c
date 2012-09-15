// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * debug.c
 *

 Functions for interfacing with MSVC 4.0 bizarre CRT debug things.

 */

#include "blakserv.h"

void InitDebug()
{
   if (ConfigBool(DEBUG_HEAP))
      DebugHeapAll();
}

void DebugHeapAll()
{
#ifdef _DEBUG
   int flag;

   flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

   /* turn on memory checking on EVERY allocation */
   flag |= _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_DELAY_FREE_MEM_DF;
   
   /* set flag */
   _CrtSetDbgFlag(flag);
#else
   FatalError("Heap debugging not supported without debug CRT");  
#endif
}

void DebugCheckHeap()
{
#ifdef _DEBUG
   _CrtCheckMemory();
#endif
}
