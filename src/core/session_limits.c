/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/core/session_limits.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/session_limits.h"

uint8_t
kn_session_idle_expired(uint64_t last_activity, uint64_t now,
	uint64_t timeout)
{
	if (timeout == 0 || last_activity == 0)
		return 0;
	if (now < last_activity)
		return 0;
	return now - last_activity >= timeout ? 1 : 0;
}

uint8_t
kn_session_length_allowed(size_t len, size_t max_len)
{
	if (max_len == 0)
		return 0;
	return len <= max_len ? 1 : 0;
}

enum kn_session_limit_error
kn_session_rate_check(struct kn_session_rate *rate, size_t max_lines,
	uint64_t window_seconds, uint64_t now)
{
	if (rate == NULL || max_lines == 0 || window_seconds == 0)
		return KN_SESSION_LIMIT_ERR_INVALID_ARGUMENT;
	if (rate->window_start == 0 || now < rate->window_start ||
	    now - rate->window_start >= window_seconds) {
		rate->window_start = now == 0 ? 1 : now;
		rate->count = 0;
	}
	if (rate->count >= max_lines)
		return KN_SESSION_LIMIT_ERR_RATE;
	rate->count++;
	return KN_SESSION_LIMIT_OK;
}

void
kn_session_rate_init(struct kn_session_rate *rate)
{
	if (rate == NULL)
		return;
	memset(rate, 0, sizeof(*rate));
}
