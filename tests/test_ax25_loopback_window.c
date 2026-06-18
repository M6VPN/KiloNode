/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_loopback_window.c */

#include <sys/types.h>

#include "kilonode/ax25_loopback_window.h"

static int test_ack_wrap(void);
static int test_default_empty(void);
static int test_reject(void);
static int test_window_block(void);

int
main(void)
{
	if (test_default_empty() != 0)
		return 1;
	if (test_window_block() != 0)
		return 1;
	if (test_ack_wrap() != 0)
		return 1;
	if (test_reject() != 0)
		return 1;
	return 0;
}

static int
test_ack_wrap(void)
{
	struct kn_ax25_loopback_window window;
	size_t acked;

	kn_ax25_loopback_window_init(&window);
	if (kn_ax25_loopback_window_record(&window, 7, 0, 3, 0, 2) !=
	    KN_AX25_LOOPBACK_WINDOW_OK)
		return 1;
	if (kn_ax25_loopback_window_ack_rr(&window, 0, &acked) !=
	    KN_AX25_LOOPBACK_WINDOW_OK)
		return 1;
	if (acked != 1 || window.acked != 1)
		return 1;
	return kn_ax25_loopback_window_in_flight(&window) == 0 ? 0 : 1;
}

static int
test_default_empty(void)
{
	struct kn_ax25_loopback_window window;

	kn_ax25_loopback_window_init(&window);
	if (window.next_id != 1)
		return 1;
	return kn_ax25_loopback_window_in_flight(&window) == 0 ? 0 : 1;
}

static int
test_reject(void)
{
	struct kn_ax25_loopback_window window;
	size_t rejected;

	kn_ax25_loopback_window_init(&window);
	if (kn_ax25_loopback_window_record(&window, 2, 0, 3, 0, 2) !=
	    KN_AX25_LOOPBACK_WINDOW_OK)
		return 1;
	if (kn_ax25_loopback_window_reject(&window, 2, &rejected) !=
	    KN_AX25_LOOPBACK_WINDOW_OK)
		return 1;
	if (rejected != 1 || window.rejected != 1)
		return 1;
	return kn_ax25_loopback_window_in_flight(&window) == 0 ? 0 : 1;
}

static int
test_window_block(void)
{
	struct kn_ax25_loopback_window window;

	kn_ax25_loopback_window_init(&window);
	if (kn_ax25_loopback_window_record(&window, 0, 0, 3, 0, 1) !=
	    KN_AX25_LOOPBACK_WINDOW_OK)
		return 1;
	if (kn_ax25_loopback_window_record(&window, 1, 0, 3, 1, 1) !=
	    KN_AX25_LOOPBACK_WINDOW_ERR_BLOCKED)
		return 1;
	if (window.blocked != 1 || window.max_in_flight_seen != 1)
		return 1;
	return kn_ax25_loopback_window_in_flight(&window) == 1 ? 0 : 1;
}
