/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_connection.c */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "kilonode/ax25_connection.h"

enum kn_ax25_connection_event
kn_ax25_connection_event_from_control(uint8_t control)
{
	struct kn_ax25_control_info info;

	kn_ax25_control_decode(control, &info);
	switch (info.class) {
	case KN_AX25_CONTROL_CLASS_UI:
		return KN_AX25_CONNECTION_EVENT_NONE;
	case KN_AX25_CONTROL_CLASS_I:
		return KN_AX25_CONNECTION_EVENT_RX_I;
	case KN_AX25_CONTROL_CLASS_S:
		switch (info.s_subtype) {
		case KN_AX25_S_SUBTYPE_RR:
			return KN_AX25_CONNECTION_EVENT_RX_RR;
		case KN_AX25_S_SUBTYPE_RNR:
			return KN_AX25_CONNECTION_EVENT_RX_RNR;
		case KN_AX25_S_SUBTYPE_REJ:
		case KN_AX25_S_SUBTYPE_SREJ:
			return KN_AX25_CONNECTION_EVENT_RX_REJ;
		case KN_AX25_S_SUBTYPE_UNKNOWN:
			return KN_AX25_CONNECTION_EVENT_NONE;
		}
		break;
	case KN_AX25_CONTROL_CLASS_U:
		switch (info.u_subtype) {
		case KN_AX25_U_SUBTYPE_SABM:
			return KN_AX25_CONNECTION_EVENT_RX_SABM;
		case KN_AX25_U_SUBTYPE_SABME:
			return KN_AX25_CONNECTION_EVENT_RX_SABME;
		case KN_AX25_U_SUBTYPE_DISC:
			return KN_AX25_CONNECTION_EVENT_RX_DISC;
		case KN_AX25_U_SUBTYPE_DM:
			return KN_AX25_CONNECTION_EVENT_RX_DM;
		case KN_AX25_U_SUBTYPE_UA:
			return KN_AX25_CONNECTION_EVENT_RX_UA;
		case KN_AX25_U_SUBTYPE_FRMR:
			return KN_AX25_CONNECTION_EVENT_RX_FRMR;
		case KN_AX25_U_SUBTYPE_UI:
		case KN_AX25_U_SUBTYPE_XID:
		case KN_AX25_U_SUBTYPE_TEST:
		case KN_AX25_U_SUBTYPE_UNKNOWN:
			return KN_AX25_CONNECTION_EVENT_NONE;
		}
		break;
	case KN_AX25_CONTROL_CLASS_UNKNOWN:
		return KN_AX25_CONNECTION_EVENT_NONE;
	}

	return KN_AX25_CONNECTION_EVENT_NONE;
}

void
kn_ax25_connection_init(struct kn_ax25_connection *connection,
	const struct kn_ax25_params *params)
{
	struct kn_ax25_params defaults;

	if (connection == NULL)
		return;

	memset(connection, 0, sizeof(*connection));
	if (params == NULL) {
		kn_ax25_params_default(&defaults);
		connection->params = defaults;
	} else {
		connection->params = *params;
	}

	connection->state = connection->params.allow_connected_mode != 0 ?
	    KN_AX25_CONNECTION_DISCONNECTED : KN_AX25_CONNECTION_DISABLED;
	connection->tx_output_allowed = 0;
}

enum kn_ax25_connection_error
kn_ax25_connection_format(const struct kn_ax25_connection *connection,
	char *buf, size_t bufsiz)
{
	int needed;

	if (connection == NULL || buf == NULL || bufsiz == 0)
		return KN_AX25_CONNECTION_ERR_INVALID_ARGUMENT;

	needed = snprintf(buf, bufsiz,
	    "state=%s modulo=%s tx_output=false vs=%u vr=%u va=%u "
	    "retries=%u remote_busy=%s retransmit=%s",
	    kn_ax25_connection_state_name(connection->state),
	    kn_ax25_params_modulo_name(connection->params.modulo_mode),
	    (unsigned int)connection->send_state,
	    (unsigned int)connection->receive_state,
	    (unsigned int)connection->acknowledge_state,
	    (unsigned int)connection->retry_count,
	    connection->remote_busy != 0 ? "true" : "false",
	    connection->retransmit_needed != 0 ? "true" : "false");
	if (needed < 0 || (size_t)needed >= bufsiz)
		return KN_AX25_CONNECTION_ERR_BUFFER;

	return KN_AX25_CONNECTION_OK;
}

