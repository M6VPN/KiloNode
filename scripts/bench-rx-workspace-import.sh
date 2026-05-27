#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/bench-rx-workspace-import.sh

set -eu

if [ "$#" -lt 2 ]; then
	echo "usage: $0 INPUT.capture WORKSPACE [notes]"
	exit 1
fi

input="$1"
workspace="$2"
notes="${3:-manual-import}"

if [ ! -x ./build/kilonode-compat ]; then
	echo "build missing: run ./scripts/build.sh"
	exit 1
fi

./build/kilonode-compat manual-import "$input" --workspace "$workspace" \
	--notes "$notes"
