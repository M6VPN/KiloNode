# AX.25 Live RX Diagnostics Feed

M1.37 wires decoded inbound AX.25 connected-mode frames into the AX.25
diagnostic runtime. The feed is disabled unless the AX.25 config block enables
the runtime and the live RX feed.

This is diagnostics only. It does not expose CONNECT, start sessions, queue
responses, dispatch frames, or deliver I-frame payloads to shell or BBS code.

## Config Gates

The optional AX.25 block keeps live processing off by default:

```text
ax25 {
	enabled false
	connected-mode false
	diagnostics true
	live-rx-feed false
	live-rx-create-connections false
	live-rx-retain-frame-plans true
	max-connections 32
	modulo 8
	window-size 1
	t1-ms 3000
	t2-ms 1000
	t3-ms 300000
	n2-retries 10
}
```

`live-rx-feed true` requires `enabled true`. Creating diagnostic records from
inbound setup frames also requires `live-rx-create-connections true`.

## Feed Policy

The feed accepts decoded AX.25 frames after KISS and AX.25 decode. UI frames
are ignored. Malformed AX.25 payloads are counted but do not create events.

A frame is relevant when it is addressed to the configured local node callsign
or when it matches an existing diagnostic connection record. SABM and SABME
frames can create a diagnostic connection only when creation is explicitly
enabled.

I-frame payloads are counted by length only. Payload bytes are not stored or
delivered.

## Frame Plans

The state machine may produce action intents and frame plans. M1.37 retains
those plans for diagnostics only. No frame plan is converted into a TX frame,
queued, dispatched, or written to a transport.

## Control Commands

The read-only control plane exposes:

```text
AX25 LIVE
AX25 STATUS
AX25 COUNTERS
AX25 CONNECTIONS
AX25 CONNECTION <id>
```

`AX25 LIVE` reports feed state and live counters. `AX25 STATUS` and
`AX25 COUNTERS` include live feed fields. There are no mutation commands.
