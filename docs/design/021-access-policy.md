# Access Policy

KiloNode uses a native local access policy for the node shell, BBS shell, and
control socket. The policy is intentionally small in this pass. It limits local
interfaces and input size before later RF, public TCP, or authenticated access
is added.

This policy is not BPQ or LinBPQ security compatibility.

## Config

The optional access block is:

```text
access {
	default-policy allow
	allow-localhost true
	max-line-bytes 512
	max-command-bytes 512
	max-clients 8
	idle-timeout-seconds 900
	input-rate-lines 30
	input-rate-window-seconds 60
	bbs-max-body-bytes 65536
	control-max-command-bytes 512
	control-max-response-lines 200
}
```

If the block is omitted, the daemon uses these defaults.

## Fields

`default-policy` is `allow` or `deny`.

`allow-localhost` permits loopback shell clients when true. The example config
binds the node shell to `127.0.0.1`.

`max-line-bytes` caps input lines from shell clients.

`max-command-bytes` caps shell and BBS commands.

`max-clients` caps active local shell sessions.

`idle-timeout-seconds` closes inactive shell sessions.

`input-rate-lines` and `input-rate-window-seconds` cap command lines accepted
from a shell session during a time window.

`bbs-max-body-bytes` caps multiline BBS message body input.

`control-max-command-bytes` caps one control socket command line.

`control-max-response-lines` caps multiline control responses.

## Defaults

The shell remains disabled unless the `shell` block enables it. When enabled in
the example config, it binds to localhost only.

The control plane remains a local Unix socket. No remote TCP control is added.

The BBS shell uses both the BBS store body limit and the access policy body
limit. The smaller value wins.

## Not Enforced Yet

This pass does not add passwords, groups, TLS, remote sysop access, remote IP
ACLs, RF access control, or per-user quotas.
