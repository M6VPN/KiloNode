/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/include/kilonode/ax25_state.h */

#ifndef KILONODE_AX25_STATE_H
#define KILONODE_AX25_STATE_H

#include <sys/types.h>

#include <stdint.h>

#include "kilonode/ax25_action.h"
#include "kilonode/ax25_connection.h"

enum kn_ax25_state_error {
	KN_AX25_STATE_OK = 0,
	KN_AX25_STATE_ERR_INVALID_ARGUMENT,
	KN_AX25_STATE_ERR_DISABLED,
	KN_AX25_STATE_ERR_INVALID_EVENT,
	KN_AX25_STATE_ERR_SEQUENCE,
	KN_AX25_STATE_ERR_ACTION_OVERFLOW
};

struct kn_ax25_state_input {
	enum kn_ax25_connection_event event;
	uint8_t ns;
	uint8_t nr;
	uint8_t poll_final;
	size_t payload_len;
};

struct kn_ax25_state_result {
	enum kn_ax25_state_error status;
	enum kn_ax25_connection_state previous_state;
	enum kn_ax25_connection_state new_state;
	struct kn_ax25_action_list actions;
};

void kn_ax25_state_input_clear(struct kn_ax25_state_input *);
enum kn_ax25_state_error kn_ax25_state_step(struct kn_ax25_connection *,
	const struct kn_ax25_state_input *, struct kn_ax25_state_result *);
const char *kn_ax25_state_error_name(enum kn_ax25_state_error);
void kn_ax25_state_result_clear(struct kn_ax25_state_result *);

#endif
