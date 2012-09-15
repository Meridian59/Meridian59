// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * cmove.c:  Handle the rules of chess.
 */

#include "client.h"
#include "chess.h"

#define SGN(x) ((x) == 0 ? 0 : ((x) > 0 ? 1 : -1))

static Bool PathIsClear(Board *b, POINT p1, POINT p2, BYTE color);
static Bool SquareIsAttacked(Board *b, POINT p, BYTE color);
static Bool PieceCanMove(Board *b, POINT p1, POINT p2, BYTE color);
static void PieceMove(Board *b, POINT p1, POINT p2);
static Bool IsLegalCastle(Board *b, POINT p1, POINT p2, BYTE color);
/****************************************************************************/
/*
 * ChessMoveIsLegal:  Return True iff the move from p1 to p2 is legal for the player
 *   of the given color.
 *   Assumes that p1 contains a piece of the player's color.
 */
Bool ChessMoveIsLegal(Board *b, POINT p1, POINT p2, BYTE color)
{
   Board new_board;
   
   if (!PieceCanMove(b, p1, p2, color))
      return False;

   // Move must not leave king in check -- see if king attacked after move performed
   memcpy(&new_board, b, sizeof(Board));
   ChessMovePerform(&new_board, p1, p2, False);

   if (IsInCheck(&new_board, color))
      return False;
   return True;
}
/****************************************************************************/
/*
 * PieceCanMove:  Return True iff the piece at p1 can move to p2.
 *   Assumes that p1 contains a piece of the player's color.
 */
Bool PieceCanMove(Board *b, POINT p1, POINT p2, BYTE color)
{
   BYTE piece;
   Square *s2;
   int direction, dy;
   
   if (p1.x == p2.x && p1.y == p2.y)
      return False;

   piece = b->squares[p1.y][p1.x].piece;
   s2 = &b->squares[p2.y][p2.x];
   
   // First verify that piece can move this way
   switch (piece)
   {
   case PAWN:
      direction = (color == WHITE) ? 1 : -1;
      dy = p2.y - p1.y;
      
      // Make sure moving in the correct direction
      if (dy == 0 || abs(dy) > 2 || SGN(dy) != direction)
	 return False;
      
      // 3 cases:  capturing diagonally, moving straight ahead, or illegal move
      switch (abs(p2.x - p1.x))
      {
      case 0:
	 // If moving straight, can't capture
	 if (s2->piece != NONE)
	    return False;
	 break;
      case 1:
	 // Look for en passant capture
	 if (b->en_passant)
	 {
	    if (b->squares[p2.y - direction][p2.x].piece == PAWN &&
		b->squares[p2.y - direction][p2.x].color != color &&
		p2.x == b->passant_square.x &&
		p2.y == b->passant_square.y)
	       break;
	 }
	 
	 // If moving diagonally, must capture
	 if (s2->piece == NONE || s2->color == color)
	    return False;
	 break;
      default:
	 return False;
      }

      // Can only move 2 squares forward on first move
      if (abs(dy) == 2)
	 if ((color == WHITE && p1.y != 1) ||
	     (color == BLACK && p1.y != 6))
	    return False;
      break;
   case ROOK:
      if (p1.x != p2.x && p1.y != p2.y)
	 return False;
      break;
   case KNIGHT:
      if (abs(p2.x - p1.x) + abs(p2.y - p1.y) != 3)
	 return False;

      if (abs(p2.x - p1.x) == 3 || abs(p2.y - p1.y) == 3)
	 return False;
      break;
   case BISHOP:
      if (abs(p2.x - p1.x) != abs(p2.y - p1.y))
	 return False;
      break;
   case QUEEN:
      if (p1.x != p2.x && p1.y != p2.y && abs(p2.x - p1.x) != abs(p2.y - p1.y))
	 return False;
      break;
   case KING:
      if (IsLegalCastle(b, p1, p2, color))
	 break;

      if (abs(p2.x - p1.x) > 1 || abs(p2.y - p1.y) > 1)
	 return False;
      break;
   default:
      debug(("ChessMoveIsLegal got unknown piece type %d\n", piece));
      return False;
   }

   // Can't move onto same color
   if (s2->piece != NONE && s2->color == color)
      return False;
      
   // Check path to make sure no pieces are in the way (except for knights)
   if (piece != KNIGHT)
      if (!PathIsClear(b, p1, p2, color))
	 return False;
   return True;
}

