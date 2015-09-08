// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* Blakston .bof (blakod object format) dump utility */

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include "bkod.h"

#define MAX_CLASSES 100
#define MAX_HANDLERS 500

/* function prototypes */
BOOL memmap_file(char *s);
void release_file();
unsigned char get_byte();
unsigned int get_int();
void dump_bof();
void dump_bkod();
void dump_unary_assign(opcode_type opcode,char *text);
void dump_binary_assign(opcode_type opcode,char *text);
void dump_goto(opcode_type opcode,char *text);
void dump_call(opcode_type opcode,char *text);
void dump_return(opcode_type return_op,char *text);
void dump_debug_line(opcode_type opcode,char *text);
char * name_unary_operation(int unary_op);
char * name_binary_operation(int binary_op);
char * name_var_type(int parm_type);
char * name_goto_cond(int cond);
char * name_function(int fnum);
void print_hex_byte(unsigned char ch);
char *str_constant(int num);
int  find_linenum(int offset);

/* global variables */
HANDLE fh;                      /* handle to the open file */
int file_size;                  /* length of th file */
HANDLE mapfh;                   /* handle to the file mapping */
char *file_mem;                 /* ptr to the memory mapped file */
int index;                      /* current location in file */
int dump_hex = 0;               /* dump the raw hex */
int inst_start;                 /* start of current instruction for dump_xxx to use */

unsigned int debug_offset;      /* start of debugging info in file */

static BYTE bof_magic[] = { 0x42, 0x4F, 0x46, 0xFF };

int main(int argc,char *argv[])
{
   if (argc != 2)
   {
      fprintf(stderr,"Usage:  blakdeco filename.bof\n");
      exit(1);
   }
   
   if (memmap_file(argv[1]))
   {
      dump_bof();
      release_file();
   }
}

