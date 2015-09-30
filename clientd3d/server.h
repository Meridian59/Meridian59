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

typedef Bool (*MessageHandler)(char *ptr, long len);

typedef struct {
   unsigned char  msg_type;
   MessageHandler handler;
} handler_struct, *HandlerTable;

void ResetSecurityToken();

/* Procedures to extract pieces from server messages */
M59EXPORT void         Extract(char **buf, void *result, UINT numbytes);
M59EXPORT void         ExtractCoordinates(char **ptr, int *x, int *y);
M59EXPORT void         ExtractObject(char **ptr, object_node *item);
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

Bool HandleMessage(char *message, int len);
void WaitForMessage(WORD type);
void ClearWaitMessage(void);

// This procedure is used by modules to parse their private message tables, as well
// as by the client for parsing of all server messages.
M59EXPORT Bool LookupMessage(char *message, int len, HandlerTable table);

/* Game mode handlers */
Bool HandlePlayer(char *ptr, long len);
Bool HandleRoomContents(char *ptr, long len);
Bool HandleObjectContents(char *ptr, long len);
Bool HandleMove(char *ptr, long len);
Bool HandleCreate(char *ptr,long len);
Bool HandleRemove(char *ptr,long len);
Bool HandleChange(char *ptr,long len);
Bool HandleInventoryAdd(char *ptr,long len);
Bool HandleInventoryRemove(char *ptr,long len);
Bool HandleInventory(char *ptr,long len);
Bool HandleStringMessage(char *ptr,long len);
Bool HandleSaid(char *ptr,long len);
Bool HandleInvalidateData(char *ptr,long len);
Bool HandleWait(char *ptr,long len);
Bool HandleUnwait(char *ptr,long len);
Bool HandleLook(char *ptr,long len);
Bool HandleUse(char *ptr,long len);
Bool HandleUnuse(char *ptr,long len);
Bool HandleUseList(char *ptr,long len);
Bool HandleOfferCanceled(char *ptr,long len);
Bool HandleRecipients(char *ptr,long len);
Bool HandleOffer(char *ptr,long len);
Bool HandleOffered(char *ptr,long len);
Bool HandleCounteroffer(char *ptr,long len);
Bool HandleCounteroffered(char *ptr,long len);
Bool HandlePlayers(char *ptr,long len);
Bool HandleAddPlayer(char *ptr,long len);
Bool HandleRemovePlayer(char *ptr,long len);
Bool HandleSysMessage(char *ptr,long len);
Bool HandleBuyList(char *ptr,long len);
Bool HandleWithdrawalList(char *ptr,long len);
Bool HandlePlayWave(char *ptr,long len);
Bool HandlePlayMidi(char *ptr,long len);
Bool HandlePlayMusic(char *ptr,long len);
Bool HandleStopWave(char *ptr,long len);
Bool HandleQuit(char *ptr, long len);
Bool HandleGameResync(char *ptr, long len);
Bool HandleClasses(char *ptr,long len);
Bool HandleNoInside(char *ptr,long len);
Bool HandleLightAmbient(char *ptr, long len);
Bool HandleLightPlayer(char *ptr, long len);
Bool HandleBackground(char *ptr, long len);
Bool HandleEffect(char *ptr, long len);
Bool HandleTurn(char *ptr, long len);
Bool HandleShoot(char *ptr, long len);
Bool HandleRadiusShoot(char *ptr, long len);
Bool HandleUsers(char *ptr, long len);
Bool HandleLoadModule(char *ptr, long len);
Bool HandleChangeResource(char *ptr, long len);
Bool HandlePlayerOverlay(char *ptr, long len);
Bool HandleSectorMove(char *ptr, long len);
Bool HandleUnloadModule(char *ptr, long len);
Bool HandleWallAnimate(char *ptr, long len);
Bool HandleSectorAnimate(char *ptr, long len);
Bool HandleAddBackgroundOverlay(char *ptr, long len);
Bool HandleRemoveBackgroundOverlay(char *ptr, long len);
Bool HandleChangeBackgroundOverlay(char *ptr, long len);
Bool HandleRoundtrip(char *ptr, long len);
Bool HandleChangeTexture(char *ptr, long len);
Bool HandleLightShading(char *ptr, long len);
Bool HandleEchoPing(char *ptr, long len);
Bool HandleSectorLight(char *ptr, long len);
Bool HandleSetView(char *ptr, long len);
Bool HandleResetView(char *ptr, long len);
Bool HandleSectorChange(char *ptr, long len);

/* Login mode handlers */
Bool HandleLoginOk(char *ptr,long len);
Bool HandleLoginFailed(char *ptr, long len);
Bool HandleEnterGame(char *ptr, long len);
Bool HandleEnterAdmin(char *ptr, long len);
Bool HandleGetLogin(char *ptr, long len);
Bool HandleGetChoice(char *ptr, long len);
Bool HandleAccountUsed(char *ptr, long len);
Bool HandleTooManyLogins(char *ptr, long len);
Bool HandleTimeout(char *ptr, long len);
Bool HandleCredits(char *ptr, long len);
Bool HandleDownload(char *ptr, long len);
Bool HandleNoCredits(char *ptr, long len);
Bool HandlePasswordOk(char *ptr, long len);
Bool HandlePasswordNotOk(char *ptr, long len);
Bool HandleLoginErrorMsg(char *ptr, long len);
Bool HandleLoginResync(char *ptr, long len);
Bool HandleDeleteRsc(char *ptr,long len);
Bool HandleDeleteAllRsc(char *ptr,long len);
Bool HandleNoCharacters(char *ptr, long len);
Bool HandleTime(char *ptr, long len);
Bool HandleGetClient(char *ptr, long len);
Bool HandleGuest(char *ptr, long len);

#endif /* #ifndef _SERVER_H */
