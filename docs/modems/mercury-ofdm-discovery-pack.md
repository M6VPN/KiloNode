# Mercury OFDM Discovery Pack

Status: planned, not implemented.

The goal is to support Mercury OFDM from Rhizomatica as an external HF OFDM
modem when its integration boundary is documented well enough for KiloNode to
use it safely. KiloNode should talk only to a documented process or TCP
interface. KiloNode must not copy Mercury source, vendor Mercury code, or
implement Mercury DSP internals.

This pass does not assume a TCP framing format, control protocol, data protocol,
ARQ model, PTT model, or payload format.

Required discovery items:

- Mercury project URL or local path.
- Licence and redistribution notes.
- Build and run method.
- Control interface.
- Data interface.
- RX data format.
- TX data format.
- Status and telemetry commands.
- Connect or session model, if any.
- PTT responsibility.
- Sample receive capture.
- Test or simulation mode.
- Expected HF channel use.
- Error reporting.
- Whether AX.25 frames are carried directly or through another framing layer.

No implementation should begin until these items are answered from documentation
or clean black-box observation.
