/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/tests/test_ax25_state.c */

#include <sys/types.h>

#include <stddef.h>
#include <stdint.h>

#include "kilonode/ax25_state.h"

static int action_has(const struct kn_ax25_state_result *,
	enum kn_ax25_action_intent);
static void connection_enabled(struct kn_ax25_connection *);
static enum kn_ax25_state_error step_event(struct kn_ax25_connection *,
	enum kn_ax25_connection_event, uint8_t, uint8_t,
	struct kn_ax25_state_result *);
static int test_awaiting_connection_retry(void);
static int test_awaiting_connection_retry_exhausted(void);
static int test_awaiting_connection_rx_dm(void);
static int test_awaiting_connection_rx_ua(void);
static int test_awaiting_release_retry(void);
static int test_awaiting_release_retry_exhausted(void);
static int test_awaiting_release_rx_ua(void);
static int test_connected_invalid_i(void);
static int test_connected_invalid_i_ack(void);
static int test_connected_local_disconnect(void);
static int test_connected_rx_disc(void);
static int test_connected_rx_dm(void);
static int test_connected_rx_i(void);
static int test_connected_rx_rej(void);
static int test_connected_rx_rnr(void);
static int test_connected_rx_rr(void);
static int test_disabled_rejects(void);
static int test_disconnected_local_connect(void);
static int test_disconnected_rx_sabm(void);
static int test_disconnected_rx_sabme(void);
static int test_no_tx_queue_write(void);

int
main(void)
{
	if (test_disabled_rejects() != 0)
		return 1;
	if (test_disconnected_local_connect() != 0)
		return 1;
	if (test_disconnected_rx_sabm() != 0)
		return 1;
	if (test_disconnected_rx_sabme() != 0)
		return 1;
	if (test_awaiting_connection_rx_ua() != 0)
		return 1;
	if (test_awaiting_connection_rx_dm() != 0)
		return 1;
	if (test_awaiting_connection_retry() != 0)
		return 1;
	if (test_awaiting_connection_retry_exhausted() != 0)
		return 1;
	if (test_connected_local_disconnect() != 0)
		return 1;
	if (test_connected_rx_disc() != 0)
		return 1;
	if (test_connected_rx_dm() != 0)
		return 1;
	if (test_connected_rx_i() != 0)
		return 1;
	if (test_connected_invalid_i() != 0)
		return 1;
	if (test_connected_invalid_i_ack() != 0)
		return 1;
	if (test_connected_rx_rr() != 0)
		return 1;
	if (test_connected_rx_rnr() != 0)
		return 1;
	if (test_connected_rx_rej() != 0)
		return 1;
	if (test_awaiting_release_rx_ua() != 0)
		return 1;
	if (test_awaiting_release_retry() != 0)
		return 1;
	if (test_awaiting_release_retry_exhausted() != 0)
		return 1;
	if (test_no_tx_queue_write() != 0)
		return 1;

	return 0;
}

static int
action_has(const struct kn_ax25_state_result *result,
	enum kn_ax25_action_intent intent)
{
	size_t i;

	for (i = 0; i < result->actions.count; i++) {
		if (result->actions.actions[i].intent == intent)
			return 1;
	}

	return 0;
}

static void
connection_enabled(struct kn_ax25_connection *connection)
{
	struct kn_ax25_params params;

	kn_ax25_params_default(&params);
	params.allow_connected_mode = 1;
	params.n2_retry_count = 3;
	kn_ax25_connection_init(connection, &params);
}

static enum kn_ax25_state_error
step_event(struct kn_ax25_connection *connection,
	enum kn_ax25_connection_event event, uint8_t ns, uint8_t nr,
	struct kn_ax25_state_result *result)
{
	struct kn_ax25_state_input input;

	kn_ax25_state_input_clear(&input);
	input.event = event;
	input.ns = ns;
	input.nr = nr;
	return kn_ax25_state_step(connection, &input, result);
}

static int
test_awaiting_connection_retry(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_AWAITING_CONNECTION;
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_TIMEOUT_T1,
	    0, 0, &result) != KN_AX25_STATE_OK)
		return 1;
	if (connection.state != KN_AX25_CONNECTION_AWAITING_CONNECTION)
		return 1;
	if (connection.retry_count != 1)
		return 1;

	return action_has(&result, KN_AX25_ACTION_SEND_SABM) &&
	    action_has(&result, KN_AX25_ACTION_INCREMENT_RETRY_COUNT) ? 0 : 1;
}

