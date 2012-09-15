// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * chess.c:  DLL for player vs. player chess.
 *
 * XXX Sounds for pieces moving
 */

#include "client.h"
#include "chess.h"

HINSTANCE hInst;              // Handle of this DLL
static HWND hChessDlg;        // Modeless game window
static HWND hPromoteDlg;      // Pawn promotion dialog

static Board b;               // State of game
static ID game_obj;           // Server object of chess game
static WNDPROC lpfnDefBoardProc; // Default board window procedure

static float aspect_ratio;    // Aspect ratio of main dialog; preserved on resize

ClientInfo *c;         // Holds data passed from main client

Bool exiting;

/* local function prototypes */
static Bool HandleUserCommand(char *ptr, long len);
static Bool HandleGameState(char *ptr, long len);
static Bool HandleGameStart(char *ptr, long len);
static Bool HandleGamePlayer(char *ptr, long len);

// Server message handler table
static handler_struct handler_table[] = {
{ BP_USERCOMMAND,        HandleUserCommand },
{ 0, NULL},
};

// Server user command message table
static handler_struct user_handler_table[] = {
{ UC_MINIGAME_MOVE,     HandleGameState },
{ UC_MINIGAME_START,    HandleGameStart },
{ UC_MINIGAME_PLAYER,   HandleGamePlayer },
{ 0, NULL},
};

// Client message table
client_message user_msg_table[] = {
{ UC_MINIGAME_STATE,         { PARAM_ID, PARAM_STRING, PARAM_END }, },
{ UC_MINIGAME_RESET_PLAYERS, { PARAM_ID, PARAM_END }, },
{ 0,                         { PARAM_END }, },
};

static ChildPlacement chess_controls[] = {
{ IDC_BOARD,       RDI_ALL },
{ IDC_WHITELABEL,  RDI_RIGHT | RDI_TOP },
{ IDC_BLACKLABEL,  RDI_RIGHT | RDI_TOP },
{ IDC_WHITENAME,   RDI_RIGHT | RDI_TOP },
{ IDC_BLACKNAME,   RDI_RIGHT | RDI_TOP },
{ IDC_STATUS,      RDI_RIGHT | RDI_TOP },
{ IDCANCEL,        RDI_RIGHT | RDI_BOTTOM },
{ IDC_RESIGN,      RDI_RIGHT | RDI_BOTTOM },
{ IDC_RESTART_GAME,RDI_RIGHT | RDI_BOTTOM },
{ IDC_RESETPLAYERS,RDI_RIGHT | RDI_BOTTOM },
{ 0,               0 },
};

static RECT dlg_rect;    // Dialog rectangle; used for resizing
// Minimum chess dialog size
static SIZE min_window_size;

static BOOL CALLBACK ChessDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void ChessMinMaxInfo(HWND hwnd, MINMAXINFO *lpmmi);
static void ChessDlgCommand(HWND hDlg, int cmd_id, HWND hwndCtl, UINT codeNotify);
static long CALLBACK ChessBoardProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
static void BoardLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static void ChessGotState(unsigned char *state);
static void ChessDlgShowMessage(char *message);
static void ChessDlgShowGameStatus(void);
static void ChessDlgShowMover(void);
static BOOL CALLBACK ChessPromotionDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static Bool AbortChessDialogs(void);
static void ChessGotPlayerName(BYTE player_num, char *name);
static void ChessSendMove(void);
/****************************************************************************/
BOOL WINAPI DllMain(HINSTANCE hModule, DWORD reason, LPVOID reserved)
{
   switch (reason)
   {
   case DLL_PROCESS_ATTACH:
      hInst = hModule;
      break;

   case DLL_PROCESS_DETACH:
      break;
   }
   return TRUE;
}
/****************************************************************************/
void WINAPI GetModuleInfo(ModuleInfo *info, ClientInfo *client_info)
{
   info->event_mask = EVENT_SERVERMSG | EVENT_RESETDATA;
   info->priority   = PRIORITY_NORMAL;
   info->module_id  = MODULE_ID;
   c = client_info;    // Save client info for our use later
						      
   game_obj = 0;
   BoardBitmapsLoad();
   hChessDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_CHESS), c->hMain, ChessDialogProc);

   exiting = False;
}
/****************************************************************************/
void WINAPI ModuleExit(void)
{
   Bool has_dialog = False;

   // Prevent infinite loop when chess dialog closed
   if (exiting)
      return;
   
   exiting = True;

   has_dialog = AbortChessDialogs();
   
   if (!has_dialog)
      PostMessage(c->hMain, BK_MODULEUNLOAD, 0, MODULE_ID);
}

