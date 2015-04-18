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

	FILE:         windeu.cpp

	Windows Doom Editor Utility, by Brendon Wyber, Raphael Quinet
	and Renaud Paquay.

	OVERVIEW
	========
	Source file for main program execution routines.
*/

#include "common.h"
#pragma hdrstop

#include <time.h>

#ifndef OWL_DIALOG_H
	#include <owl\dialog.h>
#endif

#ifndef OWL_INPUTDIA_H
	#include <owl\inputdia.h>
#endif

#ifndef OWL_TEXTGADG_H
	#include <owl\textgadg.h>
#endif

#ifndef OWL_STATIC_H
	#include <owl\static.h>
#endif

#ifndef __windeapp_h
	#include "windeapp.h"
#endif

#ifndef __mainfram_h
	#include "mainfram.h"
#endif

#ifndef __wads_h
	#include "wads.h"
#endif

#ifndef __levels_h
	#include "levels.h"
#endif

#ifndef __undo_h
	#include "undo.h"		// MaxUndo
#endif

#ifndef __funnydlg_h
	#include "funnydlg.rh"
#endif

#ifndef __usagedlg_h
	#include "usagedlg.rh"
#endif

// This header file is missing 7/3/04 ARK.  Apparently it was used to
// generate some coordinates used by kod.  Brian Green says they're
// not used any longer, so I'm disabling them.

//#include "entrance.h"

// Variables globales
int DoomVersion;            /* Doom version (1 or 2) */
FILE *logfile = NULL;		/* filepointer to the error log */
Bool Registered = TRUE;	    /* registered or shareware game? */
Bool Debug = FALSE;			/* are we debugging? */
Bool Quiet = FALSE;			/* don't play a sound when an object is selected */
Bool Quieter = FALSE;		/* don't play any sound, even when an error occurs */
Bool Expert = FALSE;		/* don't ask for confirmation for some operations */
char *CfgFile = DEU_CONFIG_FILE;/* name of the configuration file */
int  InitialScale = 8;		/* initial zoom factor for map */
Bool Colour2 = FALSE;		/* use the alternate set for things colors */
Bool InfoShown = TRUE;		/* should we display the info bar? */
Bool AdditiveSelBox = FALSE;	/* additive selection box or select in box only? */
int  SplitFactor = 8;		/* factor used by the Nodes builder */
Bool Select0 = TRUE;		/* select object 0 by default when switching modes */
Bool Reminder = TRUE;		/* display a funny message when DEU starts */
Bool AutoScroll = FALSE;	/* Auto scroll mode */
char *MainWad = NULL;	/* name of the main wad file */
char **PatchWads = NULL;	/* list of patch wad files */
Bool DrawLineDefsLen = FALSE;	/* Display length of moving LineDefs */
int BuildPriority = 0;		/* Priority for the nodes builder */
int RoomID = 0;

// Added ARK
char *BitmapDir = NULL;         /* Directory where bitmaps reside */
char *BitmapSpec  = NULL;       /* Filespec of bitmap files */
char *KodDir = NULL;
char *ServerDir = NULL;
char *EntranceData = NULL;

// Grid info.
SHORT  GridScale  = 8;
BOOL   SnapToGrid = FALSE;
BOOL   GridShown  = FALSE;


#define DEFAULT_MAIN_WAD		"DOOM.WAD"
#define DEFAULT_WALL_TEXTURE 	"-"
#define DEFAULT_UPPER_TEXTURE 	"-"
#define DEFAULT_LOWER_TEXTURE 	"-"
#define DEFAULT_FLOOR_TEXTURE	"-"
#define DEFAULT_CEILING_TEXTURE "-"
#define DEFAULT_BITMAP_DIR      "..\\run\\localclient\\resource"
#define DEFAULT_BITMAP_SPEC     "grd*.bgf"
#define DEFAULT_KOD_DIR			"..\\kod\\"
#define DEFAULT_SERVER_DIR      "..\\run\\server\\"
#define DEFAULT_ENTRANCE_FILE	"entrance.dat"

char *DefaultWallTexture;		/* default normal wall texture */
char *DefaultUpperTexture;		/* default upper wall texture */
char *DefaultLowerTexture;		/* default lower wall texture */
char *DefaultFloorTexture;		/* default floor texture */
char *DefaultCeilingTexture;	        /* default ceiling texture */

