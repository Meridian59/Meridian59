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

	FILE:         windeu.h
*/
#ifndef __windeu_h
#define __windeu_h

#ifndef __common_h
	#include "common.h"
#endif

/*
   description of the command line arguments and config file keywords
*/

enum OPT_TYPE
{
	  OPT_BOOLEAN,          /* boolean (toggle) */
	  OPT_INTEGER,          /* integer number */
	  OPT_STRING,           /* character string */
	  OPT_STRINGACC,        /* character string, but store in a list */
	  OPT_STRINGLIST,       /* list of character strings */
	  OPT_END               /* end of the options description */
};


typedef struct
{
   char *short_name;        /* abbreviated command line argument */
   char *long_name;         /* command line arg. or keyword */
   enum OPT_TYPE opt_type ; /* type of this option */
   char *msg_if_true;       /* message printed if option is true */
   char *msg_if_false;      /* message printed if option is false */
   void *data_ptr;          /* pointer to the data */
} OptDesc;

/*
   the macros and constants
*/

/* name of the configuration file */
#define DEU_CONFIG_FILE     "WINDEU.INI"

/* name of the log file (debug mode) */
#define DEU_LOG_FILE        "windeu32.log"

/*

	Macros to select and unselect the WAIT cursor inside a function

*/

#define SELECT_WAIT_CURSOR() \
	HCURSOR hSaveOldCursor = ::SetCursor(::LoadCursor(0, IDC_WAIT));

#define UNSELECT_WAIT_CURSOR() \
	::SetCursor(hSaveOldCursor);


/*

	Macros used to save and restore the working message of the status bar
	in a function

*/
#define SAVE_WORK_MSG()					\
	char OldSaveMsg[80];                \
	GetWorkMessage (OldSaveMsg, 80);


#define RESTORE_WORK_MSG()   			\
	WorkMessage (OldSaveMsg);


/*

	Use these macros to set/restore a help context ID.
	You can't use this in blocks of different scope level,
	because of the intermediate var. definition

*/
#define SET_HELP_CONTEXT(hc)			\
{                                       \
	DWORD OldHelpContext = HelpContext; \
	HelpContext          = hc;

#define RESTORE_HELP_CONTEXT()   		\
	HelpContext = OldHelpContext;       \
}


// General macro to assert a bounded number
#define assert_bound(var,min,max) 		\
	assert((var) >= (min) && (var) <= (max))

// Macros to assert a valid object number (but NOT -1)
#define assert_tnum(t)						\
	assert_bound((t), 0, NumThings-1)

#define assert_ldnum(ld)					\
	assert_bound((ld), 0, NumLineDefs-1)

#define assert_sdnum(sd)					\
	assert_bound((sd), 0, NumSideDefs-1)

#define assert_vnum(v)						\
	assert_bound((v), 0, NumVertexes-1)

#define assert_snum(s)						\
	assert_bound((s), 0, NumSectors-1)


// Macros to assert a valid object number (INCLUDING '-1')
#define assert_vtnum(t)						\
	assert_bound((t), -1, NumThings-1)

#define assert_vldnum(ld)					\
	assert_bound((ld), -1, NumLineDefs-1)

#define assert_vsdnum(sd)					\
	assert_bound((sd), -1, NumSideDefs-1)

#define assert_vvnum(v)						\
	assert_bound((v), -1, NumVertexes-1)

#define assert_vsnum(s)						\
	assert_bound((s), -1, NumSectors-1)


/*
   the interfile global variables
*/

/* from windeu.cpp */
extern int   DoomVersion;	/* 1 for DOOM or 2 for DOOM2 */
extern BOOL  Registered;    /* registered or shareware WAD file? */
extern BOOL  Debug;     	/* are we debugging? */
extern BOOL  Quiet;     	/* don't play a sound when an object is selected */
extern BOOL  Quieter;       /* don't play any sound, even when an error occurs */
extern BOOL  Expert;        /* don't ask for confirmation for some operations */
extern int   InitialScale;  /* initial zoom factor for map */
extern BOOL  Colour2;       /* use the alternate set for things colors */
extern BOOL  AdditiveSelBox;/* additive selection box or select in box only? */
extern int   SplitFactor;   /* factor used by the Nodes builder */
extern BOOL  Select0;       /* select object 0 by default when switching modes */
extern char *MainWad;       /* name of the main wad file */
extern Bool  DrawLineDefsLen;/* Display length of moving LineDefs */
extern int   BuildPriority;	/* Priority for the nodes builder */
extern int   RoomID;        /* used in batch file for reseting room contents */
// Moved from WINDEU.CPP to use them in WinDEUApp class
extern Bool  Reminder;		/* display a funny message when DEU starts */
extern Bool  AutoScroll;	/* Auto scroll mode */

