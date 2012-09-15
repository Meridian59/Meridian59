// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * board.c:  Handle chess board manipulation.
 *
 * Board states are communicated to the server via a string.  The encoding works
 * as follows:
 *
 * Each of the first 64 bytes represents one square of the board, in right-to-left,
 * top-down order.  The high order bit is the color of the piece, 0 = white, 1 = black.
 * The low order 3 bits give the piece type:
 * 1 = pawn
 * 2 = rook
 * 3 = knight
 * 4 = bishop
 * 5 = queen
 * 6 = king
 * 7 = empty square
 *
 * White starts out on the top of the board.
 *
 * The next byte contains:
 * bit 0   always 1
 * bit 1   whose turn it is; 0 = white, 1 = black
 * bit 2   1 if white can castle on the left side
 * bit 3   1 if white can castle on the right side
 * bit 4   1 if black can castle on the left side
 * bit 5   1 if black can castle on the right side
 *
 * The next byte contains:
 * bit 0   always 1
 * bit 1   1 if last move was a pawn move 2 squares forward (for en passant determination)
 * bit 2-4 row of square which could capture pawn en passant (if bit 1 is 1)
 * bit 5-7 column of square which could capture pawn en passant (if bit 1 is 1)
 *
 * The next byte contains:
 * bit 0   always 1
 * bit 1   1 if white has resigned
 * bit 2   1 if black has resigned
 */

#include "client.h"
#include "chess.h"

#define NUM_PIECES 6         // # of pieces of each color

// Palette indices for chess square colors
#define WHITE_INDEX  255
#define BLACK_INDEX    0

static int select_index;     // Palette index for selected square

// Size of piece bitmaps, in pixels
#define PIECE_WIDTH 40
#define PIECE_HEIGHT 40

typedef struct {
   int       id;     // Windows resource id of bitmap
   RawBitmap bmap;
} PieceBitmap;

static PieceBitmap piece_bitmaps[2][NUM_PIECES] = {
   { {IDB_WPAWN}, {IDB_WROOK}, {IDB_WKNIGHT}, {IDB_WBISHOP}, {IDB_WQUEEN}, {IDB_WKING} },
   { {IDB_RPAWN}, {IDB_RROOK}, {IDB_RKNIGHT}, {IDB_RBISHOP}, {IDB_RQUEEN}, {IDB_RKING} },
};

static Bool move_started = False;   // True when user has selected piece to move
static POINT move_pos1, move_pos2;  // Coordinates to move from and to

/****************************************************************************/
/*
 * BoardEncode:  Encode board state into string.  String must have length at
 *   least BOARD_STATE_LEN + 1.
 */
void BoardEncode(Board *b, unsigned char *s)
{
   int i, j, index;

   index = 0;
   for (i=0; i < BOARD_HEIGHT; i++)
      for (j=0; j < BOARD_WIDTH; j++)
      {
	 s[index] = b->squares[i][j].piece;
	 s[index] |= (b->squares[i][j].color << 7);
	 index++;
      }
   s[index] = 0x01 | (b->move_color << 1);
   s[index] |= b->pdata[WHITE].can_castle_left  ? 0x04 : 0;
   s[index] |= b->pdata[WHITE].can_castle_right ? 0x08 : 0;
   s[index] |= b->pdata[BLACK].can_castle_left  ? 0x10 : 0;
   s[index] |= b->pdata[BLACK].can_castle_right ? 0x20 : 0;

   index++;
   s[index] = 0x01 | (b->en_passant ? 0x02 : 0);
   s[index] |= (b->passant_square.y << 2);
   s[index] |= (b->passant_square.x << 5);

   index++;
   s[index] = 0x01 | (b->white_resigned ? 0x02 : 0);
   s[index] |= (b->black_resigned ? 0x04 : 0);

   index++;
   s[index] = 0;
}
/****************************************************************************/
/*
 * BoardDecode:  Decode board state from string.
 *   Return True iff string is a legal board state.
 */
Bool BoardDecode(unsigned char *s, Board *b)
{
   int i, j, index;

   index = 0;
   for (i=0; i < BOARD_HEIGHT; i++)
      for (j=0; j < BOARD_WIDTH; j++)
      {
	 b->squares[i][j].color = (s[index] & 0x80) >> 7;
	 b->squares[i][j].piece = s[index] & 0x07;
	 index++;
      }
   b->move_color = (s[index] & 0x02) >> 1;

   b->pdata[WHITE].can_castle_left  = (s[index] & 0x04) ? True : False;
   b->pdata[WHITE].can_castle_right = (s[index] & 0x08) ? True : False;
   b->pdata[BLACK].can_castle_left  = (s[index] & 0x10) ? True : False;
   b->pdata[BLACK].can_castle_right = (s[index] & 0x20) ? True : False;

   index++;
   b->en_passant = (s[index] & 0x02) ? True : False;
   b->passant_square.y = (s[index] & 0x1c) >> 2;
   b->passant_square.x = (s[index] & 0xe0) >> 5;

   index++;
   b->white_resigned = (s[index] & 0x02) ? True : False;
   b->black_resigned = (s[index] & 0x04) ? True : False;

   // Compute game over elsewhere
   b->game_over = False;

   return True;
}

/****************************************************************************/
/*
 * BoardInitialize:  Set board to initial position.
 */
