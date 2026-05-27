/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_timer_queue.h */

#ifndef KILONODE_AX25_TIMER_QUEUE_H
#define KILONODE_AX25_TIMER_QUEUE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_timer.h"

#define KN_AX25_TIMER_QUEUE_MAX     96
#define KN_AX25_TIMER_EXPIRED_MAX   32

enum kn_ax25_timer_queue_error {
	KN_AX25_TIMER_QUEUE_OK = 0,
	KN_AX25_TIMER_QUEUE_ERR_INVALID_ARGUMENT,
	KN_AX25_TIMER_QUEUE_ERR_INVALID_VALUE,
	KN_AX25_TIMER_QUEUE_ERR_FULL,
	KN_AX25_TIMER_QUEUE_ERR_NOT_FOUND,
	KN_AX25_TIMER_QUEUE_ERR_BUFFER
};

struct kn_ax25_timer_expiry {
	uint32_t connection_id;
	enum kn_ax25_timer_kind kind;
	enum kn_ax25_connection_event event;
	uint32_t generation;
	uint8_t planned;
};

struct kn_ax25_timer_queue {
	struct kn_ax25_timer timers[KN_AX25_TIMER_QUEUE_MAX];
	size_t count;
};

size_t kn_ax25_timer_queue_count_running(
	const struct kn_ax25_timer_queue *);
enum kn_ax25_timer_queue_error kn_ax25_timer_queue_collect_expired(
	struct kn_ax25_timer_queue *, uint64_t, struct kn_ax25_timer_expiry *,
	size_t, size_t *);
enum kn_ax25_timer_queue_error kn_ax25_timer_queue_find(
	const struct kn_ax25_timer_queue *, uint32_t,
	enum kn_ax25_timer_kind, size_t *);
enum kn_ax25_timer_queue_error kn_ax25_timer_queue_format(
	const struct kn_ax25_timer_queue *, char *, size_t);
void kn_ax25_timer_queue_init(struct kn_ax25_timer_queue *);
enum kn_ax25_timer_queue_error kn_ax25_timer_queue_next_expiry(
	const struct kn_ax25_timer_queue *, uint64_t *);
void kn_ax25_timer_queue_reset(struct kn_ax25_timer_queue *);
enum kn_ax25_timer_queue_error kn_ax25_timer_queue_start(
	struct kn_ax25_timer_queue *, uint32_t, enum kn_ax25_timer_kind,
	uint64_t, uint32_t);
enum kn_ax25_timer_queue_error kn_ax25_timer_queue_stop(
	struct kn_ax25_timer_queue *, uint32_t, enum kn_ax25_timer_kind);

#endif
