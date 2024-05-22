#!/bin/bash

git submodule update --init --recursive
echo "Submodules have been initialized and updated."

./vendor/vcpkg/bootstrap-vcpkg.sh
./vendor/vcpkg/vcpkg install

mkdir build
cmake -S . -B build
cmake --build build
