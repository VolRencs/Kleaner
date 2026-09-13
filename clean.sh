#!/bin/bash

rm -rf build release
rm -f translations/*.qm

find . -name CMakeFiles -type d -prune -exec rm -rf {} +
find . -name '*_autogen' -type d -prune -exec rm -rf {} +
find . -name cmake_install.cmake -delete
find . -name CMakeCache.txt -delete
find . -name build.ninja -delete
