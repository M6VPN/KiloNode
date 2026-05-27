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
| FX.25 receive validation | planned  | Future bench work must validate FX.25 detection, FEC stats, payload extraction, and fallback. |
| FX.25 pre-decode integration | planned | No FX.25 unwrap path feeds AX.25 diagnostics yet. |
| Live FX.25 integration   | planned  | No RX unwrap or TX wrap path is wired into runtime. |
| FX.25 FEC with AX.25 runtime | planned | No FEC code exists and AX.25 diagnostics do not inspect FX.25 frames. |
