// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * theme.c:  Theme-aware helpers for the merintr module: bitmap
 *   resource ID lookup and capability flags.
 */

#include "client.h"
#include "merintr.h"

/****************************************************************************/
/*
 * MerintrThemeResourceId:  Returns the variant of a merintr bitmap
 *   resource ID that matches the active theme.  Returns the input ID
 *   unchanged when no variant exists for the active theme.  Callers
 *   stay theme-blind.
 *
 *   See docs/themes.md.
 */
int MerintrThemeResourceId(int id)
{
   switch (ThemeCurrent())
   {
   case Theme::Dark:
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
   default:
      return id;
   }
}
/****************************************************************************/
/*
 * ThemeSidebarUsesInventoryFill:  Returns true when the active theme
 *   paints the right sidebar (enchantments, portrait, stats) with the
 *   inventory texture instead of the main window background.
 */
bool ThemeSidebarUsesInventoryFill(void)
{
   switch (ThemeCurrent())
   {
   case Theme::Dark: return true;
   default:          return false;
   }
}
/****************************************************************************/
/*
 * ThemeSkipStatsAreaFrame:  Returns true when the active theme does
 *   not draw the ornamental frame (corners and edge repeaters) around
 *   stats_area (the right-side tabbed panel).
 */
bool ThemeSkipStatsAreaFrame(void)
{
   switch (ThemeCurrent())
   {
   case Theme::Dark: return true;
   default:          return false;
   }
}
