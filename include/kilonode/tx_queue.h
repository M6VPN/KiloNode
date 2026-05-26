/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/tx_queue.h */

#ifndef KILONODE_TX_QUEUE_H
#define KILONODE_TX_QUEUE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/tx_frame.h"
#include "kilonode/tx_policy.h"

#define KN_TX_QUEUE_MAX KN_TX_POLICY_MAX_QUEUED_MAX

enum kn_tx_queue_error {
	KN_TX_QUEUE_OK = 0,
	KN_TX_QUEUE_ERR_INVALID_ARGUMENT,
	KN_TX_QUEUE_ERR_FULL,
	KN_TX_QUEUE_ERR_NOT_FOUND,
	KN_TX_QUEUE_ERR_POLICY
};

struct kn_tx_queue {
	struct kn_tx_frame frames[KN_TX_QUEUE_MAX];
	size_t max_frames;
	size_t count;
	uint64_t next_id;
	struct kn_tx_policy policy;
};

void kn_tx_queue_clear(struct kn_tx_queue *);
size_t kn_tx_queue_count(const struct kn_tx_queue *);
enum kn_tx_queue_error kn_tx_queue_enqueue(struct kn_tx_queue *,
	const struct kn_tx_frame *);
const struct kn_tx_frame *kn_tx_queue_get(const struct kn_tx_queue *,
	uint64_t);
enum kn_tx_queue_error kn_tx_queue_init(struct kn_tx_queue *,
	const struct kn_tx_policy *);
enum kn_tx_queue_error kn_tx_queue_list(const struct kn_tx_queue *,
	const struct kn_tx_frame **, size_t, size_t *);
enum kn_tx_queue_error kn_tx_queue_list_by_port(const struct kn_tx_queue *,
	const char *, const struct kn_tx_frame **, size_t, size_t *);
enum kn_tx_queue_error kn_tx_queue_mark_dropped(struct kn_tx_queue *,
	uint64_t);
enum kn_tx_queue_error kn_tx_queue_mark_failed(struct kn_tx_queue *,
	uint64_t, int);
enum kn_tx_queue_error kn_tx_queue_mark_sent(struct kn_tx_queue *,
	uint64_t);
struct kn_tx_frame *kn_tx_queue_mutable_get(struct kn_tx_queue *, uint64_t);
struct kn_tx_frame *kn_tx_queue_pop_next_for_port(struct kn_tx_queue *,
	const char *);
uint64_t kn_tx_queue_reserve_id(struct kn_tx_queue *);

#endif
