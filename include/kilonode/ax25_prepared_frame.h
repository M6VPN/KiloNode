/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_prepared_frame.h */

#ifndef KILONODE_AX25_PREPARED_FRAME_H
#define KILONODE_AX25_PREPARED_FRAME_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_frame_plan.h"

#define KN_AX25_PREPARED_FRAME_RAW_MAX     256
#define KN_AX25_PREPARED_FRAME_HEX_PREVIEW 64
#define KN_AX25_PREPARED_FRAME_REASON_MAX  64
#define KN_AX25_PREPARED_FRAME_PORT_MAX    32

enum kn_ax25_prepared_frame_error {
	KN_AX25_PREPARED_FRAME_OK = 0,
	KN_AX25_PREPARED_FRAME_ERR_INVALID_ARGUMENT,
	KN_AX25_PREPARED_FRAME_ERR_INVALID_VALUE,
	KN_AX25_PREPARED_FRAME_ERR_BUILD,
	KN_AX25_PREPARED_FRAME_ERR_BUFFER
};

enum kn_ax25_prepared_frame_status {
	KN_AX25_PREPARED_FRAME_STATUS_PREPARED = 0,
	KN_AX25_PREPARED_FRAME_STATUS_BUILD_FAILED,
	KN_AX25_PREPARED_FRAME_STATUS_SUPPRESSED,
	KN_AX25_PREPARED_FRAME_STATUS_TX_BRIDGE_BLOCKED
};

struct kn_ax25_prepared_frame {
	uint64_t id;
	uint64_t created_ms;
	uint32_t connection_id;
	char port_name[KN_AX25_PREPARED_FRAME_PORT_MAX];
	struct kn_callsign local;
	struct kn_callsign remote;
	struct kn_callsign digipeaters[KN_AX25_MAX_DIGIS];
	size_t digipeater_count;
	enum kn_ax25_action_intent action_source;
	enum kn_ax25_frame_plan_type type;
	uint8_t poll_final;
	uint8_t nr;
	uint8_t ns;
	uint8_t raw[KN_AX25_PREPARED_FRAME_RAW_MAX];
	size_t raw_len;
	size_t payload_len;
	enum kn_ax25_prepared_frame_status status;
	char reason[KN_AX25_PREPARED_FRAME_REASON_MAX];
	uint8_t needs_fx25;
	uint8_t needs_kiss;
};

void kn_ax25_prepared_frame_clear(struct kn_ax25_prepared_frame *);
enum kn_ax25_prepared_frame_error kn_ax25_prepared_frame_from_plan(
	struct kn_ax25_prepared_frame *, const struct kn_ax25_frame_plan *,
	uint32_t, const char *, uint64_t, uint8_t);
enum kn_ax25_prepared_frame_error kn_ax25_prepared_frame_hex_preview(
	const struct kn_ax25_prepared_frame *, char *, size_t);
const char *kn_ax25_prepared_frame_status_name(
	enum kn_ax25_prepared_frame_status);
enum kn_ax25_prepared_frame_error kn_ax25_prepared_frame_format(
	const struct kn_ax25_prepared_frame *, char *, size_t);
enum kn_ax25_prepared_frame_error kn_ax25_prepared_frame_validate(
	const struct kn_ax25_prepared_frame *);

#endif
