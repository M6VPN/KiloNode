/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_state.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_sequence.h"
#include "kilonode/ax25_state.h"

static enum kn_ax25_state_error add_action(
	struct kn_ax25_state_result *, enum kn_ax25_action_intent);
static enum kn_ax25_state_error add_action_sequence(
	struct kn_ax25_state_result *, enum kn_ax25_action_intent, uint8_t);
static enum kn_ax25_state_error handle_awaiting_connection(
	struct kn_ax25_connection *, const struct kn_ax25_state_input *,
	struct kn_ax25_state_result *);
static enum kn_ax25_state_error handle_awaiting_release(
	struct kn_ax25_connection *, const struct kn_ax25_state_input *,
	struct kn_ax25_state_result *);
static enum kn_ax25_state_error handle_connected(
	struct kn_ax25_connection *, const struct kn_ax25_state_input *,
	struct kn_ax25_state_result *);
static enum kn_ax25_state_error handle_disconnected(
	struct kn_ax25_connection *, const struct kn_ax25_state_input *,
	struct kn_ax25_state_result *);
static enum kn_ax25_state_error retry_or_disconnect(
	struct kn_ax25_connection *, struct kn_ax25_state_result *,
	enum kn_ax25_action_intent);
static void reset_sequences(struct kn_ax25_connection *);
static void sync_result(struct kn_ax25_connection *,
	struct kn_ax25_state_result *);

static enum kn_ax25_state_error
add_action(struct kn_ax25_state_result *result,
	enum kn_ax25_action_intent intent)
{
	if (kn_ax25_action_list_append(&result->actions, intent) !=
	    KN_AX25_ACTION_OK)
		return KN_AX25_STATE_ERR_ACTION_OVERFLOW;

	return KN_AX25_STATE_OK;
}

static enum kn_ax25_state_error
add_action_sequence(struct kn_ax25_state_result *result,
	enum kn_ax25_action_intent intent, uint8_t sequence)
{
	if (kn_ax25_action_list_append_sequence(&result->actions, intent,
	    sequence) != KN_AX25_ACTION_OK)
		return KN_AX25_STATE_ERR_ACTION_OVERFLOW;

	return KN_AX25_STATE_OK;
}

static enum kn_ax25_state_error
handle_awaiting_connection(struct kn_ax25_connection *connection,
	const struct kn_ax25_state_input *input,
	struct kn_ax25_state_result *result)
{
	switch (input->event) {
	case KN_AX25_CONNECTION_EVENT_RX_UA:
		connection->state = KN_AX25_CONNECTION_CONNECTED;
		connection->retry_count = 0;
		if (add_action(result, KN_AX25_ACTION_STOP_T1) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_START_T3) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_RESET_RETRY_COUNT) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_ENTER_CONNECTED) !=
		    KN_AX25_STATE_OK)
			return KN_AX25_STATE_ERR_ACTION_OVERFLOW;
		return KN_AX25_STATE_OK;
	case KN_AX25_CONNECTION_EVENT_RX_DM:
		connection->state = KN_AX25_CONNECTION_DISCONNECTED;
		if (add_action(result, KN_AX25_ACTION_STOP_T1) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_ENTER_DISCONNECTED) !=
		    KN_AX25_STATE_OK)
			return KN_AX25_STATE_ERR_ACTION_OVERFLOW;
		return KN_AX25_STATE_OK;
	case KN_AX25_CONNECTION_EVENT_TIMEOUT_T1:
		return retry_or_disconnect(connection, result,
		    connection->use_sabme != 0 ? KN_AX25_ACTION_SEND_SABME :
		    KN_AX25_ACTION_SEND_SABM);
	case KN_AX25_CONNECTION_EVENT_LOCAL_DISCONNECT_REQUEST:
		connection->state = KN_AX25_CONNECTION_DISCONNECTED;
		if (add_action(result, KN_AX25_ACTION_STOP_T1) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_ENTER_DISCONNECTED) !=
		    KN_AX25_STATE_OK)
			return KN_AX25_STATE_ERR_ACTION_OVERFLOW;
		return KN_AX25_STATE_OK;
	default:
		break;
	}

	if (add_action(result, KN_AX25_ACTION_PROTOCOL_ERROR) !=
	    KN_AX25_STATE_OK)
		return KN_AX25_STATE_ERR_ACTION_OVERFLOW;

	return KN_AX25_STATE_ERR_INVALID_EVENT;
}

