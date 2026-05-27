/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_timer_replay.h */

#ifndef KILONODE_AX25_TIMER_REPLAY_H
#define KILONODE_AX25_TIMER_REPLAY_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_action.h"
#include "kilonode/ax25_frame_plan.h"
#include "kilonode/ax25_prepared_frame.h"
#include "kilonode/ax25_runtime.h"
#include "kilonode/ax25_timer_replay_script.h"

#define KN_AX25_TIMER_REPLAY_MISMATCH_MAX 16
#define KN_AX25_TIMER_REPLAY_PREPARED_MAX 16

struct kn_ax25_timer_replay_mismatch {
	size_t line;
	char detail[KN_AX25_TIMER_REPLAY_TEXT_MAX];
};

struct kn_ax25_timer_replay_result {
	char name[KN_AX25_TIMER_REPLAY_NAME_MAX];
	uint8_t pass;
	enum kn_ax25_connection_state final_state;
	uint8_t retry_count;
	uint8_t t1_running;
	uint8_t t2_running;
	uint8_t t3_running;
	size_t connection_count;
	uint64_t frame_plans_seen;
	uint64_t prepared_count;
	uint64_t tx_writes_observed;
	uint64_t prepared_tx_writes_observed;
	uint64_t events_accepted;
	uint64_t protocol_errors;
	struct kn_ax25_prepared_frame
	    prepared[KN_AX25_TIMER_REPLAY_PREPARED_MAX];
	struct kn_ax25_action_list last_actions;
	struct kn_ax25_frame_plan_list last_plans;
	struct kn_ax25_scheduler_counters scheduler_counters;
	struct kn_ax25_timer_replay_mismatch
	    mismatches[KN_AX25_TIMER_REPLAY_MISMATCH_MAX];
	size_t mismatch_count;
};

void kn_ax25_timer_replay_result_clear(
	struct kn_ax25_timer_replay_result *);
enum kn_ax25_timer_replay_error kn_ax25_timer_replay_run(
	const struct kn_ax25_timer_replay_script *,
	struct kn_ax25_timer_replay_result *,
	struct kn_ax25_timer_replay_error_info *);
enum kn_ax25_timer_replay_error kn_ax25_timer_replay_run_file(
	const char *, struct kn_ax25_timer_replay_result *,
	struct kn_ax25_timer_replay_error_info *);

#endif
