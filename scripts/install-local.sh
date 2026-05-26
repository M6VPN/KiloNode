#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/install-local.sh

set -eu

prefix=""

usage()
{
	echo "usage: $0 --prefix PATH [-- CMAKE_ARGS...]"
	echo "default displayed prefix: /usr/local"
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
		--help)
			usage
			exit 0
			;;
		--)
			shift
			break
			;;
		*)
			break
			;;
	esac
	shift
done

if [ -z "$prefix" ]; then
	usage
	echo "choose an explicit writable prefix, for example /tmp/kilonode-install"
	exit 2
fi

if [ "$prefix" = "/" ]; then
	echo "refusing prefix /"
	exit 2
fi

cmake -S . -B build-install -DCMAKE_BUILD_TYPE=Release "$@"
cmake --build build-install

if [ -n "${DESTDIR:-}" ]; then
	DESTDIR="${DESTDIR}" cmake --install build-install --prefix "$prefix"
else
	cmake --install build-install --prefix "$prefix"
fi

echo "installed under $prefix"
echo "for system service installs, run equivalent install commands as a privileged user"
