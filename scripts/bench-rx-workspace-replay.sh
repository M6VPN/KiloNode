#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/bench-rx-workspace-replay.sh

set -eu

workspace="${1:-/tmp/kilonode-manual-captures}"
id="${2:-all}"

if [ "$#" -gt 2 ]; then
	echo "usage: $0 [workspace] [id]"
	exit 1
fi

if [ ! -x ./build/kilonode-compat ]; then
	echo "build missing: run ./scripts/build.sh"
	exit 1
fi

if [ "$id" = "all" ]; then
	./build/kilonode-compat manual-replay-all --workspace "$workspace"
else
	./build/kilonode-compat manual-replay "$id" --workspace "$workspace"
fi
