# BBS Area Model

Bulletin areas group bulletin messages by a short area name. Areas are derived
from stored bulletin messages and indexes. There is no subscription state in
this pass.

Area validation:

- empty names are rejected
- names are normalized to uppercase
- names use letters, digits, `_`, and `-`
- names use the existing message area length limit
- unsafe path characters are rejected

`AREAS` output is based on indexed bulletin metadata:

```text
OK AREAS count=1
AREA name=GENERAL count=2 newest=4
END
```

`LIST AREA <area>` normalizes the area name and lists matching non-deleted
bulletins. Unknown valid areas return an empty list. Invalid area names return a
deterministic error.

Area subscriptions, per-user unread counts, expiry, and forwarding policy are
deferred.

Future BPQ and LinBPQ topic compatibility work must use black-box research only:
node sessions, packet captures, mailbox observations, config examples, and
externally visible behavior. GPL source code must not be copied, translated, or
structurally mirrored.
