/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_session_limits.c */

#include <sys/types.h>

#include "kilonode/session_limits.h"

static int test_idle_timeout(void);
static int test_length_boundary(void);
static int test_rate_allows_initial(void);
static int test_rate_rejects_over_limit(void);
static int test_rate_resets_after_window(void);

int
main(void)
{
	if (test_rate_allows_initial() != 0)
		return 1;
	if (test_rate_rejects_over_limit() != 0)
		return 1;
	if (test_rate_resets_after_window() != 0)
		return 1;
	if (test_idle_timeout() != 0)
		return 1;
	if (test_length_boundary() != 0)
		return 1;
	return 0;
}

static int
test_idle_timeout(void)
{
	if (kn_session_idle_expired(100, 199, 100) != 0)
		return 1;
	return kn_session_idle_expired(100, 200, 100) != 0 ? 0 : 1;
}

static int
test_length_boundary(void)
{
	if (kn_session_length_allowed(10, 10) == 0)
		return 1;
	return kn_session_length_allowed(11, 10) == 0 ? 0 : 1;
}

static int
test_rate_allows_initial(void)
{
	struct kn_session_rate rate;

	kn_session_rate_init(&rate);
	if (kn_session_rate_check(&rate, 2, 60, 100) !=
	    KN_SESSION_LIMIT_OK)
		return 1;
	return kn_session_rate_check(&rate, 2, 60, 101) ==
	    KN_SESSION_LIMIT_OK ? 0 : 1;
}

static int
test_rate_rejects_over_limit(void)
{
	struct kn_session_rate rate;

	kn_session_rate_init(&rate);
	(void)kn_session_rate_check(&rate, 1, 60, 100);
	return kn_session_rate_check(&rate, 1, 60, 101) ==
	    KN_SESSION_LIMIT_ERR_RATE ? 0 : 1;
}

static int
test_rate_resets_after_window(void)
{
	struct kn_session_rate rate;

	kn_session_rate_init(&rate);
	(void)kn_session_rate_check(&rate, 1, 60, 100);
	if (kn_session_rate_check(&rate, 1, 60, 159) !=
	    KN_SESSION_LIMIT_ERR_RATE)
		return 1;
	return kn_session_rate_check(&rate, 1, 60, 160) ==
	    KN_SESSION_LIMIT_OK ? 0 : 1;
}
