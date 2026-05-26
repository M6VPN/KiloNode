# BBS User Model

The BBS user layer gives local BBS sessions a callsign identity. It is local
storage only. It does not add passwords, RF login, forwarding, or BPQ/LinBPQ
user compatibility.

User identity is the validated AX.25 callsign plus SSID. Callsigns are
normalized to the existing KiloNode callsign format, so `M6VPN-0` is stored as
`M6VPN` and `M6VPN-1` keeps the SSID suffix. Invalid callsigns and unsafe path
characters are rejected before file paths are built.

Store layout:

```text
messages/
	users/
		M6VPN-1.user
		N0CALL.user
	read/
		M6VPN-1.read
```

User files are KiloNode-native line-oriented text:

```text
call M6VPN-1
display 
home 
created 1710000000
last-seen 1710000300
login-count 2
sysop 0
disabled 0
notes 
```

Tracked fields:

- callsign and SSID
- optional display name
- optional home BBS text
- created timestamp
- last seen timestamp
- login count
- sysop flag, false by default
- disabled flag, false by default
- optional bounded notes

`BBS <callsign>` creates the user on first use, updates last seen, increments
login count, and enters identity-aware BBS mode. Disabled users are rejected.

Passwords and authentication are deferred. The current shell is local-only and
uses filesystem access plus localhost binding as its trust boundary.

This is KiloNode-native storage. It is not BPQ or LinBPQ user compatibility.
