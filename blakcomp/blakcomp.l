	/* Lex file for Blakod compiler */

%{
#include "blakcomp.h"
#include "blakcomp.tab.h"

/* Parser states */
enum {
   STATE_CODE, STATE_INITIAL,
};

int cur_state = STATE_INITIAL;

#ifdef __cplusplus
extern "C" {
#endif
int yywrap(void);
#ifdef __cplusplus
}
#endif

extern int yyparse(void);
static FILE *open_file(char *fname);
static void usage(void);

int lineno;
Bool generate_code = True;  /* True if we should generate code */
extern int yydebug;        /* Set to nonzero to get yacc debug info */
list_type file_list;       /* List of files to compile */
char *current_fname;
char *bof_fname;       /* Object code filename */
int debug_bof;         /* Should we put debugging info in .bof file? */

struct file_state {
   YY_BUFFER_STATE buffer;
   char *filename;
   int lineno;
   FILE *file_ptr;
};

#define MAX_INCLUDE_DEPTH 10
#define MAX_INCLUDE_DIRS  20

struct file_state include_stack[MAX_INCLUDE_DEPTH + 1];  /* stack of nested included files */
int include_depth;                                       /* nesting depth of include files */

static char *include_dirs[MAX_INCLUDE_DIRS];/* include directory names */             
static int num_include_dirs;               /* # of include directories specified on command line */
%}

	/* A single line of a multiple-line string constant, minus newline & initial quote */
