#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/test.sh

set -eu

cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/kilonode-compat replay-dir tests/fixtures/compat
