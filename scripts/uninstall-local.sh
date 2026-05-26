#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/uninstall-local.sh

set -eu

dry_run=0
prefix=""

usage()
{
	echo "usage: $0 --prefix PATH [--dry-run]"
}

remove_file()
{
	path="$1"

	case "$path" in
		"$prefix"/*)
			;;
		*)
			echo "refusing path outside prefix: $path"
			exit 2
			;;
	esac

	if [ "$dry_run" -eq 1 ]; then
		echo "would remove $path"
	else
		if [ -e "$path" ]; then
			echo "remove $path"
			rm -f "$path"
		fi
	fi
}

while [ "$#" -gt 0 ]; do
	case "$1" in
		--prefix)
			shift
			if [ "$#" -eq 0 ]; then
				usage
				exit 2
			fi
			prefix="$1"
			;;
		--dry-run)
			dry_run=1
			;;
		--help)
			usage
			exit 0
			;;
		*)
			usage
			exit 2
			;;
	esac
	shift
done

if [ -z "$prefix" ] || [ "$prefix" = "/" ]; then
	echo "refusing empty or root prefix"
	exit 2
fi

for name in kilonode kilonode-monitor kilonode-msg kilonode-store \
	kilonode-user kilonodectl kilonoded; do
	remove_file "$prefix/bin/$name"
done

for page in kilonodectl.1 kilonode-monitor.1 kilonode-msg.1 \
	kilonode-store.1 kilonode-user.1; do
	remove_file "$prefix/share/man/man1/$page"
done

remove_file "$prefix/share/man/man8/kilonoded.8"
