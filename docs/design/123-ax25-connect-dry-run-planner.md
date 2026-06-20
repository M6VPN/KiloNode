# AX.25 CONNECT Dry-Run Planner

The M2.6 planner is an offline `kilonode-compat` diagnostic surface. It parses
a KiloNode-native `.connectdry` file and evaluates a hypothetical local AX.25
CONNECT request.

The planner reports:

- local callsign
- remote callsign
- port name
- AX.25 params
- planned initial state
- planned setup frame kind
- prepared-to-TX bridge status
- TX write, dispatch, and FX.25 counters

Modulo 8 plans SABM. Modulo 128 plans SABME. Both are reported as intent only.
No connection is inserted into daemon runtime or any connection table.

## Commands

`kilonode-compat` exposes:

- `check-ax25-connect-dry-run PATH`
- `run-ax25-connect-dry-run PATH`
- `ax25-connect-dry-run-report PATH`

All commands are offline and deterministic.
