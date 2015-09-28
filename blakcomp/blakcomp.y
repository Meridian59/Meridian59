%{
#include "blakcomp.h"
#include <stdio.h>
%}

%union {
	int  int_val;		/* A numerical value */
	char *string_val;	/* A string value */
	char *idname;		/* The name of an identifier */
	id_type   id_val;       /* An identifier, with type info */
	stmt_type stmt_val;	/* A statement */
	list_type list_val;	/* A generic linked list */
	const_type const_val;  	/* A constant */
	expr_type  expr_val;    /* An expression */

	arg_type   arg_val;     /* An argument to a procedure call */
	param_type param_val;	/* A parameter declaration */
	classvar_type classvar_val;	/* A class variable */
	property_type prop_val;	/* A property */
	resource_type resource_val;	/* A resource */
	message_handler_type handler_val;	/* A message handler */
	message_header_type header_val;		/* A message header */
	class_type class_val;

	call_stmt_type    call_stmt_val;	/* A function call */
	if_stmt_type      if_stmt_val;		/* etc. */
	assign_stmt_type  assign_stmt_val; /* A "standalone" assignment with separator */ 
	foreach_stmt_type foreach_stmt_val; /* Iterate through KOD lists */
	while_stmt_type   while_stmt_val; /* While loop */
	for_stmt_type     for_stmt_val; /* For loop */
	switch_stmt_type  switch_stmt_val; /* Switch-case statement */
	case_stmt_type    case_stmt_val; /* Switch-case statement */
}

%type <int_val>		Classes
%type <stmt_val> 	statement
%type <list_val> 	statement_list parameter_list argument_list vars case_list
%type <list_val>	locals classvar_list property_list resource_list expression_list 
%type <list_val>	message_handler_list arg_list2 param_list2 assign_list
%type <list_val>	Messages_Block Classvars_Block Properties_Block Resources_Block
%type <int_val>         Constants_Block constants_list constant_assign

%type <id_val>	  	id var
%type <const_val> 	constant resource_const literal
%type <string_val>	fname
%type <expr_val>	expression
%type <arg_val>	        argument
%type <int_val> language_const
%type <param_val>	parameter
%type <classvar_val>	classvar
%type <prop_val>	property
%type <resource_val>	resource
%type <handler_val>   	message_handler
%type <header_val>   	message_header
%type <class_val>	Class Class_Signature

%type <stmt_val> 	call if_stmt assign_stmt foreach_stmt while_stmt for_stmt assign
%type <stmt_val> case_stmt switch_stmt

%token <int_val>  	NUMBER REL_OP INC_DEC_OP
%token <string_val> 	STRING_CONSTANT FILENAME
%token <idname>		IDENTIFIER

%token AND BREAK CLASSVARS CONSTANTS CONTINUE ELSE FOR IF IN IS LOCAL MESSAGES 
%token MOD NOT OR PROPAGATE PROPERTIES RESOURCES RETURN WHILE DO SWITCH CASE
%token END EOL SEP INCLUDE FOREACH DEFAULT

/* precedence of operators, lowest precedence first */
%left OR
%left AND
%left '|'
%left '&' 
%left REL_OP '='
%left '+' '-'
%left '*' '/' MOD
%nonassoc INC_OP DEC_OP UMINUS NOT '~'
%%

Start:
		Classes		{}

Classes: 
		/* empty */	{ $$ = 0; }
	| 	Classes Class	{ $$ = 0; }
	;

Class:
		Class_Signature Constants_Block Resources_Block Classvars_Block Properties_Block Messages_Block end_junk
		{ $$ = make_class($1, $3, $4, $5, $6); }
	;

end_junk:       END 
        |       END EOL
        ;

Class_Signature:
		id EOL		{ $$ = make_class_signature($1, NULL); }
	| 	id IS id EOL	{ $$ = make_class_signature($1, $3); }
	;

Constants_Block:
		/* empty */				{ $$ = 0; }
	|	CONSTANTS ':' EOL constants_list        { $$ = 0; }
	;

Resources_Block:
		/* empty */				{ $$ = NULL; }
	|	RESOURCES ':' EOL resource_list		{ $$ = $4; }
	;
 