/* event handlers */
/****************************************************************************/
/*
 * EVENT_SERVERMSG
 */
/****************************************************************************/
Bool WINAPI EventServerMessage(char *message, long len)
{
   Bool retval;

   retval = LookupMessage(message, len, handler_table);

   // If we handle message, don't pass it on to anyone else
   if (retval == True)
     return False;

   return True;    // Allow other modules to get other messages
}
/********************************************************************/
Bool HandleUserCommand(char *ptr, long len)
{
   BYTE type;
   int index;
   Bool success;

   Extract(&ptr, &type, SIZE_TYPE);

   // Dispatch on user command code
   index = 0;
   while (user_handler_table[index].msg_type != 0)
   {
      if (user_handler_table[index].msg_type == type)
      {
	 if (user_handler_table[index].handler != NULL)
	 {                           
	    /* Don't count type byte in length for handler */
	    success = (*user_handler_table[index].handler)(ptr, len - SIZE_TYPE);
	    if (!success)
	    {
	       debug(("Error in user command message of type %d from server\n", type));
	       return False;
	    }
	    return True;
	 }
	 break;
      }
      index++;
   }
   return False;
}
/********************************************************************/
Bool HandleGameState(char *ptr, long len)
{
   ID game;
   unsigned char state[BOARD_STATE_LEN + 1];

   Extract(&ptr, &game, SIZE_ID);

   if (game != game_obj)
   {
      debug(("Chess got game state for game %d, expecting game %d\n", game, game_obj));
      return False;
   }
   len -= SIZE_ID;
   len = ExtractString(&ptr, len, (char *) state, BOARD_STATE_LEN);

   if (len != 0)
      return False;

   ChessGotState(state);
      
   return True;
}
/********************************************************************/
Bool HandleGameStart(char *ptr, long len)
{
   char *start = ptr;
   BYTE player_num;
   
   // If we've already received our game object number, skip
   if (game_obj != 0)
      return False;

   Extract(&ptr, &game_obj, SIZE_ID);
   Extract(&ptr, &player_num, 1);
   // debug(("Chess got game start, object = %d, player # = %d\n", game_obj, player_num));

   len -= (ptr - start);
   if (len != 0)
      return False;

   switch (player_num)
   {
   case 1:
      b.color = WHITE;
      if (hChessDlg != NULL)
	 EnableWindow(GetDlgItem(hChessDlg, IDC_RESIGN), TRUE);
      break;
   case 2:
      b.color = BLACK;
      if (hChessDlg != NULL)
	 EnableWindow(GetDlgItem(hChessDlg, IDC_RESIGN), TRUE);
      break;
   default:
      b.color = OBSERVER;
      break;
   }

   // Display player's name in dialog
   ChessGotPlayerName(player_num, LookupRsc(c->player->name_res));
   
   return True;
}
/********************************************************************/
Bool HandleGamePlayer(char *ptr, long len)
{
   char name[MAXUSERNAME + 1];
   BYTE player_num;

   // If we've haven't received our game object number, skip
   if (game_obj == 0)
      return False;

   Extract(&ptr, &player_num, 1);
   len -= 1;
   len = ExtractString(&ptr, len, name, MAXUSERNAME);

   if (len != 0)
      return False;
   
   ChessGotPlayerName(player_num, name);
   return True;
}
/****************************************************************************/
/*
 * EVENT_RESETDATA
 */
