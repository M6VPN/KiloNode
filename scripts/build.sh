#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/build.sh

set -eu

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug "$@"
cmake --build build