Classvars_Block:
		/* empty */				{ $$ = NULL; }
	|	CLASSVARS ':' EOL classvar_list        { $$ = $4; }
	;

Properties_Block:
		/* empty */				{ $$ = NULL; }
	|	PROPERTIES ':' EOL property_list	{ $$ = $4; }
	;

Messages_Block:
		/* empty */				{ $$ = NULL; }
	|	MESSAGES ':' message_handler_list	{ $$ = $3; }
	;

constants_list: 
		/* empty */			{ $$ = 0; }
	|	constants_list constant_assign  { $$ = 0; }
	;

constant_assign:
		id '=' expression EOL	{ $$ = 0; make_constant_id($1, $3); }
	|       INCLUDE fname EOL       { include_file($2); }
	|	error EOL		{ $$ = 0; } 
	;

resource_list:
		/* empty */		{ $$ = NULL; }
	|	resource_list resource  { if ($2 != NULL) $$ = list_add_item($1, $2); }
	;

resource:
		id '=' resource_const EOL{ $$ = make_resource($1, $3, 0); }
	|	id '=' language_const resource_const EOL{ $$ = make_resource($1, $4, $3); }
	|	INCLUDE fname EOL { include_file($2); $$ = NULL; }
	|	error EOL			{ $$ = NULL; }
	;

language_const:
		IDENTIFIER { $$ = make_language_id($1); }
;

resource_const:
		STRING_CONSTANT { $$ = make_string_resource($1); }
	|	fname		{ $$ = make_fname_resource($1); }
	;

classvar_list:
		/* empty */		{ $$ = NULL; }
	|	classvar_list classvar 	{ $$ = list_add_item($1, $2); }
	;

property_list:
		/* empty */		{ $$ = NULL; }
	|	property_list property 	{ $$ = list_add_item($1, $2); }
	;

classvar:
		id '=' expression EOL	{ $$ = make_classvar($1, $3); }
	|	id EOL			{ $$ = make_classvar($1, make_expr_from_constant(make_nil_constant())); }
	|	error EOL		{ $$ = NULL; }
	;

property:
		id '=' expression EOL	{ $$ = make_property($1, $3); }
	|	id EOL			{ $$ = make_property($1, make_expr_from_constant(make_nil_constant())); }
	|	error EOL		{ $$ = NULL; }
	;

message_handler_list:
      /* empty */				{ $$ = NULL; }
	|	message_handler_list message_handler	{ $$ = list_add_item($1, $2); }
	;

message_handler:
		message_header '{' locals statement_list '}'
		{ $$ = make_message_handler($1, NULL, $3, $4); }
 	/* this form includes a comment string describing the message handler */
        |       message_header STRING_CONSTANT '{' locals statement_list '}'
		{ $$ = make_message_handler($1, $2, $4, $5); }
	;

message_header:
		id parameter_list
		{ $$ = make_message_header($1, $2); }
	;

locals:
		/* empty */	{ $$ = NULL; }		
	|	LOCAL vars SEP  { $$ = $2; }
	;

vars:
		var		{ $$ = list_add_item(NULL, $1); }
	|	vars ',' var	{ $$ = list_add_item($1, $3); }
	;

var:		id		{ $$ = make_var($1); }
	;

parameter_list:
		'(' ')'			{ $$ = NULL; }
	|	'(' param_list2 ')' 	{ $$ = $2; }

param_list2:
		parameter			{ $$ = list_add_item(NULL, $1); }
	|	param_list2 ',' parameter	{ $$ = list_add_item($1, $3); }
	;

parameter:
		id '=' expression	{ $$ = make_parameter($1, $3); }
	;

statement_list:
		/* empty */	{ $$ = NULL; }
	| 	statement_list statement { if ($2 != NULL) $$ = add_statement($1, $2); }
	;

