# AX.25 Loopback Simulator

M2.1 adds an offline AX.25 connected-mode loopback simulator. It runs two
KiloNode AX.25 diagnostic endpoints in one process and passes raw AX.25 frame
bytes between them through an in-memory link.

The simulator is test-only. It does not create daemon sessions, open KISS
transports, write to the real TX queue, dispatch frames, expose CONNECT, or bind
AX.25 payloads to shell or BBS code.

## Flow

The loopback path is:

```text
endpoint A event -> state machine -> action mapper -> prepared frame queue
prepared AX.25 bytes -> in-memory link -> endpoint B frame input
endpoint B state machine -> action mapper -> prepared frame queue
```

The current fixtures cover:

- local connect from endpoint A
- SABM or SABME prepared frame generation
- peer receive of SABM or SABME
- peer UA generation
- initiator receive of UA
- connected state on both endpoints
- small simulator-built I frame payload delivery and RR acknowledgement
- DISC and UA disconnect
- T1 timeout retry followed by late UA

## Time

Time is injected as monotonic millisecond values by the loopback script runner.
The simulator uses the existing logical AX.25 scheduler and timers where needed.
It does not use OS timers, signals, threads, alarms, or sleeps.

## Limits

All loopback runs are bounded by script limits and `run-until-idle` maximum step
counts. The link moves a bounded number of frames per step and rejects oversized
or malformed raw AX.25 input.

I-frame generation uses the M2.2 raw AX.25 I-frame helpers and remains
simulator-only. This keeps live TX and daemon behaviour unchanged while allowing
loopback tests to exercise small text and binary payloads plus RR acknowledgement
diagnostics.

FX.25 is not part of the simulator. The report keeps `fx25_frames=0`.
