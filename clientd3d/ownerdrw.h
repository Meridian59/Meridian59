// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * ownerdrw.h:  Header for ownerdrw.c
 */

#ifndef _OWNERDRW_H
#define _OWNERDRW_H

// Put these "style" flags in the GWL_USERDATA field of the owner drawn list box
// or combo box to modify the box's behavior.

#define OD_NONE          0x00000000        // Draw name of object only
#define OD_DRAWOBJ       0x00000001        // Draw entire object on left side of list box
#define OD_DRAWICON      0x00000002        // Draw specified icon from our image list
#define OD_ONLYSEL       0x00000004        // Draw object or icon only if selected
#define OD_COLORTEXT     0x00000008        // Draw text in specified special color
#define OD_NEXTICON      0x00000010        // Icon drawn is given index plus one
#define OD_ICONINDEXMASK 0x0000F000        // Index of icon to be drawn
#define OD_GETICONINDEX(style) (((style) & OD_ICONINDEXMASK) >> 12)
#define OD_MAKEICONINDEX(icon) (((icon) << 12) & OD_ICONINDEXMASK)

M59EXPORT void WindowBeginUpdate(HWND hwnd);
M59EXPORT void WindowEndUpdate(HWND hwnd);

/* Macros so that same procedures can be used for list boxes & combo boxes */
#define ItemListAddItem(hwnd, obj, index, quan)     OwnerListAddItem(hwnd, obj, index, false, quan)
#define ItemListRemoveItem(hwnd, id)          OwnerListRemoveItem(hwnd, id, false)
#define ItemListSetContents(hwnd, list, quan)       OwnerListSetContents(hwnd, list, false, quan)
#define ItemListChangeItem(hwnd, obj, quan)         OwnerListChangeItem(hwnd, obj, false, quan)
#define ItemListFindItem(hwnd, id)            OwnerListFindItem(hwnd, id, false)
#define ItemListFindObject(hwnd, id)          OwnerListFindObject(hwnd, id, false)
#define ItemListCompareItem(hwnd, lpcis)      OwnerListCompareItem(hwnd, lpcis, false)
#define ItemListMeasureItem(hwnd, lpcis)      OwnerListMeasureItem(hwnd, lpcis, false)
#define ItemListDrawItem(hwnd, lpdis)         OwnerListDrawItem(hwnd, lpdis, false)
#define ItemListDrawItemNoSelect(hwnd, lpdis) OwnerListDrawItemNoSelect(hwnd, lpdis, false)

#define ItemBoxAddItem(hwnd, obj, index)     OwnerListAddItem(hwnd, obj, index, true, false)
#define ItemBoxRemoveItem(hwnd, id)          OwnerListRemoveItem(hwnd, id, true)
#define ItemBoxSetContents(hwnd, list)       OwnerListSetContents(hwnd, list, true, false)
#define ItemBoxChangeItem(hwnd, obj)         OwnerListChangeItem(hwnd, obj, true)
#define ItemBoxFindItem(hwnd, id)            OwnerListFindItem(hwnd, id, true)
#define ItemBoxFindObject(hwnd, id)          OwnerListFindObject(hwnd, id, true)
#define ItemBoxCompareItem(hwnd, lpcis)      OwnerListCompareItem(hwnd, lpcis, true)
#define ItemBoxMeasureItem(hwnd, lpcis)      OwnerListMeasureItem(hwnd, lpcis, true)
#define ItemBoxDrawItem(hwnd, lpdis)         OwnerListDrawItem(hwnd, lpdis, true)
#define ItemBoxDrawItemNoSelect(hwnd, lpdis) OwnerListDrawItemNoSelect(hwnd, lpdis, true)


M59EXPORT ID ItemListGetId(HWND hList);

M59EXPORT int OwnerListAddItem(HWND hwnd, object_node *obj, int index, bool combo, bool quan);
/*M59EXPORT*/ void OwnerListRemoveItem(HWND hwnd, ID id, bool combo);
M59EXPORT void OwnerListSetContents(HWND hwnd, list_type contents, bool combo, bool quan);
M59EXPORT void OwnerListChangeItem(HWND hwnd, object_node *obj, bool combo, bool quan);
M59EXPORT object_node *OwnerListFindObject(HWND hList, ID id, bool combo);
M59EXPORT int OwnerListFindItem(HWND hList, ID id, bool combo);
M59EXPORT int OwnerListCompareItem(HWND hwnd, const COMPAREITEMSTRUCT *lpcis, bool combo);
M59EXPORT void OwnerListMeasureItem(HWND hwnd, MEASUREITEMSTRUCT *lpMeasureItem, bool combo);
M59EXPORT BOOL OwnerListDrawItem(HWND hwnd, const DRAWITEMSTRUCT *lpDrawItem, bool combo);
M59EXPORT BOOL OwnerListDrawItemNoSelect(HWND hwnd, const DRAWITEMSTRUCT *lpdis, bool combo);


#endif /* #ifndef _OWNERDRW_H */