SHORT  DefaultFloorHeight   = 0;	/* default floor height */
// Changed default ceiling height ARK
SHORT  DefaultCeilingHeight = 64;	/* default ceiling height */
//SHORT  DefaultCeilingHeight = 128;	/* default ceiling height */
SHORT  DefaultLightLevel = 128;         /* default light level */


#if (COOPERATION_VERSION == 1)

// Counter of calls to Cooperate()
int CoopCount = 0;
Bool CoopExecTab[PRIORITY_RES];

#elif (COOPERATION_VERSION == 2)

// Last time (tick count) Cooperate was called by COOPERATE
ULONG LastCoopCallTick = 0L;

#endif	// COOPERATION_VERSION


//
// description of the command line options
//
OptDesc options[] =
{
/*   short & long names   type            message if true/changed       message if false              where to store the value */
	{ "d",  "debug",       OPT_BOOLEAN,    "Debug mode ON",				"Debug mode OFF",             &Debug          },
	{ "q",  "quiet",       OPT_BOOLEAN,    "Quiet mode ON",				"Quiet mode OFF",             &Quiet          },
	{ "qq", "quieter",     OPT_BOOLEAN,    "Quieter mode ON",			"Quieter mode OFF",           &Quieter        },
	{ "e",  "expert",      OPT_BOOLEAN,    "Expert mode ON",				"Expert mode OFF",            &Expert         },
	{ "w",  "main",        OPT_STRING,     "Main WAD file",				NULL,                         &MainWad        },
	{ NULL, "file",        OPT_STRINGLIST, "Patch WAD file",				NULL,                         &PatchWads      },
	{ "pw", "pwad",        OPT_STRINGACC,  "Patch WAD file",				NULL,                         &PatchWads      },
	{ NULL, "config",      OPT_STRING,     "Config file",				NULL,                         &CfgFile        },
	{ "z",  "zoom",        OPT_INTEGER,    "Initial zoom factor",		NULL,                         &InitialScale   },
	{ "c",  "color2",      OPT_BOOLEAN,    "Alternate Things color set",	"Normal Things color set",    &Colour2        },
	{ "i",  "infobar",     OPT_BOOLEAN,    "Info bar shown",				"Info bar hidden",            &InfoShown      },
	{ "a",  "addselbox",   OPT_BOOLEAN,    "Additive selection box",		"Select objects in box only", &AdditiveSelBox },
	{ "sf", "splitfactor", OPT_INTEGER,    "Split factor",				NULL,			      		&SplitFactor    },
	{ NULL, "walltexture", OPT_STRING,     "Default wall texture",		NULL,                   &DefaultWallTexture    },
	{ NULL, "lowertexture",OPT_STRING,     "Default lower wall texture",	NULL,                &DefaultLowerTexture   },
	{ NULL, "uppertexture",OPT_STRING,     "Default upper wall texture",	NULL,                &DefaultUpperTexture   },
	{ NULL, "floortexture",OPT_STRING,     "Default floor texture",		NULL,                &DefaultFloorTexture   },
	{ NULL, "ceiltexture", OPT_STRING,     "Default ceiling texture",	NULL,                   &DefaultCeilingTexture },
	{ NULL, "floorheight", OPT_INTEGER,    "Default floor height",		NULL,			      		&DefaultFloorHeight    },
	{ NULL, "ceilheight",  OPT_INTEGER,    "Default ceiling height",		NULL,			      	&DefaultCeilingHeight  },
	{ "s0", "select0",     OPT_BOOLEAN,    "Select 0 by default",		"No default selection",	&Select0          },
	{ NULL, "reminder1",   OPT_BOOLEAN,	  	NULL,								NULL,			      		&Reminder         },
	// New for Windows:
	{ "dl", "drawlength",  	OPT_BOOLEAN,   "LineDefs length enabled",	NULL,	  	               &DrawLineDefsLen	},
	{ "bp", "buildpriority",OPT_INTEGER,	"Nodes builder priority",	NULL,                   &BuildPriority	   },
	{ "mu", "maxundo",     	OPT_INTEGER,   "Maximum UNDO levels",		NULL,                   &MaxUndo			   },
	{ "gs", "gridsize",    	OPT_INTEGER,   NULL,	 							NULL,                   &GridScale			},
	{ "sg", "showgrid",    	OPT_BOOLEAN,   NULL,								NULL,                   &GridShown		   },
	{ "ng", "snaptogrid",  	OPT_BOOLEAN,   NULL,								NULL,                   &SnapToGrid			},
	// Added ARK
	{ "bg", "bitmapdir",   	OPT_STRING,    NULL,								NULL,                   &BitmapDir			},
	{ "sp", "bitmapspec",  	OPT_STRING,    NULL,								NULL,                   &BitmapSpec			},
	{ "sc", "autoscroll",  	OPT_BOOLEAN,   NULL,   							NULL,  						&AutoScroll			},
	{ "kod", "koddir",   	OPT_STRING,    NULL,								NULL,                   &KodDir			 	},
	{ "svr", "serverdir",   OPT_STRING,    NULL,								NULL,                   &ServerDir			},
	{ "ent", "entrances", 	OPT_STRING,		NULL,								NULL,							&EntranceData		},
	{ NULL, NULL,          	OPT_END,       NULL,								NULL,                   NULL           	}
};



