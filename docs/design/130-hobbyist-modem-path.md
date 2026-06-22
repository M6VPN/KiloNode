# Hobbyist Modem Path

M2 shifts toward practical hobbyist use while keeping transmit disabled by
default. The recommended modem path is:

1. Run no-hardware checks.
2. Use Dire Wolf TCP KISS for receive-only bench testing.
3. Use KiloTNC KISS receive when hardware is available.
4. Inspect modem profiles with `kilonodectl modem-profiles`.
5. Track Mercury OFDM, VARA HF/FM, and ARDOP as planned external adapters.

The current product path is receive and diagnostics first. Real transmit,
CONNECT, shell/BBS connected-mode binding, process launch, and external modem
protocol adapters remain future gated work.
