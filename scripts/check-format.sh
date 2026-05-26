#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/check-format.sh

set -eu

if ! command -v clang-format >/dev/null 2>&1; then
	echo "check-format: clang-format not found, skipping"
	exit 0
fi

if [ ! -f .clang-format ]; then
	echo "check-format: .clang-format not found, skipping"
	exit 0
fi

find include src tests -type f \( -name '*.c' -o -name '*.h' \) \
	-exec clang-format --dry-run --Werror {} +
