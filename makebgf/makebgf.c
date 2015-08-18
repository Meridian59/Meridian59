// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * makebgf.c:  Main routines for makebgf.
 * This program combines Windows bitmaps in the .bmp format into a private BlakSton
 * format.  The output is a file containing all the bitmaps for one object, seen from
 * different views or performing different actions.
 */

#include "makebgf.h"

#define MAX_ARGS 10000       // Maximum # of command line args

#define DEFAULT_PERIOD 100  // Default period of animations

Bitmaps b;

static void Usage(void);
static void ReadCommandLine(int argc, char **argv, Options *options);
static void ReadPoint(char *arg1, char *arg2, POINT *p);
static void CommandFileLoad(char *filename, int *argc, char ***argv, int pos);

/************************************************************************/
/*
 * SafeMalloc:  Calls _fmalloc to get small chunks of memory ( < 64K).
 *   Need to use this procedure name since util library calls it.
 */
void *SafeMalloc(UINT bytes)
{
   void *temp = (void *) malloc((UINT) bytes);
   if (temp == NULL)
      Error("Out of memory!\n");

   return temp;
}
/************************************************************************/
/*
 * SafeFree:  Free a pointer allocated by SafeMalloc.
 */
void SafeFree(void *ptr)
{
   if (ptr == NULL)
   {
      Error("Freeing a NULL pointer!\n");
      return;
   }
   free(ptr);
}
/**************************************************************************/
/*
 * ReadCommandLine:  Go through bitmap description list on command line,
 *   and fill in info structure.  This includes loading the bitmaps in.
 *   "shrink" gives the shrink factor, already read in from the command line.
 */
void ReadCommandLine(int argc, char **argv, Options *options)
{
   int i, j, arg = 0;  // Argument we're currently looking at

   // First, load bitmaps 
   b.num_bitmaps = atoi(argv[0]);
   
   if (b.num_bitmaps < 1)
      Error("Number of bitmaps must be at least 1");
   arg++;
   
   b.bitmaps = (PDIB *) SafeMalloc(b.num_bitmaps * sizeof(PDIB));
   b.offsets = (POINT *) SafeMalloc(b.num_bitmaps * sizeof(POINT));
   b.hotspots = (Hotspots *) SafeMalloc(b.num_bitmaps * sizeof(Hotspots));
   for (i=0; i < b.num_bitmaps; i++)
   {
      if (arg >= argc)
         Error("Not enough bitmap files given");
      
      b.bitmaps[i] = DibOpenFile(argv[arg]);
      if (b.bitmaps[i] == NULL)
         Error("Unable to load bitmap file %s", argv[arg]);
      
      arg++;
      
      // Read offsets, if present
      b.offsets[i].x = 0;
      b.offsets[i].y = 0;
      if (arg <= argc - 2 && argv[arg][0] == '[')
      {
         ReadPoint(argv[arg] + 1, argv[arg+1], &b.offsets[i]);
         arg += 2;  // Skip read arguments
      }
      
      // Read hotspots, if present
      b.hotspots[i].num_hotspots = 0;
      
      if (arg <= argc - 1 && argv[arg][0] == ':')
      {
         b.hotspots[i].num_hotspots = atoi(argv[arg] + 1);
         arg++;
         
         b.hotspots[i].positions = 
            (POINT *) SafeMalloc(b.hotspots[i].num_hotspots * sizeof(POINT));
         b.hotspots[i].numbers = (char *) SafeMalloc(b.hotspots[i].num_hotspots * sizeof(BYTE));
         for (j=0; j < b.hotspots[i].num_hotspots; j++)
         {
            if (arg > argc - 3)
               Error("Not enough hotspots for bitmap #%d", i);
            
            b.hotspots[i].numbers[j] = atoi(argv[arg]);
            if (b.hotspots[i].numbers[j] == 0)
               Error("Hotspot #%d for bitmap #%d must not be 0", j + 1, i + 1);
            arg++;
            
            ReadPoint(argv[arg] + 1, argv[arg+1], &b.hotspots[i].positions[j]);
            arg += 2;  // Skip read arguments
         }
      }
   }
   
   // Next, store indices into structure
   if (arg >= argc)
      Error("Missing number of index groups");
   b.num_groups = atoi(argv[arg]);
   b.groups = (Group *) SafeMalloc(b.num_groups * sizeof(Group));
   arg++;
   
   for (i=0; i < b.num_groups; i++)
   {
      Group *g = &b.groups[i];
      if (arg >= argc)
         Error("Missing number of indices in index group %d", i+1);
      
      g->num_indices = atoi(argv[arg]);
      if (g->num_indices < 1)
         Error("Number of indices in a group must be at least 1");
      arg++;
      
      g->indices = (int *) SafeMalloc(g->num_indices * sizeof(int));
      
      // Read in the indices
      for (j=0; j < g->num_indices; j++)
      {
         if (arg >= argc)
            Error("Not enough indices in group %d", i+1);
         
         g->indices[j] = atoi(argv[arg]) - 1;  // Start counting from 0 in file
         if (g->indices[j] < -1)
            Error("Index must be at least 0");
         arg++;
      }
   }
   
   if (arg != argc)
      printf("Extra command line arguments ignored\n");
}
/**************************************************************************/
/*
 * ReadPoint:  Read x and y offsets from given arguments into POINT structure.
 */
