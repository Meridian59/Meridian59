// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* header file for blakcomp.l and blakcomp.y */

#ifndef _BLAKCOMP_H
#define _BLAKCOMP_H

#ifdef BLAK_PLATFORM_WINDOWS
#include <io.h>
#endif

#ifdef BLAK_PLATFORM_LINUX
#include <unistd.h>
#define stricmp strcasecmp
#define O_BINARY 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include "util.h"
#include "table.h"

#define BOF_VERSION 6

#define IDBASE        10000      /* Lowest # of user-defined id.  Builtin ids have lower #s */
#define RESOURCEBASE  20000      /* Lowest # of user-defined resource. */

#define NO_SUPERCLASS 0  	/* Value to indicate that a class has no parent */

#define MAXERRORS 25  	  	/* Maximum # of errors before we give up */

#define MAXFNAME	32	/* Max length of function name */

#define MAXARGS         30      /* Maximum # of arguments to a function */

#define TABLESIZE       1023    /* Size of symbol tables */

#define MAX_LANGUAGE_ID 184

typedef int Bool;
enum {False = 0, True = 1};

// enum for built-in class IDs. These appear in blakserv.h also.
enum
{
   USER_CLASS = 0,
   SYSTEM_CLASS = 4,
   GUEST_CLASS = 20,
   ADMIN_CLASS = 23,
   DM_CLASS = 32,
   CREATOR_CLASS = 34,
   SETTINGS_CLASS = 35,
   REALTIME_CLASS = 36,
   EVENTENGINE_CLASS = 37,
   MAX_BUILTIN_CLASS = 37
};

enum { C_NUMBER, C_STRING, C_NIL, C_FNAME, C_RESOURCE, C_CLASS, C_MESSAGE, C_OVERRIDE }; 

/* Types of operators */
enum { AND_OP, OR_OP, PLUS_OP, MINUS_OP, MULT_OP, DIV_OP, MOD_OP, NOT_OP, NEG_OP,
       NEQ_OP, EQ_OP, LT_OP, GT_OP, LEQ_OP, GEQ_OP, BITAND_OP, BITOR_OP, BITNOT_OP,
       PRE_INC_OP, PRE_DEC_OP, POST_INC_OP, POST_DEC_OP };

typedef struct {
   int type;
   union {
      int       numval;
      char      *stringval;
      char      *fnameval;
      /* No value associated with NIL */
   } value;
} *const_type, const_struct;

/* Identifier types */
enum { I_UNDEFINED, I_RESOURCE, I_MESSAGE, I_CLASS, I_PARAMETER, I_LOCAL, 
       I_CLASSVAR, I_PROPERTY, I_FUNCTION, I_CONSTANT, I_MISSING };

/* id_struct.source types */
enum { DBASE = 200, COMPILE = 201 };

typedef struct {
   const char *name;
   int type;
   int idnum;
   int ownernum; /* Id # of thing that contains this identifier in its scope.  e.g. if
		    this id is of type message, then this is the class # */
   int source;   /* Whether this id came from the database file (source = DBASE)
		    or from a source code file (source = COMPILE) */
} *id_type, id_struct;

typedef struct {
   id_type id;
   const_type rhs;
} *classvar_type, classvar_struct;

typedef struct {
   id_type id;
   const_type rhs;
} *property_type, property_struct;

typedef struct {
   id_type lhs;
   const_type resource[MAX_LANGUAGE_ID];
} *resource_type, resource_struct;

enum { E_BINARY_OP, E_UNARY_OP, E_IDENTIFIER, E_CONSTANT, E_CALL, };
typedef struct _expr {
   int type;
   int lineno;   /* line number; used for putting debugging information in output */
   union {
      struct _binexpr{
         struct _expr *left_exp, *right_exp;
         int op;
      } binary_opval;
      
      struct _unexpr {
         struct _expr *exp;
         int op;
      } unary_opval;
      
      id_type idval;
      const_type constval;  
      void *callval;    /* Actually is of type stmt_type, but can't declare
                           it that way because stmt_struct includes expr_type */
   } value;
} *expr_type, expr_struct;

