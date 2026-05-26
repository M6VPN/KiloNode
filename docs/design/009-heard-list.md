# Heard List

The heard list records AX.25 stations observed by `kilonoded`. It is diagnostic
observation state, not routing state.

Each entry is keyed by source callsign plus daemon port name. The daemon updates
an entry only after a frame has decoded as structurally valid AX.25. Malformed
AX.25 frames still affect malformed-frame counters, but they do not create heard
entries.

Stored fields:

- source callsign and SSID
- port name
- first and last heard timestamps
- frame count
- last destination callsign and SSID
- last digipeater path
- last control byte
- last PID byte when present
- last payload length
- last UI-frame flag

Payload bytes are never stored in the heard list.

The default table limit is 256 entries. When the table is full and a new station
must be inserted, the oldest entry by `last_heard` is replaced. Ties use the
lowest table index, which keeps eviction deterministic.

Timestamps are supplied by the daemon when frames are observed. Unit tests inject
fixed timestamps through the heard-list update API.

Control socket commands:

```text
HEARD
HEARD PORT kiss0
HEARD CLEAR
```

`HEARD` returns all entries. `HEARD PORT <name>` filters by configured port name.
Unknown ports return an empty successful result. `HEARD CLEAR` is reserved and
returns `ERR not-implemented` in this pass.

Deferred work:

- persistent heard lists
- NET/ROM routing
- APRS-specific parsing
- BBS user tracking
- packet transmit
- remote sysop commands
