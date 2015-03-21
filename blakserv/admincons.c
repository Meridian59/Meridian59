// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* admincons.c
*

  Has a list of constant values for admin mode.
  
*/

#include "blakserv.h"

#define MAX_CONSTANTS_LINE 200

admin_constant_node *admin_constants;

/* local function prototypes */
void AddAdminConstant(char *name,int value);


void InitAdminConstants(void)
{
	admin_constants = NULL;
}

void ResetAdminConstants(void)
{
	admin_constant_node *ac,*temp;
	
	ac = admin_constants;
	while (ac != NULL)
	{
		temp = ac->next;
		FreeMemory(MALLOC_ID_ADMIN_CONSTANTS,ac->name,strlen(ac->name)+1);
		FreeMemory(MALLOC_ID_ADMIN_CONSTANTS,ac,sizeof(admin_constant_node));
		ac = temp;
	}
	admin_constants = NULL;
	
}

void AddAdminConstant(char *name,int value)
{
	admin_constant_node *ac;
	
	ac = (admin_constant_node *)AllocateMemory(MALLOC_ID_ADMIN_CONSTANTS,
		sizeof(admin_constant_node));
	
	ac->name = (char *)AllocateMemory(MALLOC_ID_ADMIN_CONSTANTS,
		strlen(name)+1);
	strcpy(ac->name,name);
	ac->value = value;
	ac->next = admin_constants;
	
	admin_constants = ac;
	
}

void LoadAdminConstants(void)
{
	FILE *constantsfile;
	char line[MAX_CONSTANTS_LINE];
	int lineno;
	char *name_str,*value_str;
	int value;
	
	if (ConfigBool(CONSTANTS_ENABLED) == False)
		return;
	
	if ((constantsfile = fopen(ConfigStr(CONSTANTS_FILENAME),"rt")) == NULL)
	{
		eprintf("LoadAdminConstants can't open %s, no constants\n",
			ConfigStr(CONSTANTS_FILENAME));
		return;
	}
	
	lineno = 0;
	
	while (fgets(line,MAX_CONSTANTS_LINE,constantsfile))
	{
		lineno++;
		
		name_str = strtok(line,"= \t\n");
		
		if (name_str == NULL)	/* ignore blank lines */
			continue;
		
		if (name_str[0] == '%')	/* ignore comments lines */
			continue;
		
		value_str = strtok(NULL,"= \t\n");
		
		if (name_str == NULL || name_str[0] == '%')
		{
			eprintf("LoadAdminConstants error reading value on line %i\n",lineno);
			continue;
		}
		
		if (sscanf(value_str,"%i",&value) != 1)
		{
			eprintf("LoadAdminConstants error parsing integer on line %i\n",lineno);
			continue;
		}
		
		AddAdminConstant(name_str,value);
		
	}
	
	fclose(constantsfile);
}

Bool LookupAdminConstant(const char *name,int *ret_ptr)
{
	admin_constant_node *ac;
	
	ac = admin_constants;
	while (ac != NULL)
	{
		if (stricmp(name,ac->name) == 0)
		{
			*ret_ptr = ac->value;
			return True;
		}      
		ac = ac->next;
	}
	return False;
}
