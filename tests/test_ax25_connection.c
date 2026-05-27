/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_connection.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_connection.h"
#include "kilonode/ax25_state.h"

static int test_control_to_event(void);
static int test_formatting(void);
static int test_init_disabled(void);
static int test_invalid_rejected(void);
static int test_no_tx_output(void);
static int test_reset(void);
static int test_state_step_helper(void);

int
main(void)
{
	if (test_init_disabled() != 0)
		return 1;
	if (test_reset() != 0)
		return 1;
	if (test_control_to_event() != 0)
		return 1;
	if (test_no_tx_output() != 0)
		return 1;
	if (test_formatting() != 0)
		return 1;
	if (test_invalid_rejected() != 0)
		return 1;
	if (test_state_step_helper() != 0)
		return 1;

	return 0;
}

static int
test_control_to_event(void)
{
	if (kn_ax25_connection_event_from_control(0x2f) !=
	    KN_AX25_CONNECTION_EVENT_RX_SABM)
		return 1;
	if (kn_ax25_connection_event_from_control(0x6f) !=
	    KN_AX25_CONNECTION_EVENT_RX_SABME)
		return 1;
	if (kn_ax25_connection_event_from_control(0x63) !=
	    KN_AX25_CONNECTION_EVENT_RX_UA)
		return 1;
	if (kn_ax25_connection_event_from_control(0x43) !=
	    KN_AX25_CONNECTION_EVENT_RX_DISC)
		return 1;
	if (kn_ax25_connection_event_from_control(0x0f) !=
	    KN_AX25_CONNECTION_EVENT_RX_DM)
		return 1;
	if (kn_ax25_connection_event_from_control(0x00) !=
	    KN_AX25_CONNECTION_EVENT_RX_I)
		return 1;
	if (kn_ax25_connection_event_from_control(0x01) !=
	    KN_AX25_CONNECTION_EVENT_RX_RR)
		return 1;
	if (kn_ax25_connection_event_from_control(0x05) !=
	    KN_AX25_CONNECTION_EVENT_RX_RNR)
		return 1;
	if (kn_ax25_connection_event_from_control(0x09) !=
	    KN_AX25_CONNECTION_EVENT_RX_REJ)
		return 1;
	if (kn_ax25_connection_event_from_control(0x87) !=
	    KN_AX25_CONNECTION_EVENT_RX_FRMR)
		return 1;

	return kn_ax25_connection_event_from_control(0x03) ==
	    KN_AX25_CONNECTION_EVENT_NONE ? 0 : 1;
}

static int
test_formatting(void)
{
	struct kn_ax25_connection connection;
	char out[160];

	kn_ax25_connection_init(&connection, NULL);
	if (kn_ax25_connection_format(&connection, out, sizeof(out)) !=
	    KN_AX25_CONNECTION_OK)
		return 1;
	if (strstr(out, "state=disabled") == NULL)
		return 1;
	if (strcmp(kn_ax25_connection_state_name(
	    KN_AX25_CONNECTION_TIMER_RECOVERY), "timer-recovery") != 0)
		return 1;
	if (strcmp(kn_ax25_connection_event_name(
	    KN_AX25_CONNECTION_EVENT_TIMEOUT_T1), "timeout-t1") != 0)
		return 1;

	return 0;
}

static int
test_init_disabled(void)
{
	struct kn_ax25_connection connection;

	kn_ax25_connection_init(&connection, NULL);
	if (connection.state != KN_AX25_CONNECTION_DISABLED)
		return 1;
	if (connection.tx_output_allowed != 0)
		return 1;

	return kn_ax25_connection_validate(&connection) ==
	    KN_AX25_CONNECTION_OK ? 0 : 1;
}

static int
test_invalid_rejected(void)
{
	struct kn_ax25_connection connection;

	kn_ax25_connection_init(&connection, NULL);
	connection.tx_output_allowed = 1;

	return kn_ax25_connection_validate(&connection) ==
	    KN_AX25_CONNECTION_ERR_INVALID_VALUE ? 0 : 1;
}

static int
test_no_tx_output(void)
{
	struct kn_ax25_connection connection;

	kn_ax25_connection_init(&connection, NULL);
	return connection.tx_output_allowed == 0 ? 0 : 1;
}

static int
test_reset(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_params params;

	kn_ax25_params_default(&params);
	params.allow_connected_mode = 1;
	kn_ax25_connection_init(&connection, &params);
	if (connection.state != KN_AX25_CONNECTION_DISCONNECTED)
		return 1;

	connection.state = KN_AX25_CONNECTION_CONNECTED;
	connection.send_state = 4;
	kn_ax25_connection_reset(&connection);

	if (connection.state != KN_AX25_CONNECTION_DISCONNECTED)
		return 1;

	return connection.send_state == 0 ? 0 : 1;
}

static int
test_state_step_helper(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_params params;
	struct kn_ax25_state_input input;
	struct kn_ax25_state_result result;

	kn_ax25_params_default(&params);
	params.allow_connected_mode = 1;
	kn_ax25_connection_init(&connection, &params);
	kn_ax25_state_input_clear(&input);
	input.event = KN_AX25_CONNECTION_EVENT_LOCAL_CONNECT_REQUEST;

	if (kn_ax25_state_step(&connection, &input, &result) !=
	    KN_AX25_STATE_OK)
		return 1;
	if (connection.state != KN_AX25_CONNECTION_AWAITING_CONNECTION)
		return 1;

	return result.actions.count == 2 ? 0 : 1;
}
