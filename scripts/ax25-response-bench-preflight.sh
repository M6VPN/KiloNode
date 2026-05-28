#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/ax25-response-bench-preflight.sh

set -eu

cat <<'EOF'
OK ax25-response-bench-preflight stage0=current stage1=receive-only stage2=blocked stage3=blocked
Stage 0: offline replay only. Run ax25-safety-check.sh.
Stage 1: receive-only source. Keep TX blocked and do not connect PTT.
Stage 2: future dummy-load or non-radiating TX lab. Blocked.
Stage 3: future controlled legal over-air test. Blocked.
This script does not open devices, start daemons, or transmit.
EOF
