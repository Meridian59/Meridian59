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
bool EachLoadRsc(char *filename, int resource_num, int lang_id, char *string);
Bool LoadDynamicRscName(char *filename);

void LoadRsc(void)
{
	char file_load_path[MAX_PATH+FILENAME_MAX];
	
	int files_loaded = 0;
	sprintf(file_load_path,"%s%s",ConfigStr(PATH_RSC),ConfigStr(RESOURCE_RSC_SPEC));
   StringVector files;
   if (FindMatchingFiles(file_load_path, &files))
   {
      for (StringVector::iterator it = files.begin(); it != files.end(); ++it)
      {
			sprintf(file_load_path,"%s%s",ConfigStr(PATH_RSC), it->c_str());

			if (RscFileLoad(file_load_path,EachLoadRsc))
				files_loaded++;
			else
				eprintf("LoadRsc error loading %s\n", it->c_str());
		}
	}
	
	/*
	dprintf("LoadRsc loaded %i of %i found .rsc files\n",files_loaded,files.size());
	*/
}

bool EachLoadRsc(char *filename, int resource_num, int lang_id, char *string)
{
	AddResource(resource_num,lang_id,string);
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
		AddResource(resource_id,0,resource_value);
	}

	fclose(fh);
	return True;
}

