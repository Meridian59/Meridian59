// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * resource.c:  Write out resource file from symbol table information.
 */
#include "blakcomp.h"

static const int RSC_VERSION = 4;
static char rsc_magic[] = {0x52, 0x53, 0x43, 0x01};

char *GetStringFromResource(resource_type r);
/******************************************************************************/
/*
 * write_resources: Write out resources to a .rsc file.  fname should be the 
 *    name of the .rsc file to receive the resources.  Resources are found by
 *    looping through all the classes in the symbol table.
 */
void write_resources(char *fname)
{
   list_type c, l;
   resource_type r;
   int num_resources, i, temp;
   class_type cl;
   FILE *f;
   char *str;

   /* Count resources and compute length */
   num_resources = 0;
   for (c = st.classes; c != NULL; c = c->next)
   {
      cl = (class_type) c->data;
      if (cl->is_new)
         for (l = cl->resources; l != NULL; l = l->next)
         {
            r = (resource_type) (l->data);
            
            num_resources++;
         }
   }
   /* If no resources, do nothing */
   if (num_resources == 0)
      return;
   
   f = fopen(fname, "wb");
   if (f == NULL)
   {
      simple_error("Unable to open resource file %s!", fname);
      return;
   }
   
   /* Write out header information */
   for (i=0; i < 4; i++)
      fwrite(&rsc_magic[i], 1, 1, f);

   temp = RSC_VERSION;
   fwrite(&temp, 4, 1, f);
   fwrite(&num_resources, 4, 1, f);

   /* Loop through classes in this source file, and then their resources */
   for (c = st.classes; c != NULL; c = c->next)
   {
      cl = (class_type) c->data;
      if (cl->is_new)
         for (l = cl->resources; l != NULL; l = l->next)
         {
            r = (resource_type) (l->data);
            
            // Write out id #
            fwrite(&r->lhs->idnum, 4, 1, f);
            
            // Write string
            str = GetStringFromResource(r);
            fwrite(str, strlen(str) + 1, 1, f);
         }
   }

   fclose(f);
}
/***************************************************************************/
char *GetStringFromResource(resource_type r)
{
   switch (r->rhs->type)
   {
   case C_STRING:
      return r->rhs->value.stringval;

   case C_FNAME:
      return r->rhs->value.fnameval;

   default:
      simple_error("Unknown resource type (%d) encountered", r->rhs->type);
      return NULL;
   }
}
