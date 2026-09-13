<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/header.png" width="800">    
</p>

<h2 align="center">Linux System Optimizer and Monitoring</h2>

<h3 align="center">
    <strong>
        <a href="https://stacer.quentium.fr/">See the website</a>
        &middot;
        <a href="https://launchpad.net/~quentiumyt/+archive/ubuntu/stacer">View on Launchpad</a>
        &middot;
        <a href="https://crowdin.com/project/stacer">Contribute to Crowdin translations</a>
    </strong>
</h3>

<p align="center">
	<a title="Kernel.org" target="_blank" href="https://www.kernel.org">
		<img alt="Platform (GNU/Linux)" src="https://img.shields.io/badge/platform-GNU/Linux-blue.svg"/>
	</a>
	<a title="GitHub Releases" target="_blank" href="https://github.com/QuentiumYT/Stacer/releases">
		<img alt="GitHub Releases Downloads" src="https://img.shields.io/github/downloads/QuentiumYT/Stacer/total.svg"/>
	</a>
    <a title="Crowdin Translations" target="_blank" href="https://crowdin.com/project/stacer">
        <img alt="Crowdin Localized Percentage" src="https://badges.crowdin.net/stacer/localized.svg">
    </a>
</p>

## Reviews

<p align="left">
    <a href="https://www.omgubuntu.co.uk/2017/01/stacer-system-optimizer-for-ubuntu">
		<img width="64px" src="https://149366088.v2.pressablecdn.com/wp-content/themes/omgubuntu-theme-2022_04_0/images/favicons/favicon-192x192.png"/>
	</a>        
    <a href="https://diolinux.com.br/sistemas-operacionais/ubuntu/stacer-um-programa-para-otimizar-o-ubuntu.html">
		<img width="64px" src="https://diolinux.com.br/wp-content/uploads/2021/05/diolinux-logo.png"/>
	</a>    
    <a href="https://www.dobreprogramy.pl/stacer-program-do-optymalizacji-ubuntu-ktory-wyglada-jakby-uciekl-z-windowsa,6628400543275137a">
		<img width="64px" src="https://www.dobreprogramy.pl/resources/icons/icon.png"/>
	</a>
    <a href="hhttps://blog.desdelinux.net/optimizar-debian-ubuntu-linux-mint-derivados-stacer/">
		<img width="64px" src="https://blog.desdelinux.net/wp-content/uploads/2018/04/cropped-desdelinux.png"/>
	</a>
	<a href="https://www.techrepublic.com/article/how-to-install-stacer-for-quick-linux-system-optimization/">
		<img width="64px" src="https://www.techrepublic.com/wp-content/themes/techrepublic-theme/inc/images/app-icons/android-chrome-256x256.png"/>
	</a>
</p>

## Installation

### Prerequisites

- curl
- systemd

### Ubuntu (PPA)

```bash
sudo add-apt-repository ppa:quentiumyt/stacer
sudo apt update
sudo apt install stacer
```
Run: `stacer`

### Debian/Ubuntu (.deb)

