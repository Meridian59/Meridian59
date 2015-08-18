// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* codegen.c  Code generation procedures for Blakod compiler */

#include "blakcomp.h"
#include "bkod.h"
#include "codegen.h"
#include "resource.h"

static BYTE bof_magic[] = { 0x42, 0x4F, 0x46, 0xFF };

int codegen_ok;
extern int debug_bof;  /* Should we put debugging info into .bof? */

typedef struct {
   int lineno;   // Kod line number
   int offset;   // Offset in bof of beginning of statement
} DebugLine;

static list_type debug_lines;   // list of DebugLine structures
int outfile; /* File handle of output file */
static list_type loop_stack;    /* Info for all current (possibly nested) loops */
static loop_type current_loop;  /* Info for current loop */

/************************************************************************/
/*
 * codegen_header: Write out header stuff before class info.
 */
void codegen_header(void)
{
   int i;
   for (i=0; i < 4; i++)
      OutputByte(outfile, bof_magic[i]);

   OutputInt(outfile, BOF_VERSION);
}
/************************************************************************/
/*
 * codegen_classes: Generate code for all classes.
 */
void codegen_classes(void)
{
   list_type templist, c = NULL;
   int numclasses, classpos, endpos, i;

   /* Make list of only classes which appeared in current source file */
   for (templist = st.classes; templist != NULL; templist = templist->next)
      if ( ((class_type) (templist->data))->is_new == True)
	 c = list_add_item(c, templist->data);
   
   /* Write out # of classes */
   numclasses = list_length(c);
   OutputInt(outfile, numclasses);

   classpos = FileCurPos(outfile);
   
   /* Write out class table */
   for (i=0; i < numclasses; i++)
   {
      /* Leave space for class id # */
      OutputInt(outfile, 0);
      
      /* Leave space for class offset */
      OutputInt(outfile, 0);
   }
   
   /* Loop through classes found in source code */
   for ( ; c != NULL; c = c->next)
   {
      class_type each_class = (class_type) (c->data);
      
      /* Backpatch in class id # and offset of current class */
      endpos = FileCurPos(outfile);
      FileGoto(outfile, classpos); 
      OutputInt(outfile, each_class->class_id->idnum);
      OutputInt(outfile, endpos);
      classpos = FileCurPos(outfile);  /* Move to next entry */
      FileGotoEnd(outfile);
      
      codegen_class(each_class);

      if (!codegen_ok)
         return;
   }
}
/************************************************************************/
/*
 * codegen_string_table:  Write out string table.
 */
void codegen_string_table(void)
{
   int i, curpos, total_len;
   char *str;
   list_type l;

   OutputInt(outfile, st.num_strings);

   curpos = FileCurPos(outfile);

   /* Write out offsets of strings (need to know string lengths) */
   total_len = 0;
   l = st.strings;
   for (i=0; i < st.num_strings; i++)
   {
      OutputInt(outfile, curpos + total_len + st.num_strings * 4);
      str = (char *) (l->data);
      total_len += strlen(str) + 1;
      l = l->next;
   }

   /* Now write out the strings themselves */
   l = st.strings;
   for (i=0; i < st.num_strings; i++)
   {
      str = (char *) (l->data);
      write(outfile, str, strlen(str));
      OutputByte(outfile, 0);    // null terminate
      l = l->next;
   }
}
/************************************************************************/
/*
 * codegen_debug_info:  Write out debugging info.
 */
void codegen_debug_info(void)
{
   list_type l;

   OutputInt(outfile, list_length(debug_lines));

   for (l = debug_lines; l != NULL; l = l->next)
   {
      DebugLine *d = (DebugLine *) (l->data);
      OutputInt(outfile, d->offset);
      OutputInt(outfile, d->lineno);
   }
   debug_lines = list_destroy(debug_lines);
}
/************************************************************************/
/*
 * codegen_filename:  Write out given filename.
 */
void codegen_filename(char *filename)
{
   write(outfile, filename, strlen(filename));
   OutputByte(outfile, 0);    // null terminate
}
/************************************************************************/
/* 
 * codegen_call: Generate code for performing a function call and storing
 *   the return value in the given location (must be local or property).
 *   Set this location to NULL to ignore the return value.
 *   maxlocal is highest # local variable used so far in this message handler.
 *   Returns highest # local variable used in code for call.
 */
