#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/compat-validate-captures.sh

set -eu

tool="./build/kilonode-compat"

if [ ! -x "$tool" ]; then
	printf '%s\n' "error: missing $tool"
	printf '%s\n' "build first: ./scripts/build.sh"
	exit 1
fi

"$tool" replay-capture-dir tests/fixtures/compat
