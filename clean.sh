#!/bin/bash
# SPDX-FileCopyrightText: 2026 VolRen
# SPDX-License-Identifier: GPL-3.0-only

set -euo pipefail

rm -rf build build-* release pkg src pkgdir
rm -f translations/*.qm

# Remove artifacts of any in-source build without touching .git.
find . -path ./.git -prune -o -name CMakeFiles -type d -print0 | xargs -0 -r rm -rf
find . -path ./.git -prune -o -name '*_autogen' -type d -print0 | xargs -0 -r rm -rf
find . -path ./.git -prune -o -name cmake_install.cmake -type f -delete
find . -path ./.git -prune -o -name CMakeCache.txt -type f -delete
find . -path ./.git -prune -o -name build.ninja -type f -delete
