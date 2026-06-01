# AX.25 Paclen Policy

M2.3 treats paclen as an AX.25 connected-mode simulator policy value. It is not
a BPQ or LinBPQ compatibility setting.

## Validation

The policy is:

- paclen must be at least 1
- max-info must be at least 1
- paclen must not exceed max-info
- paclen and max-info must fit the existing KiloNode AX.25 parameter bounds

The default remains the KiloNode AX.25 parameter default:

```text
max_info=256
paclen=256
```

Loopback scripts may set:

```text
params modulo=8 window=1 paclen=5 t1=3000 t2=1000 t3=300000 n2=3
```

`max-info=<n>` is also accepted for explicit simulator checks.

## Runtime Boundary

This pass does not add live daemon configuration or BPQ/LinBPQ config import
for paclen. Live segmentation and real transmit policy remain future work.
