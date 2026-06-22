# Mercury OFDM

Mercury OFDM by Rhizomatica is a planned external HF OFDM modem target. KiloNode
does not implement Mercury protocol logic, DSP, OFDM, FEC, framing, process
launch, or transmit support in this milestone.

KiloNode treats Mercury as an external boundary. A future adapter may talk to a
documented process or TCP interface after the interface discovery checklist is
complete. No Mercury source is vendored, copied, linked, embedded, or used as an
implementation reference.

Use `packaging/examples/kilonode-mercury-planned.conf` to inspect the planned
profile through KiloNode status commands. The config keeps Mercury disabled,
status-only, no autostart, no CONNECT, and no transmit.
