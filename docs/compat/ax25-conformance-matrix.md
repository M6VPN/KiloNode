# AX.25 Conformance Matrix

| Area                           | Status      | Notes |
|--------------------------------|-------------|-------|
| Address encode/decode          | implemented | Destination, source, SSID, digipeater list, final bit, and repeated bit are covered by tests. |
| UI frame encode/decode         | implemented | UI frame body handling exists for RX and TX helpers. |
| PID handling                   | partial     | PID is decoded when expected and emitted for UI frames. Full PID matrix is planned. |
| Digipeater path decode         | partial     | Digipeater list and repeated bit are decoded. Command/response interpretation is deferred. |
| I frame control classification | scaffold    | Class and sequence field extraction are compile-tested. |
| S frame control classification | scaffold    | RR, RNR, REJ, and SREJ names are compile-tested. |
| U frame control classification | scaffold    | SABM, SABME, DISC, DM, UA, FRMR, UI, XID, and TEST names are compile-tested. |
| Connected-mode state machine   | planned     | No active transitions exist. |
| Timers                         | scaffold    | T1, T2, and T3 parameters validate only. |
| Retries                        | scaffold    | N2 parameter validates only. |
| Modulo 8                       | scaffold    | Parameter and basic control extraction exist. |
| Modulo 128                     | planned     | Parameter placeholder exists. Extended control decode is deferred. |
| FRMR handling                  | scaffold    | Control subtype and event placeholder exist. |
| TEST/XID handling              | scaffold    | Control subtype names exist. No procedures are implemented. |
| FCS handling                   | deferred    | KISS-facing code operates at AX.25 body boundary. |
| KISS payload boundary          | implemented | Existing KISS encode/decode treats AX.25 bytes as payload. |
