/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_live_diag.h */

#ifndef KILONODE_AX25_LIVE_DIAG_H
#define KILONODE_AX25_LIVE_DIAG_H

#include <sys/types.h>

#include "kilonode/ax25_runtime.h"

enum kn_ax25_live_diag_error {
	KN_AX25_LIVE_DIAG_OK = 0,
	KN_AX25_LIVE_DIAG_ERR_INVALID_ARGUMENT,
	KN_AX25_LIVE_DIAG_ERR_TRUNCATED
};

enum kn_ax25_live_diag_error kn_ax25_live_diag_format(
	const struct kn_ax25_runtime *, char *, size_t);

#endif
