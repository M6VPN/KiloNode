/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_timer.h */

#ifndef KILONODE_AX25_TIMER_H
#define KILONODE_AX25_TIMER_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_connection.h"

#define KN_AX25_TIMER_MAX_DURATION_MS 3600000U

enum kn_ax25_timer_error {
	KN_AX25_TIMER_OK = 0,
	KN_AX25_TIMER_ERR_INVALID_ARGUMENT,
	KN_AX25_TIMER_ERR_INVALID_VALUE,
	KN_AX25_TIMER_ERR_BUFFER
};

enum kn_ax25_timer_kind {
	KN_AX25_TIMER_NONE = 0,
	KN_AX25_TIMER_T1,
	KN_AX25_TIMER_T2,
	KN_AX25_TIMER_T3,
	KN_AX25_TIMER_IDLE_SESSION,
	KN_AX25_TIMER_BUSY_CONDITION
};

struct kn_ax25_timer {
	enum kn_ax25_timer_kind kind;
	uint8_t running;
	uint32_t connection_id;
	uint64_t started_at_ms;
	uint64_t expires_at_ms;
	uint32_t duration_ms;
	uint32_t generation;
	uint32_t expiry_count;
};

void kn_ax25_timer_init(struct kn_ax25_timer *);
enum kn_ax25_timer_error kn_ax25_timer_format(
	const struct kn_ax25_timer *, char *, size_t);
enum kn_ax25_connection_event kn_ax25_timer_kind_event(
	enum kn_ax25_timer_kind);
const char *kn_ax25_timer_kind_name(enum kn_ax25_timer_kind);
uint64_t kn_ax25_timer_remaining_ms(const struct kn_ax25_timer *, uint64_t);
enum kn_ax25_timer_error kn_ax25_timer_restart(
	struct kn_ax25_timer *, uint64_t);
enum kn_ax25_timer_error kn_ax25_timer_start(struct kn_ax25_timer *,
	enum kn_ax25_timer_kind, uint32_t, uint64_t, uint32_t);
void kn_ax25_timer_stop(struct kn_ax25_timer *);
uint8_t kn_ax25_timer_is_expired(const struct kn_ax25_timer *, uint64_t);
uint8_t kn_ax25_timer_is_running(const struct kn_ax25_timer *);

#endif
