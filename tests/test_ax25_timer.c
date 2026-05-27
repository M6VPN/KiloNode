/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_timer.c */

#include <sys/types.h>

#include <stdint.h>
#include <string.h>

#include "kilonode/ax25_timer.h"

static int test_expiry(void);
static int test_format(void);
static int test_init_stopped(void);
static int test_invalid_duration(void);
static int test_remaining(void);
static int test_restart(void);
static int test_start_stop(void);

int
main(void)
{
	if (test_init_stopped() != 0)
		return 1;
	if (test_start_stop() != 0)
		return 1;
	if (test_restart() != 0)
		return 1;
	if (test_expiry() != 0)
		return 1;
	if (test_remaining() != 0)
		return 1;
	if (test_invalid_duration() != 0)
		return 1;
	if (test_format() != 0)
		return 1;

	return 0;
}

static int
test_expiry(void)
{
	struct kn_ax25_timer timer;

	kn_ax25_timer_init(&timer);
	if (kn_ax25_timer_start(&timer, KN_AX25_TIMER_T1, 7, 1000,
	    3000) != KN_AX25_TIMER_OK)
		return 1;
	if (kn_ax25_timer_is_expired(&timer, 3999) != 0)
		return 1;
	if (kn_ax25_timer_is_expired(&timer, 4000) == 0)
		return 1;

	return 0;
}

static int
test_format(void)
{
	struct kn_ax25_timer timer;
	char buf[160];

	kn_ax25_timer_init(&timer);
	if (kn_ax25_timer_start(&timer, KN_AX25_TIMER_T3, 2, 10,
	    500) != KN_AX25_TIMER_OK)
		return 1;
	if (kn_ax25_timer_format(&timer, buf, sizeof(buf)) !=
	    KN_AX25_TIMER_OK)
		return 1;

	return strstr(buf, "kind=t3 connection=2 running=true") != NULL ?
	    0 : 1;
}

static int
test_init_stopped(void)
{
	struct kn_ax25_timer timer;

	kn_ax25_timer_init(&timer);
	if (kn_ax25_timer_is_running(&timer) != 0)
		return 1;
	if (timer.kind != KN_AX25_TIMER_NONE)
		return 1;

	return 0;
}

static int
test_invalid_duration(void)
{
	struct kn_ax25_timer timer;

	kn_ax25_timer_init(&timer);
	if (kn_ax25_timer_start(&timer, KN_AX25_TIMER_T1, 1, 0, 0) !=
	    KN_AX25_TIMER_ERR_INVALID_VALUE)
		return 1;
	if (kn_ax25_timer_start(&timer, KN_AX25_TIMER_NONE, 1, 0,
	    1) != KN_AX25_TIMER_ERR_INVALID_VALUE)
		return 1;

	return 0;
}

static int
test_remaining(void)
{
	struct kn_ax25_timer timer;

	kn_ax25_timer_init(&timer);
	if (kn_ax25_timer_start(&timer, KN_AX25_TIMER_T2, 1, 100,
	    50) != KN_AX25_TIMER_OK)
		return 1;
	if (kn_ax25_timer_remaining_ms(&timer, 125) != 25)
		return 1;
	if (kn_ax25_timer_remaining_ms(&timer, 150) != 0)
		return 1;

	return 0;
}

static int
test_restart(void)
{
	struct kn_ax25_timer timer;

	kn_ax25_timer_init(&timer);
	if (kn_ax25_timer_start(&timer, KN_AX25_TIMER_T1, 1, 10,
	    20) != KN_AX25_TIMER_OK)
		return 1;
	if (kn_ax25_timer_restart(&timer, 50) != KN_AX25_TIMER_OK)
		return 1;

	return timer.expires_at_ms == 70 && timer.generation == 2 ? 0 : 1;
}

static int
test_start_stop(void)
{
	struct kn_ax25_timer timer;

	kn_ax25_timer_init(&timer);
	if (kn_ax25_timer_start(&timer, KN_AX25_TIMER_T1, 3, 1000,
	    3000) != KN_AX25_TIMER_OK)
		return 1;
	if (kn_ax25_timer_is_running(&timer) == 0)
		return 1;
	if (timer.expires_at_ms != 4000)
		return 1;
	kn_ax25_timer_stop(&timer);

	return kn_ax25_timer_is_running(&timer) == 0 ? 0 : 1;
}