int codegen_call(call_stmt_type c, id_type destvar, int maxlocal)
{
   /* our_maxlocal gives highest #ed temporary needed to evaluate the
    * entire call.  maxtemps is the highest temp needed for a single arg. */
   int our_maxlocal = maxlocal, maxtemps, normal_args = 0;
   int argnum;
   list_type p;
   opcode_type opcode;
   arg_type arg;
   expr_type expr;
   id_type id;

   /* If an argument is complicated, we have to store it in a temporary */
   for (p = c->args; p != NULL; p = p->next)
   {
      arg = (arg_type) p->data;

      /* Get expression and place it in a temp if necessary */
      if (arg->type == ARG_EXPR)
	 expr = arg->value.expr_val;
      else expr = arg->value.setting_val->expr;

      maxtemps = simplify_expr(expr, maxlocal);
      /* If we need to use a temporary, then this temp must stay untouched until
       * the call is made.  So we must increment maxlocal in the case. */
      if (maxtemps > maxlocal)
	 maxlocal++;
      if (maxtemps > our_maxlocal)
	 our_maxlocal = maxtemps;
   }

   /* Build up call instruction */
   memset(&opcode, 0, sizeof(opcode));  /* Set opcode to all zeros */
   opcode.command = CALL;

   /* Set source1 field to place for return value */
   if (destvar == NULL)
      opcode.source1 = CALL_NO_ASSIGN;
   else
      switch (destvar->type)
      {
      case I_LOCAL:
	 opcode.source1 = CALL_ASSIGN_LOCAL_VAR;
	 break;

      case I_PROPERTY:
	 opcode.source1 = CALL_ASSIGN_PROPERTY;
	 break;

      default:
	 codegen_error("Identifier in expression not a local or property: %s", 
		       destvar->name);
      }

   OutputOpcode(outfile, opcode);

   /* Function # to call */
   OutputByte(outfile,  (BYTE) c->function);

   /* Place to store result, if any */
   if (opcode.source1 != CALL_NO_ASSIGN)
      OutputInt(outfile, destvar->idnum);

   /* Count # of "normal" arguments, i.e. all those
    * except for settings. */
   for (p = c->args; p != NULL; p = p->next)
      if ( ((arg_type) (p->data))->type != ARG_SETTING)
	 normal_args++;

   /* # of "normal" parameters */
   OutputByte(outfile,  (BYTE) normal_args);

   /* First have to generate code for "normal" parameters */
   for (p = c->args, argnum = 0; p != NULL; p = p->next, argnum++)
   {
      arg = (arg_type) p->data;

      switch (arg->type)
      {
      case ARG_EXPR:
	 /* Expression was reduced to simple form above */
	 OutputBaseExpression(outfile, arg->value.expr_val);
	 break;

      case ARG_SETTING:
	 /* Settings are handled below */
	 break;

      default:
	 codegen_error("Unknown argument type (%d) in argument %d", arg->type, argnum);	 
	 break;
      }
   }

   /* # of settings */
   OutputByte(outfile,  (BYTE) (list_length(c->args) - normal_args));

   /* Now take care of settings */
   for (p = c->args; p != NULL; p = p->next)
   {
      arg = (arg_type) p->data;

      if (arg->type == ARG_SETTING)
      {
	 id = arg->value.setting_val->id;
	 /* Write out parameter #, then rhs of assignment */
	 OutputInt(outfile, id->idnum);

	 OutputBaseExpression(outfile, arg->value.setting_val->expr);
      }
   }
   return our_maxlocal;
}
/************************************************************************/
/* 
 * codegen_return:  Generate code for a RETURN statement.
 *   expr is the expression to return, maxlocal is the highest # local
 *   variable used so far.
 *   Returns highest # local variable used in code for return.
 */
int codegen_return(expr_type expr, int maxlocal)
{
   opcode_type opcode;
   int our_maxlocal = maxlocal, sourceval;

   /* If expression is complicated, place in temp variable first */
   our_maxlocal = simplify_expr(expr, maxlocal);

   memset(&opcode, 0, sizeof(opcode));  /* Set opcode to all zeros */
   opcode.command = RETURN;
   sourceval = set_source_id(&opcode, SOURCE1, expr);
   opcode.dest = NO_PROPAGATE;

   OutputOpcode(outfile, opcode);
   
   /* Write out return value (whether a constant or a variable) */
   OutputInt(outfile, sourceval);
   return our_maxlocal;
}
/************************************************************************/
/*
 * codegen_if: Generate code for an if-then-else statement.
 *    numlocals should be # of local variables for message excluding temps.
 *   Returns highest # local variable used in code for statement.
 */
int codegen_if(if_stmt_type s, int numlocals)
{
   opcode_type opcode;
   int our_maxlocal = numlocals, numtemps, sourceval;
   long gotopos, thenpos;
   list_type p;

   /* First generate code for condition */
   our_maxlocal = simplify_expr(s->condition, numlocals);

   /* Jump over then clause if condition is false */
   memset(&opcode, 0, sizeof(opcode));  /* Set opcode to all zeros */
   opcode.command = GOTO;
   opcode.dest = GOTO_IF_FALSE;
   sourceval = set_source_id(&opcode, SOURCE1, s->condition);
   OutputOpcode(outfile, opcode);

   /* Leave space for destination address */
   gotopos = FileCurPos(outfile);
   OutputInt(outfile, 0);

   OutputInt(outfile, sourceval);

   /* Write code for then clause */
   for (p = s->then_clause; p != NULL; p = p->next)
   {
      numtemps = codegen_statement((stmt_type) p->data, numlocals);
      if (numtemps > our_maxlocal)
	 our_maxlocal = numtemps;
   }

   /* If there is an else clause, jump over it */
   if (s->else_clause != NULL || s->elseif_clause != NULL)
   {
      opcode.source1 = 0;
      opcode.source2 = GOTO_UNCONDITIONAL;
      opcode.dest = 0;
      OutputOpcode(outfile, opcode);

      /* Leave space for destination address */
      thenpos = FileCurPos(outfile);
      OutputInt(outfile, 0);
   }

   /* Go back and fill in destination address for conditional goto */
   BackpatchGoto(outfile, gotopos, FileCurPos(outfile));

   /* If there's an else clause, write out its code */
   if (s->else_clause != NULL)
   {
      for (p = s->else_clause; p != NULL; p = p->next)
      {
	 numtemps = codegen_statement((stmt_type) p->data, numlocals);
	 if (numtemps > our_maxlocal)
	    our_maxlocal = numtemps;
      }
      /* Go back and fill in destination address for end of then clause */
      BackpatchGoto(outfile, thenpos, FileCurPos(outfile));
   }
   else if (s->elseif_clause != NULL)
   {
      stmt_type elseif;
      elseif = (stmt_type)s->elseif_clause;
      numtemps = codegen_if(elseif->value.if_stmt_val, numlocals);
      if (numtemps > our_maxlocal)
         our_maxlocal = numtemps;
      BackpatchGoto(outfile, thenpos, FileCurPos(outfile));
   }
   return our_maxlocal;
}
/************************************************************************/
/*
 * codegen_switch: Generate code for an switch-case statement.
 *   numlocals should be # of local variables for message excluding temps.
 *   Returns highest # local variable used in code for statement.
 */
