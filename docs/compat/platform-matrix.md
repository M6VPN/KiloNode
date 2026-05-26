# Platform Matrix

Status values: `planned`, `partial`, `tested`, `unsupported`.

| Platform | Build | Tests | Serial KISS | PTY KISS | Unix socket | TCP KISS | Store locking | Sanitizers | Status | Notes |
| -------- | ----- | ----- | ----------- | -------- | ----------- | -------- | ------------- | ---------- | ------ | ----- |
| Linux GCC | tested | tested | partial | tested | tested | tested | tested | tested | tested | Primary CI target |
| Linux Clang | tested | tested | partial | tested | tested | tested | tested | tested | tested | Primary CI and sanitizer target |
| OpenBSD clang | planned | planned | planned | planned | planned | planned | planned | planned | partial | Strong target, local validation pending |
| FreeBSD clang | planned | planned | planned | planned | planned | planned | planned | planned | partial | Strong target, local validation pending |
| NetBSD clang/gcc | planned | planned | planned | planned | planned | planned | planned | planned | planned | Practical target when API gaps are known |
| macOS clang | planned | planned | planned | planned | planned | planned | planned | partial | planned | Later target |
| Linux systemd service example | tested | tested | unsupported | unsupported | unsupported | unsupported | unsupported | unsupported | tested | Example service only |
| OpenBSD rc.d example | tested | planned | unsupported | unsupported | unsupported | unsupported | unsupported | unsupported | partial | Example script only |
| FreeBSD rc.d example | tested | planned | unsupported | unsupported | unsupported | unsupported | unsupported | unsupported | partial | Example script only |
| NetBSD rc.d example | tested | planned | unsupported | unsupported | unsupported | unsupported | unsupported | unsupported | partial | Example script only |
| CMake install target | tested | tested | unsupported | unsupported | unsupported | unsupported | unsupported | unsupported | tested | Installs tools, examples, manpages, and service examples |
| Manpages | tested | tested | unsupported | unsupported | unsupported | unsupported | unsupported | unsupported | tested | Stub pages for installed tools |
