#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/m1-test-inventory.sh

set -eu

printf 'M1-TEST-INVENTORY tests\n'
find tests -maxdepth 1 -type f -name 'test_*.c' | sort |
while IFS= read -r file; do
	printf 'TEST source=%s\n' "$file"
done

if [ -d build ]; then
	find build -maxdepth 1 -type f -name 'test_*' -perm -111 | sort |
	while IFS= read -r file; do
		printf 'TEST-BIN path=%s\n' "$file"
	done
else
	printf 'TEST-BIN note=missing-build run="cmake --build build"\n'
fi

printf 'M1-TEST-INVENTORY scripts\n'
find scripts -maxdepth 1 -type f | sort |
while IFS= read -r file; do
	case "$file" in
	*.sh)
		printf 'SCRIPT path=%s executable=%s\n' "$file" \
		    "$([ -x "$file" ] && printf true || printf false)"
		;;
	esac
done

printf 'M1-TEST-INVENTORY fixtures\n'
find tests/fixtures -maxdepth 2 -type d | sort |
while IFS= read -r dir; do
	printf 'FIXTURE-DIR path=%s\n' "$dir"
done

printf 'OK m1-test-inventory hardware=false linbpq=false tx_dispatch_required=false\n'
