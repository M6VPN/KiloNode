#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/compat-check-node-plan.sh

set -eu

tool="./build/kilonode-compat"
pack="tests/fixtures/compat/linbpq-node/manifest.pack"
requirements="tests/fixtures/compat/linbpq-node/requirements.plan"
profiles="tests/fixtures/compat/linbpq-node/command-profiles.plan"

if [ ! -x "$tool" ]; then
	echo "ERR missing $tool; run cmake -S . -B build && cmake --build build"
	exit 1
fi

"$tool" check-requirements "$requirements"
"$tool" requirements-coverage "$requirements" "$pack"
"$tool" check-command-profiles "$profiles"
"$tool" command-profile-report "$profiles"
"$tool" risk-report "$requirements"
