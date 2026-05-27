#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/bench-rx-import-capture.sh

set -eu

allow_repo_fixture=false

if [ "${1:-}" = "--allow-repo-fixture" ]; then
	allow_repo_fixture=true
	shift
fi

if [ "$#" -ne 2 ]; then
	echo "usage: $0 [--allow-repo-fixture] INPUT.capture DESTDIR"
	exit 1
fi

input="$1"
dest="$2"
name="$(basename "$input")"

case "$input" in
	*.capture)
		;;
	*)
		echo "ERR input must end with .capture"
		exit 1
		;;
esac

case "$dest" in
	*..*|*\\*)
		echo "ERR unsafe destination"
		exit 1
		;;
esac

case "$dest" in
	tests/fixtures/bench|tests/fixtures/bench/*|"$PWD"/tests/fixtures/bench|"$PWD"/tests/fixtures/bench/*)
		if [ "$allow_repo_fixture" != "true" ]; then
			echo "ERR refusing repo fixture destination without --allow-repo-fixture"
			exit 1
		fi
		;;
esac

if [ ! -f "$input" ]; then
	echo "ERR missing input: $input"
	exit 1
fi

if [ -x ./build/kilonode-compat ]; then
	./build/kilonode-compat check-capture "$input" >/dev/null
else
	echo "WARN build missing: run ./scripts/build.sh before validation"
fi

mkdir -p "$dest"
cp -p "$input" "$dest/$name"

echo "OK imported=$dest/$name"
echo "next: ./build/kilonode-compat capture-report $dest/$name"
echo "next: review before adding manual captures to committed fixtures"
