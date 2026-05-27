/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_prepared_queue.h */

#ifndef KILONODE_AX25_PREPARED_QUEUE_H
#define KILONODE_AX25_PREPARED_QUEUE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_prepared_policy.h"

enum kn_ax25_prepared_queue_error {
	KN_AX25_PREPARED_QUEUE_OK = 0,
	KN_AX25_PREPARED_QUEUE_ERR_INVALID_ARGUMENT,
	KN_AX25_PREPARED_QUEUE_ERR_INVALID_VALUE,
	KN_AX25_PREPARED_QUEUE_ERR_FULL,
	KN_AX25_PREPARED_QUEUE_ERR_NOT_FOUND,
	KN_AX25_PREPARED_QUEUE_ERR_BUFFER
};

struct kn_ax25_prepared_queue {
	struct kn_ax25_prepared_frame frames[KN_AX25_PREPARED_QUEUE_MAX];
	size_t count;
	size_t max_frames;
	uint64_t next_id;
};

size_t kn_ax25_prepared_queue_count(
	const struct kn_ax25_prepared_queue *);
void kn_ax25_prepared_queue_free(struct kn_ax25_prepared_queue *);
const struct kn_ax25_prepared_frame *kn_ax25_prepared_queue_get(
	const struct kn_ax25_prepared_queue *, uint64_t);
void kn_ax25_prepared_queue_init(struct kn_ax25_prepared_queue *);
enum kn_ax25_prepared_queue_error kn_ax25_prepared_queue_list(
	const struct kn_ax25_prepared_queue *,
	const struct kn_ax25_prepared_frame **, size_t, size_t *);
enum kn_ax25_prepared_queue_error kn_ax25_prepared_queue_list_by_connection(
	const struct kn_ax25_prepared_queue *, uint32_t,
	const struct kn_ax25_prepared_frame **, size_t, size_t *);
enum kn_ax25_prepared_queue_error kn_ax25_prepared_queue_list_by_port(
	const struct kn_ax25_prepared_queue *, const char *,
	const struct kn_ax25_prepared_frame **, size_t, size_t *);
enum kn_ax25_prepared_queue_error kn_ax25_prepared_queue_push(
	struct kn_ax25_prepared_queue *,
	const struct kn_ax25_prepared_frame *, uint64_t *);
void kn_ax25_prepared_queue_reset(struct kn_ax25_prepared_queue *);
enum kn_ax25_prepared_queue_error kn_ax25_prepared_queue_set_max(
	struct kn_ax25_prepared_queue *, size_t);
enum kn_ax25_prepared_queue_error kn_ax25_prepared_queue_format(
	const struct kn_ax25_prepared_queue *, char *, size_t);

#endif