typedef struct {
   id_type   id;
   expr_type expr;
} *setting_type, setting_struct;

enum { ARG_EXPR = 1, ARG_SETTING };
typedef struct {
   int type;
   union {
      expr_type     expr_val;    /* Normal expression */
      setting_type  setting_val; /* "a=b" argument is a "setting" */
   } value;
} *arg_type, arg_struct;

typedef struct {
   id_type lhs;
   const_type rhs;
} *param_type, param_struct;

enum {S_IF = 1, S_ASSIGN, S_CALL, S_FOREACH, S_WHILE, S_PROP, S_RETURN,
      S_BREAK, S_CONTINUE, S_FOR, S_DOWHILE, S_CASE, S_DEFAULTCASE, S_SWITCH };

typedef struct {
   int function;    /* Opcode of function to call */
   list_type args;  /* Arguments */
} *call_stmt_type, call_stmt_struct;

typedef struct {
   expr_type condition;
   list_type then_clause;
   list_type else_clause;
   void *elseif_clause;
} *if_stmt_type, if_stmt_struct;

typedef struct {
   id_type lhs;
   expr_type rhs;
} *assign_stmt_type, assign_stmt_struct;

typedef struct {
   id_type id;
   expr_type condition;
   list_type body;
} *foreach_stmt_type, foreach_stmt_struct;

typedef struct {
   expr_type condition;
   list_type body;
} *while_stmt_type, while_stmt_struct;

typedef struct {
   list_type initassign;
   expr_type condition;
   list_type assign;
   list_type body;
} *for_stmt_type, for_stmt_struct;

typedef struct {
   expr_type condition;
   list_type body;
} *case_stmt_type, case_stmt_struct;

typedef struct {
   expr_type condition;
   list_type body;
} *switch_stmt_type, switch_stmt_struct;

typedef struct {
   int type;
   int lineno;  /* Line # of statement for debugging information */
   union {
      call_stmt_type    call_stmt_val; 
      if_stmt_type      if_stmt_val; 
      assign_stmt_type  assign_stmt_val; 
      foreach_stmt_type foreach_stmt_val; 
      while_stmt_type   while_stmt_val;
      for_stmt_type     for_stmt_val;
      switch_stmt_type  switch_stmt_val;
      case_stmt_type    case_stmt_val;
      expr_type         return_stmt_val; 
   } value;
} *stmt_type, stmt_struct;

typedef struct {
   id_type message_id;
   list_type params;
} *message_header_type, message_header_struct;

typedef struct {
   message_header_type header;
   const_type          comment;     /* Index of per-handler comment string, or NULL if none */
   list_type locals;
   list_type body;
} *message_handler_type, message_handler_struct;

typedef struct _class {
   id_type              class_id;
   struct _class        *superclass;   /* Pointer to superclass's class structure */
   int                  numproperties; /* # of properties in a class + all superclasses */
   int                  numclassvars; /* # of classvars in a class + all superclasses */
   list_type            resources;
   list_type            classvars;
   list_type            properties;
   list_type            messages;
   int                  is_new;	     /* True iff class needs code to be generated for it */
} *class_type, class_struct;

/* Function parameter types --see function.c */
enum {ANONE=0, AEXPRESSION, AEXPRESSIONS, ASETTING, ASETTINGS};
typedef struct {
   const char name[MAXFNAME];
   int  opcode;
   int  params[MAXARGS];
} function_type;

/* Class & superclass names for warning messages */
typedef struct {
   id_type class_id;
   id_type superclass;
} *recompile_type, recompile_struct;