extern char **PatchWads;	/* list of patch wad files */

extern FILE *logfile;       /* filepointer to the error log */

extern char *BitmapDir;       /* Directory where bitmaps reside */
extern char *BitmapSpec;      /* Bitmap filename spec */
extern char *KodDir;
extern char *ServerDir;

/* from editcli.cpp */
extern BOOL InfoShown;          /* is the bottom line displayed? */
extern SHORT GridScale;
extern BOOL SnapToGrid;
extern BOOL GridShown;


/* from windeapp.cpp */
extern DWORD HelpContext;

/* from windeu.cpp (config file options) */
extern char  *DefaultWallTexture;		/* default normal wall texture */
extern char  *DefaultLowerTexture;		/* default lower wall texture */
extern char  *DefaultUpperTexture;		/* default upper wall texture */
extern char  *DefaultFloorTexture;		/* default floor texture */
extern char  *DefaultCeilingTexture;	        /* default ceiling texture */
extern SHORT  DefaultFloorHeight;		/* default floor height */
extern SHORT  DefaultCeilingHeight;		/* default ceiling height */
extern SHORT  DefaultLightLevel;		/* default light level */


/*
   the function prototypes
*/

/* from windeu.cpp */
void InitWindeu (int argc, char **argv, char *init_level);
void CleanupWindeu () ;
void CenterWindow (TWindow *pWnd);
void Beep ();
void PlaySound (int, int);
void ProgError (char *, ...);
void LogMessage (char *, ...);
void LogError(char *, ...);
void CloseLog(void);
void ClearLog(void);
void WorkMessage (char*, ...);
void GetWorkMessage (char *buffer, size_t bufferSize);
BOOL Confirm (char*, ...);
void Notify (char *, ...);
void FunnyMessage ();
void Usage ();
void ParseCommandLineOptions( int argc, char *argv[], char *init_level);
void ParseConfigFileOptions (char *filename);
void AppendItemToList (char ***list, char *item);
char *FormatNumber(LONG num);