void
kn_ax25_connection_reset(struct kn_ax25_connection *connection)
{
	struct kn_ax25_params params;

	if (connection == NULL)
		return;

	params = connection->params;
	kn_ax25_connection_init(connection, &params);
}

const char *
kn_ax25_connection_event_name(enum kn_ax25_connection_event event)
{
	switch (event) {
	case KN_AX25_CONNECTION_EVENT_NONE:
		return "none";
	case KN_AX25_CONNECTION_EVENT_RX_SABM:
		return "rx-sabm";
	case KN_AX25_CONNECTION_EVENT_RX_SABME:
		return "rx-sabme";
	case KN_AX25_CONNECTION_EVENT_RX_UA:
		return "rx-ua";
	case KN_AX25_CONNECTION_EVENT_RX_DISC:
		return "rx-disc";
	case KN_AX25_CONNECTION_EVENT_RX_DM:
		return "rx-dm";
	case KN_AX25_CONNECTION_EVENT_RX_I:
		return "rx-i";
	case KN_AX25_CONNECTION_EVENT_RX_RR:
		return "rx-rr";
	case KN_AX25_CONNECTION_EVENT_RX_RNR:
		return "rx-rnr";
	case KN_AX25_CONNECTION_EVENT_RX_REJ:
		return "rx-rej";
	case KN_AX25_CONNECTION_EVENT_RX_FRMR:
		return "rx-frmr";
	case KN_AX25_CONNECTION_EVENT_TIMEOUT_T1:
		return "timeout-t1";
	case KN_AX25_CONNECTION_EVENT_TIMEOUT_T3:
		return "timeout-t3";
	case KN_AX25_CONNECTION_EVENT_LOCAL_CONNECT_REQUEST:
		return "local-connect-request";
	case KN_AX25_CONNECTION_EVENT_LOCAL_DISCONNECT_REQUEST:
		return "local-disconnect-request";
	}

	return "unknown";
}

const char *
kn_ax25_connection_state_name(enum kn_ax25_connection_state state)
{
	switch (state) {
	case KN_AX25_CONNECTION_DISABLED:
		return "disabled";
	case KN_AX25_CONNECTION_DISCONNECTED:
		return "disconnected";
	case KN_AX25_CONNECTION_AWAITING_CONNECTION:
		return "awaiting-connection";
	case KN_AX25_CONNECTION_CONNECTED:
		return "connected";
	case KN_AX25_CONNECTION_AWAITING_RELEASE:
		return "awaiting-release";
	case KN_AX25_CONNECTION_TIMER_RECOVERY:
		return "timer-recovery";
	}

	return "unknown";
}

enum kn_ax25_connection_error
kn_ax25_connection_validate(const struct kn_ax25_connection *connection)
{
	if (connection == NULL)
		return KN_AX25_CONNECTION_ERR_INVALID_ARGUMENT;
	if (kn_ax25_params_validate(&connection->params) !=
	    KN_AX25_PARAMS_OK)
		return KN_AX25_CONNECTION_ERR_INVALID_VALUE;
	if (connection->tx_output_allowed != 0)
		return KN_AX25_CONNECTION_ERR_INVALID_VALUE;
	if (connection->retry_count > connection->params.n2_retry_count)
		return KN_AX25_CONNECTION_ERR_INVALID_VALUE;
	if (connection->remote_busy > 1 || connection->retransmit_needed > 1 ||
	    connection->use_sabme > 1)
		return KN_AX25_CONNECTION_ERR_INVALID_VALUE;
	if (connection->send_state > 7 || connection->receive_state > 7 ||
	    connection->acknowledge_state > 7)
		return KN_AX25_CONNECTION_ERR_INVALID_VALUE;
	if (connection->state < KN_AX25_CONNECTION_DISABLED ||
	    connection->state > KN_AX25_CONNECTION_TIMER_RECOVERY)
		return KN_AX25_CONNECTION_ERR_INVALID_VALUE;

	return KN_AX25_CONNECTION_OK;
}
