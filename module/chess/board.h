// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * board.h:  Header file for board.c
 */

#ifndef _BOARD_H
#define _BOARD_H

#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8

#define BOARD_STATE_LEN (64 + 3)

enum {WHITE = 0, BLACK = 1, OBSERVER = 2};
enum {PAWN = 1, ROOK, KNIGHT, BISHOP, QUEEN, KING, NONE};

typedef struct {
   BYTE color;
   BYTE piece;
} Square;

typedef struct {
   bool can_castle_left;   // Can castle on the left side
   bool can_castle_right;  // Can castle on the right side
} PlayerData;

typedef struct {
   Square squares[BOARD_HEIGHT][BOARD_WIDTH];
   int square_size;      // # of pixels per square
   BYTE color;           // Color of this player, OBSERVER if not a player
   BYTE move_color;      // Whose turn it is
   bool valid;           // True when initial board data has been received from server
   PlayerData pdata[2];  // Per-player info (can castle, etc.)
   bool en_passant;      // True if last move on board was a pawn move 2 squares forward
   POINT passant_square; // Square where pawn could be captured via en passant
                         // (meaningful only if en_passant is True).
   bool white_resigned;  // True when white has resigned
   bool black_resigned;  // True when black has resigned

   bool game_over;       // True when game is over
} Board;

void BoardEncode(Board *b, unsigned char *s);
bool BoardDecode(unsigned char *s, Board *b);
void BoardBitmapsLoad(void);
void BoardDraw(HDC hdc, Board *b);
void BoardInitialize(Board *b);
bool BoardSquareSelect(Board *b, int row, int col);

#endif /* #ifndef _BOARD_H */
