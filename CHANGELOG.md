# Changelog

All notable changes to Kleaner are documented in this file.

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
