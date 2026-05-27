# Command Context Model

The command context is the read-only state passed into the runtime native
dispatcher. It keeps command parsing separate from daemon state ownership.

## Local Shell Context

The local shell context exposes:

- node callsign
- alias
- location
- port stats
- heard entries
- daemon stats
- local shell users
- BBS availability
- output limit

The local shell still owns sessions, prompts, access-policy checks, rate limits,
idle timeout handling, and BBS mode state.

## RF UI Context

The RF UI context exposes:

- node callsign
- alias
- port stats
- heard entries
- daemon stats
- output limit

RF eligibility, source rate limits, ignore lists, reply gates, TX queueing, and
dispatch gates remain outside the command dispatcher.

## BBS Boundary

The BBS shell remains unchanged in this pass. The native dispatcher can identify
the BBS mode transition for the local node shell, but message commands are still
handled by the existing BBS shell code.

## Output Safety

Outputs are bounded by the caller context. Local shell output can be multiline.
RF UI output remains short and bounded by the RF reply configuration. Unsafe
node text is escaped or replaced before it reaches command output.

## Deferred Contexts

- compatibility command context
- connected-mode AX.25 context
- RF BBS context
- NET/ROM context
- sysop/auth context
