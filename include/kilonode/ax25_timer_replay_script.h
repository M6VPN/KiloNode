/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_timer_replay_script.h */

#ifndef KILONODE_AX25_TIMER_REPLAY_SCRIPT_H
#define KILONODE_AX25_TIMER_REPLAY_SCRIPT_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_connection.h"
#include "kilonode/ax25_frame_plan.h"
#include "kilonode/ax25_params.h"
#include "kilonode/ax25_timer.h"

#define KN_AX25_TIMER_REPLAY_COMMAND_MAX 128
#define KN_AX25_TIMER_REPLAY_NAME_MAX    64
#define KN_AX25_TIMER_REPLAY_TEXT_MAX    128
#define KN_AX25_TIMER_REPLAY_LINE_MAX    512

enum kn_ax25_timer_replay_error {
	KN_AX25_TIMER_REPLAY_OK = 0,
	KN_AX25_TIMER_REPLAY_ERR_INVALID_ARGUMENT,
	KN_AX25_TIMER_REPLAY_ERR_INVALID_VALUE,
	KN_AX25_TIMER_REPLAY_ERR_PARSE,
	KN_AX25_TIMER_REPLAY_ERR_IO,
	KN_AX25_TIMER_REPLAY_ERR_FULL,
	KN_AX25_TIMER_REPLAY_ERR_MISMATCH,
	KN_AX25_TIMER_REPLAY_ERR_BUFFER
};

enum kn_ax25_timer_replay_command_type {
	KN_AX25_TIMER_REPLAY_CMD_NOW = 0,
	KN_AX25_TIMER_REPLAY_CMD_ADVANCE,
	KN_AX25_TIMER_REPLAY_CMD_PARAMS,
	KN_AX25_TIMER_REPLAY_CMD_EVENT,
	KN_AX25_TIMER_REPLAY_CMD_PROCESS_EXPIRED,
	KN_AX25_TIMER_REPLAY_CMD_EXPECT
};

enum kn_ax25_timer_replay_expect_type {
	KN_AX25_TIMER_REPLAY_EXPECT_STATE = 0,
	KN_AX25_TIMER_REPLAY_EXPECT_RETRY,
	KN_AX25_TIMER_REPLAY_EXPECT_TIMER_RUNNING,
	KN_AX25_TIMER_REPLAY_EXPECT_TIMER_EXPIRED,
	KN_AX25_TIMER_REPLAY_EXPECT_ACTION,
	KN_AX25_TIMER_REPLAY_EXPECT_PLAN,
	KN_AX25_TIMER_REPLAY_EXPECT_COUNTER,
	KN_AX25_TIMER_REPLAY_EXPECT_TX_WRITES,
	KN_AX25_TIMER_REPLAY_EXPECT_CONNECTION_COUNT,
	KN_AX25_TIMER_REPLAY_EXPECT_LAST_ERROR
};

enum kn_ax25_timer_replay_counter {
	KN_AX25_TIMER_REPLAY_COUNTER_TIMERS_STARTED = 0,
	KN_AX25_TIMER_REPLAY_COUNTER_TIMERS_STOPPED,
	KN_AX25_TIMER_REPLAY_COUNTER_TIMERS_EXPIRED,
	KN_AX25_TIMER_REPLAY_COUNTER_RETRIES_INCREMENTED,
	KN_AX25_TIMER_REPLAY_COUNTER_RETRIES_EXHAUSTED,
	KN_AX25_TIMER_REPLAY_COUNTER_FRAME_PLANS,
	KN_AX25_TIMER_REPLAY_COUNTER_EVENTS_ACCEPTED,
	KN_AX25_TIMER_REPLAY_COUNTER_PROTOCOL_ERRORS
};

struct kn_ax25_timer_replay_expect {
	enum kn_ax25_timer_replay_expect_type type;
	enum kn_ax25_connection_state state;
	enum kn_ax25_timer_kind timer_kind;
	enum kn_ax25_action_intent action;
	enum kn_ax25_frame_plan_type plan;
	enum kn_ax25_timer_replay_counter counter;
	char last_error[KN_AX25_TIMER_REPLAY_TEXT_MAX];
	uint64_t value;
	uint8_t bool_value;
};

struct kn_ax25_timer_replay_command {
	enum kn_ax25_timer_replay_command_type type;
	size_t line;
	uint64_t value;
	struct kn_ax25_params params;
	enum kn_ax25_connection_event event;
	uint8_t ns;
	uint8_t nr;
	uint8_t poll_final;
	size_t payload_len;
	struct kn_ax25_timer_replay_expect expect;
};

struct kn_ax25_timer_replay_script {
	char name[KN_AX25_TIMER_REPLAY_NAME_MAX];
	char node[KN_AX25_TIMER_REPLAY_NAME_MAX];
	char remote[KN_AX25_TIMER_REPLAY_NAME_MAX];
	char port[KN_AX25_TIMER_REPLAY_NAME_MAX];
	struct kn_ax25_timer_replay_command
	    commands[KN_AX25_TIMER_REPLAY_COMMAND_MAX];
	size_t command_count;
};

struct kn_ax25_timer_replay_error_info {
	enum kn_ax25_timer_replay_error error;
	size_t line;
	char message[KN_AX25_TIMER_REPLAY_TEXT_MAX];
};

void kn_ax25_timer_replay_error_clear(
	struct kn_ax25_timer_replay_error_info *);
const char *kn_ax25_timer_replay_error_name(
	enum kn_ax25_timer_replay_error);
void kn_ax25_timer_replay_script_clear(
	struct kn_ax25_timer_replay_script *);
enum kn_ax25_timer_replay_error kn_ax25_timer_replay_script_parse_file(
	const char *, struct kn_ax25_timer_replay_script *,
	struct kn_ax25_timer_replay_error_info *);

#endif