static enum kn_ax25_state_error
handle_awaiting_release(struct kn_ax25_connection *connection,
	const struct kn_ax25_state_input *input,
	struct kn_ax25_state_result *result)
{
	switch (input->event) {
	case KN_AX25_CONNECTION_EVENT_RX_UA:
	case KN_AX25_CONNECTION_EVENT_RX_DM:
		connection->state = KN_AX25_CONNECTION_DISCONNECTED;
		if (add_action(result, KN_AX25_ACTION_STOP_T1) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_ENTER_DISCONNECTED) !=
		    KN_AX25_STATE_OK)
			return KN_AX25_STATE_ERR_ACTION_OVERFLOW;
		return KN_AX25_STATE_OK;
	case KN_AX25_CONNECTION_EVENT_TIMEOUT_T1:
		return retry_or_disconnect(connection, result,
		    KN_AX25_ACTION_SEND_DISC);
	default:
		break;
	}

	if (add_action(result, KN_AX25_ACTION_PROTOCOL_ERROR) !=
	    KN_AX25_STATE_OK)
		return KN_AX25_STATE_ERR_ACTION_OVERFLOW;

	return KN_AX25_STATE_ERR_INVALID_EVENT;
}

static enum kn_ax25_state_error
handle_connected(struct kn_ax25_connection *connection,
	const struct kn_ax25_state_input *input,
	struct kn_ax25_state_result *result)
{
	struct kn_ax25_sequence_state sequence;

	sequence.send_state = connection->send_state;
	sequence.receive_state = connection->receive_state;
	sequence.acknowledge_state = connection->acknowledge_state;
	sequence.remote_busy = connection->remote_busy;
	sequence.retransmit_needed = connection->retransmit_needed;

	switch (input->event) {
	case KN_AX25_CONNECTION_EVENT_LOCAL_DISCONNECT_REQUEST:
		connection->state = KN_AX25_CONNECTION_AWAITING_RELEASE;
		connection->retry_count = 0;
		if (add_action(result, KN_AX25_ACTION_SEND_DISC) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_START_T1) !=
		    KN_AX25_STATE_OK)
			return KN_AX25_STATE_ERR_ACTION_OVERFLOW;
		return KN_AX25_STATE_OK;
	case KN_AX25_CONNECTION_EVENT_RX_DISC:
		connection->state = KN_AX25_CONNECTION_DISCONNECTED;
		if (add_action(result, KN_AX25_ACTION_SEND_UA) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_STOP_T1) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_STOP_T3) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_ENTER_DISCONNECTED) !=
		    KN_AX25_STATE_OK)
			return KN_AX25_STATE_ERR_ACTION_OVERFLOW;
		return KN_AX25_STATE_OK;
	case KN_AX25_CONNECTION_EVENT_RX_DM:
		connection->state = KN_AX25_CONNECTION_DISCONNECTED;
		if (add_action(result, KN_AX25_ACTION_STOP_T1) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_STOP_T3) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_ENTER_DISCONNECTED) !=
		    KN_AX25_STATE_OK)
			return KN_AX25_STATE_ERR_ACTION_OVERFLOW;
		return KN_AX25_STATE_OK;
	case KN_AX25_CONNECTION_EVENT_RX_I:
		if (input->nr > 7)
			return KN_AX25_STATE_ERR_SEQUENCE;
		if (kn_ax25_sequence_receive_i_mod8(&sequence, input->ns) !=
		    KN_AX25_SEQUENCE_OK) {
			if (add_action_sequence(result,
			    KN_AX25_ACTION_SEND_REJ,
			    connection->receive_state) != KN_AX25_STATE_OK)
				return KN_AX25_STATE_ERR_ACTION_OVERFLOW;
			connection->retransmit_needed = 1;
			return KN_AX25_STATE_ERR_SEQUENCE;
		}
		connection->receive_state = sequence.receive_state;
		if (kn_ax25_sequence_ack_update_mod8(&sequence, input->nr) ==
		    KN_AX25_SEQUENCE_OK)
			connection->acknowledge_state =
			    sequence.acknowledge_state;
		if (add_action_sequence(result,
		    KN_AX25_ACTION_DELIVER_I_PAYLOAD, input->ns) !=
		    KN_AX25_STATE_OK ||
		    add_action_sequence(result, KN_AX25_ACTION_SEND_RR,
		    connection->receive_state) != KN_AX25_STATE_OK)
			return KN_AX25_STATE_ERR_ACTION_OVERFLOW;
		return KN_AX25_STATE_OK;
	case KN_AX25_CONNECTION_EVENT_RX_RR:
		if (kn_ax25_sequence_ack_update_mod8(&sequence, input->nr) !=
		    KN_AX25_SEQUENCE_OK)
			return KN_AX25_STATE_ERR_SEQUENCE;
		connection->acknowledge_state = sequence.acknowledge_state;
		connection->remote_busy = 0;
		return KN_AX25_STATE_OK;
	case KN_AX25_CONNECTION_EVENT_RX_RNR:
		if (kn_ax25_sequence_mark_rnr_mod8(&sequence, input->nr) !=
		    KN_AX25_SEQUENCE_OK)
			return KN_AX25_STATE_ERR_SEQUENCE;
		connection->acknowledge_state = sequence.acknowledge_state;
		connection->remote_busy = sequence.remote_busy;
		return KN_AX25_STATE_OK;
	case KN_AX25_CONNECTION_EVENT_RX_REJ:
		if (kn_ax25_sequence_mark_rej_mod8(&sequence, input->nr) !=
		    KN_AX25_SEQUENCE_OK)
			return KN_AX25_STATE_ERR_SEQUENCE;
		connection->acknowledge_state = sequence.acknowledge_state;
		connection->remote_busy = sequence.remote_busy;
		connection->retransmit_needed = sequence.retransmit_needed;
		return add_action_sequence(result,
		    KN_AX25_ACTION_RETRANSMIT_NEEDED, input->nr);
	case KN_AX25_CONNECTION_EVENT_TIMEOUT_T3:
		return add_action_sequence(result, KN_AX25_ACTION_SEND_RR,
		    connection->receive_state);
	case KN_AX25_CONNECTION_EVENT_RX_FRMR:
		return add_action(result, KN_AX25_ACTION_PROTOCOL_ERROR);
	default:
		break;
	}

	if (add_action(result, KN_AX25_ACTION_PROTOCOL_ERROR) !=
	    KN_AX25_STATE_OK)
		return KN_AX25_STATE_ERR_ACTION_OVERFLOW;

	return KN_AX25_STATE_ERR_INVALID_EVENT;
}

