/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_loopback_endpoint.c */

#include <sys/types.h>

#include <stddef.h>

#include "kilonode/ax25_loopback_endpoint.h"

static int test_connect_disconnect_actions(void);
static int test_invalid_callsign(void);

int
main(void)
{
	if (test_invalid_callsign() != 0)
		return 1;
	if (test_connect_disconnect_actions() != 0)
		return 1;
	return 0;
}

static int
test_connect_disconnect_actions(void)
{
	struct kn_ax25_loopback_endpoint endpoint;
	enum kn_ax25_connection_state state;

	if (kn_ax25_loopback_endpoint_init(&endpoint, "A", "M6VPN-1",
	    "N0CALL", "kiss0", NULL) != KN_AX25_LOOPBACK_ENDPOINT_OK)
		return 1;
	if (kn_ax25_loopback_endpoint_local_connect(&endpoint) !=
	    KN_AX25_LOOPBACK_ENDPOINT_OK)
		return 1;
	if (kn_ax25_prepared_queue_count(&endpoint.prepared) != 1)
		return 1;
	if (kn_ax25_loopback_endpoint_state(&endpoint, &state) !=
	    KN_AX25_LOOPBACK_ENDPOINT_OK)
		return 1;
	if (state != KN_AX25_CONNECTION_AWAITING_CONNECTION)
		return 1;
	if (endpoint.tx_queue_writes != 0 || endpoint.dispatch_calls != 0)
		return 1;
	return 0;
}

static int
test_invalid_callsign(void)
{
	struct kn_ax25_loopback_endpoint endpoint;

	return kn_ax25_loopback_endpoint_init(&endpoint, "bad", "BAD!CALL",
	    "N0CALL", "kiss0", NULL) ==
	    KN_AX25_LOOPBACK_ENDPOINT_ERR_INVALID_VALUE ? 0 : 1;
}
