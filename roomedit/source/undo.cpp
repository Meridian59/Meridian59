/*----------------------------------------------------------------------------*
 | This file is part of WinDEU, the port of DEU to Windows.                   |
 | WinDEU was created by the DEU team:                                        |
 |  Renaud Paquay, Raphael Quinet, Brendon Wyber and others...                |
 |                                                                            |
 | DEU is an open project: if you think that you can contribute, please join  |
 | the DEU team.  You will be credited for any code (or ideas) included in    |
 | the next version of the program.                                           |
 |                                                                            |
 | If you want to make any modifications and re-distribute them on your own,  |
 | you must follow the conditions of the WinDEU license. Read the file        |
 | LICENSE or README.TXT in the top directory.  If do not  have a copy of     |
 | these files, you can request them from any member of the DEU team, or by   |
 | mail: Raphael Quinet, Rue des Martyrs 9, B-4550 Nandrin (Belgium).         |
 |                                                                            |
 | This program comes with absolutely no warranty.  Use it at your own risks! |
 *----------------------------------------------------------------------------*

	Project WinDEU
	DEU team
	Jul-Dec 1994, Jan-Mar 1995

	FILE:         undo.cpp

	OVERVIEW
	========
	Source file for implementation of UNDO/REDO functions.
*/

#include "common.h"
#pragma hdrstop

#ifndef __undo_h
	#include "undo.h"
#endif

#ifndef __levels_h
	#include "levels.h"
#endif


/*
	Structure for UNDO saving
*/
typedef struct
{
	// Name of UNDO function (program defined). This name can be retrieved
	// with GetUndoName(), to setup Undo menu item name
	// Example: Undo [move Things]
	//                ^^^^^^^^^^^
	//               retrieved by GetUndoName
	char  Name[80];
	SHORT NumThings;
	SHORT NumVertexes;
	SHORT NumLineDefs;
	SHORT NumSideDefs;
	SHORT NumSectors;
	TPtr  Things;
	VPtr  Vertexes;
	LDPtr LineDefs;
	SDPtr SideDefs;
	SPtr  Sectors;

	BOOL MadeChanges;
	BOOL MadeMapChanges;
} UndoData;


/*
   Maximum UNDO/REDO structs
*/
int MaxUndo = 50;	// Default value

/*
   Undo array
*/
static UndoData *UndoArray = NULL;

/*
   Number of UNDO entries in array (0 -> MAX_UNDO)
*/
static SHORT NbUndo = 0;

/*
   UNDO position: next UNDO to make (-1 -> NbUndo-1)
*/
static SHORT NextUndo = -1;


/*
	Local functions
*/
static void CreateUndo (const char *name, UndoData *p_undo);
static void FreeUndo(UndoData *p_undo);
static int CompareUndo (UndoData *p_undo);
static void AddUndo(UndoData *p_undo);



/*

	Alloc UNDO/REDO structures

*/
void InitUndo()
{
	// Check MaxUndo global var.
	if ( MaxUndo < 1 )	MaxUndo = 1;

	// Allocates MaxUndo+1 entries
	if ( UndoArray == NULL )
		UndoArray = (UndoData *)GetMemory (MaxUndo * sizeof(UndoData));

	NbUndo = 0;
	NextUndo = -1;
}


/*

	Free all UNDO/REDO structures

*/
void CleanupUndo()
{
	// Free UNDO entries
	for (int i = 0 ; i < NbUndo ; i++)
		FreeUndo(&UndoArray[i]);

	// Free UNDO array ptr
	if ( UndoArray != NULL )
	{
		FreeMemory (UndoArray);
		UndoArray = NULL;
	}

	NbUndo = 0;
	NextUndo = -1;
}




/////////////////////////////////////////////////////////////
//
//         Basic UNDO structure primitives
//
/////////////////////////////////////////////////////////////

