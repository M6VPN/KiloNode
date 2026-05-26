/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/bbs_user.h */

#ifndef KILONODE_BBS_USER_H
#define KILONODE_BBS_USER_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/callsign.h"
#include "kilonode/message_store.h"

#define KN_BBS_USER_DISPLAY_MAX 64
#define KN_BBS_USER_HOME_MAX    64
#define KN_BBS_USER_LIST_MAX    256
#define KN_BBS_USER_NOTES_MAX   160

enum kn_bbs_user_error {
	KN_BBS_USER_OK = 0,
	KN_BBS_USER_ERR_INVALID_ARGUMENT,
	KN_BBS_USER_ERR_INVALID_CALLSIGN,
	KN_BBS_USER_ERR_IO,
	KN_BBS_USER_ERR_NOT_FOUND,
	KN_BBS_USER_ERR_EXISTS,
	KN_BBS_USER_ERR_DISABLED,
	KN_BBS_USER_ERR_CORRUPT,
	KN_BBS_USER_ERR_BUFFER
};

struct kn_bbs_user {
	struct kn_callsign callsign;
	char call[KN_CALLSIGN_MAX + 4];
	char display[KN_BBS_USER_DISPLAY_MAX];
	char home_bbs[KN_BBS_USER_HOME_MAX];
	char notes[KN_BBS_USER_NOTES_MAX];
	uint64_t created;
	uint64_t last_seen;
	uint64_t login_count;
	uint8_t sysop;
	uint8_t disabled;
};

enum kn_bbs_user_error kn_bbs_user_create(struct kn_message_store *,
	const char *, uint64_t, struct kn_bbs_user *);
enum kn_bbs_user_error kn_bbs_user_enable(struct kn_message_store *,
	const char *, uint8_t);
const char *kn_bbs_user_error_name(enum kn_bbs_user_error);
enum kn_bbs_user_error kn_bbs_user_format(const struct kn_bbs_user *,
	char *, size_t);
enum kn_bbs_user_error kn_bbs_user_init_store(struct kn_message_store *);
enum kn_bbs_user_error kn_bbs_user_list(struct kn_message_store *,
	struct kn_bbs_user *, size_t, size_t *);
enum kn_bbs_user_error kn_bbs_user_load(struct kn_message_store *,
	const char *, struct kn_bbs_user *);
enum kn_bbs_user_error kn_bbs_user_seen(struct kn_message_store *,
	const char *, uint64_t, struct kn_bbs_user *);
enum kn_bbs_user_error kn_bbs_user_save(struct kn_message_store *,
	const struct kn_bbs_user *);

#endif