int codegen_switch(switch_stmt_type s, int numlocals)
{
   opcode_type opcode;
   int our_maxlocal = numlocals, numtemps, sourceval;
   long toppos, endpos, casepos[1024], defaultpos; // Keep track of file positions.
   list_type case_list, case_list2; // List of cases for the switch.
   list_type q; // The code statements in each case.
   list_type p; // Used when backpacking continue statements.
   stmt_type case_stmt = NULL, case_stmt2 = NULL, default_stmt = NULL;
   int numCase = 0; // Keep track of number of cases.
   expr_type temp_expr; // Used for comparison of the switch condition with each case.

   id_type temp_id; // Temp ID for non-ID switch condition expressions.
   expr_type switch_condition; // Switch condition expression.
   stmt_type temp_stmt; // Temp assign statement for non-ID switch conditions.
   assign_stmt_type assign_stmt; // Temp assign statement for non-ID switch conditions. 

   // First iterate through case list and check that none are duplicate constants or IDs.
   for (case_list = s->body; case_list != NULL; case_list = case_list->next)
   {
      case_stmt = (stmt_type)case_list->data;
      if (case_stmt->type == S_DEFAULTCASE)
         continue;
      for (case_list2 = s->body; case_list2 != NULL; case_list2 = case_list2->next)
      {
         case_stmt2 = (stmt_type)case_list2->data;
         if (case_stmt2 == case_stmt || case_stmt2->type == S_DEFAULTCASE)
            continue;
         if (case_stmt->value.case_stmt_val->condition->type
            == case_stmt2->value.case_stmt_val->condition->type)
         {
            if (case_stmt->value.case_stmt_val->condition->type == E_CONSTANT
               && case_stmt->value.case_stmt_val->condition->value.constval->value.numval
               == case_stmt2->value.case_stmt_val->condition->value.constval->value.numval)
            {
               codegen_error("Duplicate constant %i in switch statement",
                  case_stmt->value.case_stmt_val->condition->value.constval->value.numval);
            }
            if (case_stmt->value.case_stmt_val->condition->type == E_IDENTIFIER
               && case_stmt->value.case_stmt_val->condition->value.idval->idnum
               == case_stmt->value.case_stmt_val->condition->value.idval->idnum)
            {
               codegen_error("Duplicate ID %i in switch statement",
                  case_stmt->value.case_stmt_val->condition->value.idval->idnum);
            }
         }
      }
   }

   if (s->condition->type == E_IDENTIFIER || s->condition->type == E_CONSTANT)
      switch_condition = s->condition;
   else
   {
      // Make a temp ID for switch condition expression, assign the expr to it.
      temp_id = make_temp_var(numlocals + 1);

      // Assign the expression to the ID
      assign_stmt = (assign_stmt_type)SafeMalloc(sizeof(assign_stmt_struct));
      assign_stmt->lhs = temp_id;
      assign_stmt->rhs = s->condition;
      
      temp_stmt = (stmt_type)SafeMalloc(sizeof(stmt_struct));
      temp_stmt->type = S_ASSIGN;
      temp_stmt->value.assign_stmt_val = assign_stmt;
      temp_stmt->lineno = 0;
      numtemps = codegen_statement(temp_stmt, numlocals);

      /* Reserve variable "temp" through entire loop by incrementing numlocals */
      our_maxlocal = ++numlocals;
      if (numtemps > our_maxlocal)
         our_maxlocal = numtemps;

      // Make the completed ID into an expression.
      switch_condition = (expr_type)SafeMalloc(sizeof(expr_struct));
      switch_condition->type = E_IDENTIFIER;
      switch_condition->value.idval = temp_id;
      switch_condition->lineno = 0;
   }

   /* Generate code for conditions */
   numtemps = simplify_expr(switch_condition, numlocals);
   if (numtemps > our_maxlocal)
      our_maxlocal = numtemps;

   // Loop through cases, generate goto statements for each.
   for (case_list = s->body; case_list != NULL; case_list = case_list->next)
   {
      case_stmt = (stmt_type) case_list->data;
      if (case_stmt->type == S_DEFAULTCASE)
      {
         // If we already have a default case, this means we have two in
         // the same switch statement. Give codegen error.
         if (default_stmt)
            codegen_error("Two defaults defined in switch statement!");

         // Save default for last.
         default_stmt = case_stmt;
         continue;
      }

      numtemps = simplify_expr(case_stmt->value.case_stmt_val->condition, numlocals);
      if (numtemps > our_maxlocal)
         our_maxlocal = numtemps;
      // Make an equality check expression from switch condition and current case condition.
      temp_expr = make_bin_op(switch_condition, EQ_OP, case_stmt->value.case_stmt_val->condition);
      numtemps = simplify_expr(temp_expr, numlocals);
      if (numtemps > our_maxlocal)
         our_maxlocal = numtemps;

      /* Jump to clause if condition is true */
      memset(&opcode, 0, sizeof(opcode));
      opcode.command = GOTO;
      opcode.dest = GOTO_IF_TRUE;
      sourceval = set_source_id(&opcode, SOURCE1, temp_expr);
      OutputOpcode(outfile, opcode);

      /* Leave space for destination address */
      casepos[numCase] = FileCurPos(outfile);
      numCase++;
      OutputInt(outfile, 0);
      OutputInt(outfile, sourceval);
   }

   // Put the goto statement for the default case last.
   if (default_stmt)
   {
      opcode.source1 = 0;
      opcode.source2 = GOTO_UNCONDITIONAL;
      opcode.dest = 0;
      OutputOpcode(outfile, opcode);
      /* Leave space for destination address */
      defaultpos = FileCurPos(outfile);
      OutputInt(outfile, 0);
   }
   else
   {
      // No default case, goto end of switch.
      opcode.source1 = 0;
      opcode.source2 = GOTO_UNCONDITIONAL;
      opcode.dest = 0;
      OutputOpcode(outfile, opcode);
      /* Leave space for destination address */
      endpos = FileCurPos(outfile);
      OutputInt(outfile, 0);
   }

   toppos = FileCurPos(outfile);
   codegen_enter_loop();
   numCase = 0;
   case_list = NULL;
   q = NULL;

   // Loop through cases again and write out code.
   for (case_list = s->body; case_list != NULL; case_list = case_list->next)
   {
      case_stmt = (stmt_type) case_list->data;
      // Default case location is at defaultpos.
      if (case_stmt->type == S_DEFAULTCASE)
      {
         BackpatchGoto(outfile, defaultpos, FileCurPos(outfile));
      }
      else
      {
         BackpatchGoto(outfile, casepos[numCase], FileCurPos(outfile));
         numCase++;
      }
      // Write code for statements.
      for (q = case_stmt->value.case_stmt_val->body; q != NULL; q = q->next)
      {
         numtemps = codegen_statement((stmt_type)q->data, numlocals);
         if (numtemps > our_maxlocal)
            our_maxlocal = numtemps;
      }
   }

   /* Backpatch continue statements in loop body */
   for (p = current_loop->for_continue_list; p != NULL; p = p->next)
      BackpatchGoto(outfile, (int)p->data, FileCurPos(outfile));

   /* Go back and fill in destination address for conditional goto */
   if (!default_stmt)
      BackpatchGoto(outfile, endpos, FileCurPos(outfile));

   codegen_exit_loop();

   return our_maxlocal;
}
/************************************************************************/
/*
 * codegen_enter_loop:  Fix up loop stack when a loop is entered.  The top of the
 *   loop is set to the current position in the file.
 */
