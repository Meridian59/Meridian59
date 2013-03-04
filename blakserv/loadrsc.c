// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* loadrsc.c
*

  This module loads in the saved game dynarsc files and tells blakres.c
  about what it finds.  The .rsc format is described in resource.txt.
  These are stored in a series of files in the save game.
  
*/

#include "blakserv.h"

/* local function prototypes */
bool EachLoadRsc(char *filename,int resource_num, char *string);
Bool LoadDynamicRscName(char *filename);

void LoadRsc(void)
{
	HANDLE hFindFile;
	WIN32_FIND_DATA search_data;
	int files_found;
	int files_loaded;
	char file_load_path[MAX_PATH+FILENAME_MAX];
	
	files_found = 0;
	files_loaded = 0;
	sprintf(file_load_path,"%s%s",ConfigStr(PATH_RSC),ConfigStr(RESOURCE_RSC_SPEC));
	hFindFile = FindFirstFile(file_load_path,&search_data);
	if (hFindFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			files_found++; 
			sprintf(file_load_path,"%s%s",ConfigStr(PATH_RSC),search_data.cFileName);

			if (RscFileLoad(file_load_path,EachLoadRsc))
			{
				files_loaded++;
			}
			else
				eprintf("LoadRsc error loading %s\n",search_data.cFileName);
		} while (FindNextFile(hFindFile,&search_data));
		FindClose(hFindFile);
	}
	
	/*
	dprintf("LoadRsc loaded %i of %i found .rsc files\n",files_loaded,files_found);
	*/
}

bool EachLoadRsc(char *filename,int resource_num,char *string)
{
	AddResource(resource_num,string);
	return true;
}


void LoadDynamicRsc(char *filename)
{
	if (LoadDynamicRscName(filename) == False)
		eprintf("LoadDynamicRsc error loading %s\n",filename);
}

Bool LoadDynamicRscName(char *filename)
{
	FILE *fh = fopen(filename,"rb");
   int magic_num;
   int version;
   unsigned int num_resources;
	unsigned int i;
	
	if (fh == NULL)
		return False;

	fread(&magic_num,4,1,fh);
	fread(&version,4,1,fh);
	fread(&num_resources,4,1,fh);
	if (ferror(fh) || feof(fh))
	{
		fclose(fh);
		return False;
	}

	if (version != 1)
	{
		eprintf("LoadDynamicRscName can't understand rsc version != 1\n");
		return False;
	}

	for (i=0;i<num_resources;i++)
	{
		int resource_id;
		int resource_type;
		unsigned int len_data;
		char resource_value[500];

		fread(&resource_id,4,1,fh);
		fread(&resource_type,4,1,fh);
		fread(&len_data,4,1,fh);

		if (ferror(fh) || feof(fh))
		{
			fclose(fh);
			return False;
		}

		if (len_data > sizeof(resource_value)-1)
		{
			eprintf("LoadDynamicRscName got invalid long dynamic resource %u\n",
					  len_data);
			return False;
		}

		fread(&resource_value,len_data,1,fh);

		if (ferror(fh) || feof(fh))
		{
			fclose(fh);
			return False;
		}
		resource_value[len_data] = '\0'; // null-terminate string
		//dprintf("got %u %s\n",resource_id,resource_value);
		AddResource(resource_id,resource_value);
	}

	fclose(fh);
	return True;
}

