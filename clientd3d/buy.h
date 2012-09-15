// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * buy.h:  Header file for buy.c
 */

#ifndef _BUY_H
#define _BUY_H

void AbortBuyDialog(void);
void BuyList(object_node seller, list_type items);
void WithdrawalList(object_node seller, list_type items);

#endif /* #ifndef _BUY_H */
