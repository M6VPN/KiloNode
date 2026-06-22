# External Modem Status Control

The external modem control surface is read-only. It lets operators inspect the
configured modem records and built-in support profiles.

## Commands

| Command              | Purpose                     |
| -------------------- | --------------------------- |
| `MODEMS`             | List configured modems      |
| `MODEM <name>`       | Show one configured modem   |
| `MODEM PROFILES`     | List built-in modem profiles |
| `MODEM PROFILE <type>` | Show one built-in profile |

Every multi-line response ends with `END`.

The commands do not start modems, connect sockets, enqueue transmit frames,
dispatch frames, expose CONNECT, or bind modem data to the shell or BBS.
