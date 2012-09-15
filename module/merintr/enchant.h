// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * enchant.h:  Header file for enchant.c
 */

#ifndef _ENCHANT_H
#define _ENCHANT_H

#define ENCHANT_SIZE   20    // Height and width of enchantment icon
#define ENCHANT_BORDER 1     // Space between enchantment icons

void EnchantmentsInit(void);
void EnchantmentsDestroy(void);
void EnchantmentAdd(BYTE type, object_node *obj);
void EnchantmentRemove(BYTE type, ID obj_id);

void EnchantmentsResetData(void);
Bool EnchantmentDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis);
void EnchantmentsResize(int xsize, int ysize, AREA *view);
void EnchantmentsRedraw(void);
void EnchantmentsNewRoom(void);
void AnimateEnchantments(int dt);

#endif /* #ifndef _ENCHANT_H */

