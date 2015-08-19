// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* loadbof.c
*

  This module loads the compiled kod (.bof files) into memory, as memory
  mapped files.  A linked list of structures with the stuff about the
  file is maintained.  When each .bof file is loaded, the classes and
  message handlers are created by class.c and message.c.  The format of
  the .bof files is in bof.txt.
  
*/

#include "blakserv.h"

#define BOF_SPEC "*.bof"

static unsigned char magic_num[] = { 'B', 'O', 'F', 0xFF };
#define BOF_MAGIC_LEN sizeof(magic_num)


/* variables */
loaded_bof_node *mem_files;

/* local function prototypes */
Bool LoadBofName(char *fname);
void AddFileMem(char *fname,char *ptr,int size);
void FindClasses(char *fmem,char *fname);
void FindMessages(char *fmem,int class_id,bof_dispatch *dispatch);

void InitLoadBof(void)
{
	mem_files = NULL;
}

void LoadBof(void)
{
	char file_load_path[MAX_PATH+FILENAME_MAX];
	char file_copy_path[MAX_PATH+FILENAME_MAX];
	
	int files_loaded = 0;
	
	sprintf(file_load_path,"%s%s",ConfigStr(PATH_BOF),BOF_SPEC);
   StringVector files;
   if (FindMatchingFiles(file_load_path, &files))
   {
      for (StringVector::iterator it = files.begin(); it != files.end(); ++it)
      {
			sprintf(file_load_path,"%s%s",ConfigStr(PATH_BOF), it->c_str());
			sprintf(file_copy_path,"%s%s",ConfigStr(PATH_MEMMAP), it->c_str());
			if (BlakMoveFile(file_load_path,file_copy_path))
				files_loaded++;
      }
   }
	
	/*
	if (!files.empty())
	dprintf("LoadBof moved in %i of %i found new .bof files\n",files_loaded,files.size());
	*/

	//dprintf("starting to load bof files\n");
	files_loaded = 0;
	
	sprintf(file_load_path,"%s%s",ConfigStr(PATH_MEMMAP),BOF_SPEC);
   if (FindMatchingFiles(file_load_path, &files))
   {
      for (StringVector::iterator it = files.begin(); it != files.end(); ++it)
      {
			sprintf(file_load_path,"%s%s",ConfigStr(PATH_MEMMAP), it->c_str());
			
			if (LoadBofName(file_load_path))
				files_loaded++;
			else
				eprintf("LoadAllBofs can't load %s\n", it->c_str());
		}
	}
	
	SetClassesSuperPtr();
	SetClassVariables();
	SetMessagesPropagate();

	//dprintf("LoadBof loaded %i of %i found .bof files\n",files_loaded,files.size());
}

void ResetLoadBof(void)
{ 
	loaded_bof_node *lf,*temp;
	
	lf = mem_files;
	while (lf != NULL)
	{
		temp = lf->next;
		
		FreeMemory(MALLOC_ID_LOADBOF,lf->mem,lf->length);
		
		FreeMemory(MALLOC_ID_LOADBOF,lf,sizeof(loaded_bof_node));
		lf = temp;
	}
	mem_files = NULL;
}

Bool LoadBofName(char *fname)
{
   FILE *f = fopen(fname, "rb");
	if (f == NULL)
   {
      eprintf("LoadBofName can't open %s\n", fname);
		return False;
   }

   for (int i = 0; i < BOF_MAGIC_LEN; ++i)
   {
      unsigned char c;
      if (fread(&c, 1, 1, f) != 1 || c != magic_num[i])
      {
         eprintf("LoadBofName %s is not in BOF format\n", fname);
         fclose(f);
         return False;
      }
   }
   
   int version;
   if (fread(&version, 1, 4, f) != 4 || version != BOF_VERSION)
	{
		eprintf("LoadBofName %s can't understand bof version %i\n",
			fname, version);
		fclose(f);
		return False;
	}
   
   // Go back to start of file and read the whole thing into memory.
   fseek(f, 0, SEEK_SET);
   
   struct stat st;
   stat(fname, &st);
   int file_size = st.st_size;

	char *ptr = (char *)AllocateMemory(MALLOC_ID_LOADBOF,file_size);
   if (fread(ptr, 1, file_size, f) != file_size)
   {
      fclose(f);
      return False;
   }

   fclose(f);

	AddFileMem(fname,ptr,file_size);
	
	return True;
}

/* add a filename and mapped ptr to the list of loaded files */
void AddFileMem(char *fname,char *ptr,int size)
{
	loaded_bof_node *lf;
	
	/* make new loaded_file node */
	lf = (loaded_bof_node *)AllocateMemory(MALLOC_ID_LOADBOF,sizeof(loaded_bof_node));
	strcpy(lf->fname,fname);
	lf->mem = ptr;
	lf->length = size;
	
	/* we store the fname so the class structures can point to it, but kill the path */
	
	if (strrchr(lf->fname,'\\') == NULL)
		FindClasses(lf->mem,lf->fname); 
	else
		FindClasses(lf->mem,strrchr(lf->fname,'\\')+1); 
	
	/* add to front of list */
	lf->next = mem_files;
	mem_files = lf;
}

/* add the classes of a mapped file to a class list */
void FindClasses(char *fmem,char *fname)
{
	int dstring_offset;
	int classes_in_file,class_id,class_offset;
	bof_class_header *class_data;
	bof_dispatch *dispatch_section;
	bof_dstring *dstring_data;
	int line_table_offset;
	bof_line_table *line_table;
	bof_class_props *props;
   bof_list_elem *classes;
	int i;
	
	dstring_offset = ((bof_file_header *)fmem)->dstring_offset;
	dstring_data = (bof_dstring *)(fmem + dstring_offset);
	line_table_offset = ((bof_file_header *)fmem)->line_table_offset;
	if (line_table_offset == 0) /* means no line number table */
		line_table = NULL;
	else
		line_table = (bof_line_table *)(fmem + line_table_offset);
	
	classes_in_file = ((bof_file_header *)fmem)->num_classes;
   
   classes = &((bof_file_header *)fmem)->classes;
	for (i=0;i<classes_in_file;i++)
	{
		class_id     = classes[i].id;
		class_offset = classes[i].offset;
		class_data = (bof_class_header *)(fmem + class_offset);
		
		props = (bof_class_props *)(fmem + class_data->offset_properties);
		
		dispatch_section = (bof_dispatch *)(fmem + class_data->offset_dispatch);
		AddClass(class_id,class_data,fname,fmem,dstring_data,line_table,props);
		SetClassNumMessages(class_id,dispatch_section->num_messages);
		FindMessages(fmem,class_id,dispatch_section);
	}
}

void FindMessages(char *fmem,int class_id,bof_dispatch *dispatch)
{
	int i;
   bof_dispatch_list_elem *messages;

   messages = &dispatch->messages;
   
	for (i=0;i<dispatch->num_messages;i++)
		AddMessage(class_id,i,messages[i].id,
                 (char *)(fmem + messages[i].offset),
                 messages[i].dstr_id);
}
