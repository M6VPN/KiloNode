/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_rx_feed.h */

#ifndef KILONODE_AX25_RX_FEED_H
#define KILONODE_AX25_RX_FEED_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25.h"
#include "kilonode/ax25_connection_event.h"
#include "kilonode/ax25_runtime.h"
#include "kilonode/callsign.h"

enum kn_ax25_rx_feed_error {
	KN_AX25_RX_FEED_OK = 0,
	KN_AX25_RX_FEED_ERR_INVALID_ARGUMENT,
	KN_AX25_RX_FEED_ERR_DISABLED,
	KN_AX25_RX_FEED_ERR_IGNORED,
	KN_AX25_RX_FEED_ERR_NOT_RELEVANT,
	KN_AX25_RX_FEED_ERR_MALFORMED,
	KN_AX25_RX_FEED_ERR_RUNTIME
};

struct kn_ax25_rx_feed_options {
	uint8_t enabled;
	uint8_t create_connections;
	uint8_t retain_frame_plans;
};

struct kn_ax25_rx_feed_result {
	enum kn_ax25_rx_feed_error status;
	enum kn_ax25_connection_event event_kind;
	uint8_t event_generated;
	uint8_t accepted;
	uint8_t created;
	size_t record_index;
	size_t action_count;
	size_t frame_plan_count;
	size_t payload_len;
};

void kn_ax25_rx_feed_options_from_runtime(const struct kn_ax25_runtime *,
	struct kn_ax25_rx_feed_options *);
void kn_ax25_rx_feed_result_clear(struct kn_ax25_rx_feed_result *);
enum kn_ax25_rx_feed_error kn_ax25_rx_feed_frame(struct kn_ax25_runtime *,
	const struct kn_ax25_rx_feed_options *, const char *,
	const struct kn_callsign *, const struct kn_ax25_frame *, uint64_t,
	struct kn_ax25_rx_feed_result *);
enum kn_ax25_rx_feed_error kn_ax25_rx_feed_malformed(
	struct kn_ax25_runtime *, const struct kn_ax25_rx_feed_options *,
	size_t, struct kn_ax25_rx_feed_result *);

#endif
