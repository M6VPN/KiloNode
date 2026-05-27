# AX.25 Response TX Safety Boundary

Prepared AX.25 frames are diagnostics. They are not the real TX queue.

Before any later milestone can enable real response TX, KiloNode needs all of
these gates:

- explicit operator config
- per-port TX enabled
- TX policy enabled
- dispatch policy reviewed and safe
- channel/PTT policy
- rate limiting
- legal and callsign policy
- receive-only and controlled-TX bench validation

M1.47 does not satisfy those gates. It only records whether a prepared frame
would pass a diagnostic decision model. The normal runtime bridge remains
blocked and the TX queue write counter remains zero.