void codegen_enter_loop(void)
{
   list_type temp;
   /* Loop info holds info needed to handle break and continue statements */
   loop_type loop_info = (loop_type) SafeMalloc(sizeof(loop_struct));

   /* Make new loop_info structure and add it to the loop "stack" */
   loop_info->toppos = FileCurPos(outfile);
   loop_info->break_list = NULL;
   loop_info->for_continue_list = NULL;
   current_loop = loop_info;
   
   /* Add new loop to front of list */
   temp = list_create( (void *) loop_info);
   loop_stack = list_append(temp, loop_stack);   
}
/************************************************************************/
/*
 * codegen_exit_loop:  Fix up loop stack when a loop is exited.  Break statements
 *   jump to the current file position.
 */
void codegen_exit_loop(void)
{
   list_type p;

   /* Backpatch break statements to jump to end of loop */
   for (p = current_loop->break_list; p != NULL; p = p->next)
      BackpatchGoto(outfile,  (int) p->data, FileCurPos(outfile));

   /* Remove current list from loop "stack" */
   loop_stack = list_delete_first(loop_stack);
   
   /* Restore current_loop to correct state */
   current_loop = (loop_type) list_first_item(loop_stack);
}
/************************************************************************/
/*
 * codegen_while: Generate code for a while loop statement.
 *    numlocals should be # of local variables for message excluding temps.
 *   Returns highest # local variable used in code for statement.
 */
int codegen_while(while_stmt_type s, int numlocals)
{
   opcode_type opcode;
   int our_maxlocal = numlocals, numtemps, sourceval;
   long toppos;
   list_type p;

   toppos = FileCurPos(outfile);
   codegen_enter_loop();

   /* First generate code for condition */
   our_maxlocal = simplify_expr(s->condition, numlocals);

   /* Jump over body if condition is false */
   memset(&opcode, 0, sizeof(opcode));  /* Set opcode to all zeros */
   opcode.command = GOTO;
   opcode.dest = GOTO_IF_FALSE;
   sourceval = set_source_id(&opcode, SOURCE1, s->condition);
   OutputOpcode(outfile, opcode);

   /* Make believe goto is a break statement & leave space for backpatching */
   current_loop->break_list = 
      list_add_item(current_loop->break_list, (void *) FileCurPos(outfile));
   OutputInt(outfile, 0);
   OutputInt(outfile, sourceval);

   /* Write code for loop body */
   for (p = s->body; p != NULL; p = p->next)
   {
      numtemps = codegen_statement( (stmt_type) p->data, numlocals);
      if (numtemps > our_maxlocal)
	 our_maxlocal = numtemps;
   }

   /* Goto top of loop is last statement of while loop */
   opcode.source1 = 0;
   opcode.source2 = GOTO_UNCONDITIONAL;
   opcode.dest = 0;
   OutputOpcode(outfile, opcode);
   OutputGotoOffset(outfile, FileCurPos(outfile), toppos);

   codegen_exit_loop();  /* Takes care of break statements */

   return our_maxlocal;
}
/************************************************************************/
/*
 * codegen_dowhile: Generate code for a do-while loop statement.
 *   numlocals should be # of local variables for message excluding temps.
 *   Returns highest # local variable used in code for statement.
 */