void ReadPoint(char *arg1, char *arg2, POINT *p)
{
   sscanf(arg1, "%d", &p->x);
   sscanf(arg2, "%d", &p->y);
}
/**************************************************************************/
/*
 * CommandFileLoad:  Read in given file, and modify argc and argv so that
 *   they show contents of file as command line arguments.
 *   pos is argument number of command file (starting with 1)
 */
void CommandFileLoad(char *filename, int *argc, char ***argv, int pos)
{
   FILE *file;
   char line[500], *arg, **oldargs;
   int i;

   file = fopen(filename, "r");
   if (file == NULL)
      Error("Couldn't open command file %s", filename);

   // Save initial arguments
   oldargs = *argv;

   // Make room for new command line
   *argc = pos;
   *argv = (char **) SafeMalloc((MAX_ARGS + 1) * sizeof(char *));

   // Copy initial arguments
   for (i=0; i < pos; i++)
      (*argv)[i] = oldargs[i];

   while (fgets(line, 500, file) != NULL) 
   {
      if (line[strlen(line) - 1] == '\n')
         line[strlen(line) - 1] = 0;       // Kill newline
      // Skip comments
      if (line[0] == '#')
         continue;       
      
      for (arg = strtok(line, " \t"); arg != NULL; arg = strtok(NULL, " \t"))	 
      {
         // Skip backslashes, since they are just line continuation chars
         if (!strcmp(arg, "\\"))
            continue;
         
         *argc = *argc + 1;
         (*argv)[*argc - 1] = strdup(arg);
         
         if (*argc == MAX_ARGS)
            Error("Too many command line arguments; max is %d.", MAX_ARGS);
      }
   }
   
   // Print out arguments for debugging
#if 0
   for (i = 0; i < *argc; i++)
   {
      printf("Arg %d = %s\n", i, (*argv)[i]);
   }
#endif

   fclose(file);
}
/**************************************************************************/
/*
 * VerifyArguments:  Return True iff Bitmaps structure is valid.
 */