static int
test_awaiting_connection_retry_exhausted(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_AWAITING_CONNECTION;
	connection.retry_count = 2;
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_TIMEOUT_T1,
	    0, 0, &result) != KN_AX25_STATE_ERR_INVALID_EVENT)
		return 1;
	if (connection.state != KN_AX25_CONNECTION_DISCONNECTED)
		return 1;

	return action_has(&result, KN_AX25_ACTION_PROTOCOL_ERROR) &&
	    action_has(&result, KN_AX25_ACTION_STOP_T1) ? 0 : 1;
}

static int
test_awaiting_connection_rx_dm(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_AWAITING_CONNECTION;
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_RX_DM, 0, 0,
	    &result) != KN_AX25_STATE_OK)
		return 1;

	return connection.state == KN_AX25_CONNECTION_DISCONNECTED &&
	    action_has(&result, KN_AX25_ACTION_STOP_T1) ? 0 : 1;
}

static int
test_awaiting_connection_rx_ua(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_AWAITING_CONNECTION;
	connection.retry_count = 1;
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_RX_UA, 0, 0,
	    &result) != KN_AX25_STATE_OK)
		return 1;
	if (connection.state != KN_AX25_CONNECTION_CONNECTED)
		return 1;
	if (connection.retry_count != 0)
		return 1;

	return action_has(&result, KN_AX25_ACTION_ENTER_CONNECTED) &&
	    action_has(&result, KN_AX25_ACTION_START_T3) ? 0 : 1;
}

static int
test_awaiting_release_retry(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_AWAITING_RELEASE;
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_TIMEOUT_T1,
	    0, 0, &result) != KN_AX25_STATE_OK)
		return 1;

	return connection.state == KN_AX25_CONNECTION_AWAITING_RELEASE &&
	    action_has(&result, KN_AX25_ACTION_SEND_DISC) ? 0 : 1;
}

static int
test_awaiting_release_retry_exhausted(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_AWAITING_RELEASE;
	connection.retry_count = 2;
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_TIMEOUT_T1,
	    0, 0, &result) != KN_AX25_STATE_ERR_INVALID_EVENT)
		return 1;

	return connection.state == KN_AX25_CONNECTION_DISCONNECTED &&
	    action_has(&result, KN_AX25_ACTION_PROTOCOL_ERROR) ? 0 : 1;
}

static int
test_awaiting_release_rx_ua(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_AWAITING_RELEASE;
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_RX_UA, 0, 0,
	    &result) != KN_AX25_STATE_OK)
		return 1;

	return connection.state == KN_AX25_CONNECTION_DISCONNECTED &&
	    action_has(&result, KN_AX25_ACTION_STOP_T1) ? 0 : 1;
}

static int
test_connected_invalid_i(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_CONNECTED;
	connection.receive_state = 2;
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_RX_I, 1, 0,
	    &result) != KN_AX25_STATE_ERR_SEQUENCE)
		return 1;

	return connection.receive_state == 2 &&
	    action_has(&result, KN_AX25_ACTION_SEND_REJ) ? 0 : 1;
}

static int
test_connected_invalid_i_ack(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_CONNECTED;

	return step_event(&connection, KN_AX25_CONNECTION_EVENT_RX_I, 0, 8,
	    &result) == KN_AX25_STATE_ERR_SEQUENCE ? 0 : 1;
}

static int
test_connected_local_disconnect(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_CONNECTED;
	if (step_event(&connection,
	    KN_AX25_CONNECTION_EVENT_LOCAL_DISCONNECT_REQUEST, 0, 0,
	    &result) != KN_AX25_STATE_OK)
		return 1;

	return connection.state == KN_AX25_CONNECTION_AWAITING_RELEASE &&
	    action_has(&result, KN_AX25_ACTION_SEND_DISC) ? 0 : 1;
}

static int
test_connected_rx_disc(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_CONNECTED;
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_RX_DISC, 0, 0,
	    &result) != KN_AX25_STATE_OK)
		return 1;

	return connection.state == KN_AX25_CONNECTION_DISCONNECTED &&
	    action_has(&result, KN_AX25_ACTION_SEND_UA) ? 0 : 1;
}