int codegen_dowhile(while_stmt_type s, int numlocals)
{
   opcode_type opcode;
   int our_maxlocal = numlocals, numtemps, sourceval;
   long toppos;
   list_type p;

   toppos = FileCurPos(outfile);
   codegen_enter_loop();

   /* Write code for loop body */
   for (p = s->body; p != NULL; p = p->next)
   {
      numtemps = codegen_statement( (stmt_type) p->data, numlocals);
      if (numtemps > our_maxlocal)
         our_maxlocal = numtemps;
   }
   
   /* Backpatch continue statements in loop body */
   for (p = current_loop->for_continue_list; p != NULL; p = p->next)
      BackpatchGoto(outfile, (int)p->data, FileCurPos(outfile));

   /* First generate code for condition */
   our_maxlocal = simplify_expr(s->condition, numlocals);

   /* Jump over body if condition is false */
   memset(&opcode, 0, sizeof(opcode));  /* Set opcode to all zeros */
   opcode.command = GOTO;
   opcode.dest = GOTO_IF_FALSE;
   sourceval = set_source_id(&opcode, SOURCE1, s->condition);
   OutputOpcode(outfile, opcode);

   /* Make believe goto is a break statement & leave space for backpatching */
   current_loop->break_list =
      list_add_item(current_loop->break_list, (void *)FileCurPos(outfile));
   OutputInt(outfile, 0);
   OutputInt(outfile, sourceval);

   /* Goto top of loop is last statement of while loop */
   opcode.source1 = 0;
   opcode.source2 = GOTO_UNCONDITIONAL;
   opcode.dest = 0;
   OutputOpcode(outfile, opcode);
   OutputGotoOffset(outfile, FileCurPos(outfile), toppos);

   codegen_exit_loop();  /* Takes care of break statements */

   return our_maxlocal;
}
/************************************************************************/
/*
* codegen_for: Generate code for a for loop statement.
*   numlocals should be # of local variables for message excluding temps.
*   Returns highest # local variable used in code for statement.
*   Step 1: Assign variables from initassign list.
*   Step 2: Evaluate condition.
*   Step 3: Carry out loop.
*   Step 4: Execute statements from assign list.
*/
int codegen_for(for_stmt_type s, int numlocals)
{
   opcode_type opcode;
   int our_maxlocal = numlocals, numtemps = 0, sourceval;
   long toppos;
   list_type p;
   stmt_type assign_stmt;

   /* Step #1: Assign variables from initassign list. */
   /* For loop can have no assignments, list will be NULL. */
   for (p = s->initassign; p != NULL; p = p->next)
   {
      assign_stmt = (stmt_type)p->data;
      numtemps = codegen_statement(assign_stmt, numlocals);
      if (numtemps > our_maxlocal)
         our_maxlocal = numtemps;
   }

   toppos = FileCurPos(outfile);
   codegen_enter_loop();

   /* Step 2: Evaluate condition. */
   /* If no condition is listed, it will be a positive constant. */
   /* First generate code for condition */
   our_maxlocal = simplify_expr(s->condition, numlocals);

   /* Jump over body if condition is false */
   memset(&opcode, 0, sizeof(opcode));  /* Set opcode to all zeros */
   opcode.command = GOTO;
   opcode.dest = GOTO_IF_FALSE;
   sourceval = set_source_id(&opcode, SOURCE1, s->condition);
   OutputOpcode(outfile, opcode);

   /* Make believe goto is a break statement & leave space for backpatching */
   current_loop->break_list =
      list_add_item(current_loop->break_list, (void *)FileCurPos(outfile));
   OutputInt(outfile, 0);
   OutputInt(outfile, sourceval);

   /* Step 3: Carry out loop. */
   /* Write code for loop body */
   for (p = s->body; p != NULL; p = p->next)
   {
      numtemps = codegen_statement((stmt_type)p->data, numlocals);
      if (numtemps > our_maxlocal)
         our_maxlocal = numtemps;
   }

   /* Backpatch continue statements in loop body */
   for (p = current_loop->for_continue_list; p != NULL; p = p->next)
      BackpatchGoto(outfile, (int)p->data, FileCurPos(outfile));

   /* Step 4: Execute statements from assign list (iterators) */
   /* If no iteration, list will be NULL. */
   for (p = s->assign; p != NULL; p = p->next)
   {
      assign_stmt = (stmt_type)p->data;
      numtemps = codegen_statement(assign_stmt, numlocals);
      if (numtemps > our_maxlocal)
         our_maxlocal = numtemps;
   }

   /* Goto top of loop is last statement of for loop */
   opcode.source1 = 0;
   opcode.source2 = GOTO_UNCONDITIONAL;
   opcode.dest = 0;
   OutputOpcode(outfile, opcode);
   OutputGotoOffset(outfile, FileCurPos(outfile), toppos);

   codegen_exit_loop();  /* Takes care of break statements */

   return our_maxlocal;
}
/************************************************************************/
/*
 * codegen_foreach: Generate code for a for loop statement.
 *    numlocals should be # of local variables for message excluding temps.
 *   Returns highest # local variable used in code for statement.
 * Here is how code is generated for a FOR statement:
 *  for i in list ===>       temp = list              1
 *  {body}             top:  if (temp = $) goto end   2
 *                           i = First(temp)          3
 *                           { body }
 *                           temp = Rest(temp)        4
 *                           goto top                 5
 *                     end:
 *
 * Note that continue statements need to jump to statement 4.
 */
