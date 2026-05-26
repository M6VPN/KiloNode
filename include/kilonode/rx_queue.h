/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/rx_queue.h */

#ifndef KILONODE_RX_QUEUE_H
#define KILONODE_RX_QUEUE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/callsign.h"
#include "kilonode/rx_event.h"

#define KN_RX_QUEUE_DEFAULT_MAX 256
#define KN_RX_QUEUE_MAX         256

enum kn_rx_queue_error {
	KN_RX_QUEUE_OK = 0,
	KN_RX_QUEUE_ERR_INVALID_ARGUMENT,
	KN_RX_QUEUE_ERR_INVALID_VALUE,
	KN_RX_QUEUE_ERR_NOT_FOUND
};

struct kn_rx_queue {
	struct kn_rx_event events[KN_RX_QUEUE_MAX];
	size_t max_events;
	size_t count;
	size_t start;
	uint64_t next_id;
	size_t preview_bytes;
	uint8_t enabled;
};

void kn_rx_queue_clear(struct kn_rx_queue *);
size_t kn_rx_queue_count(const struct kn_rx_queue *);
const struct kn_rx_event *kn_rx_queue_get(const struct kn_rx_queue *,
	uint64_t);
void kn_rx_queue_init(struct kn_rx_queue *, size_t, size_t, uint8_t);
enum kn_rx_queue_error kn_rx_queue_list(const struct kn_rx_queue *,
	const struct kn_rx_event **, size_t, size_t *);
enum kn_rx_queue_error kn_rx_queue_list_by_destination(
	const struct kn_rx_queue *, const struct kn_callsign *,
	const struct kn_rx_event **, size_t, size_t *);
enum kn_rx_queue_error kn_rx_queue_list_by_port(const struct kn_rx_queue *,
	const char *, const struct kn_rx_event **, size_t, size_t *);
enum kn_rx_queue_error kn_rx_queue_list_by_source(const struct kn_rx_queue *,
	const struct kn_callsign *, const struct kn_rx_event **, size_t,
	size_t *);
const struct kn_rx_event *kn_rx_queue_newest(const struct kn_rx_queue *);
enum kn_rx_queue_error kn_rx_queue_push(struct kn_rx_queue *,
	const struct kn_rx_event *);
uint64_t kn_rx_queue_reserve_id(struct kn_rx_queue *);

#endif
