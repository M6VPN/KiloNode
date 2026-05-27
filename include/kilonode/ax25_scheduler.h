/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_scheduler.h */

#ifndef KILONODE_AX25_SCHEDULER_H
#define KILONODE_AX25_SCHEDULER_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_action.h"
#include "kilonode/ax25_params.h"
#include "kilonode/ax25_retry.h"
#include "kilonode/ax25_state.h"
#include "kilonode/ax25_timer_queue.h"

#define KN_AX25_SCHEDULER_CONNECTION_MAX 32

enum kn_ax25_scheduler_error {
	KN_AX25_SCHEDULER_OK = 0,
	KN_AX25_SCHEDULER_ERR_INVALID_ARGUMENT,
	KN_AX25_SCHEDULER_ERR_INVALID_VALUE,
	KN_AX25_SCHEDULER_ERR_FULL,
	KN_AX25_SCHEDULER_ERR_NOT_FOUND,
	KN_AX25_SCHEDULER_ERR_TIMER_QUEUE,
	KN_AX25_SCHEDULER_ERR_STATE,
	KN_AX25_SCHEDULER_ERR_BUFFER
};

struct kn_ax25_scheduler_counters {
	uint64_t timers_started;
	uint64_t timers_stopped;
	uint64_t timers_expired;
	uint64_t retries_incremented;
	uint64_t retries_exhausted;
	uint64_t tx_queue_writes_attempted;
};

struct kn_ax25_scheduler_connection {
	uint8_t in_use;
	uint32_t connection_id;
	struct kn_ax25_retry retry;
};

struct kn_ax25_scheduler {
	struct kn_ax25_timer_queue queue;
	struct kn_ax25_scheduler_connection
	    connections[KN_AX25_SCHEDULER_CONNECTION_MAX];
	size_t connection_count;
	struct kn_ax25_scheduler_counters counters;
};

struct kn_ax25_scheduler_expiry_result {
	uint32_t connection_id;
	enum kn_ax25_timer_kind kind;
	enum kn_ax25_connection_event event;
	enum kn_ax25_state_error state_status;
	struct kn_ax25_action_list actions;
};

enum kn_ax25_scheduler_error kn_ax25_scheduler_apply_actions(
	struct kn_ax25_scheduler *, uint32_t, const struct kn_ax25_params *,
	const struct kn_ax25_action_list *, uint64_t);
enum kn_ax25_scheduler_error kn_ax25_scheduler_format(
	const struct kn_ax25_scheduler *, char *, size_t);
void kn_ax25_scheduler_init(struct kn_ax25_scheduler *);
enum kn_ax25_scheduler_error kn_ax25_scheduler_next_wakeup(
	const struct kn_ax25_scheduler *, uint64_t *);
enum kn_ax25_scheduler_error kn_ax25_scheduler_poll_expired(
	struct kn_ax25_scheduler *, uint64_t, struct kn_ax25_timer_expiry *,
	size_t, size_t *);
enum kn_ax25_scheduler_error kn_ax25_scheduler_process_expiry(
	struct kn_ax25_scheduler *, uint32_t, enum kn_ax25_timer_kind,
	struct kn_ax25_connection *, uint64_t,
	struct kn_ax25_scheduler_expiry_result *);
void kn_ax25_scheduler_reset(struct kn_ax25_scheduler *);
void kn_ax25_scheduler_result_clear(
	struct kn_ax25_scheduler_expiry_result *);

#endif
