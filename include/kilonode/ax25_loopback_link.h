/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_loopback_link.h */

#ifndef KILONODE_AX25_LOOPBACK_LINK_H
#define KILONODE_AX25_LOOPBACK_LINK_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_loopback_endpoint.h"

#define KN_AX25_LOOPBACK_LINK_FRAME_MAX 256
#define KN_AX25_LOOPBACK_LINK_STEP_MAX  16

enum kn_ax25_loopback_link_error {
	KN_AX25_LOOPBACK_LINK_OK = 0,
	KN_AX25_LOOPBACK_LINK_ERR_INVALID_ARGUMENT,
	KN_AX25_LOOPBACK_LINK_ERR_INVALID_VALUE,
	KN_AX25_LOOPBACK_LINK_ERR_FRAME_TOO_LARGE,
	KN_AX25_LOOPBACK_LINK_ERR_ENDPOINT
};

struct kn_ax25_loopback_link {
	uint64_t raw_ax25_frames_transferred;
	uint64_t malformed_frames;
	uint64_t fx25_frames_generated;
	size_t max_frames_per_step;
};

void kn_ax25_loopback_link_init(struct kn_ax25_loopback_link *);
void kn_ax25_loopback_link_reset(struct kn_ax25_loopback_link *);
enum kn_ax25_loopback_link_error kn_ax25_loopback_link_transfer(
	struct kn_ax25_loopback_link *, struct kn_ax25_loopback_endpoint *,
	struct kn_ax25_loopback_endpoint *, size_t *);
enum kn_ax25_loopback_link_error kn_ax25_loopback_link_transfer_raw(
	struct kn_ax25_loopback_link *, struct kn_ax25_loopback_endpoint *,
	const uint8_t *, size_t);

#endif
