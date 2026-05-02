# Themes

The client supports swappable UI color themes.  This document describes
the architecture, how to add a theme, and how the saved-color version
mechanism works.

## Status

This is the initial theming infrastructure.  Two themes ship today:

- `THEME_DEFAULT` (0): the original Meridian 59 palette.  Unchanged
  visual behavior.
- `THEME_DARK` (1): a starter dark theme that recolors only the main
  background and the chat edit box.  Dialog list backgrounds, stat
  bars, bitmaps, and most other UI surfaces still use the default
  look.  Those land in follow-up work, one surface at a time.

The user picks a theme from the Settings dialog under Interface
Features.  Swaps take effect immediately without a restart.

## How a theme is selected

The active theme is stored in `Config::theme` (an `int` matching the
`THEME_*` enum in `clientd3d/config.h`).  It is loaded from the
`[Interface] Theme=N` key in `meridian.ini` via `ConfigLoad` and saved
back via `ConfigSave`.

`LoadSettings` calls `ConfigLoad` before `ColorsCreate` so the theme
is known at the moment the color table is built.

When the user picks a different theme in Settings, `ThemeApply` is
called.  It tears down the color table, rebuilds it for the new
theme, refires `EVENT_COLORCHANGED`, and invalidates the main window.

## Color tables

Each theme has its own parallel table of `MAXCOLORS` entries, indexed
by the `COLOR_*` enum from `clientd3d/color.h`:

- `colorinfo_default[]`
- `colorinfo_dark[]`

Entries are `"R,G,B"` strings.  They are strings (not `RGB()` macros)
because they are written verbatim to the INI file when the user
customizes a color via the color picker.

Two helpers select the active table at runtime:

- `ColorSectionForTheme(int)` -> INI section name
- `ColorDefaultsForTheme(int)` -> compiled default table

## INI sections

Each theme stores its customized colors in its own INI section so
switching themes does not clobber the other theme's saved values:

- `[Colors]` for `THEME_DEFAULT`
- `[ColorsDark]` for `THEME_DARK`

Each section contains `Color0`..`ColorN-1` entries plus a
`ColorVersion=N` stamp.

## Color version stamping

`THEME_COLOR_VERSION` lives in `color.c`.  When `ColorsCreate` runs,
it reads the `ColorVersion` key from the active theme's section.  If
the saved version does not match the compiled version, the saved
colors are discarded and the compiled defaults are used.

Bump `THEME_COLOR_VERSION` whenever a default value in either color
table changes.  Without the bump, users who already have a saved
section will keep seeing their stale per-user values instead of the
new defaults.

## Palette snap (default theme only)

The default theme runs against the original 256-color game palette.
`GetColor` returns `MAKEPALETTERGB(...)` and `SetColor` calls
`GetNearestPaletteColor(...)` so colors snap to the closest indexed
entry.  Other themes pass raw 24-bit RGB through
(`color & 0x00FFFFFF`) so dark colors are not collapsed against a
palette they were never tuned for.

## Dialog scoping

Dialog edit boxes and listboxes do not pick up theme colors.
`DialogCtlColor` returns stock white/black brushes for any non-default
theme.  Only `MainCtlColor` (the main window and its chat edit box)
reads from the theme palette.  This keeps the infrastructure PR
minimal and avoids dragging dialog backgrounds into theme work that
would need owner-draw changes per surface.

The chat scrollback window uses `MainCtlColor` and so does pick up
theme colors.  Server message colors (the `^r ^g ^b ...` codes
parsed by `srvrstr.c::DisplayMessage`) have a parallel
`code_table_dark[]` chosen at runtime by `ColorCodeTableForTheme`.

## Adding a new theme

1. Add an entry to the `THEME_*` enum in `clientd3d/config.h`.
2. Add a `colorinfo_<name>[]` table and an INI section name string
   in `clientd3d/color.c`.
3. Extend `ColorDefaultsForTheme` and `ColorSectionForTheme` with a
   case for the new value.
4. Optionally add a `code_table_<name>[]` and extend
   `ColorCodeTableForTheme` in `clientd3d/srvrstr.c`.
5. Add a localized display string (e.g. `IDS_THEME_<NAME>`) in
   `clientd3d/client.rc` for each locale and append it to the
   `IDC_THEME` combo in `preferences.c::CommonPreferencesDlgProc`.
6. Bump `THEME_COLOR_VERSION` so existing users pick up your defaults.

## Flow at a glance

```
+----------------+       +----------------+       +-------------------+
|  meridian.ini  | ----> |  ConfigLoad    | ----> |  config.theme     |
|  [Interface]   |       |                |       |                   |
|  Theme=N       |       +----------------+       +---------+---------+
+----------------+                                          |
                                                            v
+----------------+       +----------------+       +-------------------+
|  [Colors]      | ----> |  ColorsCreate  | ----> |  colors[] table   |
|  [ColorsDark]  |       |  (per theme)   |       |                   |
|  ColorVersion  |       +----------------+       +---------+---------+
+----------------+                                          |
                                                            v
+----------------+       +----------------+       +-------------------+
|  Settings UI   | ----> |  ThemeApply    | ----> |  repaint window   |
|  IDC_THEME     |       |                |       |                   |
+----------------+       +----------------+       +-------------------+
```
