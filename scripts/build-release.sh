#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/build-release.sh

set -eu

cmake -S . -B build-release \
	-DCMAKE_BUILD_TYPE=Release \
	-DKILONODE_HARDENING=ON \
	"$@"
cmake --build build-release
