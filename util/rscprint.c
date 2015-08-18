/*
 * rscprint.c:  A short test program to dump the contents of a rsc file.
 */

#include <stdio.h>
#include <stdlib.h>

#include "rscload.h"

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
bool test_callback(char *filename, int rsc, int lang_id, char *str)
{
   printf("rsc # = %d, language id = %d, string = %s\n", rsc,lang_id,str);
   
   num++;
   return true;
}
/***************************************************************************/
int main(int argc, char **argv)
{
   int i;

   if (argc < 2)
      Usage();

   num = 0;

   for (i=1; i < argc; i++)
   {
      printf("*** File %s\n", argv[i]);
      if (!RscFileLoad(argv[i], test_callback))
         printf("Failure reading rsc file!\n");
   }

   printf("Total: %d resources\n", num);
}