void BoardInitialize(Board *b)
{
   int i, j;

   for (i=0; i < BOARD_HEIGHT; i++)
      for (j=0; j < BOARD_WIDTH; j++)
      {
	 if (i < BOARD_HEIGHT / 2)
	    b->squares[i][j].color = WHITE;
	 else b->squares[i][j].color = BLACK;
	 b->squares[i][j].piece = NONE;
      }

   for (j=0; j < BOARD_WIDTH; j++)
   {
      b->squares[1][j].piece = PAWN;
      b->squares[6][j].piece = PAWN;
   }

   b->squares[0][0].piece = ROOK;
   b->squares[0][7].piece = ROOK;
   b->squares[7][0].piece = ROOK;
   b->squares[7][7].piece = ROOK;
   b->squares[0][1].piece = KNIGHT;
   b->squares[0][6].piece = KNIGHT;
   b->squares[7][1].piece = KNIGHT;
   b->squares[7][6].piece = KNIGHT;
   b->squares[0][2].piece = BISHOP;
   b->squares[0][5].piece = BISHOP;
   b->squares[7][2].piece = BISHOP;
   b->squares[7][5].piece = BISHOP;
   b->squares[0][3].piece = KING;
   b->squares[7][3].piece = KING;
   b->squares[0][4].piece = QUEEN;
   b->squares[7][4].piece = QUEEN;

   b->move_color = WHITE;

   for (i=0; i < 2; i++)
   {
      b->pdata[i].can_castle_left = True;
      b->pdata[i].can_castle_right = True;
   }

   b->en_passant = False;
   b->white_resigned = False;
   b->black_resigned = False;
   b->game_over = False;
}
/****************************************************************************/
/*
 * BoardBitmapsLoad:  Load piece bitmaps.
 */
void BoardBitmapsLoad(void)
{
   int i, j;

   for (i=0; i < 2; i++)
      for (j = 0; j < NUM_PIECES; j++)
      {
	 if (!GetBitmapResourceInfo(hInst, piece_bitmaps[i][j].id, &piece_bitmaps[i][j].bmap))
	    debug(("Unable to load bitmap for chess piece %d, color %d\n", j, i));
      }

   // Compute palette index
   select_index = GetClosestPaletteIndex(RGB(252, 156, 0));
}
/****************************************************************************/
/*
 * BoardDraw:  Draw board on given hdc. 
 */
void BoardDraw(HDC hdc, Board *b)
{
   int i, j, piece, color, index, mode;
   RECT rect;

   if (!b->valid)
   {
     rect.left = 0;
     rect.top = 0;
     rect.right = BOARD_WIDTH * b->square_size;
     rect.bottom = BOARD_HEIGHT * b->square_size;
     FillRect(hdc, &rect, (HBRUSH) GetStockObject(LTGRAY_BRUSH));
     return;
   }

   SelectPalette(hdc, c->hPal, FALSE);

   // Turn off color interpolation
   mode = GetStretchBltMode(hdc);
   SetStretchBltMode(hdc, STRETCH_DELETESCANS);

   for (i=0; i < BOARD_HEIGHT; i++)
      for (j=0; j < BOARD_WIDTH; j++)
      {
	 if ((i + j) % 2 == 0)
	    index = WHITE_INDEX;
	 else index = BLACK_INDEX;

	 // Draw highlight if square selected
	 if (move_started && move_pos1.x == j && move_pos1.y == i)
	    index = select_index;
	    
	 rect.left   = j * b->square_size;
	 rect.top    = i * b->square_size;
	 rect.right  = rect.left + b->square_size;
	 rect.bottom = rect.top + b->square_size;

	 piece = b->squares[i][j].piece;
	 color = b->squares[i][j].color;
	 if (piece == NONE)
	 {
	    OffscreenWindowColor(b->square_size, b->square_size, index);
	    OffscreenCopy(hdc, rect.left, rect.top, b->square_size, b->square_size, 0, 0);
	 }
	 else
	 {
	    OffscreenWindowColor(PIECE_WIDTH, PIECE_HEIGHT, index);
	    OffscreenStretchBlt(hdc, rect.left, rect.top, b->square_size, b->square_size,
				piece_bitmaps[color][piece - 1].bmap.bits,
				0, 0, PIECE_WIDTH, PIECE_HEIGHT,
				OBB_TRANSPARENT | OBB_FLIP | OBB_COPY);
	 }
      }
   SetStretchBltMode(hdc, mode);
}   
/****************************************************************************/
/*
 * BoardSquareSelect:  Called when user has selected a square on the board.
 *   Return True iff user makes a legal move.
 */
Bool BoardSquareSelect(Board *b, int row, int col)
{
   Bool retval = False;
   
   if (move_started)
   {
      move_pos2.x = col;
      move_pos2.y = row;
      move_started = False;

      // Check for legal move; perform if legal
      if (ChessMoveIsLegal(b, move_pos1, move_pos2, b->color))
      {
	 ChessMovePerform(b, move_pos1, move_pos2, True);
	 retval = True;
      }

      ChessRedrawBoard();
      return retval;
   }
   else
      // See if there's a piece of our color here
      if (b->squares[row][col].piece == NONE || b->squares[row][col].color != b->color)
	 return False;
   
   move_started = True;

   move_pos1.x = col;
   move_pos1.y = row;

   ChessRedrawBoard();
   return retval;
}
