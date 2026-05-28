# AX.25 Live Scheduler Smoke

This bench note covers the optional daemon smoke mode for AX.25 scheduler
diagnostics. No bench hardware is required.

Example local AX.25 block:

```text
ax25 {
	enabled true
	connected-mode false
	diagnostics true
	live-rx-feed false
	live-rx-create-connections false
	live-rx-retain-frame-plans true
	live-scheduler true
	live-scheduler-process-expired true
	live-scheduler-smoke true
	live-scheduler-smoke-create-test-connection true
	live-scheduler-max-expired-per-cycle 4
	live-scheduler-tx-actions false
	prepared-bridge-to-tx false
}
```

Run a daemon with a local diagnostic config, then query:

```text
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 scheduler-smoke
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 scheduler
./build/kilonodectl --socket /tmp/kilonode/control.sock ax25 prepared-bridge
./build/kilonodectl --socket /tmp/kilonode/control.sock tx gates
```

Expected safety signals:

- `tx_writes=0`
- `dispatch_calls=0`
- prepared bridge disabled or blocked
- TX gates disabled unless a future lab-only milestone changes them

Do not treat smoke mode as permission to transmit. It only proves that scheduler
diagnostics can be polled without touching the real TX queue.
