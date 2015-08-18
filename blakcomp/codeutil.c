// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* codeutil.c  Subsidiary procedures for use by codegen.c */

#include "blakcomp.h"
#include "bkod.h"
#include "codegen.h"

// file opened in codegen.c -- ugly that it has the same name as a parameter
// to many functions in this file.
extern int outfile;

/************************************************************************/
/* 
 * codegen_error: Print an error message during code generation
 */
void codegen_error(const char *fmt, ...)
{
   va_list marker;

   printf("error: ");

   va_start(marker, fmt);
   vprintf(fmt, marker);
   va_end(marker);
   printf("\nAborting.\n");

   codegen_ok = False;
}
/************************************************************************/
void OutputOpcode(int outfile, opcode_type opcode)
{
   BYTE datum;

   /* Write out a 1 byte opcode--need to memcpy since opcode is bitfield structure */
   memcpy(&datum, &opcode, 1);
   write(outfile, &datum, 1);
}
/************************************************************************/
void OutputByte(int outfile, BYTE datum)
{
   /* Write out a 1 byte # */
   write(outfile, &datum, sizeof(datum));
}
/************************************************************************/
void OutputInt(int outfile, int datum)
{
   /* Write out a 4 byte # */
   write(outfile, &datum, sizeof(datum)); 
}
/************************************************************************/
/*
 * OutputGotoOffset:  Write out jump offset from goto instruction (source)
 *    to destination.  An extra +1 is added to the offset because jump offsets
 *    are supposed to be from the BEGINNING of the goto statement, but the
 *    source byte is actually one byte into the instruction (right after
 *    the opcode).
 */  
void OutputGotoOffset(int outfile, int source, int destination)
{
   OutputInt(outfile, destination - source + 1);
}
/************************************************************************/
/*
 * const_to_int:  Convert a compiler constant into an object code constant,
 *    and then change this result to an integer and return it.
 */
int const_to_int(const_type c)
{
   constant_type code_const; /* Stuff to be written out */
   int result;

   switch(c->type)
   {
   case C_NUMBER:
      code_const.tag = TAG_INT;
      code_const.data = c->value.numval;
      break;

   case C_NIL:
      code_const.tag = TAG_NIL;
      /* Value is ignored, but put 0 anyway */
      code_const.data = 0;
      break;

   case C_RESOURCE:
      code_const.tag = TAG_RESOURCE;
      code_const.data = c->value.numval;
      break;

   case C_CLASS:
      code_const.tag = TAG_CLASS;
      code_const.data = c->value.numval;
      break;

   case C_MESSAGE:
      code_const.tag = TAG_MESSAGE;
      code_const.data = c->value.numval;
      break;

   case C_STRING:    // Added 8/2/95 for kod debugging strings ARK
      code_const.tag = TAG_DEBUGSTR;
      code_const.data = c->value.numval;
      break;

   case C_OVERRIDE:  // Added 2/16/96 for overriding class variables with properties ARK
      code_const.tag = TAG_OVERRIDE;
      code_const.data = c->value.numval;
      break;

   default:
      simple_error("Unknown constant type (%d) encountered", c->type);
      break;
   }
   
   memcpy(&result, &code_const, sizeof(code_const));
   return result;
}
/************************************************************************/
/*
 * OutputConstant: write out a constant, after adding the appropriate
 *   tag bits to the beginning of the number.  The tag bits indicate the
 *   type of the constant, and are given in bkod.h.
 */
void OutputConstant(int outfile, const_type c)
{
   int outnum = const_to_int(c);
   write(outfile, &outnum, sizeof(outnum));
}
/************************************************************************/
/*
 * OutputBaseExpression:  Given a base expression, first write out 1 byte
 *    for its type, and then 4 bytes for its value
 */
void OutputBaseExpression(int outfile, expr_type expr)
{
   id_type id;

   switch(expr->type)
   {
   case E_IDENTIFIER:
      id = expr->value.idval;
      switch (id->type)
      {
      case I_PROPERTY:
	 OutputByte(outfile, (BYTE) PROPERTY);
	 break;
	 
      case I_LOCAL:
	 OutputByte(outfile, (BYTE) LOCAL_VAR);
	 break;

      case I_CLASSVAR:
	 OutputByte(outfile, (BYTE) CLASS_VAR);
	 break;

      default:
	 codegen_error("Bad variable type on %s in OutputBaseExpression", id->name);
      }
      OutputInt(outfile, id->idnum);
      break;

   case E_CONSTANT:
      OutputByte(outfile, (BYTE) CONSTANT);
      OutputConstant(outfile, expr->value.constval);
      break;

   default:
      codegen_error("Bad expression type (%d) in OutputBaseExpression", expr->type);
   }
}
/************************************************************************/
/*
 * BackpatchGoto: Go back to the spot "source" in the file, and write out
 *   the offset required to jump to "destination".  Then return to 
 *   "destination" in the file.
 */
