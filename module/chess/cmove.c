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

// Squares king ends up on when castling
static const int CASTLE_X_LEFT = 2;
static const int CASTLE_X_RIGHT = 6;

static bool PathIsClear(Board *b, POINT p1, POINT p2, BYTE color);
static bool SquareIsAttacked(Board *b, POINT p, BYTE color);
static bool PieceCanMove(Board *b, POINT p1, POINT p2, BYTE color);
static void PieceMove(Board *b, POINT p1, POINT p2);
static bool IsLegalCastle(Board *b, POINT p1, POINT p2, BYTE color);
/****************************************************************************/
/*
 * ChessMoveIsLegal:  Return true iff the move from p1 to p2 is legal for the player
 *   of the given color.
 *   Assumes that p1 contains a piece of the player's color.
 */
bool ChessMoveIsLegal(Board *b, POINT p1, POINT p2, BYTE color)
{
   Board new_board;
   
   if (!PieceCanMove(b, p1, p2, color))
      return false;

   // Move must not leave king in check -- see if king attacked after move performed
   memcpy(&new_board, b, sizeof(Board));
   ChessMovePerform(&new_board, p1, p2, false);

   if (IsInCheck(&new_board, color))
      return false;
   return true;
}
/****************************************************************************/
/*
 * PieceCanMove:  Return true iff the piece at p1 can move to p2.
 *   Assumes that p1 contains a piece of the player's color.
 */
bool PieceCanMove(Board *b, POINT p1, POINT p2, BYTE color)
{
   BYTE piece;
   Square *s2;
   int direction, dy;
   
   if (p1.x == p2.x && p1.y == p2.y)
      return false;

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
	 return false;
      
      // 3 cases:  capturing diagonally, moving straight ahead, or illegal move
      switch (abs(p2.x - p1.x))
      {
      case 0:
	 // If moving straight, can't capture
	 if (s2->piece != NONE)
	    return false;
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
	    return false;
	 break;
      default:
	 return false;
      }

      // Can only move 2 squares forward on first move
      if (abs(dy) == 2)
	 if ((color == WHITE && p1.y != 1) ||
	     (color == BLACK && p1.y != 6))
	    return false;
      break;
   case ROOK:
      if (p1.x != p2.x && p1.y != p2.y)
	 return false;
      break;
   case KNIGHT:
      if (abs(p2.x - p1.x) + abs(p2.y - p1.y) != 3)
	 return false;

      if (abs(p2.x - p1.x) == 3 || abs(p2.y - p1.y) == 3)
	 return false;
      break;
   case BISHOP:
      if (abs(p2.x - p1.x) != abs(p2.y - p1.y))
	 return false;
      break;
   case QUEEN:
      if (p1.x != p2.x && p1.y != p2.y && abs(p2.x - p1.x) != abs(p2.y - p1.y))
	 return false;
      break;
   case KING:
      if (IsLegalCastle(b, p1, p2, color))
	 break;

      if (abs(p2.x - p1.x) > 1 || abs(p2.y - p1.y) > 1)
	 return false;
      break;
   default:
      debug(("ChessMoveIsLegal got unknown piece type %d\n", piece));
      return false;
   }

   // Can't move onto same color
   if (s2->piece != NONE && s2->color == color)
      return false;
      
   // Check path to make sure no pieces are in the way (except for knights)
   if (piece != KNIGHT)
     if (!PathIsClear(b, p1, p2, color))
       return false;
   return true;
}

/****************************************************************************/
/*
 * ChessMovePerform:  Perform the move from p1 to p2 on the given board.  Assumes
 *   that the move is legal.
 *   If interactive is true, ask user for piece on pawn promotion if necessary.
 */
