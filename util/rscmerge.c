/*
 * rscmerge.c:  Combine multiple rsc files into one.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rscload.h"

typedef struct _Resource {
   int   number;
   char *string[MAX_LANGUAGE_ID];
   struct _Resource *next;
} Resource;

static Resource *resources;

static const int RSC_VERSION = 5;
static char rsc_magic[] = {0x52, 0x53, 0x43, 0x01};

static void Error(char *fmt, ...);

/************************************************************************/
/*
 * SafeMalloc:  Calls _fmalloc to get small chunks of memory ( < 64K).
 *   Need to use this procedure name since util library calls it.
 */
void *SafeMalloc(unsigned int bytes)
{
   void *temp = (void *) malloc(bytes);
   if (temp == NULL)
   {
      Error("Out of memory!\n");
      exit(1);
   }

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
/***************************************************************************/
void Usage(void)
{
   printf("Usage: rscmerge -o <output filename> <input filename> ...\n");
   exit(1);
}
/**************************************************************************/
void Error(char *fmt, ...)
{
   char s[200];
   va_list marker;
    
   va_start(marker,fmt);
   vsprintf(s,fmt,marker);
   va_end(marker);

   printf("%s\n", s);
   exit(1);
}
/***************************************************************************/
/*
 * SaveRscFile:  Save contents of global var resources to given rsc filename.
 *   Return true on success.
 */
bool SaveRscFile(char *filename)
{
   int num_resources, i, temp;
   Resource *r;
   FILE *f;

   /* Count resources */
   num_resources = 0;
   for (r = resources; r != NULL; r = r->next)
   {
      for (int j = 0; j < MAX_LANGUAGE_ID; j++)
      {
         if (r->string[j])
            num_resources++;
      }
   }

   /* If no resources, do nothing */
   if (num_resources == 0)
      return true;

   f = fopen(filename, "wb");
   if (f == NULL)
      return false;
   
   /* Write out header information */
   for (i=0; i < 4; i++)
      fwrite(&rsc_magic[i], 1, 1, f);

   temp = RSC_VERSION;
   fwrite(&temp, 4, 1, f);
   fwrite(&num_resources, 4, 1, f);

   /* Loop through classes in this source file, and then their resources */
   for (r = resources; r != NULL; r = r->next)
   {
      // For each language string present,
      // write out language data and string.
      for (int j = 0; j < MAX_LANGUAGE_ID; j++)
      {
         if (r->string[j])
         {
            // Write out id #
            fwrite(&r->number, 4, 1, f);

            // Write language id.
            fwrite(&j,4,1,f);

            // Write string.
            fwrite(r->string[j], strlen(r->string[j]) + 1, 1, f);
         }
      }
   }

   fclose(f);
   return true;
}
/***************************************************************************/
/*
 * EachRscCallback:  Called for each resource that's loaded.
 */
bool EachRscCallback(char *filename, int rsc, int lang_id, char *string)
{
   Resource *r;

   // See if we've added this resource already.
   for (r = resources; r != NULL; r = r->next)
   {
      if (r->number == rsc)
      {
         r->string[lang_id] = strdup(string);

         return true;
      }
   }

   r = (Resource *) SafeMalloc(sizeof(Resource));
   r->number = rsc;

   for (int i = 0; i < MAX_LANGUAGE_ID; i++)
      if (i == lang_id)
         r->string[i] = strdup(string);
      else
         r->string[i] = NULL;

   r->next = resources;
   resources = r;

   return true;
}
/***************************************************************************/
/*
 * LoadRscFiles:  Read resources from given rsc files into global resources variable.
 *   Return true on success.
 */
bool LoadRscFiles(int num_files, char **filenames)
{
   int i;

   for (i=0; i < num_files; i++)
   if (!RscFileLoad(filenames[i], EachRscCallback))
   {
      printf("Failure reading rsc file %s!\n", filenames[i]);
      return false;
   }

   return true;
}
/***************************************************************************/
int main(int argc, char **argv)
{
   int arg, len;
   char *output_filename = NULL;
   int output_file_found = 0;
   
   if (argc < 3)
      Usage();
   
   for (arg = 1; arg < argc; arg++)
   {
      len = strlen(argv[arg]);
      if (len == 0)
         break;
      
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
      }
   }
   
   if (!output_file_found)
      Error("No output file specified");
   
   if (arg >= argc)
      Error("No input files specified");

   if (!LoadRscFiles(argc - arg, argv + arg))
      Error("Unable to load rsc files.");

   if (!SaveRscFile(output_filename))
      Error("Unable to save rsc file.");
}