void BackpatchGoto(int outfile, int source, int destination)
{
   FileGoto(outfile, source);
   OutputGotoOffset(outfile, source, destination);
   FileGoto(outfile, destination);
}
/************************************************************************/
/*
 * set_source_id:  Set opcode source field to LOCAL_VAR, PROPERTY, CONSTANT,
 *    or CLASS_VAR, depending on type of given expression.  
 *    (The expression must be one of these types).  
 *    An integer is returned, meaning:
 *      The id # of the local variable or property, or
 *      The contant value (from const_to_int), 
 *    depending on the type of expression.
 *    Sourcenum must be either SOURCE1 or SOURCE2; the corresponding field 
 *    of opcode is set.
 */
int set_source_id(opcode_type *opcode, int sourcenum, expr_type e)
{
   id_type id;
   int temp, retval;

   switch(e->type)
   {
   case E_CONSTANT:
      temp = CONSTANT;
      retval = const_to_int(e->value.constval);
      break;
      
   case E_IDENTIFIER:
      id = e->value.idval;
      retval = id->idnum;
      switch (id->type)
      {
      case I_LOCAL:
	 temp = LOCAL_VAR;
	 break;
	 
      case I_PROPERTY:
	 temp = PROPERTY;
	 break;

      case I_CLASSVAR:
	 temp = CLASS_VAR;
	 break;

      default:
	 codegen_error("Identifier in expression not a local or property: %s", 
		       id->name);
      }
      break;

   default:
      codegen_error("Bad expression type (%d) in set_source_id", e->type); 
      
   }

   if (sourcenum == SOURCE1)
      opcode->source1 = temp;
   else if (sourcenum == SOURCE2)
      opcode->source2 = temp;
   else
      codegen_error("Illegal sourcenum value in set_source_id");

   return retval;
}
/************************************************************************/
/*
 * set_dest_id: Set opcode destination field to local var or property,
 *   depending on type of given id.
 *   Returns id # of given id.
 */
int set_dest_id(opcode_type *opcode, id_type id)
{
   switch (id->type)
   {
   case I_LOCAL:
      opcode->dest = LOCAL_VAR;
      break;
      
   case I_PROPERTY:
      opcode->dest = PROPERTY;
      break;
      
   default:
      codegen_error("Identifier in expression not a local or property: %s", 
		    id->name);
   }
   return id->idnum;
}
/************************************************************************/
/* 
 * is_base_level: returns True iff e is a base-level expression; i.e. if
 *     it is a leaf of an expression tree. 
 */
int is_base_level(expr_type e)
{
   return e->type == E_IDENTIFIER || e->type == E_CONSTANT;
}
/************************************************************************/
/*
 * make_temp_var: Create & return an id for a local variable with the given id #.
 */
id_type make_temp_var(int idnum)
{
   id_type id = (id_type) SafeMalloc(sizeof(id_struct));
   id->type = I_LOCAL;
   id->idnum = idnum;
   return id;
}
/************************************************************************/
/* 
 * flatten_expr: Generate code to produce the result of the given expression
 *   into the given variable.
 *   maxlocal should be the highest currently used local variable.
 *   Returns highest # local variable used in evaluating expression.
 */