/****************************************************************************/
Bool WINAPI EventResetData(void)
{
   ModuleExit();
   return True;
}
/********************************************************************/
BOOL CALLBACK ChessDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   HWND hwnd;
   RECT rect;

   UserDidSomething();
   switch (message)
   {
   case WM_INITDIALOG:
      CenterWindow(hDlg, c->hMain);
      hChessDlg = hDlg;

      hwnd = GetDlgItem(hDlg, IDC_BOARD);
      lpfnDefBoardProc = SubclassWindow(hwnd, ChessBoardProc);

      /* Store dialog rectangle in case of resize */
      GetWindowRect(hDlg, &dlg_rect);
      min_window_size.cx = dlg_rect.right - dlg_rect.left;
      min_window_size.cy = dlg_rect.bottom - dlg_rect.top;
      if (min_window_size.cy == 0)
	 aspect_ratio = 1.0;
      else aspect_ratio = min_window_size.cx / min_window_size.cy;

      GetClientRect(hwnd, &rect);
      b.square_size = min(rect.bottom / BOARD_HEIGHT, rect.right / BOARD_WIDTH);

      b.valid = False;
      return TRUE;

   case WM_SIZE:
      ResizeDialog(hDlg, &dlg_rect, chess_controls);
      hwnd = GetDlgItem(hDlg, IDC_BOARD);
      GetClientRect(hwnd, &rect);
      b.square_size = min(rect.bottom / BOARD_HEIGHT, rect.right / BOARD_WIDTH);
      return TRUE;
      
      HANDLE_MSG(hDlg, WM_COMMAND, ChessDlgCommand);
      HANDLE_MSG(hDlg, WM_GETMINMAXINFO, ChessMinMaxInfo);
      
      HANDLE_MSG(hDlg, WM_INITMENUPOPUP, InitMenuPopupHandler);

   case WM_ACTIVATE:
      if (wParam == 0)
	 *c->hCurrentDlg = NULL;
      else *c->hCurrentDlg = hDlg;
      return TRUE;

   case WM_DESTROY:
      hChessDlg = NULL;
      return TRUE;
   }
   return FALSE;
}
/****************************************************************************/
void ChessMinMaxInfo(HWND hwnd, MINMAXINFO *lpmmi)
{
   /* Set minimum size of window */
   lpmmi->ptMinTrackSize.x = min_window_size.cx;
   lpmmi->ptMinTrackSize.y = min_window_size.cy;
}
/****************************************************************************/
/*
 * ChessDlgCommand:  Handle WM_COMMAND messages in the main chess dialog.
 */
void ChessDlgCommand(HWND hDlg, int cmd_id, HWND hwndCtl, UINT codeNotify)
{
   switch(cmd_id)
   {
   case IDC_RESIGN:
      if (!AreYouSure(hInst, hDlg, NO_BUTTON, IDS_ASKRESIGN))
	 break;

      if (b.color == WHITE)
	 b.white_resigned = True;
      if (b.color == BLACK)
	 b.black_resigned = True;

      ChessSendMove();
      break;

   case IDC_RESTART_GAME:
      BoardInitialize(&b);
      ChessSendMove();
      ChessRedrawBoard();
      break;

   case IDC_RESETPLAYERS:
      RequestGameResetPlayers(game_obj);
      break;

   case IDOK:
   case IDCANCEL:
      ModuleExit();
      break;
   }
}
/************************************************************************/
/*
 * ChessBoardProc:  Subclassed window procedure for board area.
 */
long CALLBACK ChessBoardProc(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
   HDC hdc;
   
   UserDidSomething();
   switch (message)
   {
   case WM_KEYDOWN:
      return 0;
      
   case WM_ERASEBKGND:
      return 1;

   case WM_PAINT:
      if (exiting)
	 break;

      hdc = GetDC(hwnd);
      BoardDraw(hdc, &b);
      ReleaseDC(hwnd, hdc);
      break;

      HANDLE_MSG(hwnd, WM_LBUTTONDOWN, BoardLButtonDown);
   }
   return CallWindowProc(lpfnDefBoardProc, hwnd, message, wParam, lParam);
}
/************************************************************************/
/*
 * ChessRedrawBoard:  Call to redraw board window.
 */
