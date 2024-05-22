#!/bin/bash

git submodule update --init --recursive
echo "Submodules have been initialized and updated."

./tools/vcpkg/bootstrap-vcpkg.sh
./tools/vcpkg/vcpkg install

mkdir build
cmake -S . -B build
cmake --build build
