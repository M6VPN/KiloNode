/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_loopback_segment.h */

#ifndef KILONODE_AX25_LOOPBACK_SEGMENT_H
#define KILONODE_AX25_LOOPBACK_SEGMENT_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_loopback_endpoint.h"
#include "kilonode/ax25_loopback_link.h"

enum kn_ax25_loopback_segment_error {
	KN_AX25_LOOPBACK_SEGMENT_OK = 0,
	KN_AX25_LOOPBACK_SEGMENT_ERR_INVALID_ARGUMENT,
	KN_AX25_LOOPBACK_SEGMENT_ERR_INVALID_VALUE,
	KN_AX25_LOOPBACK_SEGMENT_ERR_STATE,
	KN_AX25_LOOPBACK_SEGMENT_ERR_UNSUPPORTED,
	KN_AX25_LOOPBACK_SEGMENT_ERR_TRANSFER,
	KN_AX25_LOOPBACK_SEGMENT_ERR_REASSEMBLY
};

enum kn_ax25_loopback_segment_error kn_ax25_loopback_segment_send(
	struct kn_ax25_loopback_endpoint *, struct kn_ax25_loopback_endpoint *,
	struct kn_ax25_loopback_link *, const uint8_t *, size_t, size_t *);

#endif