/*
	Windeu initialization
*/
void InitWindeu (int argc, char **argv, char *init_level)
{
	TRACE ("InitWindeu(): start");
	argv++;
	argc--;

	// Initialize graphics data (GDI pen cache, ...)
	// InitGfxData();

	// Load zlib1.dll dynamically
	DibInitCompression();

	// Init. MainWad file name to default (DOOM.WAD)
	MainWad = (char *)GetMemory (strlen(DEFAULT_MAIN_WAD)+1);
	strcpy (MainWad, DEFAULT_MAIN_WAD);

	// Init. default texture strings
	DefaultWallTexture = (char *)GetMemory (strlen(DEFAULT_WALL_TEXTURE)+1);
	strcpy (DefaultWallTexture, DEFAULT_WALL_TEXTURE);

	DefaultUpperTexture = (char *)GetMemory (strlen(DEFAULT_UPPER_TEXTURE)+1);
	strcpy (DefaultUpperTexture, DEFAULT_UPPER_TEXTURE);

	DefaultLowerTexture = (char *)GetMemory (strlen(DEFAULT_LOWER_TEXTURE)+1);
	strcpy (DefaultLowerTexture, DEFAULT_LOWER_TEXTURE);

	DefaultFloorTexture = (char *)GetMemory (strlen(DEFAULT_FLOOR_TEXTURE)+1);
	strcpy (DefaultFloorTexture, DEFAULT_FLOOR_TEXTURE);

	DefaultCeilingTexture = (char *)GetMemory (strlen(DEFAULT_CEILING_TEXTURE)+1);
	strcpy (DefaultCeilingTexture, DEFAULT_CEILING_TEXTURE);

	// Init bitmap dir & filespec ARK
	BitmapDir = (char *)GetMemory (strlen(DEFAULT_BITMAP_DIR)+1);
	strcpy (BitmapDir, DEFAULT_BITMAP_DIR);
	BitmapSpec = (char *)GetMemory (strlen(DEFAULT_BITMAP_SPEC)+1);
	strcpy (BitmapSpec, DEFAULT_BITMAP_SPEC);

	KodDir = (char *)GetMemory (strlen(DEFAULT_KOD_DIR)+1);
	strcpy (KodDir, DEFAULT_KOD_DIR);
	ServerDir = (char *)GetMemory (strlen(DEFAULT_SERVER_DIR)+1);
	strcpy (ServerDir, DEFAULT_SERVER_DIR);
	EntranceData = (char*)GetMemory(strlen(DEFAULT_ENTRANCE_FILE)+1);
	strcpy (EntranceData, DEFAULT_ENTRANCE_FILE);

	/* quick and dirty check for a "-config" option */
	for (int i = 0; i < argc - 1; i++)
		if (!strcmp(argv[i], "-config"))
		{
			CfgFile = argv[i + 1];
			break;
		}

	/* read config file and command line options */
	ParseConfigFileOptions(CfgFile);
	ParseCommandLineOptions(argc, argv, init_level);

	// Setup builder priority vars.
	if ( BuildPriority < BUILD_PRIORITY_MIN )
		BuildPriority = BUILD_PRIORITY_MIN;
	if ( BuildPriority > BUILD_PRIORITY_MAX )
		BuildPriority = BUILD_PRIORITY_MAX;

	// Check grid size value
	if ( GridScale >= 256 ) 		GridScale = 256;
	else if ( GridScale >= 128 ) 	GridScale = 128;
	else if ( GridScale >= 64 ) 	GridScale = 64;
	else if ( GridScale >= 32 ) 	GridScale = 32;
	else if ( GridScale >= 16 ) 	GridScale = 16;
	else if ( GridScale >= 8 ) 		GridScale = 8;
	else                        	GridScale = 0;

#if (COOPERATION_VERSION == 1)
	BuildCoopExecTab();
#endif	// COOPERATION_VERSION

	ReadWTextureInfo();
	ReadFTextureInfo();

  // Disabled 7/04 ARK
//	char buffer[_MAX_PATH];
//	strcpy(buffer,KodDir);
//	strcat(buffer,"Objects.txt");
//	LoadKodObjects(buffer);
//	strcpy(buffer,KodDir);
//	strcat(buffer,"RoomID.txt");
//	LoadKodRooms(buffer);
//	RoomID = 0;
//	OpenEntrances(EntranceData);

	LogMessage(": Starting roomedit\n");
}


