/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_frame_builder.h */

#ifndef KILONODE_AX25_FRAME_BUILDER_H
#define KILONODE_AX25_FRAME_BUILDER_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_frame_plan.h"

enum kn_ax25_frame_builder_error {
	KN_AX25_FRAME_BUILDER_OK = 0,
	KN_AX25_FRAME_BUILDER_ERR_INVALID_ARGUMENT,
	KN_AX25_FRAME_BUILDER_ERR_INVALID_VALUE,
	KN_AX25_FRAME_BUILDER_ERR_TOO_MANY_DIGIS,
	KN_AX25_FRAME_BUILDER_ERR_TOO_LARGE,
	KN_AX25_FRAME_BUILDER_ERR_BUFFER,
	KN_AX25_FRAME_BUILDER_ERR_UNSUPPORTED
};

struct kn_ax25_frame_builder_request {
	struct kn_callsign source;
	struct kn_callsign destination;
	struct kn_callsign digipeaters[KN_AX25_MAX_DIGIS];
	size_t digipeater_count;
	enum kn_ax25_frame_plan_type type;
	uint8_t poll_final;
	uint8_t nr;
	uint8_t pid;
	const uint8_t *payload;
	size_t payload_len;
};

enum kn_ax25_frame_builder_error kn_ax25_frame_builder_build(
	const struct kn_ax25_frame_builder_request *, uint8_t *, size_t,
	size_t *);
enum kn_ax25_frame_builder_error kn_ax25_frame_builder_build_plan(
	const struct kn_ax25_frame_plan *, uint8_t *, size_t, size_t *);
void kn_ax25_frame_builder_request_clear(
	struct kn_ax25_frame_builder_request *);
enum kn_ax25_frame_builder_error kn_ax25_frame_builder_request_from_plan(
	const struct kn_ax25_frame_plan *,
	struct kn_ax25_frame_builder_request *);

#endif
