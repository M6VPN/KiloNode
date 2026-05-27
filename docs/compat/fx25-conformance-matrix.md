# FX.25 Conformance Matrix

| Area                    | Status   | Notes |
|-------------------------|----------|-------|
| Reference indexed       | scaffold | `FX-25_01_06.pdf` is indexed as an informative draft. |
| Layer model             | scaffold | Modes, statuses, payload relation, and FEC profile placeholders exist. |
| Detect-only scaffold    | scaffold | Parameters can validate detect-only mode. Detection is not implemented. |
| Tag/correlation detect  | planned  | Requires test vectors and packet-boundary captures. |
| FEC decode              | planned  | No Reed-Solomon correction is present. |
| FEC encode              | planned  | No encoder is present. |
| AX.25 payload extraction | planned | No embedded AX.25 payload is claimed. |
| Fallback to AX.25       | scaffold | Parameter defaults allow future fallback. RX path is unchanged. |
| KISS integration        | planned  | No KISS RX/TX integration in M1.32. |
| Output boundary documented | scaffold | AX.25 action mapping emits raw AX.25 bytes before any future FX.25 wrapping. |
| FX.25 wrapping          | planned  | No FX.25 TX wrapper is present. |
| Test vectors            | planned  | Synthetic and reference-derived vectors are needed. |
| Interop captures        | planned  | Future black-box captures only. |
| Connected-mode boundary | scaffold | FX.25 remains separate. Future decode must produce AX.25 bytes before AX.25 connection state handling. |
| Connection table boundary | scaffold | AX.25 connection records operate only on decoded AX.25 frames and AX.25 frame plans. |
| AX.25 diagnostics boundary | scaffold | Read-only diagnostics report AX.25 runtime state after the future FX.25 unwrap point. |
| Live AX.25 feed boundary | scaffold | M1.37 live diagnostics consume decoded AX.25 frames only. Future FX.25 decode must happen before this point. |
| FX.25 bench notes        | implemented | Receive-only bench docs include future FX.25 validation notes without claiming support. |
| FX.25 placeholder fixture | implemented | Bench pack includes a planned placeholder that is skipped during replay. |
| FX.25 placeholder replay | scaffold | Diagnostic replay reports FX.25 placeholders as planned and unsupported. |
| FX.25 decode from fixture | planned  | Placeholder does not claim valid FX.25 decode. |
| FX.25 diagnostic replay   | planned  | Future FX.25 replay must decode or reject FX.25 frames before AX.25 diagnostics. |
| Manual FX.25 capture import | scaffold | Manual workspace can catalog FX.25 placeholders as unsupported/planned. |
| Manual FX.25 replay        | planned  | Replay remains unsupported until FX.25 decode and FEC exist. |
| FX.25 receive validation | planned  | Future bench work must validate FX.25 detection, FEC stats, payload extraction, and fallback. |
| Manual FX.25 captures    | planned  | Future captures need known-good, corrected, uncorrectable, and fallback cases. |
| FX.25 pre-decode integration | planned | No FX.25 unwrap path feeds AX.25 diagnostics yet. |
| Live FX.25 integration   | planned  | No RX unwrap or TX wrap path is wired into runtime. |
| FX.25 FEC with AX.25 runtime | planned | No FEC code exists and AX.25 diagnostics do not inspect FX.25 frames. |
| FX.25 timer coupling        | unsupported | AX.25 T1/T2/T3 scheduling is kept inside AX.25 connected-mode scaffolding. |
| FX.25 FEC timer effects     | planned | Future FEC statistics must remain outside AX.25 timer logic. |
| AX.25 timer replay boundary | scaffold | Timer replay drives AX.25 connection diagnostics only and does not parse FX.25 blocks. |
| Live AX.25 scheduler boundary | scaffold | Live scheduler diagnostics operate only on AX.25 runtime state. FX.25 remains outside the scheduler. |
| FX.25 scheduler coupling      | unsupported | No FX.25 decode, FEC, wrapper, or timer policy is wired into AX.25 scheduler hooks. |
| Prepared AX.25 output boundary | scaffold | Prepared diagnostics store AX.25 bytes only. Future FX.25 wrapping remains after this boundary. |
| FX.25 wrapping from prepared frames | planned | No FX.25 wrapper or FEC encoder is present. |