int codegen_foreach(foreach_stmt_type s, int numlocals)
{
   opcode_type opcode;
   int our_maxlocal, numtemps;
   stmt_type temp_stmt = (stmt_type) SafeMalloc(sizeof(stmt_struct));
   expr_type temp_expr = (expr_type) SafeMalloc(sizeof(expr_struct));
   expr_type temp2_expr = (expr_type) SafeMalloc(sizeof(expr_struct));
   assign_stmt_type assign_stmt = (assign_stmt_type) SafeMalloc(sizeof(assign_stmt_struct));
   call_stmt_type call_stmt = (call_stmt_type) SafeMalloc(sizeof(call_stmt_struct));
   arg_type arg = (arg_type) SafeMalloc(sizeof(arg_struct));
   id_type temp_id, temp2_id;
   long toppos;
   list_type p;

   /* Make variable "temp" */
   temp_id = make_temp_var(numlocals + 1);
   
   /**** Statement #1:   temp = list ****/
   assign_stmt->lhs = temp_id;
   assign_stmt->rhs = s->condition;
   temp_stmt->type = S_ASSIGN;
   temp_stmt->value.assign_stmt_val = assign_stmt;
   temp_stmt->lineno = 0;
   numtemps = codegen_statement(temp_stmt, numlocals);

   /* Reserve variable "temp" through entire loop by incrementing numlocals */
   our_maxlocal = ++numlocals;
   if (numtemps > our_maxlocal)
      our_maxlocal = numtemps;

   toppos = FileCurPos(outfile);
   codegen_enter_loop();

   /**** Statement #2:   if (temp = $) goto end ****/
   /* First put result of temp = $ into temp2 */
   temp2_id = make_temp_var(numlocals + 1);
   if (numlocals + 1 > our_maxlocal)
      our_maxlocal = numlocals + 1;

   temp2_expr->type = E_IDENTIFIER;
   temp2_expr->value.idval = temp_id;

   temp_expr->type = E_BINARY_OP;
   temp_expr->value.binary_opval.op = EQ_OP;
   temp_expr->value.binary_opval.left_exp = temp2_expr;
   temp_expr->value.binary_opval.right_exp = make_expr_from_constant(make_nil_constant());
   
   assign_stmt->lhs = temp2_id;
   assign_stmt->rhs = temp_expr;
   temp_stmt->type = S_ASSIGN;
   temp_stmt->value.assign_stmt_val = assign_stmt;  /* YECHHH! */
   temp_stmt->lineno = 0;
   codegen_statement(temp_stmt, numlocals);  /* Won't require more temps */
   
   /* Now perform jump if temp = $ is true */
   memset(&opcode, 0, sizeof(opcode));  /* Set opcode to all zeros */
   opcode.command = GOTO;
   opcode.source1 = LOCAL_VAR;
   opcode.dest = GOTO_IF_TRUE;
   OutputOpcode(outfile, opcode);

   /* Make believe goto is a break statement & leave space for backpatching */
   current_loop->break_list = 
      list_add_item(current_loop->break_list, (void *) FileCurPos(outfile));
   OutputInt(outfile, 0);
   OutputInt(outfile, temp2_id->idnum);   /* Jump if temp2 = TRUE */ 

   /**** Statement #3:    i = First(temp) ****/
   temp_expr->type = E_IDENTIFIER;
   temp_expr->value.idval = temp_id;
   arg->type = ARG_EXPR;
   arg->value.expr_val = temp_expr;
   call_stmt->function = FIRST;
   call_stmt->args = list_create(arg);
   codegen_call(call_stmt, s->id, numlocals);  /* Won't require more temps */

   /* Write code for loop body */
   for (p = s->body; p != NULL; p = p->next)
   {
      numtemps = codegen_statement( (stmt_type) p->data, numlocals);
      if (numtemps > our_maxlocal)
	 our_maxlocal = numtemps;
   }

   /* Backpatch continue statements in loop body */
   for (p = current_loop->for_continue_list; p != NULL; p = p->next)
      BackpatchGoto(outfile,  (int) p->data, FileCurPos(outfile));

   /**** Statement #4:    temp = Rest(temp) ****/
   /* Can reuse most of statement #3 above */
   call_stmt->function = REST;
   codegen_call(call_stmt, temp_id, numlocals);  /* Won't require more temps */

   /**** Statement #5:    goto top ****/
   opcode.source1 = 0;
   opcode.source2 = GOTO_UNCONDITIONAL;
   opcode.dest = 0;
   OutputOpcode(outfile, opcode);
   OutputGotoOffset(outfile, FileCurPos(outfile), toppos);

   codegen_exit_loop();  /* Takes care of break statements */

   return our_maxlocal;
}
/************************************************************************/
/*
 * codegen_statement: Generate code for a single statement.
 *    numlocals should be # of local variables for message excluding temps.
 *   Returns highest # local variable used in code for statement.
 */
