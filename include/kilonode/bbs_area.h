/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/bbs_area.h */

#ifndef KILONODE_BBS_AREA_H
#define KILONODE_BBS_AREA_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/message.h"

enum kn_bbs_area_error {
	KN_BBS_AREA_OK = 0,
	KN_BBS_AREA_ERR_INVALID_ARGUMENT,
	KN_BBS_AREA_ERR_INVALID_NAME,
	KN_BBS_AREA_ERR_BUFFER
};

enum kn_bbs_area_error kn_bbs_area_normalize(const char *, char *, size_t);
uint8_t kn_bbs_area_valid(const char *);

#endif
