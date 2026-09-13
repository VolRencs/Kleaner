#!/bin/bash

rm -rf build translations/*.qm

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j $(nproc)

./build/stacer/stacer