/****************************************************************************/
/*
 * ChessMovePerform:  Perform the move from p1 to p2 on the given board.  Assumes
 *   that the move is legal.
 *   If interactive is True, ask user for piece on pawn promotion if necessary.
 */
void ChessMovePerform(Board *b, POINT p1, POINT p2, Bool interactive)
{
   BYTE piece;
   POINT p3, p4;
   int direction;

   piece = b->squares[p1.y][p1.x].piece;
   if (piece == PAWN && interactive &&
       ((b->move_color == WHITE && p2.y == BOARD_HEIGHT - 1) ||
	(b->move_color == BLACK && p2.y == 0)))
   {
      // Ask for pawn promotion piece
      piece = ChessGetPromotionPiece();
   }

   b->squares[p1.y][p1.x].piece = piece;  // In case piece changed due to promotion
   PieceMove(b, p1, p2);
   
   // If a castling move, move the rook also (assumes move is legal; rook must be in place)
   if (piece == KING && abs(p2.x - p1.x) > 1)
   {
      // Move rook at p3 to new location at p4
      p3.y = p2.y;
      p4.y = p2.y;
      if (p2.x == 1)
      {
	 p3.x = 0;
	 p4.x = 2;
      }
      else
      {
	 p3.x = 7;
	 p4.x = 4;
      }
      PieceMove(b, p3, p4);
   }

   // Check for en passant capture
   if (piece == PAWN && b->en_passant &&
       p2.x == b->passant_square.x && p2.y == b->passant_square.y)
   {
      direction = (b->move_color == WHITE) ? 1 : -1;
      b->squares[p2.y - direction][p2.x].piece = NONE;
   }

   
   // Set castling flags if king or rook moved
   if (piece == KING)
   {
      b->pdata[b->move_color].can_castle_left  = False;
      b->pdata[b->move_color].can_castle_right = False;
   }
   else if (piece == ROOK)
   {
      if (((b->move_color == WHITE && p1.y == 0) || (b->move_color == BLACK && p1.y == 7)))
	 if (p1.x == 0)
	    b->pdata[b->move_color].can_castle_left = False;
	 else if (p1.x == 7)
	    b->pdata[b->move_color].can_castle_right = False;
   }

   // Set en passant information
   if (piece == PAWN && abs(p2.y - p1.y) > 1) 
   {
      b->en_passant = True;
      b->passant_square.x = p1.x;
      b->passant_square.y = (p1.y + p2.y) / 2;
   }
   else b->en_passant = False;
   
   if (b->move_color == WHITE)
      b->move_color = BLACK;
   else b->move_color = WHITE;
}
/****************************************************************************/
/*
 * PieceMove:  Move piece at p1 to p2.  Does no error checking.
 */
void PieceMove(Board *b, POINT p1, POINT p2)
{
   b->squares[p2.y][p2.x].color = b->squares[p1.y][p1.x].color;
   b->squares[p2.y][p2.x].piece = b->squares[p1.y][p1.x].piece;
   
   b->squares[p1.y][p1.x].piece = NONE;
}

/****************************************************************************/
/*
 * PathIsClear:  Return True iff the path from p1 to p2 is clear, with these
 *   2 exceptions:
 *    1) p1 may contain a piece
 *    2) p2 may contain a piece NOT of the given color
 *   Assumes that p1 and p2 are connected by a horizontal, vertical, or diagonal
 *   line.
 */
Bool PathIsClear(Board *b, POINT p1, POINT p2, BYTE color)
{
   int dx, dy;
   int x, y;

   dx = SGN(p2.x - p1.x);
   dy = SGN(p2.y - p1.y);

   // Start at first square piece enters
   x = p1.x + dx;
   y = p1.y + dy;
   
   while (x != p2.x || y != p2.y)
   {
      if (b->squares[y][x].piece != NONE)
	 return False;
      x += dx;
      y += dy;
   }

   // p2 can't contain piece of same color
   if (b->squares[y][x].piece != NONE && b->squares[y][x].color == color)
      return False;
   
   return True;
}
/****************************************************************************/
/*
 * SquareIsAttacked:  Return True iff the given square is attacked by a piece
 *   of the given color.
 */