BOOL memmap_file(char *s)
{
   fh = CreateFile(s,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
   if (fh == INVALID_HANDLE_VALUE)
   {
      fprintf(stderr,"Can't open file %s\n",s);
      return FALSE;
   }

   file_size = GetFileSize(fh,NULL);
   
   mapfh = CreateFileMapping(fh,NULL,PAGE_READONLY,0,file_size,NULL);
   if (mapfh == NULL)
   {
      fprintf(stderr,"Can't create a file mapping\n");
      CloseHandle(fh);
      return FALSE;
   }

   file_mem = (char *) MapViewOfFile(mapfh,FILE_MAP_READ,0,0,0);
   if (file_mem == NULL)
   {
      fprintf(stderr,"Can't map a view of file\n");
      CloseHandle(mapfh);
      CloseHandle(fh);
      return FALSE;
   }
   return TRUE;
}

void release_file()
{
   UnmapViewOfFile(file_mem);
   CloseHandle(mapfh);
   CloseHandle(fh);
}  

unsigned char get_byte()
{
   if (index >= file_size)
   {
      fprintf(stderr,"Read past end of file.  Die.\n");
      release_file();
      exit(1);
   }

   return file_mem[index++];
}

unsigned int get_int()
{
   if (index+3 >= file_size)
   {
      fprintf(stderr,"Read past end of file on int read.  Die.\n");
      release_file();
      exit(1);
   }

   index += 4;
   return *((unsigned int *)(file_mem + index-4));
}

void get_string(char *str, int max_chars)
{
   int i;

   for (i=0; i < max_chars; i++)
   {
      if (index > file_size)
      {
	 fprintf(stderr,"Read past end of file on string read.  Die.\n");
	 release_file();
	 exit(1);
      }

      /* Copy byte of string and look for null termination */
      str[i] = *(file_mem + index);
      index++;
      if (str[i] == 0)
	 return;
   }
}

void dump_bof()
{
   unsigned int c,i,j,superclass_id;
   unsigned int num_properties, prop_num,prop_defaults;
   unsigned int num_cvars, cvar_num,cvar_defaults;
   unsigned int handler[MAX_HANDLERS],num_messages;
   unsigned int classes[MAX_CLASSES],num_classes;
   unsigned int strtable_offset, fname_offset;

   unsigned int num_strings;

   int classes_id[MAX_CLASSES];

   char *default_str, str[500];

   index = 0;

   for (i=0; i < 4; i++)
   {
      BYTE b = get_byte();
      if (b != bof_magic[i])
	 printf("Bad magic number--this is not a BOF file\n");
   }

   printf(".bof version: %i\n",get_int());

   fname_offset = get_int();
   if (fname_offset >= (unsigned int) file_size)
   {
      fprintf(stderr,"Read past end of file on kod filename.  Die.\n");
      release_file();
      exit(1);
   }
   printf("Source file = %s\n", file_mem + fname_offset);

   strtable_offset = get_int();
   printf("String table at offset %08X\n", strtable_offset);

   debug_offset = get_int();
   if (debug_offset == 0)
      printf("No line number debugging information\n");
   else printf("Debugging information at offset %08X\n", debug_offset);

   num_classes = get_int();
   printf("Classes: %i\n",num_classes);

   if (num_classes > MAX_CLASSES-1)
   {
      printf("Can only handle %i classes\n",MAX_CLASSES-1);
      num_classes = MAX_CLASSES-1;
   }
   for (i=0;i<num_classes;i++)
   {
      classes_id[i] = get_int();
      classes[i] = get_int();
      printf("class id %i at offset %08X\n",classes_id[i],classes[i]);
   }
   classes[i] = -1;
  
   for (c=0;c<num_classes;c++)
   {
      printf("Class id %i:\n",classes_id[c]);
      superclass_id = get_int();
      printf("Superclass %i\n",superclass_id);
      
      printf("Property table offset: %08X\n", get_int());
      printf("Message handler offset: %08X\n", get_int());
      
      num_cvars = get_int();
      printf("Classvars: %i\n", num_cvars);
      
      cvar_defaults = get_int();
      printf("Classvar default values: %i\n",cvar_defaults);
      for (i=0;i<cvar_defaults;i++)
      {
	 cvar_num = get_int();
	 default_str = strdup(str_constant(get_int()));
	 printf("  classvar %2i init value: %s\n",cvar_num,default_str);
      }

      num_properties = get_int();
      printf("Properties: %i\n",num_properties);
      
      prop_defaults = get_int();
      printf("Property default values: %i\n",prop_defaults);
      for (i=0;i<prop_defaults;i++)
      {
	 prop_num = get_int();
	 default_str = strdup(str_constant(get_int()));
	 printf("  property %2i init value: %s\n",prop_num,default_str);
      }
      
      num_messages = get_int();
      printf("Message handlers: %i\n",num_messages);
      if (num_messages > MAX_HANDLERS-1)
      {
	 printf("Can only handle %i handlers\n",MAX_HANDLERS-1);
	 num_messages = MAX_HANDLERS-1;
      }

      for (i=0;i<num_messages;i++)
      {
	 int id, comment;
	 
	 id = get_int();
	 handler[i] = get_int();
	 comment = get_int();
	 
	 printf(" message %5i at offset %08X\n",id,handler[i]);
	 if (comment != -1)
	    printf("  Comment string #%5i\n", comment);

      }
      handler[i] = -1;
      
      printf("BKOD data:\n");
      for (i=0;i<num_messages;i++)
      {
         unsigned char locals,num_parms;
         int parm_id,parm_default;
	 
	 locals = get_byte();
	 num_parms = get_byte();
	 
	 printf("\nMessage handler, %i local vars\n",locals);
	 
	 for (j=0;j<(unsigned int) num_parms;j++)
	 {
	    parm_id = get_int();
	    parm_default = get_int();
	    default_str = strdup(str_constant(parm_default));
	    printf("  parm id %i = %s\n",parm_id,default_str);
	 }
	 
   
	 // printf("at ofs %08X, next handler %08X, next class %08X\n",
    // index,handler[i+1],classes[c+1]); 
	 while ((unsigned int) index <  handler[i+1] &&
           (unsigned int) index < classes[c+1] &&
           index < file_size)
	 {
	    dump_bkod();
	    if (index == strtable_offset)
	       break;
	 }
      }
      printf("--------------------------------------------\n");
      num_strings = get_int();
      printf("Strings: %d\n", num_strings);
      for (i=0; i < num_strings; i++)
         printf("String %d at offset %08X\n", i, get_int());
      
      for (i=0; i < num_strings; i++)
      {
         get_string(str, 500);
         printf("String %d = %s\n", i, str);
      }
      
      printf("\n");
   }
}

void dump_bkod()
{
   opcode_type opcode;
   int i, line;
   char text[50000];
   char opcode_char;
   static int last_line = 0;

//   printf("at %08X\n",index);
   
   inst_start = index;

   line = find_linenum(index);
   if (line != last_line)
   {
      printf("*** Line %d\n", line);
      last_line = line;
   }
   
   opcode_char = get_byte();

   memcpy(&opcode,&opcode_char,1);

/*   opcode = (opcode_type)get_byte();  can't get to compile */

   switch (opcode.command)
   {
   case UNARY_ASSIGN : dump_unary_assign(opcode,text); break;
   case BINARY_ASSIGN : dump_binary_assign(opcode,text); break;
   case GOTO : dump_goto(opcode,text); break;
   case CALL : dump_call(opcode,text); break;
   case RETURN : dump_return(opcode,text); break;
   case DEBUG_LINE : dump_debug_line(opcode,text); break;
   default : sprintf(text,"INVALID"); break;
   }

   if (dump_hex)
   {
      printf("BKOD raw: ");
      for (i=0;i<index-inst_start;i++)
	 print_hex_byte(file_mem[inst_start+i]);
      printf("\n");
   }
   printf("@%08X: ",inst_start);
   printf("%s\n",text);
}

void dump_unary_assign(opcode_type opcode,char *text)
{
   char info,*source_str;
   int dest,source;

/*   printf("at %08X in unary assign\n",index); */

   info = get_byte();

   dest = get_int();
   source = get_int();
   source_str = _strdup(str_constant(source));
   sprintf(text,"%s %i = %s %s %s",name_var_type(opcode.dest),dest,
	   name_unary_operation(info),name_var_type(opcode.source1),
	   source_str);
}

void dump_binary_assign(opcode_type opcode,char *text)
{
   char info,*source1_str,*source2_str;
   int dest,source1,source2;

   info = get_byte();
   dest = get_int();
   source1 = get_int();
   source2 = get_int();
   source1_str = _strdup(str_constant(source1));
   source2_str = _strdup(str_constant(source2));
   sprintf(text,"%s %i = %s %s %s %s %s",name_var_type(opcode.dest),dest,
	   name_var_type(opcode.source1),source1_str,name_binary_operation(info),
	   name_var_type(opcode.source2),source2_str);
}


void dump_goto(opcode_type opcode,char *text)
{
   int dest_addr,var;

   dest_addr = get_int();

   if (opcode.source1 == 0 && opcode.source2 == GOTO_UNCONDITIONAL)
   {
      sprintf(text,"Goto absolute %08X",dest_addr+inst_start);
      return;
   }
   else
   {
      var = get_int();
      sprintf(text,"If %s %s %s goto absolute %08X",
	      name_var_type(opcode.source1),str_constant(var),
	      name_goto_cond(opcode.dest),dest_addr+inst_start);
   }
}

void dump_call(opcode_type opcode,char *text)
{
   unsigned char info,num_parms,parm_type;    
   int i,parm_value,assign_index, name_id;
   char tempbuf[15];
   
   info = get_byte();
   
   switch(opcode.source1)
   {
   case CALL_NO_ASSIGN : /* return value ignored */
   {
      sprintf(text,"Call %s--",name_function(info));
      break;
   }
   case CALL_ASSIGN_LOCAL_VAR :
   case CALL_ASSIGN_PROPERTY :
   {
      assign_index = get_int();
      sprintf(text,"%s %s = Call %s\n  : ",name_var_type(opcode.source1),
	      str_constant(assign_index),name_function(info));
      break;
   }
   default :
   {
      sprintf(text,"INVALID");
      return;
   }
   }

   num_parms = get_byte();
   for (i=0;i<num_parms;i++)
   {
      parm_type = get_byte();
      parm_value = get_int();
      if (i != 0)
	 strcat(text,", ");
      strcat(text,name_var_type(parm_type));
      strcat(text," ");
      strcat(text,str_constant(parm_value));
   }

   strcat(text,"\n  : ");
   
   num_parms = get_byte();
   for (i=0;i<num_parms;i++)
   {
      name_id = get_int();
      parm_type = get_byte();
      parm_value = get_int();
      if (i != 0)
	 strcat(text,", ");
      strcat(text,"Parm id ");
      strcat(text,itoa(name_id,tempbuf,10));
      strcat(text," ");
      strcat(text,name_var_type(parm_type));
      strcat(text," ");
      strcat(text,str_constant(parm_value));
   }
}

void dump_return(opcode_type return_op,char *text)
{
   int ret_val;

   switch (return_op.dest)
   {
   case PROPAGATE :
   {
      sprintf(text,"Return propagate"); 
      break;
   }
   case NO_PROPAGATE :
   {
      ret_val = get_int();
      sprintf(text,"Return %s %s",name_var_type(return_op.source1),
	      str_constant(ret_val));
      break;
   }
   }
}

char * name_unary_operation(int unary_op)
{
   switch (unary_op)
   {
      case NOT : return "NOT";
      case NEGATE : return "-";
      case NONE : return "";
      case BITWISE_NOT : return "~";
      case PRE_INCREMENT :
      case POST_INCREMENT : return "++";
      case PRE_DECREMENT :
      case POST_DECREMENT : return "--";
      
      default : return "INVALID";
   }
}

char * name_binary_operation(int binary_op)
{
   switch (binary_op)
   {
   case ADD : return "+";
   case SUBTRACT : return "-";
   case MULTIPLY : return "*";
   case DIV : return "/";
   case MOD : return "%";
   case AND : return "and";
   case OR : return "or";
   case EQUAL : return "=";
   case NOT_EQUAL : return "!=";
   case LESS_THAN : return "<";
   case GREATER_THAN : return ">";
   case LESS_EQUAL : return "<=";
   case GREATER_EQUAL : return ">=";
   case BITWISE_AND : return "&";
   case BITWISE_OR : return "|";
   default : return "INVALID";
   }
}

char * name_var_type(int parm_type)
{
   switch (parm_type)
   {
      case LOCAL_VAR : return "Local var";
      case PROPERTY : return "Property";
      case CONSTANT : return "Constant";
      case CLASS_VAR : return "Class var";
      default : return "INVALID";
   }
}

char * name_goto_cond(int cond)
{
   switch (cond)
   {
   case GOTO_IF_TRUE : return "!= 0";
   case GOTO_IF_FALSE : return "= 0";
   default : return "INVALID";
   }
}

char * name_function(int fnum)
{
   static char s[25];

   switch (fnum)
   {
   case CREATEOBJECT : return "Create";
   //   case DELETEOBJECT : return "Delete";
   case ISCLASS : return "IsClass";
   case GETCLASS : return "GetClass";

   case SENDMESSAGE : return "Send";
   case POSTMESSAGE : return "Post";

   case SAVEGAME : return "SaveGame";
   case LOADGAME : return "LoadGame";

   case GODLOG : return "GodLog";
   case DEBUG : return "Debug";  
   case ADDPACKET : return "AddPacket";
   case SENDPACKET : return "SendPacket";
   case SENDCOPYPACKET : return "SendCopyPacket";
   case CLEARPACKET : return "ClearPacket";
   case GETINACTIVETIME : return "GetInactiveTime";
   case DUMPSTACK : return "DumpStack";

   case ISSTRING : return "IsString";
   case STRINGEQUAL : return "StringEqual";
   case STRINGCONTAIN : return "StringContain";
   case SETRESOURCE : return "SetResource";
   case PARSESTRING : return "ParseString";
   case SETSTRING : return "SetString";
   case CREATESTRING : return "CreateString";
   case STRINGSUBSTITUTE : return "StringSubstitute";
   case APPENDTEMPSTRING : return "AppendTempString";
   case CLEARTEMPSTRING : return "ClearTempString";
   case GETTEMPSTRING : return "GetTempString";
   case STRINGLENGTH : return "StringLength";
   case STRINGCONSISTSOF : return "StringConsistsOf";

   case SETCLASSVAR : return "SetClassVar";

   case CREATETIMER : return "CreateTimer";
   case DELETETIMER : return "DeleteTimer";
   case GETTIMEREMAINING : return "GetTimeRemaining";
   case ISTIMER : return "IsTimer";

   case MOVESECTORBSP : return "MoveSectorBSP";
   case CHANGETEXTUREBSP : return "ChangeTextureBSP";
   case CREATEROOMDATA : return "LoadRoom";
   case FREEROOM : return "FreeRoom";
   case ROOMDATA : return "RoomData";
   case CANMOVEINROOM : return "CanMoveInRoom";
   case CANMOVEINROOMFINE : return "CanMoveInRoomFine";
   case CANMOVEINROOMHIGHRES : return "CanMoveInRoomHighRes";
   case GETHEIGHT : return "GetHeight";
   case GETHEIGHTFLOORBSP : return "GetHeightFloorBSP";
   case GETHEIGHTCEILINGBSP : return "GetHeightCeilingBSP";
   case LINEOFSIGHTBSP : return "LineOfSightBSP";

   case MINIGAMENUMBERTOSTRING : return "MiniGameNumberToString";
   case MINIGAMESTRINGTONUMBER : return "MiniGameStringToNumber";

   case APPENDLISTELEM : return "AppendListElem";
   case CONS  : return "Cons";
   case FIRST  : return "First";
   case REST  : return "Rest";
   case LENGTH  : return "Length";
   case ISLISTMATCH : return "IsListMatch";
   case NTH  : return "Nth";
   case MLIST  : return "List";
   case ISLIST : return "IsList";
   case SETFIRST : return "SetFirst";
   case SETNTH : return "SetNth";
   case DELLISTELEM : return "DelListElem";
   case FINDLISTELEM : return "FindListElem";
   case SWAPLISTELEM : return "SwapListElem";
   case INSERTLISTELEM : return "InsertListElem";
   case LAST : return "Last";

   case GETTIMEZONEOFFSET : return "GetTimeZoneOffset";
   case GETTIME : return "GetTime";
   case GETTICKCOUNT : return "GetTickCount";

   case ABS : return "Abs";
   case BOUND : return "Bound";
   case SQRT : return "Sqrt";

   case CREATETABLE : return "CreateTable";
   case ADDTABLEENTRY : return "AddTableEntry";
   case GETTABLEENTRY : return "GetTableEntry";
   case DELETETABLEENTRY : return "DeleteTableEntry";
   case DELETETABLE : return "DeleteTable";
   case ISTABLE : return "IsTable";

   case RECYCLEUSER : return "RecycleUser";

   case ISOBJECT : return "IsObject";

   case RANDOM  : return "Random";
   case RECORDSTAT : return "RecordStat";
   case GETSESSIONIP : return "GetSessionIP";
   default : sprintf(s,"Unknown function %i",fnum); return s;
   }

   /* can't get here */
   return NULL;
}

void print_hex_byte(unsigned char ch)
{
   int b1,b2;

   b1 = ch / 16;
   b2 = ch & (16-1);
   if (b1 >= 10)
      printf("%c",b1-10+'A');
   else
      printf("%i",b1);

   if (b2 >= 10)
      printf("%c",b2-10+'A');
   else
      printf("%i",b2);
}

char *str_constant(int num)
{
   static char s[20];
   constant_type bc;

   memcpy(&bc,&num,4);
   switch (bc.tag)
   {
   case TAG_NIL : 
      if (bc.data == 0) 
	 sprintf(s,"$");
      else
	 sprintf(s,"%i",bc.data);
      break;
   case TAG_INT :
      sprintf(s,"(int %i)",bc.data);
      break;
   case TAG_RESOURCE :
      sprintf(s,"(rsc %i)",bc.data);
      break;
   case TAG_CLASS :
      sprintf(s,"(class %i)",bc.data);
      break;
   case TAG_MESSAGE :
      sprintf(s,"(message %i)",bc.data);
      break;
   case TAG_DEBUGSTR :
      sprintf(s,"(debugging string %i)",bc.data);
      break;
   case TAG_OVERRIDE :
      sprintf(s,"(overridden by property %i)",bc.data);
      break;
   default :
      sprintf(s,"INVALID");
      break;
   }
   return s;
}

void dump_debug_line(opcode_type opcode,char *text)
{
   int line;

   line = get_int();
   sprintf(text,"LINE %i",line);
}

int find_linenum(int offset)
{
   char *pos;
   int line_num, line_offset;

   pos = file_mem + debug_offset + 4;  // +4 to skip # of debug entries

   while (1)
   {
      if ((pos - file_mem) + 7 >= file_size)
      {
	 printf("Read past end of file looking for debug position %d", offset);
	 exit(1);
      }
      memcpy(&line_offset, pos, 4);
      memcpy(&line_num, pos + 4, 4);
      pos += 8;
      if (offset <= line_offset)
	 return line_num;
   }
}
