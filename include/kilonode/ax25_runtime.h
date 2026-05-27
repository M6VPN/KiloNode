/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_runtime.h */

#ifndef KILONODE_AX25_RUNTIME_H
#define KILONODE_AX25_RUNTIME_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_connection_table.h"

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

struct kn_ax25_runtime {
	uint8_t enabled;
	uint8_t connected_mode_enabled;
	uint8_t diagnostics_enabled;
	size_t max_connections;
	struct kn_ax25_params params;
	struct kn_ax25_connection_table table;
	struct kn_ax25_runtime_counters counters;
};

void kn_ax25_runtime_free(struct kn_ax25_runtime *);
const struct kn_ax25_connection_record *kn_ax25_runtime_get_connection(
	const struct kn_ax25_runtime *, size_t);
void kn_ax25_runtime_init(struct kn_ax25_runtime *);
enum kn_ax25_runtime_error kn_ax25_runtime_inject_event(
	struct kn_ax25_runtime *, const struct kn_ax25_connection_event_record *,
	struct kn_ax25_connection_table_result *);
size_t kn_ax25_runtime_connection_count(const struct kn_ax25_runtime *);
void kn_ax25_runtime_reset(struct kn_ax25_runtime *);
enum kn_ax25_runtime_error kn_ax25_runtime_set_enabled(
	struct kn_ax25_runtime *, uint8_t, uint8_t);
enum kn_ax25_runtime_error kn_ax25_runtime_set_params(
	struct kn_ax25_runtime *, const struct kn_ax25_params *, size_t);

#endif
