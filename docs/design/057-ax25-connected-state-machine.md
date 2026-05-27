# AX.25 Connected State Machine

M1.33 adds an isolated AX.25 connected-mode state core. It is unit-test only
in this pass and is not wired into daemon RX, TX queueing, shell commands, BBS
sessions, or RF dispatch.

Implemented states:

| State                 | Purpose |
|-----------------------|---------|
| disabled              | Connected mode is unavailable. Events are rejected. |
| disconnected          | No active link exists. Local connect or inbound setup can start the link. |
| awaiting-connection   | A local setup request is waiting for UA or DM. |
| connected             | Basic sequence accounting is active. |
| awaiting-release      | A local release is waiting for UA or DM. |
| timer-recovery        | Placeholder only. Detailed recovery is deferred. |

Implemented event handling:

| Event area       | M1.33 behaviour |
|------------------|-----------------|
| SABM/SABME       | In disconnected state, enters connected and returns send-UA intent. |
| UA               | Completes awaiting-connection or awaiting-release. |
| DISC             | In connected state, enters disconnected and returns send-UA intent. |
| DM               | Tears down awaiting or connected state without response frame bytes. |
| RR/RNR/REJ       | Updates acknowledgement, remote-busy, and retransmit-needed flags. |
| I frame          | Accepts matching modulo 8 N(S), advances V(R), and returns delivery plus RR intents. |
| T1 timeout       | Retries setup/release until the configured retry count is exhausted. |
| T3 timeout       | Returns a poll-style RR intent placeholder while connected. |

Transition policy:

- Disconnected inbound DISC and connected-mode frames return safe action intents
  rather than live output.
- Invalid I-frame sequence returns a REJ intent and leaves V(R) unchanged.
- Retry exhaustion returns protocol-error and enter-disconnected intents.
- FRMR is represented as a protocol-error placeholder.

Incomplete areas:

- No modulo 128 control handling.
- No transition table for full timer recovery.
- No retransmission queue.
- No frame byte generation.
- No connected shell, BBS, NET/ROM, or compatibility command binding.

FX.25 boundary:

The state machine accepts decoded AX.25 control/event data only. Future FX.25
decode must produce AX.25 frame bytes before this layer is called.
