/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/session_limits.h */

#ifndef KILONODE_SESSION_LIMITS_H
#define KILONODE_SESSION_LIMITS_H

#include <sys/types.h>

#include <stdint.h>

enum kn_session_limit_error {
	KN_SESSION_LIMIT_OK = 0,
	KN_SESSION_LIMIT_ERR_INVALID_ARGUMENT,
	KN_SESSION_LIMIT_ERR_RATE,
	KN_SESSION_LIMIT_ERR_IDLE,
	KN_SESSION_LIMIT_ERR_LENGTH
};

struct kn_session_rate {
	uint64_t window_start;
	size_t count;
};

uint8_t kn_session_idle_expired(uint64_t, uint64_t, uint64_t);
uint8_t kn_session_length_allowed(size_t, size_t);
enum kn_session_limit_error kn_session_rate_check(struct kn_session_rate *,
	size_t, uint64_t, uint64_t);
void kn_session_rate_init(struct kn_session_rate *);

#endif
