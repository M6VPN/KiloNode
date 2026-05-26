/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/bbs_read_state.h */

#ifndef KILONODE_BBS_READ_STATE_H
#define KILONODE_BBS_READ_STATE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/message_index.h"

#define KN_BBS_READ_STATE_MAX 4096

enum kn_bbs_read_state_error {
	KN_BBS_READ_STATE_OK = 0,
	KN_BBS_READ_STATE_ERR_INVALID_ARGUMENT,
	KN_BBS_READ_STATE_ERR_INVALID_CALLSIGN,
	KN_BBS_READ_STATE_ERR_INVALID_ID,
	KN_BBS_READ_STATE_ERR_IO,
	KN_BBS_READ_STATE_ERR_CORRUPT,
	KN_BBS_READ_STATE_ERR_BUFFER
};

struct kn_bbs_read_state {
	char call[KN_CALLSIGN_MAX + 4];
	uint64_t ids[KN_BBS_READ_STATE_MAX];
	size_t count;
};

const char *kn_bbs_read_state_error_name(enum kn_bbs_read_state_error);
enum kn_bbs_read_state_error kn_bbs_read_state_init_store(
	struct kn_message_store *);
enum kn_bbs_read_state_error kn_bbs_read_state_is_read(
	struct kn_message_store *, const char *, uint64_t, uint8_t *);
enum kn_bbs_read_state_error kn_bbs_read_state_load(struct kn_message_store *,
	const char *, struct kn_bbs_read_state *);
enum kn_bbs_read_state_error kn_bbs_read_state_mark_read(
	struct kn_message_store *, const char *, uint64_t);
enum kn_bbs_read_state_error kn_bbs_read_state_save(
	struct kn_message_store *, const struct kn_bbs_read_state *);
enum kn_bbs_read_state_error kn_bbs_read_state_unread_count(
	struct kn_message_store *, const char *, const struct kn_message *,
	size_t, size_t *);

#endif
