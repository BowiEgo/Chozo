#!/bin/bash

RED='\033[1;31m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
MAGENTA='\033[1;35m'
CYAN='\033[1;36m'
WHITE='\033[1;37m'
NC='\033[0m'

export PS4="${CYAN}-- ${NC}\$(date \"+%Y-%m-%d %H:%M:%S\")${CYAN} --[execute bash command]: \${FUNCNAME[0]:+\${FUNCNAME[0]}: }${NC}"

set -x

git submodule update --init --recursive
{
  set +x
} 2>/dev/null
echo "Submodules have been initialized and updated."

set -x

./tools/vcpkg/bootstrap-vcpkg.sh
./tools/vcpkg/vcpkg install

mkdir build
cmake -S . -B build
cmake --build build

{
  set +x
} 2>/dev/null