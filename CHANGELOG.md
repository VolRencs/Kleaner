# Changelog

All notable changes to Kleaner are documented in this file.

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
