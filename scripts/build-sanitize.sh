#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/build-sanitize.sh

set -eu

cmake -S . -B build-sanitize \
	-DCMAKE_BUILD_TYPE=Debug \
	-DKILONODE_SANITIZE_ADDRESS=ON \
	-DKILONODE_SANITIZE_UNDEFINED=ON \
	"$@"
cmake --build build-sanitize
ctest --test-dir build-sanitize --output-on-failure
