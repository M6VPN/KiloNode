# AX.25 Prepared Response Validation

Prepared response validation checks generated AX.25 response diagnostics
without using the real TX queue.

Use:

```sh
./scripts/ax25-prepared-replay-fixtures.sh
./scripts/ax25-prepared-gate-report.sh
```

Expected properties:

- prepared replay assertions pass
- timer replay assertions pass
- prepared-to-TX bridge status remains disabled
- TX write counters remain zero
- FX.25 wrapping remains blocked

Prepared frames are AX.25 body bytes for inspection. They are not dispatched
and are not promoted into the TX queue.
