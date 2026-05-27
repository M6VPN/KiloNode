# AXIP and AXUDP Capture Plan

M1.28 treats AXIP and AXUDP captures as packet-boundary AX.25 payload
observations. Capture files contain raw AX.25 frame bytes, not IP headers.

## Future Manual Workflow

1. Capture externally visible AXIP or AXUDP packet boundaries in an isolated lab.
2. Extract raw AX.25 payload bytes.
3. Store those bytes in the KiloNode packet capture format.
4. Validate with `kilonode-compat replay-capture`.

The placeholder script does not start live capture, open sockets, or implement
routing.

## Scope

This pass does not implement AXIP sockets, AXUDP listeners, routing, pcap input,
NET/ROM, or BPQ/LinBPQ compatibility.

## Clean-Room Rule

Captured bytes are observation data only. Do not inspect source code or copy
implementation logic from any compatibility target.
