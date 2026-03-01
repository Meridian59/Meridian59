// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * server.h:  Header for server.c
 */

#ifndef _SERVER_H
#define _SERVER_H

#define LENBYTES 2   /* # of bytes required to store message length */
#define CRCBYTES 2   /* # of bytes required to store CRC16 */
#define HEADER_SIZE (2 * LENBYTES + CRCBYTES + 1)

#define MAXMESSAGE 4096              /* Max. length of string message */

#define LIST_ERROR ((list_type) -1)

// Conversion between server bitmap groups (1 based) to client bitmap groups (0 based)
#define BitmapGroupSToC(x) ((x) - 1)
#define BitmapGroupCToS(x) ((x) + 1)

/* 
 * Message handlers are given in the table handler_table.  Their two arguments
 * are a pointer to the first character in the message after the type byte, and
 * the length of the message NOT INCLUDING the type byte.
 * A message handler should return True iff the message has the correct format.
 * If a message doesn't need a handler, specify NULL as the handler.
 */

typedef bool (*MessageHandler)(char *ptr, long len);

typedef struct {
   unsigned char  msg_type;
   MessageHandler handler;
} handler_struct, *HandlerTable;

void ResetSecurityToken();

/* Procedures to extract pieces from server messages */
M59EXPORT void         Extract(char **buf, void *result, UINT numbytes);
M59EXPORT void         ExtractCoordinates(char **ptr, int *x, int *y);
M59EXPORT void         ExtractObject(char **ptr, object_node *item, bool includeLight = true);
M59EXPORT void         ExtractObjectNoLight(char **ptr, object_node *item);
M59EXPORT void			ExtractDLighting(char **ptr, d_lighting *dLighting);
M59EXPORT object_node *ExtractNewObject(char **ptr);
M59EXPORT object_node *ExtractNewObjectNoLighting(char **ptr);
//M59EXPORT room_contents_node *ExtractRoomObject(char **ptr);
M59EXPORT room_contents_node *ExtractNewRoomObject(char **ptr);
M59EXPORT list_type    ExtractObjectList(char **ptr, long len);
M59EXPORT WORD         ExtractString(char **ptr, long len, char *message, int max_chars);
M59EXPORT void         ExtractAnimation(char **ptr, Animate *a);
M59EXPORT list_type    ExtractOverlays(char **ptr);
M59EXPORT void         ExtractOverlay(char **ptr, Overlay *overlay);
BackgroundOverlay *ExtractNewBackgroundOverlay(char **ptr);

bool HandleMessage(char *message, int len);
void WaitForMessage(WORD type);
void ClearWaitMessage(void);

// This procedure is used by modules to parse their private message tables, as well
// as by the client for parsing of all server messages.
M59EXPORT bool LookupMessage(char *message, int len, HandlerTable table);

/* Game mode handlers */
bool HandlePlayer(char *ptr, long len);
bool HandleRoomContents(char *ptr, long len);
bool HandleObjectContents(char *ptr, long len);
bool HandleMove(char *ptr, long len);
bool HandleCreate(char *ptr,long len);
bool HandleRemove(char *ptr,long len);
bool HandleChange(char *ptr,long len);
bool HandleInventoryAdd(char *ptr,long len);
bool HandleInventoryRemove(char *ptr,long len);
bool HandleInventory(char *ptr,long len);
bool HandleStringMessage(char *ptr,long len);
bool HandleSaid(char *ptr,long len);
bool HandleInvalidateData(char *ptr,long len);
bool HandleWait(char *ptr,long len);
bool HandleUnwait(char *ptr,long len);
bool HandleLook(char *ptr,long len);
bool HandleUse(char *ptr,long len);
bool HandleUnuse(char *ptr,long len);
bool HandleUseList(char *ptr,long len);
bool HandleOfferCanceled(char *ptr,long len);
bool HandleRecipients(char *ptr,long len);
bool HandleOffer(char *ptr,long len);
bool HandleOffered(char *ptr,long len);
bool HandleCounteroffer(char *ptr,long len);
bool HandleCounteroffered(char *ptr,long len);
bool HandlePlayers(char *ptr,long len);
bool HandleAddPlayer(char *ptr,long len);
bool HandleRemovePlayer(char *ptr,long len);
bool HandleSysMessage(char *ptr,long len);
bool HandleBuyList(char *ptr,long len);
bool HandleWithdrawalList(char *ptr,long len);
bool HandlePlayWave(char *ptr,long len);
bool HandlePlayMidi(char *ptr,long len);
bool HandlePlayMusic(char *ptr,long len);
bool HandleQuit(char *ptr, long len);
bool HandleGameResync(char *ptr, long len);
bool HandleClasses(char *ptr,long len);
bool HandleNoInside(char *ptr,long len);
bool HandleLightAmbient(char *ptr, long len);
bool HandleLightPlayer(char *ptr, long len);
bool HandleBackground(char *ptr, long len);
bool HandleEffect(char *ptr, long len);
bool HandleTurn(char *ptr, long len);
bool HandleShoot(char *ptr, long len);
bool HandleUsers(char *ptr, long len);
bool HandleLoadModule(char *ptr, long len);
bool HandleChangeResource(char *ptr, long len);
bool HandlePlayerOverlay(char *ptr, long len);
bool HandleSectorMove(char *ptr, long len);
bool HandleUnloadModule(char *ptr, long len);
bool HandleWallAnimate(char *ptr, long len);
bool HandleSectorAnimate(char *ptr, long len);
bool HandleAddBackgroundOverlay(char *ptr, long len);
bool HandleRemoveBackgroundOverlay(char *ptr, long len);
bool HandleChangeBackgroundOverlay(char *ptr, long len);
bool HandleRoundtrip(char *ptr, long len);
bool HandleChangeTexture(char *ptr, long len);
bool HandleLightShading(char *ptr, long len);
bool HandleEchoPing(char *ptr, long len);
bool HandleSectorLight(char *ptr, long len);
bool HandleSetView(char *ptr, long len);
bool HandleResetView(char *ptr, long len);
bool HandleWeather(char *ptr, long len);

/* Login mode handlers */
bool HandleLoginOk(char *ptr,long len);
bool HandleLoginFailed(char *ptr, long len);
bool HandleEnterGame(char *ptr, long len);
bool HandleEnterAdmin(char *ptr, long len);
bool HandleGetLogin(char *ptr, long len);
bool HandleGetChoice(char *ptr, long len);
bool HandleAccountUsed(char *ptr, long len);
bool HandleTooManyLogins(char *ptr, long len);
bool HandleTimeout(char *ptr, long len);
bool HandleCredits(char *ptr, long len);
bool HandleDownload(char *ptr, long len);
bool HandleNoCredits(char *ptr, long len);
bool HandlePasswordOk(char *ptr, long len);
bool HandlePasswordNotOk(char *ptr, long len);
bool HandleLoginErrorMsg(char *ptr, long len);
bool HandleLoginResync(char *ptr, long len);
bool HandleDeleteRsc(char *ptr,long len);
bool HandleDeleteAllRsc(char *ptr,long len);
bool HandleNoCharacters(char *ptr, long len);
bool HandleTime(char *ptr, long len);
bool HandleGetClient(char *ptr, long len);

#endif /* #ifndef _SERVER_H */
