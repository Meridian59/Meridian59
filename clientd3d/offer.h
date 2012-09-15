// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * offer.h:  Header file for offer.c
 */

#ifndef _OFFER_H
#define _OFFER_H

typedef struct {
   list_type items;          /* List of offered items; nodes are from inventory */
   ID        receiver_name;  /* Recipient's name resource */
} SendOfferDialogStruct;

typedef struct {
   list_type items;          /* List of offered items; nodes are from server */
   ID        sender_name;    /* Sender's name resource */
} RcvOfferDialogStruct;

BOOL CALLBACK SendOfferDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK RcvOfferDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void OfferChangeFonts(void);
void OfferClose(void);
void OfferChangeColor(void);
void RaiseOfferWindow(void);
void ReceiveOffer(ID obj_id, ID icon_res, ID name_res, list_type items);
void Counteroffer(list_type items);
void Counteroffered(list_type items);
void OfferCanceled(void);
Bool OfferInProgress(void);

#endif /* #ifndef _OFFER_H */
