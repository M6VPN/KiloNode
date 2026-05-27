/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_connection.h */

#ifndef KILONODE_AX25_CONNECTION_H
#define KILONODE_AX25_CONNECTION_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_control.h"
#include "kilonode/ax25_params.h"
#include "kilonode/callsign.h"

enum kn_ax25_connection_error {
	KN_AX25_CONNECTION_OK = 0,
	KN_AX25_CONNECTION_ERR_INVALID_ARGUMENT,
	KN_AX25_CONNECTION_ERR_INVALID_VALUE,
	KN_AX25_CONNECTION_ERR_BUFFER
};

enum kn_ax25_connection_state {
	KN_AX25_CONNECTION_DISABLED = 0,
	KN_AX25_CONNECTION_DISCONNECTED,
	KN_AX25_CONNECTION_AWAITING_CONNECTION,
	KN_AX25_CONNECTION_CONNECTED,
	KN_AX25_CONNECTION_AWAITING_RELEASE,
	KN_AX25_CONNECTION_TIMER_RECOVERY
};

enum kn_ax25_connection_event {
	KN_AX25_CONNECTION_EVENT_NONE = 0,
	KN_AX25_CONNECTION_EVENT_RX_SABM,
	KN_AX25_CONNECTION_EVENT_RX_SABME,
	KN_AX25_CONNECTION_EVENT_RX_UA,
	KN_AX25_CONNECTION_EVENT_RX_DISC,
	KN_AX25_CONNECTION_EVENT_RX_DM,
	KN_AX25_CONNECTION_EVENT_RX_I,
	KN_AX25_CONNECTION_EVENT_RX_RR,
	KN_AX25_CONNECTION_EVENT_RX_RNR,
	KN_AX25_CONNECTION_EVENT_RX_REJ,
	KN_AX25_CONNECTION_EVENT_RX_FRMR,
	KN_AX25_CONNECTION_EVENT_TIMEOUT_T1,
	KN_AX25_CONNECTION_EVENT_TIMEOUT_T3,
	KN_AX25_CONNECTION_EVENT_LOCAL_CONNECT_REQUEST,
	KN_AX25_CONNECTION_EVENT_LOCAL_DISCONNECT_REQUEST
};

struct kn_ax25_connection {
	enum kn_ax25_connection_state state;
	struct kn_ax25_params params;
	struct kn_callsign local;
	struct kn_callsign remote;
	uint8_t send_state;
	uint8_t receive_state;
	uint8_t acknowledge_state;
	uint8_t tx_output_allowed;
};

enum kn_ax25_connection_event kn_ax25_connection_event_from_control(
	uint8_t);
void kn_ax25_connection_init(struct kn_ax25_connection *,
	const struct kn_ax25_params *);
enum kn_ax25_connection_error kn_ax25_connection_format(
	const struct kn_ax25_connection *, char *, size_t);
void kn_ax25_connection_reset(struct kn_ax25_connection *);
const char *kn_ax25_connection_event_name(enum kn_ax25_connection_event);
const char *kn_ax25_connection_state_name(enum kn_ax25_connection_state);
enum kn_ax25_connection_error kn_ax25_connection_validate(
	const struct kn_ax25_connection *);

#endif
