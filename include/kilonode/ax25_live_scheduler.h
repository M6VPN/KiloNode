/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_live_scheduler.h */

#ifndef KILONODE_AX25_LIVE_SCHEDULER_H
#define KILONODE_AX25_LIVE_SCHEDULER_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_scheduler_policy.h"

enum kn_ax25_live_scheduler_error {
	KN_AX25_LIVE_SCHEDULER_OK = 0,
	KN_AX25_LIVE_SCHEDULER_ERR_INVALID_ARGUMENT,
	KN_AX25_LIVE_SCHEDULER_ERR_INVALID_VALUE,
	KN_AX25_LIVE_SCHEDULER_ERR_SCHEDULER,
	KN_AX25_LIVE_SCHEDULER_ERR_MAPPER,
	KN_AX25_LIVE_SCHEDULER_ERR_BUFFER
};

struct kn_ax25_runtime;

struct kn_ax25_live_scheduler {
	struct kn_ax25_scheduler_policy policy;
	uint64_t last_poll_ms;
	uint64_t next_wakeup_ms;
	uint64_t cycles_run;
	uint64_t expired_processed;
	uint64_t expired_blocked_by_policy;
	uint64_t generated_frame_plans;
	uint64_t tx_actions_blocked;
	uint64_t tx_writes_attempted;
	enum kn_ax25_live_scheduler_error last_error;
};

void kn_ax25_live_scheduler_init(struct kn_ax25_live_scheduler *);
enum kn_ax25_live_scheduler_error kn_ax25_live_scheduler_poll(
	struct kn_ax25_runtime *, uint64_t);
void kn_ax25_live_scheduler_reset(struct kn_ax25_live_scheduler *);
enum kn_ax25_live_scheduler_error kn_ax25_live_scheduler_set_policy(
	struct kn_ax25_live_scheduler *,
	const struct kn_ax25_scheduler_policy *);

#endif