int codegen_statement(stmt_type s, int numlocals)
{
   opcode_type opcode;
   int our_maxtemp = numlocals; /* highest numbered temporary required for this statement alone */

   /* Save line # debugging information */
   if (debug_bof && s->lineno != 0)
   {
      DebugLine *d = (DebugLine *) SafeMalloc(sizeof(DebugLine));
      d->lineno = s->lineno;
      d->offset = FileCurPos(outfile);
      debug_lines = list_add_item(debug_lines, d);
   }
   
   memset(&opcode, 0, sizeof(opcode));  /* Set opcode to all zeros */
   switch (s->type)
   {
   case S_ASSIGN:
   {
      assign_stmt_type stmt = s->value.assign_stmt_val;

      /* Place result directly in lhs */
      our_maxtemp = flatten_expr(stmt->rhs, stmt->lhs, numlocals);
      break;
   }

   case S_CALL:
      our_maxtemp = codegen_call(s->value.call_stmt_val, NULL, numlocals);
      break;

   case S_PROP:
      opcode.command = RETURN;
      opcode.dest = PROPAGATE;
      OutputOpcode(outfile, opcode);
      break;

   case S_RETURN:
      our_maxtemp = codegen_return(s->value.return_stmt_val, numlocals);
      break;

   case S_IF:
      our_maxtemp = codegen_if(s->value.if_stmt_val, numlocals);
      break;

   case S_FOREACH:
      our_maxtemp = codegen_foreach(s->value.foreach_stmt_val, numlocals);
      break;

   case S_FOR:
      our_maxtemp = codegen_for(s->value.for_stmt_val, numlocals);
      break;

   case S_SWITCH:
      our_maxtemp = codegen_switch(s->value.switch_stmt_val, numlocals);
      break;

   case S_WHILE:
      our_maxtemp = codegen_while(s->value.while_stmt_val, numlocals);
      break;

   case S_DOWHILE:
      our_maxtemp = codegen_dowhile(s->value.while_stmt_val, numlocals);
      break;

   case S_BREAK:
      /* Goto end of loop */
      opcode.command = GOTO;
      opcode.source1 = 0;
      opcode.source2 = GOTO_UNCONDITIONAL;
      OutputOpcode(outfile, opcode);
      
      /* Add to list of gotos to be backpatched later, and leave space */
      current_loop->break_list = 
	 list_add_item(current_loop->break_list, (void *) FileCurPos(outfile));
      OutputInt(outfile, 0);
      break;

   case S_CONTINUE:
      /* Goto top of loop */
      opcode.command = GOTO;
      opcode.source1 = 0;
      opcode.source2 = GOTO_UNCONDITIONAL;
      OutputOpcode(outfile, opcode);

      /* In for loops, continue statements actually jump forward, but in while loops
       * they jump backward.  Save address of goto for backpatching; if we are
       * in a for loop, the offset written out below will be written over during
       * backpatching in codegen_foreach().
       */
      current_loop->for_continue_list = 
	 list_add_item(current_loop->for_continue_list, (void *) FileCurPos(outfile));

      OutputGotoOffset(outfile, FileCurPos(outfile), current_loop->toppos);
      break;

   default:
      simple_error("Unknown statement type (%d) encountered", s->type);
      break;
   }
   return our_maxtemp;
}
/************************************************************************/
/*
 * codegen_parameter: Generate code for a single parameter to a message handler.
 */
void codegen_parameter(param_type p)
{
   /* Write out parameter's id # */
   OutputInt(outfile, p->lhs->idnum);

   /* Write out parameter's default value */
   OutputConstant(outfile, p->rhs);   
}
/************************************************************************/
/*
 * codegen_classvar: Generate code for a single classvar.
 */
void codegen_classvar(classvar_type c)
{
   /* Write out # of classvar */
   OutputInt(outfile, c->id->idnum);

   /* Write out value of classvar */
   OutputConstant(outfile, c->rhs);
}
/************************************************************************/
/*
 * codegen_property: Generate code for a single property.
 */
void codegen_property(property_type p)
{
   /* Write out # of property */
   OutputInt(outfile, p->id->idnum);

   /* Write out value of property */
   OutputConstant(outfile, p->rhs);
}
/************************************************************************/
/*
 * codegen_message: Generate code for a single message handler.
 */
void codegen_message(message_handler_type m)
{
   int numlocals, maxtemp, maxlocals;
   list_type s, p = m->header->params;
   long localpos;

   /* Leave space for # of local variables */
   localpos = FileCurPos(outfile); 
   OutputByte(outfile, (BYTE) 0);
   
   /* Write out # of arguments */
   OutputByte(outfile,  (BYTE) list_length(p));
   
   /* Write out arguments themselves */
      for ( ; p != NULL; p = p->next)
	 codegen_parameter( (param_type) p->data);

   /* # of local variables, including parameters.  -1 is to start at 0. */
   numlocals = list_length(m->locals) + list_length(m->header->params) - 1;
   maxlocals = numlocals;

   /* Write out code */
   for (s = m->body; s != NULL; s = s->next)
   {
      maxtemp = codegen_statement( (stmt_type) s->data, numlocals);
      if (maxtemp > maxlocals)
	 maxlocals = maxtemp;

      /* Bomb out if statement had unforseen errors */
      if (!codegen_ok)
	 return;
   }

   /* Backpatch in # of local variables */
   if (maxlocals > MAX_LOCALS)
      codegen_error("More than %d local variables in handler %s.", MAX_LOCALS, 
		    m->header->message_id->name);

   FileGoto(outfile, localpos);
   OutputByte(outfile,  (BYTE) (maxlocals + 1));  /* +1 because we start counting at 0 */
   FileGotoEnd(outfile);
}
/************************************************************************/
/*
 * codegen_class: Generate code for a single class.
 */