void ChessMovePerform(Board *b, POINT p1, POINT p2, bool interactive)
{
   BYTE piece;
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
     POINT p3, p4;
      // Move rook at p3 to new location at p4
      p3.y = p2.y;
      p4.y = p2.y;
      if (p2.x == CASTLE_X_LEFT)
      {
        p3.x = 0;
        p4.x = 3;
      }
      else
      {
        p3.x = 7;
        p4.x = 5;
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
      b->pdata[b->move_color].can_castle_left  = false;
      b->pdata[b->move_color].can_castle_right = false;
   }
   else if (piece == ROOK)
   {
      if (((b->move_color == WHITE && p1.y == 0) || (b->move_color == BLACK && p1.y == 7)))
	 if (p1.x == 0)
	    b->pdata[b->move_color].can_castle_left = false;
	 else if (p1.x == 7)
	    b->pdata[b->move_color].can_castle_right = false;
   }

   // Set en passant information
   if (piece == PAWN && abs(p2.y - p1.y) > 1) 
   {
      b->en_passant = true;
      b->passant_square.x = p1.x;
      b->passant_square.y = (p1.y + p2.y) / 2;
   }
   else b->en_passant = false;
   
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
 * PathIsClear:  Return true iff the path from p1 to p2 is clear, with these
 *   2 exceptions:
 *    1) p1 may contain a piece
 *    2) p2 may contain a piece NOT of the given color
 *   Assumes that p1 and p2 are connected by a horizontal, vertical, or diagonal
 *   line.
 */
bool PathIsClear(Board *b, POINT p1, POINT p2, BYTE color)
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
	 return false;
      x += dx;
      y += dy;
   }

   // p2 can't contain piece of same color
   if (b->squares[y][x].piece != NONE && b->squares[y][x].color == color)
      return false;
   
   return true;
}
/****************************************************************************/
/*
 * SquareIsAttacked:  Return true iff the given square is attacked by a piece
 *   of the given color.
 */
bool SquareIsAttacked(Board *b, POINT p, BYTE color)
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
	       return true;
	 }
   
   return false;
}
/****************************************************************************/
/*
 * IsInCheck:  Return true iff player of given color is in check on given board.
 */
bool IsInCheck(Board *b, BYTE color)
{
   int i, j;
   BYTE other_color;
   POINT p;
   bool found;

   found = false;
   for (i=0; i < BOARD_HEIGHT; i++)
      for (j=0; j < BOARD_WIDTH; j++)
	 if (b->squares[i][j].piece == KING && b->squares[i][j].color == color)
	 {
	    p.x = j;
	    p.y = i;
	    found = true;
	    break;
	 }

   if (!found)
      return false;
   
   other_color = (color == WHITE) ? BLACK : WHITE;
   if (SquareIsAttacked(b, p, other_color))
      return true;
   return false;
}
/****************************************************************************/
/*
 * IsLegalCastle:  Return true iff the given move is a castle, the player
 *   can castle on this side, and the move is legal.
 *   Assumes p1 contains the king of the given color.
 */
bool IsLegalCastle(Board *b, POINT p1, POINT p2, BYTE color)
{
   bool left;   // true if castling on left; false if castling on right
   int i, min_col, max_col;
   BYTE other_color;
   POINT p;

   // First see if move is a castle attempt
   if (p2.x != CASTLE_X_LEFT && p2.x != CASTLE_X_RIGHT)
     return false;

   if (color == WHITE && p2.y != 0)
     return false;
   if (color == BLACK && p2.y != BOARD_HEIGHT - 1)
     return false;
   
   if (p2.x == CASTLE_X_LEFT)
      left = true;
   else left = false;
   
   // See if player allowed to castle on this side
   if ((left  && !b->pdata[color].can_castle_left) ||
       (!left && !b->pdata[color].can_castle_right))
      return false;


   // See if path is clear
   if (left)
   {
      min_col = CASTLE_X_LEFT;
      max_col = 3;
   }
   else
   {
      min_col = 5;
      max_col = CASTLE_X_RIGHT;
   }

   for (i = min_col; i <= max_col; i++)
     if (b->squares[p2.y][i].piece != NONE)
       return false;

   // See if in check at any king position
   if (left)
   {
      min_col = CASTLE_X_LEFT;
      max_col = 4;
   }
   else
   {
      min_col = 4;
      max_col = CASTLE_X_RIGHT;
   }

   other_color = (color == WHITE) ? BLACK : WHITE;
   p.y = p2.y;
   for (i = min_col; i <= max_col; i++)
   {
      p.x = i;
      if (SquareIsAttacked(b, p, other_color))
	 return false;
   }

   return true;
}
/****************************************************************************/
/*
 * HasLegalMove:  Return true iff player of given color has a legal move on the
 *   given board.
 */
bool HasLegalMove(Board *b, BYTE color)
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
		     return true;
	       }
	 }
      }
   return false;
}
