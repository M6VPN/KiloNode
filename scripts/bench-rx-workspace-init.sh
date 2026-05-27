#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/bench-rx-workspace-init.sh

set -eu

workspace="${1:-/tmp/kilonode-manual-captures}"

if [ "$#" -gt 1 ]; then
	echo "usage: $0 [workspace]"
	exit 1
fi

if [ ! -x ./build/kilonode-compat ]; then
	echo "build missing: run ./scripts/build.sh"
	exit 1
fi

./build/kilonode-compat manual-workspace-init "$workspace"
echo "next: ./scripts/bench-rx-workspace-import.sh INPUT.capture $workspace"
