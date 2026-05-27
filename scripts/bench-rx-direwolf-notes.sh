#!/usr/bin/env bash
# KiloNode - Developed by M6VPN (M6VPN@tuta.com)
# kilonode/scripts/bench-rx-direwolf-notes.sh

set -eu

cat <<'EOF'
KiloNode receive-only Dire Wolf bench notes

Topology:
  radio RX audio or test audio -> USB sound card -> Dire Wolf -> TCP KISS -> KiloNode

Manual checklist:
  1. Configure Dire Wolf for KISS TCP on 127.0.0.1:8001.
  2. Keep PTT and transmit paths disabled.
  3. Run: ./build/kilonoded --config packaging/examples/kilonode-rx-bench-tcp-kiss.conf --check-config
  4. Run: ./build/kilonoded --config packaging/examples/kilonode-rx-bench-tcp-kiss.conf --foreground
  5. Query: ./build/kilonodectl --socket /tmp/kilonode/control.sock tx gates
  6. Query: ./build/kilonodectl --socket /tmp/kilonode/control.sock rx status
  7. Query: ./build/kilonodectl --socket /tmp/kilonode/control.sock heard
  8. Query: ./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 live
  9. Query: ./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 connections

This script does not start Dire Wolf, open sound devices, or require RF hardware.
EOF