void ChessRedrawBoard(void)
{
   HWND hwnd;
   HDC hdc;

   if (hChessDlg == NULL || exiting)
      return;

   hwnd = GetDlgItem(hChessDlg, IDC_BOARD);
   hdc = GetDC(hwnd);
   BoardDraw(hdc, &b);
   ReleaseDC(hwnd, hdc);
}
/************************************************************************/
/*
 * BoardLButtonDown:  User pressed left mouse button on board.
 */
void BoardLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
   int row, col;

   // Check that it's our turn
   if (b.color != b.move_color || exiting)
      return;

   // See if game is over
   if (b.white_resigned || b.black_resigned)
      return;
   
   row = y / b.square_size;
   col = x / b.square_size;

   if (row >= BOARD_HEIGHT || col >= BOARD_WIDTH)
      return;

   // If user does a move, send to server
   if (BoardSquareSelect(&b, row, col))
      ChessSendMove();
}
/************************************************************************/
/*
 * ChessSendMove:  Send current board state to server.
 */
void ChessSendMove(void)
{
  unsigned char state[BOARD_STATE_LEN + 1];
  
  BoardEncode(&b, state);
  RequestGameMove(game_obj, state);
  ChessDlgShowGameStatus();
  ChessDlgShowMover();
}
/************************************************************************/
/*
 * ChessGotState:  Got given board state from server.
 */
void ChessGotState(unsigned char *state)
{
   //   debug(("Chess got game state %s\n", state));
   b.valid = True;

   // If game hasn't been initialized yet, set default board configuration
   if (state[0] == 0)
     BoardInitialize(&b);
   else 
     if (!BoardDecode(state, &b))
       debug(("HandleGameState got illegal state\n"));
   ChessRedrawBoard();
   
   ChessDlgShowGameStatus();
   ChessDlgShowMover();
}
/************************************************************************/
/*
 * ChessDlgShowGameStatus:  Display a message giving state of game
 *   (stalemate, checkmate, etc.)
 *   Return True iff any message was displayed.
 */
void ChessDlgShowGameStatus(void)
{
   if (hChessDlg == NULL)
      return;
   
   b.game_over = False;

   // Give a message for various situations
   if (b.white_resigned)
   {
      ChessDlgShowMessage(GetString(hInst, IDS_WHITERESIGN));
      b.game_over = True;
   }
   else if (b.black_resigned)
   {
      ChessDlgShowMessage(GetString(hInst, IDS_BLACKRESIGN));
      b.game_over = True;
   }
   else if (!HasLegalMove(&b, b.move_color))
   {
      // Give a message if checkmated
      if (IsInCheck(&b, b.move_color))
	 ChessDlgShowMessage(GetString(hInst, IDS_CHECKMATE));
      else ChessDlgShowMessage(GetString(hInst, IDS_STALEMATE));
      b.game_over = True;
   }
   else if (IsInCheck(&b, b.move_color))
   {
      ChessDlgShowMessage(GetString(hInst, IDS_CHECK));
   }
   else ChessDlgShowMessage("");

   // Observers can't reset game
   EnableWindow(GetDlgItem(hChessDlg, IDC_RESTART_GAME), b.game_over && (b.color != OBSERVER));
   EnableWindow(GetDlgItem(hChessDlg, IDC_RESETPLAYERS), b.game_over && (b.color != OBSERVER));
   EnableWindow(GetDlgItem(hChessDlg, IDC_RESIGN), !b.game_over && (b.color != OBSERVER));
}
/************************************************************************/
/*
 * ChessDlgShowMover:  Show whose turn it is in dialog.
 */
