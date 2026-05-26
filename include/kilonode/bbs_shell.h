/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/bbs_shell.h */

#ifndef KILONODE_BBS_SHELL_H
#define KILONODE_BBS_SHELL_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/message.h"
#include "kilonode/message_store.h"

#define KN_BBS_SHELL_PROMPT "BBS> "

enum kn_bbs_shell_pending_type {
	KN_BBS_SHELL_PENDING_NONE = 0,
	KN_BBS_SHELL_PENDING_PRIVATE,
	KN_BBS_SHELL_PENDING_BULLETIN
};

enum kn_bbs_shell_error {
	KN_BBS_SHELL_OK = 0,
	KN_BBS_SHELL_ERR_INVALID_ARGUMENT,
	KN_BBS_SHELL_ERR_BUFFER,
	KN_BBS_SHELL_ERR_STORE
};

struct kn_bbs_shell_session {
	uint8_t active;
	enum kn_bbs_shell_pending_type pending_type;
	char pending_from[KN_CALLSIGN_MAX + 4];
	char pending_dest[KN_MESSAGE_AREA_MAX + 1];
	char pending_subject[KN_MESSAGE_SUBJECT_MAX + 1];
	uint8_t body[KN_MESSAGE_BODY_MAX];
	size_t body_len;
	uint8_t body_overflow;
};

struct kn_bbs_shell_snapshot {
	struct kn_message_store *store;
	uint8_t enabled;
	size_t max_body_bytes;
};

enum kn_bbs_shell_error kn_bbs_shell_format(
	struct kn_bbs_shell_session *, const char *,
	const struct kn_bbs_shell_snapshot *, char *, size_t, uint8_t *,
	uint8_t *);
void kn_bbs_shell_reset(struct kn_bbs_shell_session *);

#endif