static int
test_connected_rx_dm(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_CONNECTED;
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_RX_DM, 0, 0,
	    &result) != KN_AX25_STATE_OK)
		return 1;

	return connection.state == KN_AX25_CONNECTION_DISCONNECTED &&
	    action_has(&result, KN_AX25_ACTION_STOP_T3) ? 0 : 1;
}

static int
test_connected_rx_i(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_CONNECTED;
	connection.receive_state = 0;
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_RX_I, 0, 3,
	    &result) != KN_AX25_STATE_OK)
		return 1;
	if (connection.receive_state != 1)
		return 1;
	if (connection.acknowledge_state != 3)
		return 1;

	return action_has(&result, KN_AX25_ACTION_DELIVER_I_PAYLOAD) &&
	    action_has(&result, KN_AX25_ACTION_SEND_RR) ? 0 : 1;
}

static int
test_connected_rx_rej(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_CONNECTED;
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_RX_REJ, 0, 5,
	    &result) != KN_AX25_STATE_OK)
		return 1;

	return connection.retransmit_needed == 1 &&
	    action_has(&result, KN_AX25_ACTION_RETRANSMIT_NEEDED) ? 0 : 1;
}

static int
test_connected_rx_rnr(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_CONNECTED;
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_RX_RNR, 0, 4,
	    &result) != KN_AX25_STATE_OK)
		return 1;

	return connection.remote_busy == 1 &&
	    connection.acknowledge_state == 4 ? 0 : 1;
}

static int
test_connected_rx_rr(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.state = KN_AX25_CONNECTION_CONNECTED;
	connection.remote_busy = 1;
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_RX_RR, 0, 6,
	    &result) != KN_AX25_STATE_OK)
		return 1;

	return connection.remote_busy == 0 &&
	    connection.acknowledge_state == 6 ? 0 : 1;
}

static int
test_disabled_rejects(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	kn_ax25_connection_init(&connection, NULL);

	return step_event(&connection,
	    KN_AX25_CONNECTION_EVENT_LOCAL_CONNECT_REQUEST, 0, 0,
	    &result) == KN_AX25_STATE_ERR_DISABLED ? 0 : 1;
}

static int
test_disconnected_local_connect(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	if (step_event(&connection,
	    KN_AX25_CONNECTION_EVENT_LOCAL_CONNECT_REQUEST, 0, 0,
	    &result) != KN_AX25_STATE_OK)
		return 1;
	if (connection.state != KN_AX25_CONNECTION_AWAITING_CONNECTION)
		return 1;

	return action_has(&result, KN_AX25_ACTION_SEND_SABM) &&
	    action_has(&result, KN_AX25_ACTION_START_T1) ? 0 : 1;
}

static int
test_disconnected_rx_sabm(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	connection.receive_state = 4;
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_RX_SABM, 0, 0,
	    &result) != KN_AX25_STATE_OK)
		return 1;
	if (connection.receive_state != 0)
		return 1;

	return connection.state == KN_AX25_CONNECTION_CONNECTED &&
	    action_has(&result, KN_AX25_ACTION_SEND_UA) ? 0 : 1;
}

static int
test_disconnected_rx_sabme(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	if (step_event(&connection, KN_AX25_CONNECTION_EVENT_RX_SABME, 0, 0,
	    &result) != KN_AX25_STATE_OK)
		return 1;

	return connection.state == KN_AX25_CONNECTION_CONNECTED &&
	    action_has(&result, KN_AX25_ACTION_SEND_UA) ? 0 : 1;
}

static int
test_no_tx_queue_write(void)
{
	struct kn_ax25_connection connection;
	struct kn_ax25_state_result result;

	connection_enabled(&connection);
	if (step_event(&connection,
	    KN_AX25_CONNECTION_EVENT_LOCAL_CONNECT_REQUEST, 0, 0,
	    &result) != KN_AX25_STATE_OK)
		return 1;

	return action_has(&result, KN_AX25_ACTION_SEND_SABM) &&
	    result.actions.count < KN_AX25_ACTION_LIST_MAX ? 0 : 1;
}
