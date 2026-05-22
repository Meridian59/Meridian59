// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * theme.c:  Theme-aware helpers for the interface module: bitmap
 *   resource ID lookup and capability flags.
 */

#include "client.h"
#include "merintr.h"

/****************************************************************************/
/*
 * InterfaceThemeResourceId:  Returns the variant of an interface
 *   bitmap resource ID that matches the active theme.  Returns the
 *   input ID unchanged when no variant exists for the active theme.
 *   Callers stay theme-blind.
 *
 *   See docs/themes.md
 */
int InterfaceThemeResourceId(int id)
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
      case IDB_HELP:       return IDB_HELP_DARK;
      case IDB_DROP:       return IDB_DROP_DARK;
      case IDB_GET:        return IDB_GET_DARK;
      case IDB_REST:       return IDB_REST_DARK;
      case IDB_SBUTTON1_LEFT:  return IDB_SBUTTON1_LEFT_DARK;
      case IDB_SBUTTON2_LEFT:  return IDB_SBUTTON2_LEFT_DARK;
      case IDB_SBUTTON3_LEFT:  return IDB_SBUTTON3_LEFT_DARK;
      case IDB_SBUTTON4_LEFT:  return IDB_SBUTTON4_LEFT_DARK;
      case IDB_SBUTTON1_MID:
      case IDB_SBUTTON2_MID:
      case IDB_SBUTTON3_MID:
      case IDB_SBUTTON4_MID:   return IDB_SBUTTON_MID_DARK;
      case IDB_SBUTTON1_RIGHT:
      case IDB_SBUTTON2_RIGHT:
      case IDB_SBUTTON3_RIGHT:
      case IDB_SBUTTON4_RIGHT: return IDB_SBUTTON_RIGHT_DARK;
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
