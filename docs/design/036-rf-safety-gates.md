# RF Safety Gates

KiloNode transmit remains non-transmitting by default. M1.23 adds explicit
safety gates before any real KISS transport write can happen.

## Global Gates

Real KISS dispatch requires all global transmit gates:

```text
transmit {
	enabled true
	dry-run false
	allow-ui true
	allow-control-enqueue true
	allow-shell-enqueue false
	dispatch-enabled true
	dispatch-test-only false
	dispatch-real-kiss true
	require-explicit-port-tx true
}
```

If any gate is missing, dispatch is blocked with a deterministic reason.

`allow-shell-enqueue` must remain false. TX commands are not exposed through the
node shell or BBS shell in this pass.

## Per-Port Gate

Each port has a separate TX flag:

```text
port kiss0 {
	type tcp-connect
	host 127.0.0.1
	port 8001
	enabled true
	tx-enabled false
}
```

`tx-enabled` defaults false. Real dispatch requires the target port to be
enabled, open, writable, and `tx-enabled true`.

## Blocked By Default

Default and normal example configs keep:

- `transmit enabled false`
- `dry-run true`
- `dispatch-enabled false`
- `dispatch-test-only true`
- `dispatch-real-kiss false`
- every real port `tx-enabled false`

This means normal examples do not transmit.

## Lab-Only Example

`packaging/examples/kilonode-tx-lab-only.conf` is a parsing-safe lab example.
It still blocks RF output by default because `dry-run true` and `tx-enabled
false` are set.

To use it for a non-radiating lab test, an operator must intentionally edit the
config. The operator is responsible for licensing, hardware setup, dummy load or
shielded test conditions, and local RF rules.

## Control Diagnostics

Use:

```text
TX GATES
TX GATES PORT <name>
```

Example blocked port response:

```text
OK TX GATES PORT name=kiss0 enabled=true tx_enabled=false transport=tcp-client writable=true allowed=false reason=port-tx-disabled
END
```

## Deferred Safety Work

- PTT control
- channel busy detection
- duty-cycle limits
- per-band policy
- callsign and legal ID policy
- hardware interlocks
- RF watchdog
- sysop authentication
