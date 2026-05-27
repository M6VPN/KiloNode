#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/compat-validate-observation-pack.sh

set -eu

tool="./build/kilonode-compat"
pack="tests/fixtures/compat/linbpq-node/manifest.pack"

if [ ! -x "$tool" ]; then
	printf '%s\n' "error: missing $tool"
	printf '%s\n' "build first: ./scripts/build.sh"
	exit 1
fi

"$tool" check-pack "$pack"
"$tool" pack-coverage "$pack"
"$tool" replay-pack "$pack"