/*
	Windeu cleanup routine
*/
void CleanupWindeu ()
{
	// Cleanup graphics data (GDI pen cache, ...)
	// CleanupGfxData();
	ForgetLevelData();
	Level = NULL;
	ForgetWTextureInfo();
	ForgetFTextureInfo();

	// that's all, folks!
	CloseWadFiles();

	// unload compression
	DibCloseCompression();

  // Disabled 7/04 ARK
//	UnloadKodObjects();
//	UnloadKodRooms();
//	SaveEntrances(EntranceData);
//	CloseEntrances();

	LogMessage( ": The end!\n\n\n");
	CloseLog();
}



/*
	Center a window in the client area of its parent
*/

void CenterWindow (TWindow *pWnd)
{
	TRect wRect, mRect, dRect;
	TPoint wTopLeft;
	TPoint mMiddle;
	TWindow *parent = pWnd->Parent;
	TWindow DesktopWindow (::GetDesktopWindow());

	// Get parent window client rect and transform it to screen rect
	CHECK (parent != NULL);
	parent->GetClientRect (mRect);

	TPoint TopLeft = mRect.TopLeft();
	parent->ClientToScreen (TopLeft);

	TPoint BottomRight = mRect.BottomRight();
	parent->ClientToScreen (BottomRight);

	mRect.Set (TopLeft.x, TopLeft.y, BottomRight.x, BottomRight.y);


	pWnd->GetWindowRect (wRect);
	DesktopWindow.GetWindowRect (dRect);

	mMiddle = TPoint (mRect.left + mRect.Width() / 2,
					  mRect.top  + mRect.Height() / 2);
	wTopLeft = TPoint (mMiddle.x - wRect.Width() / 2,
					   mMiddle.y - wRect.Height() / 2);

	// Keep window in its parent client area
	if ( wTopLeft.x < mRect.left )		wTopLeft.x = mRect.left;
	if ( wTopLeft.y < mRect.top )		wTopLeft.y = mRect.top;

	// Keep window in screen
	if ( wTopLeft.x < 0 )		wTopLeft.x = 0;
	if ( wTopLeft.y < 0 )		wTopLeft.y = 0;

	if ( wTopLeft.x + wRect.Width() > dRect.right )
		 wTopLeft.x = dRect.right - wRect.Width();
	if ( wTopLeft.y + wRect.Height() > dRect.bottom )
		 wTopLeft.y = dRect.bottom - wRect.Height();

	pWnd->MoveWindow (wTopLeft.x, wTopLeft.y,
					  wRect.Width(), wRect.Height());
}


/*
	Class to create a dialog box centered in the main window.
*/
class TCenterDialog : public TDialog
{
public:
	TCenterDialog(TWindow* parent, TResId resId, TModule* module = 0):
		TDialog(parent, resId, module)
	{
	}

	virtual void SetupWindow()
	{
		TDialog::SetupWindow();
		CenterWindow(this);
	}
};


/*
   display a funny message dialog box
*/

void FunnyMessage()
{
	TCenterDialog (((TApplication *)::Module)->GetMainWindow(),
				   IDD_FUNNY).Execute() ;
}


/*
   Display the program usage in a dialog box
*/

void Usage()
{
	TCenterDialog (((TApplication *)::Module)->GetMainWindow(),
				   IDD_USAGE).Execute() ;
}

/*
   Handle command line options
*/