typedef struct {
   Table globalvars;     /* Identifiers with global scope */
   Table classvars;      /* Identifiers with class scope */
   Table localvars;      /* Identifiers with message handler scope */
   Table missingvars;    /* Identifiers that have yet to be defined */
                         /*
			  * (This table is to be used for classes, messages, and
			  *  parameters that must be referenced before they are
			  *  defined, as in class names in CreateObject, etc.)
			  */
   list_type classes;    /* List of all classes */
   int maxid;            /* Highest identifier # in use */
   int maxlocals;        /* # of local variables in current handler */
   int maxproperties;    /* # of properties in current class */
   int maxclassvars;     /* # of classvars in current class */
   int maxresources;     /* Highest # resource used so far */
   int curclass;         /* Current class id # */
   int curmessage;       /* Current message handler id # */
   list_type recompile_list; /* List of classes that need to be recompiled */
   list_type constants;  /* List of constants declared in current class */

   int num_strings;      /* Number of debugging strings encountered so far */
   list_type strings;    /* List of pointers to debugging strings */ 

   list_type override_classvars; /* Classvars that are overridden by properties in current class */
} SymbolTable;


/*************************** Function prototypes *************************/
extern int yylex (void);

/* utility functions in main.c */
void yyerror(const char *s);
char *assemble_string(char *str);

void include_file(char *filename);   


/* action handlers */
const_type make_numeric_constant(int);
const_type make_nil_constant(void);
const_type make_string_constant(char *);
const_type make_string_resource(char *);
const_type make_fname_resource(char *);
const_type make_number_from_constant_id(id_type id);

const_type make_literal_class(id_type id);
const_type make_literal_message(id_type id);

id_type make_identifier(char *);
id_type make_var(id_type);


expr_type make_expr_from_id(id_type);
expr_type make_expr_from_call(stmt_type);
expr_type make_expr_from_constant(const_type);
expr_type make_expr_from_literal(id_type id);
expr_type make_bin_op(expr_type, int, expr_type);
expr_type make_un_op(int, expr_type);

arg_type make_arg_from_expr(expr_type expr);
arg_type make_arg_from_setting(id_type id, expr_type expr);

id_type make_constant_id(id_type, expr_type);
param_type make_parameter(id_type, expr_type);
classvar_type make_classvar(id_type, expr_type);
property_type make_property(id_type, expr_type);
resource_type make_resource(id_type, const_type, int);
int make_language_id(char *);

void check_break(void);
void check_continue(void);
stmt_type make_prop_stmt(void);
stmt_type make_if_stmt(expr_type, list_type, list_type, stmt_type);
stmt_type make_assign_stmt(id_type, expr_type);
stmt_type make_foreach_stmt(id_type, expr_type, list_type);
stmt_type make_while_stmt(expr_type, list_type);
stmt_type make_do_while_stmt(expr_type, list_type);
stmt_type make_for_stmt(list_type, expr_type, list_type, list_type);
stmt_type make_case_stmt(expr_type, list_type, bool);
stmt_type make_switch_stmt(expr_type, list_type);
stmt_type make_call(id_type, list_type);
stmt_type make_list_call(list_type);
stmt_type allocate_statement(void);
list_type add_statement(list_type l, stmt_type s);

message_handler_type make_message_handler(message_header_type header, char *comment,
					  list_type locals, list_type stmts);
message_header_type make_message_header(id_type, list_type);

class_type make_class_signature(id_type, id_type);
class_type make_class(class_type c, list_type resources, list_type classvars,
		      list_type properties, list_type messages);

/* miscellaneous */
void enter_loop(void);
void leave_loop(void);
void action_error(const char *fmt, ...);
void simple_error(const char *fmt, ...);
void simple_warning(const char *fmt, ...);
void initialize_parser(void);

int id_hash(const void *info, int table_size);
int id_compare(void *info1, void *info2);
int recompile_compare(void *info1, void *info2);
int class_compare(void *info1, void *info2);
int add_identifier(id_type id, int type);
int get_statement_line(stmt_type s, int curline);

void codegen(char *current_fname, char *bof_fname);
void set_kodbase_filename(char *filename);
int load_kodbase(void);
int save_kodbase(void);

/*************************** Global variables *************************/
extern int generate_code; 	/* Nonzero if we should generate code */
extern SymbolTable st;          /* Compiler's symbol table */

/**************************** Include files ***************************/
#include "sort.h"
#include "optimize.h"

#endif /* #ifdef _BLAKCOMP_H */

