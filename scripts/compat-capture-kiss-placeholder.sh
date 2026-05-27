#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/compat-capture-kiss-placeholder.sh

set -eu

cat <<'EOF'
KiloNode KISS capture placeholder

This pass does not open serial, TCP, PTY, Unix, or stdio KISS devices.

Future manual workflow:
1. Run the black-box node process explicitly in an isolated lab.
2. Capture externally visible KISS frame boundaries only.
3. Convert each captured frame to the KiloNode packet capture format.
4. Validate with:
   ./build/kilonode-compat replay-capture path/to/file.capture

Do not inspect LinBPQ/BPQ source files or copy source-derived tables, prompts,
message formats, parser logic, forwarding logic, queue logic, or dispatch logic.
EOF
