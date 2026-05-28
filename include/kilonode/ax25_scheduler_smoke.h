/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_scheduler_smoke.h */

#ifndef KILONODE_AX25_SCHEDULER_SMOKE_H
#define KILONODE_AX25_SCHEDULER_SMOKE_H

#include <sys/types.h>

#include <stdint.h>

enum kn_ax25_scheduler_smoke_error {
	KN_AX25_SCHEDULER_SMOKE_OK = 0,
	KN_AX25_SCHEDULER_SMOKE_ERR_INVALID_ARGUMENT,
	KN_AX25_SCHEDULER_SMOKE_ERR_INVALID_VALUE,
	KN_AX25_SCHEDULER_SMOKE_ERR_RUNTIME
};

struct kn_ax25_runtime;

struct kn_ax25_scheduler_smoke_options {
	uint8_t enabled;
	uint8_t create_test_connection;
};

struct kn_ax25_scheduler_smoke_counters {
	uint64_t cycles;
	uint64_t test_connections_created;
	uint64_t scheduler_polls;
	uint64_t expired_processed;
	uint64_t prepared_frames_generated;
	uint64_t prepared_bridge_blocked;
	uint64_t tx_writes_attempted;
	uint64_t dispatch_calls_attempted;
	enum kn_ax25_scheduler_smoke_error last_error;
};

void kn_ax25_scheduler_smoke_options_default(
	struct kn_ax25_scheduler_smoke_options *);
enum kn_ax25_scheduler_smoke_error kn_ax25_scheduler_smoke_poll(
	struct kn_ax25_runtime *, uint64_t);
void kn_ax25_scheduler_smoke_reset_counters(
	struct kn_ax25_scheduler_smoke_counters *);
enum kn_ax25_scheduler_smoke_error kn_ax25_scheduler_smoke_validate(
	const struct kn_ax25_runtime *,
	const struct kn_ax25_scheduler_smoke_options *);

#endif
