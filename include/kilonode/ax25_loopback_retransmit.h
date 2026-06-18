/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_loopback_retransmit.h */

#ifndef KILONODE_AX25_LOOPBACK_RETRANSMIT_H
#define KILONODE_AX25_LOOPBACK_RETRANSMIT_H

#include <sys/types.h>

#include <stdint.h>

#define KN_AX25_LOOPBACK_RETRANSMIT_MAX     64
#define KN_AX25_LOOPBACK_RETRANSMIT_RAW_MAX 256

enum kn_ax25_loopback_retransmit_error {
	KN_AX25_LOOPBACK_RETRANSMIT_OK = 0,
	KN_AX25_LOOPBACK_RETRANSMIT_ERR_INVALID_ARGUMENT,
	KN_AX25_LOOPBACK_RETRANSMIT_ERR_INVALID_VALUE,
	KN_AX25_LOOPBACK_RETRANSMIT_ERR_FULL,
	KN_AX25_LOOPBACK_RETRANSMIT_ERR_NOT_FOUND,
	KN_AX25_LOOPBACK_RETRANSMIT_ERR_BUFFER
};

enum kn_ax25_loopback_retransmit_status {
	KN_AX25_LOOPBACK_RETRANSMIT_STATUS_IN_FLIGHT = 0,
	KN_AX25_LOOPBACK_RETRANSMIT_STATUS_ACKED,
	KN_AX25_LOOPBACK_RETRANSMIT_STATUS_RETRY_NEEDED,
	KN_AX25_LOOPBACK_RETRANSMIT_STATUS_REPLAYED,
	KN_AX25_LOOPBACK_RETRANSMIT_STATUS_DROPPED
};

struct kn_ax25_loopback_retransmit_frame {
	uint64_t id;
	uint8_t ns;
	uint8_t nr;
	size_t payload_len;
	size_t segment_index;
	size_t raw_len;
	uint8_t raw[KN_AX25_LOOPBACK_RETRANSMIT_RAW_MAX];
	enum kn_ax25_loopback_retransmit_status status;
	uint64_t replay_count;
};

struct kn_ax25_loopback_retransmit_buffer {
	struct kn_ax25_loopback_retransmit_frame
	    frames[KN_AX25_LOOPBACK_RETRANSMIT_MAX];
	size_t count;
	uint64_t next_id;
	uint64_t recorded;
	uint64_t acked;
	uint64_t retry_marked;
	uint64_t replayed;
	uint64_t dropped;
	uint64_t full;
};

enum kn_ax25_loopback_retransmit_error
kn_ax25_loopback_retransmit_ack_rr(
	struct kn_ax25_loopback_retransmit_buffer *, uint8_t, size_t *);
size_t kn_ax25_loopback_retransmit_count_in_flight(
	const struct kn_ax25_loopback_retransmit_buffer *);
size_t kn_ax25_loopback_retransmit_count_retry_needed(
	const struct kn_ax25_loopback_retransmit_buffer *);
void kn_ax25_loopback_retransmit_init(
	struct kn_ax25_loopback_retransmit_buffer *);
enum kn_ax25_loopback_retransmit_error
kn_ax25_loopback_retransmit_mark_rej(
	struct kn_ax25_loopback_retransmit_buffer *, uint8_t, size_t *);
enum kn_ax25_loopback_retransmit_error kn_ax25_loopback_retransmit_next(
	struct kn_ax25_loopback_retransmit_buffer *, uint8_t *, size_t,
	size_t *);
enum kn_ax25_loopback_retransmit_error kn_ax25_loopback_retransmit_record(
	struct kn_ax25_loopback_retransmit_buffer *, uint8_t, uint8_t, size_t,
	size_t, const uint8_t *, size_t, uint64_t *);
void kn_ax25_loopback_retransmit_reset(
	struct kn_ax25_loopback_retransmit_buffer *);

#endif
