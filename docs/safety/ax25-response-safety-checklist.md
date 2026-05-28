# AX.25 Response Safety Checklist

This checklist is a future gate for real connected-mode AX.25 responses. Items
marked complete must have project tests, scripts, or docs as proof. Unchecked
items remain blockers.

## Protocol Correctness

- [x] AX.25 state machine has offline unit coverage.
- [x] Action-to-frame mapper is tested.
- [x] Prepared frame builder is tested.
- [x] Timer and retry behaviour is tested through offline replay.
- [x] Modulo 8 behaviour is documented.
- [ ] Modulo 128 behaviour is complete.
- [ ] I-frame response handling is complete.
- [ ] Full connected-mode interop is proven against reviewed captures.

## Safety Gates

- [x] Prepared-to-TX bridge is disabled by default.
- [ ] Explicit future config enables the real bridge.
- [ ] Per-port TX enablement is required and tested for real bridge use.
- [ ] TX dispatch gates are required and tested for real bridge use.
- [ ] Auto-dispatch has a separate design and operator gate.
- [ ] Real queue promotion has a reviewed rate and retry policy.

## Bench Validation

- [x] Receive-only diagnostics are validated.
- [x] Prepared-frame replay assertions pass.
- [x] Timer replay assertions pass.
- [x] Manual capture replay path is validated.
- [x] No-transmit regression script passes.
- [ ] Dummy-load or non-radiating TX lab validation exists.
- [ ] Controlled legal over-air validation exists.

## Operator Controls

- [ ] Callsign is configured and verified for the operating station.
- [ ] Legal and regulatory responsibility is documented for the operator.
- [ ] Band, frequency, and port policy are documented.
- [ ] Transmitter, audio, TNC, and PTT path are understood.
- [ ] Dummy load or isolated test setup is used for lab TX.
- [ ] Emergency stop path is documented.
- [ ] Logs are collected for each TX validation run.

## Failure Handling

- [x] Prepared queue full behaviour is counted.
- [x] Retry exhaustion behaviour is tested offline.
- [x] Timer behaviour is tested offline.
- [x] Malformed frame handling is tested in replay paths.
- [x] Unknown control handling is bounded.
- [ ] Channel and transport failure handling is tested for real TX.
- [ ] Busy-channel behaviour is defined.

## Deferred Blockers

- [ ] Real channel busy and PTT policy.
- [ ] Rate limits and duty-cycle policy.
- [ ] Sysop or operator authentication for any live TX controls.
- [ ] FX.25 wrapping checks, if FX.25 TX is enabled later.
- [ ] NET/ROM is not included.
- [ ] RF BBS is not included.
