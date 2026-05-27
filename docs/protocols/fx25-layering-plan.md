# FX.25 Layering Plan

FX.25 is planned as an optional layer that can wrap or recover AX.25 packet
bytes before normal AX.25 decode. It is disabled by default.

Planned RX flow:

1. Receive KISS or raw packet-boundary bytes.
2. If FX.25 is enabled, attempt detect/decode.
3. If FX.25 decode succeeds, pass embedded AX.25 bytes to AX.25 decode.
4. If FX.25 is absent and fallback is allowed, pass bytes to AX.25 decode.
5. Create the normal RX event.

Planned TX flow:

1. Build an AX.25 frame.
2. If FX.25 encode is enabled, wrap it in an FX.25 frame.
3. KISS-encode or dispatch through the configured transport.

M1.32 adds:

- FX.25 mode, decode status, payload relation, and FEC profile enums.
- Disabled-by-default FX.25 parameter validation.
- A decode placeholder that explicitly returns not implemented when enabled.

M1.32 does not add Reed-Solomon encode/decode, tag detection, AX.25 payload
extraction, or KISS path integration.
