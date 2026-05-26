#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/check-portability.sh

set -eu

status=0

scan()
{
	label="$1"
	pattern="$2"
	shift 2

	if grep -R -n -E "$pattern" "$@"; then
		echo "check-portability: $label"
		status=1
	fi
}

scan "Linux-only headers need guards" \
	'#include[[:space:]]+<((linux/)|sys/(epoll|inotify)\.h)' \
	include src tests

scan "GNU-only functions need wrappers" \
	'\b(strdupa|asprintf|vasprintf|getline|getrandom)\b' \
	include src tests

scan "unsafe string functions are not allowed" \
	'\b(strcpy|strcat|sprintf|gets)\b' \
	include src tests

blocked_word="s""udo"
if grep -R -n "$blocked_word" scripts --exclude='check-portability.sh'; then
	echo "check-portability: privileged helper usage found in scripts"
	status=1
fi

exit "$status"
