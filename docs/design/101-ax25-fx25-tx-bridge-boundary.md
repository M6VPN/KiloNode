# AX.25 FX.25 TX Bridge Boundary

Prepared frames are AX.25 body bytes. They do not include HDLC flags, FCS, KISS
framing as the stored prepared object, or FX.25 wrapping.

FX.25 wrapping is a future stage after AX.25 frame generation and before final
physical or TNC framing. This pass rejects FX.25 bridge requests and does not
add FEC encode or decode.

The prepared-to-TX gate reports `fx25-not-supported` when a prepared frame or
policy asks for FX.25 wrapping.