statement: 
		call SEP	{ $$ = $1; }
	|	if_stmt		{ $$ = $1; }
	|	assign_stmt	{ $$ = $1; }
	|	foreach_stmt	{ $$ = $1; }
	| 	while_stmt	{ $$ = $1; }
	| 	for_stmt	{ $$ = $1; }
	| 	switch_stmt	{ $$ = $1; }
	|	PROPAGATE SEP	{ $$ = make_prop_stmt(); }
	|	RETURN expression SEP
				{ $$ = allocate_statement();
				  $$->type = S_RETURN;
				  $$->value.return_stmt_val = $2; }
	|	RETURN SEP
				{ $$ = allocate_statement();
				  $$->type = S_RETURN;
				  $$->value.return_stmt_val = 
				     make_expr_from_constant(make_nil_constant()); }
	|	BREAK SEP	{ check_break();
				  $$ = allocate_statement();
				  $$->type = S_BREAK; }
	|	CONTINUE SEP	{ check_continue();
				  $$ = allocate_statement();
				  $$->type = S_CONTINUE; }
	|       INCLUDE fname SEP { include_file($2); $$ = NULL; }
	|	error SEP	{ $$ = NULL; }
	;

if_stmt:
		IF expression '{' statement_list '}'		{ $$ = make_if_stmt($2, $4, NULL, NULL); }
	|	IF expression '{' statement_list '}' ELSE '{' statement_list '}' 		{ $$ = make_if_stmt($2, $4, $8, NULL); }
	|	IF expression '{' statement_list '}' ELSE if_stmt	{ $$ = make_if_stmt($2, $4, NULL, $7); }
	;

// List of assignments with no separators.
assign_list:
		/* empty */	{ $$ = NULL; }
	|	assign			{ if ($1 != NULL) $$ = add_statement(NULL, $1); }
	|	assign_list ',' assign	{ if ($3 != NULL) $$ = add_statement($1, $3); }
	;

// Assignments with separators.
assign_stmt:
		id '=' expression SEP { $$ = make_assign_stmt($1, $3); }
	|	id INC_OP SEP			{ $$ = make_assign_stmt($1, make_un_op(POST_INC_OP, make_expr_from_id($1))); }
	|	DEC_OP id SEP			{ $$ = make_assign_stmt($2, make_un_op(PRE_DEC_OP, make_expr_from_id($2))); }
	|	id DEC_OP SEP			{ $$ = make_assign_stmt($1, make_un_op(POST_DEC_OP, make_expr_from_id($1))); }
	|	INC_OP id SEP			{ $$ = make_assign_stmt($2, make_un_op(PRE_INC_OP, make_expr_from_id($2))); }
	;

// Assignments without separators.
assign:
		id '=' expression { $$ = make_assign_stmt($1, $3); }
	|	id INC_OP			{ $$ = make_assign_stmt($1, make_un_op(POST_INC_OP, make_expr_from_id($1))); }
	|	DEC_OP id			{ $$ = make_assign_stmt($2, make_un_op(PRE_DEC_OP, make_expr_from_id($2))); }
	|	id DEC_OP			{ $$ = make_assign_stmt($1, make_un_op(POST_DEC_OP, make_expr_from_id($1))); }
	|	INC_OP id			{ $$ = make_assign_stmt($2, make_un_op(PRE_INC_OP, make_expr_from_id($2))); }
	;

// Iterate through KOD list.
foreach_stmt:
		FOREACH id IN expression '{' start_loop statement_list '}' end_loop
		{ $$ = make_foreach_stmt($2, $4, $7); }
	;

// For loop, e.g. for(i=0,i<10,i++).
for_stmt:
		FOR '(' assign_list SEP expression SEP assign_list ')' '{' start_loop statement_list '}' end_loop
		{ $$ = make_for_stmt($3, $5, $7, $11); }
	|	FOR '(' assign_list SEP SEP assign_list ')' '{' start_loop statement_list '}' end_loop
		{ $$ = make_for_stmt($3, make_expr_from_constant(make_numeric_constant(1)), $6, $10); }
	;

while_stmt:
		WHILE expression '{' start_loop statement_list '}' end_loop
		{ $$ = make_while_stmt($2, $5); }
	|	DO '{' start_loop statement_list '}' WHILE expression SEP end_loop
		{ $$ = make_do_while_stmt($7, $4); }
	;

switch_stmt:
		SWITCH '(' expression ')' '{' start_loop case_list '}' end_loop
		{ $$ = make_switch_stmt($3, $7); }
	;

