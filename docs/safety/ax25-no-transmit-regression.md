# AX.25 No-Transmit Regression

The no-transmit regression checks ensure default examples and automated
workflows do not enable real AX.25 response TX.

Required checks:

- example configs do not enable transmit in receive-only bench files
- prepared bridge remains disabled
- prepared replay outputs show TX writes at zero
- timer replay outputs show TX writes at zero
- scripts do not open serial or TNC transmit paths
- tests do not require hardware
- dispatch commands are not run by default
- FX.25 wrapping remains disabled

Scripts:

- `scripts/ax25-no-transmit-check.sh`
- `scripts/ax25-safety-check.sh`
- `scripts/ax25-prepared-gate-report.sh`
- `scripts/ax25-response-bench-preflight.sh`
