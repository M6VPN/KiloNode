#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/compat-capture-placeholder.sh

set -eu

cat <<'EOF'
KiloNode compatibility capture placeholder

This pass does not run LinBPQ or any other compatibility target.

Future black-box capture work must be started manually by the operator.
Allowed future inputs are externally visible behaviour only:
	- process stdout/stderr
	- packet captures
	- command response transcripts
	- mailbox files produced by running the target

Forbidden inputs:
	- GPL source code
	- copied command tables
	- copied prompts
	- copied parser logic
	- copied implementation logic

The known local paths for a future manual black-box pass are:
	~/Sync/Code/M6VPN/M6VPN-7/3rd/linbpq/linbpq
	~/Sync/Code/M6VPN/M6VPN-7/3rd/linbpq/bpq32.cfg.example
EOF