void ParseCommandLineOptions( int argc, char *argv[], char *init_level)
{
	int optnum;

	init_level[0] = 0;
	while (argc > 0)
	{
		if (argv[0][0] != '-' && argv[0][0] != '+')
		{
//			Notify ("Options must start with '-' or '+'");
		   strcpy(init_level, argv[0]);
			argc--;
			argv++;
			continue;
		}

		if ( !strcmp (argv[0], "-?") ||
			 !stricmp(argv[0], "-h") ||
			 !stricmp(argv[0], "-help"))
		{
			Usage();
			exit(1);
		}

		for (optnum = 0; options[optnum].opt_type != OPT_END; optnum++)
		{
			//BUG: We must test there's a short and/or long name before
			//     using stricmp.
			if ( (options[optnum].short_name != NULL &&
				  !stricmp(&(argv[0][1]), options[optnum].short_name)) ||
				 (options[optnum].long_name != NULL &&
				  !stricmp(&(argv[0][1]), options[optnum].long_name)) )
			{
				switch (options[optnum].opt_type)
				{
				case OPT_BOOLEAN:
					if (argv[ 0][ 0] == '-')
					{
						*((Bool *) (options[ optnum].data_ptr)) = TRUE;
						if (options[ optnum].msg_if_true)
							WorkMessage("%s", options[ optnum].msg_if_true) ;
					}
					else
					{
						*((Bool *) (options[ optnum].data_ptr)) = FALSE;
						if (options[ optnum].msg_if_false)
							WorkMessage("%s", options[ optnum].msg_if_false);
					}
					break;

				case OPT_INTEGER:
					if (argc <= 1)
					{
						Notify ("Missing argument after \"%s\"", argv[0]);
						argc--;
						argv++;
						break;
					}
					argv++;
					argc--;
					*((int *) (options[ optnum].data_ptr)) = atoi( argv[ 0]);
					if (options[ optnum].msg_if_true)
						WorkMessage("%s: %d", options[ optnum].msg_if_true,
											  atoi( argv[ 0]));
					break;

				case OPT_STRING:
					if (argc <= 1)
					{
						Notify ("missing argument after \"%s\"", argv[0]);
						argc--;
						argv++;
						break;
					}
					argv++;
					argc--;
					*((char **) (options[ optnum].data_ptr)) = argv[ 0];
					if (options[ optnum].msg_if_true)
						WorkMessage("%s: %s", options[ optnum].msg_if_true,
											  argv[ 0]);
					break;

				case OPT_STRINGACC:
					if (argc <= 1)
					{
						Notify ("Missing argument after \"%s\"", argv[0]);
						argc--;
						argv++;
						break;
					}
					argv++;
					argc--;
					AppendItemToList( (char ***) options[ optnum].data_ptr,
									  argv[ 0]);
					if (options[ optnum].msg_if_true)
						WorkMessage("%s: %s", options[ optnum].msg_if_true,
											  argv[ 0]) ;
					break;

				case OPT_STRINGLIST:
					if (argc <= 1)
					{
						Notify ("Missing argument after \"%s\"", argv[0]);
						argc--;
						argv++;
						break;
					}
					while (argc > 1 && argv[ 1][ 0] != '-' && argv[ 1][ 0] != '+')
					{
						argv++;
						argc--;
						AppendItemToList( (char ***) options[ optnum].data_ptr,
										  argv[ 0]);
						if (options[optnum].msg_if_true)
							WorkMessage("%s: %s", options[ optnum].msg_if_true,
												  argv[ 0]);
					}
					break;

				default:
					Notify ("Unknown option type (BUG!)");
				} /* end switch */
				break;
			} /* end if */
		} /* end for */
		if (options[optnum].opt_type == OPT_END)
			Notify ("Invalid argument: \"%s\"", argv[0]);
		argv++;
		argc--;
	} /* end while */
}

/*
   read the config file
*/

