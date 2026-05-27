/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/rf_command_queue.h */

#ifndef KILONODE_RF_COMMAND_QUEUE_H
#define KILONODE_RF_COMMAND_QUEUE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/config.h"
#include "kilonode/rf_command.h"

#define KN_RF_COMMAND_QUEUE_MAX KN_CONFIG_RF_COMMAND_EVENTS_MAX

enum kn_rf_command_queue_error {
	KN_RF_COMMAND_QUEUE_OK = 0,
	KN_RF_COMMAND_QUEUE_ERR_INVALID_ARGUMENT,
	KN_RF_COMMAND_QUEUE_ERR_NOT_FOUND
};

struct kn_rf_command_queue {
	struct kn_rf_command_event events[KN_RF_COMMAND_QUEUE_MAX];
	size_t max_events;
	size_t count;
	size_t head;
	uint64_t next_id;
};

void kn_rf_command_queue_clear(struct kn_rf_command_queue *);
size_t kn_rf_command_queue_count(const struct kn_rf_command_queue *);
const struct kn_rf_command_event *kn_rf_command_queue_get(
	const struct kn_rf_command_queue *, uint64_t);
enum kn_rf_command_queue_error kn_rf_command_queue_init(
	struct kn_rf_command_queue *, size_t);
enum kn_rf_command_queue_error kn_rf_command_queue_list(
	const struct kn_rf_command_queue *, const struct kn_rf_command_event **,
	size_t, size_t *);
enum kn_rf_command_queue_error kn_rf_command_queue_list_by_port(
	const struct kn_rf_command_queue *, const char *,
	const struct kn_rf_command_event **, size_t, size_t *);
enum kn_rf_command_queue_error kn_rf_command_queue_list_by_source(
	const struct kn_rf_command_queue *, const char *,
	const struct kn_rf_command_event **, size_t, size_t *);
enum kn_rf_command_queue_error kn_rf_command_queue_push(
	struct kn_rf_command_queue *, const struct kn_rf_command_event *);
uint64_t kn_rf_command_queue_reserve_id(struct kn_rf_command_queue *);

#endif
