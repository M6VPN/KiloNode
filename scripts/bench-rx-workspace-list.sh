#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/bench-rx-workspace-list.sh

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

./build/kilonode-compat manual-list --workspace "$workspace"
