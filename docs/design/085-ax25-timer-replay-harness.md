# AX.25 Timer Replay Harness

M1.43 adds an offline timer-driven replay harness for AX.25 connected-mode
diagnostics.

Replay flow:

1. Parse a KiloNode-native replay script.
2. Initialize a diagnostic AX.25 runtime with connected mode enabled only inside
   the offline harness.
3. Create a bounded connection key from the script port, node callsign, and
   remote callsign.
4. Apply synthetic AX.25 connection events.
5. Apply state-machine action side effects to the logical scheduler.
6. Advance injected monotonic time.
7. Process expired T1, T2, or T3 timers.
8. Map generated action intents to diagnostic frame plans.
9. Check expectations and emit a deterministic report.

The harness does not open transports, does not start a daemon, does not use
operating-system timers, does not sleep, and does not enqueue or dispatch TX
frames.

The scheduler remains a unit-test/offline component. No live daemon event-loop
scheduler is added in this pass.