Bool SquareIsAttacked(Board *b, POINT p, BYTE color)
{
   int i, j;
   POINT p1;

   for (i=0; i < BOARD_HEIGHT; i++)
      for (j=0; j < BOARD_WIDTH; j++)
	 if (b->squares[i][j].piece != NONE &&
	     b->squares[i][j].color == color)
	 {
	    p1.x = j;
	    p1.y = i;
	    if (PieceCanMove(b, p1, p, color))
	       return True;
	 }
   
   return False;
}
/****************************************************************************/
/*
 * IsInCheck:  Return True iff player of given color is in check on given board.
 */
Bool IsInCheck(Board *b, BYTE color)
{
   int i, j;
   BYTE other_color;
   POINT p;
   Bool found;

   found = False;
   for (i=0; i < BOARD_HEIGHT; i++)
      for (j=0; j < BOARD_WIDTH; j++)
	 if (b->squares[i][j].piece == KING && b->squares[i][j].color == color)
	 {
	    p.x = j;
	    p.y = i;
	    found = True;
	    break;
	 }

   if (!found)
      return False;
   
   other_color = (color == WHITE) ? BLACK : WHITE;
   if (SquareIsAttacked(b, p, other_color))
      return True;
   return False;
}
/****************************************************************************/
/*
 * IsLegalCastle:  Return True iff the given move is a castle, the player
 *   can castle on this side, and the move is legal.
 *   Assumes p1 contains the king of the given color.
 */
Bool IsLegalCastle(Board *b, POINT p1, POINT p2, BYTE color)
{
   Bool left;   // True if castling on left; False if castling on right
   int i, min_col, max_col;
   BYTE other_color;
   POINT p;
   
   // First see if move is a castle attempt
   if (!((color == WHITE && p2.y == 0 && (p2.x == 1 || p2.x == 5)) ||
	 (color == BLACK && p2.y == BOARD_HEIGHT - 1 && (p2.x == 1 || p2.x == 5))))
      return False;

   if (p2.x == 1)
      left = True;
   else left = False;
   
   // See if player allowed to castle on this side
   if ((left  && !b->pdata[color].can_castle_left) ||
       (!left && !b->pdata[color].can_castle_right))
      return False;

   // See if path is clear
   if (left)
   {
      min_col = 1;
      max_col = 2;
   }
   else
   {
      min_col = 4;
      max_col = 6;
   }

   for (i = min_col; i <= max_col; i++)
      if (b->squares[p2.y][i].piece != NONE)
	 return False;

   // See if in check at any king position
   if (left)
   {
      min_col = 1;
      max_col = 3;
   }
   else
   {
      min_col = 3;
      max_col = 5;
   }

   other_color = (color == WHITE) ? BLACK : WHITE;
   p.y = p2.y;
   for (i = min_col; i <= max_col; i++)
   {
      p.x = i;
      if (SquareIsAttacked(b, p, other_color))
	 return False;
   }

   return True;
}
/****************************************************************************/
/*
 * HasLegalMove:  Return True iff player of given color has a legal move on the
 *   given board.
 */
Bool HasLegalMove(Board *b, BYTE color)
{
   int i, j, k, l;
   POINT p1, p2;

   for (i=0; i < BOARD_HEIGHT; i++)
      for (j=0; j < BOARD_WIDTH; j++)
      {
	 p1.x = j;
	 p1.y = i;
	 if (b->squares[i][j].piece != NONE && b->squares[i][j].color == color)
	 {
	    // Try moving piece to every square on the board--quite inefficient
	    for (k=0; k < BOARD_HEIGHT; k++)
	       for (l=0; l < BOARD_WIDTH; l++)
	       {
		  p2.x = l;
		  p2.y = k;
		  if (ChessMoveIsLegal(b, p1, p2, color))
		     return True;
	       }
	 }
      }
   return False;
}
