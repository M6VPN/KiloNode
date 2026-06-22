# External Modem Adapter Model

KiloNode treats external modems as process or TCP boundaries owned outside the
daemon. The adapter scaffold records configuration, profile, and status data
only. It does not launch modem processes, open modem sockets, enqueue transmit
frames, or expose CONNECT.

The scaffold supports multiple named `external-modem` config blocks. Each block
has a type, mode, host, port, and safety flags. The safety flags default to
false and this milestone rejects `tx-enabled true`, `connect-enabled true`, and
`auto-start true`.

## Supported Initial Types

| Type         | Purpose                                      | Current status |
| ------------ | -------------------------------------------- | -------------- |
| kiss-tcp     | TCP KISS receive path, commonly Dire Wolf    | receive-only   |
| mercury-ofdm | Rhizomatica Mercury OFDM planned adapter     | planned        |
| vara-hf      | VARA HF planned external adapter             | planned        |
| vara-fm      | VARA FM planned external adapter             | planned        |
| ardop        | ARDOP planned external adapter               | planned        |
| generic-tcp  | Placeholder for documented TCP modem targets | planned        |

## Boundary

The daemon may expose modem profile and configured status through read-only
control commands. It must not connect to a modem, start a process, transmit, or
create connected-mode sessions from these records.
