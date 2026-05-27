/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_retry.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_retry.h"

static int test_exhausted(void);
static int test_format(void);
static int test_increment(void);
static int test_invalid_max(void);
static int test_reset(void);
static int test_under_limit(void);

int
main(void)
{
	if (test_reset() != 0)
		return 1;
	if (test_increment() != 0)
		return 1;
	if (test_under_limit() != 0)
		return 1;
	if (test_exhausted() != 0)
		return 1;
	if (test_invalid_max() != 0)
		return 1;
	if (test_format() != 0)
		return 1;

	return 0;
}

static int
test_exhausted(void)
{
	struct kn_ax25_retry retry;

	if (kn_ax25_retry_init(&retry, 2) != KN_AX25_RETRY_OK)
		return 1;
	if (kn_ax25_retry_increment(&retry) != KN_AX25_RETRY_OK)
		return 1;
	if (kn_ax25_retry_increment(&retry) != KN_AX25_RETRY_OK)
		return 1;

	return kn_ax25_retry_exhausted(&retry) != 0 ? 0 : 1;
}

static int
test_format(void)
{
	struct kn_ax25_retry retry;
	char buf[64];

	if (kn_ax25_retry_init(&retry, 3) != KN_AX25_RETRY_OK)
		return 1;
	if (kn_ax25_retry_increment(&retry) != KN_AX25_RETRY_OK)
		return 1;
	if (kn_ax25_retry_format(&retry, buf, sizeof(buf)) !=
	    KN_AX25_RETRY_OK)
		return 1;

	return strstr(buf, "count=1 max=3 exhausted=false") != NULL ? 0 : 1;
}

static int
test_increment(void)
{
	struct kn_ax25_retry retry;

	if (kn_ax25_retry_init(&retry, 2) != KN_AX25_RETRY_OK)
		return 1;
	if (kn_ax25_retry_increment(&retry) != KN_AX25_RETRY_OK)
		return 1;

	return retry.count == 1 ? 0 : 1;
}

static int
test_invalid_max(void)
{
	struct kn_ax25_retry retry;

	if (kn_ax25_retry_init(&retry, 0) != KN_AX25_RETRY_ERR_INVALID_VALUE)
		return 1;
	if (kn_ax25_retry_validate_max(KN_AX25_RETRY_MAX + 1) !=
	    KN_AX25_RETRY_ERR_INVALID_VALUE)
		return 1;

	return 0;
}

static int
test_reset(void)
{
	struct kn_ax25_retry retry;

	if (kn_ax25_retry_init(&retry, 3) != KN_AX25_RETRY_OK)
		return 1;
	if (kn_ax25_retry_increment(&retry) != KN_AX25_RETRY_OK)
		return 1;
	kn_ax25_retry_reset(&retry);

	return retry.count == 0 ? 0 : 1;
}

static int
test_under_limit(void)
{
	struct kn_ax25_retry retry;

	if (kn_ax25_retry_init(&retry, 1) != KN_AX25_RETRY_OK)
		return 1;
	if (kn_ax25_retry_under_limit(&retry) == 0)
		return 1;
	if (kn_ax25_retry_increment(&retry) != KN_AX25_RETRY_OK)
		return 1;

	return kn_ax25_retry_under_limit(&retry) == 0 ? 0 : 1;
}
