# Kleaner

Linux System Optimizer and Monitoring — a Qt 6 / KDE Frameworks 6 fork with a
Kirigami (QML) user interface.

> Kleaner is a fork of [QuentiumYT/Stacer](https://github.com/QuentiumYT/Stacer)
> that is being rewritten around Qt 6.11, KDE Frameworks 6 and Wayland.
> The project is currently packaged for Arch Linux only. Other distributions
> will follow later.

## Features

- **Dashboard** — live CPU, memory and disk usage rings, system information.
- **Resources** — rolling 60-second history charts for CPU usage, load
  average, memory/swap, disk I/O and network throughput.
- **Processes** — process list with CPU/memory usage, sorting, filtering,
  terminate and force-kill actions.
- **Services** — systemd units over D-Bus with start/stop/restart, enable and
  disable through polkit.
- **Startup Apps** — XDG autostart entries, including system entries from
  `/etc/xdg/autostart` (user overrides are created on demand).
- **System Cleaner** — trash, application caches, logs and crash reports.
  Privileged cleanup runs through a KAuth helper with an allowlist.
- **Hosts** — view and edit `/etc/hosts` with a KAuth helper writing the file
  atomically.

The user interface requires a Wayland session. Running under X11 is not
supported and not tested.

## Installation (Arch Linux)

Build and install with the provided PKGBUILD:

```bash
cd packaging/arch
makepkg -si
```

Or build manually:

```bash
sudo pacman -S --needed \
  base-devel cmake extra-cmake-modules \
  qt6-base qt6-declarative qt6-svg qt6-wayland qt6-tools \
  kirigami kquickcharts kcoreaddons kconfig kdbusaddons \
  kstatusnotifieritem kio kauth qqc2-desktop-style breeze-icons

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j $(nproc)
sudo cmake --install build
```

Run: `kleaner`

## Requirements

- systemd (services page via D-Bus)
- polkit agent (privileged cleaner and `/etc/hosts` writes)
- `qqc2-desktop-style` and an icon theme (`breeze-icons` recommended)

## Configuration

Settings are stored in `~/.config/kleanerrc` (KConfig). Translations are
installed to `/usr/share/kleaner/translations` and loaded according to the
system locale.

## Development

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j $(nproc)
./build/kleaner/kleaner
```

Translation catalogs live in `translations/` and can be refreshed with the
CMake targets:

```bash
cmake --build build --target update_translations
cmake --build build --target release_translations
```

## License

GPL-3.0. See [LICENSE](LICENSE). Kleaner is a fork of Stacer by Quentin
Lienhardt.
