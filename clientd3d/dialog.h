// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * dialog.h:  Header for dialog.c and other dialog files (lookdlg.c, ...)
 */

#ifndef _DIALOG_H
#define _DIALOG_H

#define MAX_URL   200    // Maximum length of a URL

// Flags for DisplayLookList
#define LD_MULTIPLESEL 0x00000001    // True if multiple selections allowed
#define LD_AMOUNTS     0x00000002    // True if user should enter amounts for selected items
#define LD_SINGLEAUTO  0x00000004    // True if proc should return immediately if 1 item in list
#define LD_SORT        0x00000008    // True if items should be sorted alphabetically

/* Structure passed to LookDialogProc as lParam of WM_INITDIALOG message */
typedef struct {
   /* These fields should be set before creating dialog: */
   char     *title;          /* Title of dialog */
   list_type contents;       /* List of objects to display */
   int       flags;          // Flags for dialog; behavior; see LD_ above

   /* These fields are used internally in the dialog: */
   HWND hwndListBox, hwndQuanList, hwndFind;  /* Handles of child items */
   WNDPROC lpfnDefLookProc;     /* Default list box window procedure */   
   Bool   *selected;            /* Array of booleans; true if item at index is selected */
} LookDialogStruct;

// Flags for description dialog
#define DESC_NONE          0x00000000
#define DESC_GET           0x00000001      // Display "get" button
#define DESC_DROP          0x00000002      // Display "drop" button
#define DESC_USE           0x00000004      // Display "use" button
#define DESC_UNUSE         0x00000008      // Display "unuse" button
#define DESC_INSIDE        0x00000010      // Display "inside" button
#define DESC_ACTIVATE      0x00000020      // Display "use" (activate) button
#define DESC_APPLY         0x00000040      // Display "use" (apply) button

#define DESC_NOAGE         0x00000002      // Don't display age of player
#define DESC_NOSPECIAL     0x00000004      // Don't display special string of player
#define DESC_NOURL         0x00000008      // Don't display special string of player

/* Structure passed to DescDialogProc as lParam of WM_INITDIALOG message */
typedef struct {
   object_node *obj;
   BYTE         flags;
   char        *name;
   char        *description;
   char        *fixed_string;
   char        *url;
   int          age;
   int	        numPages;
   int	        currentPage;
   HFONT        hFontTitle;
} DescDialogStruct;

/* Structure passed to BuyDialogProc as lParam of WM_INITDIALOG message */
typedef struct {
   ID seller_id;             /* Object we're buying from */
   ID seller_name;           /* Name resource of object we're buying from */
   list_type items;          /* Items available to buy */
   DWORD cost;               /* Total cost of selected items */
   HWND hwndItemList, hwndCostList, hwndQuanList, hwndCost;
} BuyDialogStruct;

/* Structure passed to AmountDialogProc as lParam of WM_INITDIALOG message */
typedef struct {
   int x, y;                 /* Initial position of dialog */

   DWORD amount;             /* initially, # to display in edit box.  On exit,
				contains # that user entered */  
   DWORD maxAmount;	      /* maximum allowable value */
   DWORD minAmount;	      /* minimum allowable value */
} AmountDialogStruct;


M59EXPORT void SetDescParams(HWND hParent, int flags);
M59EXPORT void DisplayDescription(object_node *obj, BYTE flags, char *description, 
                                  char* fixed_string, char *url);
M59EXPORT void SetDialogFixedString(char* fixed_string);

M59EXPORT list_type DisplayLookList(HWND hParent, char *title, list_type l, int flags);

INT_PTR CALLBACK LookDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SayDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DescDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AmountDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void AbortGameDialogs(void);
void FilterChangedDescription(char *desc);

void AnimateDescription(int dt);

/* lookdlg.c */

void AbortLookList(void);
M59EXPORT Bool GetAmount(HWND hParent, HWND hwnd, object_node *obj, int x, int y);
M59EXPORT Bool InputNumber(HWND hParent, HWND hwnd, int x, int y, int *returnValue, int startValue, int minValue, int maxValue);
M59EXPORT Bool GetAmountListBox(HWND hList, int index);

#endif /* #ifndef _DIALOG_H */
