# AX.25 Future TX Lab Notes

Future TX lab validation is blocked until the safety checklist is complete.

A future non-radiating lab must use a dummy load or isolated test path, explicit
operator controls, reviewed TX config, and captured logs. It must not enable
BBS, NET/ROM, or automatic dispatch unless each has its own reviewed gate.

Current bench scripts do not open devices, do not start daemons, and do not
transmit.