void ParseConfigFileOptions(char *filename)
{
	FILE *cfgfile;
	char  line[1024];
	char *value;
	char *option;
	char *p;
	int   optnum;

	if ((cfgfile = fopen (filename, "r")) == NULL)
	{
		Notify ("Configuration file not found (%s)", filename);
		return;
	}

	while (fgets (line, 1024, cfgfile) != NULL)
	{
		if (line[0] == '#' || strlen( line) < 2)
			continue;

		if (line[ strlen( line) - 1] == '\n')
			line[ strlen( line) - 1] = '\0';

		/* skip blanks before the option name */
		option = line;
		while (isspace( option[ 0]))
			option++;

		/* skip the option name */
		value = option;
		while (value[ 0] && value[ 0] != '=' && !isspace( value[ 0]))
			value++;

		if (!value[0])
			ProgError( "invalid line in %s (ends prematurely)", filename);

		if (value[0] == '=')
		{
			/* mark the end of the option name */
			value[0] = '\0';
		}
		else
		{
			/* mark the end of the option name */
			value[0] = '\0';
			value++;

			/* skip blanks after the option name */
			while (isspace( value[ 0]))
				value++;
			if (value[ 0] != '=')
				ProgError( "invalid line in %s (no '=')", filename);
		}
		value++;

		/* skip blanks after the equal sign */
		while (isspace( value[ 0]))
			value++;

		for (optnum = 0; options[ optnum].opt_type != OPT_END; optnum++)
		{
			if (!stricmp( option, options[ optnum].long_name))
			{
				switch (options[ optnum].opt_type)
				{
					case OPT_BOOLEAN:
						if ( !stricmp(value, "yes") ||
							 !stricmp(value, "true") ||
							 !stricmp(value, "on") ||
							 !stricmp(value, "1"))
						{
							*((Bool *) (options[ optnum].data_ptr)) = TRUE;
							if (options[ optnum].msg_if_true)
								WorkMessage("%s", options[ optnum].msg_if_true);
						}
						else if ( !stricmp(value, "no") ||
								  !stricmp(value, "false") ||
								  !stricmp(value, "off") ||
								  !stricmp(value, "0"))
						{
							*((Bool *) (options[ optnum].data_ptr)) = FALSE;
							if (options[ optnum].msg_if_false)
								WorkMessage("%s", options[ optnum].msg_if_false);
						}
						else
							ProgError("Invalid value for option %s: \"%s\"", option, value);
					break;

					case OPT_INTEGER:
						*((int *) (options[ optnum].data_ptr)) = atoi( value);
						if (options[ optnum].msg_if_true)
							WorkMessage("%s: %d", options[ optnum].msg_if_true, atoi( value)) ;
						break;

					case OPT_STRING:
						p = (char *)GetMemory( (strlen( value) + 1) * sizeof( char));
						strcpy( p, value);
						*((char **) (options[ optnum].data_ptr)) = p;
						if (options[ optnum].msg_if_true)
							WorkMessage("%s: %s", options[ optnum].msg_if_true, value);
						break;

					case OPT_STRINGACC:
						p = (char *)GetMemory( (strlen( value) + 1) * sizeof( char));
						strcpy( p, value);
						AppendItemToList( (char ***) options[ optnum].data_ptr, p);
						if (options[ optnum].msg_if_true)
							WorkMessage("%s: %s", options[ optnum].msg_if_true, value);
						break;

					case OPT_STRINGLIST:
						while (value[ 0])
						{
							option = value;
							while (option[ 0] && !isspace( option[ 0]))
								option++;
							option[ 0] = '\0';
							option++;

							while (isspace( option[ 0]))
								option++;

							p = (char *)GetMemory( (strlen( value) + 1) * sizeof( char));
							strcpy( p, value);
							AppendItemToList( (char ***) options[ optnum].data_ptr, p);

							if (options[ optnum].msg_if_true)
								WorkMessage("%s: %s", options[ optnum].msg_if_true, value);
							value = option;
						}
						break;

					default:
						ProgError("Unknown option type (BUG!)");
				}
				break;
			}
		}
		if (options[ optnum].opt_type == OPT_END)
			ProgError("Invalid option in %s: \"%s\"", filename, option);
	}

	fclose( cfgfile);
}


/*
   Append a string to a null-terminated string list
*/

void AppendItemToList( char ***list, char *item)
{
   int i;

   i = 0;
   if (*list != NULL)
   {
	  /* count the number of elements in the list (last = null) */
	  while ((*list)[ i] != NULL)
	 i++;
	  /* expand the list */
	  *list = (char **)ResizeMemory( *list, (i + 2) * sizeof( char **));
   }
   else
   {
	  /* create a new list */
	  *list = (char **)GetMemory( 2 * sizeof( char **));
   }
	/* append the new element */
   (*list)[ i] = item;
   (*list)[ i + 1] = NULL;
}


/*
   play a fascinating tune
*/

void Beep()
{
	if (Quieter == FALSE)
	{
		MessageBeep (-1);
	}
}



