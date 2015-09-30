// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/* 
 * protocol.h:  Header file for protocol.c.
 */

#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#define MAX_PARAMS 15   /* Max # of parameters in message to server */

typedef BYTE param_type;

enum {
   PARAM_ID = 1,
   PARAM_COORD = 2,
   PARAM_ATTACK_INFO = 3,
   PARAM_OBJECT = 4,
   PARAM_OBJECT_LIST = 5,
   PARAM_STRING = 6,
   PARAM_GROUP = 7,
   PARAM_SAY_INFO = 8,
   PARAM_ID_ARRAY = 9,     /* Length of array, followed by array of IDs */
   PARAM_BYTE = 10,
   PARAM_ID_LIST = 11,
   PARAM_INT_ARRAY = 12,
   PARAM_NEWSID = 13,
   PARAM_INDEX = 14,       /* Generic 4 byte index number (news article #, etc.) */
   PARAM_INT = 15,         /* Generic 4 byte integer (not truncated to 28 bits) */
   PARAM_WORD = 16,        /* Generic 2 byte integer */
   PARAM_END = 100,
};

typedef struct {
   BYTE type;
   param_type params[MAX_PARAMS + 1];   
} client_message, *ClientMsgTable;

/* Encapsulate calls to ToServer */

/* Login mode messages */
#define RequestLogin(v1, v2, i1, i2, i3, i4, i5, w1, w2, i7, i8, i9, str1, str2) \
  ToServer(AP_LOGIN, NULL, v1, v2, i1, i2, i3, i4, i5, w1, w2, i7, i8, i9, str1, str2)
#define RequestGame(time, catch, hostname) \
  ToServer(AP_REQ_GAME, NULL, time, catch, hostname)

#define RequestAdmin()               ToServer(AP_REQ_ADMIN, NULL)
#define RequestAdminNote(str)        ToServer(AP_ADMINNOTE, NULL, str)

#define RequestResources()           ToServer(AP_GETRESOURCE, NULL)
#define RequestAllFiles()            ToServer(AP_GETALL, NULL)
#define RequestUpload()              ToServer(AP_UPLOAD, NULL)
#define RequestRegister(str)         ToServer(AP_REGISTER, NULL, str)
#define RequestMainMenu()            ToServer(AP_REQ_MENU, NULL)
#define RequestLoginResync()         ToServer(AP_RESYNC, NULL)

#define RequestLoginPing()           ToServer(AP_PING, NULL)

/* Game mode messages */
#define SendLogoff()                 ToServer(BP_LOGOFF, NULL)
#define RequestPlayer()              ToServer(BP_SEND_PLAYER, NULL)
#define RequestRoom()                ToServer(BP_SEND_ROOM_CONTENTS, NULL)
// Convert from client 0-based coordinates to server 1-based coordinates
#define RequestMove(y, x, speed, room) \
ToServer(BP_REQ_MOVE, NULL, FinenessClientToKod(y) + KOD_FINENESS, \
	 FinenessClientToKod(x) + KOD_FINENESS, speed, room)
#define RequestObjectContents(id)    ToServer(BP_SEND_OBJECT_CONTENTS, NULL, id)
#define RequestPickup(id)            ToServer(BP_REQ_GET, NULL, id)
#define RequestInventory()           ToServer(BP_REQ_INVENTORY, NULL)
#define RequestDrop(obj)             ToServer(BP_REQ_DROP, NULL, obj)
#define RequestInventoryMove(id1, id2) ToServer(BP_REQ_INVENTORY_MOVE, NULL, id1, id2)
#define RequestPut(id1, id2)         ToServer(BP_REQ_PUT, NULL, id1, id2)
#define SendSay(info, msg)           ToServer(BP_SAY_TO, NULL, info, msg)
#define SendSayGroup(objs, msg)      ToServer(BP_SAY_GROUP, NULL, objs, msg)
#define RequestLook(id)              ToServer(BP_REQ_LOOK, NULL, id)
#define RequestUse(id)               ToServer(BP_REQ_USE, NULL, id)
#define RequestUnuse(id)             ToServer(BP_REQ_UNUSE, NULL, id)
#define RequestAttack(info, id)      ToServer(BP_REQ_ATTACK, NULL, info, id)
#define RequestOffer(id, objs)       ToServer(BP_REQ_OFFER, NULL, id, objs)
#define RequestDeposit(id, objs)     ToServer(BP_REQ_DEPOSIT, NULL, id, objs)
#define SendCancelOffer()            ToServer(BP_CANCEL_OFFER, NULL)
#define SendAcceptOffer()            ToServer(BP_ACCEPT_OFFER, NULL)
#define RequestCounteroffer(objs)    ToServer(BP_REQ_COUNTEROFFER, NULL, objs)
#define RequestGo()                  ToServer(BP_REQ_GO, NULL)
#define RequestPlayers()             ToServer(BP_SEND_PLAYERS, NULL)
#define RequestBuy(obj)              ToServer(BP_REQ_BUY, NULL, obj)
#define RequestWithdrawal(obj)	     ToServer(BP_REQ_WITHDRAWAL, NULL, obj)
#define RequestBuyItems(seller, objs) ToServer(BP_REQ_BUY_ITEMS, NULL, seller, objs)
#define RequestWithdrawalItems(banker, objs) ToServer(BP_REQ_WITHDRAWAL_ITEMS, NULL, banker, objs)
#define RequestQuit()                ToServer(BP_REQ_QUIT, NULL)
#define RequestApply(obj1, obj2)     ToServer(BP_REQ_APPLY, NULL, obj1, obj2)
#define RequestGameResync()          ToServer(BP_RESYNC, NULL)
#define RequestCast(id, objs)        ToServer(BP_REQ_CAST, NULL, id, objs)
#define RequestTurn(obj, angle)      ToServer(BP_REQ_TURN, NULL, obj, angle)
#define RequestAction(action)        ToServer(BP_ACTION, NULL, action)
#define SendSayBlocked(id)           ToServer(BP_SAY_BLOCKED, NULL, id)
#define RequestChangeDescription(id, str) ToServer(BP_CHANGE_DESCRIPTION, NULL, id, str)
#define RequestActivate(id)          ToServer(BP_REQ_ACTIVATE, NULL, id)
#define RequestGamePing()            ToServer(BP_PING, NULL)
#define RequestChangeURL(obj, s)     ToServer(BP_USERCOMMAND, user_msg_table, UC_CHANGE_URL, obj, s)
#define RequestChangePassword(str1, str2) ToServer(BP_CHANGE_PASSWORD, NULL, str1, str2)
#define RequestRescue()              ToServer(BP_USERCOMMAND, user_msg_table, UC_REQ_RESCUE)
#define RequestRoundtrip(t)          ToServer(BP_ROUNDTRIP2, NULL, t)
#define RequestTime()                ToServer(BP_USERCOMMAND, user_msg_table, UC_REQ_TIME)

M59EXPORT void Logoff(void);
M59EXPORT void _cdecl ToServer(BYTE type, ClientMsgTable table, ...);

#endif /* #ifndef _PROTOCOL_H */
