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
./build/kilonode-compat check-requirements tests/fixtures/compat/linbpq-node/requirements.plan
./build/kilonode-compat requirements-coverage tests/fixtures/compat/linbpq-node/requirements.plan tests/fixtures/compat/linbpq-node/manifest.pack
./build/kilonode-compat check-command-profiles tests/fixtures/compat/linbpq-node/command-profiles.plan
./build/kilonode-compat risk-report tests/fixtures/compat/linbpq-node/requirements.plan
./scripts/compat-check-node-plan.sh
./build/kilonode-compat check-observation tests/fixtures/compat/blackbox-node-help.observation
./build/kilonode-compat check-observation tests/fixtures/compat/blackbox-node-info.observation
./scripts/bench-rx-check-configs.sh
