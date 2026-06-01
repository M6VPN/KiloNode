/* KiloNode - Developed by M6VPN (M6VPN@tuta.com) */
/* kilonode/src/proto/ax25_sequence.c */

#include <sys/types.h>

#include <string.h>

#include "kilonode/ax25_sequence.h"

enum kn_ax25_sequence_error
kn_ax25_sequence_ack_update_mod8(struct kn_ax25_sequence_state *state,
	uint8_t nr)
{
	if (state == NULL)
		return KN_AX25_SEQUENCE_ERR_INVALID_ARGUMENT;
	if (nr > 7)
		return KN_AX25_SEQUENCE_ERR_INVALID_VALUE;

	state->acknowledge_state = nr;
	state->remote_busy = 0;
	return KN_AX25_SEQUENCE_OK;
}

uint8_t
kn_ax25_sequence_expected_mod8(const struct kn_ax25_sequence_state *state,
	uint8_t ns)
{
	if (state == NULL || ns > 7)
		return 0;

	return (uint8_t)(state->receive_state == ns);
}

uint8_t
kn_ax25_sequence_increment_mod8(uint8_t sequence)
{
	return (uint8_t)((sequence + 1U) & 0x07U);
}

enum kn_ax25_sequence_error
kn_ax25_sequence_mark_rej_mod8(struct kn_ax25_sequence_state *state,
	uint8_t nr)
{
	if (state == NULL)
		return KN_AX25_SEQUENCE_ERR_INVALID_ARGUMENT;
	if (nr > 7)
		return KN_AX25_SEQUENCE_ERR_INVALID_VALUE;

	state->acknowledge_state = nr;
	state->remote_busy = 0;
	state->retransmit_needed = 1;
	return KN_AX25_SEQUENCE_OK;
}

enum kn_ax25_sequence_error
kn_ax25_sequence_mark_rnr_mod8(struct kn_ax25_sequence_state *state,
	uint8_t nr)
{
	if (state == NULL)
		return KN_AX25_SEQUENCE_ERR_INVALID_ARGUMENT;
	if (nr > 7)
		return KN_AX25_SEQUENCE_ERR_INVALID_VALUE;

	state->acknowledge_state = nr;
	state->remote_busy = 1;
	return KN_AX25_SEQUENCE_OK;
}

enum kn_ax25_sequence_error
kn_ax25_sequence_mod128_planned(void)
{
	return KN_AX25_SEQUENCE_ERR_NOT_IMPLEMENTED;
}

uint8_t
kn_ax25_sequence_normalize_mod8(uint8_t sequence)
{
	return (uint8_t)(sequence & 0x07U);
}

enum kn_ax25_sequence_error
kn_ax25_sequence_receive_i_mod8(struct kn_ax25_sequence_state *state,
	uint8_t ns)
{
	if (state == NULL)
		return KN_AX25_SEQUENCE_ERR_INVALID_ARGUMENT;
	if (ns > 7)
		return KN_AX25_SEQUENCE_ERR_INVALID_VALUE;
	if (state->receive_state != ns)
		return KN_AX25_SEQUENCE_ERR_INVALID_VALUE;

	state->receive_state = kn_ax25_sequence_increment_mod8(
	    state->receive_state);
	return KN_AX25_SEQUENCE_OK;
}

enum kn_ax25_sequence_error
kn_ax25_sequence_send_i_mod8(struct kn_ax25_sequence_state *state,
	uint8_t *ns)
{
	if (state == NULL || ns == NULL)
		return KN_AX25_SEQUENCE_ERR_INVALID_ARGUMENT;
	if (state->send_state > 7)
		return KN_AX25_SEQUENCE_ERR_INVALID_VALUE;

	*ns = state->send_state;
	state->send_state = kn_ax25_sequence_increment_mod8(
	    state->send_state);
	return KN_AX25_SEQUENCE_OK;
}

void
kn_ax25_sequence_state_clear(struct kn_ax25_sequence_state *state)
{
	if (state == NULL)
		return;

	memset(state, 0, sizeof(*state));
}

enum kn_ax25_sequence_error
kn_ax25_sequence_window_valid(enum kn_ax25_modulo_mode mode, uint8_t window)
{
	if (window == 0)
		return KN_AX25_SEQUENCE_ERR_INVALID_VALUE;
	if (mode == KN_AX25_MODULO_8)
		return window <= 7 ? KN_AX25_SEQUENCE_OK :
		    KN_AX25_SEQUENCE_ERR_INVALID_VALUE;
	if (mode == KN_AX25_MODULO_128)
		return window <= 63 ? KN_AX25_SEQUENCE_OK :
		    KN_AX25_SEQUENCE_ERR_INVALID_VALUE;

	return KN_AX25_SEQUENCE_ERR_INVALID_VALUE;
}
