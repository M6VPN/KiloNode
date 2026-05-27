#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/test.sh

set -eu

cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/kilonode-compat replay-dir tests/fixtures/compat
./build/kilonode-compat replay-capture-dir tests/fixtures/compat
./build/kilonode-compat check-pack tests/fixtures/compat/linbpq-node/manifest.pack
./build/kilonode-compat pack-coverage tests/fixtures/compat/linbpq-node/manifest.pack
./build/kilonode-compat replay-pack tests/fixtures/compat/linbpq-node/manifest.pack
./build/kilonode-compat check-observation tests/fixtures/compat/blackbox-node-help.observation
./build/kilonode-compat check-observation tests/fixtures/compat/blackbox-node-info.observation
