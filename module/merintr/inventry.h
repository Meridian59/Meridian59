// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * inventry.h:  Header file for inventry.c
 */

#ifndef _INVENTRY_H
#define _INVENTRY_H

#define INVENTORY_OBJECT_HEIGHT 40
#define INVENTORY_OBJECT_WIDTH  40
#define INVENTORY_OBJECT_BORDER 2
#define INVENTORY_BOX_WIDTH  (INVENTORY_OBJECT_WIDTH  + 2 * INVENTORY_OBJECT_BORDER)
#define INVENTORY_BOX_HEIGHT (INVENTORY_OBJECT_HEIGHT + 2 * INVENTORY_OBJECT_BORDER)

void InventoryBoxCreate(HWND hParent);
void InventoryBoxDestroy(void);
void InventoryBoxResize(int xsize, int ysize, AREA *view);
void InventoryRedraw(void);
Bool InventoryDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpdis);
void InventoryResetFont(void);
void InventoryChangeColor(void);
void InventorySetFocus(Bool forward);
void InventoryChangeItem(object_node *obj);
void InventorySelChange(void);
void InventoryGetArea(AREA *area);
void InventoryResetData(void);
Bool InventoryMouseCaptured(void);

void InventoryAddItem(object_node *obj);
void InventoryRemoveItem(ID id);
void DisplayInventory(list_type inventory);

void DisplaySetUsing(ID obj_id, Bool is_using);
void DisplayUsing(list_type using_list);

void AnimateInventory(int dt);

void ShowInventory( Bool bShow );
HWND GetHwndInv();
RawBitmap* pinventory_bkgnd();

#endif /* #ifndef _INVENTRY_H */
