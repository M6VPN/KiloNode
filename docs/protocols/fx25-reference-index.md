# FX.25 Reference Index

Primary reference:

| File            | Standing          | Scope used here |
|-----------------|-------------------|-----------------|
| FX-25_01_06.pdf | Informative draft | FX.25 layering around AX.25 packets, correlation/tag concepts, FEC code block planning, padding, and interoperability fallback. |

Relevant FX.25 areas for KiloNode:

| Area                     | Planning note |
|--------------------------|---------------|
| Layer position           | FX.25 is modeled as a layer below or alongside AX.25 frame bytes and above physical/KISS transport boundaries. |
| AX.25 payload relation   | The embedded payload remains an AX.25 packet. KiloNode does not extract one yet. |
| Correlation/tag handling | Detect-only scaffolding is planned, but tag detection is not implemented in M1.32. |
| FEC block handling       | Reed-Solomon encode/decode is deferred. |
| Interoperability         | Normal AX.25 fallback is kept as the safe default. |
| Failure modes            | Candidate, malformed, uncorrectable, corrected, and not-implemented statuses are modeled. |

KiloNode status:

| Area                  | Status   | Notes |
|-----------------------|----------|-------|
| Reference indexed     | Scaffold | Reference is documented and linked to the future test strategy. |
| Layer model           | Scaffold | Public structs/enums compile and test. |
| Detect-only mode      | Scaffold | Config struct validates detect-only, but detection returns not implemented. |
| FEC encode/decode     | Planned  | No Reed-Solomon implementation is present. |
| AX.25 extraction      | Planned  | No payload extraction is claimed. |
| KISS integration      | Planned  | RX/TX paths are unchanged in M1.32. |

Deferred:

| Area                    | Reason |
|-------------------------|--------|
| Correlation tag matcher | Needs dedicated test vectors and capture fixtures. |
| Reed-Solomon FEC        | Out of scope for M1.32. |
| Live FX.25 RX/TX        | Requires packet-boundary validation and explicit integration design. |
| Interop captures        | Future black-box capture work only. |
