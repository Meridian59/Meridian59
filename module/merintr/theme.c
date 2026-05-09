// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * theme.c:  Theme-aware bitmap resource ID lookup for the merintr
 *   module.
 */

#include "client.h"
#include "merintr.h"

/****************************************************************************/
/*
 * ThemeResourceId:  Returns the variant of a merintr bitmap resource ID
 *   that matches the active theme.  When no override exists for the
 *   given ID (or the default theme is active), returns the input ID
 *   unchanged.  Callers stay theme-blind.
 */
int ThemeResourceId(int id)
{
   if (!ThemeIsDark())
      return id;

   switch (id)
   {
   case IDB_INVBKGND:   return IDB_INVBKGND_DARK;
   case IDB_EULTOP:     return IDB_EULTOP_DARK;
   case IDB_EULLEFT:    return IDB_EULLEFT_DARK;
   case IDB_EURTOP:     return IDB_EURTOP_DARK;
   case IDB_EURRIGHT:   return IDB_EURRIGHT_DARK;
   case IDB_ELLBOTTOM:  return IDB_ELLBOTTOM_DARK;
   case IDB_ELLLEFT:    return IDB_ELLLEFT_DARK;
   case IDB_ELRBOTTOM:  return IDB_ELRBOTTOM_DARK;
   case IDB_ELRRIGHT:   return IDB_ELRRIGHT_DARK;
   case IDB_EUREPEAT:   return IDB_EUREPEAT_DARK;
   case IDB_EBREPEAT:   return IDB_EBREPEAT_DARK;
   case IDB_ELREPEAT:   return IDB_ELREPEAT_DARK;
   case IDB_ERREPEAT:   return IDB_ERREPEAT_DARK;
   case IDB_MULTOP:     return IDB_MULTOP_DARK;
   case IDB_MULLEFT:    return IDB_MULLEFT_DARK;
   case IDB_MURTOP:     return IDB_MURTOP_DARK;
   case IDB_MURRIGHT:   return IDB_MURRIGHT_DARK;
   case IDB_MLLBOTTOM:  return IDB_MLLBOTTOM_DARK;
   case IDB_MLLLEFT:    return IDB_MLLLEFT_DARK;
   case IDB_MLRBOTTOM:  return IDB_MLRBOTTOM_DARK;
   case IDB_MLRRIGHT:   return IDB_MLRRIGHT_DARK;
   case IDB_MUREPEAT:   return IDB_MUREPEAT_DARK;
   case IDB_MLREPEAT:   return IDB_MLREPEAT_DARK;
   case IDB_MRREPEAT:   return IDB_MRREPEAT_DARK;
   case IDB_MBREPEAT:   return IDB_MBREPEAT_DARK;
   default:             return id;
   }
}
/****************************************************************************/
/*
 * ThemeSidebarUsesInventoryFill:  Returns true if the active theme paints
 *   the right sidebar (enchantments, portrait, stats) with the inventory
 *   texture rather than the main window background.  When true, UI
 *   elements drawn inside the sidebar should use the inventory texture
 *   as their background so they blend with the surrounding fill.
 */
bool ThemeSidebarUsesInventoryFill(void)
{
   return ThemeIsDark();
}
/****************************************************************************/
/*
 * ThemeSkipSilverFrame:  Returns true if the silver stats frame elements
 *   (corners and edge repeaters) should be skipped during interface
 *   element drawing.  Used by themes whose sidebar fill or color palette
 *   makes the silver artwork visually wrong.
 */
bool ThemeSkipSilverFrame(void)
{
   return ThemeIsDark();
}