/*

	Creates an UNDO struct and and copy the global level data to it.

*/
void CreateUndo (const char *name, UndoData *p_undo)
{
#define COPY_OBJECTS(name,type,ptr)									\
{                                                                   \
	p_undo->Num##name = Num##name;                                  \
	if ( Num##name == 0 )                                           \
		p_undo->##name = (ptr)NULL;                                 \
	else                                                            \
	{                                                               \
		p_undo->##name = (ptr)GetMemory (Num##name * sizeof(type));	\
		ptr pTempU = p_undo->##name;                                \
		ptr pTempL = ##name;                                        \
		for (SHORT i = 0 ; i < Num##name ; i++)						\
			*pTempU++ = *pTempL++;                                  \
	}                                                               \
}

	// Copy UNDO function name
	strcpy (p_undo->Name, name);

	// Copy level data to undo struct
	COPY_OBJECTS(Things,   Thing,   TPtr);
	COPY_OBJECTS(Vertexes, Vertex,  VPtr);
	COPY_OBJECTS(LineDefs, LineDef, LDPtr);
	COPY_OBJECTS(SideDefs, SideDef, SDPtr);
	COPY_OBJECTS(Sectors,  Sector,  SPtr);
	p_undo->MadeChanges    = MadeChanges;
	p_undo->MadeMapChanges = MadeMapChanges;
}



/*

	Swap an UNDO struct and the level data (no memory copying, but
	ptr).

*/
void SwapUndo (UndoData *p_undo)
{
#define SWAP_OBJECTS(name,type,ptr)         \
{                                           \
	SHORT ntemp = Num##name;                \
	Num##name = p_undo->Num##name;			\
	p_undo->Num##name = ntemp;				\
	ptr ptemp = ##name;                     \
	##name = UndoArray[NextUndo].##name;	\
	p_undo->##name = ptemp;					\
}

	SWAP_OBJECTS(Things,   Thing,   TPtr);
	SWAP_OBJECTS(Vertexes, Vertex,  VPtr);
	SWAP_OBJECTS(LineDefs, LineDef, LDPtr);
	SWAP_OBJECTS(SideDefs, SideDef, SDPtr);
	SWAP_OBJECTS(Sectors,  Sector,  SPtr);

	BOOL bTemp = MadeChanges;
	MadeChanges = p_undo->MadeChanges;
	p_undo->MadeChanges = bTemp;

	bTemp = MadeMapChanges;
	MadeMapChanges = p_undo->MadeMapChanges;
	p_undo->MadeMapChanges = bTemp;
}


/*

	Free an UNDO struct

*/
void FreeUndo(UndoData *p_undo)
{
#define FREE_OBJECTS(name)        		\
	if (p_undo->##name != NULL)         \
		FreeMemory (p_undo->##name);    \
	p_undo->Num##name = 0;

	FREE_OBJECTS(Things);
	FREE_OBJECTS(Vertexes);
	FREE_OBJECTS(LineDefs);
	FREE_OBJECTS(SideDefs);
	FREE_OBJECTS(Sectors);
	p_undo->MadeChanges    = FALSE;
	p_undo->MadeMapChanges = FALSE;
}


/*

	Compares UNDO structure with current level datas and
	returns 0 if there's no differences

*/
int CompareUndo (UndoData *p_undo)
{
#define COMPARE_OBJECTS(name,type,ptr)							\
{                                               				\
	ptr pTempU = p_undo->##name;            					\
	ptr pTempL = ##name;                    					\
	for (SHORT i = 0 ; i < Num##name ; i++)						\
	{                                          	 				\
		if ( memcmp (pTempU++, pTempL++, sizeof(type)) != 0 )	\
			return 1;                           				\
	}                                                           \
}

	if ( p_undo->NumThings   != NumThings   ||
		 p_undo->NumVertexes != NumVertexes ||
		 p_undo->NumLineDefs != NumLineDefs ||
		 p_undo->NumSideDefs != NumSideDefs ||
		 p_undo->NumSectors  != NumSectors  ||
		 p_undo->MadeChanges != MadeChanges ||
		 p_undo->MadeMapChanges != MadeMapChanges )
	{
		return 1;
	}

	COMPARE_OBJECTS(Things,   Thing,   TPtr);
	COMPARE_OBJECTS(Vertexes, Vertex,  VPtr);
	COMPARE_OBJECTS(LineDefs, LineDef, LDPtr);
	COMPARE_OBJECTS(SideDefs, SideDef, SDPtr);
	COMPARE_OBJECTS(Sectors,  Sector,  SPtr);

	return 0;
}


/////////////////////////////////////////////////////////////
//
//         Advanced UNDO structure primitives
//
/////////////////////////////////////////////////////////////


/*

	Delete all UNDO structs after the NextUndo position
	If the array is full, shift it left (delete the first entry).
	Add the UNDO struct at the current NextUndo position.

*/
void AddUndo(UndoData *p_undo)
{
	assert_bound (NbUndo, 0, MaxUndo);
	assert_bound (NextUndo, -1, NbUndo - 1);

	// Notify ("MaxUndo = %d, NbUndo = %d, NextUndo = %d", MaxUndo, NbUndo, NextUndo);

	// Delete UNDO structs standing after NextUndo position
	for (SHORT i = NbUndo - 1 ; i > NextUndo ; i--)
	{
		FreeUndo(&UndoArray[i]);
		NbUndo--;
	}
	assert(NbUndo == NextUndo + 1);

	// If UNDO array full, shift it left
	if ( NbUndo == MaxUndo )
	{
		FreeUndo(&UndoArray[0]);
		for (SHORT i = 0 ; i < NbUndo - 1 ; i++)
			UndoArray[i] = UndoArray[i+1];
		NbUndo--;
		NextUndo--;
	}
	assert(NbUndo == NextUndo + 1);

	// Copy level data to undo struct
	assert_bound(NbUndo, 0, MaxUndo - 1);
	UndoArray[NbUndo] = *p_undo;

	// We start a new undo cycle
	NextUndo = NbUndo;
	NbUndo++;

	// Notify ("MaxUndo = %d, NbUndo = %d, NextUndo = %d", MaxUndo, NbUndo, NextUndo);
}


/*

	Get name of next UNDO operation

*/
char *GetUndoName()
{
	assert_bound (NbUndo, 0, MaxUndo);
	assert_bound (NextUndo, -1, NbUndo - 1);

	if ( NextUndo >= 0 )
		return UndoArray[NextUndo].Name;

	return NULL;
}


/*

	Get name of next REDO operation

*/
char *GetRedoName()
{
	assert_bound (NbUndo, 0, MaxUndo);
	assert_bound (NextUndo, -1, NbUndo - 1);

	if ( NextUndo+1 < NbUndo )
		return UndoArray[NextUndo+1].Name;

	return NULL;
}


/*

	UNDO: Restore level data from NextUndo UNDO position in array
		  and NextUndo--;

*/
void Undo()
{
	assert_bound (NbUndo, 0, MaxUndo);
	assert_bound (NextUndo, -1, NbUndo - 1);

	if ( NextUndo < 0 )
	{
		Beep();
		Notify ("No more UNDO structs (BUG!)");
		return;
	}

	// Copy level data to undo struct
	assert_bound (NextUndo, 0, NbUndo - 1);
	SwapUndo (&UndoArray[NextUndo]);

	// Go to previous UNDO struct
	NextUndo--;
}


/*

	REDO: Restore level data from "NextUndo+1" UNDO position in array.
		  and NextUndo++;

*/
void Redo()
{
	assert_bound (NbUndo, 0, MaxUndo);
	assert_bound (NextUndo, -1, NbUndo - 1);

	// If last UNDO position, no REDO available
	if ( NextUndo >= NbUndo - 1)
	{
		Beep();
		Notify ("No more REDO structs (BUG!)");
		return;
	}

	// Go to next UNDO struct
	NextUndo++;

	// Copy level data to undo struct
	assert_bound (NextUndo, 0, NbUndo - 1);
	SwapUndo(&UndoArray[NextUndo]);
}


/*
	Use the StartUndoRecording and StopUndoRecording to record
	changes in the UNDO/REDO array

	Example:
		StartUndoRecording("editing");
		(...)
		   level editing operations (InsertObject, updates, ...)
		(...)
		StopUndoRecording();

	If there were changes to the level, the UNDO will be stored.
*/

static UndoData UndoRecord;
static BOOL RecordingUndo = FALSE;

/*

	Copy the current level data to the temporary UNDO struct (UndoRecord)

*/
void StartUndoRecording(const char *name)
{
	assert_bound (NbUndo, 0, MaxUndo);
	assert_bound (NextUndo, -1, NbUndo - 1);

	assert (!RecordingUndo);
	if ( RecordingUndo )
	{
		Notify ("Progam bug in UNDO recording (start)");
		return;
	}

	CreateUndo (name, &UndoRecord);
	RecordingUndo = TRUE;
}


/*

	Add an UNDO entry only if the level data were modified after
	the prvious call to StartUndoRecording()

*/
void StopUndoRecording()
{
	assert_bound (NbUndo, 0, MaxUndo);
	assert_bound (NextUndo, -1, NbUndo - 1);

	assert(RecordingUndo);
	if ( !RecordingUndo )
	{
		Notify ("Progam bug in UNDO recording (stop)");
		return;
	}

	// Add UNDO struct or forget it if no changes since last
	// call to StartUndoRecord
	if ( CompareUndo (&UndoRecord) != 0 )
		AddUndo (&UndoRecord);
	else
		FreeUndo (&UndoRecord);

	RecordingUndo = FALSE;
}

