# Session Hardening

The local node shell, BBS shell, and control socket treat all input as hostile.
This pass adds bounded input handling and deterministic rejection paths without
changing KiloNode into a public access service.

## Node Shell

Each shell session has a bounded line buffer, last activity timestamp, and input
rate window. Overlong input is discarded and rejected. Idle sessions are closed.
Rate limit violations close the session after a clear error.

The daemon enforces the configured maximum client count before admitting a new
shell session.

## BBS Shell

BBS commands use the shell command and line limits. Multiline message bodies are
bounded by the configured BBS body limit. If a body grows beyond the limit, the
pending message is aborted and no partial message is stored.

The BBS shell keeps using explicit lengths for body input and does not treat
message text as trusted terminal output.

## Control Socket

The control socket accepts one bounded command line per request. Overlong
commands are rejected deterministically.

Multiline responses are capped by policy. If a response would exceed the cap,
the command fails with a deterministic response-line-limit error.

## Policy Violations

Policy violations are local session events. A bad client can be rejected or
closed without breaking the daemon loop or other sessions.

Daemon stderr logging may report rejections, but tests do not depend on log
output.

## Deferred Work

Deferred work includes password authentication, sysop authentication, TLS,
remote IP ACLs, RF-specific access control, per-user quotas, per-area
permissions, and public access mode.
