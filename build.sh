#!/bin/bash
# SPDX-FileCopyrightText: 2026 VolRen
# SPDX-License-Identifier: GPL-3.0-only

set -euo pipefail

rm -rf build

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j "$(nproc)"
ctest --test-dir build --output-on-failure

./build/bin/kleaner
