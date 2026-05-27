# Bench Safety Policy

Receive-only bench validation must keep every transmit path closed. M1.38 adds
docs, configs, and scripts for RX diagnostics only.

## Required Gates

Bench configs must keep:

- `transmit enabled false`
- `dispatch-enabled false`
- every bench port `tx-enabled false`
- `rf-command enabled false`
- `rf-command reply-enabled false`
- `ax25 connected-mode false`

The AX.25 live RX feed may be enabled for diagnostics, but it must not enqueue,
dispatch, or write response frames.

## Operator Checks

Before feeding live receive data:

1. Run `kilonoded --check-config` on the bench config.
2. Start only receive-side audio, TNC, or KISS paths.
3. Confirm `kilonodectl tx gates` reports transmit blocked.
4. Confirm `kilonodectl ax25 live` is read-only diagnostic output.
5. Keep PTT, transmitter control, and RF output disconnected for receive
   diagnostics.

Operators are responsible for complying with local radio regulations and bench
equipment safety.

## Future TX Bench Work

Controlled TX bench validation is a separate milestone. It will need its own
configs, safety gates, dummy-load or non-radiating setup guidance, and explicit
operator actions. The receive-only bench docs must not be reused as approval for
TX dispatch.

## FX.25 Boundary

FX.25 remains future work. Receive-only AX.25 bench validation starts after KISS
and AX.25 decode. Future FX.25 receive validation should unwrap FX.25 before the
AX.25 diagnostic feed and should keep transmit disabled unless a later TX
milestone explicitly changes that boundary.
