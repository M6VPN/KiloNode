# Mercury OFDM External Modem Roadmap

Mercury is a Rhizomatica HF OFDM modem project. KiloNode support is planned as
an external modem adapter only.

KiloNode must not vendor, copy, link, or embed Mercury implementation code. The
future integration boundary is a documented external process or TCP interface.

## Planned Boundary

- Mercury runs as a separate modem process.
- KiloNode stores external endpoint settings only.
- Initial KiloNode work should be read-only configuration and status probing.
- No automatic TX, PTT, or live CONNECT support is enabled by this roadmap.

## Current Status

| Item | Status |
| ---- | ------ |
| Config schema | planned |
| Read-only status | planned |
| Receive or bench workflow | planned |
| TX bridge | blocked |
| Live CONNECT | blocked |

## Safety Notes

Mercury support must stay outside AX.25 timer, scheduler, and prepared-frame
logic. If a future adapter passes AX.25 bytes, that byte boundary must be
documented before any TX or connected-session work.
