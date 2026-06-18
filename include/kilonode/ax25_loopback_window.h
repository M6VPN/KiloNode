/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_loopback_window.h */

#ifndef KILONODE_AX25_LOOPBACK_WINDOW_H
#define KILONODE_AX25_LOOPBACK_WINDOW_H

#include <sys/types.h>

#include <stdint.h>

#define KN_AX25_LOOPBACK_WINDOW_MAX 64

enum kn_ax25_loopback_window_error {
	KN_AX25_LOOPBACK_WINDOW_OK = 0,
	KN_AX25_LOOPBACK_WINDOW_ERR_INVALID_ARGUMENT,
	KN_AX25_LOOPBACK_WINDOW_ERR_INVALID_VALUE,
	KN_AX25_LOOPBACK_WINDOW_ERR_FULL,
	KN_AX25_LOOPBACK_WINDOW_ERR_BLOCKED
};

enum kn_ax25_loopback_window_status {
	KN_AX25_LOOPBACK_WINDOW_STATUS_IN_FLIGHT = 0,
	KN_AX25_LOOPBACK_WINDOW_STATUS_ACKED,
	KN_AX25_LOOPBACK_WINDOW_STATUS_REJECTED
};

struct kn_ax25_loopback_window_frame {
	uint64_t id;
	uint8_t ns;
	uint8_t nr;
	size_t payload_len;
	size_t segment_index;
	enum kn_ax25_loopback_window_status status;
};

struct kn_ax25_loopback_window {
	struct kn_ax25_loopback_window_frame
	    frames[KN_AX25_LOOPBACK_WINDOW_MAX];
	size_t count;
	uint64_t next_id;
	uint64_t recorded;
	uint64_t acked;
	uint64_t rejected;
	uint64_t blocked;
	uint64_t max_in_flight_seen;
};

enum kn_ax25_loopback_window_error kn_ax25_loopback_window_ack_rr(
	struct kn_ax25_loopback_window *, uint8_t, size_t *);
size_t kn_ax25_loopback_window_in_flight(
	const struct kn_ax25_loopback_window *);
void kn_ax25_loopback_window_init(struct kn_ax25_loopback_window *);
enum kn_ax25_loopback_window_error kn_ax25_loopback_window_record(
	struct kn_ax25_loopback_window *, uint8_t, uint8_t, size_t, size_t,
	size_t);
enum kn_ax25_loopback_window_error kn_ax25_loopback_window_reject(
	struct kn_ax25_loopback_window *, uint8_t, size_t *);
void kn_ax25_loopback_window_reset(struct kn_ax25_loopback_window *);

#endif
