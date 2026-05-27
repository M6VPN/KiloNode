# AX.25 Action To Frame Mapping

M1.34 maps isolated AX.25 connected-mode action intents into bounded frame
plans. The mapper is not connected to daemon RX/TX paths, does not enqueue
frames, does not dispatch frames, and does not start timers.

## Scope

The mapper accepts a local endpoint, a remote endpoint, sequence state, and a
list of action intents from the connected-mode state core.

Actions that produce frame plans:

| Action     | Frame plan |
|------------|------------|
| send-sabm  | SABM       |
| send-sabme | SABME      |
| send-ua    | UA         |
| send-dm    | DM         |
| send-disc  | DISC       |
| send-rr    | RR         |
| send-rnr   | RNR        |
| send-rej   | REJ        |
| send-frmr  | FRMR placeholder |

Actions that do not produce frame plans:

| Action class | Handling |
|--------------|----------|
| I payload delivery | No outbound frame in this layer. |
| Timer intents | No scheduler or timer is started. |
| Retry counters | No frame is generated. |
| State-entry markers | No frame is generated. |
| Protocol error | No frame is generated unless a later layer emits FRMR explicitly. |

## Direction Policy

Frame plans use `source=local` and `destination=remote`. For an inbound
remote-to-local SABM, the state core returns `send-ua`; the mapper turns that
into a local-to-remote UA plan.

Digipeater reverse-path handling is deferred. The current mapper can carry a
caller-provided path, but it does not infer or reverse an inbound path.

## Safety

Frame plans are bounded arrays. Overflow returns a deterministic error. Plans
are metadata only and contain no TX queue state, dispatch state, transport
handles, or timer handles.

No live `CONNECT` path is added in this pass.
