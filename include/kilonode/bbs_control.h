/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/bbs_control.h */

#ifndef KILONODE_BBS_CONTROL_H
#define KILONODE_BBS_CONTROL_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/message_store.h"

#define KN_BBS_CONTROL_LIST_MAX    32
#define KN_BBS_CONTROL_PREVIEW_MAX 200

enum kn_bbs_control_error {
	KN_BBS_CONTROL_OK = 0,
	KN_BBS_CONTROL_ERR_INVALID_ARGUMENT,
	KN_BBS_CONTROL_ERR_DISABLED,
	KN_BBS_CONTROL_ERR_STORE,
	KN_BBS_CONTROL_ERR_BUFFER,
	KN_BBS_CONTROL_ERR_UNKNOWN_COMMAND,
	KN_BBS_CONTROL_ERR_INVALID_ID,
	KN_BBS_CONTROL_ERR_INVALID_AREA,
	KN_BBS_CONTROL_ERR_INVALID_CALLSIGN
};

const char *kn_bbs_control_error_name(enum kn_bbs_control_error);
enum kn_bbs_control_error kn_bbs_control_format(const char *, uint8_t,
	struct kn_message_store *, char *, size_t);

#endif
