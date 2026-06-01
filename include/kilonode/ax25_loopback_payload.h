/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_loopback_payload.h */

#ifndef KILONODE_AX25_LOOPBACK_PAYLOAD_H
#define KILONODE_AX25_LOOPBACK_PAYLOAD_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_i_frame.h"
#include "kilonode/ax25_loopback_endpoint.h"

enum kn_ax25_loopback_payload_error {
	KN_AX25_LOOPBACK_PAYLOAD_OK = 0,
	KN_AX25_LOOPBACK_PAYLOAD_ERR_INVALID_ARGUMENT,
	KN_AX25_LOOPBACK_PAYLOAD_ERR_INVALID_VALUE,
	KN_AX25_LOOPBACK_PAYLOAD_ERR_STATE,
	KN_AX25_LOOPBACK_PAYLOAD_ERR_BUFFER
};

enum kn_ax25_loopback_payload_error kn_ax25_loopback_payload_record(
	struct kn_ax25_loopback_endpoint *,
	const struct kn_ax25_i_frame_decoded *, uint8_t, const char *);
enum kn_ax25_loopback_payload_error kn_ax25_loopback_payload_send_i(
	struct kn_ax25_loopback_endpoint *, const uint8_t *, size_t,
	uint8_t, uint8_t, uint8_t *, size_t, size_t *);

#endif
