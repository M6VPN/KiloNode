# AX.25 Segmentation Model

M2.3 adds a diagnostics-only segmenter for AX.25 connected-mode loopback
payload tests.

The segmenter accepts explicit payload bytes, a paclen value, and a bounded
output list. It produces ordered segments with:

- segment index
- payload offset
- payload length
- final segment flag

The segmenter is binary-safe and does not assume NUL termination.

## Paclen Relationship

Loopback segmentation uses `params.paclen` as the maximum information bytes per
I frame. `paclen` must be greater than zero and no larger than
`params.max_info_len`.

The generic segmenter does not build AX.25 frames. It only splits payload bytes.
The loopback segment helper turns each segment into one raw AX.25 I frame by
calling the M2.2 I-frame builder.

## Window Policy

M2.3 implements window-size 1 for segmented loopback payloads.

The sender emits one I frame, the peer records delivery diagnostics and prepares
an RR, and the RR is moved back to the sender before the next segment is sent.
Larger send windows remain planned because they need explicit outstanding-frame
tracking and retransmission buffer policy.

## Errors

The helpers reject:

- paclen zero
- paclen larger than max-info
- too many segments for the bounded output list
- payloads that do not fit the fixed simulator bounds
- segmented send attempts when the endpoint is not connected
- segmented send attempts with unsupported window sizes

The loopback path remains simulator-only and does not touch the real TX queue.