int flatten_expr(expr_type e, id_type destvar, int maxlocal)
{
   opcode_type opcode, sc_opcode;
   expr_type tempexpr;
   int sourceval1, sourceval2, destval, our_maxlocal = maxlocal, templocals;
   int op, gotopos, exitpos;

   memset(&opcode, 0, sizeof(opcode));  /* Set opcode to all zeros */
   memset(&sc_opcode, 0, sizeof(sc_opcode));  /* Set opcode to all zeros */
   destval = set_dest_id(&opcode, destvar);

   switch (e->type)
   {
   case E_CONSTANT:
      opcode.command = UNARY_ASSIGN;
      opcode.source1 = CONSTANT;
      
      /* Opcode is <local var> = <constant> */
      OutputOpcode(outfile, opcode);

      /* Operation is plain assignment */
      OutputByte(outfile,  (BYTE) NONE);
      
      /* Destination is var #destval */
      OutputInt(outfile, destval);

      /* Source is the constant itself */
      OutputConstant(outfile, e->value.constval);

      return our_maxlocal;
      break;

   case E_IDENTIFIER:
      opcode.command = UNARY_ASSIGN;
      set_source_id(&opcode, SOURCE1, e);
      
      /* Opcode is <local var> = <identifier> */
      OutputOpcode(outfile, opcode);

      /* Operation is plain assignment */
      OutputByte(outfile,  (BYTE) NONE);
      
      /* Destination is local var #destlocal */
      OutputInt(outfile, destval);

      /* Source is the id # of the identifier */
      OutputInt(outfile, e->value.idval->idnum);

      break;

   case E_UNARY_OP:
      opcode.command = UNARY_ASSIGN;
      
      /* If operand is simple, compute result directly, else store in temp */
      tempexpr = e->value.unary_opval.exp;
      if (is_base_level(tempexpr))
	 sourceval1 = set_source_id(&opcode, SOURCE1, tempexpr);
      else
      {
	 /* Evaluate rhs, store in destlocal, then perform operation */
	 our_maxlocal = flatten_expr(tempexpr, destvar, our_maxlocal);

	 /* Source is same as destination; perform op in place */
	 opcode.source1 = opcode.dest;
	 sourceval1 = destvar->idnum; 
      }
      
      OutputOpcode(outfile, opcode);
      /* Write out operation type */
      switch(e->value.unary_opval.op)
      {
      case NEG_OP:      OutputByte(outfile,  (BYTE) NEGATE);         break;
      case NOT_OP:      OutputByte(outfile,  (BYTE) NOT);            break;
      case BITNOT_OP:   OutputByte(outfile,  (BYTE) BITWISE_NOT);    break;
      case PRE_INC_OP:  OutputByte(outfile,  (BYTE) PRE_INCREMENT);  break;
      case PRE_DEC_OP:  OutputByte(outfile,  (BYTE) PRE_DECREMENT);  break;
      case POST_INC_OP: OutputByte(outfile,  (BYTE) POST_INCREMENT); break;
      case POST_DEC_OP: OutputByte(outfile,  (BYTE) POST_DECREMENT); break;

      default:
	 codegen_error("Unknown unary operator type (%d) encountered", 
		       e->value.unary_opval.op);
      }      
      OutputInt(outfile, destval);       /* Destination is local var #destlocal */
      OutputInt(outfile, sourceval1);       

      break;

   case E_BINARY_OP:
      opcode.command = BINARY_ASSIGN;
      
      tempexpr = e->value.binary_opval.left_exp;
      op = e->value.binary_opval.op;
      templocals = maxlocal;  /* Holds max # used in one subexpression (left or right) */

      if (is_base_level(tempexpr))
	 sourceval1 = set_source_id(&opcode, SOURCE1, tempexpr);
      else
      {
	 opcode.source1 = LOCAL_VAR;

	 /* Evaluate rhs, store in temporary, and assign it to destvar */
	 our_maxlocal++;
	 templocals++;  /* Lhs must be stored in temp; rhs must not use this temp */
	 sourceval1 = our_maxlocal;
	 our_maxlocal = flatten_expr(tempexpr, make_temp_var(our_maxlocal), our_maxlocal);
      }

      exitpos = 0;
      /* Check for short circuiting of AND and OR */
      if (op == AND_OP || op == OR_OP)
      {
         /* If must evaluate rhs, create jump over short circuit code */
         sc_opcode.command = GOTO;
         sc_opcode.dest = (op == AND_OP) ? GOTO_IF_TRUE : GOTO_IF_FALSE ;
         sc_opcode.source1 = opcode.source1;  /* Check lhs of binary expression */
         
         OutputOpcode(outfile, sc_opcode);
         gotopos = FileCurPos(outfile);
         OutputInt(outfile, 0);    /* Leave room for destination address */
         OutputInt(outfile, sourceval1);  /* Same as lhs source value above */
         
         /* Short-circuit code: value of expression is known to be true or false */
         flatten_expr(make_expr_from_constant(make_numeric_constant( (op == AND_OP)
                                                                     ? 0 : 1)), 
                      destvar, maxlocal);
         
         /* Now jump to end of expression evaluation */
         sc_opcode.source1 = 0;
         sc_opcode.source2 = GOTO_UNCONDITIONAL;
         sc_opcode.dest = 0;
         OutputOpcode(outfile, sc_opcode);
         exitpos = FileCurPos(outfile);
         OutputInt(outfile, 0);    /* Leave room for destination */
         
         /* Backpatch in goto that skipped short circuit code */
         BackpatchGoto(outfile, gotopos, FileCurPos(outfile));
      }
      
      tempexpr = e->value.binary_opval.right_exp;
      if (is_base_level(tempexpr))
         sourceval2 = set_source_id(&opcode, SOURCE2, tempexpr);
      else
      {
         opcode.source2 = LOCAL_VAR;
         
         /* Evaluate rhs, store in temporary, and assign it to destvar */
         templocals++;
         sourceval2 = templocals;
         templocals = flatten_expr(tempexpr, make_temp_var(templocals), templocals);
      }
      
      OutputOpcode(outfile, opcode);
      /* Write out operation type */
      switch(op)
      {
      case AND_OP:    OutputByte(outfile,  (BYTE) AND);            break;
      case OR_OP:     OutputByte(outfile,  (BYTE) OR);             break;
      case PLUS_OP:   OutputByte(outfile,  (BYTE) ADD);            break;
      case MINUS_OP:  OutputByte(outfile,  (BYTE) SUBTRACT);       break;
      case MULT_OP:   OutputByte(outfile,  (BYTE) MULTIPLY);       break;
      case DIV_OP:    OutputByte(outfile,  (BYTE) DIV);            break;
      case MOD_OP:    OutputByte(outfile,  (BYTE) MOD);            break;
      case NEQ_OP:    OutputByte(outfile,  (BYTE) NOT_EQUAL);      break;
      case EQ_OP:     OutputByte(outfile,  (BYTE) EQUAL);          break;
      case LT_OP:     OutputByte(outfile,  (BYTE) LESS_THAN);      break;
      case GT_OP:     OutputByte(outfile,  (BYTE) GREATER_THAN);   break;
      case GEQ_OP:    OutputByte(outfile,  (BYTE) GREATER_EQUAL);  break;
      case LEQ_OP:    OutputByte(outfile,  (BYTE) LESS_EQUAL);     break;
      case BITAND_OP: OutputByte(outfile,  (BYTE) BITWISE_AND);    break;
      case BITOR_OP:  OutputByte(outfile,  (BYTE) BITWISE_OR);     break;

      default:
	 codegen_error("Unknown unary operator type (%d) encountered", op);
      }      
      OutputInt(outfile, destval);         /* Destination is local var #destlocal */
      OutputInt(outfile, sourceval1);      /* Source is variable id # or constant */
      OutputInt(outfile, sourceval2);
      
      /* If there was short circuit code, fill in the short-circuiting goto */
      if (exitpos != 0)
	 BackpatchGoto(outfile, exitpos, FileCurPos(outfile));
      
      /* See which branch used more temps */
      if (templocals > our_maxlocal)
	 our_maxlocal = templocals;
      break;

   case E_CALL:
      /* Place return value of call into destination variable */
      our_maxlocal = codegen_call( ((stmt_type) (e->value.callval))->value.call_stmt_val, 
				  destvar, our_maxlocal);
      break;
      
   default:
      codegen_error("Unknown expression type (%d) encountered", e->type);
   }

   return our_maxlocal;
}
/************************************************************************/
/*  
 * simplify_expr: Generate code to simplify the given expression 
 *    until it is an id or a constant.  
 *    If the expression is complicated, this involves
 *    evaluating it into a temporary variable, and then changing the
 *    expression to just evaluate to the temporary variable.
 *   Returns highest # local variable used in code for expression.
 */
int simplify_expr(expr_type expr, int maxlocal)
{
   int our_maxlocal = maxlocal;
   id_type id;

   if (is_base_level(expr))
      return our_maxlocal;
   
   id = make_temp_var(maxlocal + 1); /* Temporary to store expression */
   
   our_maxlocal = flatten_expr(expr, id, maxlocal);
   
   /* Count temp var, if rhs didn't use any new temps */
   if (our_maxlocal == maxlocal)
      our_maxlocal++;
   
   /* Modify expression to make it just a local variable */
   expr->value.idval = id;
   expr->type = E_IDENTIFIER;
      
   return our_maxlocal;
}
