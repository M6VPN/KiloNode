#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/compat-replay-fixtures.sh

set -eu

tool="./build/kilonode-compat"
fixtures="tests/fixtures/compat"

if [ ! -x "$tool" ]; then
	echo "ERR missing $tool"
	echo "Build first with: ./scripts/build.sh"
	exit 1
fi

"$tool" replay-dir "$fixtures"