strline  (([^\"\n\\]|\\.)*\")[ \t\r]*

     /* String mode is for inside strings.  Code mode is for when we reach the messages block */
%s STRING CODE

%%

	/* strings -- allow backslash to introduce escaped characters or extend line */

<STRING>(([^\"\n\\]|\\.)*$)	{ yyerror("Unterminated string"); 
			  yylval.string_val = assemble_string(yytext);
			  if (cur_state == STATE_INITIAL)
			     BEGIN INITIAL;
			  else BEGIN CODE;
			  return STRING_CONSTANT; 
			}

   /* This nastiness allows multiple-line C-like strings to be interpreted as a single string */
<STRING>{strline}(\n[ \t\r]*\"{strline})*   {
			  yylval.string_val = assemble_string(yytext);
			  if (cur_state == STATE_INITIAL)
			     BEGIN INITIAL;
			  else BEGIN CODE;
			  return STRING_CONSTANT; 
			}			

\"			{ BEGIN STRING; }


	/* reserved words */
and		{ return AND; }
break		{ return BREAK; }
classvars	{ return CLASSVARS; }
continue	{ return CONTINUE; }
constants	{ return CONSTANTS; }
else            { return ELSE;}
end		{ BEGIN INITIAL; cur_state = STATE_INITIAL; return END; } 
for		{ return FOR; }
if		{ return IF; }
in		{ return IN; }
is		{ return IS; }
local		{ return LOCAL; }
messages	{ BEGIN CODE; cur_state = STATE_CODE; return MESSAGES; }
mod             { return MOD; }
not		{ return NOT; }
or		{ return OR; }
propagate	{ return PROPAGATE; }
properties	{ return PROPERTIES; }
resources	{ return RESOURCES; }
return	        { return RETURN; }
while		{ return WHILE; }
include         { return INCLUDE; }

	/* punctuation */
"<>"	{ yylval.int_val = NEQ_OP; return REL_OP; }
"<"	{ yylval.int_val = LT_OP; return REL_OP; }
">"	{ yylval.int_val = GT_OP; return REL_OP; }
"<="	{ yylval.int_val = LEQ_OP; return REL_OP; }
">="	{ yylval.int_val = GEQ_OP; return REL_OP; }


[-+*/,:#&@()={}\$\[\]\|~]	{ return yytext[0]; }	/* return character itself */
\;                              { return SEP; }  /* Separator */

	/* names */

[A-Za-z][A-Za-z0-9_]*	{ yylval.idname = strdup(yytext); return IDENTIFIER; }
[A-Za-z0-9_\-]*\.[A-Za-z0-9_\-]*	{ yylval.idname = strdup(yytext); return FILENAME; }

	/* numbers */

[0-9]+			{ yylval.int_val = atoi(yytext); return NUMBER; }
0x[0-9a-fA-F]+		{ sscanf(yytext, "%x", &yylval.int_val); return NUMBER; }

	/* comments */
^[ \t\r]*"\%".*\n	lineno++;	/* Ignore comment-only lines completely */
"\%".*			;		/* Otherwise just delete comment part of line */

  	/* line continuation */
\\[ \t\r]*\n		lineno++;	/* Allow multiple-line statements */

	/* whitespace */
^[ \t\r]*\n	lineno++;	/* Skip completely blank lines */
[ \t\r]+	;
<CODE>\n    lineno++;       /* When parsing messages, throw out newlines */
<INITIAL>\n	{ lineno++; return EOL; } /* Parser needs to know about other newlines */

	/* anything else */
.	yyerror("Invalid character");

%%

/************************************************************************/
void handle_error(void)
{
   static int numerrors = 0;
   generate_code = False; /* Errors; don't generate code */
   if (++numerrors > MAXERRORS)
   {
      fprintf(stderr, "Too many errors. Aborting.\n");
      exit(1);
   }
}
/************************************************************************/
/* yyerror should be used for lexing & parsing errors. */
void yyerror(const char *s)
{
   /* If token is EOL, give useful error message */
   if (!strcmp(yytext, "\n"))
      fprintf(stderr, "%s(%d): error: Premature end of line\n", 
							 current_fname, lineno-1);
   else
      fprintf(stderr, "%s(%d): error: %s on token %s\n", 
							 current_fname, lineno, s, yytext);
   handle_error();
}
/************************************************************************/
/* action_error should be used for semantic errors. */
void action_error(const char *fmt, ...)
{
   int errline = lineno;
   va_list marker;

   /* If last token was EOL, report error on previous line */
   if (!strcmp(yytext, "\n"))
      errline--;

   fprintf(stderr, "%s(%d): error: ", current_fname, errline);  

   va_start(marker, fmt);
   vfprintf(stderr, fmt, marker);
   va_end(marker);
   fprintf(stderr, "\n");

   handle_error();
}
/************************************************************************/
/* action_warning should be used for semantic warnings. */
void action_warning(const char *fmt, ...)
{
   int errline = lineno;
   va_list marker;

   /* If last token was EOL, report error on previous line */
   if (!strcmp(yytext, "\n"))
      errline--;

   fprintf(stderr, "%s(%d): warning: ", current_fname, errline);  

   va_start(marker, fmt);
   vfprintf(stderr, fmt, marker);
   va_end(marker);
   fprintf(stderr, "\n");
}
/************************************************************************/
/* simple_error should be used for other errors. */
void simple_error(const char *fmt, ...)
{
   va_list marker;

   fprintf(stderr, "  error: ");
   va_start(marker, fmt);
   vfprintf(stderr, fmt, marker);
   va_end(marker);
   fprintf(stderr, "\n");

   handle_error();   
}
/************************************************************************/
/* simple_warning should be used for all warnings. */
void simple_warning(const char *fmt, ...)
{
   va_list marker;

   printf("  warning: ");
   va_start(marker, fmt);
   vprintf(fmt, marker);
   va_end(marker);
   printf("\n");
}
/************************************************************************/
/*
 * assemble_string:  Convert a Blakod multiple-line string into a
 *   plain string, without starting and ending quotes, and with escape
 *   sequences replaced.
 */
char *assemble_string(char *str)
{
   char *new_str = strdup(str);
   char *rd, *wt;
   int inside_quotes = 1;

   rd = new_str;
   wt = new_str;

   while (*rd != '\0')
   {
      /* Throw out junk outside quotes */
      if (!inside_quotes && *rd != '"')
      {
	 if (*rd == '\n')
	    lineno++;
	 rd++;
	 continue;
      }
	 
      switch (*rd)
      {
      case '\\':
	 rd++;
	 
	 /* Look for special characters */
	 switch (*rd)
	 {
	 case 'n':
	    *wt = '\r';    /* Ugh.. CR-LF pair for Windows ugliness */
	    wt++;
	    *wt = '\n';
	    wt++;
	    break;

	 default:
	    *wt = *rd;
	    wt++;
	    break;
	    
	 }
	 break;

      case '"':  /* Ignore quotes */
	 inside_quotes = 1 - inside_quotes;
	 break;

      case '\n': /* Ignore newlines */
	 lineno++;
	 break;

      default:
	 *wt = *rd;
	 wt++;
	 break;
      }

      rd++;
   }
   *wt = '\0';
   return new_str;
}
/************************************************************************/
/* 
 * recompile_warnings:  Give warning messages for each class that should
 *    be recompiled because one of its ancestors was recompiled.
 */
void recompile_warnings(list_type l)
{
   int column = 0;

   if (l == NULL)
      return;

   simple_warning("Need to recompile these subclasses:");

   for (; l != NULL; l = l->next)
   {
      const char *name = ((recompile_type) (l->data))->class_id->name;
      
      if (column + strlen(name) >= 75)
      {
         printf("\n");
         column = 0;
      }
      printf("%s  ", name);
      column += (int) strlen(name) + 2;
      
      /* ((recompile_type) (l->data))->superclass->name); */
   }
   printf("\n");
}
/************************************************************************/
int yywrap(void)
{
   fclose(include_stack[include_depth].file_ptr);

   yy_delete_buffer(YY_CURRENT_BUFFER);
   /* XXX Should close file; but this causes access violation, because
      FILE pointer is out of scope.  
    */

   /* If in an included file, move back to including file */
   if (include_depth > 0)
   {
      include_depth--;
      lineno = include_stack[include_depth].lineno;
      current_fname = include_stack[include_depth].filename;
      yy_switch_to_buffer(include_stack[include_depth].buffer);
      return 0;
   }

   return 1; /* Otherwise stop reading the current file */
}
/************************************************************************/
/*
 * open_file:  Open the given filename and return a FILE pointer to it.
 *    If the file has no extension, we add a ".kod" extension.
 *    Also set bof_fname to the corresponding ".bof" filename.
 */
FILE *open_file(char *fname)
{
   char temp[256];
   FILE *fin;

   /* Add .kod extension if necessary */
   set_extension(temp, fname, ".kod");
   
   if ((fin = fopen(temp, "r")) == NULL)
   {
      perror(fname);
      exit(1);
   }

   current_fname = strdup(temp);

   /* Construct object code filename */
   bof_fname = strdup(fname);
   set_extension(bof_fname, bof_fname, ".bof");

   return fin;
}
/************************************************************************/
/*
 * open_include_file:  Attempt to open given filename in all include
 *   directories.  Return file pointer, or NULL on failure.
 */
FILE *open_include_file(char *filename)
{
   FILE *fin;
   int i;
   char tempbuf[512];

   /* Loop through include directories and try to open file */
   for (i=0; i < num_include_dirs; i++)
   {
      sprintf(tempbuf, "%s/%s", include_dirs[i], filename);
      if ((fin = fopen(tempbuf, "r")) != NULL)
	 return fin;
   }

   /* Finally, just try current directory */
   fin = fopen(filename, "r");

   return fin;
}
/************************************************************************/
/*
 * include_file:  Switch flex over to the given file, parse it, and then
 *   return to the original file.  Include files may be nested.
 */
void include_file(char *filename)
{
   FILE *fin;

   if (include_depth >= MAX_INCLUDE_DEPTH)
   {
      action_error("Maximum nesting depth of include files is %d", MAX_INCLUDE_DEPTH);
      return;
   }

   if ((fin = open_include_file(filename)) == NULL)
   {
      action_error("Can't find include file %s", filename);
      return;
   }

   /* Save away current state, and move on to included file */

   include_stack[include_depth].buffer   = YY_CURRENT_BUFFER;
   include_stack[include_depth].filename = current_fname;
   include_stack[include_depth].lineno   = lineno;
   include_depth++;

   yy_switch_to_buffer(yy_create_buffer(fin, YY_BUF_SIZE));
   include_stack[include_depth].file_ptr = fin;
   current_fname = strdup(filename);
   lineno = 1;
}
/************************************************************************/
/*
 * add_include_directory:  Add given directory to list of directories
 *   that are searched for include files.  These directories can be
 *   specified on the command line.
 */
void add_include_directory(char *dirname)
{
   if (num_include_dirs == MAX_INCLUDE_DIRS)
      simple_error("Too many include directories!");
   else
   {
      include_dirs[num_include_dirs] = dirname;
      num_include_dirs++;
   }
}
/************************************************************************/
void usage(void)
{
   fprintf(stderr, "usage: bc <options> filename ...\n");
   fprintf(stderr, "     possible options are:\n");
   fprintf(stderr, "     -d            Put debugging info in .bof file\n");
   fprintf(stderr, "     -K file       Specify kodbase file\n");
   fprintf(stderr, "     -I dir        Add dir to include path\n");
}
/************************************************************************/
int main(int argc, char **argv)
{
   int i;
   list_type current_file_ptr;
   char *arg;

   if (argc == 1)
   {
      usage();
      exit(1);
   }

   file_list = NULL;
   debug_bof = False;

   num_include_dirs = 0;

   /* Parse command line */
   for (i = 1; i < argc; i++)
   {
      arg = argv[i];
      if (*arg == '-' || *arg == '/')
	 /* Switch on command-line options */
	 switch(toupper(*(arg + 1)))
	 {
	 case 'D':               /* Debugging on */
	    debug_bof = True;
	    break;
	    
	 case 'K' :              /* Specify kodbase filename */
	    if (i == argc - 1)
	       fprintf(stderr, "Switch -%c needs a filename (ignoring)\n",*(arg+1));
	    else
	    {
	       i++;
	       arg = argv[i];
	       set_kodbase_filename(arg);
	    }
	    break;
	    
	 case 'I':               /* Specify include file directory */
	    if (i == argc - 1)
		    fprintf(stderr, "Switch -%c needs a directory (ignoring)\n",*(arg+1));
	    else
	    {
	       i++;
	       arg = argv[i];
	       if (access(arg,4) == 0) /* 4 means read access */
		  add_include_directory(arg);
	       else
	       {
			 fprintf(stderr, "Unable to read include file directory %s\n",arg);
			 exit(1);
	       }
	    }
	    break;


	 default:
	    fprintf(stderr, "Ignoring unknown switch -%c.\n", *(arg + 1));
	    
	 }
      else file_list = list_add_item(file_list, argv[i]);
   }

   if (file_list == NULL)
   {
      fprintf(stderr, "No files specified!\n");
      exit(1);
   }

   current_file_ptr = file_list;

   /* Uncomment the next line to get parser trace */
   /* yydebug = 1; */

   initialize_parser();

   /* Read in database file */
   if (!load_kodbase())
      simple_error("Error loading database; continuing with compilation");
      

	bool all_success = true;
   for (current_file_ptr = file_list; current_file_ptr != NULL; 
        current_file_ptr = current_file_ptr->next)
   {
      // Reset per-file counts
      include_depth = 0;
      st.num_strings = 0;
      st.strings = NULL;
      
      include_stack[0].file_ptr = open_file( (char *) current_file_ptr->data);
      include_stack[0].buffer = yy_create_buffer(include_stack[0].file_ptr, 
                                                 YY_BUF_SIZE);
      include_stack[0].filename = current_fname;
      yy_switch_to_buffer(include_stack[0].buffer);

      lineno = 1;
      
      yyparse();

      if (generate_code) 
		   codegen(current_fname, bof_fname);
		else
		   all_success = false;
      generate_code = True;
   }

   /* Give warnings for classes that should be recompiled */
   recompile_warnings(st.recompile_list);

   return !all_success;
}
