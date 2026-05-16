# Themes

The client supports swappable UI themes selected from the Settings dialog under Interface Features.  Theme swaps take effect without a restart.

Two themes ship today: `Theme::Default` (the original Meridian 59 palette) and `Theme::Dark` (a dark theme).

A theme covers two kinds of surface: colors and bitmaps.

## Color theming

Theme definitions and the abstraction that hides them live in `clientd3d/color.c` and `clientd3d/srvrstr.c`.  The rest of the client asks for a color or brush by name (`GetColor(COLOR_X)`, `GetBrush(COLOR_X)`) and is theme-blind.

The active theme is stored in `Config::theme` (the `Theme` enum in `clientd3d/config.h`).  Each theme keeps its customized colors in its own INI section (e.g. `[Colors]`, `[ColorsDark]`) so switching themes does not clobber the other theme's saved values.

## Bitmap surfaces

Bitmap theming is per-surface.  A surface is themed when a `_DARK` (or other theme-specific) variant exists and the code path routes the ID through a resolver.  Surfaces without a variant render the default art under every theme.

Three kinds of themeable bitmap surface exist:

- Tiled backgrounds: the main window background and the inventory texture.
- Wrapper ornaments: decorative corner + edge-repeater bitmaps drawn around the major UI regions.
- Button and icon art: toolbar buttons, stat-tab buttons, mailbox icon.

The wrapper ornaments live in `module/merintr/drawint.c`.  Five groups exist today:

| Group | Wraps | Dark variants today |
| ----- | ----- | ------------------- |
| `E*` | Outer client-window edge | Yes |
| `M*` | Minimap | Yes |
| `S*` | Stats area outer panel | No (skipped in dark) |
| `B*` | Chat edit box | No (silver in dark) |
| `I*` | Inventory area | No (skipped for every theme) |

## Sidebar fill

The right sidebar (enchantments, portrait, stat bars) sits inside the main window.  Two fill strategies exist for the sidebar area:

- Show the main window background through any gaps between drawn elements.
- Paint the sidebar with the inventory texture so it has its own fill.

Each theme picks one.  The choice depends on whether the main window background reads well behind the portrait and stat bars.  For example, the dark theme paints the inventory texture across the sidebar because the dark main window background lacks contrast against the portrait; the default theme leaves the main window background visible.

The choice lives in `ThemeSidebarUsesInventoryFill` in `module/merintr/theme.c`.  It is a switch over `ThemeCurrent()` that returns true for themes that paint with the inventory texture, false for the rest.  To activate the inventory fill for a new theme, add a `case Theme::<NewName>: return true;` to that switch.

Two paint functions read the answer:

- `InterfaceDrawSidebarBackground` paints the entire sidebar area in one pass.
- `OffscreenSidebarBackground` paints a single rectangle on the sidebar.  Used for per-element redraws (portrait, stat icons, enchantment buttons).

Both return without painting when `ThemeSidebarUsesInventoryFill` returns false.

```mermaid
flowchart LR
    A[Caller paints<br/>a sidebar region] --> P[OffscreenSidebarBackground<br/>or InterfaceDrawSidebarBackground]
    P --> Q{ThemeSidebarUsesInventoryFill?}
    Q -->|true| F[Fill with inventory texture]
    Q -->|false| D[Return without painting]

    style A fill:#1971c2,color:#fff
    style P fill:#2f9e44,color:#fff
    style Q fill:#b08000,color:#fff
    style F fill:#333,color:#fff
    style D fill:#333,color:#fff
```

## Per-module bitmap resolvers

Each module owns its own bitmap IDs in its own `resource.h`.  The ID values are not shared across modules, so each module ships its own resolver:

| Module | Resolver | Themed bitmaps today |
| ------ | -------- | -------------------- |
| `clientd3d` | `MainThemeResourceId` | Main window background |
| `module/merintr` | `InterfaceThemeResourceId` | Inventory texture; window-edge and minimap wrapper ornaments |

Other client modules (`admin`, `char`, `chess`, `dm`, `mailnews`) contain bitmaps but have no themed variants today.

A resolver takes a default-theme bitmap ID and returns the variant for the active theme.  If the active theme has no variant for that ID, the resolver returns the input unchanged.

```mermaid
flowchart LR
    A[Caller in clientd3d] --> M[MainThemeResourceId]
    B[Caller in merintr] --> R[InterfaceThemeResourceId]
    M --> Q{variant for<br/>active theme?}
    R --> Q
    Q -->|yes| V[return themed ID]
    Q -->|no| D[return input ID]

    style A fill:#1971c2,color:#fff
    style B fill:#1971c2,color:#fff
    style M fill:#2f9e44,color:#fff
    style R fill:#2f9e44,color:#fff
    style Q fill:#b08000,color:#fff
    style V fill:#333,color:#fff
    style D fill:#333,color:#fff
```

Example: adding a dark variant for an existing bitmap.

1. Add `IDB_X_DARK` next to `IDB_X` in the same module's `.rc` file
2. Add a `case IDB_X: return IDB_X_DARK;` to the resolver in the same module

Call sites pass the canonical ID through the resolver:

```c
LoadBitmap(hInst, MAKEINTRESOURCE(InterfaceThemeResourceId(IDB_X)));
```

## Adding a new theme

The major components are:

1. **Theme enum.**  Add the new value to the `Theme` enum in `clientd3d/config.h`.
2. **Color tables.**  In `clientd3d/color.c`, add a `colorinfo_<name>[]` table and an INI section string.  Extend `ColorDefaultsForTheme` and `ColorSectionForTheme`.  Bump `THEME_COLOR_VERSION`.
3. **Server message codes (optional).**  In `clientd3d/srvrstr.c`, add a `code_table_<name>[]` and extend `ColorCodeTableForTheme`.
4. **Settings UI.**  Add a localized string (`IDS_THEME_<NAME>`) in `clientd3d/client.rc` for each locale.  Append it to the `IDC_THEME` combo in `clientd3d/preferences.c`.
5. **Bitmap variants (optional).**  Author `_<NAME>` BMP variants.  In `MainThemeResourceId` (`clientd3d/color.c`) or `InterfaceThemeResourceId` (`module/merintr/theme.c`), add a `case Theme::<NewName>:` block with an inner switch over bitmap IDs.
6. **Capability flags.**  In `module/merintr/theme.c`, add a `case Theme::<NewName>: return ...;` to each yes/no function (`ThemeSidebarUsesInventoryFill`, `ThemeSkipStatsAreaFrame`).
