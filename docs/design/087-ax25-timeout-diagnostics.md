# AX.25 Timeout Diagnostics

M1.43 adds offline diagnostics for timer-driven AX.25 state changes.

Covered cases:

| Case | Diagnostic coverage |
|------|---------------------|
| T1 connect retry | Local connect starts T1, expiry retries SABM, and T1 restarts. |
| T1 connect exhaustion | Repeated T1 expiry reaches N2 policy and enters disconnected/error state. |
| UA while awaiting connection | UA stops T1, starts T3, and resets retry count. |
| T3 connected poll | T3 expiry produces the current RR poll placeholder action and diagnostic plan. |
| T1 disconnect retry | Local disconnect starts T1, expiry retries DISC, and T1 restarts. |
| T2 placeholder | Accepted I-frame replay starts a diagnostic T2 placeholder and reports planned expiry handling. |

TX write attempts must remain zero for all committed replay fixtures.

T2 delayed acknowledgement behaviour is still planned. The replay harness marks
T2 expiry as `planned` and does not create an automatic ACK path.

Future live daemon integration remains separate:

- live scheduler polling
- retransmission queues
- action-plan to TX queue bridge
- RF CONNECT command
- BBS session binding
