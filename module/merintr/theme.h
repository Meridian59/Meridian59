// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * theme.h:  Theme-aware bitmap resource ID lookup and capability
 *   flags for the merintr module.  Used so callers can stay theme-blind:
 *   pass the canonical (default-theme) ID and get back the right
 *   variant for the active theme, or query the active theme's UI
 *   conventions through intent-named flags.
 */

#ifndef _MERINTR_THEME_H
#define _MERINTR_THEME_H

int MerintrThemeResourceId(int id);
bool ThemeSidebarUsesInventoryFill(void);
bool ThemeSkipStatsAreaFrame(void);

#endif /* #ifndef _MERINTR_THEME_H */
