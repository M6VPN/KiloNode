# AX.25 Sequence Accounting

M1.33 implements modulo 8 sequence helpers for the isolated connected-mode
state core.

Tracked variables:

| Field                | Meaning |
|----------------------|---------|
| send_state           | Future V(S) tracking. |
| receive_state        | Expected receive sequence for inbound I frames. |
| acknowledge_state    | Last accepted N(R) from RR, RNR, REJ, or I frames. |
| remote_busy          | Set when RNR is received and cleared by RR/REJ. |
| retransmit_needed    | Set when REJ is received or an invalid I sequence is observed. |

Implemented helpers:

- Normalize sequence numbers to modulo 8.
- Increment modulo 8 and wrap 7 to 0.
- Compare an inbound I-frame N(S) to expected V(R).
- Advance V(R) after an accepted I frame.
- Update V(A) from RR.
- Mark remote busy from RNR.
- Mark retransmit-needed from REJ.
- Validate modulo 8 window limits.

Modulo 128:

Modulo 128 remains planned. The parameter value is still accepted by the
configuration scaffold, but sequence helpers return a not-implemented status
for modulo 128 behaviour in this pass.

Limitations:

- No send window tracking beyond acknowledgement state.
- No retransmission buffer.
- No segmentation.
- No timer scheduling.
