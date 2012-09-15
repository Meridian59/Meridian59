// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * charspel.c:  Handle spells tab page.
 */

#include "client.h"
#include "char.h"

static HWND hPoints;              // Handle of "points left" graph control
static HWND hList1, hList2;       // Handles of available and chosen spell list boxes

extern list_type spells;

static void MaybeEnableAddButton(HWND hDlg);
static void CharSpellsInit(HWND hDlg);
void CharSpellsCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNoify);
/********************************************************************/
BOOL CALLBACK CharSpellsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
   case WM_INITDIALOG:
      CharSpellsInit(hDlg);
      break;
      
   case WM_NOTIFY:
      switch (((LPNMHDR) lParam)->code)
      {
      case PSN_SETACTIVE:
         SendMessage(hPoints, GRPH_POSSET, 0, spell_points);
         break;
      }
      return TRUE;      

      HANDLE_MSG(hDlg, WM_COMMAND, CharSpellsCommand);
   }
   return FALSE;
}
/********************************************************************/
/*
 * CharSpellsInit:  Initialize spells tab page.  spells is a list of Spell structures.
 */
void CharSpellsInit(HWND hDlg)
{
   list_type l;
   int index;

   hPoints = GetDlgItem(hDlg, IDC_POINTSLEFT);
   SendMessage(hPoints, GRPH_COLORSET, GRAPHCOLOR_BAR, GetColor(COLOR_BAR2));
   SendMessage(hPoints, GRPH_COLORSET, GRAPHCOLOR_BKGND, GetColor(COLOR_BAR3));
   SendMessage(hPoints, GRPH_RANGESET, 0, SPELL_POINTS_INITIAL);

   // Set spell list contents
   hList1 = GetDlgItem(hDlg, IDC_SPELLIST1);
   hList2 = GetDlgItem(hDlg, IDC_SPELLIST2);
   for (l = spells; l != NULL; l = l->next)
   {
      Spell *s = (Spell *) (l->data);
      
      index = ListBox_AddString(hList1, LookupNameRsc(s->name_res));
      ListBox_SetItemData(hList1, index, s);
   }

   ListBox_SetCurSel(hList1, 0);
   FORWARD_WM_COMMAND(hDlg, IDC_SPELLIST1, hList1, LBN_SELCHANGE, CharSpellsDialogProc);
}
/********************************************************************/
/*
 * CharSpellsCommand:  Handle WM_COMMAND messages.
 */
void CharSpellsCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
   int index1, index2;
   Spell *s;
   char temp[MAXAMOUNT + 1];

   UserDidSomething();

   switch (id)
   {
   case IDC_ADDSPELL:
      index1 = ListBox_GetCurSel(hList1);
      if (index1 == LB_ERR)
         break;
      
      s = (Spell *) ListBox_GetItemData(hList1, index1);
      
      // See if enough points to get spell
      if (s->cost > spell_points)
         break;
      spell_points -= s->cost;
      SendMessage(hPoints, GRPH_POSSET, 0, spell_points);
      
      index2 = ListBox_AddString(hList2, LookupNameRsc(s->name_res));
      ListBox_SetItemData(hList2, index2, s);
      s->chosen = True;
      ListBox_DeleteString(hList1, index1);

      ListBox_SetCurSel(hList1, min(index1, ListBox_GetCount(hList1) - 1));
      FORWARD_WM_COMMAND(hwnd, IDC_SPELLIST1, hList1, LBN_SELCHANGE, CharSpellsDialogProc);
      break;
      
   case IDC_REMOVESPELL:
      index2 = ListBox_GetCurSel(hList2);
      if (index2 == LB_ERR)
         break;
      
      s = (Spell *) ListBox_GetItemData(hList2, index2);

      spell_points += s->cost;
      SendMessage(hPoints, GRPH_POSSET, 0, spell_points);

      index1 = ListBox_AddString(hList1, LookupNameRsc(s->name_res));
      ListBox_SetItemData(hList1, index1, s);
      s->chosen = False;
      ListBox_DeleteString(hList2, index2);

      ListBox_SetCurSel(hList2, min(index2, ListBox_GetCount(hList2) - 1));
      FORWARD_WM_COMMAND(hwnd, IDC_SPELLIST2, hList2, LBN_SELCHANGE, CharSpellsDialogProc);

      MaybeEnableAddButton(hwnd);
      break;

   case IDC_SPELLIST1:
      if (codeNotify != LBN_SELCHANGE)
         break;
      
      index1 = ListBox_GetCurSel(hList1);
      if (index1 == LB_ERR)
         break;
      
      s = (Spell *) ListBox_GetItemData(hList1, index1);
      SetDlgItemText(hwnd, IDC_SPELLINFO, LookupNameRsc(s->desc_res));
      sprintf(temp, "%d", s->cost);
      SetDlgItemText(hwnd, IDC_COST2, temp);

      MaybeEnableAddButton(hwnd);
      break;
      
   case IDC_SPELLIST2:
      if (codeNotify != LBN_SELCHANGE)
         break;
      
      index2 = ListBox_GetCurSel(hList2);
      if (index2 == LB_ERR)
         break;
      
      s = (Spell *) ListBox_GetItemData(hList2, index2);
      SetDlgItemText(hwnd, IDC_SPELLINFO, LookupNameRsc(s->desc_res));
      sprintf(temp, "%d", s->cost);
      SetDlgItemText(hwnd, IDC_COST2, temp);
      break;
      
   default:
      CharTabPageCommand(hwnd, id, hwndCtl, codeNotify);
      return;
   }
}
/********************************************************************/
/*
 * MaybeEnableAddButton: Enable/disable the "add spell" button
 *   depending on whether the currently selected spell in the available
 *   list box can be chosen.
 */
void MaybeEnableAddButton(HWND hDlg)
{
   int i;
   int school = 0;
   Spell *s;
   BOOL enable = TRUE;
   int index = ListBox_GetCurSel(hList1);
   if (index != LB_ERR)
   {
      // First find any chosen Qor/Shallile spells
      for (i = 0; i < ListBox_GetCount(hList2); ++i)
      {
         s = (Spell *) ListBox_GetItemData(hList2, i);
         if (s->school == SS_QOR || s->school == SS_SHALILLE)
            school = s->school;
      }
      
      // If school of selected spell conflicts, disable button
      s = (Spell *) ListBox_GetItemData(hList1, index);
      
      if (school == SS_QOR && s->school == SS_SHALILLE ||
          school == SS_SHALILLE && s->school == SS_QOR)
         enable = FALSE;
   }

   EnableWindow(GetDlgItem(hDlg, IDC_ADDSPELL), enable);
}
