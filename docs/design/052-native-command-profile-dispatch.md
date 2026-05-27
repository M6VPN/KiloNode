# Native Command Profile Dispatch

M1.31 adds runtime KiloNode-native command profiles and a shared dispatcher for
local node shell commands and RF UI command replies.

This is not BPQ/LinBPQ compatibility. The profile table contains only
KiloNode-native commands and does not add external aliases, abbreviations,
prompts, output templates, parser logic, CONNECT behaviour, NET/ROM, RF BBS
access, forwarding, or TX commands.

## Profile Fields

Each runtime profile records:

- command name
- minimum abbreviation length, currently `0` for exact match only
- aliases, currently none
- allowed contexts
- argument policy
- output class
- safety class
- internal command ID

Initial runtime profiles cover:

- HELP
- INFO
- PORTS
- HEARD
- STATS
- USERS
- BBS
- BYE
- QUIT
- PING

## Context Differences

Local node shell allows HELP, INFO, PORTS, HEARD, USERS, STATS, BBS, BYE, and
QUIT. BBS remains a local session transition handled by the existing BBS shell.

RF UI allows HELP, INFO, PORTS, HEARD, STATS, and PING. BBS, USERS, BYE, and
QUIT are not allowed over RF UI in this pass.

## Dispatcher Flow

The dispatcher:

1. parses bounded input bytes
2. trims whitespace
3. uppercases the command token
4. rejects control characters and overlong input
5. finds a KiloNode-native profile
6. validates context and argument policy
7. formats bounded output or returns a structured action

The result records status, profile name, output text, close-session flag, and
mode-transition flag.

## Deferred

- BPQ/LinBPQ command aliases
- BPQ/LinBPQ output compatibility
- connected-mode command context
- NET/ROM command context
- RF BBS command context
- sysop/auth command context
