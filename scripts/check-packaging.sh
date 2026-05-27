#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/check-packaging.sh

set -eu

status=0

require_file()
{
	if [ ! -f "$1" ]; then
		echo "missing packaging file: $1"
		status=1
	fi
}

require_section()
{
	file="$1"
	section="$2"

	if ! grep -q "^\\.Sh ${section}$" "$file"; then
		echo "missing man section ${section}: $file"
		status=1
	fi
}

check_config()
{
	file="$1"

	require_file "$file"
	if [ -x ./build/kilonoded ]; then
		if ! ./build/kilonoded --config "$file" --check-config >/dev/null; then
			echo "config failed validation: $file"
			status=1
		fi
	fi

	if grep -n -E 'host[[:space:]]+0\.0\.0\.0|host[[:space:]]+::' "$file"; then
		echo "public bind address found in example: $file"
		status=1
	fi
}

check_manpage()
{
	file="$1"

	require_file "$file"
	for section in NAME SYNOPSIS DESCRIPTION OPTIONS FILES EXAMPLES \
		"SECURITY NOTES" "SEE ALSO" AUTHORS LICENSE; do
		require_section "$file" "$section"
	done
}

require_file packaging/systemd/kilonoded.service
require_file packaging/openbsd/kilonoded.rc
require_file packaging/freebsd/kilonoded
require_file packaging/netbsd/kilonoded

check_config packaging/examples/kilonode.conf
check_config packaging/examples/kilonode-minimal.conf
check_config packaging/examples/kilonode-bbs-local.conf
check_config packaging/examples/kilonode-monitor-only.conf
check_config packaging/examples/kilonode-tx-test-only.conf
check_config packaging/examples/kilonode-tx-lab-only.conf

check_manpage docs/man/kilonoded.8
check_manpage docs/man/kilonodectl.1
check_manpage docs/man/kilonode-monitor.1
check_manpage docs/man/kilonode-msg.1
check_manpage docs/man/kilonode-user.1
check_manpage docs/man/kilonode-store.1

blocked_word="s""udo"
if grep -R -n "$blocked_word" packaging scripts/install-local.sh \
	scripts/uninstall-local.sh; then
	echo "packaging files must not contain privileged helper commands"
	status=1
fi

exit "$status"
