# Receive-Only Safety

Keep transmit disabled during receive bench validation.

- Do not connect transmitter/PTT.
- Do not enable TX dispatch.
- Do not use lab TX configs while validating receive.
- Use receive audio or data output only.
- Confirm bench configs have `transmit enabled false`.
- Confirm each KISS port has `tx-enabled false`.
- Confirm `kilonodectl --socket /tmp/kilonode/control.sock tx gates` shows TX blocked.
- If using a transceiver, use receive audio/data output only.
- If using Dire Wolf, configure KISS output only and no PTT.
- If using KiloTNC, use receive or monitor mode where applicable.
- Follow local radio regulations.

The M1.38 bench scripts do not start RF hardware, install packages, or open
sound devices.