/*
   play a sound
*/

void PlaySound( int /*freq*/, int /*msec*/)
{
	if (Quiet == FALSE)
	{
		// TODO
	}
}



/*
   terminate the program reporting an error
*/

void ProgError( char *errstr, ...)
{
	TRACE ("ProgError: start");
	static char msg[256];	// Safer than on stack
	va_list args;
	Beep();
	Beep();

	va_start(args, errstr);
	vsprintf(msg, errstr, args);
	if (Debug == TRUE && logfile != NULL)
	{
		fprintf( logfile, "\nProgram Error: *** ");
		vfprintf( logfile, errstr, args);
		fprintf( logfile, " ***\n");
	}
	va_end( args);
	::MessageBox (0,
				  msg,
				  "WinDEU error",
				  MB_OK | MB_ICONSTOP | MB_TASKMODAL);

	// clean up things
	CleanupWindeu();

	// Exit immediately
	TRACE ("ProgError: exit(5)");
	exit (5);
}


/*
   write a message in the log file
*/

static BOOL errorFound;

void ClearLog(void)
{
   errorFound = FALSE;
	CloseLog();
	remove(DEU_LOG_FILE);
}

void CloseLog(void)
{
   if (NULL != logfile)
		fclose(logfile);
   logfile = NULL;
   if (errorFound)
   {
      char buffer[256];
		wsprintf(buffer,"NOTEPAD.EXE %s",DEU_LOG_FILE);
      WinExec(buffer,SW_SHOW);
   }
}

void LogMessage(char *logstr, ...)
{
	va_list  args;
	time_t   tval;
	char    *tstr;

	// Open logfile if closed
	if (Debug == TRUE && logfile == NULL)
	{
		logfile = fopen(DEU_LOG_FILE, "a");
		if (logfile == NULL)
		{
			Notify ("Could not open log file \"%s\"", DEU_LOG_FILE);
		}
	}
	if (Debug == TRUE && logfile != NULL)
	{
		va_start( args, logstr);
		/* if the messsage begins with ":", output the current date & time first */
		if (logstr[ 0] == ':')
		{
			time( &tval);
			tstr = ctime( &tval);
			tstr[ strlen( tstr) - 1] = '\0';
			fprintf(logfile, "%s", tstr);
		}
		vfprintf( logfile, logstr, args);
		va_end( args);
		fflush (logfile);
	}
}


void LogError(char *logstr, ...)
{
	va_list  args;
	time_t   tval;
	char    *tstr;

   errorFound = TRUE;
	// Open logfile if closed
	if (Debug == TRUE && logfile == NULL)
	{
		logfile = fopen(DEU_LOG_FILE, "a");
		if (logfile == NULL)
		{
			Notify ("Could not open log file \"%s\"", DEU_LOG_FILE);
		}
	}
	if (Debug == TRUE && logfile != NULL)
	{
		va_start( args, logstr);
		/* if the messsage begins with ":", output the current date & time first */
		if (logstr[ 0] == ':')
		{
			time( &tval);
			tstr = ctime( &tval);
			tstr[ strlen( tstr) - 1] = '\0';
			fprintf(logfile, "%s", tstr);
		}
		vfprintf( logfile, logstr, args);
		va_end( args);
		fflush (logfile);
	}
}

/*
   write a message in the status bar of the main frame
*/

void WorkMessage (char *workstr, ...)
{
	va_list		args;
	static char msg[256];

	if (!::Module)
		return;

	TMainFrame *mainFrame =
		TYPESAFE_DOWNCAST(((TApplication*)::Module)->GetMainWindow(), TMainFrame);

	va_start( args, workstr);
	vsprintf( msg, workstr, args);
	va_end( args);

	if (!mainFrame)
		return;

	TStatusBar* bar = mainFrame->GetStatusBar();

	if (!bar)
		return;

	if (bar->GadgetCount() > 0)
		((TTextGadget *)bar->FirstGadget())->SetText(msg);

	bar->UpdateWindow();
}


/*
   Get the message in the status bar
*/

void GetWorkMessage (char *buffer, size_t bufferSize)
{
	if (!::Module)
		return;

	TMainFrame *mainFrame =
		TYPESAFE_DOWNCAST(((TApplication *)::Module)->GetMainWindow(), TMainFrame);

	if (!mainFrame || !mainFrame->GetStatusBar() || mainFrame->GetStatusBar()->GadgetCount() == 0)
		return;

	strncpy (buffer,
			 ((TTextGadget *)mainFrame->GetStatusBar()->FirstGadget())->GetText(),
			 bufferSize);
	buffer[bufferSize-1] = '\0';
}


