# RF Command Transcript Format

The compatibility lab transcript format is KiloNode-native. It records expected
external behaviour for replay tests and future black-box observations.

## Syntax

- UTF-8 compatible text.
- Comments start with `#`.
- Blank lines are ignored.
- One key and value per line.
- Keys are case-sensitive.
- Unknown keys are rejected.
- Duplicate keys are rejected.
- Line length is bounded to 512 bytes.

## RF UI Mode

Current supported mode:

```text
mode rf-ui
```

Required keys:

```text
name rf-ping
mode rf-ui
node M6VPN-1
port kiss0
source N0CALL
destination M6VPN-1
pid 0xf0
input PING
expect-event command=PING status=ok
expect-reply contains=PONG
```

Optional keys:

```text
expect-reply-queued true
expect-no-dispatch true
expect-error reason
```

## Expected Replies

Supported reply expectations:

```text
expect-reply none
expect-reply contains=PONG
expect-reply exact=PONG
```

`expect-no-dispatch` defaults to true. Replay may queue a dry-run TX frame, but
must not dispatch it.

## Example

```text
# KiloNode compatibility transcript v1
name rf-ping
mode rf-ui
node M6VPN-1
port kiss0
source N0CALL
destination M6VPN-1
pid 0xf0
input PING
expect-event command=PING status=ok
expect-reply contains=PONG
expect-reply-queued true
expect-no-dispatch true
```
