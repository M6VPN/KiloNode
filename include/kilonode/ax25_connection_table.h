/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_connection_table.h */

#ifndef KILONODE_AX25_CONNECTION_TABLE_H
#define KILONODE_AX25_CONNECTION_TABLE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_action_mapper.h"
#include "kilonode/ax25_connection_event.h"

#define KN_AX25_CONNECTION_TABLE_DEFAULT_MAX 32
#define KN_AX25_CONNECTION_TABLE_MAX         32

enum kn_ax25_connection_table_error {
	KN_AX25_CONNECTION_TABLE_OK = 0,
	KN_AX25_CONNECTION_TABLE_ERR_INVALID_ARGUMENT,
	KN_AX25_CONNECTION_TABLE_ERR_INVALID_VALUE,
	KN_AX25_CONNECTION_TABLE_ERR_FULL,
	KN_AX25_CONNECTION_TABLE_ERR_NOT_FOUND,
	KN_AX25_CONNECTION_TABLE_ERR_STATE,
	KN_AX25_CONNECTION_TABLE_ERR_MAPPER
};

struct kn_ax25_connection_counters {
	uint64_t rx_sabm_sabme;
	uint64_t rx_ua;
	uint64_t rx_disc;
	uint64_t rx_dm;
	uint64_t rx_i;
	uint64_t rx_s_frames;
	uint64_t protocol_errors;
};

struct kn_ax25_connection_record {
	uint8_t in_use;
	struct kn_ax25_connection_key key;
	struct kn_ax25_connection connection;
	struct kn_ax25_params params;
	uint64_t created;
	uint64_t last_event;
	enum kn_ax25_connection_event last_event_kind;
	struct kn_ax25_control_info last_control;
	struct kn_ax25_action_list last_actions;
	struct kn_ax25_frame_plan_list last_plans;
	struct kn_ax25_connection_counters counters;
	enum kn_ax25_state_error last_state_status;
};

struct kn_ax25_connection_table {
	struct kn_ax25_connection_record records[KN_AX25_CONNECTION_TABLE_MAX];
	size_t count;
	size_t max_connections;
	struct kn_ax25_params params;
};

struct kn_ax25_connection_table_result {
	size_t record_index;
	uint8_t created;
	enum kn_ax25_state_error state_status;
	enum kn_ax25_action_mapper_error mapper_status;
	struct kn_ax25_action_list actions;
	struct kn_ax25_frame_plan_list plans;
};

void kn_ax25_connection_table_free(struct kn_ax25_connection_table *);
enum kn_ax25_connection_table_error kn_ax25_connection_table_find(
	const struct kn_ax25_connection_table *,
	const struct kn_ax25_connection_key *, size_t *);
struct kn_ax25_connection_record *kn_ax25_connection_table_get(
	struct kn_ax25_connection_table *, size_t);
enum kn_ax25_connection_table_error kn_ax25_connection_table_get_or_create(
	struct kn_ax25_connection_table *, const struct kn_ax25_connection_key *,
	uint64_t, size_t *, uint8_t *);
void kn_ax25_connection_table_init(struct kn_ax25_connection_table *);
enum kn_ax25_connection_table_error kn_ax25_connection_table_list(
	const struct kn_ax25_connection_table *,
	const struct kn_ax25_connection_record **, size_t, size_t *);
enum kn_ax25_connection_table_error kn_ax25_connection_table_process(
	struct kn_ax25_connection_table *,
	const struct kn_ax25_connection_event_record *,
	struct kn_ax25_connection_table_result *);
enum kn_ax25_connection_table_error kn_ax25_connection_table_remove(
	struct kn_ax25_connection_table *, const struct kn_ax25_connection_key *);
void kn_ax25_connection_table_reset(struct kn_ax25_connection_table *);
enum kn_ax25_connection_table_error kn_ax25_connection_table_set_params(
	struct kn_ax25_connection_table *, const struct kn_ax25_params *);
void kn_ax25_connection_table_result_clear(
	struct kn_ax25_connection_table_result *);
size_t kn_ax25_connection_table_count(
	const struct kn_ax25_connection_table *);

#endif
