#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/compat-generate-node-plan.sh

set -eu

tool="./build/kilonode-compat"
pack="tests/fixtures/compat/linbpq-node/manifest.pack"
outdir="${1:-/tmp/kilonode-node-plan}"

if [ ! -x "$tool" ]; then
	echo "ERR missing $tool; run cmake -S . -B build && cmake --build build"
	exit 1
fi

mkdir -p "$outdir"

"$tool" generate-node-plan "$pack" \
	--requirements "$outdir/requirements.plan" \
	--profiles "$outdir/command-profiles.plan"
"$tool" check-requirements "$outdir/requirements.plan"
"$tool" check-command-profiles "$outdir/command-profiles.plan"
