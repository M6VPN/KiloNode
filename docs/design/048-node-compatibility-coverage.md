# Node Compatibility Coverage

M1.29 adds coverage reporting for node-command observation packs. Coverage
tracks whether commands have synthetic placeholders, manual black-box
observations, transcript candidates, and replayable test data.

## Commands

The initial coverage set is:

- HELP
- INFO
- PORTS
- USERS
- HEARD
- STATS
- BBS
- BYE
- UNKNOWN
- CONNECT
- NODES
- ROUTES

## Statuses

- `not-started`
- `synthetic`
- `observed`
- `transcript-candidate`
- `implemented-kilonode-native`
- `compatibility-planned`
- `out-of-scope-this-milestone`

KiloNode-native implementation status is tracked separately from BPQ/LinBPQ
compatibility. This pass records coverage only and does not implement
compatibility behaviour.

## Reports

`kilonode-compat pack-coverage` prints a deterministic line-based report with
summary counts and one line per command. Future compatibility work can use this
to choose what needs black-box observation before implementation.

## Deferred

- manual LinBPQ node observations
- compatibility command implementation
- connected-mode AX.25
- NET/ROM node behaviour
- RF BBS access
