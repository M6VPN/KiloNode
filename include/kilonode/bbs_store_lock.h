/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/bbs_store_lock.h */

#ifndef KILONODE_BBS_STORE_LOCK_H
#define KILONODE_BBS_STORE_LOCK_H

#include <sys/types.h>

#include "kilonode/message_store.h"

enum kn_bbs_store_lock_error {
	KN_BBS_STORE_LOCK_OK = 0,
	KN_BBS_STORE_LOCK_ERR_INVALID_ARGUMENT,
	KN_BBS_STORE_LOCK_ERR_BUSY,
	KN_BBS_STORE_LOCK_ERR_IO,
	KN_BBS_STORE_LOCK_ERR_BUFFER
};

struct kn_bbs_store_lock {
	int fd;
	char path[KN_MESSAGE_STORE_PATH_MAX];
	uint8_t locked;
};

enum kn_bbs_store_lock_error kn_bbs_store_lock_exclusive(
	struct kn_bbs_store_lock *, const char *);
const char *kn_bbs_store_lock_error_name(enum kn_bbs_store_lock_error);
void kn_bbs_store_lock_init(struct kn_bbs_store_lock *);
void kn_bbs_store_lock_release(struct kn_bbs_store_lock *);

#endif
