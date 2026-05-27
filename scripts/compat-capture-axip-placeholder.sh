#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/compat-capture-axip-placeholder.sh

set -eu

cat <<'EOF'
KiloNode AXIP/AXUDP capture placeholder

This pass does not start live packet capture, open sockets, or implement AXIP
routing.

Future manual workflow:
1. Capture externally visible AXIP/AXUDP packet boundaries in an isolated lab.
2. Store raw AX.25 payload bytes, not implementation details.
3. Convert each observation to the KiloNode packet capture format.
4. Validate with:
   ./build/kilonode-compat replay-capture path/to/file.capture

Do not inspect LinBPQ/BPQ source files or copy source-derived tables, prompts,
message formats, parser logic, forwarding logic, queue logic, or dispatch logic.
EOF