static enum kn_ax25_state_error
handle_disconnected(struct kn_ax25_connection *connection,
	const struct kn_ax25_state_input *input,
	struct kn_ax25_state_result *result)
{
	switch (input->event) {
	case KN_AX25_CONNECTION_EVENT_LOCAL_CONNECT_REQUEST:
		connection->state = KN_AX25_CONNECTION_AWAITING_CONNECTION;
		connection->retry_count = 0;
		if (add_action(result, connection->use_sabme != 0 ?
		    KN_AX25_ACTION_SEND_SABME : KN_AX25_ACTION_SEND_SABM) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_START_T1) !=
		    KN_AX25_STATE_OK)
			return KN_AX25_STATE_ERR_ACTION_OVERFLOW;
		return KN_AX25_STATE_OK;
	case KN_AX25_CONNECTION_EVENT_RX_SABM:
	case KN_AX25_CONNECTION_EVENT_RX_SABME:
		reset_sequences(connection);
		connection->state = KN_AX25_CONNECTION_CONNECTED;
		if (add_action(result, KN_AX25_ACTION_SEND_UA) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_RESET_RETRY_COUNT) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_START_T3) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_ENTER_CONNECTED) !=
		    KN_AX25_STATE_OK)
			return KN_AX25_STATE_ERR_ACTION_OVERFLOW;
		return KN_AX25_STATE_OK;
	case KN_AX25_CONNECTION_EVENT_RX_DISC:
		return add_action(result, KN_AX25_ACTION_SEND_DM);
	case KN_AX25_CONNECTION_EVENT_RX_I:
	case KN_AX25_CONNECTION_EVENT_RX_RR:
	case KN_AX25_CONNECTION_EVENT_RX_RNR:
	case KN_AX25_CONNECTION_EVENT_RX_REJ:
		if (add_action(result, KN_AX25_ACTION_SEND_DM) !=
		    KN_AX25_STATE_OK)
			return KN_AX25_STATE_ERR_ACTION_OVERFLOW;
		return KN_AX25_STATE_ERR_INVALID_EVENT;
	default:
		break;
	}

	return KN_AX25_STATE_ERR_INVALID_EVENT;
}