Bool VerifyArguments(Bitmaps *b, Options *options)
{
   int i, j;

   for (i=0; i < b->num_groups; i++)
   {
      for (j=0; j < b->groups[i].num_indices; j++)
	 if (b->groups[i].indices[j] >= b->num_bitmaps)
	    Error("Bitmap #%d in group %d is out of range (max is %d)",
		  b->groups[i].indices[j] + 1, i + 1, b->num_bitmaps);
   }
   return True;
}
/**************************************************************************/
void Error(char *fmt,...)
{
   char s[200];
   va_list marker;
    
   va_start(marker,fmt);
   vsprintf(s,fmt,marker);
   va_end(marker);

   printf("%s\n", s);
   exit(1);
}
/**************************************************************************/
void Usage(void)
{
   printf("Usage:  makebgf -o <output file> <bitmap list> <index list>\n");
   printf("    or  makebgf @<command_file>\n");
   printf("\n");
   printf("Options:\n");
   printf("-r              rotate bitmap 90 degrees (used for wall textures)\n");
   printf("-n <string>     string is name of bitmap (defaults to empty string)\n");
   printf("-s <shrink>     shrink bitmap by given factor when drawing (defaults to 1)\n");
   printf("-x              don't compress output\n");
   printf("\n");
   printf("<bitmap list> = number of bitmaps, followed by that many <bitmap>s.\n");
   printf("<index list>  = number of groups, followed by that many groups\n");
   printf("<group>       = number of indices, followed by that many indicies\n");
   printf("An index refers to the bitmap list, where the first bitmap is counted as 1\n");
   printf("\n");
   printf("<bitmap> = filename of bitmap, followed optionally by:\n");
   printf("           [x, y]                               bitmap's offset\n");
   printf("           :n m1 [x1, y1] m2 [x2, y2], ..., mn [xn, yn]  n hotspot locations,\n");
   printf("             abs(mi) is the hotspot number, [x, y] is the hotspot position\n");
   printf("             negative hotspots are underlays; positive are overlays\n");
   exit(1);
}
/**************************************************************************/
int main(int argc, char **argv)
{
   int arg, len;
   char *output_filename = NULL;
   int output_file_found = 0;
   Options options;

   options.shrink = 1;
   options.rotate = False;
   options.compress = True;
   memset(&b.name, 0, MAX_BITMAPNAME);

   /* Parse options */
   for (arg = 1; arg < argc; arg++)
   {
      len = strlen(argv[arg]);
      if (len == 0)
	 break;

      // Look for command file
      if (argv[arg][0] == '@')
      {
	 CommandFileLoad(argv[arg] + 1, &argc, &argv, arg);
	 arg--;    // Need to reparse this argument next time through; it's been changed
	 continue;
      }
      
      if (argv[arg][0] != '-')
	 break;

      if (len < 2)
      {
	 printf("Ignoring unknown option -\n");
	 continue;
      }

      switch(argv[arg][1])
      {
      case 'o':
	 arg++;
	 if (arg >= argc)
	 {
	    printf("Missing output filename\n");
	    break;
	 }
	 output_filename = argv[arg];
	 output_file_found = 1;
	 break;

      case 'n':
	 arg++;
	 if (arg >= argc)
	 {
	    printf("Missing bitmap name\n");
	    break;
	 }
	 strncpy(b.name, argv[arg], MAX_BITMAPNAME);
	 b.name[MAX_BITMAPNAME - 1] = 0;
	 break;

      case 's':
	 arg++;
	 if (arg >= argc)
	 {
	    printf("Missing shrink factor; assuming 1\n");
	    break;
	 }
	 options.shrink = atoi(argv[arg]);
	 if (options.shrink == 0)
	 {
	    printf("Illegal shrink factor value %d; assuming 1\n", options.shrink);
	    options.shrink = 1;
	 }
	 break;

      case 'r':
	 options.rotate = True;
	 break;

      case 'x':
	 options.compress = False;
	 break;

      case '?':
	 Usage();
	 exit(0);

      default:
	 printf("Ignoring unknown option %c\n", argv[arg][1]);
      }
      
   }

   if (!output_file_found)
      Error("No output file specified");
   
   if (arg >= argc)
      Error("Missing bitmap description list");

   ReadCommandLine(argc - arg, argv + arg, &options);

   if (!VerifyArguments(&b, &options))
      exit(1);

   if (!WriteBGFFile(&b, &options, output_filename))
      Error("Error writing output file");

   return 0;
}