void ChessDlgShowMover(void)
{
   int id1, id2;

   // Show whose turn it is
   if (b.move_color == WHITE)
   {
      SetDlgItemText(hChessDlg, IDC_WHITELABEL, GetString(hInst, IDS_WHITEMOVE));
      SetDlgItemText(hChessDlg, IDC_BLACKLABEL, GetString(hInst, IDS_BLACK));
      id1 = IDC_WHITENAME;
      id2 = IDC_BLACKNAME;
   }
   else
   {
      SetDlgItemText(hChessDlg, IDC_WHITELABEL, GetString(hInst, IDS_WHITE));
      SetDlgItemText(hChessDlg, IDC_BLACKLABEL, GetString(hInst, IDS_BLACKMOVE));
      id1 = IDC_BLACKNAME;
      id2 = IDC_WHITENAME;
   }
   SetWindowFont(GetDlgItem(hChessDlg, id1), GetFont(FONT_TITLES), TRUE);
   SetWindowFont(GetDlgItem(hChessDlg, id2), GetFont(FONT_STATS), TRUE);
}
/****************************************************************************/
/*
 * ChessDlgShowMessage:  Display given text message in chess dialog.
 */
void ChessDlgShowMessage(char *message)
{
   if (hChessDlg == NULL)
      return;

   SetDlgItemText(hChessDlg, IDC_STATUS, message);
}
/************************************************************************/
/*
 * ChessGetPromotionPiece:  Return piece user chooses for pawn promotion, or
 *   NONE if dialog is aborted.
 */
BYTE ChessGetPromotionPiece(void)
{
   return DialogBox(hInst, MAKEINTRESOURCE(IDD_PROMOTION), hChessDlg, ChessPromotionDialogProc);
}
/********************************************************************/
BOOL CALLBACK ChessPromotionDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   UserDidSomething();
   switch (message)
   {
   case WM_INITDIALOG:
      CheckDlgButton(hDlg, IDC_QUEEN, TRUE);
      CenterWindow(hDlg, GetParent(hDlg));
      hPromoteDlg = hDlg;
      return TRUE;
      
   case WM_COMMAND:
      switch (GET_WM_COMMAND_ID(wParam, lParam))
      {
      case IDOK:
      case IDCANCEL:
	 if (IsDlgButtonChecked(hDlg, IDC_KNIGHT))
	    EndDialog(hDlg, KNIGHT);
	 else if (IsDlgButtonChecked(hDlg, IDC_BISHOP))
	    EndDialog(hDlg, BISHOP);
	 else if (IsDlgButtonChecked(hDlg, IDC_ROOK))
	    EndDialog(hDlg, ROOK);
	 else EndDialog(hDlg, QUEEN);
	 return TRUE;
      }
      break;

   case WM_DESTROY:
      hPromoteDlg = NULL;
      break;

   }
   return FALSE;
}
/****************************************************************************/
/*
 * ChessGotPlayerName:  Show player's name on dialog
 */
void ChessGotPlayerName(BYTE player_num, char *name)
{
   int id;
   
   if (hChessDlg == NULL)
      return;
   
   switch(player_num)
   {
   case 1:
      id = IDC_WHITENAME;
      break;
   case 2:
      id = IDC_BLACKNAME;
      break;
   default:
      // debug(("ChessGotPlayerName got unexpected player number %d\n", player_num));
      return;
   }
   SetDlgItemText(hChessDlg, id, name);
}
/****************************************************************************/
/*
 * AbortChessDialogs:  Close chess dialogs.  Return True iff any was open.
 */
Bool AbortChessDialogs(void)
{
   Bool retval = False, has_promote;

   has_promote = (hPromoteDlg != NULL);

   if (hChessDlg != NULL)
   {
      DestroyWindow(hChessDlg);
      retval = True;

      if (!has_promote)
	 PostMessage(c->hMain, BK_MODULEUNLOAD, 0, MODULE_ID);
   }
   return retval;
}
