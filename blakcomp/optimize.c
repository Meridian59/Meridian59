// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * optimize.c:  Perform optimizations (currently just constant folding).
 */

#include "blakcomp.h"

/************************************************************************/
/*
 * SimplifyExpression:  Attempt to simplify given expression.  Modifies e.
 */
void SimplifyExpression(expr_type e)
{
   expr_type e1, e2;
   const_type c1, c2;

   switch (e->type)
   {
   case E_UNARY_OP:
      e1 = e->value.unary_opval.exp;
      if (e1->type != E_CONSTANT)
    break;

      c1 = e1->value.constval;
      if (c1->type != C_NUMBER)
         break;

      switch (e->value.unary_opval.op)
      {
      case NEG_OP:
         c1->value.numval = -c1->value.numval;
         e->type = E_CONSTANT;
         e->value.constval = c1;
         break;

      case NOT_OP:
         c1->value.numval = !c1->value.numval;
         e->type = E_CONSTANT;
         e->value.constval = c1;
         break;

      case BITNOT_OP:
         c1->value.numval = ~c1->value.numval;
         e->type = E_CONSTANT;
         e->value.constval = c1;
         break;

      case PRE_INC_OP:
         c1->value.numval = ++c1->value.numval;
         e->type = E_CONSTANT;
         e->value.constval = c1;
         break;

      case PRE_DEC_OP:
         c1->value.numval = --c1->value.numval;
         e->type = E_CONSTANT;
         e->value.constval = c1;
         break;
      }
      break;

   case E_BINARY_OP:
      e1 = e->value.binary_opval.left_exp;
      e2 = e->value.binary_opval.right_exp;
      if (e1->type != E_CONSTANT || e2->type != E_CONSTANT)
         break;

      c1 = e1->value.constval;
      c2 = e2->value.constval;

      if (c1->type != C_NUMBER || c2->type != C_NUMBER)
         break;

      switch (e->value.binary_opval.op)
      {
      case PLUS_OP:
         c1->value.numval = c1->value.numval + c2->value.numval;
         e->type = E_CONSTANT;
         e->value.constval = c1;
         break;

      case MINUS_OP:
         c1->value.numval = c1->value.numval - c2->value.numval;
         e->type = E_CONSTANT;
         e->value.constval = c1;
         break;

      case AND_OP:
         c1->value.numval = c1->value.numval && c2->value.numval;
         e->type = E_CONSTANT;
         e->value.constval = c1;
         break;

      case OR_OP:
         c1->value.numval = c1->value.numval || c2->value.numval;
         e->type = E_CONSTANT;
         e->value.constval = c1;
         break;

      case MULT_OP:
         c1->value.numval = c1->value.numval * c2->value.numval;
         e->type = E_CONSTANT;
         e->value.constval = c1;
         break;

      case DIV_OP:
         if (c2->value.numval == 0)
         {
            action_error("Division by zero");
            break;
         }
         c1->value.numval = c1->value.numval / c2->value.numval;
         e->type = E_CONSTANT;
         e->value.constval = c1;
         break;

      case MOD_OP:
         if (c2->value.numval == 0)
         {
            action_error("Division by zero");
            break;
         }
         c1->value.numval = c1->value.numval % c2->value.numval;
         e->type = E_CONSTANT;
         e->value.constval = c1;
         break;

      case BITAND_OP:
         c1->value.numval = c1->value.numval & c2->value.numval;
         e->type = E_CONSTANT;
         e->value.constval = c1;
         break;

      case BITOR_OP:
         c1->value.numval = c1->value.numval | c2->value.numval;
         e->type = E_CONSTANT;
         e->value.constval = c1;
         break;
      }
      break;
   }
}
