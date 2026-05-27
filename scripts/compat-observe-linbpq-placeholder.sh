#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/compat-observe-linbpq-placeholder.sh

set -eu

cat <<'EOF'
KiloNode LinBPQ black-box observation placeholder

This script does not run LinBPQ.
Run black-box targets manually and explicitly with kilonode-compat when needed.

Example paths for a future manual lab:
	~/Sync/Code/M6VPN/M6VPN-7/3rd/linbpq/linbpq
	~/Sync/Code/M6VPN/M6VPN-7/3rd/linbpq/bpq32.cfg.example

Example command shape:
	./build/kilonode-compat observe-process \
		--binary ~/Sync/Code/M6VPN/M6VPN-7/3rd/linbpq/linbpq \
		--config ~/Sync/Code/M6VPN/M6VPN-7/3rd/linbpq/bpq32.cfg.example \
		--name linbpq-startup-001 \
		--mode process-output \
		--input "" \
		--output /tmp/linbpq-startup.observation \
		--timeout 10

Do not inspect GPL source.
Do not copy prompts, command tables, parser logic, or implementation logic.
Do not connect any RF transmit path for observation capture.
EOF