static enum kn_ax25_state_error
retry_or_disconnect(struct kn_ax25_connection *connection,
	struct kn_ax25_state_result *result, enum kn_ax25_action_intent retry)
{
	if (connection->retry_count + 1U < connection->params.n2_retry_count) {
		connection->retry_count++;
		if (add_action(result, retry) != KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_INCREMENT_RETRY_COUNT) !=
		    KN_AX25_STATE_OK ||
		    add_action(result, KN_AX25_ACTION_START_T1) !=
		    KN_AX25_STATE_OK)
			return KN_AX25_STATE_ERR_ACTION_OVERFLOW;
		return KN_AX25_STATE_OK;
	}

	connection->state = KN_AX25_CONNECTION_DISCONNECTED;
	if (add_action(result, KN_AX25_ACTION_STOP_T1) != KN_AX25_STATE_OK ||
	    add_action(result, KN_AX25_ACTION_PROTOCOL_ERROR) !=
	    KN_AX25_STATE_OK ||
	    add_action(result, KN_AX25_ACTION_ENTER_DISCONNECTED) !=
	    KN_AX25_STATE_OK)
		return KN_AX25_STATE_ERR_ACTION_OVERFLOW;
	return KN_AX25_STATE_ERR_INVALID_EVENT;
}

static void
reset_sequences(struct kn_ax25_connection *connection)
{
	connection->send_state = 0;
	connection->receive_state = 0;
	connection->acknowledge_state = 0;
	connection->remote_busy = 0;
	connection->retransmit_needed = 0;
	connection->retry_count = 0;
}

static void
sync_result(struct kn_ax25_connection *connection,
	struct kn_ax25_state_result *result)
{
	result->new_state = connection->state;
}

void
kn_ax25_state_input_clear(struct kn_ax25_state_input *input)
{
	if (input == NULL)
		return;

	memset(input, 0, sizeof(*input));
}

enum kn_ax25_state_error
kn_ax25_state_step(struct kn_ax25_connection *connection,
	const struct kn_ax25_state_input *input,
	struct kn_ax25_state_result *result)
{
	enum kn_ax25_state_error rc;

	if (connection == NULL || input == NULL || result == NULL)
		return KN_AX25_STATE_ERR_INVALID_ARGUMENT;

	kn_ax25_state_result_clear(result);
	result->previous_state = connection->state;
	result->new_state = connection->state;

	if (connection->state == KN_AX25_CONNECTION_DISABLED ||
	    connection->params.allow_connected_mode == 0) {
		result->status = KN_AX25_STATE_ERR_DISABLED;
		return KN_AX25_STATE_ERR_DISABLED;
	}
	if (kn_ax25_params_validate(&connection->params) !=
	    KN_AX25_PARAMS_OK) {
		result->status = KN_AX25_STATE_ERR_INVALID_ARGUMENT;
		return KN_AX25_STATE_ERR_INVALID_ARGUMENT;
	}

	switch (connection->state) {
	case KN_AX25_CONNECTION_DISCONNECTED:
		rc = handle_disconnected(connection, input, result);
		break;
	case KN_AX25_CONNECTION_AWAITING_CONNECTION:
		rc = handle_awaiting_connection(connection, input, result);
		break;
	case KN_AX25_CONNECTION_CONNECTED:
		rc = handle_connected(connection, input, result);
		break;
	case KN_AX25_CONNECTION_AWAITING_RELEASE:
		rc = handle_awaiting_release(connection, input, result);
		break;
	case KN_AX25_CONNECTION_TIMER_RECOVERY:
		rc = add_action(result, KN_AX25_ACTION_PROTOCOL_ERROR);
		if (rc == KN_AX25_STATE_OK)
			rc = KN_AX25_STATE_ERR_INVALID_EVENT;
		break;
	case KN_AX25_CONNECTION_DISABLED:
		rc = KN_AX25_STATE_ERR_DISABLED;
		break;
	default:
		rc = KN_AX25_STATE_ERR_INVALID_ARGUMENT;
		break;
	}

	sync_result(connection, result);
	result->status = rc;
	return rc;
}

const char *
kn_ax25_state_error_name(enum kn_ax25_state_error error)
{
	switch (error) {
	case KN_AX25_STATE_OK:
		return "ok";
	case KN_AX25_STATE_ERR_INVALID_ARGUMENT:
		return "invalid-argument";
	case KN_AX25_STATE_ERR_DISABLED:
		return "disabled";
	case KN_AX25_STATE_ERR_INVALID_EVENT:
		return "invalid-event";
	case KN_AX25_STATE_ERR_SEQUENCE:
		return "sequence";
	case KN_AX25_STATE_ERR_ACTION_OVERFLOW:
		return "action-overflow";
	}

	return "unknown";
}

void
kn_ax25_state_result_clear(struct kn_ax25_state_result *result)
{
	if (result == NULL)
		return;

	memset(result, 0, sizeof(*result));
	kn_ax25_action_list_clear(&result->actions);
}
