# Themes

The client supports swappable UI color themes selected from the
Settings dialog under Interface Features.  Theme swaps take effect
without a restart.

Two themes ship today: `Theme::Default` (the original Meridian 59
palette) and `Theme::Dark` (a starter dark theme).

## Architecture

Theme definitions and the abstraction that hides them live in
`clientd3d/color.c` and `clientd3d/srvrstr.c`.  The rest of the client
asks for a color or brush by name (`GetColor(COLOR_X)`,
`GetBrush(COLOR_X)`) and is theme-blind.

The active theme is stored in `Config::theme` (the `Theme` enum in
`clientd3d/config.h`).  Each theme keeps its customized colors in its
own INI section (e.g. `[Colors]`, `[ColorsDark]`) so switching themes
does not clobber the other theme's saved values.
