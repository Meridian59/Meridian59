/*
 * rscprint.c:  A short test program to dump the contents of a rsc file.
 */

#include <windows.h>
#include <stdio.h>

#include "rscload.h"
#include "wrap.h"

static int num;

/***************************************************************************/
void Usage(void)
{
   printf("Usage: rscprint <filename> ...\n");
   exit(1);
}
/***************************************************************************/
/*
 * test_callback:  Just print out rscs.
 */
BOOL test_callback(char *filename, int rsc, char *name)
{
   printf("rsc # = %d, string = %s\n", rsc, name);
   num++;
   return TRUE;
}
/***************************************************************************/
int main(int argc, char **argv)
{
   int i;

   if (argc < 2)
      Usage();

   num = 0;

   WrapInit();

   for (i=1; i < argc; i++)
   {
      printf("*** File %s\n", argv[i]);
      if (!RscFileLoad(argv[i], test_callback))
	 printf("Failure reading rsc file!\n");
   }

   WrapShutdown();

   printf("Total: %d resources\n", num);
}
