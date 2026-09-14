# Changelog

All notable changes to Kleaner are documented in this file.

## 1.3.1 — 2026-09-14

- Fixed the storage usage bars on the dashboard flickering: the disk list no
  longer rebuilds its delegate items on every monitoring tick, so the bars now
  animate smoothly from their current value instead of resetting to zero and
  growing again.
- Build cleanup: disabled the QML import scan (the QML plugins are loaded at
  runtime) and declared `wayland-protocols` as a build dependency.

## 1.3.0 — 2026-09-14

The interface was rebuilt on top of the Kirigami application framework and the
whole application now shares one consistent, style-independent look.

- The window is now a `Kirigami.ApplicationWindow` with a `PageRow`, a permanent
  non-modal sidebar and a `PagePool`, so pages keep their state (scroll
  position, filters, selections) when switching between them. The hand-made
  scroll view and its re-parented scrollbar were removed.
- The Qt Quick Controls style is pinned to `Basic`, so the application looks
  identical no matter where it is launched from or which platform style is
  installed.
- One dark theme instead of three competing palettes: the full Kirigami colour
  set is derived from the `Design` tokens, and the app palette lives only in
  `main.cpp`.
- New shared controls: styled inline messages with a rounded close button,
  confirmation dialogs with a destructive button variant, separators, and
  placeholder messages with helpful actions for every empty state.
- Buttons received a tonal primary variant, and the refresh buttons show a
  Kirigami busy indicator instead of a rotated, badly scaled icon.
- Dialogs use `Kirigami.FormLayout`, open with the first field focused and
  wrap long text; the settings combo boxes no longer lose their bindings.
- Charts now fill every series with the same translucent area as KDE System
  Monitor, use the system font and locale-aware numbers, and only sample the
  resource history while the charts are visible.
- The system cleaner reports progress and results through the shared inline
  message, destructive actions ask for confirmation, and the hosts editor
  tracks unsaved changes.
- Window size is remembered between sessions, the window title reflects the
  current page, search fields support Ctrl+F, and the language selector shows
  "English" instead of "American English".
- Russian and Ukrainian translations were updated for all new strings.

## 1.2.2 — 2026-09-14

- Fixed the system cleaner getting stuck on "Cleaning…" when more than one
  user-owned path was selected (for example application caches): the completion
  counter was incremented per path but decremented once for the whole batch, so
  the finished signal never arrived.

## 1.2.1 — 2026-09-14

- Fixed the interface language selector not offering English explicitly:
  English is the source language and was previously only reachable through the
  system locale fallback.

## 1.2.0 — 2026-09-14

- Fixed disk I/O rates being stuck at zero: `/sys/block` entries are symlinks
  and were skipped by the device enumeration.
- Fixed network interface detection ignoring route flags and preferring tunnel
  interfaces over the physical default route.
- The hosts editor keeps the last known good file if `/etc/hosts` cannot be
  read and refuses to save over external modifications instead of replacing the
  file with a partial list.
- The privileged helper validates canonical paths, only removes packages that
  are still actual orphans and reports partial results when a step fails.
- User-owned cleanup paths are deleted off the GUI thread, so large caches no
  longer freeze the window.
- Process CPU usage for newly seen and reused PIDs is baselined instead of
  attributing a whole lifetime of CPU time to one sample.
- The system cleaner selection is synced to disk immediately, so it survives a
  crash.
- Startup entries are written into the existing `[Desktop Entry]` group;
  creation uses an exclusive file name claim.
- Interface polish: KDE-style inset rounded highlights for list rows, the
  sidebar and sort headers, with the overlay scrollbar kept clear of both the
  card corners and the highlight.
- The refresh buttons on Processes, Services and Startup Apps spin their icon
  and are disabled while loading; startup entries are now scanned off the GUI
  thread.
- The Resources legend is more compact and uses KDE-style colour bars; CPU
  cores are coloured with the same hue-wheel algorithm as KDE System Monitor,
  so no two cores share a colour.
- Dead code and unused dependencies removed (`kquickcharts`, `qt6-wayland`,
  `Qt6::Network`, unused properties and signals); QML delegates use
  `pragma ComponentBehavior: Bound` with required properties.
- Build system, CI, packaging and translations polished: modern CMake targets,
  pinned GitHub Actions, PKGBUILD checks, only complete Russian and Ukrainian
  catalogs are shipped.

## 1.1.0 — 2026-09-14

- Resources: per-core CPU history on a single chart with translucent gradient
  areas, live min/avg/max statistics and a legend with current values.
  Hovering a legend entry highlights the corresponding series.
- System Cleaner: the whole privileged cleanup (files, orphan packages and the
  systemd journal) now runs as one KAuth action, so a single password prompt is
  shown. The journal is rotated and vacuumed completely.
- System Cleaner: new temporary files (`/tmp`) category that skips session
  sockets, per-service private directories and files touched within the last
  hour.
- System Cleaner: the exact freed size and the number of removed items are
  reported in a summary at the bottom of the page.
- Processes: the automatic two-second refresh re-sorts the list, so the busiest
  processes move to the top; the scroll offset and the selected process are
  still preserved.
- Fixed privileged cleanup and hosts saving that never ran because the KAuth
  jobs were not started.
- Fixed the interface language and close-behaviour selectors falling back to
  the first entry after a restart.
- Fixed launching from the application menu by registering the correct D-Bus
  name, and the duplicate "Quit" entry in the tray menu.
- Scrollbars are thin themed overlays on the window edge that no longer reserve
  layout space, and buttons centre their contents.

## 1.0.0 — 2026-09-14

First release of Kleaner as an independent project.

- Complete UI rewrite on Qt Quick Controls / QML with a modern dark design
  system (`Design`, `App*` controls, sidebar navigation).
- Dashboard with live CPU, memory and disk gauges, storage breakdown and
  system information.
- Resources page with persistent 60-second history for CPU, load, memory,
  swap, disk and network.
- Processes page: sortable columns, filtering, manual refresh and a
  non-intrusive two-second auto-refresh that keeps the scroll position.
- Services page: systemd units over D-Bus, sorting by name/state/startup,
  start/stop/restart and enable/disable through polkit.
- System cleaner: trash, application caches, system logs, systemd journal,
  pacman package cache, orphan packages and crash reports. Selections persist
  between sessions and privileged cleanup runs through KAuth.
- Hosts editor with atomic writes through the KAuth helper.
- Startup applications editor with atomic `.desktop` writes.
- System tray support with a compact vector icon.
- Runtime language switching; complete Russian and Ukrainian translations.
- Packaging: Arch Linux PKGBUILD, AppStream metadata and desktop entry.
