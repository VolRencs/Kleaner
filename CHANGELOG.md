# Changelog

All notable changes to Kleaner are documented in this file.

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