////////////////////////////////////////////////////////////////////////
//
// How WinDEU Cooperates with others Windows apps.
// ===============================================
//
//RP: Feb 2 1995
//
// COOPERATION_VERSION == 1
//
//    First attempt to make WinDEU multi-task with other Windows
//    apps.
//
//    1. I defined a 'Cooperate' function which makes 1 (ONE) call to
//       PeekMessage, effectively giving control to Windows and parsing
//       1(ONE) message in the WinDEU message queue. That's why you can
//       still edit your level while building it. I MUST FIND A WAY TO
//       AVOID THIS.
//
//    2. I defined a 'BuildPriority' config option which defines the
//       frenquency at which WinDEU asks Windows to give control to other
//       apps. The COOPERATE macro uses BuildPriority to make calls to
//       the 'Cooperate' function.
//       If for example, BuildPriority is defined as 15, it means
//       that 15 of 100 calls to 'COOPERATE' will effectively make a call
//       to the 'Cooperate' function (which gives control to Windows).
//
//    3. I had then to put calls to 'COOPERATE' in the nodes builder code.
//       This wad the trickiest part of the work, because until now, the time
//       is not taken into account by COOPERATE. The problem is to put
//       COOPERATE in the code at 'right places'. To define 'Right Places',
//       I had to take two parameters into account:
//       - The overhead must be controlled. That means I can't call
//         COOPERATE in every loop. If I put a COOPERATE in a loop of
//         1000 iterations which simply make 'i++', the overhead would be
//         too big (WinDEU spends its time giving control to Windows, even with
//         a priority of 1).
//       - There must be enough calls to COOPERATE to let Windows take control
//         often enough.
//       - The calls to COOPERATE should be as regular (at the 'time' point of
//         view) as possible
//       So 'Right places' means not to much calls, not to few, and
//       as regular (time point of view) as possible:
//
//  Here's the first definition of COOPERATE:
//
// 	#define COOPERATE()                      	\
// 	{                                        	\
// 		if ( BuildPriority != 0 )            	\
// 		{                                    	\
// 			CoopCount++;                     	\
// 			if ( CoopCount >= PRIORITY_RES ) 	\
// 				CoopCount = 0;               	\
// 			if (CoopExecTab[CoopCount])      	\
// 				Cooperate();                 	\
// 		}                                    	\
// 	}
//
//    NOTE: The CoopExecTab is an array of boolean values indicating
//          if the correponding COOPERATE call must
//          If BuildPriority == 50, 1 element of 2 of CoopExecTab == TRUE
//
// Conclusion:
// ----------
//    Finaly, it worked not bad, but I wasn't happy because the time is
//    not taken into account. That means calls to 'Cooperate()' depends largely
//    on the processor speed and number of iterations in a loop.
//
//RP: Feb 5 1995
//
// COOPERATION_VERSION == 2
//
//    1. I redefined to 'BuildPriority' config options. It now means
//       the MINIMUM number of milliseconds WinDEU will work before
//       calling Cooperate.
//
//    2. The 'Cooperate' function makes 'BuildPriority / 10' calls to
//       'PeekMessage' every time it's called. It means a maximum of
//       100 messages/sec will be processed by WinDEU when building
//       a level. I don't know if it's really useful.
//
//    3. COOPERATE now uses the GetTickCount() Windows function to decide
//       to call Cooperate(). Gere's the new definition of COOPERATE:
//
// 	#define COOPERATE()                      						\
// 	{                                        						\
// 		if ( BuildPriority > 0 )									\
// 		{                                                           \
// 			ULONG CurTickCount = ::GetTickCount();                  \
// 			if ( CurTickCount - LastCoopCallTick > BuildPriority )  \
// 			{                                    					\
// 				Cooperate();                                        \
// 				LastTickCount = CurTickCount;						\
// 			}                                    					\
// 		}                                                           \
// 	}
//
// Conclusion:
// ----------
//   It works not well, and BuildPriority doesn't depend anymore on the
//   processor speed, but there's still some problems:
//   - The overhead is a little bigger than previously (because of calls
//     to GetTickCount() in EVERY calls to COOPERATE.
//   - It means that I must still be carefull when inserting COOPERATE
//     calls in the code.
//   - The BuildPriority has now a 'time-slice' semantic.
//
// GENERAL NOTE:
// ============
//       When BuildPriority is equal to '0', WinDEU doesn't cooperate at
//       all, like previous version of WinDEU. The overhead of COOPERATE
//       calls in the WinDEU codes is nearly null (COOPERATE tests if
//       BuildPriority != 0 before doing something).
//
#define COOPERATION_VERSION	2

#if (COOPERATION_VERSION == 1)

// Maximum and minumum possible values for the BuildPriority.
#define BUILD_PRIORITY_MIN	  0	// no multi-tasking
#define BUILD_PRIORITY_MAX	100

#elif (COOPERATION_VERSION == 2)
// Maximum and minumum possible values for the BuildPriority.
// You should never use a BuildPriority too small (1-15), because
// WinDEU will spend most its time giving control to Windows.
// With Pentium processor, maybe these small values could be used.
#define BUILD_PRIORITY_MIN	   0	// no multi-tasking
#define BUILD_PRIORITY_MAX	1000	// 1 second

#endif	// COOPERATION_VERSION


extern void Cooperate(void);


#if (COOPERATION_VERSION == 1)

extern int CoopCount;
extern Bool  CoopExecTab[];
void BuildCoopExecTab(void);

#define COOPERATE()                      		\
{                                        		\
	if ( BuildPriority != 0 )            		\
	{                                    		\
		CoopCount++;                     		\
		if ( CoopCount >= BUILD_PRIORIY_MAX ) 	\
			CoopCount = 0;               		\
		if (CoopExecTab[CoopCount])      		\
			Cooperate();                 		\
	}                                    		\
}

#elif (COOPERATION_VERSION == 2)

// Last time (tick count) Cooperate was called by COOPERATE
extern ULONG LastCoopCallTick;

#define COOPERATE()                      									\
{                                        									\
	if ( BuildPriority > 0 )												\
	{                                                           			\
		if ( ::GetTickCount() - LastCoopCallTick >= (ULONG)BuildPriority )	\
		{                                    								\
			Cooperate();                                        			\
			LastCoopCallTick = ::GetTickCount();							\
		}                                    								\
	}                                                           			\
}

#endif	// COOPERATION_VERSION


#endif  /* __windeu_h */
/* end of file */