/*
   Ask the user to confirm a choice (message box)
*/

BOOL Confirm(char *confstr, ...)
{
	va_list  args;
	char msg[256];	// Safer than on stack

	va_start( args, confstr);
	vsprintf( msg, confstr, args);
	va_end( args);

	int ans = ::MessageBox (0,
							msg,
							"Confirmation",
							MB_YESNO | MB_ICONQUESTION | MB_TASKMODAL);

	return ((ans == IDYES) ? TRUE : FALSE);
}


/*
	Notify the user of a message in a message box
*/

void Notify(char *notstr, ...)
{
	va_list  args;
	char msg[256];

	va_start( args, notstr);
	vsprintf( msg, notstr, args);
	va_end( args);

	::MessageBox (0,
				  msg,
				  "Notification message",
				  MB_OK | MB_ICONINFORMATION | MB_TASKMODAL);
}



#if (COOPERATION_VERSION == 1)
/*
	Let's look at a CoopTabExec with a PRIORITY_RES of 10:
	Each '__' case means FALSE in the CoopTabExec array
	Each 'xx' case means TRUE in the CoopTabExec array
	The rule is to put 'xx' in a case when when change
	skip a PRIORITY_RES multiple value

	Priority = 1
	|__|__|__|__|__|__|__|__|__|xx|
	 1  2  3  4  5  6  7  8  9 10

	Priority = 2
	|__|__|__|__|xx|__|__|__|__|xx|
	 2  4  6  8 10 12 14 16 18 20

	Priority = 3
	|__|__|__|xx|__|__|xx|__|__|xx|
	 3  6  9 12 15 18 21 24 27 30

	Priority = 4
	|__|__|xx|__|xx|__|__|xx|__|xx|
	 4  8 12 16 20 24 28 32 36 40

	Priority = 10
	|xx|xx|xx|xx|xx|xx|xx|xx|xx|xx|
	10 20 30 40 50 60 70 80 90 100

*/

void BuildCoopExecTab(void)
{
	SHORT pOldVal = 0;
	SHORT pVal    = BuildPriority;

	for (SHORT i = 0 ; i < PRIORITY_RES ; i++)
	{
		if ( BuildPriority == 0 )
		{
			CoopExecTab[i] = TRUE;
		}
		else
		{
			if ( (pVal / PRIORITY_RES) > (pOldVal / PRIORITY_RES) )
				CoopExecTab[i] = TRUE;
			else
				CoopExecTab[i] = FALSE;

			pOldVal = pVal;
			pVal   += BuildPriority;
		}
	}
}
#endif	// COOPERATION_VERSION


/*
	Cooperate with others apps under Windows
*/
void Cooperate(void)
{
	MSG msg;

#if (COOPERATION_VERSION == 1)
	// We dispatch only 1 message
	if ( ::PeekMessage (&msg, NULL, 0, 0, PM_REMOVE) )
#elif (COOPERATION_VERSION == 2)
	// We dispatch a maximum of 100 messages/secs.
	/*
	int maxpeek = (BuildPriority + 9) / 10;
	for (int i = 0;
		 (i < maxpeek) && ::PeekMessage (&msg, NULL, 0, 0, PM_REMOVE);
		 i++ )
	*/
	while ( ::PeekMessage (&msg, NULL, 0, 0, PM_REMOVE) )
#endif	// COOPERATION_VERSION
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}



/*
	Format a number into a string
*/
char *FormatNumber(LONG num)
{
	static char str[30];
	int num1 = (int)((num / 1000000000L) % 1000L);
	int num2 = (int)((num / 1000000L) % 1000L);
	int num3 = (int)((num / 1000L) % 1000L);
	int num4 = (int)(num % 1000L);

	if ( num1 > 0 )
		wsprintf (str, "%d.%03d.%03d.%03d", num1, num2, num3, num4);
	else if ( num2 > 0 )
		wsprintf (str, "%d.%03d.%03d", num2, num3, num4);
	else if ( num3 > 0 )
		wsprintf (str, "%d.%03d", num3, num4);
	else
		wsprintf (str, "%d", num4);

	return str;
}