void codegen_class(class_type c)
{
   list_type p, m, cv;
   long temppos, endpos, messagepos, propertypos;
   int nummessages, i;
   
   /* Write out superclass class id */
   if (c->superclass == NULL)
      OutputInt(outfile, NO_SUPERCLASS);
   else OutputInt(outfile, c->superclass->class_id->idnum);

   /* Save current file position for backpatching */
   propertypos = FileCurPos(outfile);
   OutputInt(outfile, 0);   /* Set aside space for property table offset */
   temppos = FileCurPos(outfile);
   OutputInt(outfile, 0);   /* Set aside space for message dispatch table offset */

   /****** Classvar table ******/
   // TOTAL # of classvars for this class
   OutputInt(outfile, c->numclassvars);
   // # of classvars for which we have default values
   OutputInt(outfile, list_length(c->classvars));
	  
   /* Write out values of classvars with default values */
   for (cv = c->classvars; cv != NULL; cv = cv->next)
      codegen_classvar( (classvar_type) cv->data);


   /* Go back and fill in position of property table */
   endpos = FileCurPos(outfile);
   FileGoto(outfile, propertypos);
   OutputInt(outfile, endpos);
   FileGotoEnd(outfile);

   /****** Property table ******/
   // TOTAL # of properties for this class
   OutputInt(outfile, c->numproperties);    
   // # of properties for which we have default values
   OutputInt(outfile, list_length(c->properties));    
	  
   /* Write out values of properties with default values */
   for (p = c->properties; p != NULL; p = p->next)
      codegen_property( (property_type) p->data);


   /* Go back and fill in position of message dispatch table */
   endpos = FileCurPos(outfile);
   FileGoto(outfile, temppos);
   OutputInt(outfile, endpos);
   FileGotoEnd(outfile);

   /* Write out # of message handlers */
   nummessages = list_length(c->messages);
   OutputInt(outfile, nummessages);

   /* Save away position of message dispatch table */
   messagepos = FileCurPos(outfile);

   /* Leave space for each message's entry in dispatch table */
   for (i=0; i < nummessages; i++)
   {
      /* Leave space for each message id # */
      OutputInt(outfile, 0);
      /* Leave space for each message handler's offset */
      OutputInt(outfile, 0);
      /* Leave space for each message handler's comment string */
      OutputInt(outfile, 0);
   }

   /* Now spew code for each message */
   for (m = c->messages; m != NULL; m = m->next)
   {
      message_handler_type handler = (message_handler_type) m->data;

      /* Go back to dispatch table & fill in entry */
      endpos = FileCurPos(outfile);
      FileGoto(outfile, messagepos);

      /* Write out message id #, then handler offset */
      OutputInt(outfile, handler->header->message_id->idnum);
      OutputInt(outfile, endpos); 
      if (handler->comment == NULL)
	 OutputInt(outfile, -1);
      else OutputInt(outfile, handler->comment->value.numval);

      messagepos = FileCurPos(outfile);  /* Move to next entry */
      FileGotoEnd(outfile);
      
      codegen_message(handler);

      if (!codegen_ok)
	 break;
   }
}
/************************************************************************/
/* 
 * codegen: Generate code for all the classes in the symbol table.
 */
void codegen(char *kod_fname, char *bof_fname)
{
   list_type c = NULL;
   long endpos, stringpos, debugpos, namepos;

   codegen_ok = True;
   debug_lines = NULL;

   outfile = open(bof_fname, O_TRUNC | O_CREAT | O_RDWR | O_BINARY, S_IWRITE | S_IREAD);

   if (outfile == -1)
   {
      simple_error("Unable to open bof file %s!", bof_fname);
      return;
   }

   /* Write out header info */
   codegen_header();

   /* Remember position for backpatching location of kod filename, and leave room */
   namepos = FileCurPos(outfile);
   OutputInt(outfile, 0);

   /* Remember position for backpatching location of string table, and leave room */
   stringpos = FileCurPos(outfile);
   OutputInt(outfile, 0);

   /* Remember position for backpatching location of debugging info, and leave room */
   debugpos = FileCurPos(outfile);
   OutputInt(outfile, 0);
   
   codegen_classes();
   
   if (codegen_ok)
   {
      /* Backpatch location of string table */
      endpos = FileCurPos(outfile);
      FileGoto(outfile, stringpos); 
      OutputInt(outfile, endpos);

      FileGotoEnd(outfile);
      codegen_string_table();

      /* Backpatch location of debug info */
      endpos = FileCurPos(outfile);
      FileGoto(outfile, debugpos); 
      if (debug_bof)
         OutputInt(outfile, endpos);
      else OutputInt(outfile, 0);

      FileGotoEnd(outfile);
      if (debug_bof)
         codegen_debug_info();

      /* Backpatch location of kod filename */
      endpos = FileCurPos(outfile);
      FileGoto(outfile, namepos); 
      OutputInt(outfile, endpos);

      FileGotoEnd(outfile);
      codegen_filename(kod_fname);
   }

   close(outfile);

   /* If code generation failed, delete partial bof file */
   if (!codegen_ok)
   {
      if (unlink(bof_fname))
	 codegen_error("Couldn't delete file %s", bof_fname);
      else simple_warning("Deleted file %s", bof_fname);
   }

   /* Write out resources & new database if we compiled ok */
   if (codegen_ok)
   {
      char temp[256];
      set_extension(temp, bof_fname, ".rsc");
      write_resources(temp);
      save_kodbase();
   }

   /* Mark all classes as done */
   for (c = st.classes; c != NULL; c = c->next)
      ((class_type) (c->data))->is_new = False;
}