case_list:
		case_stmt { $$ = list_add_item(NULL, $1); }
	|	case_list case_stmt { $$ = list_add_item($1, $2); }
	;

case_stmt:
		CASE expression ':' statement_list { $$ = make_case_stmt($2, $4, 0); }
	|	DEFAULT ':' statement_list
		{ $$ = make_case_stmt(make_expr_from_constant(make_numeric_constant(1)), $3, 1); }
	;

start_loop:	/* empty*/	{ enter_loop(); }
	;

end_loop:	/* empty*/	{ leave_loop(); }
	;

call:
		id argument_list	{ $$ = make_call($1, $2); }
	|	'[' ']'			{ $$ = make_list_call(NULL); }
	|	'[' expression_list ']'	{ $$ = make_list_call($2); }
	;

expression_list:
		expression			{ $$ = list_add_item(NULL, $1); }
	|	expression_list ',' expression	{ $$ = list_add_item($1, $3); }
	;

argument_list:
		'(' ')'			{ $$ = NULL; }
	|	'(' arg_list2   ')' 	{ $$ = $2; }
	;

arg_list2:
		argument		{ $$ = list_add_item(NULL, $1); }
	|	arg_list2 ',' argument	{ $$ = list_add_item($1, $3); }
	;	

argument:
		expression		{ $$ = make_arg_from_expr($1); }
	|	'#' id '=' expression  { $$ = make_arg_from_setting($2, $4); }
	;

expression:
		expression AND expression	{ $$ = make_bin_op($1, AND_OP, $3); }
	|	expression OR expression	{ $$ = make_bin_op($1, OR_OP, $3); }
	|	expression REL_OP expression	{ $$ = make_bin_op($1, $2, $3); }
	|	expression '=' expression	{ $$ = make_bin_op($1, EQ_OP, $3); }
	|	expression '+' expression	{ $$ = make_bin_op($1, PLUS_OP, $3); }
	|	expression '-' expression	{ $$ = make_bin_op($1, MINUS_OP, $3); }
	|	expression '*' expression	{ $$ = make_bin_op($1, MULT_OP, $3); }
	|	expression '/' expression	{ $$ = make_bin_op($1, DIV_OP, $3); }
	|	expression MOD expression	{ $$ = make_bin_op($1, MOD_OP, $3); }
	|	expression '&' expression	{ $$ = make_bin_op($1, BITAND_OP, $3); }
	|	expression '|' expression	{ $$ = make_bin_op($1, BITOR_OP, $3); }
	|	'-' expression %prec UMINUS	{ $$ = make_un_op(NEG_OP, $2); }
	|	NOT expression			{ $$ = make_un_op(NOT_OP, $2); }
	|	'~' expression			{ $$ = make_un_op(BITNOT_OP, $2); }
	|	id INC_OP			{ $$ = make_un_op(POST_INC_OP, make_expr_from_id($1)); }
	|	INC_OP id			{ $$ = make_un_op(PRE_INC_OP, make_expr_from_id($2)); }
	|	id DEC_OP			{ $$ = make_un_op(POST_DEC_OP, make_expr_from_id($1)); }
	|	DEC_OP id			{ $$ = make_un_op(PRE_DEC_OP, make_expr_from_id($2)); }
	|	constant			{ $$ = make_expr_from_constant($1); }
	|	literal				{ $$ = make_expr_from_constant($1); }
	|	call				{ $$ = make_expr_from_call($1); }
	|	id				{ $$ = make_expr_from_id($1); }
	|	'(' expression ')'		{ $$ = $2; }
	;

constant:
		NUMBER		{ $$ = make_numeric_constant($1); }
	|	'$'		{ $$ = make_nil_constant(); }
        /* This is used for debugging strings in kod */
	|	STRING_CONSTANT	{ $$ = make_string_constant($1); }
	;

fname:
		FILENAME	{ $$ = $1; }	
	|	IDENTIFIER	{ $$ = $1; }
	;

literal:
		'&' id		{ $$ = make_literal_class($2); }
	| 	'@' id		{ $$ = make_literal_message($2); }
	| 	'*' id		{ $$ = make_literal_variable($2); }
	;

id:
		IDENTIFIER	{ $$ = make_identifier($1); }
	;


%%

