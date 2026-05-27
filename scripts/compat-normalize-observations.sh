#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/compat-normalize-observations.sh

set -eu

if [ "$#" -ne 1 ]; then
	echo "usage: compat-normalize-observations.sh DIR"
	exit 1
fi

tool="./build/kilonode-compat"
dir="$1"

if [ ! -x "$tool" ]; then
	echo "ERR missing $tool"
	echo "Build first with: ./scripts/build.sh"
	exit 1
fi

find "$dir" -type f -name '*.observation' | sort | while IFS= read -r file; do
	"$tool" check-observation "$file"
	"$tool" make-transcript-from-observation "$file" --output "${file}.candidate"
done