1. Download the `.deb` package from [Stacer releases](https://github.com/QuentiumYT/Stacer/releases).
2. Install dependencies:
   ```bash
   sudo apt install curl systemd libqt6core6 libqt6gui6 libqt6widgets6 libqt6charts6 libqt6svg6 libqt6concurrent6 libqt6network6
   ```
3. Install package:
   ```bash
   sudo apt install ./stacer_*.deb
   ```
4. Run: `stacer`

### Debian/Ubuntu (APT)

⚠️ Not up to date on every distro, a newer version is available already in [Debian forky](https://packages.debian.org/forky/stacer) or [Ubuntu resolute](https://packages.ubuntu.com/resolute/stacer).
Old stacer (version 1.1.0) is available up to Debian bookworm or Ubuntu questing.

```bash
sudo apt install stacer
```
Run: `stacer`

### Fedora (.rpm)

1. Download the `.rpm` package from [Stacer releases](https://github.com/QuentiumYT/Stacer/releases).
2. Install:
   ```bash
   sudo rpm --install stacer_*.rpm --nodeps --force
   ```
3. Run: `stacer`

### Fedora (DNF)

⚠️ Not up to date, this fork has not been accepted yet.
Old stacer (version 1.1.0) is available up to Fedora 42.

```bash
sudo dnf install stacer
```
Run: `stacer`

### Arch Linux (AUR)

Two maintainers propose this package up to date: [stacer-bin](https://aur.archlinux.org/packages/stacer-bin) or [stacer](https://aur.archlinux.org/packages/stacer).

Using your preferred AUR helper:
```bash
# yay
yay -S stacer-bin # OR stacer
# or paru
paru -S stacer-bin # OR stacer
```
Run: `stacer`

### CachyOS

Using your preferred package manager:
```bash
# pacman
sudo pacman -S stacer
# or paru
paru -S stacer
# or yay
yay -S stacer
```
Run: `stacer`

### Flatpak (Application bundle)

1. Download the `.flatpak` package from [Stacer releases](https://github.com/QuentiumYT/Stacer/releases).
2. Install:
   ```bash
   flatpak install fr.quentium.stacer_*-x86_64.flatpak # or aarch64
   ```
3. Run: `flatpak run fr.quentium.stacer`

> [!NOTE]
> Flatpak is running in a sandboxed environment, which may limit access to certain system resources (i.e. disk usage). Services, Processes, Uninstaller and APT Repository Manager are hidden by default.

### Flatpak (Flathub)

⚠️ Stacer couldn't be listed using it's original name. I choose an anagram for official publishing: Acters.
I recommend using the application bundle as it has an extra permission for system cleaner & startup apps to work.

```bash
flatpak install flathub fr.quentium.acters
```
Run: `flatpak run fr.quentium.acters`

> [!NOTE]
> Flatpak is running in a sandboxed environment, which may limit access to certain system resources (i.e. disk usage). Services, Processes, Uninstaller and APT Repository Manager are hidden by default. System Cleaner and Startup Apps are also limited in this version.

## Build from Source (CMake)

1. Install dependencies:
   ```bash
   sudo apt update
   sudo apt install \
    cmake \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qt6-l10n-tools \
    qt6-wayland-dev \
    qt6-charts-dev \ # Or previously libqt6charts6-dev
    qt6-svg-dev \ # Or previously libqt6svg6-dev
    libqt6opengl6-dev \
    libgl-dev \
    libxkbcommon-dev \
    libvulkan-dev
   ```

2. Build and run:
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
   cmake --build build -j $(nproc)
   ./build/stacer/stacer
   ```

## Screenshots

The Stacer Dashboard:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-dashboard.png" width="700" alt="The Stacer Dashboard">
</p>

The Stacer Dashboard on Ubuntu:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-dashboard-ubuntu.png" width="700" alt="The Stacer Dashboard on Ubuntu">
</p>

The Stacer Dashboard on WSL:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-dashboard-wsl.png" width="700" alt="The Stacer Dashboard on WSL">
</p>

The Stacer Startup Apps:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-startup-apps.png" width="700" alt="The Stacer Startup Apps">
</p>

The Stacer System cleaner:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-system-cleaner.png" width="700" alt="The Stacer System cleaner">
</p>

The Stacer System cleaner list:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-system-cleaner-list.png" width="700" alt="The Stacer System cleaner list">
</p>

The Stacer Search:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-search.png" width="700" alt="The Stacer Search">
</p>

The Stacer Services:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-services.png" width="700" alt="The Stacer Services">
</p>

The Stacer Processes:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-processes.png" width="700" alt="The Stacer Processes">
</p>

The Stacer Uninstaller:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-uninstaller.png" width="700" alt="The Stacer Uninstaller">
</p>

The Stacer Resources:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-resources.png" width="700" alt="The Stacer Resources">
</p>

The Stacer Resources usage:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-resources-usage.png" width="700" alt="The Stacer Resources usage">
</p>

The Stacer Resources system:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-resources-system.png" width="700" alt="The Stacer Resources system">
</p>

The Stacer Helpers:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-helpers.png" width="700" alt="The Stacer Helpers">
</p>

The Stacer APT Repository Manager:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-apt-repository-manager.png" width="700" alt="The Stacer APT Repository Manager">
</p>

The Stacer Gnome Settings:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-gnome-settings.png" width="700" alt="The Stacer Gnome Settings">
</p>

The Stacer Gnome Settings appearance:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-gnome-settings-appearance.png" width="700" alt="The Stacer Gnome Settings appearance">
</p>

The Stacer Settings:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-settings.png" width="700" alt="The Stacer Settings">
</p>

The Stacer Settings feedback:
<p align="center">
    <img src="https://raw.githubusercontent.com/QuentiumYT/Stacer/HEAD/screenshots/Screenshot-settings-feedback.png" width="700" alt="The Stacer Settings feedback">
</p>

## Contributors

This project exists thanks to all the people who contribute.

Thanks for people that contributes on translations on [Crowdin](https://crowdin.com/project/stacer)!

<a href="https://github.com/QuentiumYT/Stacer/graphs/contributors"><img src="https://opencollective.com/Stacer/contributors.svg?width=890&button=false" /></a>
