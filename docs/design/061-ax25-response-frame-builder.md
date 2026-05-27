# AX.25 Response Frame Builder

M1.34 adds a raw AX.25 response-frame builder for connected-mode control
frames. The builder emits AX.25 body bytes only.

## Supported Frames

| Frame | Status |
|-------|--------|
| SABM  | Built and tested |
| SABME | Built and tested |
| UA    | Built and tested |
| DM    | Built and tested |
| DISC  | Built and tested |
| RR    | Built and tested |
| RNR   | Built and tested |
| REJ   | Built and tested |
| FRMR  | Placeholder frame type only |
| UI    | Regression path only |

I-frame data building is deferred.

## Boundary

The builder validates source, destination, optional digipeaters, poll/final,
and supervisory N(R). It uses the project AX.25 address encoder and new
control-byte encode helpers.

The output excludes:

- HDLC flags
- FCS bytes
- KISS command bytes
- KISS escaping
- FX.25 wrapping

Those belong to later layers.

## Control Encoding

The control helper now encodes U and S control bytes and the existing decoder
is used in tests for round-trip validation. Modulo 8 sequence fields are
supported for S-frame N(R). Extended modulo 128 control encoding remains
planned.

## Safety

The builder writes only into caller-provided buffers, returns bounded
enum-style errors, and does not allocate persistent state.
