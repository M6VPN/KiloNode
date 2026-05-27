/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_runtime.h */

#ifndef KILONODE_AX25_RUNTIME_H
#define KILONODE_AX25_RUNTIME_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_connection_table.h"
#include "kilonode/ax25_live_scheduler.h"
#include "kilonode/ax25_prepared_queue.h"
#include "kilonode/ax25_prepared_tx_policy.h"
#include "kilonode/ax25_scheduler.h"

enum kn_ax25_runtime_error {
	KN_AX25_RUNTIME_OK = 0,
	KN_AX25_RUNTIME_ERR_INVALID_ARGUMENT,
	KN_AX25_RUNTIME_ERR_INVALID_VALUE,
	KN_AX25_RUNTIME_ERR_DISABLED,
	KN_AX25_RUNTIME_ERR_TABLE
};

struct kn_ax25_runtime_counters {
	uint64_t events_accepted;
	uint64_t events_rejected;
	uint64_t connections_created;
	uint64_t connections_removed;
	uint64_t frame_plans_generated;
	uint64_t unsupported_frame_events;
	uint64_t protocol_errors;
};

struct kn_ax25_live_options {
	uint8_t live_rx_feed;
	uint8_t live_rx_create_connections;
	uint8_t live_rx_retain_frame_plans;
};

struct kn_ax25_live_counters {
	uint64_t frames_seen;
	uint64_t frames_accepted;
	uint64_t frames_ignored;
	uint64_t frames_malformed;
	uint64_t frames_not_relevant;
	uint64_t ui_ignored;
	uint64_t events_generated;
	uint64_t events_rejected;
	uint64_t frame_plans_generated;
	uint64_t frame_plans_retained;
	uint64_t tx_queue_writes_attempted;
};

struct kn_ax25_prepared_counters {
	uint64_t frames_attempted;
	uint64_t frames_stored;
	uint64_t build_failures;
	uint64_t queue_full;
	uint64_t bridge_blocked;
	uint64_t tx_queue_writes_attempted;
};

struct kn_ax25_prepared_tx_counters {
	uint64_t checks;
	uint64_t allowed;
	uint64_t blocked;
	uint64_t test_conversions;
	uint64_t tx_queue_writes;
	uint64_t fx25_blocked;
};

struct kn_ax25_runtime {
	uint8_t enabled;
	uint8_t connected_mode_enabled;
	uint8_t diagnostics_enabled;
	size_t max_connections;
	struct kn_ax25_params params;
	struct kn_ax25_connection_table table;
	struct kn_ax25_scheduler scheduler;
	struct kn_ax25_live_scheduler live_scheduler;
	struct kn_ax25_prepared_queue prepared_queue;
	struct kn_ax25_prepared_policy prepared_policy;
	struct kn_ax25_prepared_tx_policy prepared_tx_policy;
	struct kn_ax25_runtime_counters counters;
	struct kn_ax25_live_options live;
	struct kn_ax25_live_counters live_counters;
	struct kn_ax25_prepared_counters prepared_counters;
	struct kn_ax25_prepared_tx_counters prepared_tx_counters;
};

void kn_ax25_runtime_free(struct kn_ax25_runtime *);
enum kn_ax25_runtime_error kn_ax25_runtime_bridge_prepared_to_tx(
	struct kn_ax25_runtime *, uint64_t);
const struct kn_ax25_connection_record *kn_ax25_runtime_get_connection(
	const struct kn_ax25_runtime *, size_t);
void kn_ax25_runtime_init(struct kn_ax25_runtime *);
enum kn_ax25_runtime_error kn_ax25_runtime_inject_event(
	struct kn_ax25_runtime *, const struct kn_ax25_connection_event_record *,
	struct kn_ax25_connection_table_result *);
enum kn_ax25_runtime_error kn_ax25_runtime_prepare_plans(
	struct kn_ax25_runtime *, uint32_t, const char *, uint64_t,
	const struct kn_ax25_frame_plan_list *);
size_t kn_ax25_runtime_connection_count(const struct kn_ax25_runtime *);
void kn_ax25_runtime_reset(struct kn_ax25_runtime *);
enum kn_ax25_runtime_error kn_ax25_runtime_set_enabled(
	struct kn_ax25_runtime *, uint8_t, uint8_t);
enum kn_ax25_runtime_error kn_ax25_runtime_set_live_options(
	struct kn_ax25_runtime *, const struct kn_ax25_live_options *);
enum kn_ax25_runtime_error kn_ax25_runtime_set_scheduler_policy(
	struct kn_ax25_runtime *,
	const struct kn_ax25_scheduler_policy *);
enum kn_ax25_runtime_error kn_ax25_runtime_set_prepared_policy(
	struct kn_ax25_runtime *, const struct kn_ax25_prepared_policy *);
enum kn_ax25_runtime_error kn_ax25_runtime_set_prepared_tx_policy(
	struct kn_ax25_runtime *, const struct kn_ax25_prepared_tx_policy *);
enum kn_ax25_runtime_error kn_ax25_runtime_set_params(
	struct kn_ax25_runtime *, const struct kn_ax25_params *, size_t);

#endif
